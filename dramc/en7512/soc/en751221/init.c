/* SPDX-License-Identifier: GPL-2.0-or-later */
/* EN751221 DDR2/DDR3 controller and MEMPLL initialization. */
#include "dramc_common.h"

#define WRITE(offset, value)		DRAMC_WRITE_REG(value, offset)
#define READ(offset)			DRAMC_READ_REG(offset)
#define SET(offset, value)		DRAMC_WRITE_SET(value, offset)
#define UPDATE(offset, mask, value)	WRITE(offset, (READ(offset) & (u32)(mask)) | (u32)(value))
#define DELAY(count)			delay_a_while(count)
#define PACKAGE_WRITE(offset, kgd, bga)                                                            \
	do {                                                                                       \
		if (PKG_type == KGD)                                                               \
			WRITE(offset, kgd);                                                        \
		else                                                                               \
			WRITE(offset, bga);                                                        \
	} while (0)

#define COMMAND(mask, delay)                                                                       \
	do {                                                                                       \
		if (dram_spcmd(mask, delay))                                                       \
			goto failed;                                                               \
	} while (0)

static void en751221_pll_config(u32 xtal)
{
	if (PKG_type == KGD) {
		UPDATE(DRAMC_MEMPLL_DIVIDER, ~0x230u, 0x220u);
		UPDATE(DRAMC_MEMPLL1, ~0x1eu, 0x12u);
		UPDATE(DRAMC_MEMPLL0, 0xfffc0808u, xtal ? 0x131u : 0x181u);
		DELAY(1);
		SET(DRAMC_MEMPLL0, 0x02000000u);
		DELAY(20);
		dram_mempll_finish(0x088au, 0x02800000u);
	} else {
		UPDATE(DRAMC_MEMPLL_DIVIDER, ~0x230u, 0x220u);
		UPDATE(DRAMC_MEMPLL1, ~0x1eu, 0x12u);
		UPDATE(DRAMC_MEMPLL0, 0xfbfc0808u, xtal ? 0x04000135u : 0x04000185u);
		UPDATE(DRAMC_MEMPLL2, 0x1ffffu, 0x2b020000u);
		UPDATE(DRAMC_MEMPLL3, 0xffff0000u, 0x687u);
		DELAY(1);
		SET(DRAMC_MEMPLL0, 0x02000000u);
		DELAY(20);
		SET(DRAMC_MEMPLL3, 0x00080000u);
		DELAY(1);
		SET(DRAMC_MEMPLL3, 0x00010000u);
		DELAY(20);
		dram_mempll_finish(0x08aau, 0x04000000u);
	}
}

int en7512_dramc_init(void)
{
	u32 xtal;

	prom_puts(PKG_type == KGD ? "KGD IC\n" : "BGA IC\n");

	xtal = readl(CR_AHB_HWCONF) & 1u;
	prom_puts("Xtal: ");
	prom_puts(xtal == 1u ? "25Mhz\n" : "20Mhz\n");

	if (dram_type == DDR2) {
		prom_puts("DDR2 init.\n");
		readl(0xbfb00040u) = 0;
		DELAY(200);
		WRITE(DRAMC_MISCTL0, 0x07110000u);
		if (PKG_type == KGD) {
			WRITE(DRAMC_DRVCTL0, 0x88228822u);
			WRITE(DRAMC_DRVCTL1, 0x88228822u);
		}
		PACKAGE_WRITE(DRAMC_DDR2CTL, 0xc2031263u, 0xc203126fu);
		en751221_pll_config(xtal);
		WRITE(DRAMC_TEST2_4, 0xd10du);
		PACKAGE_WRITE(DRAMC_MCKDLY, 0x00100900u, 0xc0100900u);
		WRITE(DRAMC_PADCTL4, 1u);
		WRITE(DRAMC_CLK1DELAY, 1u);
		WRITE(DRAMC_IOCTL, 0u);
		WRITE(DRAMC_R0DQSIEN, 0x10101010u);
		WRITE(DRAMC_DQSCTL0, 0x83080080u);
		WRITE(DRAMC_DQSCTL1, 0x12080080u);
		PACKAGE_WRITE(DRAMC_PHYCTL1, 0u, 0x80000000u);
		WRITE(DRAMC_GDDR3CTL1, 0u);
		WRITE(DRAMC_ARBCTL0, 0x80u);
		WRITE(DRAMC_CLKENCTL, 0x30000000u);
		PACKAGE_WRITE(DRAMC_MCKDLY, 0x00300900u, 0xc0300900u);
		DELAY(1);
		PACKAGE_WRITE(DRAMC_CONF1, 0xf0040442u, 0xf0040482u);
		WRITE(DRAMC_DQSGCTL, 0x80000011u);
		WRITE(DRAMC_R0DQSIEN, 0x10101010u);
		WRITE(DRAMC_DQSCAL0, 0xc8b8u);
		PACKAGE_WRITE(DRAMC_DDR2CTL, 0xc2031263u, 0xc203126fu);
		WRITE(DRAMC_DLLCONF, 0xf1200f01u);
		WRITE(DRAMC_LPDDR2, 0x84000000u);
		WRITE(0x158, 0u);
		WRITE(DRAMC_RKCFG, 0x00111190u);
		PACKAGE_WRITE(DRAMC_CONF1, 0xf0740042u, 0xf0740082u);
		WRITE(DRAMC_PADCTL4, 7u);
		DELAY(1);
		COMMAND(4u, 1u);
		WRITE(DRAMC_MRS, 0x4000u);
		COMMAND(1u, 1u);
		WRITE(DRAMC_MRS, 0x6000u);
		COMMAND(1u, 1u);
		PACKAGE_WRITE(DRAMC_MRS, 0x2002u, 0x2040u);
		COMMAND(1u, 1u);
		PACKAGE_WRITE(DRAMC_MRS, 0x0b63u, 0x0f63u);
		COMMAND(1u, 1u);
		COMMAND(4u, 1u);
		COMMAND(8u, 1u);
		COMMAND(8u, 1u);
		PACKAGE_WRITE(DRAMC_MRS, 0x0a63u, 0x0e63u);
		COMMAND(1u, 1u);
		PACKAGE_WRITE(DRAMC_MRS, 0x2382u, 0x23c0u);
		COMMAND(1u, 1u);
		PACKAGE_WRITE(DRAMC_MRS, 0x2002u, 0x2040u);
		COMMAND(1u, 1u);
		PACKAGE_WRITE(DRAMC_DDR2CTL, 0xc2031263u, 0xc203126fu);
		WRITE(DRAMC_PADCTL4, 0x2205u);
		WRITE(DRAMC_MRS, 0xffffu);
		COMMAND(0x20u, 1u);
		WRITE(DRAMC_PD_CTRL, 0x10606442u);
		PACKAGE_WRITE(DRAMC_CONF1, 0xf0740442u, 0xf0740482u);
		DELAY(1);
		WRITE(DRAMC_PADCTL1, 0u);
		PACKAGE_WRITE(DRAMC_ACTIM0, 0x22174441u, 0x33484484u);
		DELAY(1);
		WRITE(DRAMC_TEST2_3, 0xa80d0481u);
		WRITE(DRAMC_ACTIM1, 0x640u);
		PACKAGE_WRITE(DRAMC_CONF2, 0x000f5f2fu, 0x000f5f3fu);
		WRITE(DRAMC_PADCTL2, 0u);
		WRITE(DRAMC_PADCTL7, 0xedcb000fu);
		WRITE(DRAMC_DM_MONITOR, 0x00c80008u);
		WRITE(DRAMC_R0DELDLY, 0x31313131u);
		readl(0xbfb00074u) = 0x104u;
	} else {
		prom_puts("DDR3 init.\n");
		readl(0xbfb00040u) = 0;
		DELAY(200);
		WRITE(DRAMC_MISCTL0, 0x07110000u);
		if (PKG_type == KGD) {
			WRITE(DRAMC_DRVCTL0, 0x88228822u);
			WRITE(DRAMC_DRVCTL1, 0x88228822u);
		}
		PACKAGE_WRITE(DRAMC_DDR2CTL, 0xa18711e1u, 0xb48711edu);
		en751221_pll_config(xtal);
		WRITE(DRAMC_TEST2_4, 0x1e00d10du);
		PACKAGE_WRITE(DRAMC_MCKDLY, 0x00100900u, 0x40100900u);
		DELAY(200);
		WRITE(DRAMC_PADCTL4, 0xa3u);
		DELAY(500);
		WRITE(DRAMC_CLK1DELAY, 1u);
		WRITE(DRAMC_IOCTL, 0u);
		WRITE(DRAMC_R0DQSIEN, 0x10101010u);
		WRITE(DRAMC_DQSCTL0, 0x83080080u);
		WRITE(DRAMC_DQSCTL1, 0x12080080u);
		PACKAGE_WRITE(DRAMC_PHYCTL1, 0u, 0x80000000u);
		WRITE(DRAMC_GDDR3CTL1, 0x01000000u);
		WRITE(DRAMC_ARBCTL0, 0x80u);
		WRITE(DRAMC_CLKENCTL, 0x30000000u);
		PACKAGE_WRITE(DRAMC_MCKDLY, 0x00300900u, 0x40300900u);
		DELAY(1);
		WRITE(DRAMC_CONF1, 0xf0740642u);
		WRITE(DRAMC_DQSGCTL, 0x80000011u);
		WRITE(DRAMC_R0DQSIEN, 0x10101010u);
		WRITE(DRAMC_DQSCAL0, 0xc8b8u);
		PACKAGE_WRITE(DRAMC_DDR2CTL, 0xa18711e1u, 0xb48711e5u);
		WRITE(DRAMC_DLLCONF, 0xf1200f01u);
		WRITE(DRAMC_LPDDR2, 0x88000000u);
		WRITE(0x158, 0u);
		WRITE(DRAMC_RKCFG, 0x00111190u);
		WRITE(DRAMC_CONF1, 0xf0740642u);
		WRITE(DRAMC_PADCTL4, 0xa7u);
		DELAY(1);
		PACKAGE_WRITE(DRAMC_MRS, 0x4200u, 0x4208u);
		COMMAND(1u, 1u);
		WRITE(DRAMC_MRS, 0x6000u);
		COMMAND(1u, 1u);
		PACKAGE_WRITE(DRAMC_MRS, 0x2000u, 0x2004u);
		COMMAND(1u, 1u);
		PACKAGE_WRITE(DRAMC_MRS, 0x1521u, 0x1931u);
		COMMAND(1u, 1u);
		if (PKG_type != KGD) {
			WRITE(DRAMC_MRS, 0x400u);
			COMMAND(0x10u, 6u);
		}
		WRITE(DRAMC_SPCMD, 0x1100u);
		DELAY(1);
		WRITE(DRAMC_PADCTL4, 0x22a7u);
		WRITE(DRAMC_LPDDR2, 0x88000000u);
		WRITE(DRAMC_MRS, 0xffffu);
		COMMAND(0x20u, 1u);
		WRITE(DRAMC_PD_CTRL, 0x10602842u);
		WRITE(DRAMC_CONF1, 0xf0740642u);
		PACKAGE_WRITE(DRAMC_DDR2CTL, 0xa18711e1u, 0xb48711edu);
		DELAY(1);
		WRITE(DRAMC_PADCTL1, 0u);
		PACKAGE_WRITE(DRAMC_ACTIM0, 0x22274430u, 0x33684552u);
		DELAY(1);
		WRITE(DRAMC_TEST2_3, 0xa8830481u);
		WRITE(DRAMC_ACTIM1, 0x650u);
		PACKAGE_WRITE(DRAMC_CONF2, 0x000f5f2fu, 0x000f5f3fu);
		WRITE(DRAMC_PADCTL2, 0u);
		WRITE(DRAMC_PADCTL7, 0xedcb000fu);
		WRITE(DRAMC_DM_MONITOR, 0x00c80008u);
		WRITE(DRAMC_R0DELDLY, 0x31313131u);
		readl(0xbfb00074u) = 0x105u;
	}
	DELAY(2);
	prom_puts("DRAMC init done. \n");
	return 0;
failed:
	prom_puts("DRAMC Init Fail!\r\n");
	return -1;
}
