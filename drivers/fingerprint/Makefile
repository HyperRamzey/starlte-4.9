#
# Makefile for the sensors drivers.
#

# Each configuration option enables a list of files.

ccflags-y := $(KBUILD_FP_SENSOR_CFLAGS)

obj-$(CONFIG_SENSORS_FINGERPRINT)	+= fingerprint_sysfs.o
obj-$(CONFIG_SENSORS_VFS8XXX)			+= vfs8xxx.o
obj-$(CONFIG_SENSORS_ET5XX)				+= et5xx-spi.o et5xx-spi_data_transfer.o
# NOTE: CONFIG_SENSORS_ET7XX and CONFIG_SENSORS_ET5XX are mutually
# exclusive -- the two objects export the same global symbols.  See the
# Kconfig.  For exynos7885/a30s keep ET5XX off and ET7XX on.
obj-$(CONFIG_SENSORS_ET7XX)				+= et7xx-spi.o et7xx-spi_data_transfer.o
