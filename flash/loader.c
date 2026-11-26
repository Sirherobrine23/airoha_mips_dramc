// SPDX-License-Identifier: GPL-2.0+
/* Generic post-DRAM flash payload loader with UBI and ECNT support. */
#include "loader.h"
#include "decompress.h"
#include "ubi.h"

/* Reserved below the payload window, within the minimum 32 MiB DRAM. */
#define DECOMP_WORK_ADDR	0x80100000U
#define UBI_WORK_ADDR		0x80180000U
#define DECOMP_INPUT_ADDR	0x80200000U
#define UBI_STAGING_SIZE	(PAYLOAD_RAM_MIN - DECOMP_INPUT_ADDR)

/* U-Boot legacy image (uImage) header constants. */
#define IH_MAGIC		0x27051956U
#define IH_HEADER_SIZE		64U
#define IH_ARCH_MIPS		5U
#define IH_COMP_NONE		0U
#define IH_COMP_GZIP		1U
#define IH_COMP_LZMA		3U

struct ecnt_desc {
	u32 version;
	u32 header_size;
	u32 payload_off;
	u32 size;
	u32 load;
	u32 entry;
	u32 dcrc;
	u32 compression;
	u32 unpacked;
	u32 unpacked_crc;
};

static const char *const ubi_bootloader_names[] = {
	"u-boot",
	"uboot",
	"bootloader",
};

static void __attribute__((noreturn)) fail(const char *why)
{
	uart_printf(why);
	for (;;)
		;
}

static u32 be32(const u8 *p)
{
	return (u32)p[0] << 24 | (u32)p[1] << 16 | (u32)p[2] << 8 | p[3];
}

static u32 crc32(const u8 *p, u32 len)
{
	return ~econet_crc32_raw(p, len);
}

static void copy_bytes(u8 *dst, const u8 *src, u32 len)
{
	for (u32 i = 0; i < len; i++)
		dst[i] = src[i];
}

static void prepare_destination(u32 start, u32 size)
{
	u32 end = (start + size + 31) & ~31U;

	for (u32 p = start; p < end; p += 32)
		__asm__ volatile("cache 0x15, 0(%0)" : : "r"(p) : "memory");
	__asm__ volatile("sync" : : : "memory");
}

static void __attribute__((noreturn)) boot_payload(u32 load, u32 entry, u32 unpacked)
{
	__asm__ volatile("sync" : : : "memory");

	for (u32 p = load; p < load + unpacked; p += 32)
		__asm__ volatile("cache 0x10, 0(%0)" : : "r"(p) : "memory");
	__asm__ volatile("sync; ehb" : : : "memory");

	uart_printf("Starting payload\n");
	((void (*)(u32, u32, u32, u32))(uintptr_t)entry)(0, 0, 0, 0);
	fail("payload returned\r\n");
}

static void parse_ecnt_header(const u8 *hdr, u32 avail_len, struct ecnt_desc *d)
{
	u8 check[ECONET_BOOT_HEADER_V2];
	u32 hcrc;

	if (avail_len < ECONET_BOOT_HEADER_SIZE || be32(hdr) != ECONET_BOOT_MAGIC)
		fail("invalid payload descriptor magic\r\n");

	d->version = be32(hdr + 4);
	d->header_size = ECONET_BOOT_HEADER_SIZE;
	if (d->version == ECONET_BOOT_VERSION_V2) {
		d->header_size = ECONET_BOOT_HEADER_V2;
		if (avail_len < d->header_size)
			fail("truncated extended descriptor\r\n");
	} else if (d->version != ECONET_BOOT_VERSION) {
		fail("unsupported payload descriptor version\r\n");
	}

	copy_bytes(check, hdr, d->header_size);
	hcrc = be32(check + 28);
	check[28] = check[29] = check[30] = check[31] = 0;
	if (crc32(check, d->header_size) != hcrc)
		fail("payload descriptor CRC failed\r\n");

	d->payload_off = be32(hdr + 8);
	d->size = be32(hdr + 12);
	d->load = be32(hdr + 16);
	d->entry = be32(hdr + 20);
	d->dcrc = be32(hdr + 24);
	d->compression = ECONET_COMP_NONE;
	d->unpacked = d->size;
	d->unpacked_crc = 0;

	if (d->version == ECONET_BOOT_VERSION_V2) {
		d->compression = be32(hdr + 32);
		d->unpacked = be32(hdr + 36);
		d->unpacked_crc = be32(hdr + 40);
		if (d->compression > ECONET_COMP_LZMA || be32(hdr + 44) ||
		    (!d->compression && d->unpacked != d->size))
			fail("invalid compression metadata\r\n");
	}

	if (d->payload_off < d->header_size || d->payload_off > avail_len || !d->size ||
	    d->size > avail_len - d->payload_off || d->load < PAYLOAD_RAM_MIN ||
	    d->load >= PAYLOAD_RAM_MAX || !d->unpacked || d->unpacked > PAYLOAD_RAM_MAX - d->load ||
	    d->entry < d->load || d->entry >= d->load + d->unpacked)
		fail("invalid payload descriptor range\r\n");
}

static void __attribute__((noreturn)) boot_ecnt_buffer(const u8 *buf, u32 len)
{
	struct ecnt_desc d;
	const u8 *payload;
	u8 *dst;

	parse_ecnt_header(buf, len, &d);
	payload = buf + d.payload_off;

	uart_printf("Loading ECNT payload, load=0x%x, entry=0x%x, size=0x%x\n", d.load, d.entry,
		    d.size);
	if (crc32(payload, d.size) != d.dcrc)
		fail("payload data CRC failed\r\n");

	prepare_destination(d.load, d.unpacked);
	dst = (u8 *)(uintptr_t)(d.load | 0x20000000U);

	if (d.compression) {
		prepare_destination(DECOMP_WORK_ADDR, ECONET_DECOMP_WORK_SIZE);
		uart_printf("Decompressing payload, algorithm=%x, size=0x%x\n", d.compression,
			    d.unpacked);
		if (econet_decompress(d.compression, payload, d.size, dst, d.unpacked,
				      (void *)(uintptr_t)(DECOMP_WORK_ADDR | 0x20000000U),
				      ECONET_DECOMP_WORK_SIZE))
			fail("payload decompression failed\r\n");
	} else {
		copy_bytes(dst, payload, d.size);
	}

	if (d.version == ECONET_BOOT_VERSION_V2 && crc32(dst, d.unpacked) != d.unpacked_crc)
		fail("unpacked payload CRC failed\r\n");

	boot_payload(d.load, d.entry, d.unpacked);
}

static void __attribute__((noreturn)) boot_uimage_buffer(const u8 *buf, u32 len)
{
	u8 hdr[IH_HEADER_SIZE];
	u32 hcrc, size, load, entry, dcrc;
	u8 arch, comp;
	u8 *dst;
	size_t unpacked;

	if (len < IH_HEADER_SIZE)
		fail("truncated uImage header\r\n");

	copy_bytes(hdr, buf, IH_HEADER_SIZE);
	hcrc = be32(hdr + 4);
	hdr[4] = hdr[5] = hdr[6] = hdr[7] = 0;
	if (crc32(hdr, IH_HEADER_SIZE) != hcrc)
		fail("uImage header CRC failed\r\n");

	size = be32(buf + 12);
	load = be32(buf + 16);
	entry = be32(buf + 20);
	dcrc = be32(buf + 24);
	arch = buf[29];
	comp = buf[31];

	if (arch != IH_ARCH_MIPS || !size || size > len - IH_HEADER_SIZE)
		fail("invalid uImage metadata\r\n");
	if (load < PAYLOAD_RAM_MIN || load >= PAYLOAD_RAM_MAX)
		fail("uImage load address out of range\r\n");
	if (crc32(buf + IH_HEADER_SIZE, size) != dcrc)
		fail("uImage data CRC failed\r\n");

	uart_printf("Loading uImage, load=0x%x, entry=0x%x, size=0x%x, comp=%u\n", load, entry,
		    size, comp);

	dst = (u8 *)(uintptr_t)(load | 0x20000000U);
	if (comp == IH_COMP_NONE) {
		if (size > PAYLOAD_RAM_MAX - load)
			fail("uImage exceeds RAM window\r\n");
		prepare_destination(load, size);
		copy_bytes(dst, buf + IH_HEADER_SIZE, size);
		unpacked = size;
	} else {
		u32 alg;

		if (comp == IH_COMP_GZIP)
			alg = ECONET_COMP_GZIP;
		else if (comp == IH_COMP_LZMA)
			alg = ECONET_COMP_LZMA;
		else
			fail("unsupported uImage compression\r\n");

		prepare_destination(load, PAYLOAD_RAM_MAX - load);
		prepare_destination(DECOMP_WORK_ADDR, ECONET_DECOMP_WORK_SIZE);
		if (econet_decompress_bounded(alg, buf + IH_HEADER_SIZE, size, dst,
					      PAYLOAD_RAM_MAX - load, &unpacked,
					      (void *)(uintptr_t)(DECOMP_WORK_ADDR | 0x20000000U),
					      ECONET_DECOMP_WORK_SIZE))
			fail("uImage decompression failed\r\n");
	}

	if (entry < load || entry >= load + (u32)unpacked)
		fail("uImage entry address out of range\r\n");

	boot_payload(load, entry, (u32)unpacked);
}

static void __attribute__((noreturn)) boot_raw_buffer(const u8 *buf, u32 len)
{
	u32 load = UBOOT_LOAD_ADDR;
	u8 *dst;

	if (!len || load < PAYLOAD_RAM_MIN || load >= PAYLOAD_RAM_MAX ||
	    len > PAYLOAD_RAM_MAX - load)
		fail("raw UBI payload out of range\r\n");

	uart_printf("Loading raw payload, load=0x%x, size=0x%x\n", load, len);
	prepare_destination(load, len);
	dst = (u8 *)(uintptr_t)(load | 0x20000000U);
	copy_bytes(dst, buf, len);
	boot_payload(load, load, len);
}

static int sfc_ubi_read(uint32_t offset, void *dst, size_t len, void *ctx)
{
	(void)ctx;
	return econet_sfc_read(offset, dst, len);
}

static void try_boot_ubi(void)
{
	static const u32 ubi_offsets[] = {
		UBI_OFFSET,
		0x20000U,
		0x40000U,
		0x100000U,
	};
	struct econet_ubi_info info;
	u8 probe[4];
	u8 *staging;
	void *work;
	u32 scanned_end = 0;

	prepare_destination(UBI_WORK_ADDR, UBI_WORK_SIZE);
	prepare_destination(DECOMP_INPUT_ADDR, UBI_STAGING_SIZE);
	work = (void *)(uintptr_t)(UBI_WORK_ADDR | 0x20000000U);
	staging = (u8 *)(uintptr_t)(DECOMP_INPUT_ADDR | 0x20000000U);

	for (u32 i = 0; i < ARRAY_SIZE(ubi_offsets); i++) {
		u32 off = ubi_offsets[i];
		int duplicate = 0;
		int ret;

		for (u32 j = 0; j < i; j++) {
			if (ubi_offsets[j] == off) {
				duplicate = 1;
				break;
			}
		}
		if (duplicate || off < scanned_end)
			continue;

		/* Skip ECNT payload offset as a UBI start when an ECNT header is present. */
		if (off == PAYLOAD_OFFSET && !econet_sfc_read(off, probe, sizeof(probe)) &&
		    be32(probe) == ECONET_BOOT_MAGIC)
			continue;

		info.peb_size = 0;
		info.scanned_pebs = 0;
		ret = econet_ubi_load_volume(sfc_ubi_read, NULL, off, UBI_MAX_SCAN_SIZE,
					     ubi_bootloader_names, ARRAY_SIZE(ubi_bootloader_names),
					     staging, UBI_STAGING_SIZE, work, UBI_WORK_SIZE, &info);
		if (ret == ECONET_UBI_OK) {
			__asm__ volatile("sync" : : : "memory");
			uart_printf("UBI@0x%x: vol=%s (id=%u, peb=0x%x, size=0x%x)\n", off,
				    info.vol_name, info.vol_id, info.peb_size, info.data_size);

			if (info.data_size >= ECONET_BOOT_HEADER_SIZE &&
			    be32(staging) == ECONET_BOOT_MAGIC)
				boot_ecnt_buffer(staging, info.data_size);
			if (info.data_size >= IH_HEADER_SIZE && be32(staging) == IH_MAGIC)
				boot_uimage_buffer(staging, info.data_size);
			boot_raw_buffer(staging, info.data_size);
		}
		if (ret == ECONET_UBI_NO_VOLUME && info.peb_size && info.scanned_pebs) {
			u32 span = info.peb_size * info.scanned_pebs;

			if (off <= UINT32_MAX - span && off + span > scanned_end)
				scanned_end = off + span;
		}
		if (ret != ECONET_UBI_NO_UBI)
			uart_printf("UBI@0x%x: skipped (err=%d)\n", off, ret);
	}
}

static void __attribute__((noreturn)) boot_ecnt_flash(void)
{
	u8 hdr[ECONET_BOOT_HEADER_V2];
	struct ecnt_desc d;
	u32 input_addr;

	if (econet_sfc_read(PAYLOAD_OFFSET, hdr, ECONET_BOOT_HEADER_SIZE))
		fail("payload descriptor read failed\r\n");

	if (be32(hdr) == ECONET_BOOT_MAGIC && be32(hdr + 4) == ECONET_BOOT_VERSION_V2) {
		if (econet_sfc_read(PAYLOAD_OFFSET + ECONET_BOOT_HEADER_SIZE,
				    hdr + ECONET_BOOT_HEADER_SIZE,
				    ECONET_BOOT_HEADER_V2 - ECONET_BOOT_HEADER_SIZE))
			fail("extended descriptor read failed\r\n");
	}

	parse_ecnt_header(hdr, IMAGE_LIMIT - PAYLOAD_OFFSET, &d);

	uart_printf("Loading payload, load=0x%x, entry=0x%x, size=0x%x\n", d.load, d.entry, d.size);
	input_addr = d.compression ? DECOMP_INPUT_ADDR : d.load;
	prepare_destination(input_addr, d.size);
	if (d.compression)
		prepare_destination(d.load, d.unpacked);
	if (econet_sfc_read(PAYLOAD_OFFSET + d.payload_off,
			    (void *)(uintptr_t)(input_addr | 0x20000000U), d.size))
		fail("payload read failed\r\n");

	__asm__ volatile("sync" : : : "memory");
	if (crc32((const u8 *)(uintptr_t)(input_addr | 0x20000000U), d.size) != d.dcrc)
		fail("payload data CRC failed\r\n");

	if (d.compression) {
		prepare_destination(DECOMP_WORK_ADDR, ECONET_DECOMP_WORK_SIZE);
		uart_printf("Decompressing payload, algorithm=%x, size=0x%x\n", d.compression,
			    d.unpacked);
		if (econet_decompress(d.compression,
				      (const u8 *)(uintptr_t)(input_addr | 0x20000000U), d.size,
				      (u8 *)(uintptr_t)(d.load | 0x20000000U), d.unpacked,
				      (void *)(uintptr_t)(DECOMP_WORK_ADDR | 0x20000000U),
				      ECONET_DECOMP_WORK_SIZE))
			fail("payload decompression failed\r\n");
	}
	if (d.version == ECONET_BOOT_VERSION_V2 &&
	    crc32((const u8 *)(uintptr_t)(d.load | 0x20000000U), d.unpacked) != d.unpacked_crc)
		fail("unpacked payload CRC failed\r\n");

	boot_payload(d.load, d.entry, d.unpacked);
}

void __attribute__((noreturn)) loader_main(void)
{
	uart_printf("Econet flash payload loader\n");
	if (econet_sfc_init())
		fail("SFC init failed\r\n");

	try_boot_ubi();
	boot_ecnt_flash();
}
