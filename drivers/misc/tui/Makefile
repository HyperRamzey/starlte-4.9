#
# Makefile for the stui Module
#

obj-$(CONFIG_SAMSUNG_TUI)	+= main.o
obj-$(CONFIG_SAMSUNG_TUI)	+= stui_core.o
obj-$(CONFIG_SAMSUNG_TUI)	+= stui_inf.o

ifeq ($(CONFIG_SOC_EXYNOS3250),y)
obj-$(CONFIG_SAMSUNG_TUI)	+= platforms/exynos3250/
endif
ifeq ($(CONFIG_SOC_EXYNOS7420),y)
obj-$(CONFIG_SAMSUNG_TUI)	+= platforms/exynos7420/
endif
ifeq ($(CONFIG_SOC_EXYNOS7885),y)
# PORT-NOTE(tui): 4.4 gated the GlobalPlatform client API on
# "TEEGRIS_VERSION != 2", a proxy for "the 3.0 flavour is the one that gets
# built".  The proxy does not hold here: TEEGRIS_VERSION's Kconfig default
# is 2 (drivers/misc/tzdev/3.0/Kconfig:222), this port carries only the 3.0
# flavour, and the symbol is set by neither the defconfig nor the built
# .config.  The 4.4 test evaluates false, the -DUSE_TEE_CLIENT_API define is
# dropped, and stui_inf.c silently compiles its "not supported"
# stui_cancel_session() stub with no build diagnostic at all.
# Gate on the two things that are actually true in this tree instead.  If
# CONFIG_TZDEV is off the header include is dropped as well, so TUI still
# builds standalone and degrades exactly as 4.4 does when tzdev/ is absent.
ifneq ($(CONFIG_TZDEV),)
ifneq ($(wildcard $(srctree)/drivers/misc/tzdev/3.0/Makefile),)
ccflags-$(CONFIG_SAMSUNG_TUI)	+= -DUSE_TEE_CLIENT_API
ccflags-$(CONFIG_SAMSUNG_TUI)	+= -I$(srctree)/drivers/misc/tzdev/3.0/include/tzdev
endif
endif
obj-$(CONFIG_SAMSUNG_TUI)	+= platforms/exynos7885/
endif
ifeq ($(CONFIG_SOC_EXYNOS8895),y)
obj-$(CONFIG_SAMSUNG_TUI)	+= platforms/exynos8895/
endif
ifeq ($(CONFIG_SOC_EXYNOS9820),y)
ccflags-$(CONFIG_SAMSUNG_TUI)	+= -DUSE_TEE_CLIENT_API
ccflags-$(CONFIG_SAMSUNG_TUI)	+= -Idrivers/misc/tzdev/include/tzdev
obj-$(CONFIG_SAMSUNG_TUI)	+= platforms/exynos9820/
endif
ifeq ($(CONFIG_MACH_MT6757),y)
obj-$(CONFIG_SAMSUNG_TUI)	+= platforms/mt6757/
endif

ccflags-$(CONFIG_SAMSUNG_TUI_TEST)	+= -DSAMSUNG_TUI_TEST
obj-$(CONFIG_SAMSUNG_TUI_TEST)	+= test_driver/
