#
# Makefile for the sensors drivers.
#
# 4.9 port note (exynos7885 / A30s):
#
# Copied from the 4.4 donor, restricted to the 19 files this port actually
# brings in.  Tabs, not spaces, matter in a kernel Makefile obj- line: the
# donor used a mix of both, and only the tab form works.
#
# The duplicate-symbol constraint recorded in Kconfig applies here too:
# sensors_core.o must never be linked alongside
# drivers/sensorhub/brcm/sensors_core.o.

# Each configuration option enables a list of files.

obj-$(CONFIG_SENSORS_CORE)		+= sensors_core.o

# accelerometer_sensor
obj-$(CONFIG_SENSORS_LSM6DSL)		+= lsm6dsl_core.o
obj-$(CONFIG_SENSORS_LSM6DSL_I2C)	+= lsm6dsl_i2c.o

# magnetic_sensor
obj-$(CONFIG_SENSORS_YAS539)		+= yas_mag_drv-yas539.o yas_mag_kernel.o

# optical_sensor
obj-$(CONFIG_SENSORS_STK3X3X)		+= stk3x3x.o
obj-$(CONFIG_SENSORS_TCS3701)		+= ams_dax.o ams_i2c.o

# grip_sensor
obj-$(CONFIG_SENSORS_A96T3X6)		+= a96t3x6.o
obj-$(CONFIG_SENSORS_A96T3X6_SUB)	+= a96t3x6_sub.o
