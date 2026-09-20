/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef ECONET_FLASH_LOADER_H
#define ECONET_FLASH_LOADER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <uart.h>
#include "sfc.h"

typedef uint8_t u8;
typedef uint32_t u32;

void uart_put_hex32(uint32_t value);

#define __iomem
#define BIT(n)					(1U << (n))
#define ARRAY_SIZE(a)			(sizeof(a) / sizeof((a)[0]))
#define ETIMEDOUT				110
#define ECONET_SFC_BASE			0xbfa10000U
#define REG(a)					(*(volatile u32 *)(a))
#define __raw_readl(p)			(*(volatile u32 *)(p))
#define __raw_writel(v, p)		(*(volatile u32 *)(p) = (v))

#define mmio_read32(addr)		__raw_readl(addr)
#define mmio_write32(addr, val)	__raw_writel(val, addr)

#ifndef UBOOT_OFFSET
#define UBOOT_OFFSET			0x20000U
#endif
#define IMAGE_LIMIT				0x100000U
#define HEADER_SIZE				64U
#define IH_MAGIC				0x27051956U

#define FDT_MAGIC				0xd00dfeedU
#define FDT_BEGIN_NODE			0x00000001U
#define FDT_END_NODE			0x00000002U
#define FDT_PROP				0x00000003U
#define FDT_NOP					0x00000004U
#define FDT_END					0x00000009U

#define ECONET_BOOT_MAGIC		0x45434e54U
#define ECONET_BOOT_VERSION		1U
#define ECONET_BOOT_HEADER_SIZE	32U

#ifndef UBOOT_LOAD_ADDR
#define   UBOOT_LOAD_ADDR		0x81000000
#endif


#endif
