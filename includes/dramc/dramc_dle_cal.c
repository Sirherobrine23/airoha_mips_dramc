/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "calibration.h"

static int global_dle_value;
static int score;
int opt_dle_idx;
int opt_dle_value;
int dle_result[DLE_MAX];

void dle_factor_handler(unsigned int value)
{
	/* DATLAT[2:0] is DDR2CTL[6:4]; DATLAT3 is PADCTL4[4]. */
	DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_DDR2CTL) & ~0x70U) |
			((value & 7U) << 4), DRAMC_DDR2CTL);
	DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_PADCTL4) & ~0x10U) |
			(((value >> 3) & 1U) << 4), DRAMC_PADCTL4);
}

struct dramc_test_case dle_test_cases[] = {
	{ dramc_dma_test, DRAM_BASE_ADDR, 0x1000, (void *)-1 },
};

int dle_test(void)
{
	int pass = dle_test_cases[0].fn(dle_test_cases[0].start,
				      dle_test_cases[0].len,
				      dle_test_cases[0].arg) == 0;

	dle_result[global_dle_value++] = pass;
	return pass;
}

void dle_calib_reset(void)
{
}

int dle_calib(void)
{
	int tap;

	for (tap = DLE_START; tap <= DLE_END; tap++) {
		dle_factor_handler(tap);
		score += dle_test();
	}

	return 1;
}

int do_dle_calib(void)
{
	int tap;

	global_dle_value = 0;
	dle_calib();

	/* Select the second tap of the first adjacent passing pair. Preserve
	 * the vendor's opt_dle_idx lifetime across repeated calibrations.
	 */
	for (tap = 1; tap < DLE_MAX; tap++) {
		if (dle_result[tap - 1] && dle_result[tap]) {
			if (opt_dle_idx < tap)
				opt_dle_idx = tap;
			break;
		}
	}

	opt_dle_value = opt_dle_idx;
	if (opt_dle_value) {
		dle_factor_handler(opt_dle_value);
		return 0;
	}

	prom_puts("%cannot find opt_dle value\n");
	return -1;
}
