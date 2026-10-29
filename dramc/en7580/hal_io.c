/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en7580_ddr.h"
/* Virtual banks: DRAMC NAO, DRAMC AO, PHY byte lanes, PHY command. */
static const u32 reg_bases[] = {
	DRAMC_NAO_BASE_ADDRESS, DRAMC_AO_BASE_ADDRESS,
	DDRPHY_B01_BASE_ADDRESS, DDRPHY_CMD_BASE_ADDRESS,
};
void SaveCurrDramCtx(DRAMC_CTX_T *p) { pgCurrDramCtx = p; }
u32 u4RegBaseAddrTraslate(u32 reg)
{
	if (reg - 0x20000u < 0x40000u)
		return reg_bases[(reg - 0x20000u) >> 16] + (reg & 0x1fff);
	return reg;
}
u32 u4Dram_Register_Read(u32 reg) { return reg_read(u4RegBaseAddrTraslate(reg)); }
u8 ucDram_Register_Write(u32 reg, u32 value)
{
	reg_write(u4RegBaseAddrTraslate(reg), value);
	return 0;
}
void vIO32Write4BMsk(u32 reg, u32 value, u32 mask)
{
	ucDram_Register_Write(reg, (u4Dram_Register_Read(reg) & ~mask) | (value & mask));
}
void vIO32Write4B_All(u32 reg, u32 value)
{
	u32 count = 1;
	if ((reg & 0xf0000) >= 0x40000) { reg = (reg & 0xffff) + 0x40000; count = 2; }
	for (u32 bank = 0; bank < count; bank++)
		ucDram_Register_Write(reg + (bank << 16), value);
}
void vIO32Write4BMsk_All(u32 reg, u32 value, u32 mask)
{
	u32 count = 1;
	if ((reg & 0xf0000) >= 0x40000) { reg = (reg & 0xffff) + 0x40000; count = 2; }
	for (u32 bank = 0; bank < count; bank++) {
		u32 addr = reg + (bank << 16);
		/* Vendor broadcast API expects value already shifted/masked. */
		ucDram_Register_Write(addr, (u4Dram_Register_Read(addr) & ~mask) | value);
	}
}
