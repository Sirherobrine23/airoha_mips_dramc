/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en7580_ddr.h"
static u32 efuse_words[128], repair_table[64];
static u32 efuse_key1, efuse_key2;
static int init_flag, key_repair_cnt = 64;
static const u32 efuse_bases[] = { 0xbfbf8200, 0xbfaa0000 };
u32 efuse_disable(u32 bank)
{
	u32 base;
	if (bank >= ARRAY_SIZE(efuse_bases)) return 1;
	base = efuse_bases[bank];
	reg_write(base + 0x34, 0); reg_write(base + 0x8c, 0);
	return 0;
}
u32 get_row_col(int bit, int *column, int *row)
{
	if (bit < 0 || bit >= 2048) return 1;
	*column = bit & 31; *row = bit >> 5;
	return 0;
}
void efuse_clear_and_enable_int(int bank)
{
	u32 base = efuse_bases[bank];
	efuse_disable(bank); reg_write(base + 0x20, 3); reg_write(base + 0x18, 0x15f00104);
	efuse_key1 = 0xde7502bc; efuse_key2 = 0x78d39bf1;
}
u32 efuse_read_word(u32 bank, int row, u32 *word)
{
	u32 base;
	if (bank >= ARRAY_SIZE(efuse_bases)) return 1;
	if (row < 0 || row >= 64) return 2;
	base = efuse_bases[bank];
	reg_write(base + 0x34, efuse_key1); reg_write(base + 0x8c, efuse_key2);
	reg_write(base + 4, row); reg_write(base, 1); efuse_disable(bank);
	for (int timeout = 100; !(reg_read(base + 8) & 1); timeout--) {
		if (!timeout) return 3;
		pause_polling(1);
	}
	*word = reg_read(base + 0x14);
	return 0;
}
int read_efuse(int bank)
{
	if ((u32)bank >= ARRAY_SIZE(efuse_bases)) return 1;
	for (int row = 0; row < 64; row++) {
		int status = efuse_read_word(bank, row, &efuse_words[bank * 64 + row]);
		if (status) return status;
	}
	return 0;
}
static int efuse_init_cache(void)
{
	u8 *bytes = (u8 *)efuse_words;
	u32 config;
	int status, start;
	efuse_clear_and_enable_int(0); efuse_clear_and_enable_int(1);
	status = read_efuse(0);
	if (!status) status = read_efuse(1);
	if (!status) status = efuse_read_word(0, 0, &config);
	if (status) {
		prom_printf("Read EFUSE error, status:%d\n", status);
		key_repair_cnt = 64; return 4;
	}
	key_repair_cnt = (config & BIT(1)) ? 32 : 64;
	start = (2048 - key_repair_cnt * 12) >> 3;
	for (int i = 0; i < key_repair_cnt; i += 2) {
		u32 a = bytes[start], b = bytes[start + 1], c = bytes[start + 2];
		repair_table[i] = (((b >> 3) & 1) << 1) | (((b & 7) << 8 | a) << 2);
		repair_table[i + 1] = ((c >> 7) << 1) | (((c & 0x7f) << 4 | b >> 4) << 2);
		start += 3;
	}
	/* Preserve the object's bank-1 repair target and bank-0 extraction. */
	for (int i = 0; i < key_repair_cnt; i++) {
		u32 repair = repair_table[i];
		if (repair & BIT(1)) {
			u32 address = (repair >> 2) & 0x7ff;
			efuse_words[64 + (address >> 5)] ^= BIT(address & 31);
		}
	}
	init_flag = 1; return 0;
}
int efuse_read_data(int bit, int len, volatile void *buffer)
{
	volatile u8 *out = buffer;
	u32 bytes;
	if (bit < 0 || len < 0 || bit > 2048 || len > 2048 - bit || (!out && len)) return 2;
	if (!init_flag && efuse_init_cache()) return 4;
	bytes = ((u32)len + 7) >> 3;
	for (u32 i = 0; i < bytes; i++) out[i] = 0;
	for (int i = 0; i < len; i++) {
		u32 at = bit + i;
		out[i >> 3] |= ((efuse_words[at >> 5] >> (at & 31)) & 1) << (i & 7);
	}
	return 0;
}
