# SPDX-License-Identifier: GPL-2.0

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

# Fold the gated list above into `dtbo-y`, which is the variable this build
# system actually consumes: both `DTB_LIST := $(dtb-y) $(dtbo-y)` here and
# arch/arm64/boot/Makefile:27 read only `dtbo-y`. Assigning to
# `dtbo-$(CONFIG_...)` alone populates `dtbo-CONFIG_MACH_EXYNOS7885_A30s_OPEN`
# (or bare `dtbo-` when the symbol is not set), so without this line DTB_LIST
# would contain only the base .dtb and all four .dtbo would be silently dropped.
# If the machine symbol is not set, the list above lands in `dtbo-` and this
# expands to nothing, which is the intended gating behaviour.
dtbo-y += $(dtbo-CONFIG_MACH_EXYNOS7885_A30s_OPEN)

targets += dtbs
DTB_LIST  := $(dtb-y) $(dtbo-y)
always    := $(DTB_LIST)

dtbs: $(addprefix $(obj)/, $(DTB_LIST))

clean-files := *.dtb*
