/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <version.h>
#include "dramc_common.h"

#define ST0_IM	0x0000ff00u

unsigned char init_task_union[2560];
unsigned long kernelsp;

static inline void en751221_change_cp0_status(unsigned int clear, unsigned int set)
{
	unsigned int v;

	__asm__ __volatile__("mfc0 %0,$12" : "=r"(v));
	v = (v & ~clear) | set;
	__asm__ __volatile__("mtc0 %0,$12" ::"r"(v));
}

unsigned int soc_get_hclk_mhz(void)
{
	return isFPGA ? 32u : ((readl(SYS_GLOBAL_PARM) >> 12) & 0x3ffu);
}

void soc_dramc_banner(void)
{
	prom_puts("EN751221 DRAMC v1.2.2 - " PLAIN_VERSION "\r\n\r\n");
}

void soc_detect_dram_pkg(void)
{
	if ((readl(ECONET_PDIDR) & 0xffffu) == 1u) {
		if (isEN7526D || isEN7513) {
			dram_type = DDR3;
			PKG_type = BGA1;
		} else if (!((readl(CR_AHB_HWCONF) >> 4) & 1u)) {
			dram_type = DDR3;
			PKG_type = BGA2;
		} else {
			PKG_type = KGD;
			dram_type = EFUSE_IS_DDR3 ? DDR3 : DDR2;
		}
	} else {
		if (isLQFP) {
			PKG_type = KGD;
			dram_type = EFUSE_IS_DDR3 ? DDR3 : DDR2;
		} else {
			PKG_type = (isEN7526D || isEN7513) ? BGA1 : BGA2;
			dram_type = (int)((readl(CR_AHB_HWCONF) >> 4) & 1u);
		}
	}
}

void soc_post_calib_geometry(void)
{
	if (PKG_type == KGD && EFUSE_Fix32MB)
		DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_CONF1) & ~0x300u) | 0x100u, DRAMC_CONF1);
	else
		check_column_bank();
}

u32 soc_effective_dram_size(u32 size)
{
	return size;
}

void soc_finish_dramc(int calib_failed)
{
	u32 v = readl(SYS_GLOBAL_PARM);

	v = (v & ~0xfffu) | ((u32)dram_size & 0xfffu);
	writel(v, SYS_GLOBAL_PARM);

	set_TRFC();

	if ((DRAMC_READ_REG(DRAMC_MEMPLL6) & 0xffu) == 0xaau)
		prom_puts("DDR Speed: 1066Mhz\n");
	else if ((DRAMC_READ_REG(DRAMC_MEMPLL6) & 0xffu) == 0x8au)
		prom_puts("DDR Speed: 800Mhz\n");

	DRAMC_WRITE_SET(BIT(31), DRAMC_DQSCAL0);
	DRAMC_WRITE_SET(0x710u, DRAMC_PERFCTL0);

	prom_puts("\r\ncalibration status: ");
	prom_print_dec((u32)calib_failed);
	prom_puts("\r\n");
}

void start_spram(void)
{
	void (*jump_addr)(void);

	uart_init();

	if (!isFPGA || !isEN7526c) {
		if (isEN7513 || isEN7513G || isEN7512)
			readl(0xbfa20174u) = (readl(0xbfa20174u) & ~0xeu) | 8u;

		if (isEN7521F || isEN7521S || isEN7512) {
			readl(0xbfa20144u) = 0x12u;
			readl(0xbfa2019cu) = 0x05101b08u;
			readl(0xbfa201acu) = 0x02800000u;
			readl(0xbfa201a8u) = 0x402u;
			readl(0xbfa201a8u) = 0x406u;
			readl(SYS_GLOBAL_PARM) =
				(readl(SYS_GLOBAL_PARM) & 0xff000fffu) | 0x000af000u;
		} else if (isEN7526F || isEN7526D || isEN7513 || isEN7526G || isEN7521G ||
			   isEN7513G || isEN7586) {
			readl(0xbfa20144u) = 0x12u;
			readl(0xbfa2019cu) = 0x05102308u;
			readl(0xbfa201acu) = 0x03800000u;
			readl(0xbfa201a8u) = 0x402u;
			readl(0xbfa201a8u) = 0x406u;
			readl(SYS_GLOBAL_PARM) =
				(readl(SYS_GLOBAL_PARM) & 0xff000fffu) | 0x000e1000u;
		} else {
			while (1)
				;
		}

		if (isEN7521S || isEN7521F)
			readl(0xbfaf200cu) |= BIT(16);
	}

	time_polling_init();
	dramc_main();
	en751221_change_cp0_status(ST0_IM, 0);
	readl(CR_INTC_IMR) = 0;
	readl(CR_TIMER_CTL) = 0;
	jump_addr = (void (*)(void))(unsigned long)readl(SYS_BOOT_JUMP);
	jump_addr();
}
