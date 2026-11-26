/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef ECONET_UBI_H
#define ECONET_UBI_H

#include <stddef.h>
#include <stdint.h>

#define UBI_EC_HDR_MAGIC		0x55424923U /* "UBI#" */
#define UBI_VID_HDR_MAGIC		0x55424921U /* "UBI!" */
#define UBI_VERSION			1U

#define UBI_EC_HDR_SIZE			64U
#define UBI_VID_HDR_SIZE		64U
#define UBI_VTBL_RECORD_SIZE		172U
#define UBI_MAX_VOLUMES			128U

#define UBI_LAYOUT_VOLUME_ID		0x7fffefffU
#define UBI_LAYOUT_VOLUME_EBS		2U

#define UBI_VID_DYNAMIC			1U
#define UBI_VID_STATIC			2U

#define UBI_MIN_PEB_SIZE		0x10000U    /* 64 KiB */
#define UBI_MAX_PEB_SIZE		0x100000U   /* 1 MiB */
#define UBI_START_PROBE_SIZE		0x80000U    /* 512 KiB initial bad-block window */
#define UBI_MAX_VOL_LEBS		256U
#define UBI_MAX_CONSECUTIVE_INVALID	16U
#define UBI_WORK_SIZE			0x10000U   /* 64 KiB scratch area */

#ifndef UBI_MAX_SCAN_SIZE
#define UBI_MAX_SCAN_SIZE		0x10000000U /* 256 MiB safety ceiling */
#endif

enum econet_ubi_status {
	ECONET_UBI_OK = 0,
	ECONET_UBI_NO_UBI = 1,
	ECONET_UBI_NO_VOLUME = 2,
	ECONET_UBI_ERR_IO = -1,
	ECONET_UBI_ERR_CORRUPT = -2,
	ECONET_UBI_ERR_RANGE = -3,
};

struct econet_ubi_info {
	uint32_t peb_size;
	uint32_t leb_size;
	uint32_t usable_leb_size;
	uint32_t scanned_pebs;
	uint32_t vol_id;
	uint32_t data_size;
	uint8_t vol_type;
	char vol_name[32];
};

typedef int (*econet_ubi_read_fn)(uint32_t offset, void *dst, size_t len, void *ctx);

/* Un-inverted IEEE 802.3 CRC-32 (Linux UBI CRC-32). Standard IEEE CRC-32 is ~econet_crc32_raw(). */
uint32_t econet_crc32_raw(const uint8_t *p, uint32_t len);

/*
 * Dynamically detect a UBI partition starting at ubi_offset, locate the highest-priority
 * volume matching vol_names[], and read its contents into dst (up to dst_cap bytes).
 *
 * work must be 8-byte aligned and at least UBI_WORK_SIZE bytes long.
 */
int econet_ubi_load_volume(econet_ubi_read_fn read_fn, void *ctx, uint32_t ubi_offset,
			   uint32_t max_scan_size, const char *const *vol_names,
			   size_t vol_name_count, uint8_t *dst, size_t dst_cap, void *work,
			   size_t work_len, struct econet_ubi_info *info);

#endif
