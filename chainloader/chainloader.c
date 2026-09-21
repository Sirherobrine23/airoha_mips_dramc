/* SPDX-License-Identifier: GPL-2.0+ */
#include "chainloader.h"


static void watchdog_kick(void)
{
	u32 val = mmio_read32(CR_TIMER_CTL);

	val &= 0xffc0ffffu;
	val |= 0x00200000u;
	mmio_write32(CR_TIMER_CTL, val);
	__asm__ volatile("sync" ::: "memory");
}

/* Mutable state lives in .bss: .data is covered by the image self-check. */
/*
 * Delay between seeing DR and reading the RBR. Replicates exactly the cost
 * of watchdog_kick() (two MMIO transactions plus sync, ~5 us), which is
 * what the v10 probe had between the two reads.
 */
static void uart_rx_settle(void)
{
	u32 val = mmio_read32(CR_TIMER_CTL);

	val &= 0xffc0ffffu;
	val |= 0x00200000u;
	mmio_write32(CR_TIMER_CTL, val);
	__asm__ volatile("sync" ::: "memory");
}

/*
 * Mutable state must stay in .bss: .data is covered by the self-check.
 */
static u32 ticks_per_ms;

/*
 * Calibrate the CP0 Count against the UART shift rate: the banner has
 * already been transmitted, so the elapsed time divided by the number of
 * characters gives the period of one character, which at 115200 8N1 is
 * 1/11520 s.
 */
static void calibrate(u32 t0, u32 chars)
{
	u32 dt, per_char, per_sec;

	dt = cp0_count() - t0;

	if (!chars)
		return;
	per_char = dt / chars;
	if (per_char < 8u || per_char > 400000u)
		return;			/* implausible: stay on fallback */

	per_sec = per_char * CONSOLE_CPS;
	if (per_sec < 1000000u || per_sec > 2000000000u)
		return;

	ticks_per_ms = per_sec / 1000u;
}

static u32 rx_lsr_err;
static u32 rx_wait_n;
static int have_pushback;
static u8 pushback_byte;

static int uart_getc_to(u8 *out, u32 ms)
{
	u32 t0 = cp0_count();
	u32 limit = ms * ticks_per_ms;
	u32 last = t0;
	int waited = 0;

	if (have_pushback) {
		have_pushback = 0;
		*out = pushback_byte;
		return 1;
	}

	for (;;) {
		u32 lsr = mmio_read32(AIROHA_UART_BASE + AIROHA_UART_LSR);
		u32 now;

		rx_lsr_err |= lsr & AIROHA_UART_LSR_ERR;
		if (lsr & AIROHA_UART_LSR_DR) {
			u32 raw;

			if (waited)
				rx_wait_n++;
			/*
			 * DR going high doesn't mean the RBR is already valid on
			 * this chip. The v10 probe read correctly because it had
			 * a watchdog_kick() between the LSR and RBR reads; without
			 * it, an immediate read returns bus garbage. Same sequence
			 * here.
			 */
			uart_rx_settle();
			raw = mmio_read32(AIROHA_UART_BASE + AIROHA_UART_RBR);
			*out = (u8)raw;
			return 1;
		}

		now = cp0_count();
		if ((now - t0) >= limit) {
			watchdog_kick();
			return 0;
		}
		waited = 1;
		/* Don't kick the watchdog every loop: this is the RX loop. */
		if ((now - last) >= ticks_per_ms) {
			watchdog_kick();
			last = now;
		}
	}
}

static void uart_pushback(u8 c)
{
	pushback_byte = c;
	have_pushback = 1;
}

/* Discard everything until the line is idle for the given time. */
static void uart_purge(u32 ms)
{
	u8 c;

	have_pushback = 0;
	while (uart_getc_to(&c, ms))
		;
}

static u16 crc16_xmodem(const u8 *buf, u32 len)
{
	u16 crc = 0;
	u32 i;
	int bit;

	for (i = 0; i < len; i++) {
		crc ^= (u16)buf[i] << 8;
		for (bit = 0; bit < 8; bit++) {
			if (crc & 0x8000)
				crc = (u16)((crc << 1) ^ 0x1021);
			else
				crc = (u16)(crc << 1);
		}
		if ((i & 0x7fu) == 0)
			watchdog_kick();
	}

	return crc;
}

static u8 csum8(const u8 *buf, u32 len)
{
	u8 sum = 0;
	u32 i;

	for (i = 0; i < len; i++)
		sum = (u8)(sum + buf[i]);

	return sum;
}

static u32 crc32_ieee(const volatile u8 *buf, u32 len)
{
	u32 crc = 0xffffffffu;
	u32 i;
	int bit;

	for (i = 0; i < len; i++) {
		crc ^= buf[i];
		for (bit = 0; bit < 8; bit++)
			crc = (crc >> 1) ^ ((0u - (crc & 1u)) & 0xedb88320u);
		if ((i & 0x3fffu) == 0)
			watchdog_kick();
	}

	return ~crc;
}


static u32 get_be32(const u8 *p)
{
	return ((u32)p[0] << 24) | ((u32)p[1] << 16) |
	       ((u32)p[2] << 8) | (u32)p[3];
}

static u32 get_be32_volatile(const volatile u8 *p)
{
	return ((u32)p[0] << 24) | ((u32)p[1] << 16) |
	       ((u32)p[2] << 8) | (u32)p[3];
}

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

struct boot_image {
	const u8 *data;
	u32 size;
	u32 load;
	u32 entry;
	enum image_type type;
};

struct fdt_view {
	const u8 *base;
	u32 input_len;
	u32 totalsize;
	const u8 *structure;
	const u8 *structure_end;
	const u8 *strings;
	const u8 *strings_end;
};

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

static int fdt_init(struct fdt_view *v, const u8 *fit, u32 len)
{
	u32 off_struct, off_strings, size_struct, size_strings;

	if (len < 40 || get_be32(fit) != FDT_MAGIC)
		return 0;
	v->totalsize = get_be32(fit + 4);
	off_struct = get_be32(fit + 8);
	off_strings = get_be32(fit + 12);
	size_strings = get_be32(fit + 32);
	size_struct = get_be32(fit + 36);
	if (v->totalsize < 40 || v->totalsize > len ||
	    !range_ok(off_struct, size_struct, v->totalsize) ||
	    !range_ok(off_strings, size_strings, v->totalsize))
		return 0;
	v->base = fit;
	v->input_len = len;
	v->structure = fit + off_struct;
	v->structure_end = v->structure + size_struct;
	v->strings = fit + off_strings;
	v->strings_end = v->strings + size_strings;
	return 1;
}

static int fdt_name_eq(const char *node, const char *want)
{
	return node && want && string_eq(node, want);
}

/* Find a property in /level1 or /level1/level2. */
static int fdt_find_prop(const struct fdt_view *v, const char *level1,
			 const char *level2, const char *prop,
			 const u8 **value, u32 *value_len)
{
	const u8 *p = v->structure;
	const char *nodes[4] = { 0 };
	int depth = -1;

	while (p + 4 <= v->structure_end) {
		u32 token = get_be32(p);
		p += 4;

		if (token == FDT_BEGIN_NODE) {
			u32 nlen, skip;
			if (!fdt_cstr_len(p, v->structure_end, &nlen))
				return 0;
			depth++;
			if (depth < (int)(sizeof(nodes) / sizeof(nodes[0])))
				nodes[depth] = (const char *)p;
			skip = (nlen + 1u + 3u) & ~3u;
			if ((u32)(v->structure_end - p) < skip)
				return 0;
			p += skip;
		} else if (token == FDT_END_NODE) {
			if (depth < 0)
				return 0;
			if (depth < (int)(sizeof(nodes) / sizeof(nodes[0])))
				nodes[depth] = 0;
			depth--;
		} else if (token == FDT_PROP) {
			u32 len, nameoff, skip, nlen;
			const char *name;

			if (p + 8 > v->structure_end)
				return 0;
			len = get_be32(p);
			nameoff = get_be32(p + 4);
			p += 8;
			skip = (len + 3u) & ~3u;
			if ((u32)(v->structure_end - p) < skip ||
			    nameoff >= (u32)(v->strings_end - v->strings))
				return 0;
			name = (const char *)(v->strings + nameoff);
			if (!fdt_cstr_len((const u8 *)name, v->strings_end, &nlen))
				return 0;

			if (string_eq(name, prop) && depth >= 1 &&
			    fdt_name_eq(nodes[1], level1) &&
			    ((!level2 && depth == 1) ||
			     (level2 && depth == 2 && fdt_name_eq(nodes[2], level2)))) {
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

static int fdt_first_child(const struct fdt_view *v, const char *parent,
			   char *name, u32 name_len)
{
	const u8 *p = v->structure;
	const char *nodes[4] = { 0 };
	int depth = -1;

	while (p + 4 <= v->structure_end) {
		u32 token = get_be32(p);
		p += 4;
		if (token == FDT_BEGIN_NODE) {
			u32 nlen, skip, i;
			if (!fdt_cstr_len(p, v->structure_end, &nlen))
				return 0;
			depth++;
			if (depth < (int)(sizeof(nodes) / sizeof(nodes[0])))
				nodes[depth] = (const char *)p;
			if (depth == 2 && fdt_name_eq(nodes[1], parent)) {
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
			if (depth < (int)(sizeof(nodes) / sizeof(nodes[0])))
				nodes[depth] = 0;
			depth--;
		} else if (token == FDT_PROP) {
			u32 len, skip;
			if (p + 8 > v->structure_end)
				return 0;
			len = get_be32(p);
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

static int fdt_read_cell32(const u8 *p, u32 len, u32 *value)
{
	if (len == 4) {
		*value = get_be32(p);
		return 1;
	}
	if (len == 8 && get_be32(p) == 0) {
		*value = get_be32(p + 4);
		return 1;
	}
	return 0;
}

static int validate_destination(const struct boot_image *img)
{
	if (!img->size || img->size > UBOOT_MAX_SIZE)
		return 0;
	if (img->load < UBOOT_LOAD_ADDR || img->load >= UBOOT_DRAM_LIMIT)
		return 0;
	if (img->size > UBOOT_DRAM_LIMIT - img->load)
		return 0;
	if (img->entry < img->load || img->entry >= img->load + img->size)
		return 0;
	return 1;
}

static int parse_legacy(const u8 *buf, u32 len, struct boot_image *img)
{
	u8 hdr[IH_HEADER_SIZE];
	u32 size, load, entry, hcrc, dcrc, i;

	if (len < IH_HEADER_SIZE || get_be32(buf) != IH_MAGIC)
		return 0;
	for (i = 0; i < IH_HEADER_SIZE; i++)
		hdr[i] = buf[i];
	hcrc = get_be32(hdr + 4);
	hdr[4] = hdr[5] = hdr[6] = hdr[7] = 0;
	if (crc32_ieee(hdr, IH_HEADER_SIZE) != hcrc)
		return -1;
	size = get_be32(buf + 12);
	load = get_be32(buf + 16);
	entry = get_be32(buf + 20);
	dcrc = get_be32(buf + 24);
	if (!size || size > len - IH_HEADER_SIZE ||
	    buf[29] != IH_ARCH_MIPS || buf[30] != IH_TYPE_FIRMWARE ||
	    buf[31] != IH_COMP_NONE)
		return -1;
	if (crc32_ieee(buf + IH_HEADER_SIZE, size) != dcrc)
		return -1;
	img->data = buf + IH_HEADER_SIZE;
	img->size = size;
	img->load = load;
	img->entry = entry;
	img->type = TYPE_LEGACY;
	return validate_destination(img) ? 1 : -1;
}

static int parse_ecnt(const u8 *buf, u32 len, struct boot_image *img)
{
	u8 hdr[ECONET_BOOT_HEADER_SIZE];
	u32 version, off, size, load, entry, dcrc, hcrc, i;

	if (len < ECONET_BOOT_HEADER_SIZE || get_be32(buf) != ECONET_BOOT_MAGIC)
		return 0;
	for (i = 0; i < ECONET_BOOT_HEADER_SIZE; i++)
		hdr[i] = buf[i];
	hcrc = get_be32(hdr + 28);
	hdr[28] = hdr[29] = hdr[30] = hdr[31] = 0;
	if (crc32_ieee(hdr, sizeof(hdr)) != hcrc)
		return -1;
	version = get_be32(buf + 4);
	off = get_be32(buf + 8);
	size = get_be32(buf + 12);
	load = get_be32(buf + 16);
	entry = get_be32(buf + 20);
	dcrc = get_be32(buf + 24);
	if (version != ECONET_BOOT_VERSION || off < ECONET_BOOT_HEADER_SIZE ||
	    !range_ok(off, size, len))
		return -1;
	if (crc32_ieee(buf + off, size) != dcrc)
		return -1;
	img->data = buf + off;
	img->size = size;
	img->load = load;
	img->entry = entry;
	img->type = TYPE_ECNT;
	return validate_destination(img) ? 1 : -1;
}

static int parse_fit(const u8 *buf, u32 len, struct boot_image *img)
{
	struct fdt_view v;
	const u8 *p, *data = 0;
	u32 n, data_len = 0, load = UBOOT_LOAD_ADDR, entry = UBOOT_LOAD_ADDR;
	u32 data_pos = 0, data_size = 0;
	char config[64], firmware[64];
	int kernel_ref = 0;

	if (len < 4 || get_be32(buf) != FDT_MAGIC)
		return 0;
	if (!fdt_init(&v, buf, len))
		return -1;

	if (fdt_find_prop(&v, "configurations", 0, "default", &p, &n)) {
		if (!copy_prop_string(config, sizeof(config), p, n))
			return -1;
	} else if (!fdt_first_child(&v, "configurations", config, sizeof(config))) {
		return -1;
	}

	if (fdt_find_prop(&v, "configurations", config, "firmware", &p, &n) ||
	    fdt_find_prop(&v, "configurations", config, "loadables", &p, &n)) {
		if (!copy_prop_string(firmware, sizeof(firmware), p, n))
			return -1;
	} else if (fdt_find_prop(&v, "configurations", config, "kernel", &p, &n)) {
		/* U-Boot-generated u-boot.itb uses config->kernel for u-boot.bin. */
		if (!copy_prop_string(firmware, sizeof(firmware), p, n))
			return -1;
		kernel_ref = 1;
	} else {
		return -1;
	}

	if (fdt_find_prop(&v, "images", firmware, "compression", &p, &n) &&
	    !prop_string_eq(p, n, "none"))
		return -1;
	if (!kernel_ref && fdt_find_prop(&v, "images", firmware, "arch", &p, &n) &&
	    !prop_string_eq(p, n, "mips"))
		return -1;
	if (fdt_find_prop(&v, "images", firmware, "type", &p, &n) &&
	    !prop_string_eq(p, n, "firmware") &&
	    !prop_string_eq(p, n, "standalone") &&
	    !(kernel_ref && prop_string_eq(p, n, "kernel")))
		return -1;

	if (!kernel_ref) {
		if (fdt_find_prop(&v, "images", firmware, "load", &p, &n) &&
		    !fdt_read_cell32(p, n, &load))
			return -1;
		entry = load;
		if (fdt_find_prop(&v, "images", firmware, "entry", &p, &n) &&
		    !fdt_read_cell32(p, n, &entry))
			return -1;
	}

	if (fdt_find_prop(&v, "images", firmware, "data", &p, &n)) {
		data = p;
		data_len = n;
	} else {
		if (!fdt_find_prop(&v, "images", firmware, "data-size", &p, &n) ||
		    !fdt_read_cell32(p, n, &data_size))
			return -1;
		if (fdt_find_prop(&v, "images", firmware, "data-position", &p, &n)) {
			if (!fdt_read_cell32(p, n, &data_pos))
				return -1;
		} else if (fdt_find_prop(&v, "images", firmware, "data-offset", &p, &n)) {
			u32 rel;
			if (!fdt_read_cell32(p, n, &rel) || rel > len - v.totalsize)
				return -1;
			data_pos = v.totalsize + rel;
		} else {
			return -1;
		}
		if (!range_ok(data_pos, data_size, len))
			return -1;
		data = buf + data_pos;
		data_len = data_size;
	}

	img->data = data;
	img->size = data_len;
	img->load = load;
	img->entry = entry;
	img->type = TYPE_FIT;
	return validate_destination(img) ? 1 : -1;
}

static int parse_boot_image(const u8 *buf, u32 len, struct boot_image *img)
{
	int ret;
	u32 magic = len >= 4 ? get_be32(buf) : 0;

	ret = parse_legacy(buf, len, img);
	if (ret)
		return ret;
	ret = parse_fit(buf, len, img);
	if (ret)
		return ret;
	ret = parse_ecnt(buf, len, img);
	if (ret)
		return ret;
	if (magic == 0x7f454c46u) /* ELF: use u-boot.bin, not the linked ELF. */
		return -1;

	img->data = buf;
	img->size = len;
	img->load = UBOOT_LOAD_ADDR;
	img->entry = UBOOT_ENTRY_CACHED;
	img->type = TYPE_RAW;
	return validate_destination(img) ? 1 : -1;
}

static const char *image_type_name(enum image_type type)
{
	switch (type) {
	case TYPE_LEGACY:
		return "legacy";
	case TYPE_FIT:
		return "fit";
	case TYPE_ECNT:
		return "ecnt/raw";
	default:
		return "raw";
	}
}

static void move_payload(u8 *dst, const u8 *src, u32 len)
{
	u32 i;

	if (dst == src || !len)
		return;
	if ((uintptr_t)dst < (uintptr_t)src) {
		for (i = 0; i < len; i++)
			dst[i] = src[i];
	} else {
		for (i = len; i; i--)
			dst[i - 1] = src[i - 1];
	}
	__asm__ volatile("sync" ::: "memory");
}

/* Deferred diagnostics: nothing can be printed while the link is live. */
static u32 err_count;
static u32 err_first_kind;	/* 1 hdr, 2 timeout, 3 trailer, 4 sequence */
static u32 err_first_blk;
static u32 err_first_exp;
static u32 err_first_len;
static u32 err_first_got;
static u32 err_first_want;
static u8 err_first_data[16];
static u32 stat_blocks;
static u32 stat_dups;
static u32 stat_tries;
static int stat_1k;
static int stat_csum;

static void note_error(u32 kind, u32 blk, u32 exp, u32 len, u32 got, u32 want)
{
	if (!err_count) {
		err_first_kind = kind;
		err_first_blk = blk;
		err_first_exp = exp;
		err_first_len = len;
		err_first_got = got;
		err_first_want = want;
	}
	err_count++;
}

static void xmodem_reset_state(void)
{
	err_count = 0;
	err_first_kind = 0;
	err_first_blk = 0;
	err_first_exp = 0;
	err_first_len = 0;
	err_first_got = 0;
	err_first_want = 0;
	stat_blocks = 0;
	stat_dups = 0;
	stat_tries = 0;
	stat_1k = 0;
	stat_csum = 0;
	rx_lsr_err = 0;
	rx_wait_n = 0;
	have_pushback = 0;
}

/*
 * XMODEM receiver: 128-byte (SOH) and 1K (STX) blocks, CRC16/XMODEM with
 * fallback to 8-bit checksum, per-byte timeout, and purge-based resync.
 */
static u32 xmodem_receive(void)
{
	volatile u8 *dst = (volatile u8 *)UBOOT_LOAD_CACHED;
#ifdef EN751221_BOOTEXT
	/*
	 * DDR calibration has returned before this receiver runs. Its stack
	 * is in cached DRAM, so the packet buffer need not consume FE SRAM.
	 */
	u8 packet[1024];
#else
	static u8 packet[1024];
#endif
	u8 expected = 1;
	u32 total = 0;
	u8 ch;
	int cancels = 0;
	int tries;

	uart_purge(100);

	for (tries = 0;; tries++) {
		stat_tries = (u32)tries;
		if (tries >= HANDSHAKE_TRIES)
			return 0;
		/* Ask for CRC first; later also offer checksum mode. */
		uart_putc(tries < 40 ? CRC_REQ : NAK);
		if (!uart_getc_to(&ch, HANDSHAKE_MS))
			continue;
		if (ch == SOH || ch == STX || ch == EOT || ch == CAN)
			break;
	}

	for (;;) {
		u32 plen, i;
		u8 blk, blki, t1, t2;
		u16 got_crc, want_crc;
		int have_t2, ok;

		if (ch == EOT) {
			uart_putc(ACK);
			return total;
		}

		if (ch == CAN) {
			if (++cancels >= 2)
				return 0;
			if (!uart_getc_to(&ch, BYTE_MS))
				goto resync;
			continue;
		}
		cancels = 0;

		if (ch != SOH && ch != STX) {
			note_error(1, ch, expected, 0, 0, 0);
			goto resync;
		}

		plen = (ch == STX) ? 1024u : 128u;
		if (plen == 1024u)
			stat_1k = 1;

		if (!uart_getc_to(&blk, BYTE_MS) ||
		    !uart_getc_to(&blki, BYTE_MS)) {
			note_error(2, 0, expected, plen, 0, 0);
			goto resync;
		}

		for (i = 0; i < plen; i++) {
			if (!uart_getc_to(&packet[i], BYTE_MS)) {
				note_error(2, blk, expected, i, 0, 0);
				goto resync;
			}
			if ((i & 0x1fu) == 0)
				watchdog_kick();
		}

		if (!uart_getc_to(&t1, BYTE_MS)) {
			note_error(2, blk, expected, plen, 0, 0);
			goto resync;
		}

		have_t2 = uart_getc_to(&t2, CSUM_PROBE_MS);
		want_crc = crc16_xmodem(packet, plen);
		got_crc = have_t2 ? (u16)(((u16)t1 << 8) | t2) : 0;
		ok = 0;

		if (have_t2 && got_crc == want_crc) {
			ok = 1;
		} else if (t1 == csum8(packet, plen)) {
			/* Checksum mode: t2 is already the start of the next packet. */
			stat_csum = 1;
			ok = 1;
			if (have_t2)
				uart_pushback(t2);
		}

		if (!ok) {
			if (!err_count)
				for (i = 0; i < sizeof(err_first_data) && i < plen; i++)
					err_first_data[i] = packet[i];
			note_error(3, blk, expected, plen, got_crc, want_crc);
			goto resync;
		}

		if ((u8)(blk + blki) != 0xff) {
			note_error(1, blk, blki, plen, 0, 0);
			goto resync;
		}

		/* Retransmission of a block already received. */
		if (blk == (u8)(expected - 1)) {
			stat_dups++;
			uart_putc(ACK);
			if (!uart_getc_to(&ch, NEXT_HDR_MS))
				goto resync;
			continue;
		}

		if (blk != expected) {
			note_error(4, blk, expected, plen, 0, 0);
			goto resync;
		}

		if (total + plen > UBOOT_MAX_SIZE) {
			uart_putc(CAN);
			uart_putc(CAN);
			return 0;
		}

		for (i = 0; i < plen; i++)
			dst[total + i] = packet[i];
		total += plen;
		expected++;
		stat_blocks++;
		watchdog_kick();

		uart_putc(ACK);
		if (!uart_getc_to(&ch, NEXT_HDR_MS)) {
			note_error(2, 0, expected, 0, 0, 0);
			goto resync;
		}
		continue;

resync:
		if (err_count >= MAX_ERRORS) {
			uart_putc(CAN);
			uart_putc(CAN);
			uart_putc(CAN);
			uart_purge(PURGE_MS);
			return 0;
		}
		uart_purge(PURGE_MS);
		uart_putc(NAK);
		if (!uart_getc_to(&ch, NEXT_HDR_MS)) {
			note_error(2, 0, expected, 0, 0, 0);
			ch = 0;
		}
	}
}

static u32 chunk_crc(const volatile u8 *img, u32 len, u32 i)
{
	u32 off = i * CHK_CHUNK;
	u32 n = len - off;

	if (n > CHK_CHUNK)
		n = CHK_CHUNK;
	return crc32_ieee(img + off, n);
}

/*
 * Checks the loaded image against the per-128-byte-block CRC32 table that
 * tools/econet-image chainloader wrote at its end: locates the block, not just flags failure.
 */
static int self_check(void)
{
	const volatile u8 *img = (const volatile u8 *)&__image_start;
	const volatile u8 *tab = (const volatile u8 *)&__chk_start;
	u32 len = (u32)&__chk_start - (u32)&__image_start;
	u32 nchunks = (len + CHK_CHUNK - 1u) / CHK_CHUNK;
	u32 i, n, off, bad = 0, first_bad = 0;

	uart_printf(" self len=0x%08x blocks=0x%08x",
		    (unsigned int)len, (unsigned int)nchunks);

	for (i = 0; i < nchunks; i++) {
		if (chunk_crc(img, len, i) != get_be32_volatile(tab + i * 4u)) {
			if (!bad)
				first_bad = i;
			bad++;
		}
	}

	uart_printf(" bad=0x%08x", (unsigned int)bad);
	if (!bad) {
		uart_printf(" OK\n");
		return 1;
	}

	uart_printf("\nbad idx:");
	for (i = 0; i < nchunks; i++)
		if (chunk_crc(img, len, i) != get_be32_volatile(tab + i * 4u)) {
			uart_printf(" 0x%08x", (unsigned int)i);
		}

	off = first_bad * CHK_CHUNK;
	n = len - off;
	if (n > CHK_CHUNK)
		n = CHK_CHUNK;

	uart_printf("\ngot=0x%08x want=0x%08x\ndump @0x%08x\n",
		    (unsigned int)chunk_crc(img, len, first_bad),
		    (unsigned int)get_be32_volatile(tab + first_bad * 4u),
		    (unsigned int)((u32)&__image_start + off));
	for (i = 0; i < n; i++) {
		uart_printf("%02x", (unsigned int)img[off + i]);
		if ((i & 0x1fu) == 0x1fu)
			uart_putc('\n');
	}
	uart_printf("corrupted image; refusing to continue\n");
	return 0;
}

/*
 * Writes 16 bytes with 8-bit stores and reads them back as words, once
 * through KSEG0 (cached) and once through KSEG1 (uncached), at the same
 * physical address. Expected in both: 10111213 14151617 18191a1b 1c1d1e1f.
 */
extern void chainload_jump(u32 entry) __attribute__((noreturn));
static void halt(void) __attribute__((noreturn));

static void report(void)
{
	uart_printf("\nblocks=0x%08x dup=0x%08x tries=0x%08x mode=%s%s err=0x%08x lsrerr=0x%08x",
		    (unsigned int)stat_blocks, (unsigned int)stat_dups,
		    (unsigned int)stat_tries, stat_1k ? "1k/" : "128/",
		    stat_csum ? "csum" : "crc16", (unsigned int)err_count,
		    (unsigned int)rx_lsr_err);
	if (err_count) {
		uart_printf("\nfirst: kind=0x%08x blk=0x%08x exp=0x%08x at=0x%08x got=0x%08x want=0x%08x",
			    (unsigned int)err_first_kind, (unsigned int)err_first_blk,
			    (unsigned int)err_first_exp, (unsigned int)err_first_len,
			    (unsigned int)err_first_got, (unsigned int)err_first_want);
		uart_printf("\ndata:");
		{
			u32 i;

			for (i = 0; i < sizeof(err_first_data); i++) {
				uart_printf(" %02x", (unsigned int)err_first_data[i]);
			}
		}
		uart_printf(" waits=0x%08x", (unsigned int)rx_wait_n);
	}
	uart_printf("\n");
}

enum boot_action {
	BOOT_ACTION_CHAINLOAD,
	BOOT_ACTION_FLASH_TCBOOT,
};

static enum boot_action prompt_boot_action(void)
{
	u32 sec = 0;
	u8 ch;

	uart_purge(30);

	uart_printf("Press x to chainload or b to flash tcboot.bin [%us, default: x]\n",
		    (unsigned int)CHAINLOADER_MENU_TIMEOUT_SEC);

	while (sec++ < CHAINLOADER_MENU_TIMEOUT_SEC) {
		if (!uart_getc_to(&ch, 1000u))
			continue;

		switch (ch) {
		case 'X':
		case 'x':
			uart_printf("chainload selected\n");
			return BOOT_ACTION_CHAINLOAD;

		case 'B':
		case 'b':
			uart_printf("flash tcboot.bin selected\n");
			return BOOT_ACTION_FLASH_TCBOOT;

		default:
			break;
		}
	}

	uart_printf("timeout -> chainload\n");
	return BOOT_ACTION_CHAINLOAD;
}

static int receive_and_flash_tcboot(void)
{
	u32 len, crc;
	int ret;

	uart_printf("Send tcboot.bin via XMODEM now (expected 0x00100000 bytes)\n");
	xmodem_reset_state();
	len = xmodem_receive();
	if (!len) {
		report();
		uart_printf("XMODEM failed/cancelled\n");
		return -1;
	}

	__asm__ volatile("sync" ::: "memory");
	report();
	uart_printf("received tcboot.bin size=0x%08x", (unsigned int)len);
	if (len < TCBOOT_FLASH_SIZE_MIN) {
		uart_printf(" min=0x%08x; refusing to flash\n",
			    (unsigned int)TCBOOT_FLASH_SIZE_MIN);
		return -1;
	}
	
	/*
	 * The entire 1 MiB image is now resident in DRAM.  Only after a
	 * complete XMODEM transfer, exact-size check and CRC calculation do we
	 * touch the boot flash.  This intentionally avoids XMODEM->flash
	 * streaming: retransmissions must never partially program tcboot.bin.
	 */
	crc = crc32_ieee((const volatile u8 *)(uintptr_t)UBOOT_LOAD_CACHED, len);
	uart_printf("\ntcboot staged in RAM crc32=0x%08x\n", (unsigned int)crc);

	uart_printf("erasing/writing/verifying tcboot.bin...\n");
	ret = chainloader_flash_tcboot((const void *)(uintptr_t)UBOOT_LOAD_CACHED, len);
	if (ret == CHAINLOADER_FLASH_UNSUPPORTED) {
		uart_printf("flash write unsupported on this media/build; returning to menu\n");
		return ret;
	}
	if (ret) {
		uart_printf("flash failed status=0x%08x; returning to menu\n",
			    (unsigned int)(u32)ret);
		return ret;
	}

	uart_printf("tcboot.bin flashed and verified; reset/power-cycle the board\n");
	return 0;
}

static void halt(void)
{
	for (;;)
		watchdog_kick();
}

void chainloader_main(void)
{
	struct boot_image image;
	u32 len, image_crc, t0;
	volatile u32 *w = (volatile u32 *)UBOOT_LOAD_CACHED;
	u32 i;

	watchdog_kick();

	/* The BootROM may leave the UART IRQ enabled; this runs in polling mode. */
	mmio_write32(AIROHA_UART_BASE + AIROHA_UART_IER, 0);

	ticks_per_ms = DEFAULT_TICKS_PER_MS;
	chainloader_uart_tx_reset();
	t0 = cp0_count();

	uart_putc('\n');
	uart_printf("Airoha MIPS chainloader\n");
	uart_printf("Accepts FIT, u-boot, u-boot.bin and u-boot.img\n");
	uart_printf("XMODEM 128/1k, CRC16 or checksum -> 0x81000000\n");

	calibrate(t0, chainloader_uart_tx_count());

	uart_printf("ticks/ms=0x%08x", (unsigned int)ticks_per_ms);

	if (!self_check()) {
		uart_printf("self-check failed; reset and resend chainloader\n");
		halt();
	}

	for (;;) {
		if (prompt_boot_action() == BOOT_ACTION_FLASH_TCBOOT) {
			if (!receive_and_flash_tcboot())
				halt();
			continue;
		}
		break;
	}

	uart_printf("waiting for U-Boot\n");
	xmodem_reset_state();
	len = xmodem_receive();
	if (!len) {
		report();
		uart_printf("XMODEM failed/cancelled\n");
		halt();
	}

	__asm__ volatile("sync" ::: "memory");

	report();
	uart_printf("received 0x%08x bytes\nfirst words: ", (unsigned int)len);
	for (i = 0; i < 4; i++) {
		uart_printf("%08x", (unsigned int)w[i]);
		if (i != 3)
			uart_putc(' ');
	}
	uart_printf("\n");

	image_crc = crc32_ieee((volatile u8 *)UBOOT_LOAD_CACHED, len);
	uart_printf("xfer crc32=0x%08x\n", (unsigned int)image_crc);

	if (parse_boot_image((const u8 *)(uintptr_t)UBOOT_LOAD_CACHED, len, &image) < 0) {
		uart_printf("invalid/unsupported U-Boot image\n");
		halt();
	}

	uart_printf("image=%s size=0x%08x load=0x%08x entry=0x%08x\n",
		    image_type_name(image.type), (unsigned int)image.size,
		    (unsigned int)image.load, (unsigned int)image.entry);

	move_payload((u8 *)(uintptr_t)image.load, image.data, image.size);
	w = (volatile u32 *)(uintptr_t)image.load;
	if (w[0] == 0x00000000u || w[0] == 0xffffffffu) {
		uart_printf("invalid first word; refusing jump\n");
		halt();
	}

	watchdog_kick();
	uart_printf("jump 0x%08x\n", (unsigned int)image.entry);
	__asm__ volatile("sync" ::: "memory");
	chainload_jump(image.entry);
}
