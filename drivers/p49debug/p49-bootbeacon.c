/*
 * p49-bootbeacon - TEMPORARY bring-up instrumentation for the 4.9 port.
 *
 * THE REAL PROBLEM THIS SOLVES
 * ----------------------------
 * The 4.9 kernel hangs, and the only way back to a readable state is a HARD RESET
 * performed by a human. A hard reset re-initialises DRAM, which wipes the region
 * pstore writes into. So the panic record is destroyed by the very reset that
 * brings recovery up. That is why no log has ever been readable, no matter how
 * pstore is configured. Writing to eMMC does not help either, because the eMMC
 * block device does not exist until mmc's device_initcall, and the hang happens
 * before that.
 *
 * The fix is to make the kernel do the recording AND the restarting itself, so no
 * human reset is involved and the DRAM survives:
 *
 *   arch_initcall (level 3)  arm a timer for P49_TIMEOUT_MS
 *   ...                       the kernel carries on booting
 *   late_initcall (level 7)  reached => cancel the timer, log to eMMC, carry on
 *   P49_TIMEOUT_MS elapsed   => panic() => kmsg_dump(KMSG_DUMP_PANIC) writes the
 *                              record, then CONFIG_PANIC_TIMEOUT=5 makes the
 *                              kernel WARM-restart on its own
 *
 * Why a timer rather than an immediate panic: the suspected hang is a blocked
 * task -- a firmware read over I2C waiting for a controller that never answers --
 * not a stopped-interrupts hang. Interrupts are still running, so the timer still
 * fires, and by then the ring buffer holds the whole boot log INCLUDING the last
 * messages printed before the wedge. That log is the thing we actually want.
 *
 * If the kernel wedges BEFORE level 3, no timer is ever armed and there will be
 * no record; that is the next thing to bisect, and it would mean moving the arm
 * point earlier and accepting that pstore is not yet live to receive it.
 *
 * INITCALL LEVELS IN THIS TREE (measured from include/linux/init.h, not assumed):
 *   early_initcall=0 core_initcall=1 postcore_initcall=2 arch_initcall=3
 *   subsys_initcall=4 fs_initcall=5 device_initcall=6 late_initcall=7
 * Level 3 is the earliest point at which pstore is guaranteed usable, because
 * ramoops_init() is a postcore_initcall at level 2.
 *
 * Delete this whole directory once the hang is found. It has no business in a
 * shipping kernel, and it will deliberately panic a booting phone.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/blkdev.h>
#include <linux/mm.h>
#include <linux/kmsg_dump.h>
#include <linux/writeback.h>

#define P49_SCRATCH_NAME	"mmcblk0p28"
#define P49_LOG_MAX		(192 * 1024)
#define P49_MAGIC_LE		0x474F4F42	/* "BOOG" */
/* p28 holds a real ext4 filesystem, so stay well clear of its metadata. */
#define P49_BASE_SECTOR		(8 * 1024 * 1024 / 512)

struct p49_hdr {
	u32 magic;
	u32 version;
	u32 initcall_level;
	u32 log_len;
	u32 total_len;
	u64 jiffies;
} __attribute__((packed));

static char p49_buf[P49_LOG_MAX] __attribute__((aligned(4096)));
static bool p49_armed;

static int p49_timeout_ms = 30000;
module_param_named(timeout_ms, p49_timeout_ms, int, 0444);
MODULE_PARM_DESC(timeout_ms,
	"ms after arch_initcall before panicking; 0 disables the whole beacon");

static int p49_open_retries = 40;
module_param_named(open_retries, p49_open_retries, int, 0444);
MODULE_PARM_DESC(open_retries, "eMMC open retries at 200ms intervals");

static void p49_kmsg_hook(struct kmsg_dumper *d, enum kmsg_dump_reason reason)
{
	/* driven by kmsg_dump_get_buffer; nothing to do inline */
}

static size_t p49_grab_log(void)
{
	struct kmsg_dumper d;
	size_t len = 0;

	d.dump = p49_kmsg_hook;
	d.max_reason = KMSG_DUMP_PANIC;
	if (!kmsg_dump_get_buffer(&d, true, p49_buf, sizeof(p49_buf), &len))
		return 0;
	return len;
}

/* Block API notes for THIS tree, each of which differs from mainline:
 *   name_to_dev_t(name) takes ONE argument and RETURNS dev_t, 0 on failure.
 *   blkdev_get_by_dev(dev, fmode_t, holder), blkdev_put(bdev, mode) -- both in
 *   include/linux/fs.h, and blkdev_put takes the mode back.
 *   bdev_write_page(bdev, sector, page, writeback_control) writes a WHOLE page.
 * drivers/mtd/devices/block2mtd.c is the in-tree precedent for the open sequence.
 */
static int p49_write_scratch(const void *data, size_t len, sector_t start)
{
	const char *ptr = data;
	sector_t sector = start;
	struct writeback_control wbc;
	struct block_device *bdev;
	size_t remaining = len;
	dev_t devt;
	int ret;

	memset(&wbc, 0, sizeof(wbc));

	devt = name_to_dev_t(P49_SCRATCH_NAME);
	if (!devt)
		return -ENODEV;

	bdev = blkdev_get_by_dev(devt, FMODE_WRITE, NULL);
	if (IS_ERR(bdev))
		return PTR_ERR(bdev);

	while (remaining >= PAGE_SIZE) {
		struct page *pg = virt_to_page(ptr);

		ret = bdev_write_page(bdev, sector, pg, &wbc);
		if (ret) {
			blkdev_put(bdev, FMODE_WRITE);
			return ret;
		}
		ptr += PAGE_SIZE;
		remaining -= PAGE_SIZE;
		sector += PAGE_SIZE / 512;
	}

	blkdev_put(bdev, FMODE_WRITE);
	return 0;
}

/* Best-effort: the eMMC channel only exists once mmc has probed. */
static void p49_try_emmc(void)
{
	struct p49_hdr h;
	size_t log_len, total;
	int tries;

	log_len = p49_grab_log();
	if (!log_len) {
		pr_emerg("p49-bootbeacon: log buffer empty, skipping eMMC\n");
		return;
	}

	total = roundup(log_len + sizeof(h), PAGE_SIZE);
	memset(p49_buf + sizeof(h) + log_len, 0xA5, total - sizeof(h) - log_len);

	h.magic = P49_MAGIC_LE;
	h.version = 1;
	h.initcall_level = 7;
	h.log_len = (u32)log_len;
	h.total_len = (u32)total;
	h.jiffies = jiffies;

	memmove(p49_buf + sizeof(h), p49_buf, log_len);
	memcpy(p49_buf, &h, sizeof(h));

	for (tries = 0; tries < p49_open_retries; tries++) {
		if (!p49_write_scratch(p49_buf, total, P49_BASE_SECTOR)) {
			pr_emerg("p49-bootbeacon: log WRITTEN to p28 (%zu bytes)\n",
				 log_len);
			return;
		}
		mdelay(200);
	}
	pr_emerg("p49-bootbeacon: eMMC unavailable after %d tries\n",
		 p49_open_retries);
}

/*
 * The timer that turns a silent wedge into a recorded panic. If this ever fires
 * we know the kernel reached arch_initcall and then stopped making progress.
 */
static void p49_expired(unsigned long data)
{
	pr_emerg("p49-bootbeacon: TIMED OUT after %d ms -- boot did not reach "
		 "late_initcall. panicking so pstore can record the log\n",
		 p49_timeout_ms);
	panic("p49-bootbeacon: boot stalled before late_initcall");
}

static struct timer_list p49_timer;

static void p49_arm_timer(void)
{
	if (p49_timeout_ms <= 0) {
		pr_info("p49-bootbeacon: disabled (timeout_ms=%d)\n",
			p49_timeout_ms);
		return;
	}

	timer_setup(&p49_timer, p49_expired, 0);
	mod_timer(&p49_timer, jiffies + msecs_to_jiffies(p49_timeout_ms));
	p49_armed = true;

	pr_emerg("p49-bootbeacon: REACHED arch_initcall, kernel alive this far. "
		 "armed a %d ms timeout, so a stall will panic and be recorded\n",
		 p49_timeout_ms);
}
arch_initcall(p49_arm_timer);

static int __init p49_reached_late(void)
{
	if (p49_armed) {
		del_timer_sync(&p49_timer);
		p49_armed = false;
		pr_emerg("p49-bootbeacon: REACHED late_initcall, boot is progressing. "
			 "timeout cancelled\n");
	}
	p49_try_emmc();
	return 0;
}
late_initcall(p49_reached_late);
