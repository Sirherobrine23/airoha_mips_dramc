/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __BITS_H
#define __BITS_H

#define BITS_PER_LONG				32

#define GENMASK(h, l)				(((~0UL) - (1UL << (l)) + 1UL) & (~0UL >> (BITS_PER_LONG - 1 - (h))))
#define FIELD_PREP(mask, val)		(((val) << (__builtin_ctzl(mask))) & (mask))
#define FIELD_GET(mask, reg)		(((reg) & (mask)) >> (__builtin_ctzl(mask)))

#define BIT(nr)						(1UL << (nr))

#endif