/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef EN7512_SOC_H
#define EN7512_SOC_H

#define ECONET_HIR		0xbfb00064u
#define ECONET_PDIDR		0xbfb0005cu
#define CR_AHB_HWCONF		0xbfb0008cu
#define CR_AHB_SSTR		0xbfb0009cu
#define CR_INTC_IMR		0xbfb40004u

#define EFUSE_VERIFY_DATA0	0xbfbf8214u
#define EFUSE_VERIFY_DATA1	0xbfbf8218u
#define EFUSE_REMARK_BIT	BIT(6)
#define EFUSE_EN7526F		0x00u
#define EFUSE_EN7526D		0x01u
#define EFUSE_EN7526G		0x02u
#define EFUSE_EN7512		0x04u
#define EFUSE_EN7513		0x05u
#define EFUSE_EN7513G		0x06u
#define EFUSE_EN7586		0x0au
#define EFUSE_EN7521F		0x10u
#define EFUSE_EN7526FT		0x11u
#define EFUSE_EN7521G		0x12u
#define EFUSE_EN7521S		0x20u

static inline unsigned int en751221_efuse_package(void)
{
	unsigned int v = readl(EFUSE_VERIFY_DATA0);
	return (v & EFUSE_REMARK_BIT) ? ((v >> 7) & 0x3fu) : (v & 0x3fu);
}

static inline unsigned int en751221_efuse_package_no_bit0(void)
{
	unsigned int v = readl(EFUSE_VERIFY_DATA0);
	return (v & EFUSE_REMARK_BIT) ? ((v >> 7) & 0x3cu) : (v & 0x3cu);
}

static inline unsigned int en751221_efuse_package_sel(void)
{
	unsigned int v = readl(EFUSE_VERIFY_DATA0);
	return (v & EFUSE_REMARK_BIT) ? ((v >> 7) & 3u) : (v & 3u);
}

#define isEN7526c	((readl(ECONET_HIR) & 0xffff0000u) == 0x00080000u)
#define isEN751221	(((readl(ECONET_HIR) & 0xffff0000u) == 0x00070000u) || isEN7526c)
#define isLQFP		(isEN751221 && en751221_efuse_package_sel() == 0)
#define isEN7526FT	(isEN751221 && en751221_efuse_package() == EFUSE_EN7526FT)
#define isEN7512	(isEN751221 && en751221_efuse_package() == EFUSE_EN7512)
#define isEN7526D	(isEN751221 && en751221_efuse_package() == EFUSE_EN7526D)
#define isEN7513	(isEN751221 && en751221_efuse_package() == EFUSE_EN7513)
#define isEN7526G	(isEN751221 && en751221_efuse_package() == EFUSE_EN7526G)
#define isEN7521G	(isEN751221 && en751221_efuse_package() == EFUSE_EN7521G)
#define isEN7513G	(isEN751221 && en751221_efuse_package() == EFUSE_EN7513G)
#define isEN7586	(isEN751221 && en751221_efuse_package() == EFUSE_EN7586)
#define isEN7526F                                                                                  \
	(isEN7526c ? en751221_efuse_package_no_bit0() == EFUSE_EN7526F                             \
		   : (isEN751221 && (en751221_efuse_package() == EFUSE_EN7526F || isEN7526FT)))
#define isEN7521F                                                                                  \
	(isEN7526c ? en751221_efuse_package_no_bit0() == EFUSE_EN7521F                             \
		   : (isEN751221 && en751221_efuse_package() == EFUSE_EN7521F))
#define isEN7521S                                                                                  \
	(isEN7526c ? en751221_efuse_package_no_bit0() == EFUSE_EN7521S                             \
		   : (isEN751221 && en751221_efuse_package() == EFUSE_EN7521S))

#define EFUSE_IS_DDR3                                                                              \
	(!!(readl(EFUSE_VERIFY_DATA0) & EFUSE_REMARK_BIT)                                          \
		 ? !!(readl(EFUSE_VERIFY_DATA0) & BIT(24))                                         \
		 : !!(readl(EFUSE_VERIFY_DATA0) & BIT(23)))
#define EFUSE_Fix32MB                                                                              \
	(!!(readl(EFUSE_VERIFY_DATA0) & EFUSE_REMARK_BIT)                                          \
		 ? !!(readl(EFUSE_VERIFY_DATA0) & BIT(26))                                         \
		 : !!(readl(EFUSE_VERIFY_DATA0) & BIT(25)))

#define isFPGA                                                                                     \
	(isEN7526c ? !(readl(CR_AHB_SSTR) & BIT(0))                                                \
		   : (isEN751221 ? !(readl(CR_AHB_HWCONF) & BIT(29)) : 0))

/* Calibration parameters for EN751221. */
#define DRAM_PROBE_BASE			0xa0080000u
#define DRAM_MAX_SIZE_MIB		512u
#define DQS_GW_COARSE_START		0
#define DQS_GW_COARSE_END		31
#define DQS_GW_TEST_LENGTH		0xff
#define HW_DQS_GW_COUNTER		0xf8f8f8f8u
#define DRAMC_TRFC_USE_QFP_FOR_DDR3	1

#endif
