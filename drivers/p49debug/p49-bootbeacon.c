/*
 * p49-bootbeacon - TEMPORARY bring-up instrumentation for the 4.9 port.
 *
 * WHY THIS EXISTS
 * ---------------
 * This kernel has never booted. There is no serial cable, no framebuffer
 * console (CONFIG_VT is off and Samsung's DECON registers no console), and
 * Samsung's ECD is force-disabled by the bootloader's own bootargs
 * ("ecd_setup=disable"), which lands AFTER CONFIG_CMDLINE and therefore wins.
 * So a hang is completely invisible: the bootloader logs its own progress up to
 * "Starting kernel..." and then nothing, forever.
 *
 * pstore is the one channel that survives, because the ramoops region is
 * reserved by DT (the exynos,sec_debug_magic node) and is still present in
 * recovery. But two things have to be true for it to help:
 *
 *   1. CONFIG_PSTORE_CONSOLE must be OFF. With it on, the pmsg backend writes
 *      every printk into the SAME ramoops buffer, continuously, and leaves the
 *      panic-time dmesg record no room. That is why previous attempts produced
 *      only pmsg-ramoops-0 (written by the recovery kernel) and never a
 *      dmesg-ramoops-* record, even though the kernel clearly died.
 *   2. The kernel must PANIC rather than hang, because ramoops only writes the
 *      dmesg record from the panic notifier.
 *
 * So this module does the one thing the kernel cannot currently do on its own:
 * turn an invisible hang into a recorded panic at a chosen point.
 *
 * HOW TO USE IT
 * -------------
 * P49_BEACON_MS is a compile-time constant set from CONFIG_P49_BOOTBEACON_MS.
 * Set it to 0 to disable entirely. Move the beacon later in the boot sequence
 * to bisect where the hang is. Read the result from recovery:
 *
 *     adb pull /sys/fs/pstore/dmesg-ramoops-0
 *
 * DELETE THIS WHOLE DIRECTORY once the hang is found. It has no business in a
 * shipping kernel, and the panic is deliberately loud.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/workqueue.h>

static int p49_beacon_ms = 10000;
module_param_named(beacon_ms, p49_beacon_ms, int, 0444);
MODULE_PARM_DESC(beacon_ms,
	"milliseconds after the beacon fires before panicking; 0 disables");

static void p49_beacon_fire(struct work_struct *w);

static struct delayed_work p49_beacon_work;

static void p49_beacon_fire(struct work_struct *w)
{
	/*
	 * Deliberately unconditional. pr_emerg not printk: we want this at any
	 * loglevel, and we want it in the ramoops record even if the console
	 * is misconfigured. The panic message is the last thing written to the
	 * dmesg record, so it lands in the file we pull.
	 */
	pr_emerg("p49-bootbeacon: reached the beacon, panicking in %d ms "
		 "(kernel alive this far)\n", p49_beacon_ms);
	panic("p49-bootbeacon: deliberate panic to capture the boot log in pstore");
}

/*
 * late_initcall, not an early one. Reasoning: build 1 of this port DID program
 * DECON/DSIM (the green-artifact symptom proves it), and display initialisation
 * happens during driver_init, i.e. before late_initcall. So a beacon that
 * fires here answers the single biggest open question -- "does the kernel
 * survive all device probes?" -- in one build. If it panics and we get a
 * dmesg record, the hang is in the initramfs/userspace and not in our drivers.
 * If we get NO record, the kernel died before this point and the beacon must
 * be moved earlier, towards early_initcall.
 */
static int __init p49_beacon_init(void)
{
	if (p49_beacon_ms <= 0) {
		pr_info("p49-bootbeacon: disabled (beacon_ms=%d)\n", p49_beacon_ms);
		return 0;
	}

	pr_emerg("p49-bootbeacon: armed, will panic in %d ms if boot does not "
		 "complete\n", p49_beacon_ms);

	INIT_DELAYED_WORK(&p49_beacon_work, p49_beacon_fire);
	schedule_delayed_work(&p49_beacon_work, msecs_to_jiffies(p49_beacon_ms));
	return 0;
}
late_initcall(p49_beacon_init);
