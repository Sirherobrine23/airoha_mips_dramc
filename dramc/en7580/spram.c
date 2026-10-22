/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en7580_ddr.h"
#define KSEG0 0x80000000u
#define SPRAM_ENABLE 0x80u
#define SPRAM_MASK 0xfffff000u
#define ERRCTL_SPRAM BIT(28)
#define CACHE_OP(op, addr) __asm__ volatile("cache %0,0(%1)" :: "i"(op), "r"(addr) : "memory")
static inline void hazard(void) { __asm__ volatile("ehb" ::: "memory"); }
static u32 spram_begin(void)
{
	u32 old, enable;
	__asm__ volatile("mfc0 %0,$26" : "=r"(old));
	enable = old | ERRCTL_SPRAM;
	__asm__ volatile("mtc0 %0,$26" :: "r"(enable) : "memory");
	hazard(); return old;
}
static void spram_end(u32 old)
{
	__asm__ volatile("mtc0 %0,$26" :: "r"(old) : "memory"); hazard();
}
static u32 load_tag(u32 offset, int instruction)
{
	u32 old = spram_begin(), value;
	u32 address = KSEG0 | offset;
	if (instruction) CACHE_OP(4, address); else CACHE_OP(5, address);
	CACHE_OP(7, address); __asm__ volatile("sync" ::: "memory"); hazard();
	if (instruction) __asm__ volatile("mfc0 %0,$28" : "=r"(value));
	else __asm__ volatile("mfc0 %0,$28,2" : "=r"(value));
	hazard(); spram_end(old); return value;
}
void probe_spram(u32 base, u32 enable)
{
	int instruction = !!(enable & 2);
	u32 old, size, tag;
	(void)load_tag(0, instruction); size = load_tag(8, instruction) & SPRAM_MASK;
	if (!size) return;
	tag = ((base + size - 1) & -size) & SPRAM_MASK;
	if (enable & 1) tag |= SPRAM_ENABLE;
	old = spram_begin();
	if (instruction) {
		__asm__ volatile("mtc0 %0,$28" :: "r"(tag) : "memory"); hazard(); CACHE_OP(8, KSEG0);
	} else {
		__asm__ volatile("mtc0 %0,$28,2" :: "r"(tag) : "memory"); hazard(); CACHE_OP(9, KSEG0);
	}
	CACHE_OP(11, KSEG0); __asm__ volatile("sync" ::: "memory"); hazard(); spram_end(old);
}
void ispram_fill(u32 size, u32 from, u32 to)
{
	u32 tag = load_tag(0, 1), capacity = load_tag(8, 1);
	if (!size) size = capacity & SPRAM_MASK;
	if (!from) from = tag & SPRAM_MASK;
	for (u32 offset = 0; offset < size; offset += 8, from += 8) {
		volatile u32 *source = (void *)(KSEG0 | from);
		u32 first = source[0], second = source[1], old = spram_begin();
		/* EN7580 reverses the two words relative to the EN751221 helper. */
		__asm__ volatile("mtc0 %0,$28,1" :: "r"(second) : "memory"); hazard();
		__asm__ volatile("mtc0 %0,$29,1" :: "r"(first) : "memory"); hazard();
		CACHE_OP(12, KSEG0 | (to + offset)); CACHE_OP(15, KSEG0 | (to + offset));
		__asm__ volatile("sync" ::: "memory"); hazard(); spram_end(old);
	}
}
