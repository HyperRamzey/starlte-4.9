#
# Makefile for exynos7885 platform
#

ccflags-y += -I$(srctree)/drivers/video/fbdev/exynos/dpu_7885
ccflags-y += -I$(srctree)/drivers/misc/tui
# PORT-NOTE(tui): stui_hal_display.c reads ion_priv.h to recover the physical
# address of the ION handle, because this kernel's ION dropped ion_phys().
# See PORT-NOTES.md.  Using $(srctree) rather than the 4.4 bare relative path
# so the -I is valid from an O= out-of-tree build, which is how this tree is
# always built (AGENTS.md section 18.7).
ccflags-y += -I$(srctree)/drivers/staging/android/ion

obj-y	+= stui_hal_display.o
obj-y	+= stui_hal_touch.o
