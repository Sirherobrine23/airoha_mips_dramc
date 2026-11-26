/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef ECONET_TIMER_H
#define ECONET_TIMER_H

#include <stdint.h>
#include <asm/io.h>

#define CR_TIMER_BASE		0xbfbf0100u
#define CR_TIMER_CTL		(CR_TIMER_BASE + 0x00u)
#define CR_TIMER0_LDV		(CR_TIMER_BASE + 0x04u)
#define CR_TIMER0_VLR		(CR_TIMER_BASE + 0x08u)
#define CR_TIMER1_LDV		(CR_TIMER_BASE + 0x0cu)
#define CR_TIMER1_VLR		(CR_TIMER_BASE + 0x10u)

#define TIMER_ENABLE		1u
#define TIMER_TOGGLEMODE	1u
#define TIMER_HALTDISABLE	0u
#define TIMERTICKS_10MS		10u

static inline void watchdog_kick(void)
{
	uint32_t val = readl(CR_TIMER_CTL);

	val &= 0xffc0ffffu;
	val |= 0x00200000u;
	writel(val, CR_TIMER_CTL);
	__asm__ volatile("sync" ::: "memory");
}

/* Supplied by each SoC DRAMC runtime to report HCLK in MHz. */
unsigned int soc_get_hclk_mhz(void);

void timer_Configure(unsigned int n, unsigned int en, unsigned int mode, unsigned int halt);
void timerSet(unsigned int n, unsigned int time, unsigned int en, unsigned int mode,
	      unsigned int halt);
void time_polling_init(void);
void pause_polling(unsigned int us);

#endif
