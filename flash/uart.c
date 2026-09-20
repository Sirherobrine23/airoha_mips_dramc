/* SPDX-License-Identifier: GPL-2.0+ */
#include <stdarg.h>

#include "loader.h"

static u32 tx_chars;

void uart_putc(char c)
{
	while (!(mmio_read32(AIROHA_UART_BASE + AIROHA_UART_LSR) &
		 AIROHA_UART_LSR_THRE))
		;
	mmio_write32(AIROHA_UART_BASE + AIROHA_UART_THR, (u8)c);
	tx_chars++;
}

static void uart_putc_text(char c)
{
	if (c == '\n')
		uart_putc('\r');

	uart_putc(c);
}

static void uart_puts_text(const char *s)
{
	char prev = 0;

	while (*s) {
		if (*s == '\n' && prev != '\r')
			uart_putc('\r');
		uart_putc(*s);
		prev = *s++;
	}
}

static void uart_put_unsigned(unsigned int value, unsigned int base,
			      unsigned int width, char pad)
{
	char buf[16];
	unsigned int n = 0;

	do {
		unsigned int digit = value % base;

		buf[n++] = (char)(digit < 10 ? '0' + digit : 'a' + digit - 10);
		value /= base;
	} while (value && n < sizeof(buf));

	while (n < width && n < sizeof(buf))
		buf[n++] = pad;

	while (n)
		uart_putc(buf[--n]);
}

static void uart_put_signed(int value, unsigned int width, char pad)
{
	unsigned int magnitude;

	if (value < 0) {
		uart_putc('-');
		magnitude = 0u - (unsigned int)value;
		if (width)
			width--;
	} else {
		magnitude = (unsigned int)value;
	}

	uart_put_unsigned(magnitude, 10, width, pad);
}

void uart_printf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	while (*fmt) {
		unsigned int width = 0;
		char pad = ' ';
		char spec;

		if (*fmt != '%') {
			uart_putc_text(*fmt++);
			continue;
		}

		fmt++;
		if (*fmt == '%') {
			uart_putc('%');
			fmt++;
			continue;
		}

		if (*fmt == '0') {
			pad = '0';
			fmt++;
		}
		while (*fmt >= '0' && *fmt <= '9') {
			width = width * 10u + (unsigned int)(*fmt - '0');
			fmt++;
		}

		spec = *fmt ? *fmt++ : '\0';
		switch (spec) {
		case 'c':
			uart_putc_text((char)va_arg(ap, int));
			break;
		case 's': {
			const char *s = va_arg(ap, const char *);

			uart_puts_text(s ? s : "(null)");
			break;
		}
		case 'd':
			uart_put_signed(va_arg(ap, int), width, pad);
			break;
		case 'u':
			uart_put_unsigned(va_arg(ap, unsigned int), 10, width, pad);
			break;
		case 'x':
		case 'X':
			uart_put_unsigned(va_arg(ap, unsigned int), 16, width, pad);
			break;
		case '\0':
			fmt--;
			break;
		default:
			uart_putc('%');
			uart_putc(spec);
			break;
		}
	}
	va_end(ap);
}

void uart_put_hex32(uint32_t value)
{
	int shift;

	for (shift = 28; shift >= 0; shift -= 4) {
		unsigned int digit = (value >> shift) & 0xfu;

		uart_putc((char)(digit < 10 ? '0' + digit : 'a' + digit - 10));
	}
}
