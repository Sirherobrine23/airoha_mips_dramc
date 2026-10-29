/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en751627_ddr.h"

/* CP0/TLB instructions are the only non-C operations in this module. */
#define CP0_READ(reg)                                                                              \
    ({                                                                                             \
        u32 v;                                                                                     \
        __asm__ volatile("mfc0 %0,$" #reg : "=r"(v));                                              \
        v;                                                                                         \
    })
#define CP0_WRITE(reg, val)                                                                        \
    __asm__ volatile("mtc0 %0,$" #reg "\n\tehb" ::"r"((u32)(val)) : "memory")

void local_flush_tlb_all(void)
{
    u32 saved = CP0_READ(10);
    u32 index;

    CP0_WRITE(2, 0);
    CP0_WRITE(3, 0);
    /* The preserved EN7516 object initializes g_TLBSize to 32. */
    for (index = 0; index < 32; index++) {
        CP0_WRITE(10, 0x80000000u + (index << 27));
        CP0_WRITE(0, index);
        __asm__ volatile("tlbwi\n\tehb" ::: "memory");
    }
    CP0_WRITE(10, saved);
}

void update_tlb_wired(u32 virtual, u32 physical, u32 index, u32 shift, u32 mask)
{
    u32 asid = CP0_READ(10) & 0xff;

    CP0_WRITE(0, index);
    CP0_WRITE(5, mask);
    CP0_WRITE(10, asid | (virtual & (0xfffffffeu << shift)));
    CP0_WRITE(2, ((physical & (~0u << shift)) >> 6) | 0x417);
    CP0_WRITE(3, (((physical + BIT(shift)) & (~0u << shift)) >> 6) | 0x417);
    __asm__ volatile("tlbwi\n\tehb" ::: "memory");
}

void tlb_1g_config(void)
{
    u32 index;

    local_flush_tlb_all();
    for (index = 0; index < 8; index++)
        update_tlb_wired(0xc0000000u + (index << 27), 0x80000000u + (index << 27), index, 26,
                         0x07ffe000);
}
