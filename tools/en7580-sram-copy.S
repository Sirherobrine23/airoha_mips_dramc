/* SPDX-License-Identifier: GPL-2.0+ */
.set noreorder
.set noat

#ifndef SRAM_COPY_LENGTH
#error "SRAM_COPY_LENGTH must match the DRAMC image size"
#endif

.section .text, "ax", @progbits
.globl _start
.ent _start
_start:
	/* Normal tcboot disables the FE SRAM window after its DDR stage. */
	li	$t4, 0xbfb00958
	lw	$t5, 0($t4)
	ori	$t5, $t5, 1
	sw	$t5, 0($t4)
	sync

	/* Copy and verify full words; tcboot XMODEM cannot receive into FE SRAM. */
	li	$t0, 0xa1100000
	li	$t1, 0xbfa30000
	li	$t2, SRAM_COPY_LENGTH
1:
	lw	$t3, 0($t0)
	sw	$t3, 0($t1)
	lw	$t6, 0($t1)
	bne	$t3, $t6, 3f
	 nop
	addiu	$t0, $t0, 4
	addiu	$t1, $t1, 4
	addiu	$t2, $t2, -4
	bnez	$t2, 1b
	 nop
	sync

	/* Calls in the DRAMC image use the cached SRAM alias. */
	li	$t0, 0x9fa30000
	li	$t1, 0x9fa3b000
2:
	cache	0x10, 0($t0)
	addiu	$t0, $t0, 32
	bne	$t0, $t1, 2b
	 nop
	sync
	li	$t0, 0xa1001000
	li	$t1, 0x600d600d
	sw	$t1, 0($t0)
	jr	$ra
	 nop
3:
	li	$t0, 0xa1001000
	li	$t1, 0xbadbad00
	sw	$t1, 0($t0)
	jr	$ra
	 nop
.end _start
