// SPDX-License-Identifier: GPL-2.0+
/*
 * EcoNet/Airoha MIPS host-side image utility.
 *
 * Replaces the former Python helpers with one C program (gzip/xz are used only for optional compression):
 *   econet-image chainloader --image IN [--output OUT] [--check-offset OFF]
 *   econet-image flash --soc SOC --stages FILE --symbols FILE \
 *	   --uboot FILE --load ADDR --output FILE [--minfo FILE]
 *   econet-image bootext --dramc FILE --chainloader FILE --chain-offset OFF \
 *	   --max-size SIZE --output FILE
 *   econet-image tcboot-base --soc SOC --stages FILE --symbols FILE --output FILE
 *   econet-image tcboot --boot FILE --payload FILE --load ADDR --entry ADDR --output FILE
 *   econet-image tcboot-finalize --soc SOC --image FILE [--output FILE] [--stock FILE]
 *   econet-image selftest
 *   econet-image check-en751221-bootext (images + nm symbol listings)
 */

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../flash/ecnt.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define XMODEM_BLOCK			128u
#define CRC_TABLE_ENTRIES		320u
#define CRC_TABLE_SIZE			(CRC_TABLE_ENTRIES * 4u)

#define FLASH_LIMIT				0x100000u
#define FLASH_PAYLOAD			0x20000u
#define FLASH_CRC				(FLASH_PAYLOAD - 4u)
#define FLASH_MINFO				0xff00u

#define IH_MAGIC				0x27051956u
#define IH_HEADER_SIZE			0x40u
#define IH_ARCH_MIPS			5u
#define IH_TYPE_FIRMWARE		5u
#define IH_COMP_NONE			0u

#define FDT_MAGIC				0xd00dfeedu
#define FDT_BEGIN_NODE			0x00000001u
#define FDT_END_NODE			0x00000002u
#define FDT_PROP				0x00000003u
#define FDT_NOP					0x00000004u
#define FDT_END					0x00000009u

#define BOOT_IMAGE_SIZE			0x100000u
#define TCBOOT_MAGIC_OFFSET		0x0cu
#define CRC1_OFFSET				0x1fffcu
#define CRC2_OFFSET				0x3fdfcu

struct blob {
	uint8_t *data;
	size_t len;
};

struct symbol {
	char name[64];
	uint32_t value;
};

struct tcboot_cfg {
	const char *soc;
	bool big_endian;
	uint32_t spl_offset;
	uint32_t spl_limit;
	uint32_t ddr_offset;
	uint32_t ddr_size;
	uint32_t tcboot_len;
	bool spl_is_bootram;
	uint32_t crc_offsets[2];
	unsigned int crc_count;
	uint32_t minfo_offset;
	uint32_t minfo_size;
	bool extended_header;
};

static const struct tcboot_cfg tcboot_cfgs[] = {
	{
		.soc = "en751221",
		.big_endian = true,
		.spl_offset = 0x10000,
		.spl_limit = CRC1_OFFSET,
		.ddr_offset = 0x8000,
		.ddr_size = 0x4f70,
		.tcboot_len = 0,
		.spl_is_bootram = true,
		.crc_offsets = { CRC1_OFFSET },
		.crc_count = 1,
		.minfo_offset = 0xff00,
		.minfo_size = 0x100,
		.extended_header = false,
	},
	{
		.soc = "en751627",
		.big_endian = true,
		.spl_offset = 0x10000,
		.spl_limit = CRC1_OFFSET,
		.ddr_offset = 0x2000,
		.ddr_size = 0xd050,
		.tcboot_len = 0x40000,
		.spl_is_bootram = false,
		.crc_offsets = { CRC1_OFFSET, CRC2_OFFSET },
		.crc_count = 2,
		.minfo_offset = 0x3fe00,
		.minfo_size = 0x200,
		.extended_header = true,
	},
	{
		.soc = "en7528",
		.big_endian = false,
		.spl_offset = 0x10000,
		.spl_limit = 0x1fff8,
		.ddr_offset = 0x4000,
		.ddr_size = 0xb000,
		.tcboot_len = 0x40000,
		.spl_is_bootram = false,
		.crc_offsets = { CRC1_OFFSET, CRC2_OFFSET },
		.crc_count = 2,
		.minfo_offset = 0x3fe00,
		.minfo_size = 0x200,
		.extended_header = true,
	},
	{
		.soc = "en7580",
		.big_endian = false,
		.spl_offset = 0x10000,
		.spl_limit = 0x1fff8,
		.ddr_offset = 0x4000,
		.ddr_size = 0xb000,
		.tcboot_len = 0x40000,
		.spl_is_bootram = false,
		.crc_offsets = { CRC1_OFFSET, CRC2_OFFSET },
		.crc_count = 2,
		.minfo_offset = 0x3fe00,
		.minfo_size = 0x200,
		.extended_header = true,
	},
};

static void die(const char *msg)
{
	fprintf(stderr, "econet-image: %s\n", msg);
	exit(EXIT_FAILURE);
}

static void die_errno(const char *path)
{
	fprintf(stderr, "econet-image: %s: %s\n", path, strerror(errno));
	exit(EXIT_FAILURE);
}

static uint32_t get_be32(const uint8_t *p)
{
	return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
		   ((uint32_t)p[2] << 8) | p[3];
}

static uint32_t get_le32(const uint8_t *p)
{
	return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
		   ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static void put_be32(uint8_t *p, uint32_t v)
{
	p[0] = (uint8_t)(v >> 24);
	p[1] = (uint8_t)(v >> 16);
	p[2] = (uint8_t)(v >> 8);
	p[3] = (uint8_t)v;
}

static void put_le32(uint8_t *p, uint32_t v)
{
	p[0] = (uint8_t)v;
	p[1] = (uint8_t)(v >> 8);
	p[2] = (uint8_t)(v >> 16);
	p[3] = (uint8_t)(v >> 24);
}

static void put_native32(uint8_t *p, uint32_t v, bool big_endian)
{
	if (big_endian)
		put_be32(p, v);
	else
		put_le32(p, v);
}

/* IEEE CRC32, same result as Python zlib.crc32(data). */
static uint32_t crc32_ieee(const uint8_t *data, size_t len)
{
	uint32_t crc = 0xffffffffu;
	size_t i;
	unsigned int bit;

	for (i = 0; i < len; i++) {
		crc ^= data[i];
		for (bit = 0; bit < 8; bit++)
			crc = (crc >> 1) ^ (0xedb88320u & (0u - (crc & 1u)));
	}

	return crc ^ 0xffffffffu;
}

static struct blob read_file(const char *path)
{
	FILE *f;
	struct stat st;
	struct blob b = { 0 };

	if (stat(path, &st) < 0)
		die_errno(path);
	if (st.st_size < 0)
		die("negative file size");

	b.len = (size_t)st.st_size;
	b.data = malloc(b.len ? b.len : 1);
	if (!b.data)
		die("out of memory");

	f = fopen(path, "rb");
	if (!f)
		die_errno(path);
	if (b.len && fread(b.data, 1, b.len, f) != b.len) {
		fclose(f);
		die_errno(path);
	}
	if (fclose(f) != 0)
		die_errno(path);

	return b;
}

static void write_file(const char *path, const void *data, size_t len)
{
	FILE *f = fopen(path, "wb");

	if (!f)
		die_errno(path);
	if (len && fwrite(data, 1, len, f) != len) {
		fclose(f);
		die_errno(path);
	}
	if (fclose(f) != 0)
		die_errno(path);
}

static void free_blob(struct blob *b)
{
	free(b->data);
	b->data = NULL;
	b->len = 0;
}

static uint32_t parse_u32(const char *s)
{
	char *end;
	unsigned long v;

	errno = 0;
	v = strtoul(s, &end, 0);
	if (errno || *s == '\0' || *end != '\0' || v > UINT32_MAX)
		die("invalid 32-bit number");
	return (uint32_t)v;
}

static const struct tcboot_cfg *find_tcboot_cfg(const char *soc)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(tcboot_cfgs); i++)
		if (!strcmp(tcboot_cfgs[i].soc, soc))
			return &tcboot_cfgs[i];
	return NULL;
}

static int finalize_chainloader(const char *input, const char *output,
								bool have_check_offset, uint32_t check_offset)
{
	struct blob in = read_file(input);
	size_t check_start, required_len, chunks, padded_len, i;
	uint8_t *out;

	if (have_check_offset) {
		check_start = check_offset;
		if (in.len < check_start) {
			free_blob(&in);
			fprintf(stderr,
					"econet-image: chainloader binary ends before __chk_start "
					"(len=0x%zx check=0x%zx)\n",
					in.len, check_start);
			return 1;
		}
	} else {
		if (in.len <= CRC_TABLE_SIZE) {
			free_blob(&in);
			fprintf(stderr, "econet-image: chainloader image too short\n");
			return 1;
		}
		check_start = in.len - CRC_TABLE_SIZE;
	}

	required_len = check_start + CRC_TABLE_SIZE;
	if (in.len > required_len) {
		free_blob(&in);
		fprintf(stderr,
				"econet-image: unexpected data after chainloader CRC table "
				"(len=0x%zx expected<=0x%zx)\n",
				in.len, required_len);
		return 1;
	}

	chunks = (check_start + XMODEM_BLOCK - 1) / XMODEM_BLOCK;
	if (chunks > CRC_TABLE_ENTRIES) {
		free_blob(&in);
		fprintf(stderr, "econet-image: chainloader CRC table too small\n");
		return 1;
	}

	padded_len = (required_len + XMODEM_BLOCK - 1) &
				 ~(size_t)(XMODEM_BLOCK - 1);
	out = calloc(1, padded_len);
	if (!out)
		die("out of memory");

	/* Copy only the checked body. The CRC table is always regenerated. */
	memcpy(out, in.data, check_start);

	for (i = 0; i < chunks; i++) {
		size_t start = i * XMODEM_BLOCK;
		size_t end = start + XMODEM_BLOCK;
		uint32_t crc;

		if (end > check_start)
			end = check_start;
		crc = crc32_ieee(out + start, end - start);
		put_be32(out + check_start + i * 4, crc);
	}

	write_file(output ? output : input, out, padded_len);
	printf("  CHAIN	  %s: checked=0x%zx blocks=%zu table=%u size=0x%zx\n",
		   output ? output : input, check_start, chunks,
		   CRC_TABLE_ENTRIES, padded_len);

	free(out);
	free_blob(&in);
	return 0;
}

static int pack_bootext(const char *dramc_path, const char *chain_path,
						uint32_t chain_offset, uint32_t max_size,
						const char *output)
{
	struct blob dramc = read_file(dramc_path);
	struct blob chain = read_file(chain_path);
	size_t used, padded;
	uint8_t *image;
	int ret = 1;

	if (!chain_offset || dramc.len > chain_offset) {
		fprintf(stderr,
				"econet-image: bootext DRAMC (0x%zx bytes) overlaps chainloader at 0x%x\n",
				dramc.len, chain_offset);
		goto out;
	}
	if (chain.len > UINT32_MAX - chain_offset) {
		fprintf(stderr, "econet-image: bootext chainloader is too large\n");
		goto out;
	}
	used = (size_t)chain_offset + chain.len;
	padded = (used + XMODEM_BLOCK - 1u) & ~(size_t)(XMODEM_BLOCK - 1u);
	if (padded > max_size) {
		fprintf(stderr,
				"econet-image: bootext needs 0x%zx bytes, SRAM limit is 0x%x\n",
				padded, max_size);
		goto out;
	}

	image = calloc(1, padded);
	if (!image)
		die("out of memory");
	memcpy(image, dramc.data, dramc.len);
	memcpy(image + chain_offset, chain.data, chain.len);
	write_file(output, image, padded);
	printf("  BOOTEXT  %s: dramc=0x%zx chain@0x%x=0x%zx size=0x%zx\n",
		   output, dramc.len, chain_offset, chain.len, padded);
	free(image);
	ret = 0;
out:
	free_blob(&chain);
	free_blob(&dramc);
	return ret;
}

static bool read_symbols(const char *path, struct symbol **out_syms, size_t *out_count)
{
	FILE *f = fopen(path, "r");
	struct symbol *syms = NULL;
	size_t count = 0, cap = 0;
	char line[512];

	if (!f)
		die_errno(path);

	while (fgets(line, sizeof(line), f)) {
		char addr[64], type[16], name[128], *end;
		unsigned long value;
		struct symbol *tmp;

		if (sscanf(line, "%63s %15s %127s", addr, type, name) != 3)
			continue;
		errno = 0;
		value = strtoul(addr, &end, 16);
		if (errno || *end || value > UINT32_MAX)
			continue;
		if (count == cap) {
			cap = cap ? cap * 2 : 32;
			tmp = realloc(syms, cap * sizeof(*syms));
			if (!tmp)
				die("out of memory");
			syms = tmp;
		}
		if (strlen(name) >= sizeof(syms[count].name))
			continue;
		memcpy(syms[count].name, name, strlen(name) + 1);
		syms[count].value = (uint32_t)value;
		count++;
	}

	if (fclose(f) != 0)
		die_errno(path);
	*out_syms = syms;
	*out_count = count;
	return true;
}

static bool lookup_symbol(const struct symbol *syms, size_t count,
						  const char *name, uint32_t *value)
{
	size_t i;

	for (i = 0; i < count; i++) {
		if (!strcmp(syms[i].name, name)) {
			*value = syms[i].value;
			return true;
		}
	}
	return false;
}

static int stage_interval(const struct symbol *syms, size_t nsyms,
						  const char *name, size_t stages_len,
						  uint32_t *start, uint32_t *end)
{
	char sym_start[96], sym_end[96];
	uint32_t a, b;

	snprintf(sym_start, sizeof(sym_start), "__%s_start", name);
	snprintf(sym_end, sizeof(sym_end), "__%s_end", name);
	if (!lookup_symbol(syms, nsyms, sym_start, &a) ||
		!lookup_symbol(syms, nsyms, sym_end, &b)) {
		fprintf(stderr, "econet-image: missing %s interval symbols\n", name);
		return -1;
	}
	if (a < 0xbfc00000u || b < 0xbfc00000u) {
		fprintf(stderr, "econet-image: invalid %s interval\n", name);
		return -1;
	}
	a -= 0xbfc00000u;
	b -= 0xbfc00000u;
	if (a < 0x60 || a >= b || b > stages_len) {
		fprintf(stderr, "econet-image: invalid %s interval\n", name);
		return -1;
	}
	*start = a;
	*end = b;
	return 0;
}

/* Read-only bootext verification, shared by artifact checks and selftests. */
static bool chainloader_crc_valid(const struct blob *image, size_t checked)
{
	size_t off;

	if (!checked || checked > CRC_TABLE_ENTRIES * XMODEM_BLOCK ||
	    checked > image->len || image->len - checked < CRC_TABLE_SIZE ||
	    image->len % XMODEM_BLOCK)
		return false;
	for (off = 0; off < checked; off += XMODEM_BLOCK) {
		size_t len = checked - off;

		if (len > XMODEM_BLOCK)
			len = XMODEM_BLOCK;
		if (crc32_ieee(image->data + off, len) !=
		    get_be32(image->data + checked + (off / XMODEM_BLOCK) * 4))
			return false;
	}
	return true;
}

static bool en751221_layout_valid(uint32_t ddr_entry, uint32_t ddr_end,
				 uint32_t chain_end, size_t ddr_size, size_t chain_size)
{
	return ddr_entry == 0x9fa32a80u && ddr_end > ddr_entry &&
		ddr_end <= 0x9fa38000u && chain_end > 0x9fa38000u &&
		chain_end <= 0x9fa3c000u && ddr_size > 0x280u &&
		ddr_size <= 0x5800u && chain_size > 0 && chain_size <= 0x4000u;
}

static bool embedded_blob_matches(const struct blob *outer, size_t checked,
				 uint32_t base, uint32_t start, uint32_t end,
				 const struct blob *payload)
{
	size_t off;

	if (start < base || end < start || (start & 3) || (end & 3))
		return false;
	off = start - base;
	if (checked > outer->len || off > checked ||
	    (size_t)(end - start) != payload->len || payload->len > checked - off)
		return false;
	return !memcmp(outer->data + off, payload->data, payload->len);
}

static uint32_t required_symbol(const struct symbol *syms, size_t count,
				const char *name)
{
	uint32_t value;

	if (!lookup_symbol(syms, count, name, &value)) {
		fprintf(stderr, "econet-image: missing bootext symbol %s\n", name);
		exit(EXIT_FAILURE);
	}
	return value;
}

static int check_en751221_bootext(const char *image_path, const char *ddr_path,
				const char *chain_path, const char *boot_symbols,
				const char *ddr_symbols, const char *chain_symbols)
{
	struct blob outer = read_file(image_path), ddr = read_file(ddr_path);
	struct blob chain = read_file(chain_path);
	struct symbol *bs, *ds, *cs;
	size_t nb, nd, nc;
	uint32_t base, check, cb, cc, ss, se, dstart, dend, unused;
	int result = 1;

	read_symbols(boot_symbols, &bs, &nb);
	read_symbols(ddr_symbols, &ds, &nd);
	read_symbols(chain_symbols, &cs, &nc);
	base = required_symbol(bs, nb, "__image_start");
	check = required_symbol(bs, nb, "__chk_start");
	cb = required_symbol(cs, nc, "__image_start");
	cc = required_symbol(cs, nc, "__chk_start");
	ss = required_symbol(bs, nb, "sram_stage_start");
	se = required_symbol(bs, nb, "sram_stage_end");
	dstart = required_symbol(bs, nb, "ddr_stage_start");
	dend = required_symbol(bs, nb, "ddr_stage_end");
	if (base != 0x80009000u || required_symbol(bs, nb, "_start") != base ||
	    cb != 0x9fa38000u || required_symbol(cs, nc, "_start") != cb ||
	    check < base || cc < cb ||
	    lookup_symbol(bs, nb, "chainloader_flash_tcboot", &unused)) {
		fprintf(stderr, "econet-image: invalid bootext entry or bootstrap symbols\n");
		goto out;
	}
	if (!chainloader_crc_valid(&outer, check - base) ||
	    !chainloader_crc_valid(&chain, cc - cb)) {
		fprintf(stderr, "econet-image: bootext CRC table or image length invalid\n");
		goto out;
	}
	if (!en751221_layout_valid(required_symbol(ds, nd, "start"),
				   required_symbol(ds, nd, "_end"),
				   required_symbol(cs, nc, "__image_end"), ddr.len, chain.len)) {
		fprintf(stderr, "econet-image: bootext runtime exceeds its SRAM layout\n");
		goto out;
	}
	if (se > dstart || !embedded_blob_matches(&outer, check - base, base, ss, se, &chain) ||
	    !embedded_blob_matches(&outer, check - base, base, dstart, dend, &ddr)) {
		fprintf(stderr, "econet-image: embedded bootext payload mismatch or overlap\n");
		goto out;
	}
	printf("  CHECK    EN751221 bootext: CRCs, payloads and runtime layout OK (%zu bytes)\n",
	       outer.len);
	result = 0;
out:
	free(bs); free(ds); free(cs);
	free_blob(&outer); free_blob(&ddr); free_blob(&chain);
	return result;
}

static int validate_uboot(const struct blob *uboot, uint32_t load, bool quiet)
{
	uint8_t header[64];
	uint32_t magic, hcrc, size, addr, entry, dcrc;

	if (uboot->len < 64 || uboot->len > FLASH_LIMIT - FLASH_PAYLOAD) {
		if (!quiet)
			fprintf(stderr, "econet-image: U-Boot must fit in the remaining 896 KiB\n");
		return -1;
	}

	magic = get_be32(uboot->data + 0);
	hcrc = get_be32(uboot->data + 4);
	size = get_be32(uboot->data + 12);
	addr = get_be32(uboot->data + 16);
	entry = get_be32(uboot->data + 20);
	dcrc = get_be32(uboot->data + 24);

	memcpy(header, uboot->data, sizeof(header));
	memset(header + 4, 0, 4);
	if (magic != IH_MAGIC || crc32_ieee(header, sizeof(header)) != hcrc) {
		if (!quiet)
			fprintf(stderr, "econet-image: invalid U-Boot legacy header/CRC\n");
		return -1;
	}
	if (size != uboot->len - 64 || crc32_ieee(uboot->data + 64, uboot->len - 64) != dcrc) {
		if (!quiet)
			fprintf(stderr, "econet-image: invalid U-Boot payload size/CRC\n");
		return -1;
	}
	if (addr != load || entry != load ||
		uboot->data[29] != 5 || uboot->data[30] != 5 || uboot->data[31] != 0) {
		if (!quiet)
			fprintf(stderr, "econet-image: expected uncompressed MIPS firmware at configured TEXT_BASE\n");
		return -1;
	}
	if ((uint64_t)load + size > 0x82000000ull) {
		if (!quiet)
			fprintf(stderr, "econet-image: payload exceeds the minimum 32 MiB DRAM window\n");
		return -1;
	}
	return 0;
}


struct fit_view {
	const uint8_t *base;
	size_t input_len;
	uint32_t totalsize;
	const uint8_t *structure;
	const uint8_t *structure_end;
	const uint8_t *strings;
	const uint8_t *strings_end;
};

struct payload_view {
	const uint8_t *data;
	size_t len;
	uint32_t load;
	uint32_t entry;
};

static bool range_ok_size(size_t off, size_t len, size_t total)
{
	return off <= total && len <= total - off;
}

static bool bounded_cstr(const uint8_t *p, const uint8_t *end, size_t *len)
{
	const uint8_t *q = p;

	while (q < end && *q)
		q++;
	if (q >= end)
		return false;
	*len = (size_t)(q - p);
	return true;
}

static bool fit_init(struct fit_view *v, const uint8_t *fit, size_t len)
{
	uint32_t off_struct, off_strings, size_struct, size_strings;

	if (len < 40 || get_be32(fit) != FDT_MAGIC)
		return false;
	v->totalsize = get_be32(fit + 4);
	off_struct = get_be32(fit + 8);
	off_strings = get_be32(fit + 12);
	size_strings = get_be32(fit + 32);
	size_struct = get_be32(fit + 36);
	if (v->totalsize < 40 || v->totalsize > len ||
		!range_ok_size(off_struct, size_struct, v->totalsize) ||
		!range_ok_size(off_strings, size_strings, v->totalsize))
		return false;

	v->base = fit;
	v->input_len = len;
	v->structure = fit + off_struct;
	v->structure_end = v->structure + size_struct;
	v->strings = fit + off_strings;
	v->strings_end = v->strings + size_strings;
	return true;
}

static bool fit_find_prop(const struct fit_view *v, const char *level1,
						  const char *level2, const char *prop,
						  const uint8_t **value, uint32_t *value_len)
{
	const uint8_t *p = v->structure;
	const char *nodes[4] = { NULL };
	int depth = -1;

	while (p + 4 <= v->structure_end) {
		uint32_t token = get_be32(p);
		p += 4;

		if (token == FDT_BEGIN_NODE) {
			size_t nlen, skip;
			if (!bounded_cstr(p, v->structure_end, &nlen))
				return false;
			depth++;
			if (depth < (int)ARRAY_SIZE(nodes))
				nodes[depth] = (const char *)p;
			skip = (nlen + 1 + 3) & ~(size_t)3;
			if ((size_t)(v->structure_end - p) < skip)
				return false;
			p += skip;
		} else if (token == FDT_END_NODE) {
			if (depth < 0)
				return false;
			if (depth < (int)ARRAY_SIZE(nodes))
				nodes[depth] = NULL;
			depth--;
		} else if (token == FDT_PROP) {
			uint32_t len, nameoff;
			size_t skip, nlen;
			const char *name;

			if (p + 8 > v->structure_end)
				return false;
			len = get_be32(p);
			nameoff = get_be32(p + 4);
			p += 8;
			skip = ((size_t)len + 3) & ~(size_t)3;
			if ((size_t)(v->structure_end - p) < skip ||
				nameoff >= (uint32_t)(v->strings_end - v->strings))
				return false;
			name = (const char *)(v->strings + nameoff);
			if (!bounded_cstr((const uint8_t *)name, v->strings_end, &nlen))
				return false;

			if (!strcmp(name, prop) && depth >= 1 && nodes[1] &&
				!strcmp(nodes[1], level1) &&
				((!level2 && depth == 1) ||
				 (level2 && depth == 2 && nodes[2] && !strcmp(nodes[2], level2)))) {
				*value = p;
				*value_len = len;
				return true;
			}
			p += skip;
		} else if (token == FDT_NOP) {
			continue;
		} else if (token == FDT_END) {
			return false;
		} else {
			return false;
		}
	}
	return false;
}

static bool fit_first_child(const struct fit_view *v, const char *parent,
							char *name, size_t name_len)
{
	const uint8_t *p = v->structure;
	const char *nodes[4] = { NULL };
	int depth = -1;

	while (p + 4 <= v->structure_end) {
		uint32_t token = get_be32(p);
		p += 4;
		if (token == FDT_BEGIN_NODE) {
			size_t nlen, skip;
			if (!bounded_cstr(p, v->structure_end, &nlen))
				return false;
			depth++;
			if (depth < (int)ARRAY_SIZE(nodes))
				nodes[depth] = (const char *)p;
			if (depth == 2 && nodes[1] && !strcmp(nodes[1], parent)) {
				if (nlen + 1 > name_len)
					return false;
				memcpy(name, p, nlen + 1);
				return true;
			}
			skip = (nlen + 1 + 3) & ~(size_t)3;
			if ((size_t)(v->structure_end - p) < skip)
				return false;
			p += skip;
		} else if (token == FDT_END_NODE) {
			if (depth < 0)
				return false;
			if (depth < (int)ARRAY_SIZE(nodes))
				nodes[depth] = NULL;
			depth--;
		} else if (token == FDT_PROP) {
			uint32_t len;
			size_t skip;
			if (p + 8 > v->structure_end)
				return false;
			len = get_be32(p);
			p += 8;
			skip = ((size_t)len + 3) & ~(size_t)3;
			if ((size_t)(v->structure_end - p) < skip)
				return false;
			p += skip;
		} else if (token == FDT_NOP) {
			continue;
		} else if (token == FDT_END) {
			return false;
		} else {
			return false;
		}
	}
	return false;
}

static bool fit_copy_string(char *dst, size_t dst_len,
							const uint8_t *src, uint32_t len)
{
	size_t n;

	if (!dst_len)
		return false;
	for (n = 0; n < len && n + 1 < dst_len; n++) {
		dst[n] = (char)src[n];
		if (!src[n])
			return true;
	}
	dst[0] = '\0';
	return false;
}

static bool fit_string_is(const uint8_t *p, uint32_t len, const char *s)
{
	size_t n = strlen(s);
	return len > n && !memcmp(p, s, n) && p[n] == 0;
}

static bool fit_cell32(const uint8_t *p, uint32_t len, uint32_t *value)
{
	if (len == 4) {
		*value = get_be32(p);
		return true;
	}
	if (len == 8 && get_be32(p) == 0) {
		*value = get_be32(p + 4);
		return true;
	}
	return false;
}

static int extract_fit_payload(const struct blob *fit, uint32_t default_load,
							   struct payload_view *out)
{
	struct fit_view v;
	const uint8_t *p, *data = NULL;
	uint32_t n, data_len = 0, data_pos = 0, data_size = 0;
	uint32_t load = default_load, entry = default_load;
	char config[64], firmware[64];
	bool kernel_ref = false;

	if (!fit_init(&v, fit->data, fit->len)) {
		fprintf(stderr, "econet-image: invalid FIT/FDT header\n");
		return -1;
	}

	if (fit_find_prop(&v, "configurations", NULL, "default", &p, &n)) {
		if (!fit_copy_string(config, sizeof(config), p, n)) {
			fprintf(stderr, "econet-image: malformed FIT default configuration\n");
			return -1;
		}
	} else if (!fit_first_child(&v, "configurations", config, sizeof(config))) {
		fprintf(stderr, "econet-image: FIT has no configuration\n");
		return -1;
	}

	if (fit_find_prop(&v, "configurations", config, "firmware", &p, &n) ||
		fit_find_prop(&v, "configurations", config, "loadables", &p, &n)) {
		if (!fit_copy_string(firmware, sizeof(firmware), p, n)) {
			fprintf(stderr, "econet-image: malformed FIT firmware reference\n");
			return -1;
		}
	} else if (fit_find_prop(&v, "configurations", config, "kernel", &p, &n)) {
		/*
		 * U-Boot's generated u-boot.itb commonly exposes the RAM image as
		 * config->kernel, even though the payload is u-boot.bin.  Treat this
		 * as a container reference and keep the target's explicit --load
		 * address authoritative.
		 */
		if (!fit_copy_string(firmware, sizeof(firmware), p, n)) {
			fprintf(stderr, "econet-image: malformed FIT kernel reference\n");
			return -1;
		}
		kernel_ref = true;
	} else {
		fprintf(stderr, "econet-image: FIT configuration has no firmware/loadables/kernel\n");
		return -1;
	}

	if (fit_find_prop(&v, "images", firmware, "compression", &p, &n) &&
		!fit_string_is(p, n, "none")) {
		fprintf(stderr, "econet-image: compressed FIT U-Boot payload is not supported\n");
		return -1;
	}
	if (!kernel_ref && fit_find_prop(&v, "images", firmware, "arch", &p, &n) &&
		!fit_string_is(p, n, "mips")) {
		fprintf(stderr, "econet-image: FIT firmware is not MIPS\n");
		return -1;
	}
	if (fit_find_prop(&v, "images", firmware, "type", &p, &n) &&
		!fit_string_is(p, n, "firmware") &&
		!fit_string_is(p, n, "standalone") &&
		!(kernel_ref && fit_string_is(p, n, "kernel"))) {
		fprintf(stderr, "econet-image: unsupported FIT U-Boot image type\n");
		return -1;
	}
	if (!kernel_ref) {
		if (fit_find_prop(&v, "images", firmware, "load", &p, &n) &&
			!fit_cell32(p, n, &load)) {
			fprintf(stderr, "econet-image: unsupported FIT load address encoding\n");
			return -1;
		}
		entry = load;
		if (fit_find_prop(&v, "images", firmware, "entry", &p, &n) &&
			!fit_cell32(p, n, &entry)) {
			fprintf(stderr, "econet-image: unsupported FIT entry address encoding\n");
			return -1;
		}
	}

	if (fit_find_prop(&v, "images", firmware, "data", &p, &n)) {
		data = p;
		data_len = n;
	} else {
		if (!fit_find_prop(&v, "images", firmware, "data-size", &p, &n) ||
			!fit_cell32(p, n, &data_size)) {
			fprintf(stderr, "econet-image: FIT external firmware has no data-size\n");
			return -1;
		}
		if (fit_find_prop(&v, "images", firmware, "data-position", &p, &n)) {
			if (!fit_cell32(p, n, &data_pos))
				return -1;
		} else if (fit_find_prop(&v, "images", firmware, "data-offset", &p, &n)) {
			uint32_t rel;
			if (!fit_cell32(p, n, &rel) || rel > fit->len - v.totalsize)
				return -1;
			data_pos = v.totalsize + rel;
		} else {
			fprintf(stderr, "econet-image: FIT external firmware has no data position\n");
			return -1;
		}
		if (!range_ok_size(data_pos, data_size, fit->len)) {
			fprintf(stderr, "econet-image: FIT external firmware exceeds input file\n");
			return -1;
		}
		data = fit->data + data_pos;
		data_len = data_size;
	}

	if (!data_len || data_len > FLASH_LIMIT - FLASH_PAYLOAD - ECONET_BOOT_HEADER_SIZE ||
		load < 0x81000000u || (uint64_t)load + data_len > 0x82000000ull ||
		entry < load || (uint64_t)entry >= (uint64_t)load + data_len) {
		fprintf(stderr, "econet-image: FIT U-Boot payload size/load/entry is unsupported\n");
		return -1;
	}

	out->data = data;
	out->len = data_len;
	out->load = load;
	out->entry = entry;
	return 0;
}

static int validate_ecnt(const struct blob *b, bool quiet)
{
	uint8_t hdr[ECONET_BOOT_HEADER_V2];
	uint32_t version, off, size, load, entry, dcrc, hcrc, unpacked, comp = 0;
	size_t header_size;

	if (b->len < ECONET_BOOT_HEADER_SIZE || get_be32(b->data) != ECONET_BOOT_MAGIC)
		return -1;
	version = get_be32(b->data + 4);
	header_size = version == ECONET_BOOT_VERSION ? ECONET_BOOT_HEADER_SIZE :
		version == ECONET_BOOT_VERSION_V2 ? ECONET_BOOT_HEADER_V2 : 0;
	if (!header_size || b->len < header_size)
		goto invalid;
	memcpy(hdr, b->data, header_size);
	hcrc = get_be32(hdr + 28);
	memset(hdr + 28, 0, 4);
	if (crc32_ieee(hdr, header_size) != hcrc)
		goto invalid;
	off = get_be32(hdr + 8);
	size = get_be32(hdr + 12);
	load = get_be32(hdr + 16);
	entry = get_be32(hdr + 20);
	dcrc = get_be32(hdr + 24);
	unpacked = size;
	if (version == ECONET_BOOT_VERSION_V2) {
		comp = get_be32(hdr + 32);
		unpacked = get_be32(hdr + 36);
		if (comp > ECONET_COMP_LZMA || get_be32(hdr + 44) ||
		    (!comp && (unpacked != size || get_be32(hdr + 40) != dcrc)))
			goto invalid;
	}
	if (off < header_size || off > FLASH_LIMIT - FLASH_PAYLOAD || !range_ok_size(off, size, b->len) || !size ||
	    size > FLASH_LIMIT - FLASH_PAYLOAD - off ||
	    load < 0x81000000u || load >= 0x82000000u || !unpacked ||
	    unpacked > 0x82000000u - load ||
	    entry < load || (uint64_t)entry >= (uint64_t)load + unpacked ||
	    crc32_ieee(b->data + off, size) != dcrc)
		goto invalid;
	return 0;
invalid:
	if (!quiet)
		fprintf(stderr, "econet-image: invalid ECNT metadata/CRC\n");
	return -1;
}

static struct blob make_ecnt(const uint8_t *payload, size_t payload_len,
							 uint32_t load, uint32_t entry)
{
	struct blob out = { 0 };
	uint8_t *h;

	if (!payload_len || payload_len > UINT32_MAX ||
		payload_len + ECONET_BOOT_HEADER_SIZE > FLASH_LIMIT - FLASH_PAYLOAD)
		die("payload is too large");
	out.len = ECONET_BOOT_HEADER_SIZE + payload_len;
	out.data = calloc(1, out.len);
	if (!out.data)
		die("out of memory");
	h = out.data;
	put_be32(h + 0, ECONET_BOOT_MAGIC);
	put_be32(h + 4, ECONET_BOOT_VERSION);
	put_be32(h + 8, ECONET_BOOT_HEADER_SIZE);
	put_be32(h + 12, (uint32_t)payload_len);
	put_be32(h + 16, load);
	put_be32(h + 20, entry);
	put_be32(h + 24, crc32_ieee(payload, payload_len));
	put_be32(h + 28, 0);
	put_be32(h + 28, crc32_ieee(h, ECONET_BOOT_HEADER_SIZE));
	memcpy(h + ECONET_BOOT_HEADER_SIZE, payload, payload_len);
	return out;
}

/*
 * Accept legacy uImage, FIT, an existing ECNT container, or a bare raw
 * u-boot.bin. FIT and bare raw payloads are wrapped in the small ECNT
 * container so the early flash loader never needs to parse a large FIT
 * from serial flash. Recovery/XMODEM chainloading can still consume FIT
 * directly.
 */
static int prepare_uboot(const struct blob *input, uint32_t default_load,
						 struct blob *prepared, const char **format)
{
	uint32_t magic = input->len >= 4 ? get_be32(input->data) : 0;
	struct payload_view fit_payload;

	memset(prepared, 0, sizeof(*prepared));
	if (magic == IH_MAGIC) {
		if (validate_uboot(input, default_load, false) < 0)
			return -1;
		*prepared = make_ecnt(input->data + IH_HEADER_SIZE,
				      input->len - IH_HEADER_SIZE,
				      default_load, default_load);
		*format = "legacy->ecnt";
		return 0;
	}
	if (magic == FDT_MAGIC) {
		if (extract_fit_payload(input, default_load, &fit_payload) < 0)
			return -1;

		*prepared = make_ecnt(fit_payload.data, fit_payload.len,
					      fit_payload.load, fit_payload.entry);
		*format = "fit->ecnt";
		return 0;
	}
	if (magic == ECONET_BOOT_MAGIC) {
		if (validate_ecnt(input, false) < 0)
			return -1;
		prepared->data = malloc(input->len);
		if (!prepared->data)
			die("out of memory");
		memcpy(prepared->data, input->data, input->len);
		prepared->len = input->len;
		*format = "ecnt";
		return 0;
	}
	if (magic == 0x7f454c46u) {
		fprintf(stderr, "econet-image: U-Boot ELF is not bootable here; use u-boot.bin, u-boot.img, or FIT\n");
		return -1;
	}
	if (!input->len || input->len > FLASH_LIMIT - FLASH_PAYLOAD - ECONET_BOOT_HEADER_SIZE ||
		(uint64_t)default_load + input->len > 0x82000000ull) {
		fprintf(stderr, "econet-image: raw U-Boot binary is too large\n");
		return -1;
	}
	*prepared = make_ecnt(input->data, input->len, default_load, default_load);
	*format = "raw->ecnt";
	return 0;
}

static int pack_flash_image(const char *soc, const char *stages_path,
							const char *symbols_path, const char *uboot_path,
							uint32_t load, const char *minfo_path,
							const char *output)
{
	struct blob stages = read_file(stages_path);
	struct blob uboot_input = read_file(uboot_path);
	struct blob uboot = { 0 };
	struct blob minfo = { 0 };
	const char *uboot_format = NULL;
	struct symbol *syms = NULL;
	size_t nsyms = 0;
	uint8_t *image = NULL;
	bool big_endian;
	uint32_t move_s, move_e, boot2_s, boot2_e;
	uint32_t loader_s, loader_e, ddr_s, ddr_e;
	int ret = 1;

	if (!strcmp(soc, "en751221") || !strcmp(soc, "en751627"))
		big_endian = true;
	else if (!strcmp(soc, "en7528"))
		big_endian = false;
	else {
		fprintf(stderr, "econet-image: unsupported flash SoC: %s\n", soc);
		goto out;
	}

	if (load < 0x81000000u || load >= 0x82000000u) {
		fprintf(stderr, "econet-image: load address must be in 0x81000000..0x81ffffff\n");
		goto out;
	}
	if (stages.len >= FLASH_MINFO || stages.len < 0x60) {
		fprintf(stderr, "econet-image: initial stages overlap mi.conf or are incomplete\n");
		goto out;
	}
	if (prepare_uboot(&uboot_input, load, &uboot, &uboot_format) < 0)
		goto out;
	if (uboot.len > FLASH_LIMIT - FLASH_PAYLOAD) {
		fprintf(stderr, "econet-image: prepared U-Boot image does not fit in tcboot.bin\n");
		goto out;
	}
	printf("  UBOOT	  %s (%s, 0x%zx bytes)\n", uboot_path, uboot_format, uboot.len);

	read_symbols(symbols_path, &syms, &nsyms);
	if (stage_interval(syms, nsyms, "move_data", stages.len, &move_s, &move_e) < 0 ||
		stage_interval(syms, nsyms, "boot2", stages.len, &boot2_s, &boot2_e) < 0 ||
		stage_interval(syms, nsyms, "lzma", stages.len, &loader_s, &loader_e) < 0 ||
		stage_interval(syms, nsyms, "spram", stages.len, &ddr_s, &ddr_e) < 0)
		goto out;

	if (move_e >= 0x800 || !(move_e <= boot2_s && boot2_s < boot2_e &&
							  boot2_e <= loader_s && loader_s < loader_e &&
							  loader_e <= ddr_s)) {
		fprintf(stderr, "econet-image: invalid first-page placement or overlapping stages\n");
		goto out;
	}

	if (minfo_path) {
		minfo = read_file(minfo_path);
		if (minfo.len != 0x100) {
			fprintf(stderr, "econet-image: mi.conf must be exactly 256 bytes\n");
			goto out;
		}
	}

	image = malloc(FLASH_LIMIT);
	if (!image)
		die("out of memory");
	memset(image, 0xff, FLASH_LIMIT);
	memcpy(image, stages.data, stages.len);

	if (!strcmp(soc, "en751221")) {
		if (get_be32(image) != 0x0bf00012u || memcmp(image + 0x40, "\0\0\0\0\0\0\0\0", 8)) {
			fprintf(stderr, "econet-image: unexpected EN751221 SDK reset/header layout\n");
			goto out;
		}
		put_be32(image, 0x0bf00010u);
	} else {
		memset(image + 8, 0, 0x58);
	}

#define PUT_WORD(off, val) put_native32(image + (off), (val), big_endian)
	if (strcmp(soc, "en751221"))
		PUT_WORD(8, FLASH_PAYLOAD);
	memcpy(image + 12, "6578", 4);
	PUT_WORD(0x10, loader_s);
	PUT_WORD(0x14, loader_e);
	PUT_WORD(0x18, FLASH_PAYLOAD);
	PUT_WORD(0x1c, FLASH_PAYLOAD + (uint32_t)uboot.len);

	if (strcmp(soc, "en751221")) {
		PUT_WORD(0x28, 0x00040010u);
		PUT_WORD(0x30, 0x9fa30000u);
		PUT_WORD(0x34, 0x80000000u);
		if (!strcmp(soc, "en7528"))
			PUT_WORD(0x40, 0x035a3c96u);
		PUT_WORD(0x50, boot2_s);
		PUT_WORD(0x54, boot2_e);
		PUT_WORD(0x58, ddr_s);
		PUT_WORD(0x5c, ddr_e);
	}

	if (!strcmp(soc, "en751627")) {
		static const uint8_t zeros[8] = { 0 };
		if (memcmp(image + ddr_s, zeros, sizeof(zeros))) {
			fprintf(stderr, "econet-image: EN751627 DDR entry prefix is not reserved zero space\n");
			goto out;
		}
		put_be32(image + ddr_s, 0x0be8c0a0u);
		put_be32(image + ddr_s + 4, 0);
	}

	if (minfo_path)
		memcpy(image + FLASH_MINFO, minfo.data, 0x100);
	else
		memset(image + FLASH_MINFO, 0, 0x100);

	PUT_WORD(0xffb0, 0x50414745u);
	if (!strcmp(soc, "en7528"))
		PUT_WORD(0x1fff8, 0x50414745u);

	memcpy(image + FLASH_PAYLOAD, uboot.data, uboot.len);
	PUT_WORD(FLASH_CRC, crc32_ieee(image, FLASH_CRC) ^ 0xffffffffu);
#undef PUT_WORD

	write_file(output, image, FLASH_LIMIT);
	ret = 0;
out:
	free(image);
	free(syms);
	free_blob(&minfo);
	free_blob(&uboot);
	free_blob(&uboot_input);
	free_blob(&stages);
	return ret;
}

static int pack_tcboot_base(const char *soc, const char *stages_path,
				    const char *symbols_path, const char *minfo_path,
				    const char *output)
{
	struct blob stages = read_file(stages_path);
	struct blob minfo = { 0 };
	struct symbol *syms = NULL;
	size_t nsyms = 0;
	uint8_t *image = NULL;
	bool big_endian;
	uint32_t move_s, move_e, boot2_s, boot2_e;
	uint32_t loader_s, loader_e, ddr_s, ddr_e;
	int ret = 1;

	if (!strcmp(soc, "en751221") || !strcmp(soc, "en751627"))
		big_endian = true;
	else if (!strcmp(soc, "en7528"))
		big_endian = false;
	else {
		fprintf(stderr, "econet-image: unsupported TCBoot SoC: %s\n", soc);
		goto out;
	}

	if (stages.len >= FLASH_CRC || stages.len < 0x60) {
		fprintf(stderr, "econet-image: initial stages exceed base region or are incomplete\n");
		goto out;
	}

	read_symbols(symbols_path, &syms, &nsyms);
	if (stage_interval(syms, nsyms, "move_data", stages.len, &move_s, &move_e) < 0 ||
		stage_interval(syms, nsyms, "boot2", stages.len, &boot2_s, &boot2_e) < 0 ||
		stage_interval(syms, nsyms, "lzma", stages.len, &loader_s, &loader_e) < 0 ||
		stage_interval(syms, nsyms, "spram", stages.len, &ddr_s, &ddr_e) < 0)
		goto out;

	if (move_e >= 0x800 || !(move_e <= boot2_s && boot2_s < boot2_e &&
					  boot2_e <= loader_s && loader_s < loader_e &&
					  boot2_e <= ddr_s && ddr_s < ddr_e &&
					  (loader_e <= ddr_s || ddr_e <= loader_s))) {
		fprintf(stderr, "econet-image: invalid first-page placement or overlapping stages\n");
		goto out;
	}

	/* Stages may occupy the second 64 KiB, but never mi.conf/PAGE. */
	if ((move_s < 0x10000 && move_e > FLASH_MINFO) ||
	    (boot2_s < 0x10000 && boot2_e > FLASH_MINFO) ||
	    (loader_s < 0x10000 && loader_e > FLASH_MINFO) ||
	    (ddr_s < 0x10000 && ddr_e > FLASH_MINFO)) {
		fprintf(stderr, "econet-image: stages overlap manufacturing data\n");
		goto out;
	}

	if (minfo_path) {
		minfo = read_file(minfo_path);
		if (minfo.len != 0x100) {
			fprintf(stderr, "econet-image: mi.conf must be exactly 256 bytes\n");
			goto out;
		}
	}

	image = malloc(FLASH_PAYLOAD);
	if (!image)
		die("out of memory");
	memset(image, 0xff, FLASH_PAYLOAD);
	memcpy(image, stages.data, stages.len);

	if (!strcmp(soc, "en751221")) {
		if (get_be32(image) != 0x0bf00012u ||
		    memcmp(image + 0x40, "\0\0\0\0\0\0\0\0", 8)) {
			fprintf(stderr, "econet-image: unexpected EN751221 SDK reset/header layout\n");
			goto out;
		}
		put_be32(image, 0x0bf00010u);
	} else {
		memset(image + 8, 0, 0x58);
	}

#define PUT_WORD(off, val) put_native32(image + (off), (val), big_endian)
	if (strcmp(soc, "en751221"))
		PUT_WORD(8, FLASH_PAYLOAD);
	memcpy(image + 12, "6578", 4);
	PUT_WORD(0x10, loader_s);
	PUT_WORD(0x14, loader_e);
	PUT_WORD(0x18, FLASH_PAYLOAD);
	/* Empty base image: the OpenWrt image builder extends this to payload end. */
	PUT_WORD(0x1c, FLASH_PAYLOAD);

	if (strcmp(soc, "en751221")) {
		PUT_WORD(0x28, 0x00040010u);
		PUT_WORD(0x30, 0x9fa30000u);
		PUT_WORD(0x34, 0x80000000u);
		if (!strcmp(soc, "en7528"))
			PUT_WORD(0x40, 0x035a3c96u);
		PUT_WORD(0x50, boot2_s);
		PUT_WORD(0x54, boot2_e);
		PUT_WORD(0x58, ddr_s);
		PUT_WORD(0x5c, ddr_e);
	}

	if (!strcmp(soc, "en751627")) {
		static const uint8_t zeros[8] = { 0 };
		if (memcmp(image + ddr_s, zeros, sizeof(zeros))) {
			fprintf(stderr, "econet-image: EN751627 DDR entry prefix is not reserved zero space\n");
			goto out;
		}
		put_be32(image + ddr_s, 0x0be8c0a0u);
		put_be32(image + ddr_s + 4, 0);
	}

	if (minfo_path)
		memcpy(image + FLASH_MINFO, minfo.data, 0x100);
	else
		memset(image + FLASH_MINFO, 0, 0x100);

	PUT_WORD(0xffb0, 0x50414745u);
	if (!strcmp(soc, "en7528"))
		PUT_WORD(0x1fff8, 0x50414745u);
	PUT_WORD(FLASH_CRC, crc32_ieee(image, FLASH_CRC) ^ 0xffffffffu);
#undef PUT_WORD

	write_file(output, image, FLASH_PAYLOAD);
	ret = 0;
out:
	free(image);
	free(syms);
	free_blob(&minfo);
	free_blob(&stages);
	return ret;
}

static int tcboot_base_endian(const struct blob *boot, bool *big_endian)
{
	uint32_t be_start, le_start, be_end, le_end;

	if (boot->len != FLASH_PAYLOAD) {
		fprintf(stderr, "econet-image: TCBoot base must be exactly 0x%x bytes, got 0x%zx\n",
				FLASH_PAYLOAD, boot->len);
		return -1;
	}
	if (memcmp(boot->data + 12, "6578", 4)) {
		fprintf(stderr, "econet-image: TCBoot magic '6578' is missing at offset 0x0c\n");
		return -1;
	}

	be_start = get_be32(boot->data + 0x18);
	le_start = get_le32(boot->data + 0x18);
	be_end = get_be32(boot->data + 0x1c);
	le_end = get_le32(boot->data + 0x1c);
	if (be_start == FLASH_PAYLOAD && be_end == FLASH_PAYLOAD &&
	    !(le_start == FLASH_PAYLOAD && le_end == FLASH_PAYLOAD)) {
		*big_endian = true;
		return 0;
	}
	if (le_start == FLASH_PAYLOAD && le_end == FLASH_PAYLOAD &&
	    !(be_start == FLASH_PAYLOAD && be_end == FLASH_PAYLOAD)) {
		*big_endian = false;
		return 0;
	}

	fprintf(stderr, "econet-image: TCBoot base has invalid payload bounds\n");
	return -1;
}

/* Use argv-based child processes, never a shell. Compression is optional;
 * ordinary ECNT v1 packaging keeps the host tool dependency-free.
 */
static struct blob compress_payload(const struct blob *input, uint32_t comp)
{
	struct blob out = { 0 };
	FILE *source = tmpfile(), *dest = tmpfile();
	pid_t child;
	int status;
	long length;

	if (!source || !dest)
		die("cannot create compression temporary files");
	if (fwrite(input->data, 1, input->len, source) != input->len || fflush(source))
		die("cannot write compression input");
	rewind(source);
	child = fork();
	if (child < 0)
		die("cannot start compressor");
	if (!child) {
		if (dup2(fileno(source), STDIN_FILENO) < 0 ||
		    dup2(fileno(dest), STDOUT_FILENO) < 0)
			_exit(126);
		if (comp == ECONET_COMP_GZIP)
			execlp("gzip", "gzip", "-n", "-9", "-c", (char *)NULL);
		else
			execlp("xz", "xz", "--format=lzma",
			       "--lzma1=dict=1MiB,lc=3,lp=0,pb=2", "-c", (char *)NULL);
		perror("econet-image: compressor");
		_exit(127);
	}
	while (waitpid(child, &status, 0) < 0) {
		if (errno != EINTR)
			die("cannot wait for compressor");
	}
	if (!WIFEXITED(status) || WEXITSTATUS(status))
		die("compression failed (gzip or xz must be installed)");
	if (fseek(dest, 0, SEEK_END) || (length = ftell(dest)) <= 0 ||
	    (unsigned long)length > FLASH_LIMIT - FLASH_PAYLOAD - ECONET_BOOT_HEADER_V2)
		die("compressed payload does not fit in the boot region");
	out.len = (size_t)length;
	out.data = malloc(out.len);
	if (!out.data)
		die("out of memory");
	rewind(dest);
	if (fread(out.data, 1, out.len, dest) != out.len)
		die("cannot read compression output");
	fclose(source);
	fclose(dest);
	return out;
}

static struct blob make_ecnt_compressed(const struct blob *payload,
		uint32_t load, uint32_t entry, uint32_t comp)
{
	struct blob stored = compress_payload(payload, comp);
	struct blob out = { 0 };
	uint8_t *h;

	out.len = ECONET_BOOT_HEADER_V2 + stored.len;
	out.data = calloc(1, out.len);
	if (!out.data)
		die("out of memory");
	h = out.data;
	put_be32(h, ECONET_BOOT_MAGIC);
	put_be32(h + 4, ECONET_BOOT_VERSION_V2);
	put_be32(h + 8, ECONET_BOOT_HEADER_V2);
	put_be32(h + 12, (uint32_t)stored.len);
	put_be32(h + 16, load);
	put_be32(h + 20, entry);
	put_be32(h + 24, crc32_ieee(stored.data, stored.len));
	put_be32(h + 32, comp);
	put_be32(h + 36, (uint32_t)payload->len);
	put_be32(h + 40, crc32_ieee(payload->data, payload->len));
	put_be32(h + 28, crc32_ieee(h, ECONET_BOOT_HEADER_V2));
	memcpy(h + ECONET_BOOT_HEADER_V2, stored.data, stored.len);
	free_blob(&stored);
	return out;
}

static int compose_tcboot(const char *boot_path, const char *payload_path,
			  uint32_t load, uint32_t entry, const char *output_path,
			  uint32_t compression)
{
	struct blob boot = read_file(boot_path);
	struct blob payload = read_file(payload_path);
	struct blob container = { 0 };
	uint8_t *image = NULL;
	bool big_endian;
	size_t image_len;
	int ret = 1;

	if (tcboot_base_endian(&boot, &big_endian) < 0)
		goto out;
	if (!payload.len || payload.len > UINT32_MAX ||
	    load < 0x81000000u || load >= 0x82000000u ||
	    payload.len > 0x82000000u - load ||
	    entry < load || (uint64_t)entry >= (uint64_t)load + payload.len) {
		fprintf(stderr, "econet-image: payload size/load/entry is unsupported\n");
		goto out;
	}

	container = compression ? make_ecnt_compressed(&payload, load, entry, compression) :
		make_ecnt(payload.data, payload.len, load, entry);
	image_len = FLASH_PAYLOAD + container.len;
	if (image_len > FLASH_LIMIT) {
		fprintf(stderr, "econet-image: payload does not fit in the 1 MiB boot region\n");
		goto out;
	}

	image = malloc(image_len);
	if (!image)
		die("out of memory");
	memcpy(image, boot.data, boot.len);
	memcpy(image + FLASH_PAYLOAD, container.data, container.len);

	put_native32(image + 0x18, FLASH_PAYLOAD, big_endian);
	put_native32(image + 0x1c, (uint32_t)image_len, big_endian);
	put_native32(image + FLASH_CRC,
		     crc32_ieee(image, FLASH_CRC) ^ 0xffffffffu, big_endian);

	write_file(output_path, image, image_len);
	ret = 0;
out:
	free(image);
	free_blob(&container);
	free_blob(&payload);
	free_blob(&boot);
	return ret;
}

static int finalize_tcboot(const char *soc, const char *image_path,
						   const char *output_path, const char *stock_path)
{
	const struct tcboot_cfg *cfg = find_tcboot_cfg(soc);
	struct blob image = { 0 }, stock = { 0 };
	uint32_t payload_size, spl_end;
	unsigned int i;
	int ret = 1;

	if (!cfg) {
		fprintf(stderr, "econet-image: unsupported SoC '%s'\n", soc);
		return 1;
	}

	image = read_file(image_path);
	if (image.len != BOOT_IMAGE_SIZE) {
		fprintf(stderr, "econet-image: combined image must be exactly 0x%x bytes, got 0x%zx\n",
				BOOT_IMAGE_SIZE, image.len);
		goto out;
	}
	if (memcmp(image.data + TCBOOT_MAGIC_OFFSET, "6578", 4)) {
		fprintf(stderr, "econet-image: TCBoot magic '6578' is missing at offset 0x0c\n");
		goto out;
	}

	if (stock_path) {
		stock = read_file(stock_path);
		if (stock.len < (size_t)cfg->minfo_offset + cfg->minfo_size) {
			fprintf(stderr, "econet-image: stock bootloader is too small for manufacturing data\n");
			goto out;
		}
		memcpy(image.data + cfg->minfo_offset, stock.data + cfg->minfo_offset,
			   cfg->minfo_size);
	}

	if (cfg->spl_offset + IH_HEADER_SIZE > image.len ||
		get_be32(image.data + cfg->spl_offset) != IH_MAGIC) {
		fprintf(stderr, "econet-image: invalid SPL legacy image header at 0x%x\n",
				cfg->spl_offset);
		goto out;
	}
	payload_size = get_be32(image.data + cfg->spl_offset + 12);
	if (!payload_size) {
		fprintf(stderr, "econet-image: SPL legacy image has an empty payload\n");
		goto out;
	}
	spl_end = cfg->spl_offset + IH_HEADER_SIZE + payload_size;
	if (spl_end > cfg->spl_limit) {
		fprintf(stderr, "econet-image: SPL ends at 0x%x, beyond TCBoot limit 0x%x\n",
				spl_end, cfg->spl_limit);
		goto out;
	}

	put_native32(image.data + 0x08, cfg->tcboot_len, cfg->big_endian);
	if (cfg->spl_is_bootram) {
		put_native32(image.data + 0x10, 0, cfg->big_endian);
		put_native32(image.data + 0x14, 0, cfg->big_endian);
		put_native32(image.data + 0x18, cfg->spl_offset, cfg->big_endian);
		put_native32(image.data + 0x1c, spl_end, cfg->big_endian);
	} else {
		put_native32(image.data + 0x10, cfg->spl_offset, cfg->big_endian);
		put_native32(image.data + 0x14, spl_end, cfg->big_endian);
		put_native32(image.data + 0x18, 0, cfg->big_endian);
		put_native32(image.data + 0x1c, 0, cfg->big_endian);
	}

	if (cfg->extended_header) {
		put_native32(image.data + 0x58, cfg->ddr_offset, cfg->big_endian);
		put_native32(image.data + 0x5c, cfg->ddr_offset + cfg->ddr_size,
					 cfg->big_endian);
	}

	for (i = 0; i < cfg->crc_count; i++) {
		uint32_t off = cfg->crc_offsets[i];
		uint32_t crc = ~crc32_ieee(image.data, off);
		put_native32(image.data + off, crc, cfg->big_endian);
	}

	write_file(output_path ? output_path : image_path, image.data, image.len);
	ret = 0;
out:
	free_blob(&stock);
	free_blob(&image);
	return ret;
}

static void usage(FILE *f)
{
	fprintf(f,
		"usage:\n"
		"  econet-image chainloader --image FILE [--output FILE] [--check-offset OFF]\n"
		"  econet-image flash --soc SOC --stages FILE --symbols FILE --uboot FILE\\\n"
		"\t  --load ADDR --output FILE [--minfo FILE]\n"
		"  econet-image bootext --dramc FILE --chainloader FILE --chain-offset OFF\\\n"
		"\t  --max-size SIZE --output FILE\n"
		"  econet-image tcboot-base --soc SOC --stages FILE --symbols FILE --output FILE [--minfo FILE]\n"
		"  econet-image tcboot --boot FILE --payload FILE --load ADDR --entry ADDR --output FILE [--compression none|gzip|lzma]\n"
		"  econet-image tcboot-finalize --soc SOC --image FILE [--output FILE] [--stock FILE]\n"
		"  econet-image check-en751221-bootext --image FILE --dramc FILE --chainloader FILE\\\n"
		"\t  --bootstrap-symbols FILE --dramc-symbols FILE --chainloader-symbols FILE\n"
		"  econet-image selftest\n");
}

static const char *next_arg(int *i, int argc, char **argv, const char *opt)
{
	if (*i + 1 >= argc) {
		fprintf(stderr, "econet-image: %s requires an argument\n", opt);
		exit(2);
	}
	return argv[++*i];
}

static int cmd_chainloader(int argc, char **argv)
{
	const char *image = NULL, *output = NULL;
	uint32_t check_offset = 0;
	bool have_check_offset = false;
	int i;

	for (i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "--image"))
			image = next_arg(&i, argc, argv, "--image");
		else if (!strcmp(argv[i], "--output"))
			output = next_arg(&i, argc, argv, "--output");
		else if (!strcmp(argv[i], "--check-offset")) {
			check_offset = parse_u32(next_arg(&i, argc, argv,
											  "--check-offset"));
			have_check_offset = true;
		} else {
			usage(stderr);
			return 2;
		}
	}
	if (!image) {
		usage(stderr);
		return 2;
	}
	return finalize_chainloader(image, output,
								have_check_offset, check_offset);
}

static int cmd_flash(int argc, char **argv)
{
	const char *soc = NULL, *stages = NULL, *symbols = NULL;
	const char *uboot = NULL, *output = NULL, *minfo = NULL;
	uint32_t load = 0;
	bool have_load = false;
	int i;

	for (i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "--soc"))
			soc = next_arg(&i, argc, argv, "--soc");
		else if (!strcmp(argv[i], "--stages"))
			stages = next_arg(&i, argc, argv, "--stages");
		else if (!strcmp(argv[i], "--symbols"))
			symbols = next_arg(&i, argc, argv, "--symbols");
		else if (!strcmp(argv[i], "--uboot"))
			uboot = next_arg(&i, argc, argv, "--uboot");
		else if (!strcmp(argv[i], "--load")) {
			load = parse_u32(next_arg(&i, argc, argv, "--load"));
			have_load = true;
		} else if (!strcmp(argv[i], "--output"))
			output = next_arg(&i, argc, argv, "--output");
		else if (!strcmp(argv[i], "--minfo"))
			minfo = next_arg(&i, argc, argv, "--minfo");
		else {
			usage(stderr);
			return 2;
		}
	}

	if (!soc || !stages || !symbols || !uboot || !output || !have_load) {
		usage(stderr);
		return 2;
	}
	return pack_flash_image(soc, stages, symbols, uboot, load, minfo, output);
}

static int cmd_bootext(int argc, char **argv)
{
	const char *dramc = NULL, *chain = NULL, *output = NULL;
	uint32_t chain_offset = 0, max_size = 0;
	bool have_offset = false, have_max = false;
	int i;

	for (i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "--dramc"))
			dramc = next_arg(&i, argc, argv, "--dramc");
		else if (!strcmp(argv[i], "--chainloader"))
			chain = next_arg(&i, argc, argv, "--chainloader");
		else if (!strcmp(argv[i], "--chain-offset")) {
			chain_offset = parse_u32(next_arg(&i, argc, argv, "--chain-offset"));
			have_offset = true;
		} else if (!strcmp(argv[i], "--max-size")) {
			max_size = parse_u32(next_arg(&i, argc, argv, "--max-size"));
			have_max = true;
		} else if (!strcmp(argv[i], "--output"))
			output = next_arg(&i, argc, argv, "--output");
		else {
			usage(stderr);
			return 2;
		}
	}
	if (!dramc || !chain || !output || !have_offset || !have_max) {
		usage(stderr);
		return 2;
	}
	return pack_bootext(dramc, chain, chain_offset, max_size, output);
}

static int cmd_tcboot_base(int argc, char **argv)
{
	const char *soc = NULL, *stages = NULL, *symbols = NULL;
	const char *output = NULL, *minfo = NULL;
	int i;

	for (i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "--soc"))
			soc = next_arg(&i, argc, argv, "--soc");
		else if (!strcmp(argv[i], "--stages"))
			stages = next_arg(&i, argc, argv, "--stages");
		else if (!strcmp(argv[i], "--symbols"))
			symbols = next_arg(&i, argc, argv, "--symbols");
		else if (!strcmp(argv[i], "--output"))
			output = next_arg(&i, argc, argv, "--output");
		else if (!strcmp(argv[i], "--minfo"))
			minfo = next_arg(&i, argc, argv, "--minfo");
		else {
			usage(stderr);
			return 2;
		}
	}
	if (!soc || !stages || !symbols || !output) {
		usage(stderr);
		return 2;
	}
	return pack_tcboot_base(soc, stages, symbols, minfo, output);
}

static int cmd_tcboot(int argc, char **argv)
{
	const char *boot = NULL, *payload = NULL, *output = NULL;
	uint32_t load = 0, entry = 0, compression = ECONET_COMP_NONE;
	bool have_load = false, have_entry = false;
	int i;

	for (i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "--boot"))
			boot = next_arg(&i, argc, argv, "--boot");
		else if (!strcmp(argv[i], "--payload"))
			payload = next_arg(&i, argc, argv, "--payload");
		else if (!strcmp(argv[i], "--compression")) {
			const char *name = next_arg(&i, argc, argv, "--compression");

			if (!strcmp(name, "none"))
				compression = ECONET_COMP_NONE;
			else if (!strcmp(name, "gzip"))
				compression = ECONET_COMP_GZIP;
			else if (!strcmp(name, "lzma"))
				compression = ECONET_COMP_LZMA;
			else {
				fprintf(stderr, "econet-image: unsupported compression '%s'\n", name);
				return 2;
			}
		} else if (!strcmp(argv[i], "--load")) {
			load = parse_u32(next_arg(&i, argc, argv, "--load"));
			have_load = true;
		} else if (!strcmp(argv[i], "--entry")) {
			entry = parse_u32(next_arg(&i, argc, argv, "--entry"));
			have_entry = true;
		} else if (!strcmp(argv[i], "--output"))
			output = next_arg(&i, argc, argv, "--output");
		else {
			usage(stderr);
			return 2;
		}
	}
	if (!boot || !payload || !output || !have_load || !have_entry) {
		usage(stderr);
		return 2;
	}
	return compose_tcboot(boot, payload, load, entry, output, compression);
}

static int cmd_tcboot_finalize(int argc, char **argv)
{
	const char *soc = NULL, *image = NULL, *output = NULL, *stock = NULL;
	int i;

	for (i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "--soc"))
			soc = next_arg(&i, argc, argv, "--soc");
		else if (!strcmp(argv[i], "--image"))
			image = next_arg(&i, argc, argv, "--image");
		else if (!strcmp(argv[i], "--output"))
			output = next_arg(&i, argc, argv, "--output");
		else if (!strcmp(argv[i], "--stock"))
			stock = next_arg(&i, argc, argv, "--stock");
		else {
			usage(stderr);
			return 2;
		}
	}
	if (!soc || !image) {
		usage(stderr);
		return 2;
	}
	return finalize_tcboot(soc, image, output, stock);
}

/* Small dependency-free regression suite for the host-side image formats. */
struct flash_fixture {
	uint8_t stages[0x9000];
	struct symbol syms[8];
	uint8_t header[64];
	uint8_t data[256];
	size_t data_len;
};

static void fixture_init(struct flash_fixture *f)
{
	static const struct {
		const char *name;
		uint32_t a, b;
	} ranges[] = {
		{ "move_data", 0x280, 0x768 },
		{ "boot2", 0x768, 0x1660 },
		{ "lzma", 0x1660, 0x2000 },
		{ "spram", 0x2000, 0x9000 },
	};
	size_t i;

	memset(f, 0, sizeof(*f));
	f->stages[0] = 0x0b;
	f->stages[1] = 0xf0;
	f->stages[2] = 0x00;
	f->stages[3] = 0x18;

	for (i = 0; i < ARRAY_SIZE(ranges); i++) {
		snprintf(f->syms[i * 2].name, sizeof(f->syms[i * 2].name),
				 "__%s_start", ranges[i].name);
		f->syms[i * 2].value = 0xbfc00000u + ranges[i].a;
		snprintf(f->syms[i * 2 + 1].name, sizeof(f->syms[i * 2 + 1].name),
				 "__%s_end", ranges[i].name);
		f->syms[i * 2 + 1].value = 0xbfc00000u + ranges[i].b;
	}

	f->data_len = 17 * 15;
	for (i = 0; i < f->data_len; i++)
		f->data[i] = "U-Boot payload\0"[i % 15];

	put_be32(f->header + 0, IH_MAGIC);
	put_be32(f->header + 12, (uint32_t)f->data_len);
	put_be32(f->header + 16, 0x81000000u);
	put_be32(f->header + 20, 0x81000000u);
	put_be32(f->header + 24, crc32_ieee(f->data, f->data_len));
	f->header[28] = 17;
	f->header[29] = 5;
	f->header[30] = 5;
	f->header[31] = 0;
	put_be32(f->header + 4, 0);
	put_be32(f->header + 4, crc32_ieee(f->header, sizeof(f->header)));
}


struct test_fit_builder {
	uint8_t data[2048];
	size_t len;
};

static void test_fit_put32(struct test_fit_builder *b, uint32_t v)
{
	if (b->len + 4 > sizeof(b->data))
		die("selftest FIT buffer overflow");
	put_be32(b->data + b->len, v);
	b->len += 4;
}

static void test_fit_bytes(struct test_fit_builder *b, const void *p, size_t len)
{
	if (b->len + len > sizeof(b->data))
		die("selftest FIT buffer overflow");
	memcpy(b->data + b->len, p, len);
	b->len += len;
	while (b->len & 3) {
		if (b->len >= sizeof(b->data))
			die("selftest FIT buffer overflow");
		b->data[b->len++] = 0;
	}
}

static void test_fit_begin(struct test_fit_builder *b, const char *name)
{
	test_fit_put32(b, FDT_BEGIN_NODE);
	test_fit_bytes(b, name, strlen(name) + 1);
}

static void test_fit_end(struct test_fit_builder *b)
{
	test_fit_put32(b, FDT_END_NODE);
}

static void test_fit_prop(struct test_fit_builder *b, uint32_t nameoff,
						  const void *data, size_t len)
{
	test_fit_put32(b, FDT_PROP);
	test_fit_put32(b, (uint32_t)len);
	test_fit_put32(b, nameoff);
	test_fit_bytes(b, data, len);
}

static size_t make_test_fit(uint8_t *out, size_t out_len,
							const uint8_t *payload, size_t payload_len,
							bool kernel_style)
{
	static const char strings[] =
		"data\0type\0arch\0compression\0load\0entry\0default\0firmware\0kernel\0";
	enum {
		OFF_DATA = 0,
		OFF_TYPE = 5,
		OFF_ARCH = 10,
		OFF_COMPRESSION = 15,
		OFF_LOAD = 27,
		OFF_ENTRY = 32,
		OFF_DEFAULT = 38,
		OFF_FIRMWARE = 46,
		OFF_KERNEL = 55,
	};
	struct test_fit_builder st = { { 0 }, 0 };
	uint8_t cell[4];
	const size_t reserve_len = 16;
	size_t off_struct = 40 + reserve_len;
	size_t off_strings, total;

	test_fit_begin(&st, "");
	test_fit_begin(&st, "images");
	test_fit_begin(&st, "uboot");
	test_fit_prop(&st, OFF_DATA, payload, payload_len);
	test_fit_prop(&st, OFF_TYPE,
				  kernel_style ? "kernel" : "firmware",
				  kernel_style ? sizeof("kernel") : sizeof("firmware"));
	test_fit_prop(&st, OFF_ARCH,
				  kernel_style ? "arm" : "mips",
				  kernel_style ? sizeof("arm") : sizeof("mips"));
	test_fit_prop(&st, OFF_COMPRESSION, "none", sizeof("none"));
	put_be32(cell, kernel_style ? 0x81e00000u : 0x81000000u);
	test_fit_prop(&st, OFF_LOAD, cell, sizeof(cell));
	test_fit_prop(&st, OFF_ENTRY, cell, sizeof(cell));
	test_fit_end(&st);
	test_fit_end(&st);
	test_fit_begin(&st, "configurations");
	test_fit_prop(&st, OFF_DEFAULT, "conf-1", sizeof("conf-1"));
	test_fit_begin(&st, "conf-1");
	test_fit_prop(&st, kernel_style ? OFF_KERNEL : OFF_FIRMWARE,
				  "uboot", sizeof("uboot"));
	test_fit_end(&st);
	test_fit_end(&st);
	test_fit_end(&st);
	test_fit_put32(&st, FDT_END);

	off_strings = off_struct + st.len;
	total = off_strings + sizeof(strings);
	if (total > out_len)
		die("selftest FIT output overflow");
	memset(out, 0, total);
	put_be32(out + 0, FDT_MAGIC);
	put_be32(out + 4, (uint32_t)total);
	put_be32(out + 8, (uint32_t)off_struct);
	put_be32(out + 12, (uint32_t)off_strings);
	put_be32(out + 16, 40);
	put_be32(out + 20, 17);
	put_be32(out + 24, 16);
	put_be32(out + 28, 0);
	put_be32(out + 32, (uint32_t)sizeof(strings));
	put_be32(out + 36, (uint32_t)st.len);
	memcpy(out + off_struct, st.data, st.len);
	memcpy(out + off_strings, strings, sizeof(strings));
	return total;
}

static int cmd_check_en751221_bootext(int argc, char **argv)
{
	const char *image = NULL, *ddr = NULL, *chain = NULL;
	const char *bs = NULL, *ds = NULL, *cs = NULL;
	int i;

	for (i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "--image"))
			image = next_arg(&i, argc, argv, "--image");
		else if (!strcmp(argv[i], "--dramc"))
			ddr = next_arg(&i, argc, argv, "--dramc");
		else if (!strcmp(argv[i], "--chainloader"))
			chain = next_arg(&i, argc, argv, "--chainloader");
		else if (!strcmp(argv[i], "--bootstrap-symbols"))
			bs = next_arg(&i, argc, argv, "--bootstrap-symbols");
		else if (!strcmp(argv[i], "--dramc-symbols"))
			ds = next_arg(&i, argc, argv, "--dramc-symbols");
		else if (!strcmp(argv[i], "--chainloader-symbols"))
			cs = next_arg(&i, argc, argv, "--chainloader-symbols");
		else {
			usage(stderr);
			return 2;
		}
	}
	if (!image || !ddr || !chain || !bs || !ds || !cs) {
		usage(stderr);
		return 2;
	}
	return check_en751221_bootext(image, ddr, chain, bs, ds, cs);
}

static void fixture_chain_crc(uint8_t *image, size_t checked)
{
	size_t off;

	for (off = 0; off < checked; off += XMODEM_BLOCK) {
		size_t len = checked - off;

		if (len > XMODEM_BLOCK)
			len = XMODEM_BLOCK;
		put_be32(image + checked + (off / XMODEM_BLOCK) * 4,
			 crc32_ieee(image + off, len));
	}
}

static int selftest_crc(void)
{
	static const char s[] = "123456789";
	return crc32_ieee((const uint8_t *)s, 9) == 0xcbf43926u ? 0 : -1;
}

static int selftest(void)
{
	struct flash_fixture f;
	int passed = 0;
	uint32_t move_s, move_e, boot2_s, boot2_e, loader_s, loader_e, ddr_s, ddr_e;

#define TEST(cond, name) do { \
	if (!(cond)) { fprintf(stderr, "not ok - %s\n", name); return 1; } \
	printf("ok %d - %s\n", ++passed, name); \
} while (0)

	TEST(selftest_crc() == 0, "crc32 implementation");

	fixture_init(&f);
	TEST(stage_interval(f.syms, ARRAY_SIZE(f.syms), "move_data", sizeof(f.stages), &move_s, &move_e) == 0 &&
		 stage_interval(f.syms, ARRAY_SIZE(f.syms), "boot2", sizeof(f.stages), &boot2_s, &boot2_e) == 0 &&
		 stage_interval(f.syms, ARRAY_SIZE(f.syms), "lzma", sizeof(f.stages), &loader_s, &loader_e) == 0 &&
		 stage_interval(f.syms, ARRAY_SIZE(f.syms), "spram", sizeof(f.stages), &ddr_s, &ddr_e) == 0 &&
		 move_e < 0x800 && move_e <= boot2_s && boot2_e <= loader_s && loader_e <= ddr_s,
		 "stage interval/layout validation");

	{
		struct blob u = { 0 };
		uint8_t buf[64 + sizeof(f.data)];
		memcpy(buf, f.header, 64);
		memcpy(buf + 64, f.data, f.data_len);
		u.data = buf;
		u.len = 64 + f.data_len;
		TEST(validate_uboot(&u, 0x81000000u, true) == 0, "valid U-Boot legacy image");
	}

	{
		struct blob u = { 0 };
		uint8_t buf[64 + sizeof(f.data)];
		memcpy(buf, f.header, 64);
		memcpy(buf + 64, f.data, f.data_len);
		buf[64] ^= 1;
		u.data = buf;
		u.len = 64 + f.data_len;
		TEST(validate_uboot(&u, 0x81000000u, true) < 0, "bad payload CRC rejected");
	}

	{
		struct blob u = { 0 };
		uint8_t buf[64 + sizeof(f.data)];
		memcpy(buf, f.header, 64);
		memcpy(buf + 64, f.data, f.data_len);
		buf[4] ^= 1;
		u.data = buf;
		u.len = 64 + f.data_len;
		TEST(validate_uboot(&u, 0x81000000u, true) < 0, "bad header CRC rejected");
	}

	{
		struct blob u = { 0 };
		uint8_t buf[64 + sizeof(f.data)];
		memcpy(buf, f.header, 64);
		memcpy(buf + 64, f.data, f.data_len);
		buf[31] = 3;
		memset(buf + 4, 0, 4);
		put_be32(buf + 4, crc32_ieee(buf, 64));
		u.data = buf;
		u.len = 64 + f.data_len;
		TEST(validate_uboot(&u, 0x81000000u, true) < 0, "compressed payload rejected");
	}

	TEST(sizeof(f.stages) < FLASH_MINFO && FLASH_PAYLOAD + 64 + f.data_len < FLASH_LIMIT,
		 "flash image size limits");

	{
		struct blob raw = { f.data, f.data_len }, prepared = { 0 };
		const char *format = NULL;
		TEST(prepare_uboot(&raw, 0x81000000u, &prepared, &format) == 0 &&
			 !strcmp(format, "raw->ecnt") && validate_ecnt(&prepared, true) == 0,
			 "raw U-Boot wrapped as ECNT");
		free_blob(&prepared);
	}

	{
		struct blob v2 = { calloc(1, ECONET_BOOT_HEADER_V2 + f.data_len),
				   ECONET_BOOT_HEADER_V2 + f.data_len };
		uint8_t saved[ECONET_BOOT_HEADER_V2];
		unsigned int j;
		static const struct { unsigned int offset; uint32_t value; } bad[] = {
			{ 4, 3 }, { 8, 32 }, { 8, UINT32_MAX }, { 12, 0 },
			{ 16, 0x82000000u }, { 20, 0x81010000u }, { 32, 3 },
			{ 36, 0 }, { 36, 0x1000001u }, { 40, 0 }, { 44, 1 }
		};

		if (!v2.data)
			die("out of memory");
		put_be32(v2.data, ECONET_BOOT_MAGIC);
		put_be32(v2.data + 4, ECONET_BOOT_VERSION_V2);
		put_be32(v2.data + 8, ECONET_BOOT_HEADER_V2);
		put_be32(v2.data + 12, (uint32_t)f.data_len);
		put_be32(v2.data + 16, 0x81000000u);
		put_be32(v2.data + 20, 0x81000000u);
		put_be32(v2.data + 24, crc32_ieee(f.data, f.data_len));
		put_be32(v2.data + 36, (uint32_t)f.data_len);
		put_be32(v2.data + 40, crc32_ieee(f.data, f.data_len));
		put_be32(v2.data + 28, crc32_ieee(v2.data, ECONET_BOOT_HEADER_V2));
		memcpy(v2.data + ECONET_BOOT_HEADER_V2, f.data, f.data_len);
		TEST(validate_ecnt(&v2, true) == 0, "ECNT v2 uncompressed metadata accepted");
		memcpy(saved, v2.data, sizeof(saved));
		for (j = 0; j < ARRAY_SIZE(bad); j++) {
			memcpy(v2.data, saved, sizeof(saved));
			put_be32(v2.data + bad[j].offset, bad[j].value);
			put_be32(v2.data + 28, 0);
			put_be32(v2.data + 28, crc32_ieee(v2.data, ECONET_BOOT_HEADER_V2));
			TEST(validate_ecnt(&v2, true) < 0, "ECNT v2 malformed metadata rejected");
		}
		memcpy(v2.data, saved, sizeof(saved));
		v2.data[40] ^= 1;
		TEST(validate_ecnt(&v2, true) < 0, "ECNT v2 extended header CRC checked");
		memcpy(v2.data, saved, sizeof(saved));
		v2.len = ECONET_BOOT_HEADER_V2 - 1;
		TEST(validate_ecnt(&v2, true) < 0, "ECNT v2 truncated header rejected");
		free_blob(&v2);
	}

	{
		uint8_t fit_buf[4096];
		struct blob fit, prepared = { 0 };
		const char *format = NULL;
		fit.data = fit_buf;
		fit.len = make_test_fit(fit_buf, sizeof(fit_buf), f.data, f.data_len, false);
		TEST(prepare_uboot(&fit, 0x81000000u, &prepared, &format) == 0 &&
			 !strcmp(format, "fit->ecnt") && validate_ecnt(&prepared, true) == 0 &&
			 get_be32(prepared.data + 12) == f.data_len &&
			 !memcmp(prepared.data + ECONET_BOOT_HEADER_SIZE, f.data, f.data_len),
			 "FIT firmware wrapped as ECNT for flash loader");
		free_blob(&prepared);
	}

	{
		uint8_t fit_buf[4096];
		struct blob fit, prepared = { 0 };
		const char *format = NULL;
		fit.data = fit_buf;
		fit.len = make_test_fit(fit_buf, sizeof(fit_buf), f.data, f.data_len, true);
		TEST(prepare_uboot(&fit, 0x81000000u, &prepared, &format) == 0 &&
			 !strcmp(format, "fit->ecnt") && validate_ecnt(&prepared, true) == 0 &&
			 get_be32(prepared.data + 12) == f.data_len &&
			 !memcmp(prepared.data + ECONET_BOOT_HEADER_SIZE, f.data, f.data_len),
			 "U-Boot kernel-style FIT wrapped as ECNT for flash loader");
		free_blob(&prepared);
	}

	{
		struct blob legacy = { 0 }, prepared = { 0 };
		uint8_t buf[64 + sizeof(f.data)];
		const char *format = NULL;
		memcpy(buf, f.header, 64);
		memcpy(buf + 64, f.data, f.data_len);
		legacy.data = buf;
		legacy.len = 64 + f.data_len;
		TEST(prepare_uboot(&legacy, 0x81000000u, &prepared, &format) == 0 &&
			 !strcmp(format, "legacy->ecnt") && validate_ecnt(&prepared, true) == 0 &&
			 get_be32(prepared.data + 12) == f.data_len &&
			 !memcmp(prepared.data + ECONET_BOOT_HEADER_SIZE, f.data, f.data_len),
			 "legacy U-Boot payload wrapped as ECNT");
		free_blob(&prepared);
	}

	{
		uint8_t inner[1536] = { 0 }, ddr_data[1024] = { 0 };
		uint8_t outer[3968] = { 0 };
		struct blob chain = { inner, sizeof(inner) }, ddr = { ddr_data, sizeof(ddr_data) };
		struct blob boot = { outer, sizeof(outer) };
		const size_t checked = 128 + sizeof(inner) + sizeof(ddr_data);
		const uint32_t base = 0x80009000u, ss = base + 128;
		const uint32_t se = ss + sizeof(inner), de = se + sizeof(ddr_data);
		size_t i;

		for (i = 0; i < 129; i++)
			inner[i] = (uint8_t)(i * 17 + 3);
		fixture_chain_crc(inner, 129);
		memcpy(outer + 128, inner, sizeof(inner));
		memcpy(outer + 128 + sizeof(inner), ddr_data, sizeof(ddr_data));
		fixture_chain_crc(outer, checked);
		TEST(chainloader_crc_valid(&chain, 129) && chainloader_crc_valid(&boot, checked),
		     "bootext inner/outer CRC tables, including partial block");
		TEST(embedded_blob_matches(&boot, checked, base, ss, se, &chain) &&
		     embedded_blob_matches(&boot, checked, base, se, de, &ddr),
		     "bootext embedded payloads exact and covered");
		outer[0] ^= 1;
		TEST(!chainloader_crc_valid(&boot, checked), "bootext wrapper corruption rejected");
		outer[0] ^= 1;
		outer[128] ^= 1;
		TEST(!chainloader_crc_valid(&boot, checked) &&
		     !embedded_blob_matches(&boot, checked, base, ss, se, &chain),
		     "bootext receiver corruption rejected");
		outer[128] ^= 1;
		outer[se - base] ^= 1;
		TEST(!chainloader_crc_valid(&boot, checked) &&
		     !embedded_blob_matches(&boot, checked, base, se, de, &ddr),
		     "bootext DDR corruption rejected");
		outer[se - base] ^= 1;
		outer[checked - 1] ^= 1;
		TEST(!chainloader_crc_valid(&boot, checked), "bootext last body byte covered");
		outer[checked - 1] ^= 1;
		outer[checked] ^= 1;
		TEST(!chainloader_crc_valid(&boot, checked), "bootext CRC table corruption rejected");
		outer[checked] ^= 1;
		boot.len -= 128;
		TEST(!chainloader_crc_valid(&boot, checked), "bootext truncated table rejected");
		boot.len += 127;
		TEST(!chainloader_crc_valid(&boot, checked), "bootext truncated padding rejected");
		boot.len++;
		TEST(!chainloader_crc_valid(&boot, 0) &&
		     !chainloader_crc_valid(&boot, CRC_TABLE_ENTRIES * XMODEM_BLOCK + 1) &&
		     !chainloader_crc_valid(&boot, SIZE_MAX), "bootext invalid CRC extents rejected");
		TEST(!embedded_blob_matches(&boot, checked, base, base - 4, se, &chain) &&
		     !embedded_blob_matches(&boot, checked, base, se, ss, &chain) &&
		     !embedded_blob_matches(&boot, checked, base, ss + 1, se + 1, &chain),
		     "bootext malformed payload addresses rejected");
		TEST(!embedded_blob_matches(&boot, checked - 1, base, se, de, &ddr) &&
		     !embedded_blob_matches(&boot, checked, base, se, de - 4, &ddr),
		     "bootext uncovered or wrong-size payload rejected");
		TEST(en751221_layout_valid(0x9fa32a80u, 0x9fa38000u, 0x9fa3c000u, 0x5800, 0x4000),
		     "bootext exact SRAM boundaries accepted");
		TEST(!en751221_layout_valid(0x9fa32a81u, 0x9fa38000u, 0x9fa3c000u, 0x5800, 0x4000),
		     "bootext unexpected DDR entry rejected");
		TEST(!en751221_layout_valid(0x9fa32a80u, 0x9fa38001u, 0x9fa3c000u, 0x5800, 0x4000),
		     "bootext DDR runtime overlap rejected");
		TEST(!en751221_layout_valid(0x9fa32a80u, 0x9fa38000u, 0x9fa3c001u, 0x5800, 0x4000),
		     "bootext receiver runtime overflow rejected");
		TEST(!en751221_layout_valid(0x9fa32a80u, 0x9fa38000u, 0x9fa3c000u, 0x5801, 0x4000) &&
		     !en751221_layout_valid(0x9fa32a80u, 0x9fa38000u, 0x9fa3c000u, 0x5800, 0x4001),
		     "bootext oversized payloads rejected");
		TEST(!en751221_layout_valid(0x9fa32a80u, 0x9fa32a80u, 0x9fa38000u, 0, 0) &&
		     !en751221_layout_valid(0x9fa32a80u, 0x9fa38000u, 0x9fa3c000u, 0x280, 0x4000),
		     "bootext empty runtime or missing DDR entry bytes rejected");
	}

	printf("1..%d\n", passed);
#undef TEST
	return 0;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		usage(stderr);
		return 2;
	}
	if (!strcmp(argv[1], "chainloader"))
		return cmd_chainloader(argc - 2, argv + 2);
	if (!strcmp(argv[1], "flash"))
		return cmd_flash(argc - 2, argv + 2);
	if (!strcmp(argv[1], "bootext"))
		return cmd_bootext(argc - 2, argv + 2);
	if (!strcmp(argv[1], "tcboot-base"))
		return cmd_tcboot_base(argc - 2, argv + 2);
	if (!strcmp(argv[1], "tcboot"))
		return cmd_tcboot(argc - 2, argv + 2);
	if (!strcmp(argv[1], "tcboot-finalize"))
		return cmd_tcboot_finalize(argc - 2, argv + 2);
	if (!strcmp(argv[1], "selftest"))
		return selftest();
	if (!strcmp(argv[1], "check-en751221-bootext"))
		return cmd_check_en751221_bootext(argc - 2, argv + 2);

	usage(stderr);
	return 2;
}
