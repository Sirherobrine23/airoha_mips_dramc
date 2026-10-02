/* SPDX-License-Identifier: GPL-2.0+ */
/* EN751627 keeps its large vendor DRAMC below mi.conf. The post-DRAM
 * loader lives in the second 64 KiB, addressed by the existing header.
 */
OUTPUT_ARCH(mips)
ENTRY(__start)
SECTIONS {
	. = 0xbfc00000;
	.text : { *(.text) }
	.move_data : {
		__move_data_start = .; *(.move_data) __move_data_end = .;
	}
	ASSERT(__move_data_end < 0xbfc00800, "move_data exceeds first NAND page")
	.boot2 : { __boot2_start = .; *(.boot2) __boot2_end = .; }
	. = ALIGN(4);
	.spram : { __spram_start = .; *(.spram) __spram_end = .; }
	ASSERT(__spram_end < 0xbfc0ff00, "calibration overlaps manufacturing data")
	. = 0xbfc10000;
	.lzma : { __lzma_start = .; *(.loader) __lzma_end = .; }
	ASSERT(__lzma_end < 0xbfc1fffc, "loader overlaps boot CRC")
	__boot_start = 0xbfc20000;
	__boot_end = 0xbfc20000;
	/DISCARD/ : { *(.reginfo) *(.MIPS.abiflags) *(.pdr) *(.comment) *(.eh_frame) }
}
