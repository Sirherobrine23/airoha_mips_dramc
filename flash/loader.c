// SPDX-License-Identifier: GPL-2.0+
/* Generic post-DRAM flash payload loader. */
#include "loader.h"
#include "decompress.h"

/* Reserved below the payload window, within the minimum 32 MiB DRAM. */
#define DECOMP_WORK_ADDR 0x80100000U
#define DECOMP_INPUT_ADDR 0x80200000U

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
	u8 hdr[ECONET_BOOT_HEADER_V2];
	u8 check[ECONET_BOOT_HEADER_V2];
	u32 payload_off, size, load, entry, dcrc, hcrc;
	u32 i, version, header_size, unpacked, unpacked_crc = 0;
	u32 compression = ECONET_COMP_NONE, input_addr;

	uart_printf("Econet flash payload loader\n");
	if (econet_sfc_init() ||
	    econet_sfc_read(PAYLOAD_OFFSET, hdr, ECONET_BOOT_HEADER_SIZE))
		fail("payload descriptor read failed\r\n");

	if (be32(hdr) != ECONET_BOOT_MAGIC)
		fail("invalid payload descriptor magic\r\n");
	version = be32(hdr + 4);
	header_size = ECONET_BOOT_HEADER_SIZE;
	if (version == ECONET_BOOT_VERSION_V2) {
		header_size = ECONET_BOOT_HEADER_V2;
		if (econet_sfc_read(PAYLOAD_OFFSET + ECONET_BOOT_HEADER_SIZE,
				    hdr + ECONET_BOOT_HEADER_SIZE,
				    header_size - ECONET_BOOT_HEADER_SIZE))
			fail("extended descriptor read failed\r\n");
	} else if (version != ECONET_BOOT_VERSION) {
		fail("unsupported payload descriptor version\r\n");
	}

	for (i = 0; i < header_size; i++)
		check[i] = hdr[i];
	hcrc = be32(check + 28);
	check[28] = check[29] = check[30] = check[31] = 0;
	if (crc32(check, header_size) != hcrc)
		fail("payload descriptor CRC failed\r\n");

	payload_off = be32(hdr + 8);
	size = be32(hdr + 12);
	load = be32(hdr + 16);
	entry = be32(hdr + 20);
	dcrc = be32(hdr + 24);
	unpacked = size;
	if (version == ECONET_BOOT_VERSION_V2) {
		compression = be32(hdr + 32);
		unpacked = be32(hdr + 36);
		unpacked_crc = be32(hdr + 40);
		if (compression > ECONET_COMP_LZMA || be32(hdr + 44) ||
		    (!compression && unpacked != size))
			fail("invalid compression metadata\r\n");
	}

	if (payload_off < header_size ||
	    payload_off > IMAGE_LIMIT - PAYLOAD_OFFSET ||
	    !size || size > IMAGE_LIMIT - PAYLOAD_OFFSET - payload_off ||
	    load < PAYLOAD_RAM_MIN || load >= PAYLOAD_RAM_MAX ||
	    !unpacked || unpacked > PAYLOAD_RAM_MAX - load ||
	    entry < load || entry >= load + unpacked)
		fail("invalid payload descriptor range\r\n");

	uart_printf("Loading payload, load=0x%x, entry=0x%x, size=0x%x\n",
		    load, entry, size);
	input_addr = compression ? DECOMP_INPUT_ADDR : load;
	prepare_destination(input_addr, size);
	if (compression)
		prepare_destination(load, unpacked);
	if (econet_sfc_read(PAYLOAD_OFFSET + payload_off,
			    (void *)(uintptr_t)(input_addr | 0x20000000U), size))
		fail("payload read failed\r\n");

	__asm__ volatile("sync" : : : "memory");
	if (crc32((const u8 *)(uintptr_t)(input_addr | 0x20000000U), size) != dcrc)
		fail("payload data CRC failed\r\n");

	if (compression) {
		prepare_destination(DECOMP_WORK_ADDR, ECONET_DECOMP_WORK_SIZE);
		uart_printf("Decompressing payload, algorithm=%x, size=0x%x\n",
			    compression, unpacked);
		if (econet_decompress(compression,
			(const u8 *)(uintptr_t)(input_addr | 0x20000000U), size,
			(u8 *)(uintptr_t)(load | 0x20000000U), unpacked,
			(void *)(uintptr_t)(DECOMP_WORK_ADDR | 0x20000000U),
			ECONET_DECOMP_WORK_SIZE))
			fail("payload decompression failed\r\n");
	}
	if (version == ECONET_BOOT_VERSION_V2 &&
	    crc32((const u8 *)(uintptr_t)(load | 0x20000000U), unpacked) != unpacked_crc)
		fail("unpacked payload CRC failed\r\n");
	__asm__ volatile("sync" : : : "memory");

	for (u32 p = load; p < load + unpacked; p += 32)
		__asm__ volatile("cache 0x10, 0(%0)" : : "r"(p) : "memory");
	__asm__ volatile("sync; ehb" : : : "memory");

	uart_printf("Starting payload\n");
	((void (*)(u32, u32, u32, u32))(uintptr_t)entry)(0, 0, 0, 0);
	fail("payload returned\r\n");
}
