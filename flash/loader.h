/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef ECONET_FLASH_LOADER_H
#define ECONET_FLASH_LOADER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint32_t u32;

#define __iomem
#define BIT(n)				(1U << (n))
#define ARRAY_SIZE(a)		(sizeof(a) / sizeof((a)[0]))
#define ETIMEDOUT			110
#define EIO					5
#define EINVAL				22
#define ECONET_SFC_BASE		0xbfa10000U
#define REG(a)				(*(volatile u32 *)(a))
#define __raw_readl(p)		(*(volatile u32 *)(p))
#define __raw_writel(v, p)	(*(volatile u32 *)(p) = (v))

#ifndef UBOOT_OFFSET
#define UBOOT_OFFSET		0x20000U
#endif
#define IMAGE_LIMIT			0x100000U
#define HEADER_SIZE			64U
#define IH_MAGIC			0x27051956U
#define ECONET_BOOT_MAGIC		0x45434e54U
#define ECONET_BOOT_VERSION		1U
#define ECONET_BOOT_HEADER_SIZE	32U

#ifndef UBOOT_LOAD_ADDR
#define   UBOOT_LOAD_ADDR	0x81000000
#endif

int econet_sfc_init(void);
int econet_sfc_read(u32 offset, void *dst, size_t len);

#endif
