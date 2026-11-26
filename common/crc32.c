/* SPDX-License-Identifier: GPL-2.0+ */
#include <crc32.h>
#include <timer.h>

uint32_t econet_crc32_ieee(const volatile uint8_t *buf, uint32_t len)
{
	uint32_t crc = 0xffffffffu;
	uint32_t i;
	int bit;

	for (i = 0; i < len; i++) {
		crc ^= buf[i];
		for (bit = 0; bit < 8; bit++)
			crc = (crc >> 1) ^ ((0u - (crc & 1u)) & 0xedb88320u);
	}

	return ~crc;
}

uint32_t econet_crc32_ieee_wdog(const volatile uint8_t *buf, uint32_t len)
{
	uint32_t crc = 0xffffffffu;
	uint32_t i;
	int bit;

	for (i = 0; i < len; i++) {
		crc ^= buf[i];
		for (bit = 0; bit < 8; bit++)
			crc = (crc >> 1) ^ ((0u - (crc & 1u)) & 0xedb88320u);
		if ((i & 0x3fffu) == 0)
			watchdog_kick();
	}

	return ~crc;
}
