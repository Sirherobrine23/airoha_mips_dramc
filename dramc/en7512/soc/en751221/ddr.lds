/* SPDX-License-Identifier: GPL-2.0-or-later */
ENTRY(start)
SECTIONS
{
	. = 0x9fa32800;

	.init : { *(.init) } = 0

	.text : {
		KEEP(*(.text.head))
		*(.text .text.*)
		*(.gnu.warning)
	} = 0
	_etext = .;
	.data : {
		_fdata = .;
		*(.rodata .rodata.*)
		*(.rodata1)
		*(.srodata .srodata.*)
		*(.data .data.*)
		*(.sdata .sdata.*)
		_edata = .;
	}
	__bss_start = .;
	_fbss = .;
	.sbss (NOLOAD) : { *(.sbss .sbss.*) *(.scommon) }
	.bss (NOLOAD) : { *(.dynbss) *(.bss .bss.*) *(COMMON) . = ALIGN(4); _end = .; }
	ASSERT(start == 0x9fa32a80, "EN751221 DRAMC entry must be at 0x9fa32a80")
	ASSERT(_end <= 0x9fa3b000, "DDR payload overlaps the TPL stack")
	/DISCARD/ : { *(.comment) *(.pdr) *(.reginfo) *(.MIPS.abiflags) *(.gnu.attributes) *(.note*) *(.eh_frame) }
}
