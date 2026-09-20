/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __CHAINLOADER_H__
#define __CHAINLOADER_H__

/*
 * This is freestanding firmware, not Linux kernel code.  Use only headers
 * provided by the C toolchain itself; do not depend on linux/kernel.h or
 * other in-kernel headers being installed in the cross sysroot.
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#define UART_BASE						0xbfbf0000u
#define UART_RBR						0x00u
#define UART_THR						0x00u
#define UART_IER						0x04u
#define UART_LSR						0x14u
#define UART_LSR_DR						0x01u
#define UART_LSR_THRE					0x20u
#define UART_LSR_TEMT					0x40u
#define UART_LSR_ERR					0x1eu  /* OE | PE | FE | BI */

#ifndef UBOOT_LOAD_ADDR
#define UBOOT_LOAD_ADDR						0x81000000u
#endif

/* Cached KSEG0: uncached 8-bit stores corrupt the word on this chip. */
#define UBOOT_LOAD_CACHED				(UBOOT_LOAD_ADDR)
#define UBOOT_ENTRY_CACHED				(UBOOT_LOAD_ADDR)
#define UBOOT_MAX_SIZE					0x00400000u
#define UBOOT_DRAM_LIMIT				0x82000000u

/* Interactive bootext/recovery menu. */
#define CHAINLOADER_MENU_TIMEOUT_SEC	10u
#define CHAINLOADER_FLASH_UNSUPPORTED	(-38)
#define TCBOOT_FLASH_SIZE				0x00100000u
#define TCBOOT_FLASH_SIZE_MIN			0x00020000u

#define CR_TIMER_CTL					0xbfbf0100u

/* Assumed console baud rate for calibrating the timebase. */
#define CONSOLE_CPS						11520u /* 115200 8N1 = 11520 chars/s */
#define DEFAULT_TICKS_PER_MS			250000u

/* Protocol tuning. */
#define HANDSHAKE_TRIES					200
#define HANDSHAKE_MS					3000
#define BYTE_MS							1000
#define NEXT_HDR_MS						5000
#define CSUM_PROBE_MS					300
#define PURGE_MS						300
#define MAX_ERRORS						16

#define CHK_CHUNK						128u

/* Supported U-Boot input/container formats. */
#define IH_MAGIC						0x27051956u
#define IH_HEADER_SIZE					64u
#define IH_ARCH_MIPS					5u
#define IH_TYPE_FIRMWARE				5u
#define IH_COMP_NONE					0u

#define FDT_MAGIC						0xd00dfeedu
#define FDT_BEGIN_NODE					0x00000001u
#define FDT_END_NODE					0x00000002u
#define FDT_PROP						0x00000003u
#define FDT_NOP							0x00000004u
#define FDT_END							0x00000009u

#define ECONET_BOOT_MAGIC				0x45434e54u /* "ECNT" */
#define ECONET_BOOT_VERSION				1u
#define ECONET_BOOT_HEADER_SIZE			32u

/*
 * On-wire ECNT header.  Every field is stored big-endian regardless of SoC
 * endianness.  Payload normally follows immediately at payload_offset.
 */
struct econet_boot_header {
	uint32_t magic;
	uint32_t version;
	uint32_t payload_offset;
	uint32_t payload_size;
	uint32_t load_addr;
	uint32_t entry_addr;
	uint32_t payload_crc32;
	uint32_t header_crc32;
};

extern u32 __image_start;
extern u32 __chk_start;

static u32 ticks_per_ms;
static u32 tx_chars;


static inline u32 mmio_read32(u32 addr)
{
	return *(volatile u32 *)(uintptr_t)addr;
}

static inline void mmio_write32(u32 addr, u32 val)
{
	*(volatile u32 *)(uintptr_t)addr = val;
}

/* CP0 Count: fixed-rate timebase, independent of loop speed. */
static inline u32 cp0_count(void)
{
	u32 v;

	__asm__ volatile("mfc0 %0, $9" : "=r"(v));
	return v;
}


static inline void delay_ms(unsigned int ms)
{
	while (ms--) {
		u32 start = cp0_count();

		while ((u32)(cp0_count() - start) < ticks_per_ms)
			;
	}
}

static inline void sleep_sec(unsigned int sec)
{
	while (sec--)
		delay_ms(1000);
}


enum {
	SOH = 0x01,
	STX = 0x02,
	EOT = 0x04,
	ACK = 0x06,
	NAK = 0x15,
	CAN = 0x18,
	CRC_REQ = 'C',
};

enum image_type {
	TYPE_LEGACY,
	TYPE_FIT,
	TYPE_RAW,
	TYPE_ECNT,
};

int chainloader_flash_tcboot(const void *image, u32 len);

#endif
