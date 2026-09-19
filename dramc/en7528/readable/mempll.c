/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Readable EN7528 DRAMC V1.8 memory-PLL setup/calibration. */
#include "en7528_ddr.h"

extern int dram_speed;

#define R(o)        DRAMC_READ_REG(o)
#define W(o, v)     DRAMC_WRITE_REG((v), (o))
#define SET(o, v)   DRAMC_WRITE_SET((v), (o))
#define UPDATE(o, mask, value) W((o), (R(o) & (u32)(mask)) | (u32)(value))

static void mempll_finish(u32 feedback, u32 phase)
{
    SET(DRAMC_MEMPLL1, 0x00000100);
    UPDATE(DRAMC_MEMPLL6,  0xf3cf0000, feedback);
    UPDATE(DRAMC_MEMPLL9,  0xf3cf0000, feedback);
    UPDATE(DRAMC_MEMPLL12, 0xf3cf0000, feedback);
    W(0x648, 0xc0004ab2);
    pause_polling(1);
    UPDATE(DRAMC_MEMPLL5, 0xf01fffff, phase);
    SET(DRAMC_MEMPLL6,  0x02000000);
    SET(DRAMC_MEMPLL9,  0x02000000);
    SET(DRAMC_MEMPLL12, 0x02000000);
    pause_polling(20);
    SET(DRAMC_MEMPLL_DIVIDER, 0x10);
    pause_polling(1);
}

void mempll_init_int(u32 xtal, u32 speed)
{
    u32 fb = (speed / 100u) << 4;

    dram_speed = (int)speed;
    UPDATE(DRAMC_MEMPLL_DIVIDER, 0xfffffdc8, 0x222);
    UPDATE(DRAMC_MEMPLL1,       0xffffffe1, 0x12);
    UPDATE(DRAMC_MEMPLL0,       0xfffc0808, xtal == 1 ? 0x131 : 0x181);
    pause_polling(1);
    SET(DRAMC_MEMPLL0, 0x02000000);
    pause_polling(20);

    SET(DRAMC_MEMPLL1, 0x100);
    UPDATE(DRAMC_MEMPLL6,  0xf3cf0000, 0x080a | fb);
    UPDATE(DRAMC_MEMPLL9,  0xf3cf0000, 0x080a | fb);
    UPDATE(DRAMC_MEMPLL12, 0xf3cf0000, 0x080a | fb);
    W(0x648, 0xc0004ab2);
    pause_polling(1);
    UPDATE(DRAMC_MEMPLL5, 0xf01fffff, 0x02800000);
    SET(DRAMC_MEMPLL6,  0x02000000);
    SET(DRAMC_MEMPLL9,  0x02000000);
    SET(DRAMC_MEMPLL12, 0x02000000);
    pause_polling(20);
    SET(DRAMC_MEMPLL_DIVIDER, 0x10);
    pause_polling(1);
}

void mempll_init_ddr1066(u32 xtal)
{
    (void)xtal;
    dram_speed = 1066;
    UPDATE(DRAMC_MEMPLL_DIVIDER, 0xfffffdc8, 0x222);
    UPDATE(DRAMC_MEMPLL1,       0xffffffe1, 0x12);
    UPDATE(DRAMC_MEMPLL0,       0xfbfc0808, 0x04000135);
    UPDATE(DRAMC_MEMPLL2,       0x0001ffff, 0x2b020000);
    UPDATE(DRAMC_MEMPLL3,       0xffff0000, 0x0687);
    pause_polling(1);
    SET(DRAMC_MEMPLL0, 0x02000000);
    pause_polling(20);
    SET(DRAMC_MEMPLL3, 0x00080000);
    pause_polling(1);
    SET(DRAMC_MEMPLL3, 0x00010000);
    pause_polling(20);
    mempll_finish(0x08aa, 0x04000000);
}

void mempll_init_ddr1333(u32 xtal)
{
    dram_speed = 1333;
    UPDATE(DRAMC_MEMPLL_DIVIDER, 0xfffffdc8, 0x222);
    UPDATE(DRAMC_MEMPLL1,       0xffffffe1, 0x12);
    UPDATE(DRAMC_MEMPLL0,       0xfbfc0808, 0x04000135);

    if (xtal == 1) {
        UPDATE(DRAMC_MEMPLL2, 0x0001ffff, 0x7df20000);
        UPDATE(DRAMC_MEMPLL3, 0xffff0000, 0x08a9);
    } else {
        UPDATE(DRAMC_MEMPLL2, 0x0001ffff, 0x3d700000);
        UPDATE(DRAMC_MEMPLL3, 0xffff0000, 0x0b54);
    }

    pause_polling(1);
    SET(DRAMC_MEMPLL0, 0x02000000);
    pause_polling(20);
    SET(DRAMC_MEMPLL3, 0x00080000);
    pause_polling(1);
    SET(DRAMC_MEMPLL3, 0x00010000);
    pause_polling(20);
    mempll_finish(0x08aa, 0x04000000);
}

int mt_mempll_cali(void)
{
    u32 pll2_done = 0, pll3_done = 0;
    u32 pll2_tap = 0, pll3_tap = 0;

    /* Enable MEMPLL jitter-meter calibration path. */
    W(DRAMC_DM_MONITOR, (R(DRAMC_DM_MONITOR) & 0x0000ffff) | 0x04000000);

    for (;;) {
        if (!pll2_done) {
            UPDATE(DRAMC_MEMPLL7, 0xfe0fffff, 0x01000000);
            W(DRAMC_MEMPLL7,
              (R(DRAMC_MEMPLL7) & 0x83ffffff) | (pll2_tap << 26));
        }
        if (!pll3_done) {
            UPDATE(0x634, 0xfe0fffff, 0x01000000);
            W(0x634, (R(0x634) & 0x83ffffff) | (pll3_tap << 26));
        }

        if (!pll2_done)
            SET(DRAMC_DM_MONITOR, 0x00000001);
        if (!pll3_done)
            SET(DRAMC_DM_MONITOR, 0x00004000);

        pause_polling(40);

        if (!pll2_done) {
            u32 jm = R(0x320);
            (void)R(0x324); /* vendor performs both status reads */
            pll2_done = ((jm >> 16) > 512u);
        }
        if (!pll3_done) {
            u32 jm = R(0x328);
            (void)R(0x32c);
            pll3_done = ((jm >> 16) > 512u);
        }

        if (!pll2_done) {
            ++pll2_tap;
            DRAMC_WRITE_CLEAR(0x00000001, DRAMC_DM_MONITOR);
        }
        if (!pll3_done) {
            ++pll3_tap;
            DRAMC_WRITE_CLEAR(0x00004000, DRAMC_DM_MONITOR);
        }

        if (pll2_done && pll3_done) {
            SET(DRAMC_MEMPLL6, 0x8);
            SET(DRAMC_MEMPLL12, 0x8);
            return 0;
        }
        if (pll2_tap >= 32 || pll3_tap >= 32) {
            SET(DRAMC_MEMPLL6, 0x8);
            SET(DRAMC_MEMPLL12, 0x8);
            return -1;
        }
    }
}
