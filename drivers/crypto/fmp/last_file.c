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

__attribute__ ((section(".init.text"), used, __optimize("-O0"), unused))
/* `used` is load-bearing: these empty static functions are the FIPS
 * integrity boundary markers that scripts/crypto/fips_crypto_integrity.py
 * and scripts/fmp/fips_fmp_integrity.py look up by name to compute the HMAC
 * range. Without `used` the compiler drops the empty function and the whole
 * .init.text section with it, and the script dies with
 * "AttributeError: 'NoneType' object has no attribute 'addr'". The sibling
 * first_*_text / first_*_rodata anchors survive only because they are
 * non-static, so they cannot be elided. Keep `used` here. */
static void last_fmp_init(void){};
