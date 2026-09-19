/* SPDX-License-Identifier: GPL-2.0+ */
.set noreorder
.set noat

.section .text.start, "ax", @progbits
.globl _start
.ent _start
_start:
	/* Mask every interrupt inherited from the BootROM before polling the UART. */
	mfc0	$t0, $12
	li	$t1, 0xffff00fe
	and	$t0, $t0, $t1
	mtc0	$t0, $12
	ehb
	mtc0	$zero, $13		/* Cause: DC=0, Count runs */
	ehb

	/*
	 * The BootROM writes the image to DRAM through the cache. Before
	 * switching to running uncached, write back the dirty lines and
	 * invalidate the I-cache, otherwise we read stale DRAM.
	 */
	mfc0	$t0, $16, 1		/* Config1 */

	/* D-cache: DL=[12:10] line, DS=[15:13] sets, DA=[9:7] ways */
	srl	$t1, $t0, 10
	andi	$t1, $t1, 7
	beqz	$t1, 2f
	 nop
	li	$t2, 2
	sllv	$t2, $t2, $t1		/* line size */
	srl	$t3, $t0, 13
	andi	$t3, $t3, 7
	li	$t4, 64
	sllv	$t4, $t4, $t3		/* sets */
	srl	$t5, $t0, 7
	andi	$t5, $t5, 7
	addiu	$t5, $t5, 1		/* ways */
	mul	$t6, $t2, $t4
	mul	$t6, $t6, $t5		/* total size */
	lui	$t7, 0x8000
	addu	$t8, $t7, $t6
1:
	cache	0x01, 0($t7)		/* Index_Writeback_Inv_D */
	addu	$t7, $t7, $t2
	bne	$t7, $t8, 1b
	 nop
	sync
2:
	/* I-cache: IL=[21:19] line, IS=[24:22] sets, IA=[18:16] ways */
	srl	$t1, $t0, 19
	andi	$t1, $t1, 7
	beqz	$t1, 4f
	 nop
	li	$t2, 2
	sllv	$t2, $t2, $t1
	srl	$t3, $t0, 22
	andi	$t3, $t3, 7
	li	$t4, 64
	sllv	$t4, $t4, $t3
	srl	$t5, $t0, 16
	andi	$t5, $t5, 7
	addiu	$t5, $t5, 1
	mul	$t6, $t2, $t4
	mul	$t6, $t6, $t5
	lui	$t7, 0x8000
	addu	$t8, $t7, $t6
3:
	cache	0x00, 0($t7)		/* Index_Invalidate_I */
	addu	$t7, $t7, $t2
	bne	$t7, $t8, 3b
	 nop
	sync
4:
	/*
	 * CACHED KSEG0. Uncached 8-bit writes on this chip do a
	 * read-modify-write that brings bus garbage into the other 3
	 * bytes of the word; with cache, every store becomes a full
	 * line writeback and the problem goes away. The BootROM runs
	 * this way and it works.
	 */
	mfc0	$t0, $16
	li	$t1, -8
	and	$t0, $t0, $t1
	ori	$t0, $t0, 3
	mtc0	$t0, $16
	ehb
	sync

	/* Stack in cached KSEG0, below the download address. */
	lui	$sp, 0x8001
	addiu	$sp, $sp, -0x8000
	addiu	$sp, $sp, -32

	/* Zero .bss: the BootROM's XMODEM padding lands exactly there. */
	la	$t0, __bss_start
	la	$t1, __bss_end
5:
	sltu	$t2, $t0, $t1
	beqz	$t2, 6f
	 nop
	sw	$zero, 0($t0)
	b	5b
	 addiu	$t0, $t0, 4
6:
	sync

	jal	chainloader_main
	 nop

7:	b	7b
	 nop
.end _start

.globl chainload_jump
.ent chainload_jump
chainload_jump:
	/* Write the received image back to DRAM before leaving. */
	move	$s0, $a0
	mfc0	$t0, $16, 1
	srl	$t1, $t0, 10
	andi	$t1, $t1, 7
	beqz	$t1, 2f
	 nop
	li	$t2, 2
	sllv	$t2, $t2, $t1
	srl	$t3, $t0, 13
	andi	$t3, $t3, 7
	li	$t4, 64
	sllv	$t4, $t4, $t3
	srl	$t5, $t0, 7
	andi	$t5, $t5, 7
	addiu	$t5, $t5, 1
	mul	$t6, $t2, $t4
	mul	$t6, $t6, $t5
	lui	$t7, 0x8000
	addu	$t8, $t7, $t6
1:
	cache	0x01, 0($t7)		/* Index_Writeback_Inv_D */
	addu	$t7, $t7, $t2
	bne	$t7, $t8, 1b
	 nop
	sync
2:
	srl	$t1, $t0, 19
	andi	$t1, $t1, 7
	beqz	$t1, 4f
	 nop
	li	$t2, 2
	sllv	$t2, $t2, $t1
	srl	$t3, $t0, 22
	andi	$t3, $t3, 7
	li	$t4, 64
	sllv	$t4, $t4, $t3
	srl	$t5, $t0, 16
	andi	$t5, $t5, 7
	addiu	$t5, $t5, 1
	mul	$t6, $t2, $t4
	mul	$t6, $t6, $t5
	lui	$t7, 0x8000
	addu	$t8, $t7, $t6
3:
	cache	0x00, 0($t7)		/* Index_Invalidate_I */
	addu	$t7, $t7, $t2
	bne	$t7, $t8, 3b
	 nop
	sync
4:
	/* Hand off control with uncached KSEG0; U-Boot reconfigures it. */
	mfc0	$t0, $16
	li	$t1, -8
	and	$t0, $t0, $t1
	ori	$t0, $t0, 2
	mtc0	$t0, $16
	ehb
	sync
	jr	$s0
	 nop
.end chainload_jump
