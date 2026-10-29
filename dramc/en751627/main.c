/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en751627_ddr.h"

extern int main(void);

static void read_efuse_macro(u32 base)
{
    REG32(base) = 8;
    while (!(REG32(base + 0xc) & 4))
        ;
    REG32(base) = 0xd;
    while (!(REG32(base + 0xc) & 2))
        ;
    REG32(base) = 0;
}

void start_spram(int calibrate)
{
    void (*next_stage)(void);

    uart_init();
    init_system(1);
    timerSet(1, 10000, 1, 1, 0);
    read_efuse_macro(EFUSE0_CTRL);
    read_efuse_macro(EFUSE1_CTRL);
    tlb_1g_config();
    if (calibrate == 1)
        main();
    local_flush_tlb_all();
    /* Normal TCBoot returns to boot2; recovery/flash writing belongs to
     * the standalone chainloader, not the DRAM calibration payload.
     */
    next_stage = (void (*)(void))(unsigned long)REG32(SYS_BOOT_JUMP);
    next_stage();
    for (;;)
        ;
}
