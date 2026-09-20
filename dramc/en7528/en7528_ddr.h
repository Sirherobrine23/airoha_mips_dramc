/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef EN7528_DDR_READABLE_H
#define EN7528_DDR_READABLE_H

/*
 * Register definitions shared by the readable EN7528 DDR calibration code.
 *
 * The readable calibration blocks were recovered from the EN7528 GPL DDR
 * objects and validated on an XC220-G3 (EN7528HU) using the V1.8 init/system
 * path.
 * The register map is intentionally flattened because the vendor build mixes
 * EN7528 and legacy EN7516/EN7527 preprocessor branches.
 */

typedef unsigned int u32;
typedef signed int s32;
typedef unsigned char u8;
typedef signed char s8;

#define BIT(n)                  (1u << (n))
#define MMIO32(addr)            (*(volatile u32 *)(unsigned long)(addr))
#define REG32(addr)             MMIO32(addr)

#define SYS_GLOBAL_PARM         0xbfb00284u
#define SYS_BOOT_JUMP           0xbfb00280u
#define SYS_HWCONF              0xbfb0009cu
#define SYS_HWTRAP              0xbfb00240u
#define SYS_CHIP_ID             0xbfb00064u
#define SYS_BOOT_CFG            0xbfb000b8u

#define EFUSE0_CTRL             0xbfbf8200u
#define EFUSE0_STATUS           0xbfbf820cu
#define EFUSE0_DATA0            0xbfbf8214u
#define EFUSE0_CLK_WIDTH        0xbfbf8230u
#define EFUSE1_CTRL             0xbfaa0000u
#define EFUSE1_STATUS           0xbfaa000cu
#define EFUSE1_DATA3            0xbfaa0018u
#define EFUSE1_CLK_WIDTH        0xbfaa0030u

#define DRAMC0_BASE             0xbfb20000u
#define DRAMC_REG(off)          MMIO32(DRAMC0_BASE + (off))
#define DRAMC_READ_REG(off)     DRAMC_REG(off)
#define DRAMC_WRITE_REG(v, off) do { DRAMC_REG(off) = (u32)(v); } while (0)
#define DRAMC_WRITE_SET(v, off) do { DRAMC_REG(off) |= (u32)(v); } while (0)
#define DRAMC_WRITE_CLEAR(v,off) do { DRAMC_REG(off) &= ~(u32)(v); } while (0)
#define ADDR_READ_REG(addr)     MMIO32(addr)
#define ADDR_WRITE_REG(v,addr)  do { MMIO32(addr) = (u32)(v); } while (0)

#define DRAMC_ACTIM0            0x000
#define DRAMC_CONF1             0x004
#define DRAMC_CONF2             0x008
#define DRAMC_PADCTL1           0x00c
#define DRAMC_PADCTL2           0x010
#define DRAMC_PADCTL3           0x014
#define DRAMC_R0DELDLY          0x018
#define DRAMC_DLLCONF           0x028
#define DRAMC_TEST2_1           0x03c
#define DRAMC_TEST2_2           0x040
#define DRAMC_TEST2_3           0x044
#define DRAMC_TEST2_4           0x048
#define DRAMC_DDR2CTL           0x07c
#define DRAMC_MRS               0x088
#define DRAMC_CLK1DELAY         0x08c
#define DRAMC_IOCTL             0x090
#define DRAMC_R0DQSIEN          0x094
#define DRAMC_DRVCTL0           0x0b8
#define DRAMC_DRVCTL1           0x0bc
#define DRAMC_MCKDLY            0x0d8
#define DRAMC_DQSCTL0           0x0dc
#define DRAMC_DQSCTL1           0x0e0
#define DRAMC_PADCTL4           0x0e4
#define DRAMC_PHYCTL1           0x0f0
#define DRAMC_GDDR3CTL1         0x0f4
#define DRAMC_PADCTL7           0x0f8
#define DRAMC_MISCTL0           0x0fc
#define DRAMC_RKCFG             0x110
#define DRAMC_DQSGCTL           0x124
#define DRAMC_CLKENCTL          0x130
#define DRAMC_ARBCTL0           0x168
#define DRAMC_DQSCAL0           0x1c0
#define DRAMC_DM_MONITOR        0x1d8
#define DRAMC_PD_CTRL           0x1dc
#define DRAMC_LPDDR2            0x1e0
#define DRAMC_SPCMD             0x1e4
#define DRAMC_ACTIM1            0x1e8
#define DRAMC_PERFCTL0          0x1ec
#define DRAMC_DQODLY1           0x200
#define DRAMC_DQODLY2           0x204
#define DRAMC_DQODLY3           0x208
#define DRAMC_DQODLY4           0x20c
#define DRAMC_DQIDLY1           0x210
#define DRAMC_DQIDLY2           0x214
#define DRAMC_DQIDLY3           0x218
#define DRAMC_DQIDLY4           0x21c
#define DRAMC_CMP_ERR           0x370
#define DRAMC_SPCMDRESP         0x3b8
#define DRAMC_DQSGNWCNT0        0x3c0
#define DRAMC_TESTRPT           0x3fc
#define DRAMC_MEMPLL0           0x600
#define DRAMC_MEMPLL1           0x604
#define DRAMC_MEMPLL2           0x608
#define DRAMC_MEMPLL3           0x60c
#define DRAMC_MEMPLL5           0x614
#define DRAMC_MEMPLL6           0x618
#define DRAMC_MEMPLL7           0x61c
#define DRAMC_MEMPLL9           0x624
#define DRAMC_MEMPLL12          0x630
#define DRAMC_MEMPLL_DIVIDER    0x640

#define DDR3                    0
#define DDR2                    1
#define KGD                     0
#define BGA1                    1
#define BGA2                    2
#define QFP                     3

/* Standalone DDR training uses the direct KSEG1 alias until TLB setup is
 * restored in the surrounding boot stage. */
#define DRAM_PROBE_BASE         0xa0080000u
#define DRAM_BASE_ADDR          0xa0000000u
#define DQ_DATA_WIDTH           16
#define DQS_NUMBER              2
#define DQS_BIT_NUMBER          8
#define TX_DQ_DATA_WIDTH        16
#define TX_DQS_NUMBER           2
#define MAX_RX_DQDLY_TAPS       16
#define MAX_RX_DQSDLY_TAPS      96
#define MAX_TX_DQDLY_TAPS       16
#define MAX_TX_DQSDLY_TAPS      16
#define DLE_START               0
#define DLE_END                 15
#define DLE_MAX                 16
#define DQS_GW_COARSE_START     6
#define DQS_GW_COARSE_END       20
#define DQS_GW_COARSE_COUNT     15
#define DQS_GW_FINE_START       0
#define DQS_GW_FINE_END         95
#define DQS_GW_FINE_COUNT       96
#define DQS_GW_WORDS_PER_COARSE 3
#define DQS_GW_WORDS            45
#define HW_DQS_GW_COUNTER       0x78787878u

#define DRAMC_TA1_ENABLE         BIT(29)
#define DRAMC_TA2_READ_ENABLE    BIT(30)
#define DRAMC_TA2_WRITE_ENABLE   BIT(31)
#define DRAMC_TA2_RW_ENABLE      (BIT(30) | BIT(31))
#define DRAMC_TEST_DM_CMP_CPT    BIT(10)
#define DRAMC_TEST_DM_CMP_ERR    BIT(14)

struct ddr_test_case {
    int (*fn)(u32 start, u32 len, void *arg);
    u32 start;
    u32 len;
    void *arg;
};

extern void pause_polling(u32 count);
extern void time_polling_init(void);
extern void prom_puts(const char *s);
extern void prom_print_hex(u32 value, int digits);
extern void prom_print_dec(u32 value);

static inline void delay_a_while(u32 n) { pause_polling(n); }

static inline int en7528_efuse_remarked(void)
{
    return !!(REG32(EFUSE0_DATA0) & BIT(0));
}

static inline int en7528_is_qfp(void)
{
    u32 e0 = REG32(EFUSE0_DATA0);
    return en7528_efuse_remarked() ? (((e0 >> 3) & 3) == 0)
                                   : (((e0 >> 1) & 3) == 0);
}

static inline int en7528_is_ddr3(void)
{
    if (en7528_efuse_remarked())
        return !((REG32(EFUSE1_DATA3) >> 8) & 1);
    return !((REG32(EFUSE0_DATA0) >> 5) & 1);
}

static inline int en7528_half_size(void)
{
    if (en7528_efuse_remarked())
        return (REG32(EFUSE1_DATA3) >> 9) & 1;
    return (REG32(EFUSE0_DATA0) >> 6) & 1;
}

static inline void en7528_ddr_phy_reset(void)
{
    DRAMC_WRITE_SET(BIT(28), DRAMC_PHYCTL1);
    DRAMC_WRITE_SET(BIT(25), DRAMC_GDDR3CTL1);
    delay_a_while(1);
    DRAMC_WRITE_CLEAR(BIT(28), DRAMC_PHYCTL1);
    DRAMC_WRITE_CLEAR(BIT(25), DRAMC_GDDR3CTL1);
}

#endif
