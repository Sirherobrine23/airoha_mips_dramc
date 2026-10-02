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
#define BIT(n)                  (1U << (n))
#define ARRAY_SIZE(a)           (sizeof(a) / sizeof((a)[0]))
#define ETIMEDOUT               110
#define ECONET_SFC_BASE         0xbfa10000U
#define REG(a)                  (*(volatile u32 *)(a))
#define __raw_readl(p)          (*(volatile u32 *)(p))
#define __raw_writel(v, p)      (*(volatile u32 *)(p) = (v))

#define mmio_read32(addr)       __raw_readl(addr)
#define mmio_write32(addr, val) __raw_writel(val, addr)

#ifndef PAYLOAD_OFFSET
#define PAYLOAD_OFFSET          0x20000U
#endif

#define IMAGE_LIMIT             0x100000U
#define PAYLOAD_RAM_MIN         0x81000000U
#define PAYLOAD_RAM_MAX         0x82000000U

#include "ecnt.h"

#endif
