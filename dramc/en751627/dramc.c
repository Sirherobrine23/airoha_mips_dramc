/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en751627_ddr.h"

int PKG_type;
int dram_type;
int dram_size;

static int probe_pair(u32 base, u32 offset, u32 *value)
{
    u32 other;

    REG32(base) = 0x12345678;
    pause_polling(2);
    REG32(base + offset) = 0x87654321;
    pause_polling(2);
    do {
        other = REG32(base + offset);
        *value = REG32(base);
        if (other != 0x87654321)
            REG32(base + offset) = 0x87654321;
    } while (other != 0x87654321);
    return *value == 0x12345678 || *value == 0x87654321 ? 0 : -1;
}

int check_column_bank(void)
{
    u32 column, value;

    if (dram_type != DDR2)
        return 0;
    for (column = 8; column < 10; column++) {
        DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_CONF1) & ~0x300u) | ((column - 7) << 8), DRAMC_CONF1);
        pause_polling(2);
        if (probe_pair(DRAM_BASE_ADDR, BIT(column + 1), &value))
            return -1;
        if (value == 0x87654321)
            break;
    }
    DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_CONF1) & ~0x300u) | ((column - 8) << 8), DRAMC_CONF1);
    pause_polling(2);
    if (column != 10)
        return 0;
    REG32(0xbfb00074u) = (REG32(0xbfb00074u) & ~3u) | 1;
    pause_polling(2);
    DRAMC_WRITE_SET(BIT(24), DRAMC_GDDR3CTL1);
    pause_polling(2);
    if (probe_pair(DRAM_BASE_ADDR, 4u << (column + 1), &value))
        return -1;
    DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_GDDR3CTL1) & ~BIT(24)) |
                        (value == 0x12345678 ? BIT(24) : 0),
                    DRAMC_GDDR3CTL1);
    pause_polling(2);
    return 0;
}

void set_TRFC(void)
{
    static const u32 qfp_trfc[] = {0xa88d0481, 0xa8830481, 0xa8800481, 0xa8850481};
    static const u32 bga_trfc[] = {0xa8830481, 0xa88a0481, 0xa88b0481, 0xa8890481};
    static const u32 qfp_actim[] = {0x600, 0x610, 0x620, 0x640};
    static const u32 bga_actim[] = {0x610, 0x610, 0x620, 0x650};
    u32 index;

    for (index = 0; index < 4; index++) {
        if (dram_size != (64 << index))
            continue;
        DRAMC_WRITE_REG(PKG_type == QFP ? qfp_trfc[index] : bga_trfc[index], DRAMC_TEST2_3);
        DRAMC_WRITE_REG(PKG_type == QFP ? qfp_actim[index] : bga_actim[index], DRAMC_ACTIM1);
        break;
    }
    prom_puts("Set new TRFC.\n");
}

void dramc_reg_dump(void)
{
    u32 offset;

    for (offset = 0; offset < 0x650; offset += 4) {
        if (offset == 0x400)
            offset = 0x600;
        prom_puts("0x");
        prom_print_hex(offset, 3);
        prom_puts(": ");
        prom_print_hex(DRAMC_READ_REG(offset), 8);
        prom_puts("\n");
    }
}

int dramc_calib(void)
{
    int result = do_dqs_gw_calib_1();

    if (result < 0)
        return result;
    dle_factor_handler(8);
    result = do_sw_rx_dq_dqs_calib();
    if (result < 0)
        return result;
    result = do_dle_calib();
    if (result < 0)
        return result;
    return do_sw_tx_dq_dqs_calib();
}

int detect_dram_size_mib(void)
{
    u32 size, value;

    for (size = 32u << 20; size < (1024u << 20); size <<= 1) {
        if (probe_pair(DRAM_PROBE_BASE, size, &value)) {
            prom_puts("dram r/w error!\n");
            return 0;
        }
        if (value == 0x87654321) {
            prom_puts("DRAM size=");
            prom_print_dec(size >> 20);
            prom_puts("MB\n");
            return size >> 20;
        }
    }
    prom_puts("DRAM size=1GB\n");
    return 1024;
}

int main(void)
{
    u32 strap_reg, value, offset;
    int failed = 0;

    dram_type = DDR3;
    strap_reg = REG32(SYS_GLOBAL_PARM) & BIT(22) ? 0xbfa2ff28u : 0xbfa20174u;
    PKG_type = REG32(strap_reg) & BIT(15) ? QFP : BGA1;
    pause_polling(200);
    if (en7512_dramc_init() < 0)
        return 0;
    if (dramc_calib() < 0) {
        prom_puts("%dqs_gw (coarse/fine): ");
        prom_print_dec(opt_gw_coarse_value);
        prom_puts("/");
        prom_print_dec(opt_gw_fine_value);
        prom_puts("\n%DQS input dly:\n");
        prom_print_hex(DRAMC_READ_REG(DRAMC_R0DELDLY), 8);
        prom_puts("\n%DQ input dly:\n");
        for (offset = DRAMC_DQIDLY1; offset < DRAMC_DQIDLY1 + 16; offset += 4) {
            prom_print_hex(DRAMC_READ_REG(offset), 8);
            prom_puts("\n");
        }
        prom_puts("%dle: ");
        prom_print_dec(opt_dle_value);
        prom_puts("\n%DRAMC calibration fail\n\r");
        REG32(0xbfb00040u) |= BIT(31);
        failed = 1;
    }
    value = PKG_type == KGD ? REG32(EFUSE0_DATA0) : 0;
    if (PKG_type == KGD && (value & (value & BIT(6) ? BIT(26) : BIT(25))))
        DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_CONF1) & ~0x300u) | 0x100, DRAMC_CONF1);
    else
        check_column_bank();
    prom_puts("Calculate size.\n");
    dram_size = detect_dram_size_mib();
    REG32(SYS_GLOBAL_PARM) = (REG32(SYS_GLOBAL_PARM) & ~0xfffu) | (u32)dram_size;
    set_TRFC();
    value = DRAMC_READ_REG(DRAMC_MEMPLL6) & 0xff;
    if (value == 0xaa) {
        value = DRAMC_READ_REG(DRAMC_MEMPLL3) & 0xfff;
        if (value == 0x687)
            prom_puts("ddr-1066\n");
        else if (value == 0x8a9)
            prom_puts("ddr-1333\n");
    } else if (value == 0x8a)
        prom_puts("ddr-800\n");
    else if (value == 0xca)
        prom_puts("ddr-1200\n");
    else if (value == 0x4a)
        prom_puts("ddr-400\n");
    DRAMC_WRITE_SET(BIT(31), DRAMC_DQSCAL0);
    DRAMC_WRITE_SET(0x710, DRAMC_PERFCTL0);
    prom_puts("\r\n7516DRAMC V1.0 (");
    prom_print_dec(failed);
    prom_puts(")\r\n");
    return 0;
}
