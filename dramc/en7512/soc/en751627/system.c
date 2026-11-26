/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "dramc_common.h"

unsigned int soc_get_hclk_mhz(void)
{
	return (REG32(SYS_GLOBAL_PARM) & 0x003fffffu) >> 12;
}

void en751627_setup_clk(u32 cpu_mhz)
{
	if (cpu_mhz == 900) {
		REG32(SYS_GLOBAL_PARM) = (REG32(SYS_GLOBAL_PARM) & 0xffc00fffu) | 0xe1000u;
		return;
	}
	REG32(0xbfa201a0u) = 0x12u;
	switch (cpu_mhz) {
	case 800:
		REG32(0xbfa201ecu) = 0x05101f08u;
		REG32(0xbfa201fcu) = 0x03000000u;
		break;
	case 700:
		REG32(0xbfa201ecu) = 0x05101b08u;
		REG32(0xbfa201fcu) = 0x02800000u;
		break;
	case 450:
		REG32(0xbfa201ecu) = 0x05201108u;
		REG32(0xbfa201fcu) = 0x03800000u;
		break;
	case 225:
		REG32(0xbfa201ecu) = 0x05301108u;
		REG32(0xbfa201fcu) = 0x03800000u;
		break;
	default:
		REG32(SYS_GLOBAL_PARM) = (REG32(SYS_GLOBAL_PARM) & 0xffc00fffu) | 0xe1000u;
		REG32(0xbfa201a0u) = 0;
		return;
	}
	REG32(0xbfa201f8u) = 0x402u;
	REG32(0xbfa201f8u) = 0x406u;
	REG32(0xbfa201a0u) = 0;
	REG32(SYS_GLOBAL_PARM) =
		(REG32(SYS_GLOBAL_PARM) & 0xffc00fffu) | (((cpu_mhz & 0xfffu) >> 2) << 12);
}

static void set_spi_ctrler_ecc(void)
{
	u32 strap_reg, shift;
	u32 hir = REG32(SYS_CHIP_ID) & 0xffff0000u;

	if (hir == 0x00070000u)
		REG32(SYS_GLOBAL_PARM) &= ~BIT(23);
	strap_reg = REG32(SYS_GLOBAL_PARM) & BIT(22) ? 0xbfa2ff28u : 0xbfa20174u;
	shift = REG32(SYS_GLOBAL_PARM) & BIT(22) ? 19 : 0;
	if (!(REG32(strap_reg) & BIT(shift)))
		REG32(SYS_GLOBAL_PARM) = (REG32(SYS_GLOBAL_PARM) & ~BIT(23)) | BIT(23);
	else
		REG32(SYS_GLOBAL_PARM) &= ~BIT(23);
}

void init_system(int preserve_clock)
{
	REG32(SYS_GLOBAL_PARM) =
		(REG32(SYS_GLOBAL_PARM) & ~BIT(22)) | (((REG32(0xbfb0009cu) & 1u) ^ 1u) << 22);
	set_spi_ctrler_ecc();
	if (REG32(SYS_GLOBAL_PARM) & BIT(22))
		REG32(SYS_GLOBAL_PARM) = (REG32(SYS_GLOBAL_PARM) & 0xffc00fffu) | 0x28000u;
	else if (!preserve_clock) {
		if ((REG32(SYS_REVISION) & 0xffffu) < 3u)
			en751627_setup_clk(800);
		else if (!(REG32(SYS_HWTRAP) & 3u))
			en751627_setup_clk(900);
		else
			en751627_setup_clk(450);
		REG32(0xbfb0094cu) &= ~1u;
		REG32(0xbfa201c4u) = (REG32(0xbfa201c4u) & 0xfffffff1u) | 8u;
	} else
		REG32(SYS_GLOBAL_PARM) = (REG32(SYS_GLOBAL_PARM) & 0xffc00fffu) | 0xe1000u;
}

static void read_efuse_macro(u32 base)
{
	REG32(base) = 8u;
	while (!(REG32(base + 0xcu) & 4u))
		;
	REG32(base) = 0xdu;
	while (!(REG32(base + 0xcu) & 2u))
		;
	REG32(base) = 0;
}

void soc_dramc_banner(void)
{
}

void soc_detect_dram_pkg(void)
{
	u32 strap_reg;

	dram_type = DDR3;
	strap_reg = REG32(SYS_GLOBAL_PARM) & BIT(22) ? 0xbfa2ff28u : 0xbfa20174u;
	PKG_type = (REG32(strap_reg) & BIT(15)) ? QFP : BGA1;
}

void soc_post_calib_geometry(void)
{
	u32 value = PKG_type == KGD ? REG32(EFUSE0_DATA0) : 0;

	if (PKG_type == KGD && (value & (value & BIT(6) ? BIT(26) : BIT(25))))
		DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_CONF1) & ~0x300u) | 0x100u, DRAMC_CONF1);
	else
		check_column_bank();
}

u32 soc_effective_dram_size(u32 size)
{
	return size;
}

void soc_finish_dramc(int calib_failed)
{
	u32 value;

	REG32(SYS_GLOBAL_PARM) = ((REG32(SYS_GLOBAL_PARM)) & ~(0xffu << 20)) |
				 ((((u32)dram_size >> 4) & 0xffu) << 20);
	set_TRFC();

	value = DRAMC_READ_REG(DRAMC_MEMPLL6) & 0xffu;
	if (value == 0xaau) {
		value = DRAMC_READ_REG(DRAMC_MEMPLL3) & 0xfffu;
		if (value == 0x687u)
			prom_puts("ddr-1066\n");
		else if (value == 0x8a9u)
			prom_puts("ddr-1333\n");
	} else if (value == 0x8au)
		prom_puts("ddr-800\n");
	else if (value == 0xcau)
		prom_puts("ddr-1200\n");
	else if (value == 0x4au)
		prom_puts("ddr-400\n");

	DRAMC_WRITE_SET(BIT(31), DRAMC_DQSCAL0);
	DRAMC_WRITE_SET(0x710u, DRAMC_PERFCTL0);
	prom_puts("\r\n7516DRAMC V1.0 (");
	prom_print_dec((u32)calib_failed);
	prom_puts(")\r\n");
}

void start_spram(int calibrate)
{
	void (*next_stage)(void);

	uart_init();
	init_system(1);
	timerSet(1, 10000, 1, 1, 0);
	read_efuse_macro(EFUSE0_CTRL);
	read_efuse_macro(EFUSE1_CTRL);
	tlb_1g_config();
	if (calibrate == 1)
		dramc_main();
	local_flush_tlb_all();
	next_stage = (void (*)(void))(unsigned long)REG32(SYS_BOOT_JUMP);
	next_stage();
	for (;;)
		;
}
