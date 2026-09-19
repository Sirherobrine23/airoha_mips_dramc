/* SPDX-License-Identifier: GPL-2.0+ */
/* Enter the reconstructed calibration code with initialized static storage. */
	.set noreorder
	.set nomips16
	.section .text.entry,"ax",@progbits
	.globl ddr_entry
	.type ddr_entry, @function
ddr_entry:
	/* boot2 jumps here without a C stack; use the top of FE SRAM. */
	li $sp, 0xbfa3fff0
	move $fp, $sp
	la $8, __bss_start
	la $9, _end
1:
	beq $8, $9, 2f
	 nop
	sw $0, 0($8)
	b 1b
	 addiu $8, $8, 4
2:
	/* start_spram(NEED_DDR_CALIBRATION), as in the SDK setup.c. */
	li $4, 1
	la $25, start_spram
	jr $25
	 nop
	.size ddr_entry, .-ddr_entry
