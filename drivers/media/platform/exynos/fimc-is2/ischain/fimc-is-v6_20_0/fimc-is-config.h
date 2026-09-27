/*
 * Samsung Exynos5 SoC series FIMC-IS driver
 *
 *
 * Copyright (c) 2017 Samsung Electronics Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef FIMC_IS_CONFIG_H
#define FIMC_IS_CONFIG_H

#include <fimc-is-common-config.h>
#include <fimc-is-vendor-config.h>

/*
 * =================================================================================================
 * CONFIG -PLATFORM CONFIG
 * =================================================================================================
 */

#define SOC_30S
#define SOC_30C
#define SOC_30P
#define SOC_I0S
#define SOC_I0C
#define SOC_I0P
#define SOC_31S
#define SOC_31C
#define SOC_31P
/* #define SOC_I1S */
/* #define SOC_I1C */
/* #define SOC_I1P */
/* #define SOC_DRC */
/* #define SOC_D0S */
/* #define SOC_D0C */
/* #define SOC_D1S */
/* #define SOC_D1C */
/* #define SOC_ODC */
#define SOC_DNR
/* #define SOC_SCC */
/* #define SOC_SCP */
#define SOC_MCS
#define SOC_VRA
#define SOC_SSVC0
#define SOC_SSVC1
#define SOC_SSVC2
#define SOC_SSVC3

/* FIMC-IS task priority setting */
/*
 * These live here, and not only in the v5_2_0/v5_15_0 config headers they
 * were introduced in, because the code that uses them is not generation
 * specific: lib_get_task_priority() in interface/fimc-is-interface-library.c
 * and fimc-is_groupmgr() in fimc-is-groupmgr.c are compiled for whichever
 * FIMC-IS version is selected, and with V6_20_0 selected the v5_2_0/v5_15_0
 * headers are not on the include path at all, so the names were simply
 * undeclared. Every consumer of this header reaches it -- verified by
 * walking the #include closure of each of the five affected TUs.
 *
 * Values are copied verbatim from
 * xxmustafacooTR/exynos-linux-stable main@ffd1341ec
 *   ischain/fimc-is-v5_15_0/fimc-is-config.h:82-92
 * which is md5-identical (904b4c06ee20e121605fddd21ca7bff5) across our
 * tree, xm_main, ExyHyperBrick eh_9810@baa585f6 and els_starlte.
 */
#define TASK_SENSOR_WORK_PRIO		(FIMC_IS_MAX_PRIO - 48) /* 52 */
#define TASK_GRP_OTF_INPUT_PRIO		(FIMC_IS_MAX_PRIO - 49) /* 51 */
#define TASK_GRP_DMA_INPUT_PRIO		(FIMC_IS_MAX_PRIO - 50) /* 50 */
#define TASK_MSHOT_WORK_PRIO		(FIMC_IS_MAX_PRIO - 43) /* 57 */
#define TASK_LIB_OTF_PRIO		(FIMC_IS_MAX_PRIO - 44) /* 56 */
#define TASK_LIB_AF_PRIO		(FIMC_IS_MAX_PRIO - 45) /* 55 */
#define TASK_LIB_ISP_DMA_PRIO		(FIMC_IS_MAX_PRIO - 46) /* 54 */
#define TASK_LIB_3AA_DMA_PRIO		(FIMC_IS_MAX_PRIO - 47) /* 53 */
#define TASK_LIB_AA_PRIO		(FIMC_IS_MAX_PRIO - 48) /* 52 */
#define TASK_LIB_RTA_PRIO		(FIMC_IS_MAX_PRIO - 49) /* 51 */
#define TASK_LIB_VRA_PRIO		(FIMC_IS_MAX_PRIO - 45) /* 55 */

/*
 * VRA channel-1 interrupt count per frame, used to cadence the
 * msinfo_hw() ch1 trace in hardware/fimc-is-hw-vra.c. Same reasoning and
 * same provenance as the priorities above: v5_15_0/fimc-is-hw-chain.h:104
 * (identical in xm_main, eh_9810 and els_starlte) is the only definition on
 * our base, and that header is not on the include path for V6_20_0.
 */
#define VRA_CH1_INTR_CNT_PER_FRAME	(4)
/* #define SOC_DCP *//* TODO */
/* #define SOC_SRDZ *//* TODO */

/* Post Processing Configruation */
/* #define ENABLE_DRC */
/* #define ENABLE_DIS */
/* #define ENABLE_DNR_IN_TPU */
#ifndef DISABLE_TDNR_IN_MCSC
#define ENABLE_DNR_IN_MCSC
#define ENABLE_DNR_COMPRESSOR_IN_MCSC
#define MCSC_TDNR_YIC_MODE	(0)	/* enc/dec_mode> 0: compression, 1: raw mode */
#endif	//DISABLE_TDNR_IN_MCSC
#define ENABLE_10BIT_MCSC
/* #define ENABLE_DJAG_IN_MCSC */
#define ENABLE_VRA

/* VRA 1.4 improvement - adding VRA 1.4 interface */
#define ENABLE_VRA_LIBRARY_IMPROVE

#if defined(ENABLE_VRA_LIBRARY_IMPROVE)
#define ENABLE_VRA_CHANGE_SETFILE_PARSING
#undef VRA_OLD_POSES
#else
#undef ENABLE_VRA_CHANGE_SETFILE_PARSING
#define VRA_OLD_POSES
#endif

#define USE_ONE_BINARY
#define USE_RTA_BINARY
#define USE_DDK_SHUT_DOWN_FUNC
#define ENABLE_IRQ_MULTI_TARGET
#define FIMC_IS_ONLINE_CPU_MIN	4
/* #define USE_MCUCTL */

/* #define SOC_3AAISP */
#define SOC_MCS0
/* #define SOC_MCS1 */
/* #define SOC_TPU0 */
/* #define SOC_TPU1 */

#define HW_SLOT_MAX            (5)
#define valid_hw_slot_id(slot_id) \
       (0 <= slot_id && slot_id < HW_SLOT_MAX)
/* #define DISABLE_SETFILE */
/* #define DISABLE_LIB */

#define USE_SENSOR_IF_DPHY
#undef ENABLE_CLOCK_GATE
/* #define ENABLE_DIRECT_CLOCK_GATE */
#define ENABLE_HMP_BOOST

/*
 * =================================================================================================
 * CONFIG - FEATURE ENABLE
 * =================================================================================================
 */

#define FIMC_IS_MAX_TASK		(40)

#undef OVERFLOW_PANIC_ENABLE_ISCHAIN
#if defined(CONFIG_ARM_EXYNOS7885_BUS_DEVFREQ)
#define CONFIG_FIMC_IS_BUS_DEVFREQ
#endif
#define DDK_OVERFLOW_RECOVERY		(1)	/* 0: do not execute recovery, 1: execute recovery */
#define CAPTURE_NODE_MAX		12
#define OTF_YUV_FORMAT			(OTF_INPUT_FORMAT_YUV422)
#define USE_YUV_RANGE_BY_ISP
/* #define ENABLE_3AA_DMA_CROP */
/* #define ENABLE_ULTRA_FAST_SHOT */
#define ENABLE_HWFC
/* #define FW_SUSPEND_RESUME */
/* #define TPU_COMPRESSOR */
/* #define USE_I2C_LOCK */
#undef ENABLE_FULL_BYPASS
#define SENSOR_REQUEST_DELAY		2

#ifdef ENABLE_IRQ_MULTI_TARGET
#define FIMC_IS_HW_IRQ_FLAG     IRQF_GIC_MULTI_TARGET
#else
#define FIMC_IS_HW_IRQ_FLAG     0
#endif

/* #define MULTI_SHOT_KTHREAD */
/* #define ENABLE_EARLY_SHOT */

#ifdef USE_I2C_LOCK
#define I2C_MUTEX_LOCK(lock)	mutex_lock(lock)
#define I2C_MUTEX_UNLOCK(lock)	mutex_unlock(lock)
#else
#define I2C_MUTEX_LOCK(lock)
#define I2C_MUTEX_UNLOCK(lock)
#endif

/* init AWB */
/* #define ENABLE_INIT_AWB */
#define WB_GAIN_COUNT		(4)
#define INIT_AWB_COUNT_REAR	(3)
#define INIT_AWB_COUNT_FRONT	(7)

/* #define ENABLE_DBG_EVENT_PRINT */

#define USE_NEW_PER_FRAME_CONTROL
/* HACK */
#define DISABLE_CHECK_PERFRAME_FMT_SIZE

#define FAST_FDAE

#define CHAIN_SKIP_GFRAME_FOR_VRA
#endif
