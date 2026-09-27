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
 * The canonical Samsung charging/extended-property header. It MUST be
 * the battery_v2 copy: include/linux/battery/sec_charging_common.h is a
 * byte-identical, same-include-guard, but STALE upstream duplicate that
 * predates enum power_supply_ext_property, enum sec_battery_cable and
 * enum sec_battery_inbat_fgsrc_switching. Including it yields a silently
 * different set of declarations, so the include path is the defect.
 */
#include "../battery_v2/include/sec_charging_common.h"

/*
 * The 4.4 tree carried 19 extra POWER_SUPPLY_PROP_* enumerators (VCHGIN ..
 * SOH) directly in enum power_supply_property; 4.9.219 has none of them.
 * They are declared once, as POWER_SUPPLY_EXT_PROP_*, at the end of
 * enum power_supply_ext_property in ../battery_v2/include/sec_charging_common.h,
 * in a private window that starts ABOVE POWER_SUPPLY_EXT_PROP_MAX so it can
 * neither collide with the shared 85..114 names nor fall inside the
 * `case POWER_SUPPLY_PROP_MAX ... POWER_SUPPLY_EXT_PROP_MAX:` ranges that
 * every in-tree Samsung driver dispatches with.
 *
 * What is left here is only the SPELLING bridge: the ported sources use the
 * 4.4 names POWER_SUPPLY_PROP_VCHGIN etc., so alias them rather than edit
 * every call site. No POWER_SUPPLY_EXT_PROP_* name is defined here -- those
 * are real enumerators, and a macro of the same name would be a
 * self-referential define that also makes `#ifdef` lie.
 *
 * Dispatch a private property with both ranges:
 *   case POWER_SUPPLY_PROP_MAX ... POWER_SUPPLY_EXT_PROP_MAX:
 *   case POWER_SUPPLY_EXT_PROP_S2MU106_BASE ... POWER_SUPPLY_EXT_PROP_S2MU106_MAX:
 *
 * If a future wave folds the Samsung extended-property enum into
 * include/linux/power_supply.h proper, delete this file and its #include
 * lines: the POWER_SUPPLY_PROP_* spellings would then be plain enumerators
 * and these aliases would become unnecessary.
 */
/* The ported sources use the 4.4 spelling, so alias rather than edit them. */
#define POWER_SUPPLY_PROP_VCHGIN		POWER_SUPPLY_EXT_PROP_VCHGIN
#define POWER_SUPPLY_PROP_VWCIN		POWER_SUPPLY_EXT_PROP_VWCIN
#define POWER_SUPPLY_PROP_VBYP		POWER_SUPPLY_EXT_PROP_VBYP
#define POWER_SUPPLY_PROP_VSYS		POWER_SUPPLY_EXT_PROP_VSYS
#define POWER_SUPPLY_PROP_VBAT		POWER_SUPPLY_EXT_PROP_VBAT
#define POWER_SUPPLY_PROP_VGPADC		POWER_SUPPLY_EXT_PROP_VGPADC
#define POWER_SUPPLY_PROP_VCC1		POWER_SUPPLY_EXT_PROP_VCC1
#define POWER_SUPPLY_PROP_VCC2		POWER_SUPPLY_EXT_PROP_VCC2
#define POWER_SUPPLY_PROP_ICHGIN		POWER_SUPPLY_EXT_PROP_ICHGIN
#define POWER_SUPPLY_PROP_IWCIN		POWER_SUPPLY_EXT_PROP_IWCIN
#define POWER_SUPPLY_PROP_IOTG		POWER_SUPPLY_EXT_PROP_IOTG
#define POWER_SUPPLY_PROP_ITX		POWER_SUPPLY_EXT_PROP_ITX
#define POWER_SUPPLY_PROP_CO_ENABLE	POWER_SUPPLY_EXT_PROP_CO_ENABLE
#define POWER_SUPPLY_PROP_RR_ENABLE	POWER_SUPPLY_EXT_PROP_RR_ENABLE
#define POWER_SUPPLY_PROP_PM_FACTORY	POWER_SUPPLY_EXT_PROP_PM_FACTORY
#define POWER_SUPPLY_PROP_FUELGAUGE_RESET	POWER_SUPPLY_EXT_PROP_FUELGAUGE_RESET
#define POWER_SUPPLY_PROP_USBPD_RESET	POWER_SUPPLY_EXT_PROP_USBPD_RESET
#define POWER_SUPPLY_PROP_FACTORY_MODE	POWER_SUPPLY_EXT_PROP_FACTORY_MODE
#define POWER_SUPPLY_PROP_SOH		POWER_SUPPLY_EXT_PROP_SOH

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
