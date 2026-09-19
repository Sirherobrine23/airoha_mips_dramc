/* SPDX-License-Identifier: GPL-2.0-or-later */
OUTPUT_ARCH(mips)
ENTRY(ddr_entry)
SECTIONS
{
	. = 0x9fa30000;
	.text : {
		KEEP(*(.text.entry))
		*(.text) *(.text.*)
	}
	.rodata : { *(.rodata) *(.rodata.*) *(.rdata) }
	.data : { *(.data) *(.data.*) *(.sdata) *(.sdata.*) }
	.got : { *(.got) *(.got.plt) }
	__image_end = .;
	. = ALIGN(16);
	__bss_start = .;
	.sbss (NOLOAD) : { *(.sbss) *(.sbss.*) *(.scommon) }
	.bss (NOLOAD) : { *(.bss) *(.bss.*) *(COMMON) . = ALIGN(4); }
	_end = .;
	ASSERT(ddr_entry == 0x9fa30000, "DDR entry must be first in the binary")
	ASSERT(_end <= 0x9fa3b000, "DDR payload overlaps the TPL stack")
	/DISCARD/ : {
		*(.comment) *(.pdr) *(.reginfo) *(.MIPS.abiflags)
		*(.gnu.attributes)
	}
}
