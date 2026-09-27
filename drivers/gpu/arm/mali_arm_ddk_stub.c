/* SPDX-License-Identifier: GPL-2.0 */
/*
 * drivers/gpu/arm - placeholder translation unit
 *
 * The parent directory (drivers/gpu/Makefile) descends into this directory
 * unconditionally, with "obj-y += arm/".  scripts/Makefile.lib rewrites every
 * "obj-y" entry of the form "dir/" into a hard "dir/built-in.o" prerequisite
 * of the parent's built-in.o, so this directory must always be able to
 * produce a built-in.o even when it contributes no driver.
 *
 * Every "obj-y += <generation>/" line in this directory's Kbuild is guarded
 * by CONFIG_MALI_TMIX or CONFIG_MALI_THEX, and those two symbols are only
 * defined when drivers/gpu/arm/Kconfig sources the matching generation
 * Kconfig - which it does only for SOC_EXYNOS8895 (tMIx) and SOC_EXYNOS9810
 * (tHEx).  On any other SoC (Exynos 7885 / A30s, for example) no
 * configuration symbol selects a generation, the directory builds nothing,
 * scripts/Makefile.build never defines builtin-target for it, and the link
 * of drivers/gpu/built-in.o then fails with
 *
 *   ld.bfd: cannot find drivers/gpu/arm/built-in.o: No such file or directory
 *
 * Kbuild always adds a subdirectory's built-in.o to the parent link, whether
 * or not the subdirectory has any objects, so the empty case has to yield an
 * object anyway.  This file exists solely to be that object when no Mali
 * generation is selected.  It is NOT added when a generation is selected (see
 * the guard at the bottom of Kbuild), so a correctly configured build never
 * compiles it.
 */

int mali_arm_ddk_stub_placeholder(void);
int mali_arm_ddk_stub_placeholder(void)
{
	return 0;
}
