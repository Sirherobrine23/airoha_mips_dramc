// SPDX-License-Identifier: GPL-2.0+
/* Generic post-DRAM flash payload loader. */
#include "loader.h"

static void __attribute__((noreturn)) fail(const char *why)
{
	uart_printf(why);
	for (;;);
}

static u32 be32(const u8 *p)
{
	return (u32)p[0] << 24 | (u32)p[1] << 16 | (u32)p[2] << 8 | p[3];
}

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
	u8 hdr[ECONET_BOOT_HEADER_SIZE];
	u8 check[ECONET_BOOT_HEADER_SIZE];
	u32 payload_off, size, load, entry, dcrc, hcrc;
	u32 i;

	uart_printf("Econet flash payload loader\n");
	if (econet_sfc_init() ||
	    econet_sfc_read(PAYLOAD_OFFSET, hdr, sizeof(hdr)))
		fail("payload descriptor read failed\r\n");

	if (be32(hdr) != ECONET_BOOT_MAGIC)
		fail("invalid payload descriptor magic\r\n");
	if (be32(hdr + 4) != ECONET_BOOT_VERSION)
		fail("unsupported payload descriptor version\r\n");

	for (i = 0; i < sizeof(check); i++)
		check[i] = hdr[i];
	hcrc = be32(check + 28);
	check[28] = check[29] = check[30] = check[31] = 0;
	if (crc32(check, sizeof(check)) != hcrc)
		fail("payload descriptor CRC failed\r\n");

	payload_off = be32(hdr + 8);
	size = be32(hdr + 12);
	load = be32(hdr + 16);
	entry = be32(hdr + 20);
	dcrc = be32(hdr + 24);

	if (payload_off < ECONET_BOOT_HEADER_SIZE ||
	    payload_off > IMAGE_LIMIT - PAYLOAD_OFFSET ||
	    !size || size > IMAGE_LIMIT - PAYLOAD_OFFSET - payload_off ||
	    load < PAYLOAD_RAM_MIN || load >= PAYLOAD_RAM_MAX ||
	    size > PAYLOAD_RAM_MAX - load ||
	    entry < load || entry >= load + size)
		fail("invalid payload descriptor range\r\n");

	uart_printf("Loading payload, load=0x%x, entry=0x%x, size=0x%x\n",
		    load, entry, size);
	prepare_destination(load, size);
	if (econet_sfc_read(PAYLOAD_OFFSET + payload_off,
			    (void *)(uintptr_t)(load | 0x20000000U), size))
		fail("payload read failed\r\n");

	__asm__ volatile("sync" : : : "memory");
	if (crc32((const u8 *)(uintptr_t)(load | 0x20000000U), size) != dcrc)
		fail("payload data CRC failed\r\n");

	for (u32 p = load; p < load + size; p += 32)
		__asm__ volatile("cache 0x10, 0(%0)" : : "r"(p) : "memory");
	__asm__ volatile("sync; ehb" : : : "memory");

	uart_printf("Starting payload\n");
	((void (*)(u32, u32, u32, u32))(uintptr_t)entry)(0, 0, 0, 0);
	fail("payload returned\r\n");
}
