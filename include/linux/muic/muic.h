/*
 * include/linux/muic/muic.h
 *
 * header file supporting MUIC common information
 *
 * Copyright (C) 2010 Samsung Electronics
 * Seoyoung Jeong <seo0.jeong@samsung.com>
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

#ifndef __MUIC_H__
#define __MUIC_H__

#include <linux/mutex.h>

/* Only ever used through a pointer in this header and in
 * struct muic_platform_data; declared here so muic.h does not have to pull in
 * linux/device.h. */
struct device;

/* Status of IF PMIC chip (suspend and resume) */
enum {
	MUIC_SUSPEND		= 0,
	MUIC_RESUME,
};

/* MUIC Interrupt */
enum {
	MUIC_INTR_DETACH	= 0,
	MUIC_INTR_ATTACH
};

enum muic_op_mode {
	OPMODE_MUIC = 0<<0,
	OPMODE_CCIC = 1<<0,
};

/* MUIC Dock Observer Callback parameter */
enum {
	MUIC_DOCK_DETACHED	= 0,
	MUIC_DOCK_DESKDOCK	= 1,
	MUIC_DOCK_CARDOCK	= 2,
	MUIC_DOCK_AUDIODOCK	= 101,
	MUIC_DOCK_SMARTDOCK	= 102,
	MUIC_DOCK_HMT		= 105,
	MUIC_DOCK_ABNORMAL	= 106,
	MUIC_DOCK_GAMEPAD	= 107,
	MUIC_DOCK_GAMEPAD_WITH_EARJACK	= 108,
};

/* MUIC Path */
enum {
	MUIC_PATH_USB_AP	= 0,
	MUIC_PATH_USB_CP,
	MUIC_PATH_UART_AP,
	MUIC_PATH_UART_CP,
	MUIC_PATH_OPEN,
	MUIC_PATH_AUDIO,
};

#ifdef CONFIG_MUIC_HV_FORCE_LIMIT
enum {
	HV_9V = 0,
	HV_5V,
};
#endif

/* bootparam SWITCH_SEL */
enum {
	SWITCH_SEL_USB_MASK	= 0x1,
	SWITCH_SEL_UART_MASK	= 0x2,
	SWITCH_SEL_RUSTPROOF_MASK	= 0x8,
	SWITCH_SEL_AFC_DISABLE_MASK	= 0x100,
};

/* bootparam CHARGING_MODE */
enum {
	CH_MODE_AFC_DISABLE_VAL = 0x31, /* char '1' */
};

/* MUIC ADC table */
typedef enum {
	ADC_GND			= 0x00,
	ADC_SEND_END		= 0x01, /* 0x00001 2K ohm */
	ADC_REMOTE_S11		= 0x0c, /* 0x01100 20.5K ohm */
	ADC_REMOTE_S12		= 0x0d, /* 0x01101 24.07K ohm */
	ADC_RESERVED_VZW	= 0x0e, /* 0x01110 28.7K ohm */
	ADC_INCOMPATIBLE_VZW	= 0x0f, /* 0x01111 34K ohm */
	ADC_SMARTDOCK		= 0x10, /* 0x10000 40.2K ohm */
	ADC_RDU_TA		= 0x10, /* 0x10000 40.2K ohm */
	ADC_HMT			= 0x11, /* 0x10001 49.9K ohm */
	ADC_AUDIODOCK		= 0x12, /* 0x10010 64.9K ohm */
	ADC_USB_LANHUB		= 0x13, /* 0x10011 80.07K ohm */
	ADC_CHARGING_CABLE	= 0x14,	/* 0x10100 102K ohm */
	ADC_UNIVERSAL_MMDOCK	= 0x15, /* 0x10101 121K ohm */
	ADC_GAMEPAD		= 0x15, /* 0x10101 121K ohm */
	ADC_UART_CABLE		= 0x16, /* 0x10110 150K ohm */
	ADC_CEA936ATYPE1_CHG	= 0x17,	/* 0x10111 200K ohm */
	ADC_JIG_USB_OFF		= 0x18, /* 0x11000 255K ohm */
	ADC_JIG_USB_ON		= 0x19, /* 0x11001 301K ohm */
	ADC_DESKDOCK		= 0x1a, /* 0x11010 365K ohm */
	ADC_CEA936ATYPE2_CHG	= 0x1b, /* 0x11011 442K ohm */
	ADC_JIG_UART_OFF	= 0x1c, /* 0x11100 523K ohm */
	ADC_JIG_UART_ON		= 0x1d, /* 0x11101 619K ohm */
	ADC_AUDIOMODE_W_REMOTE	= 0x1e, /* 0x11110 1000K ohm */
	ADC_OPEN		= 0x1f,
	ADC_OPEN_219		= 0xfb, /* ADC open or 219.3K ohm */
	ADC_219			= 0xfc, /* ADC open or 219.3K ohm */

	ADC_UNDEFINED		= 0xfd, /* Undefied range */
	ADC_DONTCARE		= 0xfe, /* ADC don't care for MHL */
	ADC_ERROR		= 0xff, /* ADC value read error */
} muic_adc_t;

/* MUIC attached device type */
typedef enum {
	ATTACHED_DEV_NONE_MUIC = 0,

	ATTACHED_DEV_USB_MUIC,
	ATTACHED_DEV_CDP_MUIC,
	ATTACHED_DEV_OTG_MUIC,
	ATTACHED_DEV_TA_MUIC,
	ATTACHED_DEV_UNOFFICIAL_MUIC,
	ATTACHED_DEV_UNOFFICIAL_TA_MUIC,
	ATTACHED_DEV_UNOFFICIAL_ID_MUIC,
	ATTACHED_DEV_UNOFFICIAL_ID_TA_MUIC,
	ATTACHED_DEV_UNOFFICIAL_ID_ANY_MUIC,
	ATTACHED_DEV_UNOFFICIAL_ID_USB_MUIC,

	ATTACHED_DEV_UNOFFICIAL_ID_CDP_MUIC,
	ATTACHED_DEV_UNDEFINED_CHARGING_MUIC,
	ATTACHED_DEV_DESKDOCK_MUIC,
	ATTACHED_DEV_UNKNOWN_VB_MUIC,
	ATTACHED_DEV_DESKDOCK_VB_MUIC,
	ATTACHED_DEV_CARDOCK_MUIC,
	ATTACHED_DEV_JIG_UART_OFF_MUIC,
	ATTACHED_DEV_JIG_UART_OFF_VB_MUIC,	/* VBUS enabled */
	ATTACHED_DEV_JIG_UART_OFF_VB_OTG_MUIC,	/* for otg test */
	ATTACHED_DEV_JIG_UART_OFF_VB_FG_MUIC,	/* for fuelgauge test */

	ATTACHED_DEV_JIG_UART_ON_MUIC,
	ATTACHED_DEV_JIG_UART_ON_VB_MUIC,	/* VBUS enabled */
	ATTACHED_DEV_JIG_USB_OFF_MUIC,
	ATTACHED_DEV_JIG_USB_ON_MUIC,
	ATTACHED_DEV_SMARTDOCK_MUIC,
	ATTACHED_DEV_SMARTDOCK_VB_MUIC,
	ATTACHED_DEV_SMARTDOCK_TA_MUIC,
	ATTACHED_DEV_SMARTDOCK_USB_MUIC,
	ATTACHED_DEV_UNIVERSAL_MMDOCK_MUIC,
	ATTACHED_DEV_AUDIODOCK_MUIC,

	ATTACHED_DEV_MHL_MUIC,
	ATTACHED_DEV_CHARGING_CABLE_MUIC,
	ATTACHED_DEV_AFC_CHARGER_PREPARE_MUIC,
	ATTACHED_DEV_AFC_CHARGER_PREPARE_DUPLI_MUIC,
	ATTACHED_DEV_AFC_CHARGER_5V_MUIC,
	ATTACHED_DEV_AFC_CHARGER_5V_DUPLI_MUIC,
	ATTACHED_DEV_AFC_CHARGER_9V_MUIC,
	ATTACHED_DEV_AFC_CHARGER_9V_DUPLI_MUIC,
	ATTACHED_DEV_AFC_CHARGER_12V_MUIC,
	ATTACHED_DEV_AFC_CHARGER_12V_DUPLI_MUIC,

	ATTACHED_DEV_AFC_CHARGER_ERR_V_MUIC,
	ATTACHED_DEV_AFC_CHARGER_ERR_V_DUPLI_MUIC,
	ATTACHED_DEV_QC_CHARGER_PREPARE_MUIC,
	ATTACHED_DEV_QC_CHARGER_5V_MUIC,
	ATTACHED_DEV_QC_CHARGER_ERR_V_MUIC,
	ATTACHED_DEV_QC_CHARGER_9V_MUIC,
	ATTACHED_DEV_HV_ID_ERR_UNDEFINED_MUIC,
	ATTACHED_DEV_HV_ID_ERR_UNSUPPORTED_MUIC,
	ATTACHED_DEV_HV_ID_ERR_SUPPORTED_MUIC,
	ATTACHED_DEV_HMT_MUIC,

	ATTACHED_DEV_VZW_ACC_MUIC,
	ATTACHED_DEV_VZW_INCOMPATIBLE_MUIC,
	ATTACHED_DEV_USB_LANHUB_MUIC,
	ATTACHED_DEV_TYPE1_CHG_MUIC,
	ATTACHED_DEV_TYPE2_CHG_MUIC,
	ATTACHED_DEV_UNSUPPORTED_ID_MUIC,
	ATTACHED_DEV_UNSUPPORTED_ID_VB_MUIC,
	ATTACHED_DEV_UNDEFINED_RANGE_MUIC,
	ATTACHED_DEV_RDU_TA_MUIC,
	ATTACHED_DEV_GAMEPAD_MUIC,

	ATTACHED_DEV_TIMEOUT_OPEN_MUIC,
	ATTACHED_DEV_HICCUP_MUIC,
	ATTACHED_DEV_POGO_DOCK_MUIC,
	ATTACHED_DEV_POGO_DOCK_5V_MUIC,
	ATTACHED_DEV_POGO_DOCK_9V_MUIC,
	/* 4.4-only members; appended so no existing 4.9 ordinal moves.
	 *
	 * The enum is renumbered between the two trees: 4.4 has 79 members,
	 * 4.9 has 68, 14 are 4.4-only, 3 are 4.9-only, and 22 shared
	 * members disagree. First divergence is 4.4 ordinal 43,
	 * ATTACHED_DEV_AFC_CHARGER_DISABLED_MUIC. These are appended HERE
	 * with no explicit values, so they take the free 4.9 ordinals 66..79
	 * and only ATTACHED_DEV_UNKNOWN_MUIC (66 -> 80) and ATTACHED_DEV_NUM
	 * (67 -> 81) move. Giving them their 4.4 ordinals instead was
	 * rejected: every 4.4 ordinal in the 43..76 range is already taken
	 * by a 4.9 member, which would alias ~14 pairs of enumerators --
	 * and dev_to_str() is a switch over these symbols, so a duplicate
	 * value is a duplicate-case compile error, not merely a mislabel.
	 *
	 * The AFC device constants, ATTACHED_DEV_AFC_CHARGER_PREPARE_MUIC
	 * through ATTACHED_DEV_AFC_CHARGER_ERR_V_DUPLI_MUIC, are 4.4
	 * ordinals 33..42 and are identical in both trees, so AFC voltage
	 * negotiation is unaffected. */
	ATTACHED_DEV_AFC_CHARGER_DISABLED_MUIC,	/* 4.4 ord 43 */
	ATTACHED_DEV_TYPE3_MUIC,			/* 57 */
	ATTACHED_DEV_TYPE3_MUIC_TA,			/* 58 */
	ATTACHED_DEV_TYPE3_ADAPTER_MUIC,		/* 59 */
	ATTACHED_DEV_TYPE3_CHARGER_MUIC,		/* 60 */
	ATTACHED_DEV_NONE_TYPE3_MUIC,			/* 61 */
	ATTACHED_DEV_WIRELESS_PAD_MUIC,		/* 65 */
	ATTACHED_DEV_CARKIT_MUIC,			/* 66 */
	ATTACHED_DEV_POWERPACK_MUIC,			/* 67 */
	ATTACHED_DEV_CHK_WATER_REQ,			/* 70 */
	ATTACHED_DEV_CHK_WATER_DRY_REQ,			/* 71 */
	ATTACHED_DEV_CHECK_OCP,			/* 73 */
	ATTACHED_DEV_FACTORY_UART_MUIC,		/* 75 */
	ATTACHED_DEV_ABNORMAL_OTG_MUIC,		/* 76 */

	ATTACHED_DEV_UNKNOWN_MUIC,
	ATTACHED_DEV_NUM,
} muic_attached_dev_t;

#ifdef CONFIG_MUIC_HV_FORCE_LIMIT
/* MUIC attached device type */
typedef enum {
	SILENT_CHG_DONE = 0,
	SILENT_CHG_CHANGING = 1,

	SILENT_CHG_NUM,
} muic_silent_change_state_t;
#endif


/* muic common callback driver internal data structure
 * that setted at muic-core.c file
 */
struct muic_platform_data {
	int irq_gpio;

	int switch_sel;

	/* muic current USB/UART path */
	int usb_path;
	int uart_path;

	int gpio_uart_sel;

	bool rustproof_on;
	bool afc_disable;

#ifdef CONFIG_MUIC_HV_FORCE_LIMIT
	int hv_sel;
	int silent_chg_change_state;
#endif
	enum muic_op_mode opmode;

	/* muic switch dev register function for DockObserver */
	void (*init_switch_dev_cb)(void);
	void (*cleanup_switch_dev_cb)(void);

	/* muic GPIO control function
	 *
	 * The 4.9 base declared this as "(int switch_sel)", the max77705 /
	 * universal generation form. 4.4 muic.h:310 declares
	 * "(void *data, int switch_sel)", and that is the form the S2MU106
	 * stack implements: drivers/muic/muic_core.c:271 casts the first
	 * argument to struct muic_platform_data * and dereferences ->muic_if,
	 * ->usb_path and ->uart_path, so the argument is load-bearing and
	 * cannot be dropped at the call site.
	 *
	 * Every caller of the 1-argument form lives in a translation unit this
	 * board does not build: CONFIG_MUIC_MAX77705, MUIC_MAX77854,
	 * MUIC_UNIVERSAL, MUIC_SM5508 and MUIC_TSU6721 are all unset in the
	 * effective .config, and the build log only ever reaches muic_core.o,
	 * muic_manager.o, muic_notifier.o, muic_sysfs.o, s2mu106-muic.o and
	 * s2mu106-muic-afc.o. Of the surviving users the only one is
	 * drivers/muic/s2mu106-muic.c:2912, which already passes two
	 * arguments. Restored to the 4.4 signature.
	 */
	int (*init_gpio_cb)(void *data, int switch_sel);
	int (*set_gpio_usb_sel)(int usb_path);
	int (*set_gpio_uart_sel)(int uart_path);
	int (*set_safeout)(int safeout_path);

	/* muic path switch function for rustproof */
	void (*set_path_switch_suspend)(struct device *dev);
	void (*set_path_switch_resume)(struct device *dev);

	/* muic AFC voltage switching function */
	int (*muic_afc_set_voltage_cb)(int voltage);

	/* muic check charger init function */
	int (*muic_hv_charger_init_cb)(void);

	/* muic set hiccup mode function */
	int (*muic_set_hiccup_mode_cb)(int on_off);

	/* --- S2MU106 per-chip state, restored from the 4.4 tree ---
	 * drivers/muic/{muic_core,muic_manager,s2mu106-muic,s2mu106-muic-afc}.c
	 * all read these; the 4.9 max77705-generation header dropped them.
	 * Appended AFTER every pre-existing member, so no existing field
	 * offset moves. Verified: no 4.9 file reads any of these through
	 * struct muic_platform_data -- the universal/max77705 stack keeps its
	 * own struct muic_data fields of the same names, reached via pmuic->
	 * and muic_data->, never pdata->. Every initialiser of this struct is
	 * designated and the only allocation is kzalloc(sizeof(*pdata)), so
	 * growing it is inert. */
	void *drv_data;
	void *muic_if;
	bool suspended;
	bool need_to_noti;
	int gpio_usb_sel;
	muic_attached_dev_t attached_dev;
	bool is_usb_ready;
	bool is_factory_start;
	bool is_rustproof;
	bool is_otg_test;
	bool is_jig_on;
	int vbvolt;
	int adc;
	/* muic_hv_state_t: that typedef only exists in the S2MU106-private
	 * include/linux/muic/s2mu106-muic.h and is not visible from here.
	 * Its values (HV_STATE_INVALID..HV_STATE_MAX_NUM) all fit an int. */
	int hv_state;
	void (*jig_uart_cb)(int jig_state);

#ifdef CONFIG_MUIC_SYSFS
	/* 4.4 include/linux/muic/muic.h:278-281, guarded by CONFIG_MUIC_SYSFS
	 * there and here. The 4.9 max77705 generation never created a
	 * "switch" sec_device at all: muic_interface.h only declares the extern
	 * global, while drivers/muic/muic_sysfs.c:645-678 hangs
	 * muic_sysfs_group off muic_pdata->switch_device->kobj and takes
	 * sysfs_mutex, so the two fields are required, not optional. Appended,
	 * not interleaved, for the same reason as the fields above: every
	 * initialiser of this struct is designated and the only allocation is
	 * kzalloc(sizeof(*pdata)), so growing it is inert.
	 *
	 * The 4.9-generation callers of muic_sysfs_init() are not built on this
	 * board (see init_gpio_cb above); universal/muic_sysfs.c is a different
	 * object behind a different symbol. */
	struct device *switch_device;
	struct mutex sysfs_mutex;
#endif
};

int get_switch_sel(void);
int get_afc_mode(void);
int get_ccic_info(void);
void muic_set_hmt_status(int status);
int muic_afc_set_voltage(int voltage);
int muic_hv_charger_init(void);
int muic_set_hiccup_mode(int on_off);
#ifdef CONFIG_SEC_FACTORY
extern void muic_send_attached_muic_cable_intent(int type);
#endif /* CONFIG_SEC_FACTORY */
#endif /* __MUIC_H__ */
