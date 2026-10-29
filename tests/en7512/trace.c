/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Deterministic MMIO traces for the old and shared calibration paths. */
static unsigned int regs[0x400 / 4];
static unsigned int operations, reads, writes, delays;
static unsigned int hash = 2166136261U, hash2 = 0x12345678U;
static unsigned int profile, phase;

extern TEST_CASE_TYPE dle_test_cases[];
extern unsigned int dqs_gw[DQS_GW_WORDS];
extern int dle_result[DLE_MAX];
extern int opt_dle_idx, opt_dle_value;
extern int opt_gw_coarse_value, opt_gw_fine_value;
int do_dle_calib(void);
int do_dqs_gw_calib_1(void);

static void record(unsigned int kind, unsigned int reg, unsigned int value)
{

	hash = (hash ^ kind) * 16777619U;
	hash = (hash ^ reg) * 16777619U;
	hash = (hash ^ value) * 16777619U;
	hash2 = ((hash2 << 7) | (hash2 >> 25)) + kind + reg + value;
	operations++;
}

static int gating_pass(unsigned int coarse, unsigned int fine)
{
	unsigned int first = DQS_GW_COARSE_START;
	unsigned int last = DQS_GW_COARSE_END;

	switch (profile) {
	case 0: return 0;
	case 1: return 1;
	case 2: return coarse == first && fine >= 31 && fine <= 33;
	case 3: return coarse == last && fine >= 62 && fine <= 66;
	case 4: return (coarse == first || coarse == last) &&
			fine >= 10 && fine < 20;
	case 5: return coarse == first + 1 &&
			((fine >= 3 && fine < 7) || (fine >= 40 && fine < 44));
	case 6: return coarse == last && fine == 95;
	case 7: return coarse == last && fine == 0;
	case 8: return ((coarse * 97 + fine * 13) % 17) < 8;
	case 9: return !phase && coarse == first + 2 &&
			fine >= 20 && fine < 40;
	default: return 0;
	}
}

unsigned int test_read(unsigned int offset)
{
	unsigned int value = regs[offset / 4];

	if (offset == DRAMC_TESTRPT)
		value |= DRAMC_TEST_DM_CMP_CPT;
	if (offset == DRAMC_DQSGNWCNT0) {
		unsigned int coarse = ((regs[DRAMC_DQSCTL1 / 4] >> 24) & 7) * 4 +
				      (regs[DRAMC_DQSGCTL / 4] & 3);
		unsigned int fine = regs[DRAMC_R0DQSIEN / 4] & 0x7f;

		value = gating_pass(coarse, fine) ? HW_DQS_GW_COUNTER : 0;
	}
	reads++;
	record(1, offset, value);
	return value;
}

void test_write(unsigned int offset, unsigned int value)
{
	regs[offset / 4] = value;
	writes++;
	record(2, offset, value);
}

void pause_polling(unsigned int count)
{
	delays++;
	record(3, 0, count);
}

void test_phy_reset(void)
{
	DRAMC_WRITE_SET(BIT(28), DRAMC_PHYCTL1);
	DRAMC_WRITE_SET(BIT(25), DRAMC_GDDR3CTL1);
	pause_polling(1);
	DRAMC_WRITE_CLEAR(BIT(28), DRAMC_PHYCTL1);
	DRAMC_WRITE_CLEAR(BIT(25), DRAMC_GDDR3CTL1);
}

void prom_puts(const char *s)
{
	while (*s)
		record(4, 0, (unsigned char)*s++);
}

static int dma_result(unsigned int start, unsigned int len, void *arg)
{
	unsigned int tap = ((regs[DRAMC_DDR2CTL / 4] >> 4) & 7) |
			   ((regs[DRAMC_PADCTL4 / 4] & 0x10) >> 1);
	int pass = 0;

	record(5, start, len);
	record(6, tap, (unsigned int)(unsigned long)arg);
	switch (profile) {
	case 1: pass = 1; break;
	case 2: pass = tap <= 1; break;
	case 3: pass = tap >= 14; break;
	case 4: pass = !(tap & 1); break;
	case 5: pass = (tap >= 1 && tap <= 2) ||
			(tap >= 10 && tap <= 11); break;
	case 6: pass = tap == 7 || tap == 8; break;
	case 7: pass = !phase && tap >= 8 && tap <= 9; break;
	}
	return pass ? 0 : -1;
}

static void sys_write(const void *data, unsigned int len)
{
	register int v0 __asm__("$2") = 4004;
	register int a0 __asm__("$4") = 1;
	register const void *a1 __asm__("$5") = data;
	register unsigned int a2 __asm__("$6") = len;

	__asm__ volatile("syscall" : "+r"(v0) : "r"(a0), "r"(a1), "r"(a2)
			 : "$7", "memory");
}

int guard_test(int argc, char **argv)
{
	unsigned int mode = 0, i;
	int result, first_result;

	if (argc > 1)
		mode = argv[1][0] - '0';
	if (argc > 2)
		for (char *p = argv[2]; *p; p++)
			profile = profile * 10 + *p - '0';
	for (i = 0; i < sizeof(regs) / sizeof(regs[0]); i++)
		regs[i] = 0x5a5aa5a5U ^ (i * 0x01010101U);
	dle_test_cases[0].TEST_CALLBACK = dma_result;
	first_result = mode ? do_dqs_gw_calib_1() : do_dle_calib();
	result = first_result;
	if ((!mode && profile == 7) || (mode && profile == 9)) {
		phase = 1;
		result = mode ? do_dqs_gw_calib_1() : do_dle_calib();
	}

	unsigned int out[] = {
		result, first_result, opt_dle_idx, opt_dle_value,
		opt_gw_coarse_value, opt_gw_fine_value,
		operations, reads, writes, delays, hash, hash2,
	};

	sys_write(out, sizeof(out));
	sys_write(regs, sizeof(regs));
	sys_write(dle_result, sizeof(dle_result));
	sys_write(dqs_gw, sizeof(dqs_gw));
	return 0;
}
