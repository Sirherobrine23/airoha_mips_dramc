/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en7528_ddr.h"

/* EN7528 DRAMC V1.8 readable DLE calibration.
 *
 * Validated from SRAM on EN7528HU together with readable DQS/RX/TX training.
 */

static int global_dle_value;
static int score;
int opt_dle_idx;
int opt_dle_value;
int dle_result[DLE_MAX];

int dramc_dma_test(u32 start, u32 len, void *arg)
{
    volatile u32 count = 0;
    const u32 timeout = 0x00100000;
    volatile u32 *src = (volatile u32 *)(unsigned long)DRAM_PROBE_BASE;
    volatile u32 *dst = (volatile u32 *)(unsigned long)(DRAM_PROBE_BASE + len);
    u32 words = len >> 2;
    u32 i;

    (void)start;
    (void)arg;

    for (i = 0; i < words; i++)
        dst[i] = 0;
    for (i = 0; i < words; i++)
        src[i] = i;

    /* GDMA sees physical addresses; CPU accesses the vendor 0xc008 alias. */
    REG32(0xbfb30000) = 0x00080000;
    REG32(0xbfb30004) = 0x00080000 + len;
    REG32(0xbfb3000c) |= 4;
    REG32(0xbfb30008) = (REG32(0xbfb30008) & 0x0000ffc4u) |
                         0x23u | (len << 16);

    while (!(REG32(0xbfb30204) & 1)) {
        if (++count == timeout)
            break;
    }
    REG32(0xbfb30204) = 1;

    for (i = 0; i < words; i++)
        if (dst[i] != i)
            return -1;

    return 0;
}

int dramc_ta1(u32 start, u32 len, void *check_result)
{
    u32 timeout = 0x00100000;
    int result = 0;

    (void)start;

    DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_TEST2_2) & 0xff000000u) | len,
                    DRAMC_TEST2_2);
    DRAMC_WRITE_SET(BIT(29), DRAMC_CONF2);

    while (!(DRAMC_READ_REG(DRAMC_TESTRPT) & 0x400))
        if (--timeout == 0)
            break;

    delay_a_while(1);

    if (check_result)
        result = (DRAMC_READ_REG(DRAMC_TESTRPT) & 0x4000) ? -1 : 0;

    DRAMC_WRITE_CLEAR(BIT(29), DRAMC_CONF2);
    return result;
}

void dle_factor_handler(u32 value)
{
    DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_DDR2CTL) & ~0x70u) |
                    ((value & 7u) << 4), DRAMC_DDR2CTL);
    DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_PADCTL4) & ~0x10u) |
                    (((value >> 3) & 1u) << 4), DRAMC_PADCTL4);
}

struct ddr_test_case dle_test_cases[] = {
    { dramc_dma_test, DRAM_BASE_ADDR, 0x1000, (void *)(unsigned long)-1 },
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

    /* Vendor selects the second member of the first adjacent passing pair. */
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
