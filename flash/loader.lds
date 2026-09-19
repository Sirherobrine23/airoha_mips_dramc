/* SPDX-License-Identifier: GPL-2.0+ */
OUTPUT_ARCH(mips)
ENTRY(loader_entry)
SECTIONS {
	. = 0x80000000;
	.text : { KEEP(*(.text.entry)) *(.text*) }
	.rodata : { *(.rodata*) }
	.data : { *(.data*) *(.sdata*) }
	. = ALIGN(4);
	__image_end = .;
	.bss (NOLOAD) : {
		__bss_start = .;
		*(.bss*) *(.sbss*) *(COMMON)
		. = ALIGN(4);
		__bss_end = .;
	}
	ASSERT(__bss_end < 0x80010000, "loader exceeds reserved RAM")
	/DISCARD/ : { *(.reginfo) *(.MIPS.abiflags) *(.pdr) *(.comment) *(.eh_frame) }
}
