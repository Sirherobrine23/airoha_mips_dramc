// SPDX-License-Identifier: GPL-2.0-or-later
/* Minimal runtime used by the readable EN7528 DDR calibration code. */

typedef unsigned char u8;
typedef unsigned int u32;
#define REG32(addr) (*(volatile u32 *)(addr))

#define ECONET_SYS_GLOBAL_PARM  0xbfb00284u
#define ECONET_TIMER_BASE       0xbfbf0100u
#define ECONET_TIMER_CTL        (ECONET_TIMER_BASE + 0x00)
#define ECONET_TIMER1_LDV       (ECONET_TIMER_BASE + 0x0c)
#define ECONET_TIMER1_VLR       (ECONET_TIMER_BASE + 0x10)
#define ECONET_UART_BASE        0xbfbf0000u

static unsigned int get_hclk_mhz(void)
{
	u32 mhz = (REG32(ECONET_SYS_GLOBAL_PARM) >> 10) & 0x3ff;

	return mhz ? mhz : 250;
}

static void serial_outc(char c)
{
	while (!(REG32(ECONET_UART_BASE + 0x14) & 0x20))
		;
	REG32(ECONET_UART_BASE + 0x00) = (u8)c;
}

void prom_puts(const char *str)
{
	while (*str) {
		if (*str == '\n')
			serial_outc('\r');
		serial_outc(*str++);
	}
}

void prom_print_hex(unsigned long val, int len)
{
	int i;

	for (i = len - 1; i >= 0; i--) {
		u8 n = (val >> (i * 4)) & 0xf;
		serial_outc(n < 10 ? '0' + n : 'a' + n - 10);
	}
}

void prom_print_dec(unsigned long val)
{
	char buf[11];
	int i = sizeof(buf);

	buf[--i] = '\0';
	do {
		buf[--i] = '0' + val % 10;
		val /= 10;
	} while (val && i);
	prom_puts(&buf[i]);
}

void time_polling_init(void)
{
	u32 hclk = get_hclk_mhz();
	u32 ctl;

	REG32(ECONET_TIMER1_LDV) = hclk * 1000u * 10u / 2u;
	ctl = REG32(ECONET_TIMER_CTL);
	ctl |= (1u << 1) | (1u << 9);
	REG32(ECONET_TIMER_CTL) = ctl;
}

void pause_polling(int usec)
{
	u32 hclk = get_hclk_mhz();
	u32 ldv = REG32(ECONET_TIMER1_LDV);
	u32 last = REG32(ECONET_TIMER1_VLR);
	u32 target = (u32)usec * (hclk / 2u);
	u32 elapsed = 0;

	while (elapsed < target) {
		u32 now = REG32(ECONET_TIMER1_VLR);
		elapsed += last >= now ? last - now : ldv - now + last;
		last = now;
	}
}

extern void init_system(int is_bootext);

int spram_preprocess(void)
{
	/* Cold boot cannot inherit the chainloader's UART configuration. */
	REG32(ECONET_UART_BASE + 0x0c) = 0x80;
	REG32(ECONET_UART_BASE + 0x2c) = 0xea00fde8;
	REG32(ECONET_UART_BASE + 0x00) = 1;
	REG32(ECONET_UART_BASE + 0x04) = 0;
	REG32(ECONET_UART_BASE + 0x0c) = 3;
	REG32(ECONET_UART_BASE + 0x08) = 0x0f;
	REG32(ECONET_UART_BASE + 0x10) = 0;
	REG32(ECONET_UART_BASE + 0x24) = 0;

	/*
	 * Match the normal-flash vendor path.  init_system(0) sets up the
	 * EN7528 system clock and loads both eFuse macros before DRAM
	 * detection reads the package and DDR type shadows.
	 */
	init_system(0);
	time_polling_init();

	return 0;
}

void spram_postprocess(void)
{
}

/* Temporary cold-boot diagnostics: preserve each calibration return value. */
static int trace_calibration(const char *name, int (*run)(void))
{
	int ret;

	prom_puts("DDR enter: ");
	prom_puts(name);
	prom_puts("\n");
	ret = run();
	prom_puts("DDR leave: ");
	prom_puts(name);
	prom_puts(" ret=0x");
	prom_print_hex((u32)ret, 8);
	prom_puts("\n");
	return ret;
}

extern int en7512_dramc_init(void);
int trace_en7512_dramc_init(void)
{
	return trace_calibration("en7512_dramc_init", en7512_dramc_init);
}

extern int dramc_calib(void);
int trace_dramc_calib(void)
{
	return trace_calibration("dramc_calib", dramc_calib);
}

extern int do_dqs_gw_calib_1(void);
int trace_do_dqs_gw_calib_1(void)
{
	return trace_calibration("do_dqs_gw_calib_1", do_dqs_gw_calib_1);
}

extern int do_sw_rx_dq_dqs_calib(void);
int trace_do_sw_rx_dq_dqs_calib(void)
{
	return trace_calibration("do_sw_rx_dq_dqs_calib", do_sw_rx_dq_dqs_calib);
}

extern int do_dle_calib(void);
int trace_do_dle_calib(void)
{
	return trace_calibration("do_dle_calib", do_dle_calib);
}

extern int do_sw_tx_dq_dqs_calib(void);
int trace_do_sw_tx_dq_dqs_calib(void)
{
	return trace_calibration("do_sw_tx_dq_dqs_calib", do_sw_tx_dq_dqs_calib);
}

extern int check_column_bank(void);
int trace_check_column_bank(void)
{
	return trace_calibration("check_column_bank", check_column_bank);
}
