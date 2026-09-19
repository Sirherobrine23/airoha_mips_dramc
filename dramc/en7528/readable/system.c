/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en7528_ddr.h"

/*
 * EN7528 DRAMC V1.8 system/eFuse setup reconstructed from the XC220-G3
 * release_bsp system.o.  This is intentionally kept freestanding and close
 * to the vendor control flow because it runs before DRAM is available.
 */

#define CLK_CTRL_A0             0xbfa201a0u
#define CLK_PLL_CFG             0xbfa201ecu
#define CLK_PLL_LF              0xbfa201f8u
#define CLK_PLL_FRAC            0xbfa201fcu
#define SYS_MISC_094C           0xbfb0094cu
#define SYS_MISC_01C4           0xbfa201c4u

#define EFUSE0_DATA1            0xbfbf8218u

static void global_set_sysclk(u32 mhz)
{
    u32 v = REG32(SYS_GLOBAL_PARM);

    v &= ~(0x3ffu << 10);
    v |= (mhz & 0x3ffu) << 10;
    REG32(SYS_GLOBAL_PARM) = v;
}

static void global_set_package(u32 package)
{
    u32 v = REG32(SYS_GLOBAL_PARM);

    v &= ~0xfu;
    v |= package & 0xfu;
    REG32(SYS_GLOBAL_PARM) = v;
}

static void global_set_flag(unsigned int bit, int set)
{
    u32 v = REG32(SYS_GLOBAL_PARM);

    v &= ~BIT(bit);
    if (set)
        v |= BIT(bit);
    REG32(SYS_GLOBAL_PARM) = v;
}

static void en751627_setup_clk(u32 cpu_mhz)
{
    u32 pll_cfg = 0;
    u32 pll_frac = 0;

    /* 900 MHz is the strap/default path; the PLL is already running. */
    if (cpu_mhz == 900) {
        global_set_sysclk(225);
        return;
    }

    REG32(CLK_CTRL_A0) = 0x12;

    switch (cpu_mhz) {
    case 1100:
        pll_cfg = 0x04001508;
        pll_frac = 0x04800000;
        break;
    case 1050:
        pll_cfg = 0x04001408;
        pll_frac = 0x04400000;
        break;
    case 1000:
        pll_cfg = 0x05102708;
        pll_frac = 0x04000000;
        break;
    case 975:
        pll_cfg = 0x05102608;
        pll_frac = 0x03e00000;
        break;
    case 950:
        pll_cfg = 0x05102508;
        pll_frac = 0x03c00000;
        break;
    case 925:
        pll_cfg = 0x05102408;
        pll_frac = 0x03a00000;
        break;
    case 800:
        pll_cfg = 0x05101f08;
        pll_frac = 0x03000000;
        break;
    case 700:
        pll_cfg = 0x05101b08;
        pll_frac = 0x02800000;
        break;
    case 500:
        pll_cfg = 0x04101308;
        pll_frac = 0x04000000;
        break;
    case 450:
        pll_cfg = 0x05201108;
        pll_frac = 0x03800000;
        break;
    case 225:
        pll_cfg = 0x05301108;
        pll_frac = 0x03800000;
        break;
    default:
        global_set_sysclk(225);
        REG32(CLK_CTRL_A0) = 0;
        return;
    }

    REG32(CLK_PLL_CFG) = pll_cfg;
    REG32(CLK_PLL_FRAC) = pll_frac;
    REG32(CLK_PLL_LF) = 0x402;
    REG32(CLK_PLL_LF) = 0x406;
    REG32(CLK_CTRL_A0) = 0;

    /* V1.8 stores CPU/4 in SYS_GLOBAL_PARM.sysclk. */
    global_set_sysclk(cpu_mhz >> 2);
}

void set_spi_ctrler_ecc(void)
{
    u32 chip = REG32(SYS_CHIP_ID) & 0xffff0000u;

    /* Present in the vendor object before the final boot-strap override. */
    if (chip == 0x00070000u)
        global_set_flag(8, 0);

    /* SYS_BOOT_CFG[4] selects whether controller ECC is exposed. */
    global_set_flag(8, !(REG32(SYS_BOOT_CFG) & BIT(4)));
}

static int efuse_wait(u32 status, unsigned int bit)
{
    int retries = 3;

    do {
        pause_polling(1000);
        if (REG32(status) & BIT(bit))
            return 0;
    } while (--retries);

    return -1;
}

/*
 * Some EN75xx revisions use a 500 MHz system-clock path selected by package
 * eFuse data.  This reproduces the exact V1.8 tests in system.o.
 */
static int use_500mhz_clock(void)
{
    u32 chip = REG32(SYS_CHIP_ID) & 0xffff0000u;
    u32 e0, e1;

    if (chip != 0x00090000u)
        return 0;

    e0 = REG32(EFUSE0_DATA0);
    e1 = REG32(EFUSE0_DATA1);

    if (e0 & BIT(0)) {
        if (((e0 >> 2) & 0x000c0000u) != 0x000c0000u)
            return 0;
        return !!((e1 >> 9) & 1u);
    }

    if ((e0 & 0x000c0000u) != 0x000c0000u)
        return 0;

    return !!(e1 & BIT(8));
}

void init_system(int is_bootext)
{
    u32 e0, e3, package;

    /* SYS_GLOBAL_PARM.isFpga = !HWCONF[0]. */
    global_set_flag(9, !(REG32(SYS_HWCONF) & BIT(0)));
    set_spi_ctrler_ecc();

    if (REG32(SYS_GLOBAL_PARM) & BIT(9)) {
        global_set_sysclk(64);
        return;
    }

    if (use_500mhz_clock()) {
        en751627_setup_clk(500);
    } else if (is_bootext) {
        en751627_setup_clk(900);
    } else if (REG32(SYS_HWTRAP) & 3u) {
        en751627_setup_clk(450);
    } else {
        en751627_setup_clk(900);
    }

    REG32(SYS_MISC_094C) &= ~1u;
    REG32(SYS_MISC_01C4) = (REG32(SYS_MISC_01C4) & ~0xeu) | 8u;

    /* The EN7528 tcboot header/build selects EFUSE_CLK_WIDTH=3. */
    REG32(EFUSE0_CLK_WIDTH) = (REG32(EFUSE0_CLK_WIDTH) & ~7u) | 3u;
    REG32(EFUSE1_CLK_WIDTH) = (REG32(EFUSE1_CLK_WIDTH) & ~7u) | 3u;

    time_polling_init();

    /* Enable Fsource/read path for the second macro. */
    REG32(EFUSE0_CTRL) = 0;
    REG32(EFUSE1_CTRL) = 8;
    if (efuse_wait(EFUSE1_STATUS, 2) < 0)
        prom_puts("\nError: Fsource read timeout\n");

    /* Read first macro. */
    REG32(EFUSE0_CTRL) = 5;
    if (efuse_wait(EFUSE0_STATUS, 1) < 0)
        prom_puts("\nError: EFUSE 1 read timeout\n");

    /* Read second macro. */
    REG32(EFUSE1_CTRL) = 13;
    if (efuse_wait(EFUSE1_STATUS, 1) < 0)
        prom_puts("\nError: EFUSE 2 read timeout\n");

    e0 = REG32(EFUSE0_DATA0);
    e3 = REG32(EFUSE1_DATA3);

    if (!(e0 & BIT(0)))
        package = ((e0 >> 1) & 3u) | ((e3 >> 8) & 4u);
    else
        package = ((e0 >> 3) & 3u) | ((e3 >> 9) & 4u);

    global_set_package(package);
}
