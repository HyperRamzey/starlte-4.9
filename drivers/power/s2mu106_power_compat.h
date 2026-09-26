/*
 * s2mu106_power_compat.h - 4.4 -> 4.9 compatibility shims for the S2MU106
 *                          power / charger / fuel-gauge / haptic stack.
 *
 * WHY THIS FILE EXISTS
 * --------------------
 * The 4.4 A30s tree carries Samsung's private additions to the upstream
 * power_supply ABI directly in include/linux/power_supply.h: 19 extra
 * POWER_SUPPLY_PROP_* enumerators (VCHGIN/VWCIN/VBYP/VSYS/VBAT/VGPADC/
 * VCC1/VCC2/ICHGIN/IWCIN/IOTG/ITX, the two enable flags, the four
 * factory/test hooks and FUELGAUGE_RESET/SOH/USBPD_RESET). The 4.9.219
 * tree's power_supply.h has none of them.
 *
 * 4.9 also reserves the extended-property range explicitly:
 *     POWER_SUPPLY_EXT_PROP_MAX = POWER_SUPPLY_PROP_MAX + 256
 * and Samsung's own 4.9 drivers already use POWER_SUPPLY_EXT_PROP_* names in
 * that range (max77705_cc.c, max77865_charger.c, mfc_charger.c,
 * da9155_charger.c), so the convention is established. The missing
 * enumerators are therefore re-declared here, continuing the same extended
 * numbering the 4.4 tree used, rather than being renumbered into the core
 * range -- that would collide with upstream and change the meaning of any
 * value userspace already sees.
 *
 * NOTE FOR THE ORCHESTRATOR: this is a workaround for a gap in the shared
 * 4.9 tree. If a future wave adds the Samsung extended-property enum to
 * include/linux/power_supply.h, delete this file and its #include lines.
 *
 * factory_mode: 4.9 moved the `int factory_mode` global (and its
 * declaration in <linux/sec_batt.h>) behind CONFIG_SEC_FACTORY, which is
 * NOT set in the A30s defconfig, so the symbol does not exist at all in a
 * 4.9 build. The 4.4 tree declared it unconditionally, where it is
 * initialised to 0 and only ever written by the CONFIG_SEC_FACTORY
 * `factory_mode=` boot parameter. Therefore folding it to the constant 0
 * when the config is off is behaviour-identical and avoids a link error.
 *
 * Samsung's own 4.4 tree sets the precedent for the local-declaration
 * approach: drivers/battery_v2/s2mu005_charger.c:43 declares
 * `extern int factory_mode;` locally for exactly this reason.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __LINUX_S2MU106_POWER_COMPAT_H
#define __LINUX_S2MU106_POWER_COMPAT_H

#include <linux/power_supply.h>

/*
 * The enumerators the ported S2MU106 drivers reference. Values are explicit
 * so this file cannot silently shift if the core enum grows; they all sit
 * above POWER_SUPPLY_PROP_MAX, i.e. inside the reserved
 * POWER_SUPPLY_EXT_PROP_* window that 4.9 already declares room for
 * (POWER_SUPPLY_EXT_PROP_MAX = POWER_SUPPLY_PROP_MAX + 256, 255 slots free).
 *
 * Group A reproduces the 19 Samsung properties that 4.4 carried in the core
 * enum. Group B reproduces the 8 Samsung EXT properties this stack uses.
 *
 * IMPORTANT -- PRE-EXISTING 4.9 TREE DEFECT, not caused by this port:
 * the 4.9 tree USES POWER_SUPPLY_EXT_PROP_* in five of Samsung's own
 * drivers (max77705_cc.c, max77705_pd.c, max77865_charger.c, mfc_charger.c,
 * da9155_charger.c) but defines the extended-property enum NOWHERE. Those
 * drivers cannot compile until the enum exists. If a future wave adds the
 * real Samsung extended enum to include/linux/power_supply.h, delete this
 * file, its #include lines, and fold the names in below into it.
 */
enum s2mu106_ext_power_supply_property {
	/* group A: 4.4 core-enum Samsung properties */
	S2MU106_EXT_PROP_VCHGIN		= POWER_SUPPLY_PROP_MAX + 1,
	S2MU106_EXT_PROP_VWCIN		= POWER_SUPPLY_PROP_MAX + 2,
	S2MU106_EXT_PROP_VBYP		= POWER_SUPPLY_PROP_MAX + 3,
	S2MU106_EXT_PROP_VSYS		= POWER_SUPPLY_PROP_MAX + 4,
	S2MU106_EXT_PROP_VBAT		= POWER_SUPPLY_PROP_MAX + 5,
	S2MU106_EXT_PROP_VGPADC		= POWER_SUPPLY_PROP_MAX + 6,
	S2MU106_EXT_PROP_VCC1		= POWER_SUPPLY_PROP_MAX + 7,
	S2MU106_EXT_PROP_VCC2		= POWER_SUPPLY_PROP_MAX + 8,
	S2MU106_EXT_PROP_ICHGIN		= POWER_SUPPLY_PROP_MAX + 9,
	S2MU106_EXT_PROP_IWCIN		= POWER_SUPPLY_PROP_MAX + 10,
	S2MU106_EXT_PROP_IOTG		= POWER_SUPPLY_PROP_MAX + 11,
	S2MU106_EXT_PROP_ITX		= POWER_SUPPLY_PROP_MAX + 12,
	S2MU106_EXT_PROP_CO_ENABLE	= POWER_SUPPLY_PROP_MAX + 13,
	S2MU106_EXT_PROP_RR_ENABLE	= POWER_SUPPLY_PROP_MAX + 14,
	S2MU106_EXT_PROP_PM_FACTORY	= POWER_SUPPLY_PROP_MAX + 15,
	S2MU106_EXT_PROP_FUELGAUGE_RESET	= POWER_SUPPLY_PROP_MAX + 16,
	S2MU106_EXT_PROP_USBPD_RESET	= POWER_SUPPLY_PROP_MAX + 17,
	S2MU106_EXT_PROP_FACTORY_MODE	= POWER_SUPPLY_PROP_MAX + 18,
	S2MU106_EXT_PROP_SOH		= POWER_SUPPLY_PROP_MAX + 19,
	/* group B: Samsung extended properties used by this stack */
	S2MU106_EXT_PROP_AICL_CURRENT	= POWER_SUPPLY_PROP_MAX + 20,
	S2MU106_EXT_PROP_CURRENT_MEASURE	= POWER_SUPPLY_PROP_MAX + 21,
	S2MU106_EXT_PROP_FACTORY_VOLTAGE_REGULATION = POWER_SUPPLY_PROP_MAX + 22,
	S2MU106_EXT_PROP_FUELGAUGE_FACTORY	= POWER_SUPPLY_PROP_MAX + 23,
	S2MU106_EXT_PROP_INBAT_VOLTAGE_FGSRC_SWITCHING = POWER_SUPPLY_PROP_MAX + 24,
	S2MU106_EXT_PROP_TTF_FULL_CAPACITY	= POWER_SUPPLY_PROP_MAX + 25,
	S2MU106_EXT_PROP_UPDATE_BATTERY_DATA	= POWER_SUPPLY_PROP_MAX + 26,
	S2MU106_EXT_PROP_WIRELESS_TXMODE_DISCON	= POWER_SUPPLY_PROP_MAX + 27,
};

/* The ported sources use the 4.4 spelling, so alias rather than edit them. */
#define POWER_SUPPLY_PROP_VCHGIN		S2MU106_EXT_PROP_VCHGIN
#define POWER_SUPPLY_PROP_VWCIN		S2MU106_EXT_PROP_VWCIN
#define POWER_SUPPLY_PROP_VBYP		S2MU106_EXT_PROP_VBYP
#define POWER_SUPPLY_PROP_VSYS		S2MU106_EXT_PROP_VSYS
#define POWER_SUPPLY_PROP_VBAT		S2MU106_EXT_PROP_VBAT
#define POWER_SUPPLY_PROP_VGPADC		S2MU106_EXT_PROP_VGPADC
#define POWER_SUPPLY_PROP_VCC1		S2MU106_EXT_PROP_VCC1
#define POWER_SUPPLY_PROP_VCC2		S2MU106_EXT_PROP_VCC2
#define POWER_SUPPLY_PROP_ICHGIN		S2MU106_EXT_PROP_ICHGIN
#define POWER_SUPPLY_PROP_IWCIN		S2MU106_EXT_PROP_IWCIN
#define POWER_SUPPLY_PROP_IOTG		S2MU106_EXT_PROP_IOTG
#define POWER_SUPPLY_PROP_ITX		S2MU106_EXT_PROP_ITX
#define POWER_SUPPLY_PROP_CO_ENABLE	S2MU106_EXT_PROP_CO_ENABLE
#define POWER_SUPPLY_PROP_RR_ENABLE	S2MU106_EXT_PROP_RR_ENABLE
#define POWER_SUPPLY_PROP_PM_FACTORY	S2MU106_EXT_PROP_PM_FACTORY
#define POWER_SUPPLY_PROP_FUELGAUGE_RESET	S2MU106_EXT_PROP_FUELGAUGE_RESET
#define POWER_SUPPLY_PROP_USBPD_RESET	S2MU106_EXT_PROP_USBPD_RESET
#define POWER_SUPPLY_PROP_FACTORY_MODE	S2MU106_EXT_PROP_FACTORY_MODE
#define POWER_SUPPLY_PROP_SOH		S2MU106_EXT_PROP_SOH

/* group B keeps the 4.4 spelling the ported sources use. */
#define POWER_SUPPLY_EXT_PROP_AICL_CURRENT		S2MU106_EXT_PROP_AICL_CURRENT
#define POWER_SUPPLY_EXT_PROP_CURRENT_MEASURE	S2MU106_EXT_PROP_CURRENT_MEASURE
#define POWER_SUPPLY_EXT_PROP_FACTORY_VOLTAGE_REGULATION \
	S2MU106_EXT_PROP_FACTORY_VOLTAGE_REGULATION
#define POWER_SUPPLY_EXT_PROP_FUELGAUGE_FACTORY	S2MU106_EXT_PROP_FUELGAUGE_FACTORY
#define POWER_SUPPLY_EXT_PROP_INBAT_VOLTAGE_FGSRC_SWITCHING \
	S2MU106_EXT_PROP_INBAT_VOLTAGE_FGSRC_SWITCHING
#define POWER_SUPPLY_EXT_PROP_TTF_FULL_CAPACITY	S2MU106_EXT_PROP_TTF_FULL_CAPACITY
#define POWER_SUPPLY_EXT_PROP_UPDATE_BATTERY_DATA	S2MU106_EXT_PROP_UPDATE_BATTERY_DATA
#define POWER_SUPPLY_EXT_PROP_WIRELESS_TXMODE_DISCON \
	S2MU106_EXT_PROP_WIRELESS_TXMODE_DISCON

/*
 * 4.9 gates the factory_mode global behind CONFIG_SEC_FACTORY. With the
 * config off it is permanently 0 in the 4.4 tree too, so substitute a
 * constant. `extern` (not a static local) is used when the config is on,
 * matching <linux/sec_batt.h>.
 */
#if defined(CONFIG_SEC_FACTORY)
extern int factory_mode;
#else
#define factory_mode 0
#endif

#endif /* __LINUX_S2MU106_POWER_COMPAT_H */
