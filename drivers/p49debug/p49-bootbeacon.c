/*
 * p49-bootbeacon - TEMPORARY bring-up instrumentation for the 4.9 port.
 *
 * This kernel has never booted and a hang is completely invisible: no serial
 * cable, CONFIG_VT is unset so there is no framebuffer console, and Samsung's
 * ECD is force-disabled by the bootloader's own bootargs (ecd_setup=disable),
 * which are applied after CONFIG_CMDLINE and so cannot be overridden from the
 * defconfig. pstore is the only channel that survives into recovery.
 *
 * The point of this module is to convert an invisible hang into a recorded
 * panic, so that kmsg_dump(KMSG_DUMP_PANIC) writes a dmesg-ramoops record.
 * That is the ONLY trigger available: in this tree kmsg_dump() is called from
 * exactly two places, panic() and the oops path. There is no
 * KMSG_DUMP_RESTART call site, so a clean restart writes nothing either.
 *
 * It is registered at core_initcall, deliberately as EARLY as possible:
 * pstore's ramoops_init() is a postcore_initcall, which runs before
 * core_initcall, so pstore is guaranteed to be live by the time this fires.
 * A previous version used late_initcall and produced no record, which is
 * consistent with the hang happening before late_initcall -- at that point the
 * beacon would never even run, so no panic and no record. Moving it early
 * brackets the hang from the other side.
 *
 * A plain mdelay() is used rather than a workqueue, because at core_initcall
 * workqueues are not reliably up yet and an ordering mistake here would look
 * exactly like "the beacon did not run".
 *
 * Delete this whole directory once the hang is found. It has no business in a
 * shipping kernel, and the panic is deliberately loud.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/kmsg_dump.h>

static int p49_beacon_delay_ms = 2000;
module_param_named(delay_ms, p49_beacon_delay_ms, int, 0444);
MODULE_PARM_DESC(delay_ms,
	"ms to wait at core_initcall before panicking; 0 disables");

/*
 * Pretend to be the panic dumper so that this record wins over anything the
 * console dumper might want to write. With CONFIG_PSTORE_CONSOLE off there is
 * no console dumper, but registering with a real max_reason means the record
 * cannot be skipped for being "not a panic".
 */
static void p49_beacon_dump(struct kmsg_dumper *d, enum kmsg_dump_reason r)
{
	pr_emerg("p49-bootbeacon: dumper invoked, reason=%d\n", r);
}

static int __init p49_beacon_init(void)
{
	if (p49_beacon_delay_ms <= 0) {
		pr_info("p49-bootbeacon: disabled (delay_ms=%d)\n",
			p49_beacon_delay_ms);
		return 0;
	}

	pr_emerg("p49-bootbeacon: REACHED core_initcall, kernel is alive this "
		 "far. panicking in %d ms to capture the log in pstore\n",
		 p49_beacon_delay_ms);

	/* Let a little of boot log accumulate so the record is not empty. */
	mdelay(p49_beacon_delay_ms);

	pr_emerg("p49-bootbeacon: about to panic, this line is the last thing "
		 "written to the pstore record\n");
	panic("p49-bootbeacon: deliberate panic at core_initcall to capture the boot log");
}
core_initcall(p49_beacon_init);
