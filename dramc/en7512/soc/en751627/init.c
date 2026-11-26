/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Package/revision-specific PLL and DDR3 setup from EN7516 V1.0. */
#include "dramc_common.h"

int en7512_dramc_init(void)
{
	u32 xtal, straps, strap_reg;

	prom_puts(PKG_type == QFP ? "QFP IC\n" : "BGA IC\n");
	strap_reg = REG32(SYS_GLOBAL_PARM) & BIT(22) ? 0xbfa2ff28u : 0xbfa20174u;
	xtal = (REG32(strap_reg) >> 21) & 1u;
	straps = REG32(strap_reg);
	prom_puts("Xtal:");
	prom_print_dec(xtal);
	prom_puts("\nDDR3 init.\n");
	REG32(0xbfb00040u) = 0;
	pause_polling(200);

	if (PKG_type == QFP) {
		DRAMC_REG(DRAMC_MISCTL0) = 0x07110000u;
		DRAMC_REG(DRAMC_DRVCTL0) = 0xcc00cc00u;
		DRAMC_REG(DRAMC_DRVCTL1) = 0xcc00cc00u;
		DRAMC_REG(DRAMC_DDR2CTL) = 0xb48711edu;
	} else {
		DRAMC_REG(DRAMC_MISCTL0) = 0x07010000u;
		DRAMC_REG(DRAMC_DRVCTL0) = 0xaa33aa33u;
		DRAMC_REG(DRAMC_DRVCTL1) = 0xaa33aa33u;
		DRAMC_REG(DRAMC_DDR2CTL) = 0xc38722edu;
	}

	if ((REG32(SYS_REVISION) & 0xffffu) == 1u) {
		DRAMC_REG(DRAMC_MEMPLL_DIVIDER) = (DRAMC_REG(DRAMC_MEMPLL_DIVIDER) &
						   (PKG_type == QFP ? 0xfffffdcfu : 0xfffffdc8u)) |
						  0x220u;
		DRAMC_REG(DRAMC_MEMPLL1) = (DRAMC_REG(DRAMC_MEMPLL1) & 0xffffffe1u) | 0x12u;
		DRAMC_REG(DRAMC_MEMPLL0) =
			(DRAMC_REG(DRAMC_MEMPLL0) & 0xfffc0808u) | (xtal == 0 ? 0x181u : 0x131u);
		pause_polling(1);
		DRAMC_REG(DRAMC_MEMPLL0) |= 0x02000000u;
		pause_polling(20);
		dram_mempll_finish(PKG_type == QFP ? 0x088au : 0x08cau, 0x02800000u);
	} else if (PKG_type == QFP) {
		DRAMC_REG(DRAMC_MEMPLL_DIVIDER) =
			(DRAMC_REG(DRAMC_MEMPLL_DIVIDER) & 0xfffffdcfu) | 0x220u;
		DRAMC_REG(DRAMC_MEMPLL1) = (DRAMC_REG(DRAMC_MEMPLL1) & 0xffffffe1u) | 0x12u;
		DRAMC_REG(DRAMC_MEMPLL0) = (DRAMC_REG(DRAMC_MEMPLL0) & 0xfbfc0808u) | 0x04000135u;
		DRAMC_REG(DRAMC_MEMPLL2) = (DRAMC_REG(DRAMC_MEMPLL2) & 0x0001ffffu) | 0x2b020000u;
		DRAMC_REG(DRAMC_MEMPLL3) = (DRAMC_REG(DRAMC_MEMPLL3) & 0xffff0000u) | 0x0687u;
		pause_polling(1);
		DRAMC_REG(DRAMC_MEMPLL0) |= 0x02000000u;
		pause_polling(20);
		DRAMC_REG(DRAMC_MEMPLL3) |= 0x00080000u;
		pause_polling(1);
		DRAMC_REG(DRAMC_MEMPLL3) |= 0x00010000u;
		pause_polling(20);
		dram_mempll_finish(0x08aau, 0x04000000u);
	} else {
		DRAMC_REG(DRAMC_MEMPLL_DIVIDER) =
			(DRAMC_REG(DRAMC_MEMPLL_DIVIDER) & 0xfffffdc8u) | 0x220u;
		DRAMC_REG(DRAMC_MEMPLL1) = (DRAMC_REG(DRAMC_MEMPLL1) & 0xffffffe1u) | 0x12u;
		DRAMC_REG(DRAMC_MEMPLL0) = (DRAMC_REG(DRAMC_MEMPLL0) & 0xfbfc0808u) | 0x04000135u;
		if (xtal == 0) {
			DRAMC_REG(DRAMC_MEMPLL2) =
				(DRAMC_REG(DRAMC_MEMPLL2) & 0x0001ffffu) | 0x3d700000u;
			DRAMC_REG(DRAMC_MEMPLL3) =
				(DRAMC_REG(DRAMC_MEMPLL3) & 0xffff0000u) | 0x0b54u;
		} else {
			DRAMC_REG(DRAMC_MEMPLL2) =
				(DRAMC_REG(DRAMC_MEMPLL2) & 0x0001ffffu) | 0x7df20000u;
			DRAMC_REG(DRAMC_MEMPLL3) =
				(DRAMC_REG(DRAMC_MEMPLL3) & 0xffff0000u) | 0x08a9u;
		}
		pause_polling(1);
		DRAMC_REG(DRAMC_MEMPLL0) |= 0x02000000u;
		pause_polling(20);
		DRAMC_REG(DRAMC_MEMPLL3) |= 0x00080000u;
		pause_polling(1);
		DRAMC_REG(DRAMC_MEMPLL3) |= 0x00010000u;
		pause_polling(20);
		dram_mempll_finish(0x08aau, 0x04000000u);
	}

	DRAMC_REG(DRAMC_TEST2_4) = 0x1e00d10du;
	if (PKG_type == QFP)
		DRAMC_REG(DRAMC_MCKDLY) = 0xc0100900u;
	else if (straps & BIT(4))
		DRAMC_REG(DRAMC_MCKDLY) = 0x40100900u;
	else
		DRAMC_REG(DRAMC_MCKDLY) = 0x80100900u;
	pause_polling(200);
	DRAMC_REG(DRAMC_PADCTL4) = 0xa3u;
	pause_polling(500);
	DRAMC_REG(DRAMC_CLK1DELAY) = 1u;
	DRAMC_REG(DRAMC_IOCTL) = 0u;
	DRAMC_REG(DRAMC_R0DQSIEN) = 0x10101010u;
	DRAMC_REG(DRAMC_DQSCTL0) = 0x83080080u;
	DRAMC_REG(DRAMC_DQSCTL1) = 0x12080080u;
	if (PKG_type == QFP) {
		DRAMC_REG(DRAMC_PHYCTL1) = 0x80000000u;
		DRAMC_REG(DRAMC_GDDR3CTL1) = 0x01000000u;
	} else {
		DRAMC_REG(DRAMC_PHYCTL1) = 0u;
		DRAMC_REG(DRAMC_GDDR3CTL1) = 0x11000000u;
	}
	DRAMC_REG(DRAMC_ARBCTL0) = 0x80u;
	DRAMC_REG(DRAMC_CLKENCTL) = 0x30000000u;
	if (PKG_type == QFP)
		DRAMC_REG(DRAMC_MCKDLY) = 0xc0300900u;
	else if (straps & BIT(4))
		DRAMC_REG(DRAMC_MCKDLY) = 0x40300900u;
	else
		DRAMC_REG(DRAMC_MCKDLY) = 0x80300900u;
	pause_polling(1);
	DRAMC_WRITE_REG(PKG_type == QFP ? 0xf0740642u : 0xf0740682u, DRAMC_CONF1);
	DRAMC_REG(DRAMC_DQSGCTL) = 0x80000011u;
	DRAMC_REG(DRAMC_R0DQSIEN) = 0x10101010u;
	DRAMC_REG(DRAMC_DQSCAL0) = 0xc8b8u;
	DRAMC_REG(DRAMC_DDR2CTL) = PKG_type == QFP ? 0xb48711e5u : 0xc38722e5u;
	DRAMC_REG(DRAMC_DLLCONF) = 0xf1200f01u;
	DRAMC_REG(DRAMC_LPDDR2) = 0x88000000u;
	DRAMC_REG(0x158) = 0u;
	DRAMC_REG(DRAMC_RKCFG) = 0x00111190u;
	DRAMC_REG(DRAMC_CONF1) = PKG_type == QFP ? 0xf0740642u : 0xf0740682u;
	DRAMC_REG(DRAMC_PADCTL4) = 0xa7u;
	pause_polling(1);

	if (dram_mode_command(PKG_type == QFP ? 0x4208u : 0x4210u, 1u, 1u) ||
	    dram_mode_command(0x6000u, 1u, 1u) ||
	    dram_mode_command(PKG_type == QFP ? 0x2006u : 0x2004u, 1u, 1u) ||
	    dram_mode_command(PKG_type == QFP ? 0x1931u : 0x1b51u, 1u, 1u))
		goto failed;
	if (PKG_type != KGD && dram_mode_command(0x400u, 0x10u, 6u))
		goto failed;

	DRAMC_WRITE_REG(0x1100u, DRAMC_SPCMD);
	pause_polling(1);
	DRAMC_WRITE_REG(0x22a7u, DRAMC_PADCTL4);
	DRAMC_WRITE_REG(0x88000000u, DRAMC_LPDDR2);
	if (dram_mode_command(0xffffu, 0x20u, 1u))
		goto failed;

	DRAMC_WRITE_REG(0x104d2842u, DRAMC_PD_CTRL);
	DRAMC_WRITE_REG(PKG_type == QFP ? 0xf0740642u : 0xf0740682u, DRAMC_CONF1);
	DRAMC_WRITE_REG(PKG_type == QFP ? 0xb48711edu : 0xc38722edu, DRAMC_DDR2CTL);
	pause_polling(1);
	DRAMC_WRITE_REG(0u, DRAMC_PADCTL1);
	DRAMC_WRITE_REG(PKG_type == QFP ? 0x33684552u : 0x447a4694u, DRAMC_ACTIM0);
	pause_polling(1);
	DRAMC_WRITE_REG(PKG_type == QFP ? 0xa8830481u : 0xa88a0481u, DRAMC_TEST2_3);
	DRAMC_WRITE_REG(PKG_type == QFP ? 0x650u : 0x660u, DRAMC_ACTIM1);
	DRAMC_WRITE_REG(PKG_type == QFP ? 0xf5f3fu : 0xf5f50u, DRAMC_CONF2);
	DRAMC_WRITE_REG(0u, DRAMC_PADCTL2);
	DRAMC_WRITE_REG(0xedcb000fu, DRAMC_PADCTL7);
	DRAMC_WRITE_REG(0x00c80008u, DRAMC_DM_MONITOR);
	DRAMC_WRITE_REG(0x31313131u, DRAMC_R0DELDLY);
	REG32(0xbfb00074u) = 0x105u;
	pause_polling(2);
	prom_puts("DRAMC init done. \n");
	return 0;
failed:
	prom_puts("DRAMC Init Fail!\r\n");
	return -1;
}
