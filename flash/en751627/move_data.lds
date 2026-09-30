/* SPDX-License-Identifier: GPL-2.0-or-later */
OUTPUT_ARCH(mips)
ENTRY(_ftext)

SECTIONS
{
	/* Vendor EN7516/EN751627 move_data executes from GDMP SRAM. */
	. = 0x9fa40300;

	.text : {
		*(.text)
		*(.text.*)
		*(.rodata)
		*(.rodata.*)
	}

	__stage_end = .;
	ASSERT(__stage_end <= 0x9fa41000,
	       "EN751627 move_data overlaps boot2 SRAM window")

	/DISCARD/ : {
		*(.reginfo)
		*(.MIPS.abiflags)
		*(.pdr)
		*(.comment)
		*(.eh_frame)
		*(.note*)
	}
}
