#
# Makefile for the sensor hub.
#
# PORT NOTE (exynos7885 / A30s, 2026-09-27).  This brcm/ SSP sensor hub is
# BROADCOM hardware and it is NOT what the A30s uses.  Comment only; nothing
# below changes behaviour, and no build flag is touched here.
#
# Evidence, all from the live A30s (4.4 kernel, port49_evidence/dev):
#   - /proc/kallsyms contains 0 sensorhub symbols and 0 SSP sensor symbols.
#     The only ssp_* hits in that file are kernel crypto (decode_ntlmssp_
#     challenge, build_ntlmssp_auth_blob, sess_auth_rawntlmssp_negotiate, ...).
#   - The live config has CONFIG_SENSORS_SSP not set, CONFIG_SENSORS_SSP_STM32
#     not set, CONFIG_IIO_SSP_SENSORHUB not set, and every CONFIG_SENSORS_SSP_*
#     sensor (ACCELOMETER, GYROSCOPE, MAGNETIC, PROXIMITY, LIGHT, BAROMETER,
#     MOBEAM and all their per-part variants) not set.  The 4.4 tree agrees:
#     its own drivers/sensorhub/Kconfig sources ONLY stm_v2/Kconfig, and 4.4's
#     live config sets none of them either.
#   - The A30s has no SSP coprocessor in its device tree at all.  Its real
#     sensors are plain I2C parts on i2c@13880000 and i2c@13890000, and
#     platform_binding.txt records the two that bind:
#         i2c 12-0047 STK3031
#         i2c 13-006b lsm6dsl-i2c
#     The live device tree also carries ams_dax@39 (compatible "ams,tcs3701"),
#     yas539@2e, and no SSP/BRCM node whatsoever.
#   - Those parts are claimed by 4.4's drivers/sensors/, NOT by sensorhub:
#     ams_dax.c claims "ams,tcs3701", stk3x3x.c claims "stk,stk3x3x",
#     lsm6dsl_i2c.c claims lsm6dsl-i2c, yas_mag_drv-yas539.c claims the mag.
#     The A30s live config enables exactly those: CONFIG_SENSORS_TCS3701=y,
#     CONFIG_SENSORS_STK3X3X=y, CONFIG_SENSORS_LSM6DSL=y and _I2C=y,
#     CONFIG_SENSORS_YAS539=y, CONFIG_SENSORS_A96T3X6=y with
#     CONFIG_SENSORS_A96T3X6_A30S=y and _SUB=y, plus
#     CONFIG_SENSORS_LSM6DSL_SUPPORT_VDIS=y.
#
# Two consequences for the 7885 port:
#   1. drivers/sensors/ does not exist in this tree at all (4.4 has 57 files,
#      ~52.7k lines for it), so the drivers that actually claim the A30s
#      sensors have not been ported.  That is the real sensor gap.
#   2. CONFIG_SENSORS_SSP=y, CONFIG_SENSORS_SSP_BBD=y,
#      CONFIG_SENSORS_SSP_LSM6DSL=y and CONFIG_SENSORS_SSP_LPS22H=y are in the
#      current exynos7885-a30s_defconfig and resolve (drivers/Kconfig sources
#      drivers/sensorhub/brcm/Kconfig).  They are 9810 carry-over: the S9's
#      sensors are behind the Broadcom hub, the A30s's are not.  Nothing here
#      can ever bind, and as of b11 this whole directory has never been
#      compiled -- obj_a30s/drivers/sensorhub contains 0 .o files, because the
#      build aborted in drivers/net before reaching it.  93 files / ~195k lines
#      of unverified clang-24 code is a large latent surface.
#      Recommended: turn those four symbols off in the defconfig.  That is a
#      defconfig edit, which this port does not own, so it is reported rather
#      than applied.
#
# Note on the "no Kconfig here" shape: drivers/sensorhub/Kconfig does not exist
# in this tree.  That is not an orphan -- drivers/Kconfig sources
# drivers/sensorhub/brcm/Kconfig directly, which is where SENSORS_SSP,
# SENSORS_SSP_BBD and SENSORS_SSP_SENSORHUB are defined.  The symbol names
# simply collide across generations, which is the trap: SENSORS_SSP_BBD here
# is the Broadcom hub's gate, while 4.4's SENSORS_SSP_STM32 gated the STM32F410
# hub.  Same name family, different silicon, opposite verdict.

obj-$(CONFIG_SENSORS_SSP_BBD)	+= brcm/
