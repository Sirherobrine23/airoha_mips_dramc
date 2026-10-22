/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en7580_ddr.h"
void start_spram(int calibrate)
{
	spram_preprocess();
	if (calibrate == 1) main();
	spram_postprocess();
	((void (*)(void))reg_read(0xbfb00280))();
}
