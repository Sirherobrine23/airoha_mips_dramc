/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en7528_ddr.h"

/*
 * Readable start_spram() recovery.  This path was validated from SRAM with the
 * EN7528 DRAMC V1.8 training image.
 */
extern int spram_preprocess(void);
extern void spram_postprocess(void);
extern int main(void);

unsigned char init_task_union[2560];
unsigned long kernelsp;

int start_spram(int is_bootext)
{
    void (*next_stage)(void);

    spram_preprocess();

    if (is_bootext == 1)
        main();

    spram_postprocess();

    next_stage = (void (*)(void))(unsigned long)REG32(SYS_BOOT_JUMP);
    next_stage();

    return 0;
}
