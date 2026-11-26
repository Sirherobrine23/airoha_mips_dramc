/* SPDX-License-Identifier: GPL-2.0+ */
.set noreorder
.set noat

.section .text, "ax", @progbits
.globl _start
.ent _start
_start:
	/* The stock tcboot receives U-Boot through the uncached KSEG1 alias. */
	li	$t0, 0x81000000
	li	$t1, 0x81080000
1:
	/* Invalidate stale instructions before entering the KSEG0 image. */
	cache	0x10, 0($t0)
	addiu	$t0, $t0, 32
	bne	$t0, $t1, 1b
	 nop
	sync
	li	$t2, 0x81000000
	jr	$t2
	 nop
.end _start
