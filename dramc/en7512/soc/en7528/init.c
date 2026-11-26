/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Readable EN7528 DRAMC V1.8 initialization (validated QFP + DDR3 path).
 */
#include "dramc_common.h"

#define W(o, v)	DRAMC_WRITE_REG((v), (o))

static u32 en7528_xtal_select(void)
{
	/* V1.8 chooses the strap source through SYS_GLOBAL_PARM bit 9. */
	u32 strap = REG32(SYS_GLOBAL_PARM) & BIT(9);
	u32 addr = strap ? 0xbfa2ff28u : 0xbfa20174u;

	return (REG32(addr) >> 21) & 1u;
}

int en7512_dramc_init(void)
{
	u32 xtal;

	if (PKG_type == KGD)
		prom_puts("KGD IC\n");
	else if (PKG_type == QFP)
		prom_puts("QFP IC\n");
	else
		prom_puts("BGA IC\n");

	xtal = en7528_xtal_select();
	prom_puts("Xtal: ");
	prom_puts(xtal == 1u ? "25Mhz\n" : "20Mhz\n");

	if (PKG_type != QFP || dram_type != DDR3) {
		prom_puts("DRAMC unsupported package/type in full-C SRAM test\r\n");
		return -1;
	}

	prom_puts("DDR3 init.\n");

	REG32(0xbfb00040u) = 0;
	pause_polling(200);

	W(DRAMC_MISCTL0, 0x07010000u);
	W(DRAMC_DRVCTL0, 0xcc00cc00u);
	W(DRAMC_DRVCTL1, 0xcc00cc00u);
	W(DRAMC_DDR2CTL, 0xb28711edu);

	/* QFP V1.8 runs the DRAM PLL at 900 MHz. */
	mempll_init_int(xtal, 900);
	(void)mt_mempll_cali();

	W(DRAMC_TEST2_4, 0x1e00d10du);
	W(DRAMC_MCKDLY, 0xc0100900u);
	pause_polling(200);

	W(DRAMC_PADCTL4, 0x000000a3u);
	pause_polling(500);
	W(DRAMC_CLK1DELAY, 0x00000001u);
	W(DRAMC_IOCTL, 0x00000000u);
	W(DRAMC_R0DQSIEN, 0x10101010u);
	W(DRAMC_DQSCTL0, 0x83080080u);
	W(DRAMC_DQSCTL1, 0x12080080u);
	W(DRAMC_PHYCTL1, 0x00008000u);
	W(DRAMC_GDDR3CTL1, 0x11000000u);
	W(DRAMC_ARBCTL0, 0x00000080u);
	W(DRAMC_CLKENCTL, 0x30000000u);
	W(DRAMC_MCKDLY, 0xc0300900u);
	pause_polling(1);

	W(DRAMC_CONF1, 0xf0740642u);
	W(DRAMC_DQSGCTL, 0x80000011u);
	W(DRAMC_R0DQSIEN, 0x10101010u);
	W(DRAMC_DQSCAL0, 0x0000c8b8u);
	W(DRAMC_DDR2CTL, 0xb28711e5u);
	W(DRAMC_DLLCONF, 0xf1200f01u);
	W(DRAMC_LPDDR2, 0x88000000u);
	W(0x158, 0x00000000u);
	W(DRAMC_RKCFG, 0x00111190u);
	W(DRAMC_CONF1, 0xf0740642u);
	W(DRAMC_PADCTL4, 0x000000a7u);
	pause_polling(1);

	/* DDR3 mode-register initialization, QFP path. */
	if (dram_mode_command(0x00004208u, 0x00000001u, 1u) ||
	    dram_mode_command(0x00006000u, 0x00000001u, 1u) ||
	    dram_mode_command(0x00002044u, 0x00000001u, 1u) ||
	    dram_mode_command(0x00001931u, 0x00000001u, 1u) ||
	    dram_mode_command(0x00000400u, 0x00000010u, 6u))
		goto failed;

	/* Vendor writes 0x1100 directly here; it is not polled. */
	W(DRAMC_SPCMD, 0x00001100u);
	pause_polling(1);

	W(DRAMC_PADCTL4, 0x000022a7u);
	W(DRAMC_LPDDR2, 0x88000000u);
	if (dram_mode_command(0x0000ffffu, 0x00000020u, 1u))
		goto failed;

	W(DRAMC_PD_CTRL, 0x104d2842u);
	W(DRAMC_CONF1, 0xf0740642u);
	W(DRAMC_DDR2CTL, 0xb28711edu);
	pause_polling(1);

	/* V1.8: non-KGD at 900 MHz uses 0x0c000000 here. */
	W(DRAMC_PADCTL1, dram_speed == 900 ? 0x0c000000u : 0x00000000u);
	W(DRAMC_ACTIM0, 0x33684552u);
	pause_polling(1);

	W(DRAMC_TEST2_3, 0xa8830481u);
	W(DRAMC_ACTIM1, 0x00000650u);
	/* V1.8 changed this from the V1.6 value 0x000f5f3f. */
	W(DRAMC_CONF2, 0x000f5f36u);
	W(DRAMC_PADCTL2, 0x00000000u);
	W(DRAMC_PADCTL7, 0xedcb000fu);
	W(DRAMC_DM_MONITOR, 0x00c80008u);
	W(DRAMC_R0DELDLY, 0x31313131u);
	REG32(0xbfb00074u) = 0x105u;

	pause_polling(2);
	prom_puts("DRAMC init done. \n");
	return 0;

failed:
	prom_puts("DRAMC Init Fail!\r\n");
	return -1;
}
