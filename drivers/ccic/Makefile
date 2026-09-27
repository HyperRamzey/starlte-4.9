#
# Makefile for ccic devices
#

# S2MU106 CCIC. CCIC_S2MU106 is intentionally left off - see drivers/ccic/
# Kconfig for why the USB-PD framework is not ported.
obj-$(CONFIG_CCIC_S2MU106)	+= s2mu106-usbpd.o
obj-$(CONFIG_CCIC_PDIC_NOTIFIER)	+= pdic_notifier.o

obj-$(CONFIG_CCIC_NOTIFIER)		+= ccic_notifier.o ccic_sysfs.o ccic_core.o ccic_misc.o
obj-$(CONFIG_CCIC_ALTERNATE_MODE)	+= ccic_alternate.o
obj-$(CONFIG_CCIC_S2MM003)		+= s2mm003.o
obj-$(CONFIG_CCIC_S2MM005)		+= s2mm005_fw.o s2mm005_cc.o s2mm005_pd.o sec_pd.o s2mm005.o
obj-$(CONFIG_CCIC_MAX77705)		+= max77705_cc.o max77705_pd.o sec_pd.o max77705_usbc.o max77705_alternate.o
obj-$(CONFIG_CCIC_MAX77705_DEBUG)	+= max77705_debug.o
