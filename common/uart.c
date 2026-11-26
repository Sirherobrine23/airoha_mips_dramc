/* SPDX-License-Identifier: GPL-2.0+ */
#include <stdarg.h>
#include <stdint.h>
#include <asm/io.h>
#include <uart.h>

/*
 * Shared polling NS16550-compatible UART driver for EcoNet/Airoha MIPS stages.
 * Compiled with -ffunction-sections so small SRAM stages discard uart_printf()
 * when linked with --gc-sections.
 */

#ifdef UART_ACCESS_BYTE_BE
#define UART_REG_READ(off)		readb(AIROHA_UART_BASE + (off) + 3u)
#define UART_REG_WRITE(off, val)	writeb((uint8_t)(val), AIROHA_UART_BASE + (off) + 3u)
#else
#define UART_REG_READ(off)		readl(AIROHA_UART_BASE + (off))
#define UART_REG_WRITE(off, val)	writel((uint32_t)(uint8_t)(val), AIROHA_UART_BASE + (off))
#endif

static uint32_t tx_chars;

void uart_tx_reset(void)
{
	tx_chars = 0;
}

uint32_t uart_tx_count(void)
{
	return tx_chars;
}

void uart_init(void)
{
	UART_REG_WRITE(AIROHA_UART_FCR, 0x0fu);
	UART_REG_WRITE(AIROHA_UART_MCR, 0x00u);
	UART_REG_WRITE(AIROHA_UART_MISCC, 0x00u);
	UART_REG_WRITE(AIROHA_UART_IER, 0x01u);
	UART_REG_WRITE(AIROHA_UART_LCR, 0x80u);
	/* 115200 baud from 20 MHz UART clock: X = 59904 (0xea00), Y = 65000 (0xfde8). */
	writel(0xea00fde8u, AIROHA_UART_BASE + AIROHA_UART_XYD);
	UART_REG_WRITE(AIROHA_UART_BRDL, 0x01u);
	UART_REG_WRITE(AIROHA_UART_BRDH, 0x00u);
	UART_REG_WRITE(AIROHA_UART_LCR, 0x03u);
}

void uart_putc(char c)
{
	while (!(UART_REG_READ(AIROHA_UART_LSR) & AIROHA_UART_LSR_THRE))
		;
	UART_REG_WRITE(AIROHA_UART_THR, (uint8_t)c);
	tx_chars++;
}

static void uart_putc_text(char c)
{
	if (c == '\n')
		uart_putc('\r');

	uart_putc(c);
}

void uart_puts(const char *s)
{
	char prev = 0;

	while (*s) {
		if (*s == '\n' && prev != '\r')
			uart_putc('\r');
		uart_putc(*s);
		prev = *s++;
	}
}

void uart_put_hex(uint32_t value, int digits)
{
	int shift;

	for (shift = (digits - 1) * 4; shift >= 0; shift -= 4) {
		uint32_t nibble = (value >> shift) & 0xfu;

		uart_putc((char)(nibble < 10 ? '0' + nibble : 'a' + nibble - 10));
	}
}

void uart_put_hex32(uint32_t value)
{
	uart_put_hex(value, 8);
}

void uart_put_dec(uint32_t value)
{
	char buf[11];
	unsigned int i = sizeof(buf);

	buf[--i] = '\0';
	do {
		buf[--i] = (char)('0' + value % 10u);
		value /= 10u;
	} while (value && i);
	uart_puts(&buf[i]);
}

void prom_puts(const char *s)
{
	uart_puts(s);
}

void prom_print_hex(unsigned int value, int digits)
{
	uart_put_hex(value, digits);
}

void prom_print_dec(unsigned int value)
{
	uart_put_dec(value);
}

static void uart_put_unsigned(unsigned int value, unsigned int base, unsigned int width, char pad)
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

			uart_puts(s ? s : "(null)");
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
