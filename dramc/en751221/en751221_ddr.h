/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef __ECONET_EN751221_DDR_H__
#define __ECONET_EN751221_DDR_H__

#include <asm/bit.h>
#include <asm/io.h>
#include "dramc/regs.h"

#define ECONET_HIR			0xbfb00064U
#define ECONET_PDIDR		0xbfb0005cU
#define CR_AHB_HWCONF		0xbfb0008cU
#define CR_AHB_SSTR			0xbfb0009cU
#define REG_SAVE_INFO		0xbfb00284U
#define BOOT_RETURN_ADDR	0xbfb00280U
#define GET_SYS_CLK			((readl(REG_SAVE_INFO) >> 12) & 0x3ffU)
#define GET_DRAM_SIZE		(readl(REG_SAVE_INFO) & 0xfffU)

#define EFUSE_VERIFY_DATA0	0xbfbf8214U
#define EFUSE_VERIFY_DATA1	0xbfbf8218U
#define EFUSE_REMARK_BIT	BIT(6)
#define EFUSE_EN7526F		0x00U
#define EFUSE_EN7526D		0x01U
#define EFUSE_EN7526G		0x02U
#define EFUSE_EN7512		0x04U
#define EFUSE_EN7513		0x05U
#define EFUSE_EN7513G		0x06U
#define EFUSE_EN7586		0x0aU
#define EFUSE_EN7521F		0x10U
#define EFUSE_EN7526FT		0x11U
#define EFUSE_EN7521G		0x12U
#define EFUSE_EN7521S		0x20U

#define SET_DRAM_SIZE(x)                        \
	do {                                        \
		unsigned int v = readl(REG_SAVE_INFO); \
		v = (v & ~0xfffU) | ((x) & 0xfffU);     \
		readl(REG_SAVE_INFO) = v;              \
	} while (0)

static inline unsigned int en751221_efuse_package(void)
{
	unsigned int v = readl(EFUSE_VERIFY_DATA0);
	return (v & EFUSE_REMARK_BIT) ? ((v >> 7) & 0x3fU) : (v & 0x3fU);
}

static inline unsigned int en751221_efuse_package_no_bit0(void)
{
	unsigned int v = readl(EFUSE_VERIFY_DATA0);
	return (v & EFUSE_REMARK_BIT) ? ((v >> 7) & 0x3cU) : (v & 0x3cU);
}

static inline unsigned int en751221_efuse_package_sel(void)
{
	unsigned int v = readl(EFUSE_VERIFY_DATA0);
	return (v & EFUSE_REMARK_BIT) ? ((v >> 7) & 3U) : (v & 3U);
}

#define isLQFP		(isEN751221 && en751221_efuse_package_sel() == 0)
#define isEN7526c	((readl(ECONET_HIR) & 0xffff0000U) == 0x00080000U)
#define isEN751221	(((readl(ECONET_HIR) & 0xffff0000U) == 0x00070000U) || isEN7526c)
#define isEN7526FT	(isEN751221 && en751221_efuse_package() == EFUSE_EN7526FT)
#define isEN7512	(isEN751221 && en751221_efuse_package() == EFUSE_EN7512)
#define isEN7526D	(isEN751221 && en751221_efuse_package() == EFUSE_EN7526D)
#define isEN7513	(isEN751221 && en751221_efuse_package() == EFUSE_EN7513)
#define isEN7526G	(isEN751221 && en751221_efuse_package() == EFUSE_EN7526G)
#define isEN7521G	(isEN751221 && en751221_efuse_package() == EFUSE_EN7521G)
#define isEN7513G	(isEN751221 && en751221_efuse_package() == EFUSE_EN7513G)
#define isEN7586	(isEN751221 && en751221_efuse_package() == EFUSE_EN7586)
#define isEN7526F	(isEN7526c ? en751221_efuse_package_no_bit0() == EFUSE_EN7526F : \
					 (isEN751221 && (en751221_efuse_package() == EFUSE_EN7526F || isEN7526FT)))
#define isEN7521F	(isEN7526c ? en751221_efuse_package_no_bit0() == EFUSE_EN7521F : \
					 (isEN751221 && en751221_efuse_package() == EFUSE_EN7521F))
#define isEN7521S	(isEN7526c ? en751221_efuse_package_no_bit0() == EFUSE_EN7521S : \
					 (isEN751221 && en751221_efuse_package() == EFUSE_EN7521S))
 
#define EFUSE_IS_DDR3 \
	(!!(readl(EFUSE_VERIFY_DATA0) & EFUSE_REMARK_BIT) ? \
		 !!(readl(EFUSE_VERIFY_DATA0) & BIT(24)) :  \
		 !!(readl(EFUSE_VERIFY_DATA0) & BIT(23)))
#define EFUSE_Fix32MB \
	(!!(readl(EFUSE_VERIFY_DATA0) & EFUSE_REMARK_BIT) ? \
		 !!(readl(EFUSE_VERIFY_DATA0) & BIT(26)) : \
		 !!(readl(EFUSE_VERIFY_DATA0) & BIT(25)))

#define ST0_IM		0x0000ff00U

static inline void en751221_change_cp0_status(unsigned int clear,
					      unsigned int set)
{
	unsigned int v;
	__asm__ __volatile__("mfc0 %0,$12" : "=r"(v));
	v = (v & ~clear) | set;
	__asm__ __volatile__("mtc0 %0,$12" ::"r"(v));
}

#define change_cp0_status(clear, set)	en751221_change_cp0_status(clear, set)

#define isFPGA							(isEN7526c ? !(readl(CR_AHB_SSTR) & BIT(0)) : \
										 (isEN751221 ? !(readl(CR_AHB_HWCONF) & BIT(29)) : 0))

#define EN7512_SYS_HCLK					(isFPGA ? 32U : GET_SYS_CLK)
#define CR_INTC_BASE					0xbfb40000U
#define CR_INTC_IMR						(CR_INTC_BASE + 0x04)
#define CR_TIMER_BASE					0xbfbf0100U
#define CR_TIMER_CTL					(CR_TIMER_BASE + 0x00)
#define CR_TIMER0_LDV					(CR_TIMER_BASE + 0x04)
#define CR_TIMER0_VLR					(CR_TIMER_BASE + 0x08)
#define CR_TIMER1_LDV					(CR_TIMER_BASE + 0x0c)
#define CR_TIMER1_VLR					(CR_TIMER_BASE + 0x10)
#define ENABLE							1U
#define TIMER_TOGGLEMODE				1U
#define TIMER_HALTDISABLE				0U
#define TIMERTICKS_10MS					10U
#define CR_UART_BASE					0xbfbf0000U
#define CR_UART_RBR						(CR_UART_BASE + 0x00)
#define CR_UART_THR						(CR_UART_BASE + 0x00)
#define CR_UART_BRDL					(CR_UART_BASE + 0x00)
#define CR_UART_BRDH					(CR_UART_BASE + 0x04)
#define CR_UART_IER						(CR_UART_BASE + 0x04)
#define CR_UART_FCR						(CR_UART_BASE + 0x08)
#define CR_UART_LCR						(CR_UART_BASE + 0x0c)
#define CR_UART_MCR						(CR_UART_BASE + 0x10)
#define CR_UART_LSR						(CR_UART_BASE + 0x14)
#define CR_UART_MISCC					(CR_UART_BASE + 0x24)
#define CR_UART_XYD						(CR_UART_BASE + 0x2c)

/* UART registers are byte-wide in the low byte of each BE word. */
#define UART8(reg)						readb((reg) + 3)
#define UART_BRD_ACCESS					0x80U
#define UART_XYD_Y						65000U
#define UART_BRDL_20M					1U
#define UART_BRDH_20M					0U
#define UART_LCR						3U
#define UART_FCR						0x0fU
#define UART_MCR						0U
#define UART_MISCC						0U
#define UART_IER						1U
#define LSR_THRE						0x20U
#define DDR3							0
#define DDR2							1
#define KGD								0
#define BGA1							1
#define BGA2							2
#define DRAM_START						0xa0080000U
#define DRAM_BASE_ADDR					0xa0000000U
#define DQ_DATA_WIDTH					16
#define DQS_NUMBER						2
#define DQS_BIT_NUMBER					8
#define MAX_RX_DQSDLY_TAPS				96
#define MAX_RX_DQDLY_TAPS				16
#define TX_DQS_NUMBER					2
#define TX_DQ_DATA_WIDTH				16
#define MAX_TX_DQDLY_TAPS				16
#define MAX_TX_DQSDLY_TAPS				16
#define DLE_START						0
#define DLE_END							15
#define DLE_STEP						1
#define DLE_MAX							16
#define DQS_GW_COARSE_START				0
#define DQS_GW_COARSE_END				31
#define DQS_GW_COARSE_STEP				1
#define DQS_GW_COARSE_MAX				32
#define DQS_GW_FINE_START				0
#define DQS_GW_FINE_END					95
#define DQS_GW_FINE_STEP				1
#define DQS_GW_FINE_MAX					96
#define DQS_GW_LEN_PER_COARSE_ELEMENT	(8 * sizeof(unsigned int))
#define DQS_GW_LEN_PER_COARSE			((DQS_GW_FINE_MAX + DQS_GW_LEN_PER_COARSE_ELEMENT - 1) / DQS_GW_LEN_PER_COARSE_ELEMENT)
#define DQS_GW_LEN						(DQS_GW_COARSE_MAX * DQS_GW_LEN_PER_COARSE)
#define HW_DQS_GW_COUNTER				0xf8f8f8f8U
#define max_col_bits					10

#define DRAMC0_BASE						0xbfb20000U
#define DRAMC_REG(o)					readl(DRAMC0_BASE + (o))
#define DRAMC_READ_REG(o)				DRAMC_REG(o)
#define DRAMC_WRITE_REG(v, o)			do {DRAMC_REG(o) = (unsigned int)(v);} while (0)
#define DRAMC_WRITE_SET(v, o)			do {DRAMC_REG(o) |= (unsigned int)(v);} while (0)
#define DRAMC_WRITE_CLEAR(v, o)			do {DRAMC_REG(o) &= ~(unsigned int)(v);} while (0)
	
#define ADDR_READ_REG(a) 				readl(a)
#define ADDR_WRITE_REG(v, a)			do {readl(a) = (unsigned int)(v);} while (0)

#define delay_a_while(c)				do {pause_polling(c);} while (0)

extern void pause_polling(unsigned int count);

static inline void en751221_ddr_phy_reset(void)
{
	DRAMC_WRITE_SET(BIT(28), DRAMC_PHYCTL1);
	DRAMC_WRITE_SET(BIT(25), DRAMC_GDDR3CTL1);
	delay_a_while(1);
	DRAMC_WRITE_CLEAR(BIT(28), DRAMC_PHYCTL1);
	DRAMC_WRITE_CLEAR(BIT(25), DRAMC_GDDR3CTL1);
}

#define DDR_PHY_RESET_NEW() en751221_ddr_phy_reset()

#endif
