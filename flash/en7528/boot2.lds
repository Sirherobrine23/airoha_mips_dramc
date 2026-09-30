/* SPDX-License-Identifier: GPL-2.0-or-later */
OUTPUT_ARCH(mips)
ENTRY(__reset_vector)

SECTIONS
{
	/* Vendor EN7528 LE boot2 executes from GDMP SRAM. */
	. = 0x9fa41000;

	.text : {
		*(.text)
		*(.text.*)
		*(.rodata)
		*(.rodata.*)
	}

	__stage_end = .;
	ASSERT(__stage_end <= 0x9fa42000,
	       "EN7528 boot2 exceeds vendor GDMP SRAM window")

	/DISCARD/ : {
		*(.reginfo)
		*(.MIPS.abiflags)
		*(.pdr)
		*(.comment)
		*(.eh_frame)
		*(.note*)
	}
}
