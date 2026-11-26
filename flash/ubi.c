// SPDX-License-Identifier: GPL-2.0+
/* Minimal read-only UBI scanner and volume loader for post-DRAM boot. */
#include "ubi.h"

struct ubi_ec_parsed {
	uint32_t vid_hdr_offset;
	uint32_t data_offset;
	uint32_t image_seq;
};

struct ubi_vid_parsed {
	uint8_t vol_type;
	uint8_t copy_flag;
	uint32_t vol_id;
	uint32_t lnum;
	uint32_t data_size;
	uint32_t used_ebs;
	uint32_t data_pad;
	uint32_t data_crc;
	uint64_t sqnum;
};

struct ubi_leb_entry {
	uint64_t sqnum;
	uint32_t peb_off;
	uint32_t data_size;
	uint32_t data_crc;
	uint8_t mapped;
	uint8_t check_crc;
};

struct ubi_work {
	struct ubi_leb_entry lebs[UBI_MAX_VOL_LEBS];
};

static uint16_t get_be16(const uint8_t *p)
{
	return (uint16_t)p[0] << 8 | p[1];
}

static uint32_t get_be32(const uint8_t *p)
{
	return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3];
}

static uint64_t get_be64(const uint8_t *p)
{
	return (uint64_t)get_be32(p) << 32 | get_be32(p + 4);
}

static uint32_t crc32_update(uint32_t crc, const uint8_t *p, uint32_t len)
{
	while (len--) {
		crc ^= *p++;
		for (unsigned int bit = 0; bit < 8; bit++)
			crc = (crc >> 1) ^ (0xedb88320U & (0U - (crc & 1U)));
	}
	return crc;
}

uint32_t econet_crc32_raw(const uint8_t *p, uint32_t len)
{
	return crc32_update(~0U, p, len);
}

static int parse_ec_hdr(const uint8_t *buf, struct ubi_ec_parsed *ec)
{
	uint32_t vid_off, data_off;

	if (get_be32(buf) != UBI_EC_HDR_MAGIC || buf[4] != UBI_VERSION)
		return 0;
	if (econet_crc32_raw(buf, UBI_EC_HDR_SIZE - 4U) != get_be32(buf + 60))
		return 0;

	vid_off = get_be32(buf + 16);
	data_off = get_be32(buf + 20);
	if (vid_off < UBI_EC_HDR_SIZE || data_off <= vid_off ||
	    data_off - vid_off < UBI_VID_HDR_SIZE || data_off > UBI_MIN_PEB_SIZE)
		return 0;

	ec->vid_hdr_offset = vid_off;
	ec->data_offset = data_off;
	ec->image_seq = get_be32(buf + 24);
	return 1;
}

static int ec_matches(const struct ubi_ec_parsed *ref, const struct ubi_ec_parsed *cur)
{
	if (cur->vid_hdr_offset != ref->vid_hdr_offset || cur->data_offset != ref->data_offset)
		return 0;
	if (ref->image_seq && cur->image_seq && cur->image_seq != ref->image_seq)
		return 0;
	return 1;
}

static int parse_vid_hdr(const uint8_t *buf, struct ubi_vid_parsed *vid)
{
	if (get_be32(buf) != UBI_VID_HDR_MAGIC || buf[4] != UBI_VERSION)
		return 0;
	if (econet_crc32_raw(buf, UBI_VID_HDR_SIZE - 4U) != get_be32(buf + 60))
		return 0;

	vid->vol_type = buf[5];
	vid->copy_flag = buf[6];
	vid->vol_id = get_be32(buf + 8);
	vid->lnum = get_be32(buf + 12);
	vid->data_size = get_be32(buf + 20);
	vid->used_ebs = get_be32(buf + 24);
	vid->data_pad = get_be32(buf + 28);
	vid->data_crc = get_be32(buf + 32);
	vid->sqnum = get_be64(buf + 40);
	return 1;
}

static int bytes_equal(const uint8_t *a, const uint8_t *b, size_t len)
{
	for (size_t i = 0; i < len; i++) {
		if (a[i] != b[i])
			return 0;
	}
	return 1;
}

static int is_uniform_byte(const uint8_t *buf, size_t len, uint8_t val)
{
	for (size_t i = 0; i < len; i++) {
		if (buf[i] != val)
			return 0;
	}
	return 1;
}

static int name_matches(const uint8_t *rec_name, uint16_t rec_len, const char *target)
{
	size_t i;

	for (i = 0; target[i]; i++) {
		if (i >= rec_len || rec_name[i] != (uint8_t)target[i])
			return 0;
	}
	return i == rec_len;
}

static int detect_geometry(econet_ubi_read_fn read_fn, void *ctx, uint32_t ubi_offset,
			   uint32_t max_scan_size, struct ubi_ec_parsed *ref_ec,
			   uint32_t *peb_size_out)
{
	uint8_t buf[UBI_EC_HDR_SIZE];
	struct ubi_ec_parsed cur;
	uint32_t first_off = 0;
	uint32_t diff_or = 0;
	uint32_t probe_limit;
	unsigned int found = 0;

	/* Probe up to UBI_START_PROBE_SIZE from ubi_offset for the first valid EC header. */
	probe_limit = max_scan_size < UBI_START_PROBE_SIZE ? max_scan_size : UBI_START_PROBE_SIZE;
	for (uint32_t delta = 0; delta < probe_limit; delta += UBI_MIN_PEB_SIZE) {
		if (ubi_offset > UINT32_MAX - delta)
			break;
		if (read_fn(ubi_offset + delta, buf, sizeof(buf), ctx))
			return ECONET_UBI_ERR_IO;
		if (parse_ec_hdr(buf, ref_ec)) {
			first_off = ubi_offset + delta;
			found = 1;
			break;
		}
	}
	if (!found)
		return ECONET_UBI_NO_UBI;

	/*
	 * Probe subsequent 64 KiB-aligned offsets to infer the power-of-2 PEB size.
	 * Taking the lowest set bit of the bitwise OR of all valid header deltas recovers
	 * the exact PEB size even when bad blocks interrupt adjacent PEBs.
	 */
	for (uint32_t step = UBI_MIN_PEB_SIZE; step <= UBI_MAX_PEB_SIZE * 4U;
	     step += UBI_MIN_PEB_SIZE) {
		uint32_t off;

		if (first_off > UINT32_MAX - step)
			break;
		off = first_off + step;
		if (off - ubi_offset >= max_scan_size)
			break;
		if (read_fn(off, buf, sizeof(buf), ctx))
			return ECONET_UBI_ERR_IO;
		if (!parse_ec_hdr(buf, &cur) || !ec_matches(ref_ec, &cur))
			continue;
		if (!ref_ec->image_seq && cur.image_seq)
			ref_ec->image_seq = cur.image_seq;
		diff_or |= step;
		found++;
		/* Stop once we have enough valid PEBs to exclude half-stride aliases. */
		if (found >= 5U && (diff_or & (0U - diff_or)) <= UBI_MAX_PEB_SIZE)
			break;
	}

	if (!diff_or)
		return ECONET_UBI_NO_UBI;

	uint32_t peb_size = diff_or & (0U - diff_or);

	if (peb_size < UBI_MIN_PEB_SIZE || peb_size > UBI_MAX_PEB_SIZE ||
	    ref_ec->data_offset >= peb_size || ((first_off - ubi_offset) & (peb_size - 1U)))
		return ECONET_UBI_ERR_CORRUPT;

	*peb_size_out = peb_size;
	return ECONET_UBI_OK;
}

static int parse_vtbl_copy(econet_ubi_read_fn read_fn, void *ctx, uint32_t peb_off,
			   uint32_t data_offset, uint32_t leb_size, const char *const *vol_names,
			   size_t vol_name_count, uint32_t *out_vol_id, uint32_t *out_reserved_pebs,
			   uint32_t *out_data_pad, uint8_t *out_vol_type, char out_name[32],
			   int *out_found)
{
	uint8_t rec[UBI_VTBL_RECORD_SIZE];
	uint32_t max_vols = leb_size / UBI_VTBL_RECORD_SIZE;
	size_t best_prio = vol_name_count;

	if (max_vols > UBI_MAX_VOLUMES)
		max_vols = UBI_MAX_VOLUMES;
	*out_found = 0;

	for (uint32_t i = 0; i < max_vols; i++) {
		uint32_t reserved_pebs, alignment, data_pad;
		uint16_t name_len;
		uint8_t vol_type, upd_marker;

		if (read_fn(peb_off + data_offset + i * UBI_VTBL_RECORD_SIZE, rec, sizeof(rec),
			    ctx))
			return ECONET_UBI_ERR_IO;
		if (econet_crc32_raw(rec, UBI_VTBL_RECORD_SIZE - 4U) != get_be32(rec + 168))
			return ECONET_UBI_ERR_CORRUPT;

		reserved_pebs = get_be32(rec + 0);
		if (!reserved_pebs)
			continue;

		alignment = get_be32(rec + 4);
		data_pad = get_be32(rec + 8);
		vol_type = rec[12];
		upd_marker = rec[13];
		name_len = get_be16(rec + 14);

		if (name_len == 0 || name_len >= 128U || rec[16 + name_len] != 0)
			return ECONET_UBI_ERR_CORRUPT;
		if (vol_type != UBI_VID_DYNAMIC && vol_type != UBI_VID_STATIC)
			return ECONET_UBI_ERR_CORRUPT;
		if (!alignment || data_pad >= leb_size)
			return ECONET_UBI_ERR_CORRUPT;
		if (upd_marker)
			continue;

		for (size_t p = 0; p < best_prio; p++) {
			if (name_matches(rec + 16, name_len, vol_names[p])) {
				size_t copy_len = name_len < 31U ? name_len : 31U;

				best_prio = p;
				*out_vol_id = i;
				*out_reserved_pebs = reserved_pebs;
				*out_data_pad = data_pad;
				*out_vol_type = vol_type;
				for (size_t c = 0; c < copy_len; c++)
					out_name[c] = (char)rec[16 + c];
				out_name[copy_len] = '\0';
				*out_found = 1;
				break;
			}
		}
	}

	return ECONET_UBI_OK;
}

static int verify_leb_crc(econet_ubi_read_fn read_fn, void *ctx, uint32_t data_off,
			  uint32_t data_size, uint32_t expected_crc)
{
	uint8_t chunk[256];
	uint32_t crc = ~0U;
	uint32_t pos = 0;

	while (pos < data_size) {
		uint32_t n = data_size - pos;

		if (n > sizeof(chunk))
			n = sizeof(chunk);
		if (read_fn(data_off + pos, chunk, n, ctx))
			return ECONET_UBI_ERR_IO;
		crc = crc32_update(crc, chunk, n);
		pos += n;
	}
	return crc == expected_crc ? 1 : 0;
}

int econet_ubi_load_volume(econet_ubi_read_fn read_fn, void *ctx, uint32_t ubi_offset,
			   uint32_t max_scan_size, const char *const *vol_names,
			   size_t vol_name_count, uint8_t *dst, size_t dst_cap, void *work,
			   size_t work_len, struct econet_ubi_info *info)
{
	struct ubi_work *w = work;
	struct ubi_ec_parsed ref_ec, ec;
	struct ubi_vid_parsed vid;
	uint8_t hdr[UBI_EC_HDR_SIZE];
	uint8_t head0[16];
	int have_head0 = 0;
	uint32_t peb_size = 0, leb_size, usable_leb_size;
	uint32_t scanned_pebs = 0, consecutive_invalid = 0;
	uint32_t layout_peb[UBI_LAYOUT_VOLUME_EBS] = { 0, 0 };
	uint64_t layout_sqnum[UBI_LAYOUT_VOLUME_EBS] = { 0, 0 };
	uint8_t layout_found[UBI_LAYOUT_VOLUME_EBS] = { 0, 0 };
	uint32_t target_vol_id = 0, reserved_pebs = 0, data_pad = 0;
	uint8_t target_vol_type = 0;
	char target_name[32];
	int vtbl_ok = 0, vol_found = 0;
	uint32_t static_used_ebs = 0;
	int ret;

	if (!read_fn || !vol_names || !vol_name_count || !dst || !dst_cap || !work ||
	    work_len < sizeof(*w) || ((uintptr_t)work & 7U))
		return ECONET_UBI_ERR_RANGE;

	if (!max_scan_size)
		max_scan_size = UBI_MAX_SCAN_SIZE;

	ret = detect_geometry(read_fn, ctx, ubi_offset, max_scan_size, &ref_ec, &peb_size);
	if (ret != ECONET_UBI_OK)
		return ret;

	leb_size = peb_size - ref_ec.data_offset;

	/*
	 * Snapshot the first 16 bytes of flash offset 0 to detect power-of-2
	 * SPI-NAND address wrap-around when scanning to the end of the chip.
	 */
	if (ubi_offset >= peb_size && !read_fn(0, head0, sizeof(head0), ctx) &&
	    !is_uniform_byte(head0, sizeof(head0), 0xffU) &&
	    !is_uniform_byte(head0, sizeof(head0), 0x00U))
		have_head0 = 1;

	/* Pass 1: discover active PEB span and locate both layout volume copies. */
	for (uint32_t peb = 0;; peb++) {
		uint32_t delta;
		uint32_t off;

		if (peb > max_scan_size / peb_size)
			break;
		delta = peb * peb_size;
		if (delta >= max_scan_size || ubi_offset > UINT32_MAX - delta)
			break;
		off = ubi_offset + delta;
		if (off > UINT32_MAX - peb_size)
			break;

		if (read_fn(off, hdr, sizeof(hdr), ctx))
			return ECONET_UBI_ERR_IO;

		if (have_head0 && off >= 0x1000000U && (off & (off - 1U)) == 0 &&
		    bytes_equal(hdr, head0, sizeof(head0)))
			break;

		if (!parse_ec_hdr(hdr, &ec) || !ec_matches(&ref_ec, &ec)) {
			consecutive_invalid++;
			if (consecutive_invalid >= UBI_MAX_CONSECUTIVE_INVALID)
				break;
			continue;
		}

		consecutive_invalid = 0;
		scanned_pebs = peb + 1U;
		if (!ref_ec.image_seq && ec.image_seq)
			ref_ec.image_seq = ec.image_seq;

		if (read_fn(off + ref_ec.vid_hdr_offset, hdr, sizeof(hdr), ctx))
			return ECONET_UBI_ERR_IO;
		if (!parse_vid_hdr(hdr, &vid))
			continue;

		if (vid.vol_id == UBI_LAYOUT_VOLUME_ID && vid.lnum < UBI_LAYOUT_VOLUME_EBS) {
			if (!layout_found[vid.lnum] || vid.sqnum > layout_sqnum[vid.lnum]) {
				layout_found[vid.lnum] = 1;
				layout_sqnum[vid.lnum] = vid.sqnum;
				layout_peb[vid.lnum] = off;
			}
		}
	}

	if (!layout_found[0] && !layout_found[1])
		return ECONET_UBI_ERR_CORRUPT;

	/* Try the newer layout volume copy first; fall back to the other copy if corrupt. */
	unsigned int order[2] = { 0, 1 };

	if (layout_found[1] && (!layout_found[0] || layout_sqnum[1] > layout_sqnum[0])) {
		order[0] = 1;
		order[1] = 0;
	}

	target_name[0] = '\0';
	for (unsigned int k = 0; k < 2; k++) {
		unsigned int idx = order[k];

		if (!layout_found[idx])
			continue;
		ret = parse_vtbl_copy(read_fn, ctx, layout_peb[idx], ref_ec.data_offset, leb_size,
				      vol_names, vol_name_count, &target_vol_id, &reserved_pebs,
				      &data_pad, &target_vol_type, target_name, &vol_found);
		if (ret == ECONET_UBI_ERR_IO)
			return ret;
		if (ret == ECONET_UBI_OK) {
			vtbl_ok = 1;
			break;
		}
	}

	if (!vtbl_ok)
		return ECONET_UBI_ERR_CORRUPT;
	if (!vol_found) {
		if (info) {
			info->peb_size = peb_size;
			info->leb_size = leb_size;
			info->scanned_pebs = scanned_pebs;
		}
		return ECONET_UBI_NO_VOLUME;
	}

	usable_leb_size = leb_size - data_pad;
	if (!usable_leb_size || reserved_pebs > UBI_MAX_VOL_LEBS)
		return ECONET_UBI_ERR_RANGE;

	for (uint32_t i = 0; i < reserved_pebs; i++)
		w->lebs[i].mapped = 0;

	/* Pass 2: collect the latest valid PEB for every LEB of target_vol_id. */
	for (uint32_t peb = 0; peb < scanned_pebs; peb++) {
		uint32_t off = ubi_offset + peb * peb_size;
		struct ubi_leb_entry *e;

		if (read_fn(off, hdr, sizeof(hdr), ctx))
			return ECONET_UBI_ERR_IO;
		if (!parse_ec_hdr(hdr, &ec) || !ec_matches(&ref_ec, &ec))
			continue;

		if (read_fn(off + ref_ec.vid_hdr_offset, hdr, sizeof(hdr), ctx))
			return ECONET_UBI_ERR_IO;
		if (!parse_vid_hdr(hdr, &vid))
			continue;
		if (vid.vol_id != target_vol_id)
			continue;
		if (vid.vol_type != target_vol_type || vid.lnum >= reserved_pebs ||
		    vid.data_pad != data_pad)
			continue;
		if ((target_vol_type == UBI_VID_STATIC || vid.copy_flag) &&
		    vid.data_size > usable_leb_size)
			continue;

		e = &w->lebs[vid.lnum];
		if (e->mapped && vid.sqnum <= e->sqnum)
			continue;

		/*
		 * If a PEB has copy_flag set, verify its data CRC before accepting it
		 * so an interrupted wear-level copy is never used.
		 */
		if (vid.copy_flag) {
			int ok = verify_leb_crc(read_fn, ctx, off + ref_ec.data_offset,
						vid.data_size, vid.data_crc);

			if (ok < 0)
				return ok;
			if (!ok)
				continue;
		}

		e->mapped = 1;
		e->peb_off = off;
		e->sqnum = vid.sqnum;
		e->data_size = vid.data_size;
		e->data_crc = vid.data_crc;
		e->check_crc = (target_vol_type == UBI_VID_STATIC || vid.copy_flag) ? 1U : 0U;

		if (target_vol_type == UBI_VID_STATIC) {
			if (vid.used_ebs == 0 || vid.used_ebs > reserved_pebs)
				return ECONET_UBI_ERR_CORRUPT;
			if (!static_used_ebs)
				static_used_ebs = vid.used_ebs;
			else if (static_used_ebs != vid.used_ebs)
				return ECONET_UBI_ERR_CORRUPT;
		}
	}

	uint32_t total_size = 0;

	if (target_vol_type == UBI_VID_STATIC) {
		if (!static_used_ebs)
			return ECONET_UBI_ERR_CORRUPT;

		for (uint32_t i = 0; i < static_used_ebs; i++) {
			struct ubi_leb_entry *e = &w->lebs[i];

			if (!e->mapped)
				return ECONET_UBI_ERR_CORRUPT;
			if (i + 1U < static_used_ebs && e->data_size != usable_leb_size)
				return ECONET_UBI_ERR_CORRUPT;
			if (e->data_size > dst_cap - total_size)
				return ECONET_UBI_ERR_RANGE;
			if (e->data_size) {
				if (read_fn(e->peb_off + ref_ec.data_offset, dst + total_size,
					    e->data_size, ctx))
					return ECONET_UBI_ERR_IO;
				if (econet_crc32_raw(dst + total_size, e->data_size) != e->data_crc)
					return ECONET_UBI_ERR_CORRUPT;
			}
			total_size += e->data_size;
		}
	} else {
		uint32_t num_lebs = 0;

		for (uint32_t i = 0; i < reserved_pebs; i++) {
			if (w->lebs[i].mapped)
				num_lebs = i + 1U;
		}
		if (!num_lebs)
			return ECONET_UBI_ERR_CORRUPT;

		for (uint32_t i = 0; i < num_lebs; i++) {
			struct ubi_leb_entry *e = &w->lebs[i];

			if (usable_leb_size > dst_cap - total_size)
				return ECONET_UBI_ERR_RANGE;
			if (e->mapped) {
				if (read_fn(e->peb_off + ref_ec.data_offset, dst + total_size,
					    usable_leb_size, ctx))
					return ECONET_UBI_ERR_IO;
				if (e->check_crc &&
				    econet_crc32_raw(dst + total_size, e->data_size) != e->data_crc)
					return ECONET_UBI_ERR_CORRUPT;
			} else {
				for (uint32_t b = 0; b < usable_leb_size; b++)
					dst[total_size + b] = 0xffU;
			}
			total_size += usable_leb_size;
		}
	}

	if (info) {
		info->peb_size = peb_size;
		info->leb_size = leb_size;
		info->usable_leb_size = usable_leb_size;
		info->scanned_pebs = scanned_pebs;
		info->vol_id = target_vol_id;
		info->data_size = total_size;
		info->vol_type = target_vol_type;
		for (size_t c = 0; c < sizeof(info->vol_name); c++)
			info->vol_name[c] = target_name[c];
	}

	return ECONET_UBI_OK;
}
