/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en751221_ddr.h"

/* Preserve this port's DMA access ordering and test-agent polling. */
extern void prom_puts(const char *s);

int dramc_dma_test(unsigned int start, unsigned int len, void *ext_arg)
{
	int i;
	volatile unsigned int count;
	volatile unsigned int timeout = 0x7fffffff;
	unsigned int *source = (unsigned int *)DRAM_START;
	unsigned int *destination = (unsigned int *)(DRAM_START + len);
	unsigned int destination_phys = 0x80000 + len;

	for (i = 0; i < len / sizeof(unsigned int); i++)
		destination[i] = 0;
	for (i = 0; i < len / sizeof(unsigned int); i++)
		source[i] = i;

	ADDR_WRITE_REG(0x80000, 0xbfb30000);
	ADDR_WRITE_REG(destination_phys, 0xbfb30004);
	ADDR_WRITE_REG(ADDR_READ_REG(0xbfb3000c) | 4, 0xbfb3000c);
	ADDR_WRITE_REG((ADDR_READ_REG(0xbfb30008) & 0xffc4) | 0x23 |
			       (len << 16),
		       0xbfb30008);
	count = 0;
	while (!(ADDR_READ_REG(0xbfb30204) & 1)) {
		count++;
		if (count == timeout)
			break;
	}
	ADDR_WRITE_REG(1, 0xbfb30204);
	for (i = 0; i < len / sizeof(unsigned int); i++)
		if (i != destination[i])
			return -1;
	return 0;
}

int dramc_ta1(unsigned int start, unsigned int len, void *ext_arg)
{
	unsigned int count = 0x7fffffff;
	int result = 0;
	/* The reference retains both copies of ext_arg and the final -1 constant.
     * Empty GCC asm constraints emit no instructions themselves; they preserve
     * those otherwise-dead lifetimes and the observed register allocation.
     * These constraints are reconstruction aids, not claimed original source.
     */
	register void *check_result __asm__("$17") = ext_arg;
	__asm__ volatile("" : "+r"(check_result));

	register unsigned int base __asm__("$3") = DRAMC0_BASE;
	__asm__ volatile("" : "+r"(base));
	unsigned int value = *(volatile unsigned int *)(base + DRAMC_TEST2_2) &
			     0xff000000;
	__asm__ volatile("" : "+r"(len));
	*(volatile unsigned int *)(base + DRAMC_TEST2_2) = len | value;
	*(volatile unsigned int *)(base + DRAMC_CONF2) |= DRAMC_TA1_ENABLE;
	while (!(DRAMC_READ_REG(DRAMC_TESTRPT) & DRAMC_TEST_DM_CMP_CPT)) {
		if (--count == 0)
			break;
	}
	delay_a_while(1);
	if (check_result)
		result = (DRAMC_READ_REG(DRAMC_TESTRPT) &
			  DRAMC_TEST_DM_CMP_ERR) ?
				 -1 :
				 0;
	DRAMC_WRITE_CLEAR(DRAMC_TA1_ENABLE, DRAMC_CONF2);
	__asm__ volatile("" : : "r"(ext_arg), "r"(-1));
	return result;
}

