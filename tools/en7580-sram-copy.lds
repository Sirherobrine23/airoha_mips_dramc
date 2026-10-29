OUTPUT_ARCH(mips)
ENTRY(_start)

SECTIONS {
	. = 0xa1000000;
	.text : { *(.text*) *(.rodata*) }
	ASSERT(SIZEOF(.text) <= 0x1000, "SRAM copy stage exceeds 4 KiB")
	/DISCARD/ : { *(.comment*) *(.note*) *(.gnu*) *(.reginfo*) *(.MIPS.abiflags*) }
}
