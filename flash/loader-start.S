/* SPDX-License-Identifier: GPL-2.0+ */
	.set noreorder
	.set nomips16
	.section .text.entry,"ax",@progbits
	.globl loader_entry
loader_entry:
	/* boot2 has initialized DRAM and parked the secondary contexts. */
	li $sp, 0x8007fff0
	move $fp, $sp
	la $8, __bss_start
	la $9, __bss_end
1:
	beq $8, $9, 2f
	 nop
	sw $0, 0($8)
	b 1b
	 addiu $8, $8, 4
2:
	la $25, loader_main
	jalr $25
	 nop
3:
	b 3b
	 nop
