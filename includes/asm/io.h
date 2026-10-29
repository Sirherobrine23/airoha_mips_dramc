/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef __IO__
#define __IO__

#include <asm/addrspace.h>

/* Raw native-endian register access; callers supply the mapped address. */
#define readl(reg)		(*(volatile unsigned int *)(unsigned long)(reg))
#define writel(val, reg)		(readl(reg) = (unsigned int)(val))
#define setl(reg, val)		do { readl(reg) |= (val); } while (0)
#define clearl(reg, val)		do { readl(reg) &= ~(val); } while (0)

#define readw(reg)		(*(volatile unsigned short *)(unsigned long)(reg))
#define writew(val, reg)		(readw(reg) = (unsigned short)(val))
#define setw(reg, val)		do { readw(reg) |= (val); } while (0)
#define clearw(reg, val)		do { readw(reg) &= ~(val); } while (0)

#define readb(reg)		(*(volatile unsigned char *)(unsigned long)(reg))
#define writeb(val, reg)		(readb(reg) = (unsigned char)(val))
#define setb(reg, val)		do { readb(reg) |= (val); } while (0)
#define clearb(reg, val)		do { readb(reg) &= ~(val); } while (0)

#endif
