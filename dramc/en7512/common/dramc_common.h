/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef EN7512_DRAMC_COMMON_H
#define EN7512_DRAMC_COMMON_H

#include <stdint.h>
#include <asm/bit.h>
#include <asm/io.h>
#include <dramc/regs.h>
#include <timer.h>
#include <uart.h>

typedef uint32_t u32;
typedef int32_t s32;
typedef uint8_t u8;
typedef int8_t s8;

#define MMIO32(addr)		readl(addr)
#define REG32(addr)		readl(addr)

#define SYS_REVISION		0xbfb0005cu
#define SYS_CHIP_ID		0xbfb00064u
#define SYS_HWTRAP		0xbfb00240u
#define SYS_BOOT_JUMP		0xbfb00280u
#define SYS_GLOBAL_PARM		0xbfb00284u

#define DRAMC0_BASE		0xbfb20000u
#define DRAMC_REG(off)		readl(DRAMC0_BASE + (off))
#define DRAMC_READ_REG(off)	DRAMC_REG(off)
#define DRAMC_WRITE_REG(v, off)                                                                    \
	do {                                                                                       \
		DRAMC_REG(off) = (u32)(v);                                                         \
	} while (0)
#define DRAMC_WRITE_SET(v, off)                                                                    \
	do {                                                                                       \
		DRAMC_REG(off) |= (u32)(v);                                                        \
	} while (0)
#define DRAMC_WRITE_CLEAR(v, off)                                                                  \
	do {                                                                                       \
		DRAMC_REG(off) &= ~(u32)(v);                                                       \
	} while (0)

#define ADDR_READ_REG(addr)	readl(addr)
#define ADDR_WRITE_REG(v, addr)                                                                    \
	do {                                                                                       \
		readl(addr) = (u32)(v);                                                            \
	} while (0)

#define DDR3			0
#define DDR2			1
#define KGD			0
#define BGA1			1
#define BGA2			2
#define QFP			3

#define DRAM_BASE_ADDR		0xa0000000u
#define DQ_DATA_WIDTH		16
#define DQS_NUMBER		2
#define DQS_BIT_NUMBER		8
#define TX_DQ_DATA_WIDTH	16
#define TX_DQS_NUMBER		2
#define MAX_RX_DQDLY_TAPS	16
#define MAX_RX_DQSDLY_TAPS	96
#define MAX_TX_DQDLY_TAPS	16
#define MAX_TX_DQSDLY_TAPS	16

#define DLE_START		0
#define DLE_END			15
#define DLE_MAX			16

#define DQS_GW_FINE_START	0
#define DQS_GW_FINE_END		95
#define DQS_GW_FINE_COUNT	96
#define DQS_GW_WORDS_PER_COARSE	3

/* Selected via -I dramc/en7512/soc/<soc> at build time. */
#include "en7512_soc.h"

#ifndef DQS_GW_COARSE_COUNT
#define DQS_GW_COARSE_COUNT	(DQS_GW_COARSE_END - DQS_GW_COARSE_START + 1)
#endif
#ifndef DQS_GW_WORDS
#define DQS_GW_WORDS	(DQS_GW_COARSE_COUNT * DQS_GW_WORDS_PER_COARSE)
#endif
#ifndef DRAMC_POLL_TIMEOUT
#define DRAMC_POLL_TIMEOUT	0x7fffffffu
#endif
#ifndef DRAM_MAX_SIZE_MIB
#define DRAM_MAX_SIZE_MIB	1024u
#endif

static inline void delay_a_while(u32 n)
{
	pause_polling(n);
}

static inline void en7512_ddr_phy_reset(void)
{
	DRAMC_WRITE_SET(BIT(28), DRAMC_PHYCTL1);
	DRAMC_WRITE_SET(BIT(25), DRAMC_GDDR3CTL1);
	delay_a_while(1);
	DRAMC_WRITE_CLEAR(BIT(28), DRAMC_PHYCTL1);
	DRAMC_WRITE_CLEAR(BIT(25), DRAMC_GDDR3CTL1);
}

#ifndef dramc_phy_reset
#define dramc_phy_reset()	en7512_ddr_phy_reset()
#endif

struct dramc_test_case {
	int (*fn)(unsigned int start, unsigned int len, void *arg);
	unsigned int start;
	unsigned int len;
	void *arg;
};

/* Shared DRAMC calibration, tests, and init helpers. */
int en7512_dramc_init(void);
int dramc_calib(void);
int dramc_main(void);
int do_dqs_gw_calib_1(void);
int do_dle_calib(void);
int do_sw_rx_dq_dqs_calib(void);
int do_sw_tx_dq_dqs_calib(void);
void dle_factor_handler(unsigned int value);
int dramc_dma_test(unsigned int start, unsigned int len, void *arg);
int dramc_ta1(unsigned int start, unsigned int len, void *arg);
int check_column_bank(void);
void set_TRFC(void);
void dramc_reg_dump(void);

int dram_spcmd(u32 command, u32 delay);
int dram_mode_command(u32 mrs, u32 command, u32 delay);
void dram_mempll_finish(u32 feedback, u32 phase);

/* SoC-supplied hooks. */
void soc_dramc_banner(void);
void soc_detect_dram_pkg(void);
void soc_post_calib_geometry(void);
u32 soc_effective_dram_size(u32 size);
void soc_finish_dramc(int calib_failed);

extern int PKG_type;
extern int dram_type;
extern int dram_size;
extern int opt_gw_coarse_value;
extern int opt_gw_fine_value;
extern int opt_dle_value;

#endif
