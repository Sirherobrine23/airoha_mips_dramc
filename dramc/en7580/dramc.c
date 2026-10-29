/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en7580_ddr.h"
int prom_printf_s(char *str, ...) { prom_puts(str); return 0; }
u32 calculate_dram_size(void)
{
#ifdef EN7580_BOOTEXT
	/* tcboot does not provide the vendor's KSEG2 TLB map for this probe. */
	const u32 base_addr = 0xa0080000u;
#else
	const u32 base_addr = 0xc0080000u;
#endif
	volatile u32 *base = (void *)base_addr;
	u32 size = 32u << 20;
	u32 maximum = detected_dram_type == 0 ? 0x40000000u : 0x20000000u;
	prom_puts("Calculate size.\n");
	while (size < maximum) {
		volatile u32 *probe = (void *)(base_addr + size);
		u32 original;
#ifdef EN7580_BOOTEXT
		u32 retries = 1000000;
#endif
		*base = 0x12345678; pause_polling(2);
		*probe = 0x87654321; pause_polling(2);
		for (;;) {
			u32 observed = *probe;
			original = *base;
			if (observed == 0x87654321) break;
#ifdef EN7580_BOOTEXT
			if (!--retries) {
				prom_puts("dram probe timeout\n");
				return 0;
			}
#endif
			*probe = 0x87654321;
		}
		if (original == 0x87654321) break;
		if (original != 0x12345678) {
			prom_puts("dram r/w error!\naddr=0x");
			prom_print_hex(base_addr, 8); prom_puts("\n");
			prom_print_hex(original, 8); prom_puts("\n");
			return 0;
		}
		size <<= 1;
	}
	prom_puts("DRAM size="); prom_print_dec(size >> 20); prom_puts("MB\n");
	return size >> 20;
}
void dramc_reg_dump(void)
{
	for (u32 reg = 0xbfb24000; reg < 0xbfb24400; reg += 4) {
		prom_puts("0x"); prom_print_hex(reg, 3); prom_puts(":");
		prom_print_hex(reg_read(reg), 8); prom_puts("\n");
	}
}
int main(void)
{
	u32 value;
	reg_write(0xbfb00040, 1); reg_write(0xbfb00040, 0);
	detected_dram_type = (reg_read(0xbfa20254) >> 4) & 3;
	value = reg_read(0xbfb00284);
	if (detected_dram_type == 3) {
		reg_write(0xbfb00284, value & ~BIT(4)); DPI_SW_main_PCDDR3();
	} else if (detected_dram_type == 0) {
		reg_write(0xbfb00284, value | BIT(4)); DPI_SW_main_PCDDR4();
	}
	value = reg_read(0xbfb00074) | BIT(2);
	if (!detected_dram_type) value |= BIT(13);
	reg_write(0xbfb00074, value);
	dram_size = calculate_dram_size();
	value = reg_read(0xbfb00284) & 0xf00fffff;
	reg_write(0xbfb00284, value | (((dram_size & 0xfff) >> 4) << 20));
	prom_puts("\r\n7580DRAMC V0.7 ("); prom_print_dec(0); prom_puts(")\r\n");
	return 0;
}
