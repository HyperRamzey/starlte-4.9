#
# Copyright (c) 2013 Samsung Electronics Co., Ltd.
#		http://www.samsung.com
#
# Licensed under GPLv2
#
# Only the A30s S6E8FC1 panel is ported. The other 84 panel files in the 4.4
# dpu_7885/panels/ tree (A10/A20/M20/A6/J7/J2/XCover4S/...) are deliberately
# omitted: every one of them is "# CONFIG_... is not set" in the live
# running.config, and together they are ~112k lines.
#

obj-y += dsim_panel.o ../decon_board.o ../decon_notify.o ../decon_abd.o
obj-$(CONFIG_DISPLAY_USE_INFO) += dpui.o
obj-$(CONFIG_EXYNOS_DECON_MDNIE) += mdnie_lite.o

obj-$(CONFIG_EXYNOS_DECON_LCD_S6E8FC1_A30S)	+= s6e8fc1_a30s_lcd_ctrl.o

# 4.4 had $(call cc-disable-warning, int-conversion) for ~7 panels, including
# this one (4.4 panels/Makefile:44). $(call cc-disable-warning,...) still
# resolves in 4.9, so keep it: the s6e8fc1 mdnie tables mix u32 and s32 and
# would otherwise trip -Wint-conversion under the tree's -Werror.
CFLAGS_s6e8fc1_a30s_lcd_ctrl.o := $(call cc-disable-warning, int-conversion)

# NOT ported, all disabled in the live running.config:
#   s6e3fa0/s6e3fa3/s6e3fa7 ...         panel drivers, not set
#   mdnie_control.o                       CONFIG_EXYNOS_DECON_MDNIE_CONTROL=n
#   dd_mdnie.o dd_lcd.o dd_backlight.o
#   dd_dpu.o dd_param.o                   CONFIG_SEC_GPIO_DVS=n
#   poc.o ea8076_a30_poc.o                CONFIG_SUPPORT_POC_FLASH=n
#   dynamic_aid.o                         only built for A6ELTE/J7DUO/Jackpot/A7Y18
# dd.h IS ported: under CONFIG_SAMSUNG_PRODUCT_SHIP=y (running.config:3942) every
# declaration in it degrades to a static inline stub, so no dd_*.o is needed.
