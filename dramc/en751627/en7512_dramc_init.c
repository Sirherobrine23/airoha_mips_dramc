/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en751627_ddr.h"

/* Package/revision-specific PLL and DDR3 setup from EN7516 V1.0. */
static int mode_command(u32 value, u32 command, u32 delay)
{
    u32 timeout = 0xffff;

    DRAMC_WRITE_REG(value, DRAMC_MRS);
    DRAMC_WRITE_REG(command, DRAMC_SPCMD);
    do {
        timeout--;
        if (DRAMC_READ_REG(DRAMC_SPCMDRESP) & command) {
            pause_polling(delay);
            DRAMC_WRITE_REG(0, DRAMC_SPCMD);
            return 0;
        }
    } while (timeout);
    return -1;
}

int en7512_dramc_init(void)
{
    u32 xtal, straps;
    u32 strap_reg;

    prom_puts(PKG_type == QFP ? "QFP IC\n" : "BGA IC\n");
    strap_reg = REG32(SYS_GLOBAL_PARM) & BIT(22) ? 0xbfa2ff28u : 0xbfa20174u;
    xtal = (REG32(strap_reg) >> 21) & 1;
    strap_reg = REG32(SYS_GLOBAL_PARM) & BIT(22) ? 0xbfa2ff28u : 0xbfa20174u;
    straps = REG32(strap_reg);
    prom_puts("Xtal:");
    prom_print_dec(xtal);
    prom_puts("\nDDR3 init.\n");
    REG32(0xbfb00040u) = 0;
    pause_polling(200);
    if (PKG_type == QFP) {
        DRAMC_REG(DRAMC_MISCTL0) = 0x7110000;
        DRAMC_REG(DRAMC_DRVCTL0) = 0xcc00cc00;
        DRAMC_REG(DRAMC_DRVCTL1) = 0xcc00cc00;
        DRAMC_REG(DRAMC_DDR2CTL) = 0xb48711ed;
    } else {
        DRAMC_REG(DRAMC_MISCTL0) = 0x7010000;
        DRAMC_REG(DRAMC_DRVCTL0) = 0xaa33aa33;
        DRAMC_REG(DRAMC_DRVCTL1) = 0xaa33aa33;
        DRAMC_REG(DRAMC_DDR2CTL) = 0xc38722ed;
    }
    if ((REG32(0xbfb0005cu) & 0xffff) == 1) {
        if (PKG_type == QFP) {
            DRAMC_REG(DRAMC_MEMPLL_DIVIDER) =
                (DRAMC_REG(DRAMC_MEMPLL_DIVIDER) & 0xfffffdcf) | 0x220;
        } else {
            DRAMC_REG(DRAMC_MEMPLL_DIVIDER) =
                (DRAMC_REG(DRAMC_MEMPLL_DIVIDER) & 0xfffffdc8) | 0x220;
        }
        DRAMC_REG(DRAMC_MEMPLL1) = (DRAMC_REG(DRAMC_MEMPLL1) & 0xffffffe1) | 0x12;
        if (xtal == 0) {
            DRAMC_REG(DRAMC_MEMPLL0) = (DRAMC_REG(DRAMC_MEMPLL0) & 0xfffc0808) | 0x181;
        } else {
            DRAMC_REG(DRAMC_MEMPLL0) = (DRAMC_REG(DRAMC_MEMPLL0) & 0xfffc0808) | 0x131;
        }
        pause_polling(1);
        DRAMC_REG(DRAMC_MEMPLL0) = DRAMC_REG(DRAMC_MEMPLL0) | 0x2000000;
        pause_polling(0x14);
        DRAMC_REG(DRAMC_MEMPLL1) = DRAMC_REG(DRAMC_MEMPLL1) | 0x100;
        if (PKG_type == QFP) {
            DRAMC_REG(DRAMC_MEMPLL6) = (DRAMC_REG(DRAMC_MEMPLL6) & 0xf3cf0000) | 0x88a;
            DRAMC_REG(DRAMC_MEMPLL9) = (DRAMC_REG(DRAMC_MEMPLL9) & 0xf3cf0000) | 0x88a;
            DRAMC_REG(DRAMC_MEMPLL12) = (DRAMC_REG(DRAMC_MEMPLL12) & 0xf3cf0000) | 0x88a;
        } else {
            DRAMC_REG(DRAMC_MEMPLL6) = (DRAMC_REG(DRAMC_MEMPLL6) & 0xf3cf0000) | 0x8ca;
            DRAMC_REG(DRAMC_MEMPLL9) = (DRAMC_REG(DRAMC_MEMPLL9) & 0xf3cf0000) | 0x8ca;
            DRAMC_REG(DRAMC_MEMPLL12) = (DRAMC_REG(DRAMC_MEMPLL12) & 0xf3cf0000) | 0x8ca;
        }
        DRAMC_REG(0x648) = 0xc0004ab2;
        pause_polling(1);
        DRAMC_REG(DRAMC_MEMPLL5) = (DRAMC_REG(DRAMC_MEMPLL5) & 0xf01fffff) | 0x2800000;
    } else if (PKG_type == QFP) {
        DRAMC_REG(DRAMC_MEMPLL_DIVIDER) = (DRAMC_REG(DRAMC_MEMPLL_DIVIDER) & 0xfffffdcf) | 0x220;
        DRAMC_REG(DRAMC_MEMPLL1) = (DRAMC_REG(DRAMC_MEMPLL1) & 0xffffffe1) | 0x12;
        DRAMC_REG(DRAMC_MEMPLL0) = (DRAMC_REG(DRAMC_MEMPLL0) & 0xfbfc0808) | 0x4000135;
        DRAMC_REG(DRAMC_MEMPLL2) = (DRAMC_REG(DRAMC_MEMPLL2) & 0x1ffff) | 0x2b020000;
        DRAMC_REG(DRAMC_MEMPLL3) = (DRAMC_REG(DRAMC_MEMPLL3) & 0xffff0000) | 0x687;
        pause_polling(1);
        DRAMC_REG(DRAMC_MEMPLL0) = DRAMC_REG(DRAMC_MEMPLL0) | 0x2000000;
        pause_polling(0x14);
        DRAMC_REG(DRAMC_MEMPLL3) = DRAMC_REG(DRAMC_MEMPLL3) | 0x80000;
        pause_polling(1);
        DRAMC_REG(DRAMC_MEMPLL3) = DRAMC_REG(DRAMC_MEMPLL3) | 0x10000;
        pause_polling(0x14);
        DRAMC_REG(DRAMC_MEMPLL1) = DRAMC_REG(DRAMC_MEMPLL1) | 0x100;
        DRAMC_REG(DRAMC_MEMPLL6) = (DRAMC_REG(DRAMC_MEMPLL6) & 0xf3cf0000) | 0x8aa;
        DRAMC_REG(DRAMC_MEMPLL9) = (DRAMC_REG(DRAMC_MEMPLL9) & 0xf3cf0000) | 0x8aa;
        DRAMC_REG(DRAMC_MEMPLL12) = (DRAMC_REG(DRAMC_MEMPLL12) & 0xf3cf0000) | 0x8aa;
        DRAMC_REG(0x648) = 0xc0004ab2;
        pause_polling(1);
        DRAMC_REG(DRAMC_MEMPLL5) = (DRAMC_REG(DRAMC_MEMPLL5) & 0xf01fffff) | 0x4000000;
    } else {
        DRAMC_REG(DRAMC_MEMPLL_DIVIDER) = (DRAMC_REG(DRAMC_MEMPLL_DIVIDER) & 0xfffffdc8) | 0x220;
        DRAMC_REG(DRAMC_MEMPLL1) = (DRAMC_REG(DRAMC_MEMPLL1) & 0xffffffe1) | 0x12;
        DRAMC_REG(DRAMC_MEMPLL0) = (DRAMC_REG(DRAMC_MEMPLL0) & 0xfbfc0808) | 0x4000135;
        if (xtal == 0) {
            DRAMC_REG(DRAMC_MEMPLL2) = (DRAMC_REG(DRAMC_MEMPLL2) & 0x1ffff) | 0x3d700000;
            DRAMC_REG(DRAMC_MEMPLL3) = (DRAMC_REG(DRAMC_MEMPLL3) & 0xffff0000) | 0xb54;
        } else {
            DRAMC_REG(DRAMC_MEMPLL2) = (DRAMC_REG(DRAMC_MEMPLL2) & 0x1ffff) | 0x7df20000;
            DRAMC_REG(DRAMC_MEMPLL3) = (DRAMC_REG(DRAMC_MEMPLL3) & 0xffff0000) | 0x8a9;
        }
        pause_polling(1);
        DRAMC_REG(DRAMC_MEMPLL0) = DRAMC_REG(DRAMC_MEMPLL0) | 0x2000000;
        pause_polling(0x14);
        DRAMC_REG(DRAMC_MEMPLL3) = DRAMC_REG(DRAMC_MEMPLL3) | 0x80000;
        pause_polling(1);
        DRAMC_REG(DRAMC_MEMPLL3) = DRAMC_REG(DRAMC_MEMPLL3) | 0x10000;
        pause_polling(0x14);
        DRAMC_REG(DRAMC_MEMPLL1) = DRAMC_REG(DRAMC_MEMPLL1) | 0x100;
        DRAMC_REG(DRAMC_MEMPLL6) = (DRAMC_REG(DRAMC_MEMPLL6) & 0xf3cf0000) | 0x8aa;
        DRAMC_REG(DRAMC_MEMPLL9) = (DRAMC_REG(DRAMC_MEMPLL9) & 0xf3cf0000) | 0x8aa;
        DRAMC_REG(DRAMC_MEMPLL12) = (DRAMC_REG(DRAMC_MEMPLL12) & 0xf3cf0000) | 0x8aa;
        DRAMC_REG(0x648) = 0xc0004ab2;
        pause_polling(1);
        DRAMC_REG(DRAMC_MEMPLL5) = (DRAMC_REG(DRAMC_MEMPLL5) & 0xf01fffff) | 0x4000000;
    }
    DRAMC_REG(DRAMC_MEMPLL6) = DRAMC_REG(DRAMC_MEMPLL6) | 0x2000000;
    DRAMC_REG(DRAMC_MEMPLL9) = DRAMC_REG(DRAMC_MEMPLL9) | 0x2000000;
    DRAMC_REG(DRAMC_MEMPLL12) = DRAMC_REG(DRAMC_MEMPLL12) | 0x2000000;
    pause_polling(0x14);
    DRAMC_REG(DRAMC_MEMPLL_DIVIDER) = DRAMC_REG(DRAMC_MEMPLL_DIVIDER) | 0x10;
    pause_polling(1);
    DRAMC_REG(DRAMC_TEST2_4) = 0x1e00d10d;
    if (PKG_type == QFP) {
        DRAMC_REG(DRAMC_MCKDLY) = 0xc0100900;
    } else if (straps & BIT(4)) {
        DRAMC_REG(DRAMC_MCKDLY) = 0x40100900;
    } else {
        DRAMC_REG(DRAMC_MCKDLY) = 0x80100900;
    }
    pause_polling(200);
    DRAMC_REG(DRAMC_PADCTL4) = 0xa3;
    pause_polling(500);
    DRAMC_REG(DRAMC_CLK1DELAY) = 1;
    DRAMC_REG(DRAMC_IOCTL) = 0;
    DRAMC_REG(DRAMC_R0DQSIEN) = 0x10101010;
    DRAMC_REG(DRAMC_DQSCTL0) = 0x83080080;
    DRAMC_REG(DRAMC_DQSCTL1) = 0x12080080;
    if (PKG_type == QFP) {
        DRAMC_REG(DRAMC_PHYCTL1) = 0x80000000;
        DRAMC_REG(DRAMC_GDDR3CTL1) = 0x1000000;
    } else {
        DRAMC_REG(DRAMC_PHYCTL1) = 0;
        DRAMC_REG(DRAMC_GDDR3CTL1) = 0x11000000;
    }
    DRAMC_REG(DRAMC_ARBCTL0) = 0x80;
    DRAMC_REG(DRAMC_CLKENCTL) = 0x30000000;
    if (PKG_type == QFP) {
        DRAMC_REG(DRAMC_MCKDLY) = 0xc0300900;
    } else if (straps & BIT(4)) {
        DRAMC_REG(DRAMC_MCKDLY) = 0x40300900;
    } else {
        DRAMC_REG(DRAMC_MCKDLY) = 0x80300900;
    }
    pause_polling(1);
    DRAMC_WRITE_REG(PKG_type == QFP ? 0xf0740642 : 0xf0740682, DRAMC_CONF1);
    DRAMC_REG(DRAMC_DQSGCTL) = 0x80000011;
    DRAMC_REG(DRAMC_R0DQSIEN) = 0x10101010;
    DRAMC_REG(DRAMC_DQSCAL0) = 0xc8b8;
    if (PKG_type == QFP) {
        DRAMC_REG(DRAMC_DDR2CTL) = 0xb48711e5;
    } else {
        DRAMC_REG(DRAMC_DDR2CTL) = 0xc38722e5;
    }
    DRAMC_REG(DRAMC_DLLCONF) = 0xf1200f01;
    DRAMC_REG(DRAMC_LPDDR2) = 0x88000000;
    DRAMC_REG(0x158) = 0;
    DRAMC_REG(DRAMC_RKCFG) = 0x111190;
    if (PKG_type == QFP) {
        DRAMC_REG(DRAMC_CONF1) = 0xf0740642;
    } else {
        DRAMC_REG(DRAMC_CONF1) = 0xf0740682;
    }
    DRAMC_REG(DRAMC_PADCTL4) = 0xa7;
    pause_polling(1);
    if (mode_command(PKG_type == QFP ? 0x4208 : 0x4210, 1, 1) || mode_command(0x6000, 1, 1) ||
        mode_command(PKG_type == QFP ? 0x2006 : 0x2004, 1, 1) ||
        mode_command(PKG_type == QFP ? 0x1931 : 0x1b51, 1, 1))
        goto failed;
    if (PKG_type != KGD && mode_command(0x400, 0x10, 6))
        goto failed;
    DRAMC_WRITE_REG(0x1100, DRAMC_SPCMD);
    pause_polling(1);
    DRAMC_WRITE_REG(0x22a7, DRAMC_PADCTL4);
    DRAMC_WRITE_REG(0x88000000, DRAMC_LPDDR2);
    if (mode_command(0xffff, 0x20, 1))
        goto failed;
    DRAMC_WRITE_REG(0x104d2842, DRAMC_PD_CTRL);
    DRAMC_WRITE_REG(PKG_type == QFP ? 0xf0740642 : 0xf0740682, DRAMC_CONF1);
    DRAMC_WRITE_REG(PKG_type == QFP ? 0xb48711ed : 0xc38722ed, DRAMC_DDR2CTL);
    pause_polling(1);
    DRAMC_WRITE_REG(0, DRAMC_PADCTL1);
    DRAMC_WRITE_REG(PKG_type == QFP ? 0x33684552 : 0x447a4694, DRAMC_ACTIM0);
    pause_polling(1);
    DRAMC_WRITE_REG(PKG_type == QFP ? 0xa8830481 : 0xa88a0481, DRAMC_TEST2_3);
    DRAMC_WRITE_REG(PKG_type == QFP ? 0x650 : 0x660, DRAMC_ACTIM1);
    DRAMC_WRITE_REG(PKG_type == QFP ? 0xf5f3f : 0xf5f50, DRAMC_CONF2);
    DRAMC_WRITE_REG(0, DRAMC_PADCTL2);
    DRAMC_WRITE_REG(0xedcb000f, DRAMC_PADCTL7);
    DRAMC_WRITE_REG(0xc80008, DRAMC_DM_MONITOR);
    DRAMC_WRITE_REG(0x31313131, DRAMC_R0DELDLY);
    REG32(0xbfb00074u) = 0x105;
    pause_polling(2);
    prom_puts("DRAMC init done. \n");
    return 0;
failed:
    prom_puts("DRAMC Init Fail!\r\n");
    return -1;
}
