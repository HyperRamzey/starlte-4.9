#
# Makefile for muic devices
#

# muic-core.c is the max77705 / MUIC_UNIVERSAL generation's core. It is NOT
# the same file as the S2MU106 generation's drivers/muic/muic_core.c, and
# the two export the same five globals (get_switch_sel, get_afc_mode,
# get_ccic_info, muic_send_dock_intent, muic_afc_set_voltage), so they
# cannot both be linked. Gate the legacy core on the legacy chips.
# With MUIC_MAX77705 and MUIC_UNIVERSAL both = y -- the current defconfig --
# this yields exactly the same object set as the old unconditional obj-y;
# the ifeq is only there so the object is not added twice.
# MUIC_NOTIFIER=y does not need this object: the only muic_notifier.c ->
# muic-core.c reference, muic_send_attached_muic_cable_intent, sits inside
# #ifdef CONFIG_SEC_FACTORY on both sides, and SEC_FACTORY is not set.
ifneq ($(filter y,$(CONFIG_MUIC_MAX77705) $(CONFIG_MUIC_UNIVERSAL)),)
obj-y					+= muic-core.o
endif
obj-$(CONFIG_MUIC_NOTIFIER)		+= muic_notifier.o
obj-$(CONFIG_MUIC_MAX77854)		+= max77854-muic.o
obj-$(CONFIG_HV_MUIC_MAX77854_AFC)	+= max77854-muic-afc.o
obj-$(CONFIG_MUIC_MAX77705)		+= max77705-muic.o
obj-$(CONFIG_HV_MUIC_MAX77705_AFC)	+= max77705-muic-afc.o
obj-$(CONFIG_MUIC_MAX77705_CCIC)	+= max77705-muic-ccic.o

# MUIC_UNIVERSAL
ifeq ($(CONFIG_MUIC_UNIVERSAL), y)
  obj-$(CONFIG_MUIC_SUPPORT_CCIC)	+= universal/muic_ccic.o
endif
obj-$(CONFIG_MUIC_UNIVERSAL)	+= universal/muic_usb.o
obj-$(CONFIG_MUIC_UNIVERSAL)	+= universal/muic_coagent.o
#obj-$(CONFIG_MUIC_UNIVERSAL)	+= universal/muic_task.o
obj-$(CONFIG_MUIC_UNIVERSAL)	+= universal/muic_state.o
obj-$(CONFIG_MUIC_UNIVERSAL)	+= universal/muic_apis.o
obj-$(CONFIG_MUIC_UNIVERSAL)	+= universal/muic_sysfs.o
obj-$(CONFIG_MUIC_UNIVERSAL)	+= universal/muic_debug.o
obj-$(CONFIG_MUIC_UNIVERSAL)	+= universal/muic_dt.o
obj-$(CONFIG_MUIC_UNIVERSAL)	+= universal/muic_i2c.o
obj-$(CONFIG_MUIC_UNIVERSAL)	+= universal/muic_regmap.o
obj-$(CONFIG_MUIC_UNIVERSAL)	+= universal/muic_vps.o
obj-$(CONFIG_MUIC_UNIVERSAL_MAX77854)	+= universal/muic_task_max77854.o
obj-$(CONFIG_MUIC_UNIVERSAL_MAX77854)	+= universal/muic_regmap_max77854.o
obj-$(CONFIG_MUIC_HV_MAX77854)		+= universal/muic_hv_max77854.o
obj-$(CONFIG_MUIC_UNIVERSAL_MAX77865)	+= universal/muic_task_max77865.o
obj-$(CONFIG_MUIC_UNIVERSAL_MAX77865)	+= universal/muic_regmap_max77865.o
obj-$(CONFIG_MUIC_HV_MAX77865)		+= universal/muic_hv_max77865.o
obj-$(CONFIG_MUIC_SM5508)		+= sm5508-muic.o
#
# S2MU106 generation (4.4 exynos7885 / Galaxy A30s). muic_core.o and
# muic_manager.c were already in the tree but no Makefile rule reached them;
# see drivers/muic/Kconfig for why MUIC_S2MU106 excludes the legacy chips.
# drivers/muic/muic_sysfs.c is deliberately absent here: MUIC_SYSFS has no
# 4.9 Kconfig symbol and the 4.4 source file (654 lines) has not been ported,
# so s2mu106-muic.c's muic_sysfs_init/_deinit calls stay unresolved until the
# USB-PD wave brings it over.
obj-$(CONFIG_MUIC_CORE)				+= muic_core.o
obj-$(CONFIG_MUIC_MANAGER)		+= muic_manager.o
obj-$(CONFIG_MUIC_S2MU106)			+= s2mu106-muic.o
obj-$(CONFIG_HV_MUIC_S2MU106_AFC)	+= s2mu106-muic-afc.o
