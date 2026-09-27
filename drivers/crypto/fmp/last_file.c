/*
 * Last file for Exynos FMP FIPS integrity check
 *
 * Copyright (C) 2015 Samsung Electronics Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include <linux/compiler.h>

#include <linux/init.h>

__attribute__ ((section(".rodata"), unused))
const unsigned char last_fmp_rodata = 0x20;

__attribute__ ((section(".text"), unused))
void last_fmp_text(void){}

/* Backport from xxmustafacooTR/exynos-linux-stable
 * main@ffd1341ec75864d08a32e0523fe850a133f00d1a, which is a sibling fork of this
 * exact base (4.9.219); main-ems@1589ebebbd2d916794ef068c7534e5b1274f01f6 carries the
 * identical fix. clang does not implement optimize("-O0"), so under clang the
 * attribute is dropped and the anchor is compiled at the file optimisation level,
 * which is what let it be reordered away. Ask clang for nothing instead of asking
 * for something it will not honour. CC_USE_CLANG is defined by the top-level
 * Makefile on the same CLANG_FLAGS line as --target=. */
#ifdef CC_USE_CLANG
__attribute__ ((section(".init.text"), used, unused))
/* `used` is load-bearing: these empty static functions are the FIPS
 * integrity boundary markers that scripts/crypto/fips_crypto_integrity.py
 * and scripts/fmp/fips_fmp_integrity.py look up by name to compute the HMAC
 * range. Without `used` the compiler drops the empty function and the whole
 * .init.text section with it, and the script dies with
 * "AttributeError: 'NoneType' object has no attribute 'addr'". The sibling
 * first_*_text / first_*_rodata anchors survive only because they are
 * non-static, so they cannot be elided. Keep `used` here. */
#else
__attribute__ ((section(".init.text"), optimize("-O0"), unused))
#endif
static void last_fmp_init(void){};
