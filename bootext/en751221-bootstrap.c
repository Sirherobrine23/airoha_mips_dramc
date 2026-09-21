/* SPDX-License-Identifier: GPL-2.0+ */
#include "chainloader.h"

extern const u32 sram_stage_start[], sram_stage_end[];
extern const u32 ddr_stage_start[], ddr_stage_end[];
extern void chainload_jump(u32 entry) __attribute__((noreturn));

static void kick(void)
{
	mmio_write32(CR_TIMER_CTL,
		    (mmio_read32(CR_TIMER_CTL) & 0xffc0ffffu) | 0x00200000u);
	__asm__ volatile("sync" ::: "memory");
}

static void stop(void) __attribute__((noreturn));
static void stop(void)
{
	for (;;)
		kick();
}

static u32 crc32(const volatile u8 *p, u32 len)
{
	u32 crc = ~0u;
	unsigned int bit;

	while (len--) {
		crc ^= *p++;
		for (bit = 0; bit < 8; bit++)
			crc = (crc >> 1) ^ ((0u - (crc & 1u)) & 0xedb88320u);
	}
	return ~crc;
}

static void check_image(void)
{
	const volatile u8 *image = (const volatile u8 *)&__image_start;
	const volatile u32 *table = (const volatile u32 *)&__chk_start;
	u32 len = (u32)&__chk_start - (u32)&__image_start, off;

	/* EN751221 is big-endian, as is the host tool's CRC table. */
	for (off = 0; off < len; off += CHK_CHUNK) {
		u32 n = len - off;

		if (n > CHK_CHUNK)
			n = CHK_CHUNK;
		if (crc32(image + off, n) != table[off / CHK_CHUNK]) {
			uart_printf("bootext CRC failed at 0x%08x; stopped\n", off);
			stop();
		}
		kick();
	}
}

static void copy_words(u32 dst, const u32 *src, const u32 *end)
{
	const u32 *p;
	u32 addr;

	/* Byte stores through the uncached aperture are unsafe on EN751221. */
	for (p = src, addr = dst; p < end; p++, addr += 4) {
		mmio_write32(addr, *p);
		kick();
	}
	__asm__ volatile("sync" ::: "memory");
	for (p = src, addr = dst; p < end; p++, addr += 4) {
		if (mmio_read32(addr) != *p) {
			uart_printf("FE SRAM readback failed at 0x%08x; stopped\n", addr);
			stop();
		}
		kick();
	}
}

void chainloader_main(void)
{
	mmio_write32(AIROHA_UART_BASE + AIROHA_UART_IER, 0);
	uart_printf("EN751221 bootext: checking DDR and SRAM payloads\n");
	check_image();
	mmio_write32(0xbfb00958u, mmio_read32(0xbfb00958u) | 1u);
	__asm__ volatile("sync" ::: "memory");
	copy_words(0xbfa32800u, ddr_stage_start, ddr_stage_end);
	copy_words(0xbfa38000u, sram_stage_start, sram_stage_end);
	uart_printf("FE SRAM verified; entering DDR stage\n");
	chainload_jump(0x9fa38000u);
}
