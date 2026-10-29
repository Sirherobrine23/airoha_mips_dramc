OUTPUT_ARCH(mips)
ENTRY(_start)

SECTIONS {
	. = 0xa0f00000;
	.text : { *(.text*) *(.rodata*) }
	ASSERT(SIZEOF(.text) <= 0x1000, "RAM jump stage exceeds 4 KiB")
	/DISCARD/ : { *(.comment*) *(.note*) *(.gnu*) *(.reginfo*) *(.MIPS.abiflags*) }
}
