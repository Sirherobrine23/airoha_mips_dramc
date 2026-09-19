/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Readable EN7528 DRAMC V1.8 initialization.
 *
 * This first no-reconstructed SRAM validation implements the exact path used
 * by the TP-Link XC220-G3 v1 / EN7528HU hardware under test: QFP + DDR3.
 * The sequence and constants are reconstructed from the V1.8
 * en7512_dramc_init.o shipped in the XC220-G3 GPL release.
 */
#include "en7528_ddr.h"

extern int PKG_type;
extern int dram_type;
extern int dram_speed;

extern void mempll_init_int(u32 xtal, u32 speed);
extern void mempll_init_ddr1066(u32 xtal);
extern void mempll_init_ddr1333(u32 xtal);
extern int mt_mempll_cali(void);

#define W(o, v) DRAMC_WRITE_REG((v), (o))
#define R(o)    DRAMC_READ_REG(o)

static int wait_spcmd(u32 mask)
{
	u32 timeout = 0xffff;

	while (!(R(DRAMC_SPCMDRESP) & mask)) {
		if (!--timeout)
			return -1;
	}

	return 0;
}

static int dram_command(u32 command, u32 delay)
{
	W(DRAMC_SPCMD, command);
	if (wait_spcmd(command))
		return -1;
	pause_polling(delay);
	W(DRAMC_SPCMD, 0);
	return 0;
}

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
	prom_puts("Xtal:");
	prom_print_dec(xtal);
	prom_puts("\n");

	/*
	 * Keep this initial all-C validation restricted to the exact hardware
	 * path that has been measured on the XC220-G3.  KGD/BGA and DDR2 are
	 * added after this path has passed independently in SRAM.
	 */
	if (PKG_type != QFP || dram_type != DDR3) {
		prom_puts("DRAMC unsupported package/type in full-C SRAM test\r\n");
		return -1;
	}

	prom_puts("DDR3 init.\n");

	REG32(0xbfb00040u) = 0;
	pause_polling(200);

	W(DRAMC_MISCTL0, 0x07010000);
	W(DRAMC_DRVCTL0, 0xcc00cc00);
	W(DRAMC_DRVCTL1, 0xcc00cc00);
	W(DRAMC_DDR2CTL, 0xb28711ed);

	/* QFP V1.8 runs the DRAM PLL at 900 MHz. */
	mempll_init_int(xtal, 900);
	(void)mt_mempll_cali();

	W(DRAMC_TEST2_4, 0x1e00d10d);
	W(DRAMC_MCKDLY, 0xc0100900);
	pause_polling(200);

	W(DRAMC_PADCTL4, 0x000000a3);
	pause_polling(500);
	W(DRAMC_CLK1DELAY, 0x00000001);
	W(DRAMC_IOCTL, 0x00000000);
	W(DRAMC_R0DQSIEN, 0x10101010);
	W(DRAMC_DQSCTL0, 0x83080080);
	W(DRAMC_DQSCTL1, 0x12080080);
	W(DRAMC_PHYCTL1, 0x00008000);
	W(DRAMC_GDDR3CTL1, 0x11000000);
	W(DRAMC_ARBCTL0, 0x00000080);
	W(DRAMC_CLKENCTL, 0x30000000);
	W(DRAMC_MCKDLY, 0xc0300900);
	pause_polling(1);

	W(DRAMC_CONF1, 0xf0740642);
	W(DRAMC_DQSGCTL, 0x80000011);
	W(DRAMC_R0DQSIEN, 0x10101010);
	W(DRAMC_DQSCAL0, 0x0000c8b8);
	W(DRAMC_DDR2CTL, 0xb28711e5);
	W(DRAMC_DLLCONF, 0xf1200f01);
	W(DRAMC_LPDDR2, 0x88000000);
	W(0x158, 0x00000000);
	W(DRAMC_RKCFG, 0x00111190);
	W(DRAMC_CONF1, 0xf0740642);
	W(DRAMC_PADCTL4, 0x000000a7);
	pause_polling(1);

	/* DDR3 mode-register initialization, QFP path. */
	W(DRAMC_MRS, 0x00004208);
	if (dram_command(0x00000001, 1))
		goto failed;

	W(DRAMC_MRS, 0x00006000);
	if (dram_command(0x00000001, 1))
		goto failed;

	W(DRAMC_MRS, 0x00002044);
	if (dram_command(0x00000001, 1))
		goto failed;

	W(DRAMC_MRS, 0x00001931);
	if (dram_command(0x00000001, 1))
		goto failed;

	/* ZQ calibration is executed on all non-KGD packages. */
	W(DRAMC_MRS, 0x00000400);
	if (dram_command(0x00000010, 6))
		goto failed;

	/* Vendor writes 0x1100 directly here; it is not polled. */
	W(DRAMC_SPCMD, 0x00001100);
	pause_polling(1);

	W(DRAMC_PADCTL4, 0x000022a7);
	W(DRAMC_LPDDR2, 0x88000000);
	W(DRAMC_MRS, 0x0000ffff);
	if (dram_command(0x00000020, 1))
		goto failed;

	W(DRAMC_PD_CTRL, 0x104d2842);
	W(DRAMC_CONF1, 0xf0740642);
	W(DRAMC_DDR2CTL, 0xb28711ed);
	pause_polling(1);

	/* V1.8: non-KGD at 900 MHz uses 0x0c000000 here. */
	W(DRAMC_PADCTL1, dram_speed == 900 ? 0x0c000000 : 0x00000000);
	W(DRAMC_ACTIM0, 0x33684552);
	pause_polling(1);

	W(DRAMC_TEST2_3, 0xa8830481);
	W(DRAMC_ACTIM1, 0x00000650);
	/* V1.8 changed this from the V1.6 value 0x000f5f3f. */
	W(DRAMC_CONF2, 0x000f5f36);
	W(DRAMC_PADCTL2, 0x00000000);
	W(DRAMC_PADCTL7, 0xedcb000f);
	W(DRAMC_DM_MONITOR, 0x00c80008);
	W(DRAMC_R0DELDLY, 0x31313131);
	REG32(0xbfb00074u) = 0x105;

	pause_polling(2);
	prom_puts("DRAMC init done. \n");
	return 0;

failed:
	prom_puts("DRAMC Init Fail!\r\n");
	return -1;
}
