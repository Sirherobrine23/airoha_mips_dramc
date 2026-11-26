/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef EN7512_SOC_H
#define EN7512_SOC_H

#define EFUSE0_CTRL	0xbfbf8200u
#define EFUSE0_DATA0	0xbfbf8214u
#define EFUSE1_CTRL	0xbfaa0000u

/* EN751627 uses KSEG2 TLB mapping and split TA2 write/read in RX/TX training. */
#define DRAM_PROBE_BASE		0xc0080000u
#define DQS_GW_COARSE_START	6
#define DQS_GW_COARSE_END	20
#define DQS_GW_TEST_LENGTH	0x0f
#define HW_DQS_GW_COUNTER	0x78787878u
#define DRAMC_TA2_SPLIT_RW	1

void local_flush_tlb_all(void);
void tlb_1g_config(void);
void init_system(int preserve_clock);
void en751627_setup_clk(unsigned int cpu_mhz);

#endif
