/* SPDX-License-Identifier: GPL-2.0-or-later */
#if defined(__ASSEMBLER__) && defined(CHAINLOADER_EN751627)
/* Config2 describes the 1004K secondary cache. Keep Config1 in t0 for
 * the following I-cache loop, and s0 for the payload handoff address.
 */
.macro en751627_flush_l2
	mfc0 $t1, $16, 2
	ext $t2, $t1, 4, 4
	beqz $t2, .Ll2_done\@
	 nop
	li $t5, 2
	sllv $t2, $t5, $t2
	ext $t3, $t1, 8, 4
	li $t5, 64
	sllv $t3, $t5, $t3
	ext $t4, $t1, 0, 4
	addiu $t4, $t4, 1
	mul $t6, $t2, $t3
	mul $t6, $t6, $t4
	lui $t7, 0x8000
	addu $t8, $t7, $t6
.Ll2_loop\@:
	cache 0x03, 0($t7) /* Index_Writeback_Inv_SD */
	addu $t7, $t7, $t2
	bne $t7, $t8, .Ll2_loop\@
	 nop
	sync
.Ll2_done\@:
.endm
#endif
