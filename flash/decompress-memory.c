// SPDX-License-Identifier: GPL-2.0+
#include <stddef.h>

void *memcpy(void *dst, const void *src, size_t size)
{
	unsigned char *d = dst;
	const unsigned char *s = src;

	while (size--)
		*d++ = *s++;
	return dst;
}

void *memset(void *dst, int value, size_t size)
{
	unsigned char *d = dst;

	while (size--)
		*d++ = (unsigned char)value;
	return dst;
}
