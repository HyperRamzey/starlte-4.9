/*
 * Copyright (c) 2015-2018 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/errno.h>

#include "stui_core.h"
#include "stui_hal.h"

static int touch_requested;

/*
 * PORT-NOTE(tui): 4.4 declared these two extern and relied on the
 * touchscreen driver to define them.  On this device that is
 * drivers/input/touchscreen/zinitix/zinitix_zt75xx_ts.c:10515 and :10540
 * (the live config sets CONFIG_TOUCHSCREEN_ZINITIX_ZT75XX_TCLM=y; the other
 * four providers -- melfas_mss100, sec_incell_ts, imagis ist4050, himax --
 * are all "# ... is not set").  No zinitix driver exists in this tree yet and
 * nothing else in it defines the pair, so the externs are unresolved symbols
 * that would fail the final link and take the whole Image down with them.
 *
 * Weak definitions keep the driver linkable now and step aside later: the
 * moment a real TSP port lands with strong symbols, ld binds to those and
 * discards these.
 *
 * They return -ENODEV rather than 0 on purpose.  stui_i2c_protect()
 * propagates a non-zero return, which sends stui_process_cmd() down
 * clean_fb_prepare and reports STUI_RET_ERR_INTERNAL_ERROR.  Returning 0
 * would tell stui_process_cmd() that the touch controller had been handed to
 * the secure world when it had not, and TUI would go on to protect the
 * display and copy a framebuffer to userspace for a session that can never
 * be started.
 */
__attribute__((weak)) int stui_tsp_enter(void)
{
	pr_warn("[STUI] %s: no TSP TUI hook present (touchscreen driver not ported?)\n",
			__func__);
	return -ENODEV;
}

__attribute__((weak)) int stui_tsp_exit(void)
{
	return -ENODEV;
}

static int request_touch(void)
{
	int ret = 0;

	if (touch_requested == 1)
		return -EALREADY;

	ret = stui_tsp_enter();
	if (ret) {
		pr_err("[STUI] stui_tsp_enter failed:%d\n", ret);
		return ret;
	}

	touch_requested = 1;
	printk(KERN_DEBUG "[STUI] Touch requested\n");

	return ret;
}

static int release_touch(void)
{
	int ret = 0;

	if (touch_requested != 1)
		return -EALREADY;

	ret = stui_tsp_exit();
	if (ret) {
		pr_err("[STUI] stui_tsp_exit failed : %d\n", ret);
		return ret;
	}

	touch_requested = 0;
	printk(KERN_DEBUG "[STUI] Touch release\n");

	return ret;
}

int stui_i2c_protect(bool is_protect)
{
	int ret;

	printk(KERN_DEBUG "[STUI] %s(%s) called\n",
			__func__, is_protect ? "true" : "false");

	if (is_protect)
		ret = request_touch();
	else
		ret = release_touch();

	return ret;
}
