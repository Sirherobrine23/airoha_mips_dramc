/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en7580_ddr.h"
void start_spram(int calibrate)
{
	spram_preprocess();
#ifdef EN7580_BOOTEXT
	prom_puts("C-DRAMC start\n");
#endif
	if (calibrate == 1) main();
	spram_postprocess();
#ifdef EN7580_BOOTEXT
	/* The recovery probe leaves the flash untouched after DDR training. */
	if (dram_size)
		prom_puts("C-DRAMC done\n");
	else
		prom_puts("C-DRAMC size fail\n");
	for (;;) ;
#else
	((void (*)(void))reg_read(0xbfb00280))();
#endif
}
