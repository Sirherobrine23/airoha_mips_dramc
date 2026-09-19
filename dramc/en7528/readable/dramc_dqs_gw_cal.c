/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en7528_ddr.h"

/* High-confidence semantic recovery of output/dramc_dqs_gw_cal.o. */

static int dqs_gw_coarse;
static int dqs_gw_fine;
static int dqs_gw_fine_cnt;
static int score;
u32 dqs_gw[DQS_GW_WORDS];
int opt_gw_coarse_value;
int opt_gw_fine_value;

int dramc_ta2(u32 start, u32 len, void *arg)
{
    u32 timeout = 0x7fffffff;
    u32 counter;

    (void)arg;

    DRAMC_WRITE_REG(((start >> 8) & 0x001fffffu) | 0x55000000u,
                    DRAMC_TEST2_1);
    DRAMC_WRITE_REG((len & 0x00ffffffu) | 0xaa000000u, DRAMC_TEST2_2);
    DRAMC_WRITE_SET(BIT(30), DRAMC_CONF2);

    while (!(DRAMC_READ_REG(DRAMC_TESTRPT) & 0x400))
        if (--timeout == 0)
            break;

    delay_a_while(1);
    counter = DRAMC_READ_REG(DRAMC_DQSGNWCNT0);
    DRAMC_WRITE_CLEAR(BIT(30), DRAMC_CONF2);

    return counter == HW_DQS_GW_COUNTER ? 0 : -1;
}

void dqsi_gw_dly_coarse_factor_handler(u32 value)
{
    DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_DQSCTL1) & ~0x07000000u) |
                    (((value >> 2) & 7u) << 24), DRAMC_DQSCTL1);
    DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_DQSGCTL) & ~0x33u) |
                    (value & 3u) | ((value & 3u) << 4), DRAMC_DQSGCTL);
}

void dqsi_gw_dly_fine_factor_handler(u32 value)
{
    value &= 0x7f;
    DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_R0DQSIEN) & 0x80000000u) |
                    value | (value << 8) | (value << 16) | (value << 24),
                    DRAMC_R0DQSIEN);
}

void dqs_gw_counter_reset(void)
{
    DRAMC_WRITE_SET(0x200, DRAMC_SPCMD);
    DRAMC_WRITE_CLEAR(0x200, DRAMC_SPCMD);
    en7528_ddr_phy_reset();
}

struct ddr_test_case dqs_gw_test_cases_1[] = {
    { dramc_ta2, DRAM_PROBE_BASE, 0x0f, (void *)(unsigned long)-1 },
};

int dqs_gw_calib(void)
{
    int coarse, fine;

    for (coarse = DQS_GW_COARSE_START; coarse <= DQS_GW_COARSE_END; coarse++) {
        dqsi_gw_dly_coarse_factor_handler(coarse);

        for (fine = DQS_GW_FINE_START; fine <= DQS_GW_FINE_END; fine++) {
            int coarse_idx = coarse - DQS_GW_COARSE_START;
            int word = fine >> 5;
            int bit = fine & 31;
            int pass;

            dqsi_gw_dly_fine_factor_handler(fine);
            dqs_gw_counter_reset();
            dqs_gw_test_cases_1[0].start = DRAM_PROBE_BASE;
            pass = dqs_gw_test_cases_1[0].fn(dqs_gw_test_cases_1[0].start,
                                             dqs_gw_test_cases_1[0].len,
                                             dqs_gw_test_cases_1[0].arg) >= 0;
            if (pass)
                dqs_gw[coarse_idx * DQS_GW_WORDS_PER_COARSE + word] |= BIT(bit);

            score += pass;
            dqs_gw_fine++;
            dqs_gw_fine_cnt++;
            if (dqs_gw_fine >= 32 || dqs_gw_fine_cnt >= DQS_GW_FINE_COUNT) {
                dqs_gw_coarse++;
                dqs_gw_fine = 0;
                if (dqs_gw_fine_cnt >= DQS_GW_FINE_COUNT)
                    dqs_gw_fine_cnt = 0;
            }
        }

        dqs_gw_fine = 0;
        dqs_gw_fine_cnt = 0;
    }

    dqs_gw_coarse = 0;
    dqs_gw_counter_reset();
    return 1;
}

static int popcount32(u32 v)
{
    int n = 0;
    while (v) {
        n += v & 1;
        v >>= 1;
    }
    return n;
}

static int first_set(u32 v)
{
    int bit;
    for (bit = 0; bit < 32; bit++)
        if (v & BIT(bit))
            return bit;
    return -1;
}

int do_dqs_gw_calib_1(void)
{
    int coarse, word;
    int best_idx = 0;
    int max_pass = 0;
    int best_fine = 0;

    dqs_gw_coarse = 0;
    dqs_gw_fine = 0;
    dqs_gw_fine_cnt = 0;

    for (coarse = 0; coarse < DQS_GW_WORDS; coarse++)
        dqs_gw[coarse] = 0;

    DRAMC_WRITE_CLEAR(BIT(15), DRAMC_TEST2_4);
    DRAMC_WRITE_SET(BIT(28), DRAMC_DQSCTL1);
    DRAMC_WRITE_SET(0x100, DRAMC_SPCMD);

    dqs_gw_calib();

    for (coarse = 0; coarse < DQS_GW_COARSE_COUNT; coarse++) {
        int pass_count = 0;
        for (word = 0; word < DQS_GW_WORDS_PER_COARSE; word++)
            pass_count += popcount32(dqs_gw[coarse * 3 + word]);

        /* Object uses >=, so a later coarse value wins a tie. */
        if (pass_count >= max_pass) {
            best_idx = coarse;
            max_pass = pass_count;
        }
    }

    if (!max_pass) {
        DRAMC_WRITE_CLEAR(0x100, DRAMC_SPCMD);
        DRAMC_WRITE_SET(BIT(15), DRAMC_TEST2_4);
        return -1;
    }

    for (word = 0; word < DQS_GW_WORDS_PER_COARSE; word++) {
        int bit = first_set(dqs_gw[best_idx * 3 + word]);
        if (bit >= 0) {
            best_fine = word * 32 + bit + max_pass / 2;
            break;
        }
    }

    opt_gw_coarse_value = best_idx + DQS_GW_COARSE_START;
    opt_gw_fine_value = best_fine;
    dqsi_gw_dly_coarse_factor_handler(opt_gw_coarse_value);
    dqsi_gw_dly_fine_factor_handler(opt_gw_fine_value);

    DRAMC_WRITE_CLEAR(0x100, DRAMC_SPCMD);
    DRAMC_WRITE_SET(BIT(15), DRAMC_TEST2_4);
    return 0;
}
