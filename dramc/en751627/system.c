/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en751627_ddr.h"

void en751627_setup_clk(u32 cpu_mhz)
{
    if (cpu_mhz == 900) {
        REG32(SYS_GLOBAL_PARM) = (REG32(SYS_GLOBAL_PARM) & 0xffc00fffu) | 0xe1000;
        return;
    }
    REG32(0xbfa201a0u) = 0x12;
    switch (cpu_mhz) {
    case 800:
        REG32(0xbfa201ecu) = 0x05101f08;
        REG32(0xbfa201fcu) = 0x03000000;
        break;
    case 700:
        REG32(0xbfa201ecu) = 0x05101b08;
        REG32(0xbfa201fcu) = 0x02800000;
        break;
    case 450:
        REG32(0xbfa201ecu) = 0x05201108;
        REG32(0xbfa201fcu) = 0x03800000;
        break;
    case 225:
        REG32(0xbfa201ecu) = 0x05301108;
        REG32(0xbfa201fcu) = 0x03800000;
        break;
    default:
        REG32(SYS_GLOBAL_PARM) = (REG32(SYS_GLOBAL_PARM) & 0xffc00fffu) | 0xe1000;
        REG32(0xbfa201a0u) = 0;
        return;
    }
    REG32(0xbfa201f8u) = 0x402;
    REG32(0xbfa201f8u) = 0x406;
    REG32(0xbfa201a0u) = 0;
    REG32(SYS_GLOBAL_PARM) =
        (REG32(SYS_GLOBAL_PARM) & 0xffc00fffu) | (((cpu_mhz & 0xfff) >> 2) << 12);
}

void set_spi_ctrler_ecc(void)
{
    u32 strap_reg, shift;
    u32 hir = REG32(SYS_CHIP_ID) & 0xffff0000u;

    if (hir == 0x00070000u)
        REG32(SYS_GLOBAL_PARM) &= ~BIT(23);
    strap_reg = REG32(SYS_GLOBAL_PARM) & BIT(22) ? 0xbfa2ff28u : 0xbfa20174u;
    shift = REG32(SYS_GLOBAL_PARM) & BIT(22) ? 19 : 0;
    if (!(REG32(strap_reg) & BIT(shift)))
        REG32(SYS_GLOBAL_PARM) = (REG32(SYS_GLOBAL_PARM) & ~BIT(23)) | BIT(23);
    else
        REG32(SYS_GLOBAL_PARM) &= ~BIT(23);
}

void init_system(int preserve_clock)
{
    REG32(SYS_GLOBAL_PARM) =
        (REG32(SYS_GLOBAL_PARM) & ~BIT(22)) | (((REG32(0xbfb0009cu) & 1) ^ 1) << 22);
    set_spi_ctrler_ecc();
    if (REG32(SYS_GLOBAL_PARM) & BIT(22))
        REG32(SYS_GLOBAL_PARM) = (REG32(SYS_GLOBAL_PARM) & 0xffc00fffu) | 0x28000;
    else if (!preserve_clock) {
        if ((REG32(SYS_REVISION) & 0xffff) < 3)
            en751627_setup_clk(800);
        else if (!(REG32(SYS_HWTRAP) & 3))
            en751627_setup_clk(900);
        else
            en751627_setup_clk(450);
        REG32(0xbfb0094cu) &= ~1u;
        REG32(0xbfa201c4u) = (REG32(0xbfa201c4u) & 0xfffffff1u) | 8;
    } else
        REG32(SYS_GLOBAL_PARM) = (REG32(SYS_GLOBAL_PARM) & 0xffc00fffu) | 0xe1000;
}
