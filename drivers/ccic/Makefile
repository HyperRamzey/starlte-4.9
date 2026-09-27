#
# Makefile for ccic devices
#

# S2MU106 CCIC, plus the chip-independent USB-PD framework.
#
# The four framework objects are the whole of the 10,345-line port that
# CCIC_S2MU106 was previously unable to switch on. 4.4 reaches them from
# USE_CCIC, and so does this.
#
# 4.4's equivalent lines are
#   obj-$(CONFIG_USE_CCIC) += usbpd.o usbpd_cc.o
#   obj-$(CONFIG_USE_CCIC) += usbpd_policy.o usbpd_manager.o pdic_notifier.o ccic_misc.o
# and the two trailing objects are deliberately NOT repeated here. Both are
# already reachable by their own, more specific symbols in this tree:
# pdic_notifier.o by CONFIG_CCIC_PDIC_NOTIFIER and ccic_misc.o by
# CONFIG_CCIC_NOTIFIER, both of which are above. Adding them a second time
# would put the same object into obj-y twice. 4.4 had the same hazard between
# USE_CCIC and CCIC_ALTERNATE_MODE, which both named ccic_misc.o; it just
# never bit because that combination was never enabled.
obj-$(CONFIG_USE_CCIC)		+= usbpd.o usbpd_cc.o usbpd_policy.o usbpd_manager.o
obj-$(CONFIG_CCIC_S2MU106)	+= s2mu106-usbpd.o
obj-$(CONFIG_CCIC_PDIC_NOTIFIER)	+= pdic_notifier.o

obj-$(CONFIG_CCIC_NOTIFIER)		+= ccic_notifier.o ccic_sysfs.o ccic_core.o ccic_misc.o
obj-$(CONFIG_CCIC_ALTERNATE_MODE)	+= ccic_alternate.o
obj-$(CONFIG_CCIC_S2MM003)		+= s2mm003.o
obj-$(CONFIG_CCIC_S2MM005)		+= s2mm005_fw.o s2mm005_cc.o s2mm005_pd.o s2mm005.o
obj-$(CONFIG_CCIC_MAX77705)		+= max77705_cc.o max77705_pd.o max77705_usbc.o max77705_alternate.o

obj-$(CONFIG_CCIC_MAX77705_DEBUG)	+= max77705_debug.o
# sec_pd.c exists only to satisfy the "extern struct pdic_notifier_struct
# pd_noti" that max77705_pd.c:48 declares; it is the S2MM005 / MAX77705
# generation's home for that symbol. Two defects, both fixed here:
#   1. it was added by BOTH the S2MM005 and the MAX77705 line, so with both
#      = y (the current defconfig) sec_pd.o landed in obj-y twice;
#   2. s2mu106-usbpd.c:59 DEFINES pd_noti itself, under
#      CONFIG_BATTERY_SAMSUNG && CONFIG_USB_TYPEC_MANAGER_NOTIFIER (both
#      = y in the defconfig), so linking sec_pd.o with s2mu106-usbpd.o is
#      a duplicate-symbol error. Excluded from the S2MU106 case.
ifneq ($(filter y,$(CONFIG_CCIC_S2MM005) $(CONFIG_CCIC_MAX77705)),)
ifneq ($(CONFIG_CCIC_S2MU106),y)
obj-y					+= sec_pd.o
endif
endif
