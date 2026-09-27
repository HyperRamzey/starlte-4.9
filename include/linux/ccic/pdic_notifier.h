/*
 * include/linux/muic/muic_notifier.h
 *
 * header file supporting MUIC notifier call chain information
 *
 * Copyright (C) 2010 Samsung Electronics
 * Seung-Jin Hahn <sjin.hahn@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */
/*
 * DERP PORT NOTE (4.4 exynos7885 -> 4.9.219 starlte port)
 *
 * Brought across verbatim from the 4.4 Exynos7885 (Galaxy A30s) tree, which
 * still carries the complete, non-closed-source Samsung S2MU106 CCIC/MUIC
 * framework. The 4.9 base (Samsung Exynos9810 / Galaxy S9) forked before
 * that framework existed and instead ships the max77705-era CCIC and the
 * drivers/muic/universal/ MUIC stack, so nothing here had a 4.9 donor.
 *
 * Only the include paths were retargeted onto the 4.9 tree layout; no logic
 * was altered. See the port report for the dependency closure and for the
 * framework .c files (usbpd*.c, muic_manager.c, muic_core.c) that are still
 * required at link time and are NOT yet in the tree.
 */
#include <linux/muic/muic_notifier.h>
#include <linux/muic/muic.h>
#ifndef __PDIC_NOTIFIER_H__
#define __PDIC_NOTIFIER_H__

struct s2m_pdic_notifier_struct {
	muic_attached_dev_t attached_dev;
	muic_notifier_cmd_t cmd;
	struct blocking_notifier_head notifier_call_chain;
};

enum {
	S2M_PDIC_NOTIFY_DEV_MUIC,
	S2M_PDIC_NOTIFY_DEV_USB,
	S2M_PDIC_NOTIFY_DEV_CHARGER,
};

#define MUIC_NOTIFIER_BLOCK(name)	\
	struct notifier_block (name)

/* muic notifier init/notify function
 * this function is for JUST MUIC device driver.
 * DON'T use function anywhrer else!!
 */
extern void s2m_pdic_notifier_attach_attached_dev(muic_attached_dev_t new_dev);
extern void s2m_pdic_notifier_detach_attached_dev(muic_attached_dev_t cur_dev);
extern void s2m_pdic_notifier_logically_attach_attached_dev(muic_attached_dev_t new_dev);
extern void s2m_pdic_notifier_logically_detach_attached_dev(muic_attached_dev_t cur_dev);
void s2m_pdic_notifier_attach_attached_jig_dev(muic_attached_dev_t new_dev);

/* muic notifier register/unregister API
 * for used any where want to receive muic attached device attach/detach. */
extern int s2m_pdic_notifier_register(struct notifier_block *nb,
		notifier_fn_t notifier, muic_notifier_device_t listener);
extern int s2m_pdic_notifier_unregister(struct notifier_block *nb);

#endif
