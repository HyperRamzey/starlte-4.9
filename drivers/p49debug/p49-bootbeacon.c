/*
 * p49-bootbeacon - TEMPORARY bring-up instrumentation for the 4.9 port.
 *
 * WHAT IT DOES
 * ------------
 * Two snapshots of the kernel log to the `cache` partition (p28), at the end of
 * device_initcall and again at the end of late_initcall, and then it gets out of
 * the way. It does NOT panic. That is deliberate: the point of this build is to
 * find out whether the phone boots with TZDEV and TUI switched off, and a panic
 * would destroy that answer.
 *
 * WHY eMMC AND NOT PSTORE
 * -----------------------
 * pstore/ramoops lives in DRAM, and DRAM does not survive what this phone
 * actually does to it. The kernel hangs, the only way back to a readable state
 * is a hard reset, and a hard reset re-initialises DRAM, wiping the reserved
 * region pstore was writing into. The record is destroyed by the same reset that
 * brings recovery up. (fs/pstore/ram.c is still configured as closely to stock as
 * possible -- 0x915A0000/32K with record=pmsg=0x4000, the exact values read out
 * of the running stock recovery kernel -- but it cannot survive a cold reset, so
 * it is a bonus channel, not the mechanism.)
 *
 * WHY INITCALL LEVELS IN THIS TREE
 * -------------------------------
 * core_initcall=1, postcore_initcall=2, arch_initcall=3, subsys_initcall=4,
 * fs_initcall=5, device_initcall=6, late_initcall=7. Measured, not assumed -- an
 * earlier version of this file used core_initcall on the belief that it ran
 * after postcore_initcall, and it does not.
 *
 * WHY p49debug SITS LAST IN drivers/Makefile
 * -------------------------------------------
 * Within a level, initcalls run in link order, so being last means the snapshot
 * contains everything the other drivers printed. The cost is that a driver that
 * wedges before we run means no snapshot at all. That trade is deliberate: this
 * build has TZDEV and TUI disabled precisely so that the level-6 suspects are
 * only touch (input/), i2c and media/display, and those are the ones we want to
 * see in the log.
 *
 * Reading it back:
 *   adb shell 'dd if=/dev/block/mmcblk0p28 bs=1M count=9 skip=8' > p49log.bin
 *   # or pull the whole partition: it is 200 MB
 *
 * Delete this whole directory once the hang is found. It has no business in a
 * shipping kernel.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/blkdev.h>
#include <linux/mm.h>
#include <linux/kmsg_dump.h>
#include <linux/writeback.h>

#define P49_SCRATCH_NAME	"mmcblk0p28"
#define P49_LOG_MAX		(192 * 1024)
#define P49_MAGIC_LE		0x474F4F42	/* "BOOG" */
/*
 * Start at 8 MiB. p28 turned out to hold a real ext4 filesystem (superblock
 * magic 53 ef at 0x438), so writing at sector 0 would stomp its metadata. 8 MiB
 * is far past anything an empty ext4 uses, and the partition is 200 MB.
 */
#define P49_BASE_SECTOR		(8 * 1024 * 1024 / 512)
#define P49_SNAPSHOT_STRIDE	(256 * 1024 / 512)

struct p49_hdr {
	u32 magic;
	u32 version;
	u32 snapshot;
	u32 initcall_level;
	u32 log_len;
	u32 total_len;
	u64 jiffies;
	u32 reserved[2];
} __attribute__((packed));

static char p49_buf[P49_LOG_MAX] __attribute__((aligned(4096)));

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

/*
 * Block API notes for THIS tree, all of which differ from mainline and all of
 * which cost me a build each:
 *   name_to_dev_t(name) takes ONE argument and RETURNS dev_t, 0 on failure.
 *   blkdev_get_by_dev(dev, fmode_t, holder) and blkdev_put(bdev, mode) are in
 *   include/linux/fs.h, and blkdev_put takes the mode back.
 *   bdev_write_page() takes (bdev, sector, page, writeback_control) and writes a
 *   WHOLE page -- there is no offset/size pair, so the buffer is page-padded.
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
	if (!devt) {
		pr_emerg("p49-bootbeacon: name_to_dev_t(%s) -> 0\n",
			 P49_SCRATCH_NAME);
		return -ENODEV;
	}

	bdev = blkdev_get_by_dev(devt, FMODE_WRITE, NULL);
	if (IS_ERR(bdev)) {
		pr_emerg("p49-bootbeacon: blkdev_get_by_dev err %ld\n",
			 PTR_ERR(bdev));
		return PTR_ERR(bdev);
	}

	while (remaining >= PAGE_SIZE) {
		struct page *pg = virt_to_page(ptr);

		ret = bdev_write_page(bdev, sector, pg, &wbc);
		if (ret) {
			pr_emerg("p49-bootbeacon: bdev_write_page err %d at "
				 "sector %llu\n", ret, (unsigned long long)sector);
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

static void p49_snapshot(int which, int level, int retries)
{
	struct p49_hdr h;
	size_t log_len, total;
	int tries;

	log_len = p49_grab_log();
	if (!log_len) {
		pr_emerg("p49-bootbeacon: log buffer empty at snapshot %d\n",
			 which);
		return;
	}

	/* bdev_write_page() writes whole pages, so pad up to a page multiple. */
	total = roundup(log_len + sizeof(h), PAGE_SIZE);
	memset(p49_buf + sizeof(h) + log_len, 0xA5, total - sizeof(h) - log_len);

	h.magic = P49_MAGIC_LE;
	h.version = 1;
	h.snapshot = which;
	h.initcall_level = level;
	h.log_len = (u32)log_len;
	h.total_len = (u32)total;
	h.jiffies = jiffies;

	/*
	 * Header and log in one write, and the header first, so a reader can
	 * validate the magic before trusting the length in it and never sees a
	 * torn pair.
	 */
	memmove(p49_buf + sizeof(h), p49_buf, log_len);
	memcpy(p49_buf, &h, sizeof(h));

	pr_emerg("p49-bootbeacon: snapshot %d (level %d): %zu bytes of log, jiffies %llu\n",
		 which, level, h.log_len, (unsigned long long)h.jiffies);

	/*
	 * eMMC card detection is asynchronous, so a single open can lose the log to
	 * a race. Retry briefly rather than throw it away.
	 */
	for (tries = 0; tries < retries; tries++) {
		if (!p49_write_scratch(p49_buf, total,
				       P49_BASE_SECTOR + (sector_t)which * P49_SNAPSHOT_STRIDE)) {
			pr_emerg("p49-bootbeacon: snapshot %d WRITTEN to p28\n", which);
			return;
		}
		mdelay(200);
	}
	pr_emerg("p49-bootbeacon: snapshot %d could NOT be written after %d tries\n",
		 which, retries);
}

static int p49_open_retries = 40;
module_param_named(open_retries, p49_open_retries, int, 0444);
MODULE_PARM_DESC(open_retries, "eMMC open retries at 200ms intervals");

static int __init p49_beacon_dev(void)
{
	pr_emerg("p49-bootbeacon: REACHED end of device_initcall, kernel alive\n");
	p49_snapshot(0, 6, p49_open_retries);
	return 0;
}
device_initcall(p49_beacon_dev);

static int __init p49_beacon_late(void)
{
	pr_emerg("p49-bootbeacon: REACHED end of late_initcall, kernel alive\n");
	p49_snapshot(1, 7, p49_open_retries);
	return 0;
}
late_initcall(p49_beacon_late);
