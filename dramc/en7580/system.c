/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en7580_ddr.h"
#define GLOBAL_PARM 0xbfb00284u
static void set_hclk(u32 mhz)
{
	reg_write(GLOBAL_PARM, (reg_read(GLOBAL_PARM) & 0xfff003ff) | ((mhz & 0x3ff) << 10));
}
void en7580_setup_clk(u32 cpu_mhz)
{
	if (cpu_mhz > 1300) {
		prom_puts("\nWrong CPU Clock("); prom_print_dec(cpu_mhz); prom_puts("MHz)\n");
		for (;;) ;
	}
	if (cpu_mhz == 1300) { set_hclk(260); return; }
	reg_write(0xbfa20264, 0x80);
	for (u32 step = 0; step < (1300 - cpu_mhz) / 25; step++) {
		reg_write(0xbfa202ac, ((102 - step * 2) & 0x7f) << 24);
		reg_write(0xbfa202b0, reg_read(0xbfa202b0) ^ BIT(3));
		set_hclk((1275 - step * 25) / 5); pause_polling(1);
	}
	reg_write(0xbfa20264, 0);
}
void set_spi_ctrler_ecc(void)
{
	u32 chip = reg_read(0xbfb00064) & 0xffff0000;
	u32 strap;
	if (chip == 0x70000) reg_write(GLOBAL_PARM, reg_read(GLOBAL_PARM) & ~BIT(8));
	strap = reg_read((reg_read(GLOBAL_PARM) & BIT(9)) ? 0xbfa2ff30 : 0xbfa20254);
	reg_write(GLOBAL_PARM, (reg_read(GLOBAL_PARM) & ~BIT(8)) | ((strap & 1) ? 0 : BIT(8)));
}
void init_system(int bootrom_ext)
{
	u8 package_id = 0;
	u32 fpga = (reg_read(0xbfb0009c) & 1) ^ 1;
	reg_write(GLOBAL_PARM, (reg_read(GLOBAL_PARM) & ~BIT(9)) | (fpga << 9));
	set_spi_ctrler_ecc();
	if (reg_read(GLOBAL_PARM) & BIT(9)) { set_hclk(40); return; }
	if (bootrom_ext) { set_hclk(260); return; }
	reg_write(0xbfaf692c, 0); reg_write(0xbfaf6000, 0xa0800000); reg_write(0xbfaf692c, 1);
	pause_polling(1000);
	en7580_setup_clk((reg_read(0xbfb00240) & 3) ? 325 : 1300);
	time_polling_init();
	if (efuse_read_data(0, 0x500, (volatile void *)0xbfa40b00)) {
		prom_puts("package init fail\n"); for (;;) ;
	}
	if (efuse_read_data((*(volatile u8 *)0xbfa40b00 & BIT(3)) ? 8 : 4, 4, &package_id)) {
		prom_puts("package ID init fail\n"); for (;;) ;
	}
	reg_write(GLOBAL_PARM, (reg_read(GLOBAL_PARM) & ~0xf) | (package_id & 0xf));
}
