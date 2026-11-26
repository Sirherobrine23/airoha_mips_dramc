/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "dramc_common.h"

int PKG_type;
int dram_type;
int dram_size;

int dram_spcmd(u32 command, u32 delay)
{
	u32 timeout = 0xffffu;

	DRAMC_WRITE_REG(command, DRAMC_SPCMD);
	while (!(DRAMC_READ_REG(DRAMC_SPCMDRESP) & command)) {
		if (--timeout == 0)
			return -1;
	}
	delay_a_while(delay);
	DRAMC_WRITE_REG(0, DRAMC_SPCMD);
	return 0;
}

int dram_mode_command(u32 mrs, u32 command, u32 delay)
{
	DRAMC_WRITE_REG(mrs, DRAMC_MRS);
	return dram_spcmd(command, delay);
}

void dram_mempll_finish(u32 feedback, u32 phase)
{
	DRAMC_WRITE_SET(0x100u, DRAMC_MEMPLL1);
	DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_MEMPLL6) & 0xf3cf0000u) | feedback, DRAMC_MEMPLL6);
	DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_MEMPLL9) & 0xf3cf0000u) | feedback, DRAMC_MEMPLL9);
	DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_MEMPLL12) & 0xf3cf0000u) | feedback, DRAMC_MEMPLL12);
	DRAMC_WRITE_REG(0xc0004ab2u, 0x648);
	delay_a_while(1);
	DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_MEMPLL5) & 0xf01fffffu) | phase, DRAMC_MEMPLL5);
	DRAMC_WRITE_SET(0x02000000u, DRAMC_MEMPLL6);
	DRAMC_WRITE_SET(0x02000000u, DRAMC_MEMPLL9);
	DRAMC_WRITE_SET(0x02000000u, DRAMC_MEMPLL12);
	delay_a_while(20);
	DRAMC_WRITE_SET(0x10u, DRAMC_MEMPLL_DIVIDER);
	delay_a_while(1);
}

static void print_probe_error(u32 address, u32 value)
{
	prom_puts("addr=0x");
	prom_print_hex(address, 8);
	prom_puts("\n");
	prom_print_hex(value, 8);
	prom_puts("\n");
}

static int probe_pair(u32 base, u32 offset, u32 *value)
{
	u32 other;

	ADDR_WRITE_REG(0x12345678u, base);
	delay_a_while(2);
	ADDR_WRITE_REG(0x87654321u, base + offset);
	delay_a_while(2);
	do {
		other = ADDR_READ_REG(base + offset);
		*value = ADDR_READ_REG(base);
		if (other != 0x87654321u)
			ADDR_WRITE_REG(0x87654321u, base + offset);
	} while (other != 0x87654321u);

	return (*value == 0x12345678u || *value == 0x87654321u) ? 0 : -1;
}

int check_column_bank(void)
{
	u32 col_bits, value;

	if (dram_type == DDR3)
		return 0;

	for (col_bits = 8; col_bits < 10; col_bits++) {
		DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_CONF1) & ~0x300u) | ((col_bits - 7u) << 8),
				DRAMC_CONF1);
		delay_a_while(2);
		if (probe_pair(DRAM_BASE_ADDR, BIT(col_bits + 1), &value)) {
			prom_puts("Checking col num. dram r/w error!\n");
			print_probe_error(DRAM_BASE_ADDR, value);
			return -1;
		}
		if (value == 0x87654321u)
			break;
	}

	DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_CONF1) & ~0x300u) | ((col_bits - 8u) << 8),
			DRAMC_CONF1);
	delay_a_while(2);

#ifndef DRAMC_ALWAYS_CHECK_BANK
	if (col_bits != 10)
		return 0;
#endif

	REG32(0xbfb00074u) = (REG32(0xbfb00074u) & ~3u) | 1u;
	delay_a_while(2);
	DRAMC_WRITE_SET(BIT(24), DRAMC_GDDR3CTL1);
	delay_a_while(2);

	if (probe_pair(DRAM_BASE_ADDR, 0x2000u, &value)) {
		prom_puts("Checking bank num. dram r/w error!\n");
		print_probe_error(DRAM_BASE_ADDR, value);
		return -1;
	}

	if (value == 0x12345678u)
		DRAMC_WRITE_SET(BIT(24), DRAMC_GDDR3CTL1);
	else
		DRAMC_WRITE_CLEAR(BIT(24), DRAMC_GDDR3CTL1);
	delay_a_while(2);
	return 0;
}

static u32 detect_dram_size_mib(void)
{
	u32 size, value;

	prom_puts("Calculate size.\n");

	for (size = 32u << 20; size < (DRAM_MAX_SIZE_MIB << 20); size <<= 1) {
		if (probe_pair(DRAM_PROBE_BASE, size, &value)) {
			prom_puts("dram r/w error!\n");
			print_probe_error(DRAM_PROBE_BASE, value);
			return 0;
		}
		if (value == 0x87654321u) {
			prom_puts("DRAM size=");
			prom_print_dec(size >> 20);
			prom_puts("MB\n");
			return size >> 20;
		}
	}

	if (DRAM_MAX_SIZE_MIB == 1024u)
		prom_puts("DRAM size=1GB\n");
	else
		prom_puts("DRAM size=512MB(supported max size)\n");
	return DRAM_MAX_SIZE_MIB;
}

struct trfc_entry {
	u32 size_mib;
	u32 test2_3;
	u32 actim1;
};

static void apply_trfc_table(const struct trfc_entry *tbl, unsigned int count, u32 size_mib)
{
	unsigned int i;

	for (i = 0; i < count; i++) {
		if (tbl[i].size_mib == size_mib) {
			DRAMC_WRITE_REG(tbl[i].test2_3, DRAMC_TEST2_3);
			DRAMC_WRITE_REG(tbl[i].actim1, DRAMC_ACTIM1);
			return;
		}
	}
}

void set_TRFC(void)
{
	static const struct trfc_entry ddr2_tbl[] = {
		{ 32, 0xa80b0481u, 0x600u },  { 64, 0xa8030481u, 0x610u },
		{ 128, 0xa8080481u, 0x610u }, { 256, 0xa80b0481u, 0x620u },
		{ 512, 0xa80d0481u, 0x640u },
	};
	static const struct trfc_entry ddr3_qfp_tbl[] = {
		{ 64, 0xa88d0481u, 0x600u },
		{ 128, 0xa8830481u, 0x610u },
		{ 256, 0xa8800481u, 0x620u },
		{ 512, 0xa8850481u, 0x640u },
	};
	static const struct trfc_entry ddr3_bga_tbl[] = {
		{ 64, 0xa8830481u, 0x610u },
		{ 128, 0xa88a0481u, 0x610u },
		{ 256, 0xa88b0481u, 0x620u },
		{ 512, 0xa8890481u, 0x650u },
	};
	u32 eff_size = soc_effective_dram_size((u32)dram_size);

	if (PKG_type == QFP) {
		apply_trfc_table(ddr3_qfp_tbl, 4, eff_size);
	} else if (dram_type == DDR2) {
		apply_trfc_table(ddr2_tbl, 5, eff_size);
	} else {
#ifdef DRAMC_TRFC_USE_QFP_FOR_DDR3
		apply_trfc_table(ddr3_qfp_tbl, 4, eff_size);
#else
		apply_trfc_table(ddr3_bga_tbl, 4, eff_size);
#endif
	}

	prom_puts("Set new TRFC.\n");
}

void dramc_reg_dump(void)
{
	u32 off;

	for (off = 0; off < 0x650u; off += 4) {
		if (off == 0x400u)
			off = 0x600u;
		prom_puts("0x");
		prom_print_hex(off, 3);
		prom_puts(":0x");
		prom_print_hex(DRAMC_READ_REG(off), 8);
		prom_puts("\n");
	}
}

int dramc_calib(void)
{
	int ret;

	ret = do_dqs_gw_calib_1();
	if (ret < 0)
		return ret;

	dle_factor_handler(8);

	ret = do_sw_rx_dq_dqs_calib();
	if (ret < 0)
		return ret;

	ret = do_dle_calib();
	if (ret < 0)
		return ret;

	return do_sw_tx_dq_dqs_calib();
}

int dramc_main(void)
{
	int failed = 0;
	u32 off;

	soc_dramc_banner();
	soc_detect_dram_pkg();

	delay_a_while(200);
	if (en7512_dramc_init() < 0)
		return 0;

	if (dramc_calib() < 0) {
		prom_puts("%dqs_gw (coarse/fine): ");
		prom_print_dec(opt_gw_coarse_value);
		prom_puts("/");
		prom_print_dec(opt_gw_fine_value);
		prom_puts("\n%DQS input dly:\n");
		prom_print_hex(DRAMC_READ_REG(DRAMC_R0DELDLY), 8);
		prom_puts("\n%DQ input dly:\n");
		for (off = DRAMC_DQIDLY1; off < DRAMC_DQIDLY1 + 0x10u; off += 4) {
			prom_print_hex(DRAMC_READ_REG(off), 8);
			prom_puts("\n");
		}
		prom_puts("%dle: ");
		prom_print_dec(opt_dle_value);
		prom_puts("\n%DRAMC calibration fail\n\r");
		REG32(0xbfb00040u) |= BIT(31);
		failed = 1;
	}

	soc_post_calib_geometry();
	dram_size = (int)detect_dram_size_mib();
	soc_finish_dramc(failed);
	return 0;
}
