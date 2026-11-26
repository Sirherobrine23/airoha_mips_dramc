/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef ECONET_CRC32_H
#define ECONET_CRC32_H

#include <stdint.h>

uint32_t econet_crc32_ieee(const volatile uint8_t *buf, uint32_t len);
uint32_t econet_crc32_ieee_wdog(const volatile uint8_t *buf, uint32_t len);

#endif
