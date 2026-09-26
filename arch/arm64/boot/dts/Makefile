# SPDX-License-Identifier: GPL-2.0
dtb-$(CONFIG_CAMERA_STAR2) += exynos/exynos9810-star2lte_eur_open_26.dtb

dtb-$(CONFIG_CAMERA_STAR) += exynos/exynos9810-starlte_eur_open_26.dtb

# EXYNOS7885 / EXYNOS7904 (Galaxy A30s, SM-A307) -- device tree skeleton ported
# verbatim from the live 4.4 A30s tree. The base SoC DTB is unconditional, exactly
# as in 4.4; the four board overlays are gated on the machine symbol, as in 4.4.
# NOTE: this block must stay ABOVE the `DTB_LIST :=` line below, which is an
# immediate assignment and therefore would not pick up anything declared after it.
dtb-y += exynos/dtb/exynos7885.dtb

dtbo-$(CONFIG_MACH_EXYNOS7885_A30s_OPEN) := \
	exynos/dtbo/exynos7904-a30s_eur_open_00.dtbo \
	exynos/dtbo/exynos7904-a30s_eur_open_01.dtbo \
	exynos/dtbo/exynos7904-a30s_eur_open_02.dtbo \
	exynos/dtbo/exynos7904-a30s_eur_open_03.dtbo

targets += dtbs
DTB_LIST  := $(dtb-y) $(dtbo-y)
always    := $(DTB_LIST)

dtbs: $(addprefix $(obj)/, $(DTB_LIST))

clean-files := *.dtb*
