/* SPDX-License-Identifier: GPL-2.0+ */
OUTPUT_ARCH(mips)
ENTRY(ddr_entry)
SECTIONS
{
	. = BOOTEXT_BASE;
	__image_start = .;
	.text : ALIGN(4) {
		KEEP(*(.text.entry))
		KEEP(*(.text.start))
		*(.text .text.*)
	}
	.rodata : ALIGN(4) { *(.rodata .rodata.* .rdata) }
	.data : ALIGN(4) { *(.data .data.* .sdata .sdata.*) }
	.got : ALIGN(4) { *(.got .got.plt) }
	/* CRC32 table for 128-byte blocks covering the entire unified bootext
	 * (DRAMC + chainloader), populated by tools/econet-image chainloader.
	 */
	.imgchk : ALIGN(4) {
		__chk_start = .;
		FILL(0x00000000)
		. = . + (320 * 4 - 4);
		LONG(0);
		__chk_end = .;
	}
	__image_load_end = .;
	. = ALIGN(16);
	__bss_start = .;
	.sbss (NOLOAD) : { *(.sbss .sbss.*) *(.scommon) }
	.bss (NOLOAD) : { *(.bss .bss.*) *(COMMON) . = ALIGN(16); }
	__bss_end = .;
	_end = .;
	__image_end = .;
	ASSERT(ddr_entry == BOOTEXT_BASE, "Unified bootext must enter at BOOTEXT_BASE")
	ASSERT(__chk_start - __image_start <= 320 * 128, "Unified bootext exceeds CRC self-check table")
	ASSERT(_end <= BOOTEXT_BASE + BOOTEXT_SRAM_SIZE, "Unified bootext overlaps reserved SRAM stack")
	/DISCARD/ : {
		*(.comment) *(.pdr) *(.reginfo) *(.MIPS.abiflags)
		*(.gnu.attributes) *(.note*) *(.eh_frame)
	}
}
