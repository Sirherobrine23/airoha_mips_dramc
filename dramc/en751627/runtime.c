/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en751627_ddr.h"

static void serial_outc(char c)
{
    while (!(REG32(0xbfbf0014u) & 0x20))
        ;
    REG32(0xbfbf0000u) = (u32)(signed char)c;
}

void uart_init(void)
{
    /* EN7516 uses native 32-bit UART accesses, including on big endian. */
    REG32(0xbfbf0008u) = 0xf;
    REG32(0xbfbf0010u) = 0;
    REG32(0xbfbf0024u) = 0;
    REG32(0xbfbf0004u) = 1;
    REG32(0xbfbf000cu) = 0x80;
    REG32(0xbfbf002cu) = 0xea00fde8;
    REG32(0xbfbf0000u) = 1;
    REG32(0xbfbf0004u) = 0;
    REG32(0xbfbf000cu) = 3;
}

void prom_puts(const char *s)
{
    while (*s) {
        if (*s == '\n')
            serial_outc('\r');
        serial_outc(*s++);
    }
}

void prom_print_hex(u32 value, int digits)
{
    int shift;

    for (shift = (digits - 1) * 4; shift >= 0; shift -= 4) {
        u32 nibble = (value >> shift) & 15;
        serial_outc(nibble < 10 ? '0' + nibble : 'a' + nibble - 10);
    }
}

void prom_print_dec(u32 value)
{
    u32 divisor, digit;
    int leading = 1;

    for (divisor = 1000000000; divisor; divisor /= 10) {
        digit = value / divisor;
        value %= divisor;
        if (digit || divisor == 1)
            leading = 0;
        if (!leading)
            serial_outc('0' + digit);
    }
}

u32 get_SYS_HCLK(void)
{
    return (REG32(SYS_GLOBAL_PARM) & 0x003fffffu) >> 12;
}

void timer_Configure(u32 timer, u32 enable)
{
    u32 value = REG32(0xbfbf0100u);

    if ((enable & 0xff) == 1)
        value |= BIT(timer & 0xff);
    else
        value &= ~BIT(timer & 0xff);
    REG32(0xbfbf0100u) = value;
}

void timerSet(u32 timer, u32 time, u32 enable, u32 mode, u32 halt)
{
    (void)mode;
    (void)halt;
    REG32(0xbfbf0104u + timer * 8) = (get_SYS_HCLK() * 1000 / 2) * time;
    timer_Configure(timer, enable);
}

void time_polling_init(void)
{
    timerSet(1, 10, 1, 1, 0);
}

void pause_polling(u32 us)
{
    volatile u32 now, last, ticks, load;
    u32 wait = ((get_SYS_HCLK() * 500) / 1000) * us;

    load = REG32(0xbfbf010cu);
    ticks = 0;
    last = REG32(0xbfbf0110u);
    do {
        now = REG32(0xbfbf0110u);
        ticks += last >= now ? last - now : load + last - now;
        last = now;
    } while (ticks < wait);
}
