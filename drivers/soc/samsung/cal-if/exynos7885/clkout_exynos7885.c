#include "clkout_exynos7885.h"

/*
 * derp-4.9-port: the 4.4 tree's CLKOUT() took an 8th parameter (SEL_ENABLE)
 * and struct cmucal_clkout carried it as `.en`.  The 4.9 tree dropped both,
 * and 4.9 ra.c:ra_enable_clkout() now HARDCODES the active level:
 *     enable -> write 0x0 ; disable -> write 0x1   (i.e. active-LOW)
 * For this SoC that is correct for CLKOUT0 (4.4 SEL_ENABLE was 0, so
 * 4.4 also wrote 0 on enable) but WRONG for CLKOUT1, whose 4.4
 * SEL_ENABLE was 1 (active-HIGH).  CLKOUT1 drives OSCCLK_NFC
 * (see clk-exynos7885.c), so NFC clock gating will be inverted under 4.9
 * unless ra_enable_clkout() regains a per-SoC active level.  A correct fix
 * touches shared cal-if/ra.c + cmucal.h and affects 9810/8895/8890 too,
 * so it is deliberately NOT done here.
 */

#define CLKOUT0_OFFSET		(0xA00)
#define CLKOUT0_SEL_SHIFT	(8)
#define CLKOUT0_SEL_WIDTH	(6)
#define CLKOUT0_SEL_TCXO	(0x1)
#define CLKOUT0_ENABLE_SHIFT	(0)
#define CLKOUT0_ENABLE_WIDTH	(1)

#define CLKOUT1_OFFSET		(0x600C)
#define CLKOUT1_SEL_SHIFT	(8)
#define CLKOUT1_SEL_WIDTH	(6)
#define CLKOUT1_SEL_TCXO	(0x0)
#define CLKOUT1_ENABLE_SHIFT	(31)
#define CLKOUT1_ENABLE_WIDTH	(1)

struct cmucal_clkout cmucal_clkout_list[] = {
	CLKOUT(VCLK_CLKOUT0, CLKOUT0_OFFSET, CLKOUT0_SEL_SHIFT, CLKOUT0_SEL_WIDTH, CLKOUT0_SEL_TCXO, CLKOUT0_ENABLE_SHIFT, CLKOUT0_ENABLE_WIDTH),
	CLKOUT(VCLK_CLKOUT1, CLKOUT1_OFFSET, CLKOUT1_SEL_SHIFT, CLKOUT1_SEL_WIDTH, CLKOUT1_SEL_TCXO, CLKOUT1_ENABLE_SHIFT, CLKOUT1_ENABLE_WIDTH),
};

unsigned int cmucal_clkout_size = 2;
