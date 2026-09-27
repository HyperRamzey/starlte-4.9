#include "clkout_exynos7885.h"

/*
 * derp-4.9-port: per-SoC CLKOUT enable active level.
 *
 * 4.4's CLKOUT() took an 8th argument, SEL_ENABLE, stored as `.en`, and
 * ra.c:ra_enable_clkout() wrote `.en` to enable and `!en` to disable.  4.9
 * dropped the field and hardcoded active-LOW (0x0 on enable, 0x1 on disable).
 * exynos9810 / exynos9810_evt0 / exynos8895 all had SEL_ENABLE = 0, so the
 * hardcode is right for them; cmucal.h's CLKOUT() therefore still defaults
 * .en to 0 and their behaviour is bit-for-bit unchanged.
 *
 * Exynos7885 is the exception, and only for CLKOUT1:
 *   CLKOUT0  offset 0xA00   enable bit 0    SEL_ENABLE 0 -> active-LOW
 *   CLKOUT1  offset 0x600C  enable bit 31   SEL_ENABLE 1 -> ACTIVE-HIGH
 * CLKOUT1 is also the only CLKOUT in the tree with its enable bit at bit 31.
 * It is published as OSCCLK_NFC (clk-exynos7885.c), so under the 4.9 hardcode
 * an enable request would have written 0 (actually disabling the NFC clock) and
 * a disable request 1 (enabling it) -- exactly inverted.
 *
 * So this table uses the 8-argument CLKOUT_EN() and states both levels.
 *
 * Note for the NFC path specifically: on this device nothing calls
 * clk_prepare_enable() on OSCCLK_NFC, and the live DT node
 * (hsi2c@13930000/sec-nfc@27, compatible = "sec-nfc") carries no `clocks`
 * property -- drivers/nfc/sec_nfc.c gates the part through the
 * `sec-nfc,nfc_clkreq` GPIO plus a direct `clkctrl-reg` PMU poke, not through
 * CCF.  So the inversion was latent rather than active, but it is a landmine
 * for any future CCF consumer, hence it is fixed here rather than documented.
 */

#define CLKOUT0_OFFSET		(0xA00)
#define CLKOUT0_SEL_SHIFT	(8)
#define CLKOUT0_SEL_WIDTH	(6)
#define CLKOUT0_SEL_TCXO	(0x1)
#define CLKOUT0_ENABLE_SHIFT	(0)
#define CLKOUT0_ENABLE_WIDTH	(1)
#define CLKOUT0_SEL_ENABLE	(0)

#define CLKOUT1_OFFSET		(0x600C)
#define CLKOUT1_SEL_SHIFT	(8)
#define CLKOUT1_SEL_WIDTH	(6)
#define CLKOUT1_SEL_TCXO	(0x0)
#define CLKOUT1_ENABLE_SHIFT	(31)
#define CLKOUT1_ENABLE_WIDTH	(1)
#define CLKOUT1_SEL_ENABLE	(1)

struct cmucal_clkout cmucal_clkout_list[] = {
	CLKOUT_EN(VCLK_CLKOUT0, CLKOUT0_OFFSET, CLKOUT0_SEL_SHIFT, CLKOUT0_SEL_WIDTH, CLKOUT0_SEL_TCXO, CLKOUT0_ENABLE_SHIFT, CLKOUT0_ENABLE_WIDTH, CLKOUT0_SEL_ENABLE),
	CLKOUT_EN(VCLK_CLKOUT1, CLKOUT1_OFFSET, CLKOUT1_SEL_SHIFT, CLKOUT1_SEL_WIDTH, CLKOUT1_SEL_TCXO, CLKOUT1_ENABLE_SHIFT, CLKOUT1_ENABLE_WIDTH, CLKOUT1_SEL_ENABLE),
};

unsigned int cmucal_clkout_size = 2;
