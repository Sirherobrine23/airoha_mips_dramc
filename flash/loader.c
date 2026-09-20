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


#define FIT_TMP_ADDR 0x80800000U

struct fit_view {
	const u8 *base;
	u32 totalsize;
	const u8 *structure;
	const u8 *structure_end;
	const u8 *strings;
	const u8 *strings_end;
};

static int string_eq(const char *a, const char *b)
{
	while (*a && *b && *a == *b) {
		a++;
		b++;
	}
	return *a == *b;
}

static int prop_string_eq(const u8 *p, u32 len, const char *s)
{
	u32 i = 0;

	while (i < len && s[i]) {
		if (p[i] != (u8)s[i])
			return 0;
		i++;
	}
	return i < len && p[i] == 0 && s[i] == 0;
}

static int copy_prop_string(char *dst, u32 dst_len, const u8 *src, u32 len)
{
	u32 i;

	if (!dst_len)
		return 0;
	for (i = 0; i < len && i + 1 < dst_len; i++) {
		dst[i] = (char)src[i];
		if (!src[i])
			return 1;
	}
	dst[0] = 0;
	return 0;
}

static int range_ok(u32 off, u32 size, u32 total)
{
	return off <= total && size <= total - off;
}

static int fdt_cstr_len(const u8 *p, const u8 *end, u32 *len)
{
	const u8 *q = p;

	while (q < end && *q)
		q++;
	if (q >= end)
		return 0;
	*len = (u32)(q - p);
	return 1;
}

static int fit_init(struct fit_view *v, const u8 *fit, u32 len)
{
	u32 off_struct, off_strings, size_struct, size_strings;

	if (len < 40 || be32(fit) != FDT_MAGIC)
		return 0;
	v->totalsize = be32(fit + 4);
	off_struct = be32(fit + 8);
	off_strings = be32(fit + 12);
	size_strings = be32(fit + 32);
	size_struct = be32(fit + 36);
	if (v->totalsize < 40 || v->totalsize > len ||
	    !range_ok(off_struct, size_struct, v->totalsize) ||
	    !range_ok(off_strings, size_strings, v->totalsize))
		return 0;
	v->base = fit;
	v->structure = fit + off_struct;
	v->structure_end = v->structure + size_struct;
	v->strings = fit + off_strings;
	v->strings_end = v->strings + size_strings;
	return 1;
}

static int fit_name_eq(const char *node, const char *want)
{
	return node && want && string_eq(node, want);
}

static int fit_find_prop(const struct fit_view *v, const char *level1,
			 const char *level2, const char *prop,
			 const u8 **value, u32 *value_len)
{
	const u8 *p = v->structure;
	const char *nodes[4] = { 0 };
	int depth = -1;

	while (p + 4 <= v->structure_end) {
		u32 token = be32(p);
		p += 4;
		if (token == FDT_BEGIN_NODE) {
			u32 nlen, skip;
			if (!fdt_cstr_len(p, v->structure_end, &nlen))
				return 0;
			depth++;
			if (depth < (int)ARRAY_SIZE(nodes))
				nodes[depth] = (const char *)p;
			skip = (nlen + 1u + 3u) & ~3u;
			if ((u32)(v->structure_end - p) < skip)
				return 0;
			p += skip;
		} else if (token == FDT_END_NODE) {
			if (depth < 0)
				return 0;
			if (depth < (int)ARRAY_SIZE(nodes))
				nodes[depth] = 0;
			depth--;
		} else if (token == FDT_PROP) {
			u32 len, nameoff, skip, nlen;
			const char *name;

			if (p + 8 > v->structure_end)
				return 0;
			len = be32(p);
			nameoff = be32(p + 4);
			p += 8;
			skip = (len + 3u) & ~3u;
			if ((u32)(v->structure_end - p) < skip ||
			    nameoff >= (u32)(v->strings_end - v->strings))
				return 0;
			name = (const char *)(v->strings + nameoff);
			if (!fdt_cstr_len((const u8 *)name, v->strings_end, &nlen))
				return 0;
			if (string_eq(name, prop) && depth >= 1 &&
			    fit_name_eq(nodes[1], level1) &&
			    ((!level2 && depth == 1) ||
			     (level2 && depth == 2 && fit_name_eq(nodes[2], level2)))) {
				*value = p;
				*value_len = len;
				return 1;
			}
			p += skip;
		} else if (token == FDT_NOP) {
			continue;
		} else if (token == FDT_END) {
			return 0;
		} else {
			return 0;
		}
	}
	return 0;
}

static int fit_first_child(const struct fit_view *v, const char *parent,
			   char *name, u32 name_len)
{
	const u8 *p = v->structure;
	const char *nodes[4] = { 0 };
	int depth = -1;

	while (p + 4 <= v->structure_end) {
		u32 token = be32(p);
		p += 4;
		if (token == FDT_BEGIN_NODE) {
			u32 nlen, skip, i;
			if (!fdt_cstr_len(p, v->structure_end, &nlen))
				return 0;
			depth++;
			if (depth < (int)ARRAY_SIZE(nodes))
				nodes[depth] = (const char *)p;
			if (depth == 2 && fit_name_eq(nodes[1], parent)) {
				if (nlen + 1 > name_len)
					return 0;
				for (i = 0; i <= nlen; i++)
					name[i] = (char)p[i];
				return 1;
			}
			skip = (nlen + 1u + 3u) & ~3u;
			if ((u32)(v->structure_end - p) < skip)
				return 0;
			p += skip;
		} else if (token == FDT_END_NODE) {
			if (depth < 0)
				return 0;
			if (depth < (int)ARRAY_SIZE(nodes))
				nodes[depth] = 0;
			depth--;
		} else if (token == FDT_PROP) {
			u32 len, skip;
			if (p + 8 > v->structure_end)
				return 0;
			len = be32(p);
			p += 8;
			skip = (len + 3u) & ~3u;
			if ((u32)(v->structure_end - p) < skip)
				return 0;
			p += skip;
		} else if (token == FDT_NOP) {
			continue;
		} else if (token == FDT_END) {
			return 0;
		} else {
			return 0;
		}
	}
	return 0;
}

static int fit_cell32(const u8 *p, u32 len, u32 *value)
{
	if (len == 4) {
		*value = be32(p);
		return 1;
	}
	if (len == 8 && be32(p) == 0) {
		*value = be32(p + 4);
		return 1;
	}
	return 0;
}

static int parse_fit(const u8 *fit, u32 fit_len, u32 flash_limit,
			 u32 *payload_off, u32 *size, u32 *load, u32 *entry)
{
	struct fit_view v;
	const u8 *p, *data = 0;
	u32 n, data_pos = 0, data_size = 0;
	char config[64], firmware[64];
	int kernel_ref = 0;

	if (!fit_init(&v, fit, fit_len))
		return 0;
	if (fit_find_prop(&v, "configurations", 0, "default", &p, &n)) {
		if (!copy_prop_string(config, sizeof(config), p, n))
			return 0;
	} else if (!fit_first_child(&v, "configurations", config, sizeof(config))) {
		return 0;
	}
	if (fit_find_prop(&v, "configurations", config, "firmware", &p, &n) ||
	    fit_find_prop(&v, "configurations", config, "loadables", &p, &n)) {
		if (!copy_prop_string(firmware, sizeof(firmware), p, n))
			return 0;
	} else if (fit_find_prop(&v, "configurations", config, "kernel", &p, &n)) {
		if (!copy_prop_string(firmware, sizeof(firmware), p, n))
			return 0;
		kernel_ref = 1;
	} else {
		return 0;
	}
	if (fit_find_prop(&v, "images", firmware, "compression", &p, &n) &&
	    !prop_string_eq(p, n, "none"))
		return 0;
	if (!kernel_ref && fit_find_prop(&v, "images", firmware, "arch", &p, &n) &&
	    !prop_string_eq(p, n, "mips"))
		return 0;
	if (fit_find_prop(&v, "images", firmware, "type", &p, &n) &&
	    !prop_string_eq(p, n, "firmware") &&
	    !prop_string_eq(p, n, "standalone") &&
	    !(kernel_ref && prop_string_eq(p, n, "kernel")))
		return 0;

	*load = UBOOT_LOAD_ADDR;
	*entry = UBOOT_LOAD_ADDR;
	if (!kernel_ref) {
		if (fit_find_prop(&v, "images", firmware, "load", &p, &n) &&
		    !fit_cell32(p, n, load))
			return 0;
		*entry = *load;
		if (fit_find_prop(&v, "images", firmware, "entry", &p, &n) &&
		    !fit_cell32(p, n, entry))
			return 0;
	}

	if (fit_find_prop(&v, "images", firmware, "data", &p, &n)) {
		data = p;
		*size = n;
		*payload_off = (u32)(data - fit);
	} else {
		if (!fit_find_prop(&v, "images", firmware, "data-size", &p, &n) ||
		    !fit_cell32(p, n, &data_size))
			return 0;
		if (fit_find_prop(&v, "images", firmware, "data-position", &p, &n)) {
			if (!fit_cell32(p, n, &data_pos))
				return 0;
		} else if (fit_find_prop(&v, "images", firmware, "data-offset", &p, &n)) {
			u32 rel;
			if (!fit_cell32(p, n, &rel) || rel > flash_limit - v.totalsize)
				return 0;
			data_pos = v.totalsize + rel;
		} else {
			return 0;
		}
		*payload_off = data_pos;
		*size = data_size;
	}

	if (!*size || !range_ok(*payload_off, *size, flash_limit) ||
	    *load < 0x81000000U || *load >= 0x82000000U ||
	    *size > 0x82000000U - *load ||
	    *entry < *load || *entry >= *load + *size)
		return 0;
	return 1;
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
	u32 fit_size;
	int check_crc = 1;
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
	} else if (magic == FDT_MAGIC) {
		const u8 *fit;

		fit_size = be32(hdr + 4);
		if (fit_size < 40 || fit_size > IMAGE_LIMIT - UBOOT_OFFSET ||
		    fit_size > UBOOT_LOAD_ADDR - FIT_TMP_ADDR)
			fail("unsupported FIT size\r\n");
		fit = (const u8 *)(uintptr_t)(FIT_TMP_ADDR | 0x20000000U);
		if (econet_sfc_read(UBOOT_OFFSET, (void *)(uintptr_t)fit, fit_size))
			fail("FIT read failed\r\n");
		if (!parse_fit(fit, fit_size, IMAGE_LIMIT - UBOOT_OFFSET,
			       &payload_off, &size, &load, &entry))
			fail("unsupported FIT layout\r\n");
		dcrc = 0;
		check_crc = 0;
		kind = "FIT";
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
	if (check_crc && crc32((const u8 *)(uintptr_t)(load | 0x20000000U), size) != dcrc)
		fail("U-Boot data CRC failed\r\n");

	for (u32 p = load; p < load + size; p += 32)
		__asm__ volatile("cache 0x10, 0(%0)" : : "r"(p) : "memory");
	__asm__ volatile("sync; ehb" : : : "memory");

	puts_uart("Starting U-Boot\r\n");
	((void (*)(u32, u32, u32, u32))(uintptr_t)entry)(0, 0, 0, 0);
	fail("U-Boot returned\r\n");
}
