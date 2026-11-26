/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "dramc_common.h"

int dramc_dma_test(unsigned int start, unsigned int len, void *arg)
{
	volatile u32 count = 0;
	const u32 timeout = DRAMC_POLL_TIMEOUT;
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
	ADDR_WRITE_REG(0x00080000u, 0xbfb30000u);
	ADDR_WRITE_REG(0x00080000u + len, 0xbfb30004u);
	ADDR_WRITE_REG(ADDR_READ_REG(0xbfb3000cu) | 4u, 0xbfb3000cu);
	ADDR_WRITE_REG((ADDR_READ_REG(0xbfb30008u) & 0x0000ffc4u) | 0x23u | (len << 16),
		       0xbfb30008u);

	while (!(ADDR_READ_REG(0xbfb30204u) & 1u)) {
		if (++count == timeout)
			break;
	}
	ADDR_WRITE_REG(1u, 0xbfb30204u);

	for (i = 0; i < words; i++)
		if (dst[i] != i)
			return -1;

	return 0;
}

int dramc_ta1(unsigned int start, unsigned int len, void *check_result)
{
	u32 timeout = DRAMC_POLL_TIMEOUT;
	int result = 0;

	(void)start;

	DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_TEST2_2) & 0xff000000u) | len, DRAMC_TEST2_2);
	DRAMC_WRITE_SET(DRAMC_TA1_ENABLE, DRAMC_CONF2);

	while (!(DRAMC_READ_REG(DRAMC_TESTRPT) & DRAMC_TEST_DM_CMP_CPT))
		if (--timeout == 0)
			break;

	delay_a_while(1);

	if (check_result)
		result = (DRAMC_READ_REG(DRAMC_TESTRPT) & DRAMC_TEST_DM_CMP_ERR) ? -1 : 0;

	DRAMC_WRITE_CLEAR(DRAMC_TA1_ENABLE, DRAMC_CONF2);
	return result;
}
