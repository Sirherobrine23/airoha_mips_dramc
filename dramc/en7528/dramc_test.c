/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en7528_ddr.h"

/* Preserve this port's DMA access ordering and test-agent polling. */
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

    /* GDMA sees physical addresses; CPU uses the DRAM_PROBE_BASE alias. */
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

