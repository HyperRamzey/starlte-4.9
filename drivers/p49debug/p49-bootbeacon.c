/*
 * p49-bootbeacon - TEMPORARY bring-up instrumentation for the 4.9 port.
 *
 * WHY THIS WRITES TO A PARTITION AND NOT TO PSTORE
 * ------------------------------------------------
 * pstore/ramoops lives in DRAM, and DRAM does not survive what this phone
 * actually does to it. The 4.9 kernel hangs, the only way back to a readable
 * state is a hard reset, and a hard reset re-initialises DRAM -- which wipes
 * the very reserved region pstore was writing into. So the panic record is
 * destroyed by the same reset that brings recovery up, and pstore on this board
 * may simply be unusable as a channel no matter how it is configured. (It is
 * configured as closely to stock as possible in fs/pstore/ram.c: 0x915A0000/32K
 * with record=pmsg=0x4000, the exact values read out of the running stock
 * recovery kernel. That fix stands on its own merits, but it cannot survive a
 * cold reset.)
 *
 * eMMC survives both warm and cold resets, so the log is written to the `cache`
 * partition (p28, 200 MB of genuine scratch). Nothing recovery does can touch
 * it. Pull it with:
 *
 *     adb -s RF8M834JT4D shell 'dd if=/dev/block/mmcblk0p28 bs=1M count=1' > p49log.bin
 *
 * WHY A BEACON AT ALL
 * ------------------
 * This kernel has never booted and a hang is otherwise completely invisible: no
 * serial cable, CONFIG_VT is unset so there is no framebuffer console, and
 * Samsung's ECD is force-disabled by the bootloader's own bootargs
 * (ecd_setup=disable), which are applied after CONFIG_CMDLINE and so cannot be
 * overridden from the defconfig. The phone's USB-C port is not a UART bridge.
 *
 * The beacon is registered at device_initcall, which is level 6 in THIS tree
 * (core_initcall is 1, postcore_initcall is 2, device_initcall is 6,
 * late_initcall is 7). Level 6 was chosen for two reasons that were measured
 * rather than assumed:
 *
 *   - it is after ramoops_init (level 2), so a panic can still be recorded there;
 *   - it is before late_initcall (level 7), which is where the previous version
 *     of this beacon lived and where it produced no record at all, consistent
 *     with the hang happening before level 7.
 *
 * drivers/mmc links before drivers/p49debug in drivers/Makefile, so within
 * level 6 the mmc block driver has already registered. eMMC card detection is
 * nevertheless asynchronous, hence the retry loop rather than a single open.
 *
 * Delete this whole directory once the hang is found. It has no business in a
 * shipping kernel, and the panic at the end is deliberate.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/mm.h>
#include <linux/kmsg_dump.h>
#include <linux/writeback.h>

#define P49_SCRATCH_NAME	"mmcblk0p28"
#define P49_LOG_MAX		(384 * 1024)
#define P49_MAGIC		"P49BOOTLOG"

/* Set to 0 to reach device_initcall and then do nothing, as a liveness probe. */
static int p49_beacon_delay_ms = 2000;
module_param_named(delay_ms, p49_beacon_delay_ms, int, 0444);
MODULE_PARM_DESC(delay_ms,
	"ms to wait at device_initcall before dumping and panicking; 0 disables");

static int p49_open_retries = 40;
module_param_named(open_retries, p49_open_retries, int, 0444);
MODULE_PARM_DESC(open_retries,
	"how many 200ms intervals to keep retrying the eMMC open before giving up");

/*
 * Scratch header so a blank or stale partition is never mistaken for a fresh
 * log. 32 bytes, and the tail is padded with a byte pattern that the log text
 * can never contain, so a short write is still obvious.
 */
struct p49_hdr {
	u32 magic;		/* 0x474F4F42 "BOOG" little-endian check below */
	u32 version;
	u32 log_len;
	u32 hdr_crc;
	u32 pad[4];
} __attribute__((packed));

#define P49_MAGIC_LE	0x474F4F42

static char p49_buf[P49_LOG_MAX] __attribute__((aligned(4096)));

/*
 * Pull the kernel log out of the ring buffer into p49_buf.
 *
 * This works well here for a reason worth knowing: with CONFIG_VT off and ECD
 * disabled there is NO console registered at all, so nothing ever flushes or
 * reclaims log_buf and the boot log is still sitting in it at level 6. That is
 * the one piece of good luck in this whole mess.
 */
static void p49_kmsg_hook(struct kmsg_dumper *d, enum kmsg_dump_reason reason)
{
	/* Nothing to do inline; kmsg_dump_get_buffer drives the real copy. */
}

static size_t p49_grab_log(void)
{
	struct kmsg_dumper d;
	size_t len = 0;

	d.dump = p49_kmsg_hook;
	d.max_reason = KMSG_DUMP_PANIC;

	if (!kmsg_dump_get_buffer(&d, true, p49_buf, sizeof(p49_buf), &len)) {
		pr_emerg("p49-bootbeacon: kmsg_dump_get_buffer returned nothing\n");
		return 0;
	}
	return len;
}

/*
 * Write one buffer to the head of the scratch partition.
 * Returns 0 on success, negative errno otherwise.
 *
 * This tree's block API is not the mainline one, and getting it wrong is easy:
 *   - name_to_dev_t(name) takes ONE argument and RETURNS dev_t, 0 on failure.
 *     (Not the two-argument out-parameter form of newer kernels.)
 *   - blkdev_get_by_dev(dev, fmode_t mode, void *holder) and
 *     blkdev_put(bdev, mode) both live in include/linux/fs.h, and blkdev_put
 *     takes the mode back.
 *   - bdev_write_page() is the clean single-page writer, but here it takes a
 *     struct writeback_control* and writes a WHOLE page -- there is no
 *     offset/size pair. So the buffer is padded up to a page multiple and the
 *     loop just walks one page at a time. p28 is 200 MB, so the padding is free.
 * drivers/mtd/devices/block2mtd.c is the in-tree precedent for the
 * name_to_dev_t + blkdev_get_by_dev sequence.
 */
static int p49_write_scratch(const void *data, size_t len)
{
	const char *ptr = data;
	sector_t sector = 0;
	struct writeback_control wbc;
	struct block_device *bdev;
	size_t remaining = len;
	dev_t devt;
	int ret;

	memset(&wbc, 0, sizeof(wbc));

	devt = name_to_dev_t(P49_SCRATCH_NAME);
	if (!devt) {
		pr_emerg("p49-bootbeacon: name_to_dev_t(%s) -> 0, no such device\n",
			 P49_SCRATCH_NAME);
		return -ENODEV;
	}

	bdev = blkdev_get_by_dev(devt, FMODE_WRITE, NULL);
	if (IS_ERR(bdev)) {
		pr_emerg("p49-bootbeacon: blkdev_get_by_dev(%s) err %ld\n",
			 P49_SCRATCH_NAME, PTR_ERR(bdev));
		return PTR_ERR(bdev);
	}

	while (remaining >= PAGE_SIZE) {
		struct page *pg = virt_to_page(ptr);

		ret = bdev_write_page(bdev, sector, pg, &wbc);
		if (ret) {
			pr_emerg("p49-bootbeacon: bdev_write_page err %d "
				 "at sector %llu\n", ret,
				 (unsigned long long)sector);
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

/* Header + log, written as one buffer so a reader can never see a torn pair. */
static void p49_emit(const char *hdrline, size_t log_len)
{
	struct p49_hdr h;
	size_t total;
	int ret, tries;

	memset(&h, 0, sizeof(h));
	h.magic = P49_MAGIC_LE;
	h.version = 1;
	h.log_len = (u32)log_len;
	h.hdr_crc = 0;

	/* log first, then the 32-byte header, into one contiguous write */
	memmove(p49_buf + sizeof(h), p49_buf, log_len);
	memcpy(p49_buf, &h, sizeof(h));

	/*
	 * bdev_write_page() can only write whole pages, so pad up to a page
	 * multiple and mark the pad with a byte the log text cannot contain, so a
	 * short write is still obvious to whoever reads this back.
	 */
	total = roundup(sizeof(h) + log_len, PAGE_SIZE);
	memset(p49_buf + sizeof(h) + log_len, 0xA5, total - sizeof(h) - log_len);

	pr_emerg("p49-bootbeacon: %s, %zu bytes of log -> %s\n",
		 hdrline, log_len, P49_SCRATCH_NAME);

	/*
	 * eMMC card detection is asynchronous and this runs at level 6, so the
	 * block device may not be there yet. Retry rather than lose the log to a
	 * race.
	 */
	for (tries = 0; tries < p49_open_retries; tries++) {
		ret = p49_write_scratch(p49_buf, total);
		if (!ret) {
			pr_emerg("p49-bootbeacon: log WRITTEN to p28\n");
			return;
		}
		mdelay(200);
	}
	pr_emerg("p49-bootbeacon: could NOT write p28 after %d tries\n",
		 p49_open_retries);
}

static int __init p49_beacon_init(void)
{
	size_t log_len;

	if (p49_beacon_delay_ms <= 0) {
		pr_info("p49-bootbeacon: disabled (delay_ms=%d)\n",
			p49_beacon_delay_ms);
		return 0;
	}

	pr_emerg("p49-bootbeacon: REACHED device_initcall, kernel is alive this "
		 "far. waiting %d ms\n", p49_beacon_delay_ms);
	mdelay(p49_beacon_delay_ms);

	log_len = p49_grab_log();
	if (!log_len) {
		/* nothing in the ring buffer: still prove liveness, still panic */
		log_len = 1;
		p49_buf[0] = '\n';
	}

	p49_emit("capturing boot log", log_len);

	/*
	 * Panic last. kmsg_dump(KMSG_DUMP_PANIC) may also leave a pstore record,
	 * which is a bonus, not the point -- eMMC is the reliable channel.
	 * panic() is not declared noreturn in this tree, hence the return.
	 */
	pr_emerg("p49-bootbeacon: about to panic, eMMC log is already saved\n");
	panic("p49-bootbeacon: deliberate panic at device_initcall after saving the boot log to eMMC");

	return 0;
}
device_initcall(p49_beacon_init);
