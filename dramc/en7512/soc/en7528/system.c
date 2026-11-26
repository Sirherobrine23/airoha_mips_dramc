/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <version.h>
#include "dramc_common.h"

/*
 * EN7528 DRAMC V1.8 system, eFuse, and stage handoff.
 */

#define CLK_CTRL_A0	0xbfa201a0u
#define CLK_PLL_CFG	0xbfa201ecu
#define CLK_PLL_LF	0xbfa201f8u
#define CLK_PLL_FRAC	0xbfa201fcu
#define SYS_MISC_094C	0xbfb0094cu
#define SYS_MISC_01C4	0xbfa201c4u

int dram_speed;

unsigned int soc_get_hclk_mhz(void)
{
	u32 mhz = (REG32(SYS_GLOBAL_PARM) >> 10) & 0x3ffu;

	return mhz ? mhz : 250u;
}

static void global_set_sysclk(u32 mhz)
{
	u32 v = REG32(SYS_GLOBAL_PARM);

	v &= ~(0x3ffu << 10);
	v |= (mhz & 0x3ffu) << 10;
	REG32(SYS_GLOBAL_PARM) = v;
}

static void global_set_package(u32 package)
{
	u32 v = REG32(SYS_GLOBAL_PARM);

	v &= ~0xfu;
	v |= package & 0xfu;
	REG32(SYS_GLOBAL_PARM) = v;
}

static void global_set_flag(unsigned int bit, int set)
{
	u32 v = REG32(SYS_GLOBAL_PARM);

	v &= ~BIT(bit);
	if (set)
		v |= BIT(bit);
	REG32(SYS_GLOBAL_PARM) = v;
}

static void en7528_setup_clk(u32 cpu_mhz)
{
	u32 pll_cfg = 0;
	u32 pll_frac = 0;

	/* 900 MHz is the strap/default path; the PLL is already running. */
	if (cpu_mhz == 900) {
		global_set_sysclk(225);
		return;
	}

	REG32(CLK_CTRL_A0) = 0x12u;

	switch (cpu_mhz) {
	case 1100:
		pll_cfg = 0x04001508u;
		pll_frac = 0x04800000u;
		break;
	case 1050:
		pll_cfg = 0x04001408u;
		pll_frac = 0x04400000u;
		break;
	case 1000:
		pll_cfg = 0x05102708u;
		pll_frac = 0x04000000u;
		break;
	case 975:
		pll_cfg = 0x05102608u;
		pll_frac = 0x03e00000u;
		break;
	case 950:
		pll_cfg = 0x05102508u;
		pll_frac = 0x03c00000u;
		break;
	case 925:
		pll_cfg = 0x05102408u;
		pll_frac = 0x03a00000u;
		break;
	case 800:
		pll_cfg = 0x05101f08u;
		pll_frac = 0x03000000u;
		break;
	case 700:
		pll_cfg = 0x05101b08u;
		pll_frac = 0x02800000u;
		break;
	case 500:
		pll_cfg = 0x04101308u;
		pll_frac = 0x04000000u;
		break;
	case 450:
		pll_cfg = 0x05201108u;
		pll_frac = 0x03800000u;
		break;
	case 225:
		pll_cfg = 0x05301108u;
		pll_frac = 0x03800000u;
		break;
	default:
		global_set_sysclk(225);
		REG32(CLK_CTRL_A0) = 0;
		return;
	}

	REG32(CLK_PLL_CFG) = pll_cfg;
	REG32(CLK_PLL_FRAC) = pll_frac;
	REG32(CLK_PLL_LF) = 0x402u;
	REG32(CLK_PLL_LF) = 0x406u;
	REG32(CLK_CTRL_A0) = 0;

	/* V1.8 stores CPU/4 in SYS_GLOBAL_PARM.sysclk. */
	global_set_sysclk(cpu_mhz >> 2);
}

static void set_spi_ctrler_ecc(void)
{
	u32 chip = REG32(SYS_CHIP_ID) & 0xffff0000u;

	if (chip == 0x00070000u)
		global_set_flag(8, 0);

	/* SYS_BOOT_CFG[4] selects whether controller ECC is exposed. */
	global_set_flag(8, !(REG32(SYS_BOOT_CFG) & BIT(4)));
}

static int efuse_wait(u32 status, unsigned int bit)
{
	int retries = 3;

	do {
		pause_polling(1000);
		if (REG32(status) & BIT(bit))
			return 0;
	} while (--retries);

	return -1;
}

static int use_500mhz_clock(void)
{
	u32 chip = REG32(SYS_CHIP_ID) & 0xffff0000u;
	u32 e0, e1;

	if (chip != 0x00090000u)
		return 0;

	e0 = REG32(EFUSE0_DATA0);
	e1 = REG32(EFUSE0_DATA1);

	if (e0 & BIT(0)) {
		if (((e0 >> 2) & 0x000c0000u) != 0x000c0000u)
			return 0;
		return !!((e1 >> 9) & 1u);
	}

	if ((e0 & 0x000c0000u) != 0x000c0000u)
		return 0;

	return !!(e1 & BIT(8));
}

static void init_system(int is_bootext)
{
	u32 e0, e3, package;

	/* SYS_GLOBAL_PARM.isFpga = !HWCONF[0]. */
	global_set_flag(9, !(REG32(SYS_HWCONF) & BIT(0)));
	set_spi_ctrler_ecc();

	if (REG32(SYS_GLOBAL_PARM) & BIT(9)) {
		global_set_sysclk(64);
		return;
	}

	if (use_500mhz_clock()) {
		en7528_setup_clk(500);
	} else if (is_bootext) {
		en7528_setup_clk(900);
	} else if (REG32(SYS_HWTRAP) & 3u) {
		en7528_setup_clk(450);
	} else {
		en7528_setup_clk(900);
	}

	REG32(SYS_MISC_094C) &= ~1u;
	REG32(SYS_MISC_01C4) = (REG32(SYS_MISC_01C4) & ~0xeu) | 8u;

	/* The EN7528 tcboot header/build selects EFUSE_CLK_WIDTH=3. */
	REG32(EFUSE0_CLK_WIDTH) = (REG32(EFUSE0_CLK_WIDTH) & ~7u) | 3u;
	REG32(EFUSE1_CLK_WIDTH) = (REG32(EFUSE1_CLK_WIDTH) & ~7u) | 3u;

	time_polling_init();

	/* Enable Fsource/read path for the second macro. */
	REG32(EFUSE0_CTRL) = 0;
	REG32(EFUSE1_CTRL) = 8u;
	if (efuse_wait(EFUSE1_STATUS, 2) < 0)
		prom_puts("\nError: Fsource read timeout\n");

	/* Read first macro. */
	REG32(EFUSE0_CTRL) = 5u;
	if (efuse_wait(EFUSE0_STATUS, 1) < 0)
		prom_puts("\nError: EFUSE 1 read timeout\n");

	/* Read second macro. */
	REG32(EFUSE1_CTRL) = 13u;
	if (efuse_wait(EFUSE1_STATUS, 1) < 0)
		prom_puts("\nError: EFUSE 2 read timeout\n");

	e0 = REG32(EFUSE0_DATA0);
	e3 = REG32(EFUSE1_DATA3);

	if (!(e0 & BIT(0)))
		package = ((e0 >> 1) & 3u) | ((e3 >> 8) & 4u);
	else
		package = ((e0 >> 3) & 3u) | ((e3 >> 9) & 4u);

	global_set_package(package);
}

void soc_dramc_banner(void)
{
	prom_puts("EN7528 DRAMC v1.8 - " PLAIN_VERSION "\r\n\r\n");
}

void soc_detect_dram_pkg(void)
{
	if (en7528_is_qfp()) {
		PKG_type = QFP;
		dram_type = DDR3;
	} else {
		PKG_type = KGD;
		dram_type = en7528_is_ddr3() ? DDR3 : DDR2;
	}
}

void soc_post_calib_geometry(void)
{
	check_column_bank();

	if (PKG_type == KGD && en7528_half_size()) {
		DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_CONF1) & ~0x300u) | 0x100u, DRAMC_CONF1);
		REG32(0xbfb00074u) = 0x104u;
	}
}

u32 soc_effective_dram_size(u32 size)
{
	return size << en7528_half_size();
}

void soc_finish_dramc(int calib_failed)
{
	u32 v = REG32(SYS_GLOBAL_PARM);

	/* V1.8 stores the size in units of 16 MiB in bits [27:20]. */
	v &= ~(0xffu << 20);
	v |= (((u32)dram_size >> 4) & 0xffu) << 20;
	REG32(SYS_GLOBAL_PARM) = v;

	/* V1.8 marks 512-MiB geometry in DRAMC register 0x7c. */
	if (dram_size == 512)
		DRAMC_WRITE_SET(BIT(2), DRAMC_DDR2CTL);

	set_TRFC();

	prom_puts("DDR Speed: ");
	prom_print_dec((u32)dram_speed);
	prom_puts("Mhz\n");

	prom_puts("\r\ncalibration status: ");
	prom_print_dec((u32)calib_failed);
	prom_puts("\r\n");

	DRAMC_WRITE_SET(0x710u, DRAMC_PERFCTL0);
}

int start_spram(int is_bootext)
{
	void (*next_stage)(void);

	uart_init();
	init_system(0);
	time_polling_init();

	if (is_bootext == 1)
		dramc_main();

	next_stage = (void (*)(void))(unsigned long)REG32(SYS_BOOT_JUMP);
	next_stage();
	return 0;
}
