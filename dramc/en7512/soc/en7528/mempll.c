/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Readable EN7528 DRAMC V1.8 memory-PLL setup and jitter-meter calibration. */
#include "dramc_common.h"

#define R(o)			DRAMC_READ_REG(o)
#define W(o, v)			DRAMC_WRITE_REG((v), (o))
#define SET(o, v)		DRAMC_WRITE_SET((v), (o))
#define UPDATE(o, mask, value)	W((o), (R(o) & (u32)(mask)) | (u32)(value))

void mempll_init_int(u32 xtal, u32 speed)
{
	u32 fb = (speed / 100u) << 4;

	dram_speed = (int)speed;
	UPDATE(DRAMC_MEMPLL_DIVIDER, 0xfffffdc8u, 0x222u);
	UPDATE(DRAMC_MEMPLL1, 0xffffffe1u, 0x12u);
	UPDATE(DRAMC_MEMPLL0, 0xfffc0808u, xtal == 1 ? 0x131u : 0x181u);
	pause_polling(1);
	SET(DRAMC_MEMPLL0, 0x02000000u);
	pause_polling(20);
	dram_mempll_finish(0x080au | fb, 0x02800000u);
}

void mempll_init_ddr1066(u32 xtal)
{
	(void)xtal;
	dram_speed = 1066;
	UPDATE(DRAMC_MEMPLL_DIVIDER, 0xfffffdc8u, 0x222u);
	UPDATE(DRAMC_MEMPLL1, 0xffffffe1u, 0x12u);
	UPDATE(DRAMC_MEMPLL0, 0xfbfc0808u, 0x04000135u);
	UPDATE(DRAMC_MEMPLL2, 0x0001ffffu, 0x2b020000u);
	UPDATE(DRAMC_MEMPLL3, 0xffff0000u, 0x0687u);
	pause_polling(1);
	SET(DRAMC_MEMPLL0, 0x02000000u);
	pause_polling(20);
	SET(DRAMC_MEMPLL3, 0x00080000u);
	pause_polling(1);
	SET(DRAMC_MEMPLL3, 0x00010000u);
	pause_polling(20);
	dram_mempll_finish(0x08aau, 0x04000000u);
}

void mempll_init_ddr1333(u32 xtal)
{
	dram_speed = 1333;
	UPDATE(DRAMC_MEMPLL_DIVIDER, 0xfffffdc8u, 0x222u);
	UPDATE(DRAMC_MEMPLL1, 0xffffffe1u, 0x12u);
	UPDATE(DRAMC_MEMPLL0, 0xfbfc0808u, 0x04000135u);

	if (xtal == 1) {
		UPDATE(DRAMC_MEMPLL2, 0x0001ffffu, 0x7df20000u);
		UPDATE(DRAMC_MEMPLL3, 0xffff0000u, 0x08a9u);
	} else {
		UPDATE(DRAMC_MEMPLL2, 0x0001ffffu, 0x3d700000u);
		UPDATE(DRAMC_MEMPLL3, 0xffff0000u, 0x0b54u);
	}

	pause_polling(1);
	SET(DRAMC_MEMPLL0, 0x02000000u);
	pause_polling(20);
	SET(DRAMC_MEMPLL3, 0x00080000u);
	pause_polling(1);
	SET(DRAMC_MEMPLL3, 0x00010000u);
	pause_polling(20);
	dram_mempll_finish(0x08aau, 0x04000000u);
}

int mt_mempll_cali(void)
{
	u32 pll2_done = 0, pll3_done = 0;
	u32 pll2_tap = 0, pll3_tap = 0;

	/* Enable MEMPLL jitter-meter calibration path. */
	W(DRAMC_DM_MONITOR, (R(DRAMC_DM_MONITOR) & 0x0000ffffu) | 0x04000000u);

	for (;;) {
		if (!pll2_done) {
			UPDATE(DRAMC_MEMPLL7, 0xfe0fffffu, 0x01000000u);
			W(DRAMC_MEMPLL7, (R(DRAMC_MEMPLL7) & 0x83ffffffu) | (pll2_tap << 26));
		}
		if (!pll3_done) {
			UPDATE(0x634, 0xfe0fffffu, 0x01000000u);
			W(0x634, (R(0x634) & 0x83ffffffu) | (pll3_tap << 26));
		}

		if (!pll2_done)
			SET(DRAMC_DM_MONITOR, 0x00000001u);
		if (!pll3_done)
			SET(DRAMC_DM_MONITOR, 0x00004000u);

		pause_polling(40);

		if (!pll2_done) {
			u32 jm = R(0x320);
			(void)R(0x324); /* vendor performs both status reads */
			pll2_done = ((jm >> 16) > 512u);
		}
		if (!pll3_done) {
			u32 jm = R(0x328);
			(void)R(0x32c);
			pll3_done = ((jm >> 16) > 512u);
		}

		if (!pll2_done) {
			++pll2_tap;
			DRAMC_WRITE_CLEAR(0x00000001u, DRAMC_DM_MONITOR);
		}
		if (!pll3_done) {
			++pll3_tap;
			DRAMC_WRITE_CLEAR(0x00004000u, DRAMC_DM_MONITOR);
		}

		if (pll2_done && pll3_done) {
			SET(DRAMC_MEMPLL6, 0x8u);
			SET(DRAMC_MEMPLL12, 0x8u);
			return 0;
		}
		if (pll2_tap >= 32 || pll3_tap >= 32) {
			SET(DRAMC_MEMPLL6, 0x8u);
			SET(DRAMC_MEMPLL12, 0x8u);
			return -1;
		}
	}
}
