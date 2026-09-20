OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
	. = CHAINLOADER_BASE;
	__image_start = .;
	.text : ALIGN(4) { KEEP(*(.text.start)) *(.text .text.*) }
	.rodata : ALIGN(4) { *(.rodata .rodata.*) }
	.data : ALIGN(4) { *(.data .data.*) }
	.got : ALIGN(4) { *(.got .got.*) }
	/* Tabela de CRC32 por bloco de 128 B, gravada depois do link pelo
	 * tools/econet-image chainloader. Fica por ultimo: tudo antes dela e o que e conferido.
	 */
	.imgchk : ALIGN(4) {
		__chk_start = .;
		/*
		 * Do not merely advance the location counter here. GNU ld may
		 * turn an all-zero reservation into a section without CONTENTS,
		 * so objcopy -O binary omits the CRC table entirely. Emit one
		 * real word at the end to force a loadable PROGBITS section.
		 */
		FILL(0x00000000)
		. = . + (320 * 4 - 4);
		LONG(0);
		__chk_end = .;
	}
	__image_load_end = .;
	.bss (NOLOAD) : ALIGN(16) { __bss_start = .; *(.bss .bss.* COMMON) . = ALIGN(16); __bss_end = .; }
	__image_end = .;
	/DISCARD/ : { *(.comment) *(.note*) *(.MIPS.abiflags) *(.reginfo) *(.pdr) *(.gnu.attributes) }
}
