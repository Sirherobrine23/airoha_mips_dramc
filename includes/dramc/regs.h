/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef EN7512_DRAMC_REGS_H
#define EN7512_DRAMC_REGS_H

#include <asm/io.h>

/* Legacy EN7512 DRAMC register offsets shared by EN751221, EN751627 and EN7528.
 * Corroborated by bootrom/ddr_cal_en7512/inc/dramc.h in the vendor SDK.
 * Absolute MMIO access and SoC-specific clock/eFuse setup stay in each port.
 */
#define DRAMC_ACTIM0				0x000
#define DRAMC_CONF1					0x004
#define DRAMC_CONF2					0x008
#define DRAMC_PADCTL1				0x00c
#define DRAMC_PADCTL2				0x010
#define DRAMC_PADCTL3				0x014
#define DRAMC_R0DELDLY				0x018
#define DRAMC_R1DELDLY				0x01c
#define DRAMC_R0DIFDLY				0x020
#define DRAMC_R1DIFDLY				0x024
#define DRAMC_DLLCONF				0x028
#define DRAMC_TESTMODE				0x02c
#define DRAMC_TEST2_1				0x03c
#define DRAMC_TEST2_2				0x040
#define DRAMC_TEST2_3				0x044
#define DRAMC_TEST2_4				0x048
#define DRAMC_DDR2CTL				0x07c
#define DRAMC_MRS					0x088
#define DRAMC_CLK1DELAY				0x08c
#define DRAMC_IOCTL					0x090
#define DRAMC_R0DQSIEN				0x094
#define DRAMC_R1DQSIEN				0x098
#define DRAMC_DRVCTL00				0x0b4
#define DRAMC_DRVCTL0				0x0b8
#define DRAMC_DRVCTL1				0x0bc
#define DRAMC_DLLSEL				0x0c0
#define DRAMC_TDSEL0				0x0cc
#define DRAMC_TDSEL1				0x0d0
#define DRAMC_MCKDLY				0x0d8
#define DRAMC_DQSCTL0				0x0dc
#define DRAMC_DQSCTL1				0x0e0
#define DRAMC_PADCTL4				0x0e4
#define DRAMC_PADCTL5				0x0e8
#define DRAMC_PADCTL6				0x0ec
#define DRAMC_PHYCTL1				0x0f0
#define DRAMC_GDDR3CTL1				0x0f4
#define DRAMC_PADCTL7				0x0f8
#define DRAMC_MISCTL0				0x0fc
#define DRAMC_OCDK					0x100
#define DRAMC_LBWDAT0				0x104
#define DRAMC_LBWDAT1				0x108
#define DRAMC_LBWDAT2				0x10c
#define DRAMC_RKCFG					0x110
#define DRAMC_CKPHDET				0x114
#define DRAMC_DQSGCTL				0x124
#define DRAMC_CLKENCTL				0x130
#define DRAMC_DQSGCTL1				0x140
#define DRAMC_DQSGCTL2				0x144
#define DRAMC_ARBCTL0				0x168
#define DRAMC_CMDDLY0				0x1a8
#define DRAMC_CMDDLY1				0x1ac
#define DRAMC_CMDDLY2				0x1b0
#define DRAMC_CMDDLY3				0x1b4
#define DRAMC_CMDDLY4				0x1b8
#define DRAMC_CMDDLY5				0x1bc
#define DRAMC_DQSCAL0				0x1c0
#define DRAMC_DQSCAL1				0x1c4
#define DRAMC_DM_MONITOR			0x1d8
#define DRAMC_PD_CTRL				0x1dc
#define DRAMC_LPDDR2				0x1e0
#define DRAMC_SPCMD					0x1e4
#define DRAMC_ACTIM1				0x1e8
#define DRAMC_PERFCTL0				0x1ec
#define DRAMC_DQODLY1				0x200
#define DRAMC_DQODLY2				0x204
#define DRAMC_DQODLY3				0x208
#define DRAMC_DQODLY4				0x20c
#define DRAMC_DQIDLY1				0x210
#define DRAMC_DQIDLY2				0x214
#define DRAMC_DQIDLY3				0x218
#define DRAMC_DQIDLY4				0x21c
#define DRAMC_CMP_ERR				0x370
#define DRAMC_SPCMDRESP				0x3b8
#define DRAMC_DQSGNWCNT0			0x3c0
#define DRAMC_TESTRPT				0x3fc
#define DRAMC_MEMPLL0				0x600
#define DRAMC_MEMPLL1				0x604
#define DRAMC_MEMPLL2				0x608
#define DRAMC_MEMPLL3				0x60c
#define DRAMC_MEMPLL5				0x614
#define DRAMC_MEMPLL6				0x618
#define DRAMC_MEMPLL7				0x61c
#define DRAMC_MEMPLL9				0x624
#define DRAMC_MEMPLL12				0x630
#define DRAMC_MEMPLL_DIVIDER		0x640

#define DRAMC_TA1_ENABLE			BIT(29)
#define DRAMC_TA2_READ_ENABLE		BIT(30)
#define DRAMC_TA2_WRITE_ENABLE		BIT(31)
#define DRAMC_TA2_RW_ENABLE			(DRAMC_TA2_READ_ENABLE | DRAMC_TA2_WRITE_ENABLE)
#define DRAMC_TEST_DM_CMP_CPT		BIT(10)
#define DRAMC_TEST_DM_CMP_ERR		BIT(14)

#endif
