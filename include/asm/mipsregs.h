/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __AIROHA_EARLYBOOT_MIPSREGS_H
#define __AIROHA_EARLYBOOT_MIPSREGS_H
#define CP0_STATUS		$12
#define CP0_CAUSE		$13
#define CP0_CONFIG		$16
#define CP0_WATCHLO		$18
#define CP0_WATCHHI		$19

/* MIPS32 CP0 Status/Cause bits used by the EN751221 reset vector. */
#define ST0_IE			0x00000001
#define ST0_KSU			0x00000018
#define CAUSEF_IV		0x00800000
#endif
