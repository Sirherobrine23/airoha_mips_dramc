/* SPDX-License-Identifier: GPL-2.0+ */
/* Establish an SRAM stack and clear C static storage before DDR setup. */
	.set noreorder
	.set nomips16
	.section .text.entry,"ax",@progbits
	.globl ddr_entry
	.type ddr_entry, @function
ddr_entry:
#ifdef BOOTEXT_UNIFIED
	/* Unified SRAM bootext: return from DRAMC directly into the linked chainloader. */
	la $9, _start
	li $8, 0xbfb00280
	sw $9, 0($8)
	sync
#elif defined(BOOTEXT_CHAINLOAD_ENTRY)
	/* Standalone composite bootext: hand DRAMC return to the fixed chainloader address. */
	li $8, 0xbfb00280
	li $9, BOOTEXT_CHAINLOAD_ENTRY
	sw $9, 0($8)
	sync
#endif
	/* boot2/BootROM jumps here without a C stack; use the top of FE SRAM. */
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
