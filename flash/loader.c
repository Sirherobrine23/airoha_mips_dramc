// SPDX-License-Identifier: GPL-2.0+
/* A raw flash reader, not an LZMA decompressor or a second boot monitor. */
#include "loader.h"

static void serial_outc(char c)
{
	unsigned int timeout = 1000000;
	while (!(REG(0xbfbf0014) & 0x20) && --timeout);
	if (timeout)
		REG(0xbfbf0000) = (unsigned char)c;
}

static void puts_uart(const char *s)
{
	while (*s) {
		serial_outc(*s);
		s++;
	}
}

void puts_uart_hex(unsigned int value)
{
	int shift;

	serial_outc('0');
	serial_outc('x');
	for (shift = 28; shift >= 0; shift -= 4) {
		unsigned int digit = (value >> shift) & 0xf;

		serial_outc(digit < 10 ? '0' + digit : 'a' + digit - 10);
	}
}

static void __attribute__((noreturn)) fail(const char *why)
{
	puts_uart(why);
	for (;;);
}

static u32 be32(const u8 *p)
{
	return (u32)p[0] << 24 | (u32)p[1] << 16 | (u32)p[2] << 8 | p[3];
}

/* Legacy uImage uses the standard CRC, including the final complement. */
static u32 crc32(const u8 *p, u32 len)
{
	u32 crc = ~0U;

	while (len--) {
		crc ^= *p++;
		for (unsigned int bit = 0; bit < 8; bit++)
			crc = (crc >> 1) ^ (0xedb88320U & (0U - (crc & 1)));
	}
	return ~crc;
}

static void prepare_destination(u32 start, u32 size)
{
	u32 end = (start + size + 31) & ~31U;

	for (u32 p = start; p < end; p += 32)
		__asm__ volatile("cache 0x15, 0(%0)" : : "r"(p) : "memory");
	__asm__ volatile("sync" : : : "memory");
}

void __attribute__((noreturn)) loader_main(void)
{
	u8 hdr[HEADER_SIZE];
	u32 magic, size, load, entry, hcrc, dcrc, payload_off;
	const char *kind;

	puts_uart("Econet flash loader\r\n");
	if (econet_sfc_init() || econet_sfc_read(UBOOT_OFFSET, hdr, sizeof(hdr)))
		fail("flash header read failed\r\n");

	magic = be32(hdr);
	if (magic == IH_MAGIC) {
		hcrc = be32(hdr + 4);
		hdr[4] = hdr[5] = hdr[6] = hdr[7] = 0;
		if (crc32(hdr, sizeof(hdr)) != hcrc)
			fail("uImage header CRC failed\r\n");

		size = be32(hdr + 12);
		load = be32(hdr + 16);
		entry = be32(hdr + 20);
		dcrc = be32(hdr + 24);
		payload_off = HEADER_SIZE;
		kind = "legacy";

		if (!size || size > IMAGE_LIMIT - UBOOT_OFFSET - payload_off ||
		    load != UBOOT_LOAD_ADDR || entry != load ||
		    load < 0x81000000U || load > 0x82000000U - size ||
		    hdr[29] != 5 || hdr[30] != 5 || hdr[31] != 0)
			fail("unsupported uImage size/address/type\r\n");
	} else if (magic == ECONET_BOOT_MAGIC) {
		u8 check[ECONET_BOOT_HEADER_SIZE];
		u32 i;

		for (i = 0; i < ECONET_BOOT_HEADER_SIZE; i++)
			check[i] = hdr[i];
		hcrc = be32(check + 28);
		check[28] = check[29] = check[30] = check[31] = 0;
		if (crc32(check, sizeof(check)) != hcrc)
			fail("ECNT header CRC failed\r\n");
		if (be32(hdr + 4) != ECONET_BOOT_VERSION)
			fail("unsupported ECNT version\r\n");

		payload_off = be32(hdr + 8);
		size = be32(hdr + 12);
		load = be32(hdr + 16);
		entry = be32(hdr + 20);
		dcrc = be32(hdr + 24);
		kind = "ECNT";

		if (payload_off < ECONET_BOOT_HEADER_SIZE ||
		    payload_off > IMAGE_LIMIT - UBOOT_OFFSET ||
		    !size || size > IMAGE_LIMIT - UBOOT_OFFSET - payload_off ||
		    load < 0x81000000U || load >= 0x82000000U ||
		    size > 0x82000000U - load ||
		    entry < load || entry >= load + size)
			fail("unsupported ECNT size/address\r\n");
	} else {
		fail("unknown U-Boot image format\r\n");
	}

	puts_uart("Loading U-Boot (");
	puts_uart(kind);
	puts_uart("), load=");
	puts_uart_hex(load);
	puts_uart(", entry=");
	puts_uart_hex(entry);
	puts_uart("\r\n");

	prepare_destination(load, size);
	if (econet_sfc_read(UBOOT_OFFSET + payload_off,
			    (void *)(uintptr_t)(load | 0x20000000U), size))
		fail("U-Boot read failed\r\n");

	__asm__ volatile("sync" : : : "memory");
	if (crc32((const u8 *)(uintptr_t)(load | 0x20000000U), size) != dcrc)
		fail("U-Boot data CRC failed\r\n");

	for (u32 p = load; p < load + size; p += 32)
		__asm__ volatile("cache 0x10, 0(%0)" : : "r"(p) : "memory");
	__asm__ volatile("sync; ehb" : : : "memory");

	puts_uart("Starting U-Boot\r\n");
	((void (*)(u32, u32, u32, u32))(uintptr_t)entry)(0, 0, 0, 0);
	fail("U-Boot returned\r\n");
}
