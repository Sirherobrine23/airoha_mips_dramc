/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "chainloader.h"

/* EN7516 BootROM launch records, also used by the SDK spram_ext stage. */
struct cpu_launch {
	u32 pc, gp, sp, a0;
	u32 reserved[3];
	u32 flags;
};

#define CPU_LAUNCH_BASE 0xbfa40140u
#define LAUNCH_READY 1u
#define LAUNCH_GO 2u
#define LAUNCH_GONE 4u
#define LAUNCH_TIMEOUT 1000000u

extern void en751627_secondary_park(void);

int en751627_prepare_cpus(void)
{
	volatile struct cpu_launch *launch = (volatile void *)CPU_LAUNCH_BASE;
	u32 cpu;

	/* Re-enter the BootROM wait loop after clearing Cause.WP on each VPE.
	 * The callback uses no stack or global pointer. Unavailable/already
	 * launched contexts are left alone, as in the SDK amon_cpu_avail().
	 */
	for (cpu = 1; cpu < 4; cpu++) {
		u32 flags = launch[cpu].flags;
		u32 timeout = LAUNCH_TIMEOUT;

		if (!(flags & LAUNCH_READY) || (flags & (LAUNCH_GO | LAUNCH_GONE)))
			continue;
		launch[cpu].pc = (u32)(uintptr_t)en751627_secondary_park;
		launch[cpu].gp = 0;
		launch[cpu].sp = 0;
		launch[cpu].a0 = 0;
		__asm__ volatile("sync" ::: "memory");
		launch[cpu].flags = LAUNCH_GO;
		__asm__ volatile("sync" ::: "memory");
		while (!(launch[cpu].flags & LAUNCH_READY))
			if (!--timeout)
				return -1;
		__asm__ volatile("sync" ::: "memory");
	}
	return 0;
}
