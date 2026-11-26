/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <timer.h>

static unsigned int timer_ldv_reg(unsigned int timer)
{
	return CR_TIMER_BASE + 0x04u + timer * 8u;
}

void timer_Configure(unsigned int n, unsigned int en, unsigned int mode, unsigned int halt)
{
	unsigned int w = readl(CR_TIMER_CTL);

	if (en & 0xffu)
		w |= (1u << (n & 0xffu)) | ((mode & 1u) << ((n & 0xffu) + 8u)) |
		     ((halt & 1u) << ((n & 0xffu) + 26u));
	else
		w &= ~(1u << (n & 0xffu));
	writel(w, CR_TIMER_CTL);
}

void timerSet(unsigned int n, unsigned int time, unsigned int en, unsigned int mode,
	      unsigned int halt)
{
	unsigned int w = soc_get_hclk_mhz() * 1000u / 2u;

	writel(w * time, timer_ldv_reg(n));
	timer_Configure(n, en, mode, halt);
}

void time_polling_init(void)
{
	timerSet(1, TIMERTICKS_10MS, TIMER_ENABLE, TIMER_TOGGLEMODE, TIMER_HALTDISABLE);
}

void pause_polling(unsigned int us)
{
	volatile unsigned int now, last, ticks, ldv;
	unsigned int wait = us * (soc_get_hclk_mhz() / 2u);

	ldv = readl(CR_TIMER1_LDV);
	ticks = 0;
	last = readl(CR_TIMER1_VLR);
	do {
		now = readl(CR_TIMER1_VLR);
		ticks += last >= now ? last - now : ldv - now + last;
		last = now;
	} while (ticks < wait);
}
