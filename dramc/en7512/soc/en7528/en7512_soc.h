/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef EN7512_SOC_H
#define EN7512_SOC_H

#define SYS_HWCONF		0xbfb0009cu
#define SYS_BOOT_CFG		0xbfb000b8u

#define EFUSE0_CTRL		0xbfbf8200u
#define EFUSE0_STATUS		0xbfbf820cu
#define EFUSE0_DATA0		0xbfbf8214u
#define EFUSE0_DATA1		0xbfbf8218u
#define EFUSE0_CLK_WIDTH	0xbfbf8230u
#define EFUSE1_CTRL		0xbfaa0000u
#define EFUSE1_STATUS		0xbfaa000cu
#define EFUSE1_DATA3		0xbfaa0018u
#define EFUSE1_CLK_WIDTH	0xbfaa0030u

/* Standalone DDR training uses the direct KSEG1 alias and bounded polling. */
#define DRAM_PROBE_BASE		0xa0080000u
#define DQS_GW_COARSE_START	6
#define DQS_GW_COARSE_END	20
#define DQS_GW_TEST_LENGTH	0x0f
#define HW_DQS_GW_COUNTER	0x78787878u
#define DRAMC_POLL_TIMEOUT	0x00100000u
#define DRAMC_ALWAYS_CHECK_BANK	1

static inline int en7528_efuse_remarked(void)
{
	return !!(readl(EFUSE0_DATA0) & BIT(0));
}

static inline int en7528_is_qfp(void)
{
	unsigned int e0 = readl(EFUSE0_DATA0);

	return en7528_efuse_remarked() ? (((e0 >> 3) & 3u) == 0) : (((e0 >> 1) & 3u) == 0);
}

static inline int en7528_is_ddr3(void)
{
	if (en7528_efuse_remarked())
		return !((readl(EFUSE1_DATA3) >> 8) & 1u);
	return !((readl(EFUSE0_DATA0) >> 5) & 1u);
}

static inline int en7528_half_size(void)
{
	if (en7528_efuse_remarked())
		return (int)((readl(EFUSE1_DATA3) >> 9) & 1u);
	return (int)((readl(EFUSE0_DATA0) >> 6) & 1u);
}

extern int dram_speed;

void mempll_init_int(unsigned int xtal, unsigned int speed);
void mempll_init_ddr1066(unsigned int xtal);
void mempll_init_ddr1333(unsigned int xtal);
int mt_mempll_cali(void);

#endif
