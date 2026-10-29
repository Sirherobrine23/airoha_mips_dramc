/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en751627_ddr.h"

int dramc_dma_test(u32 start, u32 len, void *arg)
{
    volatile u32 count = 0;
    u32 *source = (u32 *)(unsigned long)DRAM_PROBE_BASE;
    u32 *destination = (u32 *)(unsigned long)(DRAM_PROBE_BASE + len);
    u32 words = len >> 2, i;

    (void)start;
    (void)arg;
    for (i = 0; i < words; i++)
        destination[i] = 0;
    for (i = 0; i < words; i++)
        source[i] = i;
    REG32(0xbfb30000u) = 0x80000;
    REG32(0xbfb30004u) = 0x80000 + len;
    REG32(0xbfb3000cu) |= 4;
    REG32(0xbfb30008u) = (REG32(0xbfb30008u) & 0xffc4) | 0x23 | (len << 16);
    while (!(REG32(0xbfb30204u) & 1))
        if (++count == 0x7fffffffu)
            break;
    REG32(0xbfb30204u) = 1;
    for (i = 0; i < words; i++)
        if (destination[i] != i)
            return -1;
    return 0;
}

int dramc_ta1(u32 start, u32 len, void *arg)
{
    u32 timeout = 0x7fffffff;
    int result = (int)DRAMC0_BASE;

    (void)start;
    DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_TEST2_2) & 0xff000000u) | len, DRAMC_TEST2_2);
    DRAMC_WRITE_SET(DRAMC_TA1_ENABLE, DRAMC_CONF2);
    while (!(DRAMC_READ_REG(DRAMC_TESTRPT) & DRAMC_TEST_DM_CMP_CPT))
        if (--timeout == 0)
            break;
    pause_polling(1);
    if (arg)
        result = DRAMC_READ_REG(DRAMC_TESTRPT) & DRAMC_TEST_DM_CMP_ERR ? -1 : 0;
    DRAMC_WRITE_CLEAR(DRAMC_TA1_ENABLE, DRAMC_CONF2);
    return result;
}
