#
# Copyright (c) 2013 Samsung Electronics Co., Ltd.
#		http://www.samsung.com
#
# Licensed under GPLv2
#
# Exynos7885 (A30s) display stack: DECON + DPP + DSIM.
# Ported verbatim from the 4.4 A30s tree; see Kconfig for the delta list.
#

subdir-ccflags-$(CONFIG_SEC_GPIO_DVS)	+= -DCONFIG_SMCDSD_LCD_DEBUG
subdir-ccflags-y	+= -DCONFIG_EXYNOS_DPU20

obj-$(CONFIG_EXYNOS_DPP) += dpp.o
dpp-y := dpp_drv.o
obj-$(CONFIG_EXYNOS_MIPI_DSIM) += dsim.o
dsim-y += dsim_drv.o
obj-$(CONFIG_SOC_EXYNOS7885) += dsim_reg.o dpp_reg.o decon_reg.o
obj-$(CONFIG_EXYNOS_DECON_7885) += decon.o
obj-$(CONFIG_DECON_EVENT_LOG) += event_log.o
decon-y += decon_core.o decon_dsi.o helper.o win_update.o

# PORT-NOTE(dpu_7885): in 4.4 bts.o was unconditionally in decon-y. It cannot
# stay there: 4.4's bts.c includes <soc/samsung/bts.h> unconditionally, and in
# the 4.9 tree that header's entire body is gated on
#   #if defined(CONFIG_EXYNOS7885_BTS) || defined(CONFIG_EXYNOS9810_BTS)
# (include/soc/samsung/bts.h:14). With neither set the header is empty and
# bts.c fails on the undeclared cal_bts7885_* helpers. Gating on the same
# symbol the compiling 9810 sibling uses (dpu_9810/Makefile:17).
obj-$(CONFIG_EXYNOS7885_BTS) += bts.o

obj-y += panels/

# NOT ported (4.4-only, and off in the live running.config):
#   obj-$(CONFIG_EXYNOS_DOZE) += decon_doze.o dsim_doze.o
#     CONFIG_EXYNOS_DOZE=n (running.config:2888)
#   ccflags-$(CONFIG_SAMSUNG_TUI) += -Idrivers/misc/tui
#     no tui/ in either tree; drivers/misc is another agent's
