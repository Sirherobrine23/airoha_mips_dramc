/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __AIROHA_UART_H__
#define __AIROHA_UART_H__

#include <stdint.h>

#define AIROHA_UART_BASE	0xbfbf0000u
#define AIROHA_UART_RBR		0x00u
#define AIROHA_UART_THR		0x00u
#define AIROHA_UART_BRDL	0x00u
#define AIROHA_UART_BRDH	0x04u
#define AIROHA_UART_IER		0x04u
#define AIROHA_UART_FCR		0x08u
#define AIROHA_UART_LCR		0x0cu
#define AIROHA_UART_MCR		0x10u
#define AIROHA_UART_LSR		0x14u
#define AIROHA_UART_MISCC	0x24u
#define AIROHA_UART_XYD		0x2cu

#define AIROHA_UART_LSR_DR	0x01u
#define AIROHA_UART_LSR_THRE	0x20u
#define AIROHA_UART_LSR_TEMT	0x40u
#define AIROHA_UART_LSR_ERR	0x1eu /* OE | PE | FE | BI */

void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *s);
void uart_put_hex(uint32_t value, int digits);
void uart_put_hex32(uint32_t value);
void uart_put_dec(uint32_t value);
void uart_printf(const char *fmt, ...);

void uart_tx_reset(void);
uint32_t uart_tx_count(void);

/* Legacy DRAMC console names backed by the same shared implementation. */
void prom_puts(const char *s);
void prom_print_hex(unsigned int value, int digits);
void prom_print_dec(unsigned int value);

#endif
