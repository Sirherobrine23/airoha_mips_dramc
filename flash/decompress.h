/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef ECONET_DECOMPRESS_H
#define ECONET_DECOMPRESS_H
#include <stddef.h>
#include <stdint.h>
#include "ecnt.h"

#define ECONET_DECOMP_WORK_SIZE 0x10000U

/* Input, output and workspace must be disjoint. No heap or libc required.
 * Success requires exactly src_len input bytes and dst_len output bytes.
 */
int econet_decompress(uint32_t algorithm, const uint8_t *src, size_t src_len,
                      uint8_t *dst, size_t dst_len, void *work, size_t work_len);
#endif
