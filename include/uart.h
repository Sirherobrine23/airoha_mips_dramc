/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __AIROHA_UART_H__
#define __AIROHA_UART_H__

#include <stdint.h>

#define AIROHA_UART_BASE		0xbfbf0000u
#define AIROHA_UART_RBR			0x00u
#define AIROHA_UART_THR			0x00u
#define AIROHA_UART_IER			0x04u
#define AIROHA_UART_LSR			0x14u
#define AIROHA_UART_LSR_DR		0x01u
#define AIROHA_UART_LSR_THRE	0x20u
#define AIROHA_UART_LSR_TEMT	0x40u
#define AIROHA_UART_LSR_ERR		0x1eu /* OE | PE | FE | BI */

void uart_putc(char c);

/*
 * Tiny chainloader formatter, deliberately not libc printf().  Flash stages
 * can stay on their smaller stage-local helpers to avoid carrying it.
 */
void uart_printf(const char *fmt, ...);

#endif
