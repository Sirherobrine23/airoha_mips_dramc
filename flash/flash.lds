/* SPDX-License-Identifier: GPL-2.0+ */
OUTPUT_ARCH(mips)
ENTRY(__start)
SECTIONS {
	. = 0xbfc00000;
	.text : { *(.text) }
	.move_data : {
		__move_data_start = .;
		*(.move_data)
		__move_data_end = .;
	}
	ASSERT(__move_data_end < 0xbfc00800, "move_data exceeds first NAND page")
	.boot2 : { __boot2_start = .; *(.boot2) __boot2_end = .; }
	/* Put the small raw loader before calibration, as in flash boot 128K. */
	.lzma : { __lzma_start = .; *(.loader) __lzma_end = .; }
	. = ALIGN(4);
	.spram : { __spram_start = .; *(.spram) __spram_end = .; }
	ASSERT(__spram_end < 0xbfc0ff00, "stages overlap manufacturing data")
	/* No legacy compressed bootram: the loader reads U-Boot at 128 KiB. */
	__boot_start = 0xbfc20000;
	__boot_end = 0xbfc20000;
	/DISCARD/ : { *(.reginfo) *(.MIPS.abiflags) *(.pdr) *(.comment) *(.eh_frame) }
}
