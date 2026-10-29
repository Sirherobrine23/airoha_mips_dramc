/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "calibration.h"

static int score;
unsigned int dqs_gw[DQS_GW_WORDS];
int opt_gw_coarse_value;
int opt_gw_fine_value;

int dramc_ta2(unsigned int start, unsigned int len, void *arg)
{
	unsigned int timeout = 0x7fffffff;
	unsigned int counter;

	(void)arg;
	DRAMC_WRITE_REG(((start >> 8) & 0x001fffffU) | 0x55000000U,
			DRAMC_TEST2_1);
	DRAMC_WRITE_REG((len & 0x00ffffffU) | 0xaa000000U, DRAMC_TEST2_2);
	DRAMC_WRITE_SET(DRAMC_TA2_READ_ENABLE, DRAMC_CONF2);

	while (!(DRAMC_READ_REG(DRAMC_TESTRPT) & DRAMC_TEST_DM_CMP_CPT))
		if (--timeout == 0)
			break;

	delay_a_while(1);
	counter = DRAMC_READ_REG(DRAMC_DQSGNWCNT0);
	DRAMC_WRITE_CLEAR(DRAMC_TA2_READ_ENABLE, DRAMC_CONF2);

	return counter == HW_DQS_GW_COUNTER ? 0 : -1;
}

void dqsi_gw_dly_coarse_factor_handler(unsigned int value)
{
	/* Legacy EN7512 uses three DQSINCTL bits, unlike newer DRAMC IP. */
	DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_DQSCTL1) & ~0x07000000U) |
			(((value >> 2) & 7U) << 24), DRAMC_DQSCTL1);
	DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_DQSGCTL) & ~0x33U) |
			(value & 3U) | ((value & 3U) << 4), DRAMC_DQSGCTL);
}

void dqsi_gw_dly_fine_factor_handler(unsigned int value)
{
	value &= 0x7f;
	DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_R0DQSIEN) & 0x80000000U) |
			value | (value << 8) | (value << 16) | (value << 24),
			DRAMC_R0DQSIEN);
}

void dqs_gw_counter_reset(void)
{
	DRAMC_WRITE_SET(0x200, DRAMC_SPCMD);
	DRAMC_WRITE_CLEAR(0x200, DRAMC_SPCMD);
	dramc_phy_reset();
}

struct dramc_test_case dqs_gw_test_cases_1[] = {
	{ dramc_ta2, DRAM_PROBE_BASE, DQS_GW_TEST_LENGTH, (void *)-1 },
};

int dqs_gw_calib(void)
{
	int coarse, fine;

	for (coarse = DQS_GW_COARSE_START; coarse <= DQS_GW_COARSE_END;
	     coarse++) {
		int index = coarse - DQS_GW_COARSE_START;

		dqsi_gw_dly_coarse_factor_handler(coarse);
		for (fine = DQS_GW_FINE_START; fine <= DQS_GW_FINE_END; fine++) {
			int word = fine / 32;
			int bit = fine % 32;
			int pass;

			dqsi_gw_dly_fine_factor_handler(fine);
			dqs_gw_counter_reset();
			dqs_gw_test_cases_1[0].start = DRAM_PROBE_BASE;
			pass = dqs_gw_test_cases_1[0].fn(
				dqs_gw_test_cases_1[0].start,
				dqs_gw_test_cases_1[0].len,
				dqs_gw_test_cases_1[0].arg) >= 0;
			if (pass)
				dqs_gw[index * DQS_GW_WORDS_PER_COARSE + word] |=
					BIT(bit);
			score += pass;
		}
	}

	dqs_gw_counter_reset();
	return 1;
}

static int popcount32(unsigned int value)
{
	int count = 0;

	while (value) {
		count += value & 1;
		value >>= 1;
	}
	return count;
}

static int first_set(unsigned int value)
{
	int bit;

	for (bit = 0; bit < 32; bit++)
		if (value & BIT(bit))
			return bit;
	return -1;
}

int do_dqs_gw_calib_1(void)
{
	int coarse, word;
	int best_index = 0;
	int max_pass = 0;
	int best_fine = 0;

	for (coarse = 0; coarse < DQS_GW_WORDS; coarse++)
		dqs_gw[coarse] = 0;

	DRAMC_WRITE_CLEAR(BIT(15), DRAMC_TEST2_4);
	DRAMC_WRITE_SET(BIT(28), DRAMC_DQSCTL1);
	DRAMC_WRITE_SET(0x100, DRAMC_SPCMD);
	dqs_gw_calib();

	for (coarse = 0; coarse < DQS_GW_COARSE_COUNT; coarse++) {
		int pass_count = 0;

		for (word = 0; word < DQS_GW_WORDS_PER_COARSE; word++)
			pass_count += popcount32(
				dqs_gw[coarse * DQS_GW_WORDS_PER_COARSE + word]);

		/* A later coarse value wins a tie, as in the vendor objects. */
		if (pass_count >= max_pass) {
			best_index = coarse;
			max_pass = pass_count;
		}
	}

	if (max_pass) {
		for (word = 0; word < DQS_GW_WORDS_PER_COARSE; word++) {
			int bit = first_set(
				dqs_gw[best_index * DQS_GW_WORDS_PER_COARSE + word]);

			if (bit >= 0) {
				best_fine = word * 32 + bit + max_pass / 2;
				break;
			}
		}

		opt_gw_coarse_value = best_index + DQS_GW_COARSE_START;
		opt_gw_fine_value = best_fine;
		dqsi_gw_dly_coarse_factor_handler(opt_gw_coarse_value);
		dqsi_gw_dly_fine_factor_handler(opt_gw_fine_value);
	}

	DRAMC_WRITE_CLEAR(0x100, DRAMC_SPCMD);
	DRAMC_WRITE_SET(BIT(15), DRAMC_TEST2_4);
	return max_pass ? 0 : -1;
}
