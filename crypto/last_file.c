#include <linux/compiler.h>

__attribute__ ((section(".rodata"), unused))
const unsigned char last_crypto_rodata = 0x20;

__attribute__ ((section(".text"), unused))
void last_crypto_text(void){}

__attribute__ ((section(".init.text"), used, __optimize("-O0"), unused))
/* `used` is load-bearing: these empty static functions are the FIPS
 * integrity boundary markers that scripts/crypto/fips_crypto_integrity.py
 * and scripts/fmp/fips_fmp_integrity.py look up by name to compute the HMAC
 * range. Without `used` the compiler drops the empty function and the whole
 * .init.text section with it, and the script dies with
 * "AttributeError: 'NoneType' object has no attribute 'addr'". The sibling
 * first_*_text / first_*_rodata anchors survive only because they are
 * non-static, so they cannot be elided. Keep `used` here. */
static void last_crypto_init(void){};
