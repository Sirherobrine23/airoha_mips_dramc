OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
	. = 0x80009000;
	__image_start = .;
	.text : ALIGN(4) { KEEP(*(.text.start)) *(.text .text.*) }
	.rodata : ALIGN(4) { *(.rodata .rodata.*) }
	.data : ALIGN(4) { *(.data .data.*) }
	.got : ALIGN(4) { *(.got .got.*) }
	/* Tabela de CRC32 por bloco de 128 B, gravada depois do link pelo
	 * patch_crc.py. Fica por ultimo: tudo antes dela e o que e conferido.
	 */
	.imgchk : ALIGN(4) { __chk_start = .; FILL(0x00000000) . = . + 96 * 4; }
	__image_load_end = .;
	.bss (NOLOAD) : ALIGN(16) { __bss_start = .; *(.bss .bss.* COMMON) . = ALIGN(16); __bss_end = .; }
	__image_end = .;
	/DISCARD/ : { *(.comment) *(.note*) *(.MIPS.abiflags) *(.reginfo) *(.pdr) *(.gnu.attributes) }
}
