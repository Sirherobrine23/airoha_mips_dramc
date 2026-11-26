// SPDX-License-Identifier: GPL-2.0+
/* Bounded post-DRAM gzip and LZMA-alone decompression. */
#include "decompress.h"
#include "decompress/LzmaDec.h"
#include "decompress/zlib.h"

struct arena {
	uint8_t *base;
	size_t size, used;
};

static void *arena_alloc(struct arena *a, size_t size)
{
	void *p;

	if (size > SIZE_MAX - 7)
		return NULL;
	size = (size + 7) & ~(size_t)7;
	if (size > a->size - a->used)
		return NULL;
	p = a->base + a->used;
	a->used += size;
	return p;
}

static voidpf gzip_alloc(voidpf opaque, uInt count, uInt size)
{
	if (size && count > SIZE_MAX / size)
		return NULL;
	return arena_alloc(opaque, (size_t)count * size);
}

static void gzip_free(voidpf opaque, voidpf address)
{
	(void)opaque;
	(void)address;
}

struct lzma_allocator {
	ISzAlloc ops;
	struct arena *arena;
};

static void *lzma_alloc(void *opaque, size_t size)
{
	struct lzma_allocator *a = opaque;

	return arena_alloc(a->arena, size);
}

static void lzma_free(void *opaque, void *address)
{
	(void)opaque;
	(void)address;
}

static int decompress_common(uint32_t algorithm, const uint8_t *src, size_t src_len, uint8_t *dst,
			     size_t dst_cap, int exact, size_t *out_len, void *work,
			     size_t work_len)
{
	struct arena arena = { work, work_len, 0 };

	if (!src || !dst || !work || !src_len || !dst_cap || ((uintptr_t)work & 7) ||
	    src_len > UINT32_MAX || dst_cap > UINT32_MAX)
		return -1;
	if (algorithm == ECONET_COMP_GZIP) {
		z_stream stream = { 0 };
		int ret, valid;

		stream.zalloc = gzip_alloc;
		stream.zfree = gzip_free;
		stream.opaque = &arena;
		stream.next_in = (Bytef *)(uintptr_t)src;
		stream.avail_in = (uInt)src_len;
		stream.next_out = dst;
		stream.avail_out = (uInt)dst_cap;
		/* Gzip only: verify its header, trailer CRC32 and ISIZE. */
		if (inflateInit2(&stream, 15 + 16) != Z_OK)
			return -1;
		ret = inflate(&stream, Z_FINISH);
		valid = ret == Z_STREAM_END && stream.total_in == src_len &&
			(exact ? stream.total_out == dst_cap
			       : (stream.total_out > 0 && stream.total_out <= dst_cap));
		if (valid && out_len)
			*out_len = stream.total_out;
		inflateEnd(&stream);
		return valid ? 0 : -1;
	}
	if (algorithm == ECONET_COMP_LZMA) {
		struct lzma_allocator allocator = { { lzma_alloc, lzma_free }, &arena };
		ELzmaFinishMode finish_mode = LZMA_FINISH_END;
		ELzmaStatus status;
		SizeT in_size, out_size = dst_cap;
		uint32_t declared;
		int declared_unknown;
		uint32_t dict;
		unsigned int lc, lp, prop;
		int ret;

		/* LZMA-alone: properties (5 bytes), LE unpacked size (8 bytes). */
		if (src_len < 18 || src[0] >= 9 * 5 * 5)
			return -1;
		prop = src[0];
		lc = prop % 9;
		lp = (prop / 9) % 5;
		/* Bound the probability models to our 64 KiB workspace. */
		if (lc + lp > 4)
			return -1;
		dict = (uint32_t)src[1] | (uint32_t)src[2] << 8 | (uint32_t)src[3] << 16 |
		       (uint32_t)src[4] << 24;
		if (dict > 0x1000000U)
			return -1;
		declared = (uint32_t)src[5] | (uint32_t)src[6] << 8 | (uint32_t)src[7] << 16 |
			   (uint32_t)src[8] << 24;
		declared_unknown = src[5] == 0xff && src[6] == 0xff && src[7] == 0xff &&
				   src[8] == 0xff && src[9] == 0xff && src[10] == 0xff &&
				   src[11] == 0xff && src[12] == 0xff;
		/* dst_cap is bounded to u32, so known LZMA sizes need no u64 math. */
		if (!declared_unknown) {
			if (src[9] || src[10] || src[11] || src[12])
				return -1;
			if (exact) {
				if (declared != dst_cap)
					return -1;
			} else {
				if (!declared || declared > dst_cap)
					return -1;
				out_size = declared;
			}
		} else if (!exact) {
			finish_mode = LZMA_FINISH_ANY;
		}
		in_size = src_len - 13;
		ret = LzmaDecode(dst, &out_size, src + 13, &in_size, src, 5, finish_mode, &status,
				 &allocator.ops);
		if (ret != SZ_OK || in_size != src_len - 13)
			return -1;
		if (exact) {
			if (out_size != dst_cap)
				return -1;
		} else {
			if (!out_size || out_size > dst_cap)
				return -1;
			if (!declared_unknown && out_size != declared)
				return -1;
		}
		if (status == LZMA_STATUS_FINISHED_WITH_MARK ||
		    (!declared_unknown && status == LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK)) {
			if (out_len)
				*out_len = out_size;
			return 0;
		}
	}
	return -1;
}

int econet_decompress(uint32_t algorithm, const uint8_t *src, size_t src_len, uint8_t *dst,
		      size_t dst_len, void *work, size_t work_len)
{
	return decompress_common(algorithm, src, src_len, dst, dst_len, 1, NULL, work, work_len);
}

int econet_decompress_bounded(uint32_t algorithm, const uint8_t *src, size_t src_len, uint8_t *dst,
			      size_t dst_cap, size_t *out_len, void *work, size_t work_len)
{
	if (!out_len)
		return -1;
	return decompress_common(algorithm, src, src_len, dst, dst_cap, 0, out_len, work, work_len);
}
