/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en7528_ddr.h"

/*
 * Readable EN7528 DRAMC V1.8 core.
 *
 * check_column_bank(), set_TRFC(), dramc_reg_dump() and dramc_calib() are
 * unchanged at the machine-code level between the vendor V1.6 and V1.8
 * objects.  main() carries the V1.8 changes: SYS_GLOBAL_PARM stores DRAM
 * size in 16-MiB units and 512-MiB parts set DRAMC 0x7c bit 2.
 */

extern int do_dqs_gw_calib_1(void);
extern void dle_factor_handler(u32 value);
extern int do_sw_rx_dq_dqs_calib(void);
extern int do_dle_calib(void);
extern int do_sw_tx_dq_dqs_calib(void);
extern int en7512_dramc_init(void);
extern int opt_dle_value, opt_gw_coarse_value, opt_gw_fine_value;

/* Standalone SRAM diagnostics supplied by support.c. */
extern int trace_en7512_dramc_init(void);
extern int trace_do_dqs_gw_calib_1(void);
extern int trace_do_sw_rx_dq_dqs_calib(void);
extern int trace_do_dle_calib(void);
extern int trace_do_sw_tx_dq_dqs_calib(void);
extern int trace_dramc_calib(void);
extern int trace_check_column_bank(void);

int dram_speed;
int PKG_type;
int dram_size;
int dram_type;

int prom_printf_s(const char *fmt, ...)
{
	prom_puts(fmt);
	return 0;
}

static void print_probe_error(u32 address, u32 value)
{
	prom_puts("addr=0x");
	prom_print_hex(address, 8);
	prom_puts("\n");
	prom_print_hex(value, 8);
	prom_puts("\n");
}

int check_column_bank(void)
{
	int col_bits;
	u32 value;

	if (dram_type == DDR3)
		return 0;

	for (col_bits = 8; col_bits < 10; col_bits++) {
		u32 test_addr;
		u32 readback;

		DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_CONF1) & ~0x300u) |
				((u32)(col_bits - 7) << 8), DRAMC_CONF1);
		delay_a_while(2);

		ADDR_WRITE_REG(0x12345678, DRAM_BASE_ADDR);
		delay_a_while(2);
		test_addr = DRAM_BASE_ADDR + BIT(col_bits + 1);
		ADDR_WRITE_REG(0x87654321, test_addr);
		delay_a_while(2);

		do {
			readback = ADDR_READ_REG(test_addr);
			value = ADDR_READ_REG(DRAM_BASE_ADDR);
			if (readback != 0x87654321)
				ADDR_WRITE_REG(0x87654321, test_addr);
		} while (readback != 0x87654321);

		if (value == 0x12345678)
			continue;
		if (value == 0x87654321)
			break;

		prom_puts("Checking col num. dram r/w error!\n");
		print_probe_error(DRAM_BASE_ADDR, value);
		return -1;
	}

	DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_CONF1) & ~0x300u) |
			((u32)(col_bits - 8) << 8), DRAMC_CONF1);
	delay_a_while(2);

	REG32(0xbfb00074) = (REG32(0xbfb00074) & ~3u) | 1u;
	delay_a_while(2);
	DRAMC_WRITE_SET(BIT(24), DRAMC_GDDR3CTL1);
	delay_a_while(2);

	ADDR_WRITE_REG(0x12345678, DRAM_BASE_ADDR);
	delay_a_while(2);
	ADDR_WRITE_REG(0x87654321, DRAM_BASE_ADDR + 0x2000);
	delay_a_while(2);

	do {
		u32 rb = ADDR_READ_REG(DRAM_BASE_ADDR + 0x2000);
		value = ADDR_READ_REG(DRAM_BASE_ADDR);
		if (rb == 0x87654321)
			break;
		ADDR_WRITE_REG(0x87654321, DRAM_BASE_ADDR + 0x2000);
	} while (1);

	if (value == 0x12345678)
		DRAMC_WRITE_SET(BIT(24), DRAMC_GDDR3CTL1);
	else if (value == 0x87654321)
		DRAMC_WRITE_CLEAR(BIT(24), DRAMC_GDDR3CTL1);
	else {
		prom_puts("Checking bank num. dram r/w error!\n");
		print_probe_error(DRAM_BASE_ADDR, value);
		return -1;
	}

	delay_a_while(2);
	return 0;
}

static void set_trfc_values(u32 test2_3, u32 actim1)
{
	DRAMC_WRITE_REG(test2_3, DRAMC_TEST2_3);
	DRAMC_WRITE_REG(actim1, DRAMC_ACTIM1);
}

void set_TRFC(void)
{
	u32 effective_size = (u32)dram_size << en7528_half_size();

	if (PKG_type == QFP) {
		switch (effective_size) {
		case 64:  set_trfc_values(0xa88d0481, 0x600); break;
		case 128: set_trfc_values(0xa8830481, 0x610); break;
		case 256: set_trfc_values(0xa8800481, 0x620); break;
		case 512: set_trfc_values(0xa8850481, 0x640); break;
		}
	} else if (dram_type == DDR2) {
		switch (effective_size) {
		case 32:  set_trfc_values(0xa80b0481, 0x600); break;
		case 64:  set_trfc_values(0xa8030481, 0x610); break;
		case 128: set_trfc_values(0xa8080481, 0x610); break;
		case 256: set_trfc_values(0xa80b0481, 0x620); break;
		case 512: set_trfc_values(0xa80d0481, 0x640); break;
		}
	} else {
		switch (effective_size) {
		case 64:  set_trfc_values(0xa8830481, 0x610); break;
		case 128: set_trfc_values(0xa88a0481, 0x610); break;
		case 256: set_trfc_values(0xa88b0481, 0x620); break;
		case 512: set_trfc_values(0xa8890481, 0x650); break;
		}
	}

	prom_puts("Set new TRFC.\n");
}

void dramc_reg_dump(void)
{
	u32 off;

	for (off = 0; off < 0x400; off += 4) {
		prom_puts("0x");
		prom_print_hex(off, 3);
		prom_puts(":0x");
		prom_print_hex(DRAMC_READ_REG(off), 8);
		prom_puts("\n");
	}

	for (off = 0x600; off < 0x650; off += 4) {
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

	ret = trace_do_dqs_gw_calib_1();
	if (ret < 0)
		return ret;

	dle_factor_handler(8);

	ret = trace_do_sw_rx_dq_dqs_calib();
	if (ret < 0)
		return ret;

	ret = trace_do_dle_calib();
	if (ret < 0)
		return ret;

	return trace_do_sw_tx_dq_dqs_calib();
}

static int detect_dram_size_mib(void)
{
	u32 size;
	volatile u32 *base = (volatile u32 *)(unsigned long)DRAM_PROBE_BASE;

	prom_puts("Calculate size.\n");

	for (size = 32u << 20; size < (1u << 30); size <<= 1) {
		volatile u32 *alias =
			(volatile u32 *)(unsigned long)(DRAM_PROBE_BASE + size);
		u32 value, rb;

		*base = 0x12345678;
		delay_a_while(2);
		*alias = 0x87654321;
		delay_a_while(2);

		do {
			rb = *alias;
			value = *base;
			if (rb != 0x87654321)
				*alias = 0x87654321;
		} while (rb != 0x87654321);

		if (value == 0x12345678)
			continue;
		if (value == 0x87654321) {
			prom_puts("DRAM size=");
			prom_print_dec(size >> 20);
			prom_puts("MB\n");
			return size >> 20;
		}

		prom_puts("dram r/w error!\n");
		print_probe_error(DRAM_PROBE_BASE, value);
		return 0;
	}

	prom_puts("DRAM size=1GB\n");
	return 1024;
}

int main(void)
{
	int calib_failed = 0;
	int ret;
	u32 v;

	if (en7528_is_qfp()) {
		PKG_type = QFP;
		dram_type = DDR3;
	} else {
		PKG_type = KGD;
		dram_type = en7528_is_ddr3() ? DDR3 : DDR2;
	}

	delay_a_while(200);

	ret = trace_en7512_dramc_init();
	if (ret < 0)
		return 0;

	if (trace_dramc_calib() < 0) {
		prom_puts("%dqs_gw (coarse/fine): ");
		prom_print_dec(opt_gw_coarse_value);
		prom_puts("/");
		prom_print_dec(opt_gw_fine_value);
		prom_puts("\n%DQS input dly:\n");
		prom_print_hex(DRAMC_READ_REG(DRAMC_R0DELDLY), 8);
		prom_puts("\n%DQ input dly:\n");
		for (v = DRAMC_DQIDLY1; v < DRAMC_DQIDLY1 + 0x10; v += 4) {
			prom_print_hex(DRAMC_READ_REG(v), 8);
			prom_puts("\n");
		}
		prom_puts("%dle: ");
		prom_print_dec(opt_dle_value);
		prom_puts("\n%DRAMC calibration fail\n");
		REG32(0xbfb00040) |= BIT(31);
		calib_failed = 1;
	}

	trace_check_column_bank();

	if (PKG_type == KGD && en7528_half_size()) {
		DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_CONF1) & ~0x300u) | 0x100u,
				DRAMC_CONF1);
		REG32(0xbfb00074) = 0x104;
	}

	dram_size = detect_dram_size_mib();

	/* V1.8 stores the size in units of 16 MiB in bits [27:20]. */
	v = REG32(SYS_GLOBAL_PARM);
	v &= ~(0xffu << 20);
	v |= (((u32)dram_size >> 4) & 0xffu) << 20;
	REG32(SYS_GLOBAL_PARM) = v;

	/* V1.8 marks 512-MiB geometry in DRAMC register 0x7c. */
	if (dram_size == 512)
		DRAMC_WRITE_SET(BIT(2), 0x07c);

	set_TRFC();

	prom_puts("ddr-");
	prom_print_dec(dram_speed);
	prom_puts("\n\n7528DRAMC V1.8 (");
	prom_print_dec(calib_failed);
	prom_puts(")\r\n");

	DRAMC_WRITE_SET(0x710, DRAMC_PERFCTL0);
	return 0;
}
