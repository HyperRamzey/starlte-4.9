/*
 * driver/ccic/ccic_misc.h - S2MM005 CCIC MISC driver
 *
 * Copyright (C) 2017 Samsung Electronics
 * Author: Wookwang Lee <wookwang.lee@samsung.com>
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
 * along with this program; If not, see <http://www.gnu.org/licenses/>.
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

enum uvdm_data_type {
	TYPE_SHORT = 0,
	TYPE_LONG,
};

enum uvdm_direction_type {
	DIR_OUT = 0,
	DIR_IN,
};

struct uvdm_data {
	unsigned short pid; /* Product ID */
	char type; /* uvdm_data_type */
	char dir; /* uvdm_direction_type */
	unsigned int size; /* data size */
	void __user *pData; /* data pointer */
};

#ifdef CONFIG_COMPAT
struct uvdm_data_32 {
	unsigned short pid; /* Product ID */
	char type; /* uvdm_data_type */
	char dir; /* uvdm_direction_type */
	unsigned int size; /* data size */
	compat_uptr_t pData; /* data pointer */
};
#endif

struct ccic_misc_dev {
	struct uvdm_data u_data;
#ifdef CONFIG_COMPAT
	struct uvdm_data_32 u_data_32;
#endif
	atomic_t open_excl;
	atomic_t ioctl_excl;
	int (*uvdm_write)(void *data, int size);
	int (*uvdm_read)(void *data, int size);
};

extern int ccic_misc_init(void);
extern ssize_t samsung_uvdm_out_request_message(void *data, size_t size);
extern int samsung_uvdm_in_request_message(void *data);
extern int samsung_uvdm_ready(void);
extern void samsung_uvdm_close(void);
