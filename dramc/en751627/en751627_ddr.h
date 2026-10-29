/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef EN751627_DDR_H
#define EN751627_DDR_H

/* EN7516 DRAMC V1.0, recovered from the supplied big-endian objects. */
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned char u8;
typedef signed char s8;

#include <asm/bit.h>
#define MMIO32(addr) (*(volatile u32 *)(unsigned long)(addr))
#define REG32(addr) MMIO32(addr)

#define SYS_GLOBAL_PARM 0xbfb00284u
#define SYS_BOOT_JUMP 0xbfb00280u
#define SYS_CHIP_ID 0xbfb00064u
#define SYS_REVISION 0xbfb0005cu
#define SYS_HWTRAP 0xbfb00240u
#define EFUSE0_CTRL 0xbfbf8200u
#define EFUSE0_DATA0 0xbfbf8214u
#define EFUSE1_CTRL 0xbfaa0000u

#define DRAMC0_BASE 0xbfb20000u
#define DRAMC_REG(off) MMIO32(DRAMC0_BASE + (off))
#define DRAMC_READ_REG(off) DRAMC_REG(off)
#define DRAMC_WRITE_REG(v, off)                                                                    \
    do {                                                                                           \
        DRAMC_REG(off) = (u32)(v);                                                                 \
    } while (0)
#define DRAMC_WRITE_SET(v, off)                                                                    \
    do {                                                                                           \
        DRAMC_REG(off) |= (u32)(v);                                                                \
    } while (0)
#define DRAMC_WRITE_CLEAR(v, off)                                                                  \
    do {                                                                                           \
        DRAMC_REG(off) &= ~(u32)(v);                                                               \
    } while (0)
#define ADDR_READ_REG(addr) MMIO32(addr)
#define ADDR_WRITE_REG(v, addr)                                                                    \
    do {                                                                                           \
        MMIO32(addr) = (u32)(v);                                                                   \
    } while (0)

#include "dramc/regs.h"

#define DDR3 0
#define DDR2 1
#define KGD 0
#define BGA1 1
#define BGA2 2
#define QFP 3

/* The vendor TLB setup maps the complete DRAM through KSEG2. */
#define DRAM_PROBE_BASE 0xc0080000u
#define DRAM_BASE_ADDR 0xa0000000u
#define DQ_DATA_WIDTH 16
#define DQS_NUMBER 2
#define DQS_BIT_NUMBER 8
#define TX_DQ_DATA_WIDTH 16
#define TX_DQS_NUMBER 2
#define MAX_RX_DQDLY_TAPS 16
#define MAX_RX_DQSDLY_TAPS 96
#define MAX_TX_DQDLY_TAPS 16
#define MAX_TX_DQSDLY_TAPS 16
#define DLE_START 0
#define DLE_END 15
#define DLE_MAX 16
#define DQS_GW_COARSE_START 6
#define DQS_GW_COARSE_END 20
#define DQS_GW_COARSE_COUNT 15
#define DQS_GW_FINE_START 0
#define DQS_GW_FINE_END 95
#define DQS_GW_FINE_COUNT 96
#define DQS_GW_WORDS_PER_COARSE 3
#define DQS_GW_WORDS 45
#define HW_DQS_GW_COUNTER 0x78787878u

extern void pause_polling(u32 count);
extern void time_polling_init(void);
extern void prom_puts(const char *s);
extern void prom_print_hex(u32 value, int digits);
extern void prom_print_dec(u32 value);

static inline void delay_a_while(u32 n)
{
    pause_polling(n);
}

static inline void en751627_ddr_phy_reset(void)
{
    DRAMC_WRITE_SET(BIT(28), DRAMC_PHYCTL1);
    DRAMC_WRITE_SET(BIT(25), DRAMC_GDDR3CTL1);
    delay_a_while(1);
    DRAMC_WRITE_CLEAR(BIT(28), DRAMC_PHYCTL1);
    DRAMC_WRITE_CLEAR(BIT(25), DRAMC_GDDR3CTL1);
}

int en7512_dramc_init(void);
int dramc_calib(void);
int do_dqs_gw_calib_1(void);
int do_dle_calib(void);
int do_sw_rx_dq_dqs_calib(void);
int do_sw_tx_dq_dqs_calib(void);
void dle_factor_handler(u32 value);
void set_TRFC(void);
int check_column_bank(void);
void init_system(int preserve_clock);
void en751627_setup_clk(u32 cpu_mhz);
void uart_init(void);
void timerSet(u32 timer, u32 time, u32 enable, u32 mode, u32 halt);
void local_flush_tlb_all(void);
void tlb_1g_config(void);
extern int PKG_type, dram_type, dram_size;
extern int opt_gw_coarse_value, opt_gw_fine_value, opt_dle_value;

#endif
