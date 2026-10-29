/* SPDX-License-Identifier: GPL-2.0-or-later */
/* EN7580 implementation recovered from the preserved vendor objects. */
#include "en7580_ddr.h"

void v4bitPinmuxEnableVerify(DRAMC_CTX_T *p)
{
  return;
}

u32 DdrUpdateACTimingReg(void)
{
  return 0;
}

void Set_MRR_Pinmux_Mapping(DRAMC_CTX_T *p)
{
  return;
}

void Global_Option_Init(DRAMC_CTX_T *p)
{
  SaveCurrDramCtx(p);
  vSetRankNumber(p);
  return;
}

int SscEnable(DRAMC_CTX_T *p)
{
  if (p->ssc_en == '\x01') {
    prom_puts("Enable SSC\n");
    vIO32Write4BMsk(0x40018,4,4);
  }
  return 0;
}

void DDRPhyPLLSetting_EN7580(DRAMC_CTX_T *p)
{
  struct {
    uint local_30;
    uint local_2c;
    uint local_28;
  } state;
  uint uVar1;
  uint uVar2;
  uint uVar3;
  uint quotient;

  vIO32Write4BMsk(0x40d18,0,0xc000000);
  vIO32Write4BMsk(0x40c18,0,0xc000000);
  vIO32Write4BMsk(0x40c98,0,0xc000000);
  vIO32Write4BMsk(0x4000c,0x40000,0xd0000);
  vIO32Write4BMsk(0x40000,0,0x80000000);
  vIO32Write4BMsk(0x40004,0,0x80000000);
  vIO32Write4BMsk(0x400c0,0,0x4000000);
  vIO32Write4BMsk(0x500c0,0,0x4000000);
  vIO32Write4BMsk(0x500c0,0,2);
  vIO32Write4BMsk(0x400c0,0,2);
  vIO32Write4BMsk_All(0x40194,0,0x800);
  vIO32Write4BMsk(0x40094,0,0x800);
  vIO32Write4BMsk(0x40114,0,0x800);
  vIO32Write4BMsk_All(0x40180,0,8);
  vIO32Write4BMsk(0x40080,0,8);
  vIO32Write4BMsk(0x40100,0,8);
  vIO32Write4BMsk_All(0x40188,0x400,0x400);
  vIO32Write4BMsk(0x40088,0x20400,0x20400);
  vIO32Write4BMsk(0x40d80,1,0xffff);
  vIO32Write4BMsk(0x40da4,0,0x300);
  vIO32Write4BMsk(0x40dac,0,0x300);
  uVar1 = p->frequency;
  if (uVar1 == 0x640) {
    prom_puts("DDR1600 PLL setting init\n");
    vIO32Write4BMsk(0x40da8,1,0xc0007);
    vIO32Write4BMsk(0x40da0,1,0xc0007);
    vIO32Write4BMsk(0x40d98,0x10000,0x10000);
    vIO32Write4BMsk(0x40d90,0,0x10000);
    vIO32Write4BMsk(0x40d9c,0x40000000,0x7fffffff);
    vIO32Write4BMsk(0x40d94,0x40000000,0x7fffffff);
    vIO32Write4BMsk(0x40018,0x1a10002,0xffff0002);
    vIO32Write4BMsk(0x40010,0x1a10002,0xffff0002);
    ucDram_Register_Write(0x4001c,0x3250325);
    ucDram_Register_Write(0x40014,0x3250325);
    vIO32Write4BMsk(0x40dac,0x80,0x80);
    vIO32Write4BMsk(0x40da4,0x80,0x80);
  }
  else if (uVar1 == 0x74a) {
    prom_puts("DDR1866 PLL setting init\n");
    vIO32Write4BMsk(0x40da8,0,0xc0007);
    vIO32Write4BMsk(0x40da0,0,0xc0007);
    vIO32Write4BMsk(0x40d98,0x10000,0x10000);
    vIO32Write4BMsk(0x40d90,0,0x10000);
    vIO32Write4BMsk(0x40d9c,0x25000000,0x7fffffff);
    vIO32Write4BMsk(0x40d94,0x25000000,0x7fffffff);
    vIO32Write4BMsk(0x40018,0x1a10002,0xffff0002);
    vIO32Write4BMsk(0x40010,0x1a10002,0xffff0002);
    ucDram_Register_Write(0x4001c,0x3a303a3);
    ucDram_Register_Write(0x40014,0x3a303a3);
    vIO32Write4BMsk(0x40dac,0x80,0x80);
    vIO32Write4BMsk(0x40da4,0x80,0x80);
  }
  else {
    if (uVar1 != 0x4b0) {
      prom_puts("PLL defalut case.\n");
      uVar1 = u4Dram_Register_Read(0x40da8);
      state.local_2c = (uVar1 & 0xc0000) >> 0x12;
      uVar1 = u4Dram_Register_Read(0x40da8);
      uVar1 = uVar1 & 7;
      uVar2 = u4Dram_Register_Read(0x40dac);
      state.local_28 = (uVar2 & 0x80) >> 7;
      vIO32Write4BMsk(0x40d98,0x10000,0x10000);
      vIO32Write4BMsk(0x40d90,0,0x10000);
      uVar2 = p->frequency;
      if (uVar2 < 0x640) {
        p->frequency = uVar2 << 1;
      }
      uVar3 = p->frequency << uVar1;
      while (0xe94 < uVar3) {
        prom_puts("FVCO ");
        prom_print_dec(uVar3);
        prom_puts("MHz is too large! Try to adjust...\n");
        if (uVar1 == 0) {
          prom_puts("FVCO cannot adjust...\n");
          uVar1 = 0;
          break;
        }
        uVar1 = uVar1 - 1;
        uVar3 = p->frequency << (uVar1 & 0x1f);
      }
      do {
        if (0x67b < uVar3) goto LAB_9fa306e8;
        prom_puts("FVCO ");
        prom_print_dec(uVar3);
        prom_puts("MHz is too small! Try to adjust...\n");
        if (3 < uVar1) {
          prom_puts("FVCO cannot adjust...\n");
          goto LAB_9fa306e8;
        }
        uVar1 = uVar1 + 1;
        uVar3 = p->frequency << (uVar1 & 0x1f);
      } while( true );
    }
    vIO32Write4BMsk(0x40da8,0,0xc0007);
    vIO32Write4BMsk(0x40da0,0,0xc0007);
    vIO32Write4BMsk(0x40d98,0,0x10000);
    vIO32Write4BMsk(0x40d90,0,0x10000);
    vIO32Write4BMsk(0x40d9c,0x30000000,0x7fffffff);
    vIO32Write4BMsk(0x40d94,0x30000000,0x7fffffff);
    vIO32Write4BMsk(0x40018,0x1a10002,0xffff0002);
    vIO32Write4BMsk(0x40010,0x1a10002,0xffff0002);
    ucDram_Register_Write(0x4001c,0x1d201d2);
    ucDram_Register_Write(0x40014,0x1d201d2);
    vIO32Write4BMsk(0x40dac,0x80,0x80);
    vIO32Write4BMsk(0x40da4,0x80,0x80);
  }
  goto LAB_9fa30896;
LAB_9fa306e8:
  vIO32Write4BMsk(0x40da8,uVar1,7);
  if (0x19 >> state.local_2c == 0) {
    trap(7);
  }
  else {
    quotient = p->frequency / (uint)(0x19 >> state.local_2c);
  }
  state.local_30 = (quotient >> state.local_28) << (uVar1 & 0x1f);
  if (0x78 < state.local_30) {
    prom_puts("PCW = 0x");
    prom_print_hex(state.local_30,8);
    prom_puts(" is too large! Try to adjust...\n");
    do {
      if (state.local_2c == 0) {
        if (state.local_28 != 0) {
          prom_puts("Cannot adjust...\n");
          break;
        }
        state.local_28 = 1;
      }
      else {
        state.local_2c = state.local_2c - 1;
      }
      uVar3 = 0x19 >> (state.local_2c & 0x1f);
      if (uVar3 == 0) {
        trap(7);
      }
      else {
        quotient = p->frequency / uVar3;
      }
      state.local_30 = (quotient >> state.local_28) << (uVar1 & 0x1f);
    } while (0x78 < state.local_30);
    uVar3 = (state.local_2c & 3) << 0x12 | uVar1 & 7;
    vIO32Write4BMsk(0x40da8,uVar3,0xc0007);
    vIO32Write4BMsk(0x40da0,uVar3,0xc0007);
    vIO32Write4BMsk(0x40dac,state.local_28 << 7,0x80);
    vIO32Write4BMsk(0x40da4,state.local_28 << 7,0x80);
  }
  if (state.local_30 < 10) {
    prom_puts("PCW = 0x");
    prom_print_hex(state.local_30,8);
    prom_puts(" is too Small! Try to adjust...\n");
    do {
      if (state.local_2c < 2) {
        state.local_2c = state.local_2c + 1;
      }
      else {
        if (state.local_28 == 0) {
          prom_puts("Cannot adjust...\n");
          break;
        }
        state.local_28 = state.local_28 - 1;
      }
      uVar3 = 0x19 >> (state.local_2c & 0x1f);
      if (uVar3 == 0) {
        trap(7);
      }
      else {
        quotient = p->frequency / uVar3;
      }
      state.local_30 = (quotient >> (state.local_28 & 0x1f)) << (uVar1 & 0x1f);
    } while (state.local_30 < 10);
    uVar1 = (state.local_2c & 3) << 0x12 | uVar1 & 7;
    vIO32Write4BMsk(0x40da8,uVar1,0xc0007);
    vIO32Write4BMsk(0x40da0,uVar1,0xc0007);
    vIO32Write4BMsk(0x40dac,state.local_28 << 7,0x80);
    vIO32Write4BMsk(0x40da4,state.local_28 << 7,0x80);
  }
  vIO32Write4BMsk(0x40d9c,state.local_30 << 0x18,0x7fffffff);
  vIO32Write4BMsk(0x40d94,state.local_30 << 0x18,0x7fffffff);
  if (uVar2 < 0x640) {
    p->frequency = p->frequency >> 1;
  }
LAB_9fa30896:
  pause_polling(1);
  vIO32Write4BMsk(0x40d84,0,1);
  vIO32Write4BMsk_All(0x40180,4,5);
  vIO32Write4BMsk(0x40080,1,5);
  vIO32Write4BMsk(0x40100,1,5);
  vIO32Write4BMsk_All(0x40194,0x698600,0xc6fff70c);
  vIO32Write4BMsk(0x40094,0xc0778600,0xc6fff70c);
  vIO32Write4BMsk(0x40114,0xc0778600,0xc6fff70c);
  vIO32Write4BMsk(0x40088,0x880aec00,0x880aec00);
  vIO32Write4BMsk(0x40108,0x8808e800,0x8808e800);
  vIO32Write4BMsk(0x40188,0x880bac00,0x880bac00);
  vIO32Write4BMsk(0x40004,0x80000000,0x80000000);
  vIO32Write4BMsk(0x40000,0,0x80000000);
  pause_polling(0x1f);
  SscEnable(p);
  vIO32Write4BMsk(0x400ac,0x10000,0x10000);
  vIO32Write4BMsk(0x4012c,0x10000,0x10000);
  vIO32Write4BMsk(0x40c14,0xd,0x3f);
  vIO32Write4BMsk(0x40c94,0xd,0x3f);
  vIO32Write4BMsk(0x400ac,0xd00,0x3f00);
  vIO32Write4BMsk(0x4012c,0xd00,0x3f00);
  vIO32Write4BMsk(0x400ac,0,0x20000);
  vIO32Write4BMsk(0x4012c,0,0x20000);
  vIO32Write4BMsk(0x4000c,0x6d0000,0x6d0000);
  vIO32Write4BMsk_All(0x40180,8,8);
  vIO32Write4BMsk(0x40080,8,8);
  vIO32Write4BMsk(0x40100,8,8);
  pause_polling(1);
  if (p->frequency < 0x640) {
    vIO32Write4BMsk(0x400c0,0x8000fa0,0xc000ff9);
    vIO32Write4BMsk(0x400bc,0x20308700,0xff3fffff);
    vIO32Write4BMsk(0x40c18,0,0xf0000);
    vIO32Write4BMsk(0x40c98,0,0xf0000);
    vIO32Write4BMsk(0x500c0,0x8000fa0,0xc000ff9);
    vIO32Write4BMsk(0x500bc,0x20308700,0xff3fffff);
    vIO32Write4BMsk(0x40d18,0,0xf0000);
    vIO32Write4BMsk(0x400c4,0x8000,0x8000);
    vIO32Write4BMsk(0x400c8,0x20,0x20);
    vIO32Write4BMsk(0x500c0,0,2);
    vIO32Write4BMsk(0x400c0,0,2);
  }
  else {
    vIO32Write4BMsk(0x400c0,0x40000e0,0xc000ff9);
    vIO32Write4BMsk(0x400bc,0x8700,0xff3fffff);
    vIO32Write4BMsk(0x40c18,0,0xf0000);
    vIO32Write4BMsk(0x40c98,0,0xf0000);
    vIO32Write4BMsk(0x500c0,0x40000e0,0xc000ff9);
    vIO32Write4BMsk(0x500bc,0x8700,0xff3fffff);
    vIO32Write4BMsk(0x40d18,0,0xf0000);
    vIO32Write4BMsk(0x400c4,0x8000,0x8000);
    vIO32Write4BMsk(0x400c8,0x20,0x20);
    vIO32Write4BMsk(0x500c0,2,2);
    pause_polling(1);
    vIO32Write4BMsk(0x400c0,2,2);
  }
  vIO32Write4BMsk_All(0x40190,0,0xba800);
  vIO32Write4BMsk(0x40090,0,0xae800);
  vIO32Write4BMsk(0x40110,0,0xae800);
  vIO32Write4BMsk_All(0x40184,0x2ba800,0x702ba800);
  vIO32Write4BMsk(0x40084,0x2ae800,0x702ae83f);
  vIO32Write4BMsk(0x40104,0x2ae800,0x702ae83f);
  vIO32Write4BMsk_All(0x4018c,0xba000,0xba800);
  vIO32Write4BMsk(0x4008c,0x2e800,0xae800);
  vIO32Write4BMsk(0x4010c,0x2e800,0xae800);
  vIO32Write4BMsk(0x400c8,0x180,0x7fc0);
  vIO32Write4BMsk(0x400c4,0xe0000,0x1fff0000);
  vIO32Write4BMsk(0x500ac,0,6);
  vIO32Write4BMsk(0x400ac,0,6);
  vIO32Write4BMsk(0x40d10,0,0x3f3f3f);
  vIO32Write4BMsk(0x40c10,0,0x3f3f3f);
  vIO32Write4BMsk(0x40c90,0,0x3f3f3f);
  vIO32Write4BMsk_All(0x40180,0,0x30);
  vIO32Write4BMsk(0x40080,0,0x30);
  vIO32Write4BMsk(0x40100,0,0x30);
  vIO32Write4BMsk(0x400a4,0x40c,0x40e);
  vIO32Write4BMsk(0x40124,0x40c,0x40e);
  vIO32Write4BMsk(0x401b8,0x40101,0x41f1f);
  ucDram_Register_Write(0x40294,0x13400000);
  vIO32Write4BMsk(0x40270,0x50909,0xd0d0d);
  vIO32Write4BMsk(0x40da4,0,0x1b000);
  vIO32Write4BMsk(0x40dac,0,0x1b000);
  vIO32Write4BMsk(0x40d9c,0x80000000,0x80000000);
  vIO32Write4BMsk(0x40d94,0x80000000,0x80000000);
  pause_polling(1);
  vIO32Write4BMsk(0x40180,4,4);
  vIO32Write4BMsk(0x40080,1,1);
  vIO32Write4BMsk(0x40100,1,1);
  pause_polling(1);
  vIO32Write4BMsk_All(0x402a0,2,2);
  vIO32Write4BMsk(0x30000,2,2);
  vIO32Write4BMsk(0x40d18,0x8000000,0xc000000);
  vIO32Write4BMsk_All(0x40c18,0x8000000,0xc000000);
  vIO32Write4BMsk_All(0x40c98,0x8000000,0xc000000);
  pause_polling(1);
  vIO32Write4BMsk_All(0x40188,0,0x400);
  vIO32Write4BMsk_All(0x40088,0,0x20400);
  vIO32Write4BMsk_All(0x402a0,0,2);
  vIO32Write4BMsk(0x30000,0,2);
  vIO32Write4BMsk(0x40000,0,0x10000);
  vIO32Write4BMsk_All(0x40188,0,0x880bac00);
  vIO32Write4BMsk(0x40088,0,0x880aec00);
  vIO32Write4BMsk(0x40108,0,0x8808e800);
  pause_polling(1);
  vIO32Write4B_All(0x40284,0x21f);
  vIO32Write4BMsk_All(0x40194,0x800,0x8f0);
  pause_polling(1);
  vIO32Write4BMsk(0x40094,0x800,0x8f0);
  vIO32Write4BMsk(0x40114,0x800,0x8f0);
  pause_polling(1);
  return;
}

void TransferToSPMControl(DRAMC_CTX_T *p)
{
  return;
}

void Switch26MHzDisableDummyReadRefreshAllBank(DRAMC_CTX_T *p)
{
  return;
}

void EnableDramcPhyDCM(DRAMC_CTX_T *p, bool bEn)
{
  if ((bEn & 0xff) == 0) {
    vIO32Write4BMsk_All(0x30038,0x20,0xc0000027);
    vIO32Write4BMsk(0x402a8,0,0x400040);
    vIO32Write4BMsk(0x502a8,0,0x400000);
  }
  else {
    vIO32Write4BMsk_All(0x30038,0xc0000007,0xc0000027);
    vIO32Write4BMsk(0x402a8,0x400000,0x400040);
    vIO32Write4BMsk(0x502a8,0x400000,0x400000);
  }
  return;
}

void HwSaveForSR(DRAMC_CTX_T *p)
{
  return;
}

void DramcRunTimeConfig(DRAMC_CTX_T *p)
{
  vIO32Write4BMsk_All(0x30064,0x80000000,0x80000000);
  EnableDramcPhyDCM(p,1);
  vIO32Write4BMsk_All(0x30064,0x20000000,0x20000000);
  vIO32Write4BMsk_All(0x3004c,0,0x40000);
  return;
}

void DramcSetRankEngine2(DRAMC_CTX_T *p, U8 u1RankSel)
{
  if (u1RankSel >= RANK_MAX)
    return;
  vIO32Write4BMsk(0x30004,2,2);
  vIO32Write4BMsk(0x300a0,0,0x70000000);
  vIO32Write4BMsk(0x300a0,u1RankSel << 0x18,0x3000000);
  return;
}

U32 DramcEngine1(DRAMC_CTX_T *p, U32 test2_1, U32 test2_2, S16 loopforever, U8 period)
{
  return 0;
}

DRAM_STATUS_T DramcEngine2Init(DRAMC_CTX_T *p, U32 test2_1, U32 test2_2, U8 testaudpat, U8 log2loopcount)
{
  int iVar1;

  iVar1 = 1;
  if ((p != (DRAMC_CTX_T *)0x0) && ((log2loopcount & 0xff) < 0x10)) {
    uiReg0D0h = u4Dram_Register_Read(0x300d0);
    vIO32Write4BMsk(0x300d0,0,0xf000c0);
    vIO32Write4BMsk(0x20030,0,0x1000);
    vIO32Write4BMsk(0x3009c,0,0xe0000000);
    vIO32Write4BMsk(0x30090,(test2_1 >> 0x18) << 8 | test2_2 >> 0x18,0xffff);
    ucDram_Register_Write(0x30094,(test2_1 & 0xfffff) << 4);
    ucDram_Register_Write(0x30098,(test2_2 & 0xfffff) << 4);
    if ((testaudpat & 0xff) == 2) {
      vIO32Write4BMsk(0x300a0,0,0x20000);
      vIO32Write4BMsk(0x3000c,0,0x10);
      vIO32Write4BMsk(0x3009c,log2loopcount & 0xf,0x8f);
      vIO32Write4BMsk(0x300a0,0x10000,0x1c000);
      vIO32Write4BMsk(0x300a0,0,0xc0);
      iVar1 = 0;
    }
    else if ((testaudpat & 0xff) == 1) {
      vIO32Write4BMsk(0x300a0,0xd10d,0x1df1f);
      vIO32Write4BMsk(0x3009c,log2loopcount & 0xf | 0x80,0x8f);
      iVar1 = 0;
    }
    else {
      vIO32Write4BMsk(0x3009c,log2loopcount & 0xf,0x8f);
      vIO32Write4BMsk(0x300a0,0,0x10000);
      iVar1 = 0;
    }
  }
  return iVar1;
}

U32 DramcEngine2Run(DRAMC_CTX_T *p, DRAM_TE_OP_T wr, U8 testaudpat)
{
  uint uVar1;
  uint uVar2;
  uint uVar3;

  if (wr == 1) {
    if (((testaudpat & 0xff) - 1 & 0xff) < 2) {
      vIO32Write4BMsk(0x300a0,0,0x8000);
    }
    vIO32Write4BMsk(0x3009c,0x40000000,0xe0000000);
    uVar2 = 0;
  }
  else {
    if (wr == 0) {
      vIO32Write4BMsk(0x3009c,0x80000000,0xe0000000);
      uVar2 = 0;
      do {
        uVar1 = u4Dram_Register_Read(0x20120);
        if ((uVar1 & 1) != 0) break;
        pause_polling(1);
        uVar1 = uVar2 + 1;
        uVar3 = uVar2 - 3;
        uVar2 = uVar1;
      } while ((uVar3 < 0xfffd) || (uVar1 < 0x10001));
      vIO32Write4BMsk(0x3009c,0,0xe0000000);
      pause_polling(1);
      vIO32Write4BMsk(0x3009c,0x40000000,0xe0000000);
    }
    uVar2 = 0;
  }
  do {
    uVar1 = u4Dram_Register_Read(0x20120);
    if ((uVar1 & 1) != 0) break;
    pause_polling(1);
    uVar1 = uVar2 + 1;
    uVar3 = uVar2 - 3;
    uVar2 = uVar1;
  } while ((uVar3 < 0xfffd) || (uVar1 < 0x10001));
  pause_polling(1);
  uVar2 = u4Dram_Register_Read(0x20124);
  vIO32Write4BMsk(0x3009c,0,0xf0000000);
  return uVar2;
}

void DramcEngine2End(DRAMC_CTX_T *p)
{
  ucDram_Register_Write(0x300d0,uiReg0D0h);
  return;
}

U32 DramcEngine2(DRAMC_CTX_T *p, DRAM_TE_OP_T wr, U32 test2_1, U32 test2_2, U8 testaudpat, S16 loopforever, U8 period, U8 log2loopcount)
{
  uint val;
  uint uVar1;
  uint uVar2;
  uint uVar3;

  uVar1 = testaudpat & 0xff;
  uVar2 = period & 0xff;
  if (p == (DRAMC_CTX_T *)0x0) {
    uVar1 = 0xffffffff;
  }
  else if ((log2loopcount & 0xff) < 0x10) {
    DramcSetRankEngine2(p,(uint)*(byte *)&p->rank);
    val = u4Dram_Register_Read(0x300d0);
    vIO32Write4BMsk(0x300d0,0,0xf000c0);
    vIO32Write4BMsk(0x20030,0,0x1000);
    vIO32Write4BMsk(0x3009c,0,0xe0000000);
    if ((short)loopforever < 0) {
      if ((short)loopforever == -1) {
        if (uVar2 == 0) {
          return 0xffffffff;
        }
      }
      else if ((short)loopforever == -2) {
        if (uVar2 == 0) {
          return 0xffffffff;
        }
      }
      else {
        if ((short)loopforever != -3) {
          return 0xffffffff;
        }
        if (uVar2 == 0) {
          return 0xffffffff;
        }
      }
    }
    vIO32Write4BMsk(0x30090,test2_2 >> 0x18 | (test2_1 >> 0x18) << 8,0xffff);
    ucDram_Register_Write(0x30094,(test2_1 & 0xfffff) << 4);
    ucDram_Register_Write(0x30098,(test2_2 & 0xfffff) << 4);
    if (uVar1 == 2) {
      vIO32Write4BMsk(0x300a0,0,0x20000);
      vIO32Write4BMsk(0x3000c,0,0x10);
      vIO32Write4BMsk(0x3009c,log2loopcount & 0xf,0x8f);
      vIO32Write4BMsk(0x300a0,0x10000,0x1c000);
      vIO32Write4BMsk(0x300a0,0,0xc0);
    }
    else if (uVar1 == 1) {
      vIO32Write4BMsk(0x300a0,0xd10d,0x1df1f);
      vIO32Write4BMsk(0x3009c,log2loopcount & 0xf | 0x80,0x8f);
    }
    else {
      vIO32Write4BMsk(0x3009c,log2loopcount & 0xf,0x8f);
      vIO32Write4BMsk(0x300a0,0,0x10000);
    }
    if (wr == 1) {
      if ((uVar1 - 1 & 0xff) < 2) {
        vIO32Write4BMsk(0x300a0,0,0x8000);
      }
      vIO32Write4BMsk(0x3009c,0x40000000,0xe0000000);
      uVar1 = 0;
    }
    else {
      if (wr == 0) {
        vIO32Write4BMsk(0x3009c,0x80000000,0xe0000000);
        uVar1 = 0;
        do {
          uVar2 = u4Dram_Register_Read(0x20120);
          if ((uVar2 & 1) != 0) break;
          pause_polling(1);
          uVar2 = uVar1 + 1;
          uVar3 = uVar1 - 3;
          uVar1 = uVar2;
        } while ((uVar3 < 0xfffd) || (uVar2 < 0x10001));
        vIO32Write4BMsk(0x3009c,0,0xe0000000);
        pause_polling(1);
        vIO32Write4BMsk(0x3009c,0x40000000,0xe0000000);
      }
      uVar1 = 0;
    }
    do {
      uVar2 = u4Dram_Register_Read(0x20120);
      if ((uVar2 & 1) != 0) break;
      pause_polling(1);
      uVar2 = uVar1 + 1;
      uVar3 = uVar1 - 3;
      uVar1 = uVar2;
    } while ((uVar3 < 0xfffd) || (uVar2 < 0x10001));
    pause_polling(1);
    uVar1 = u4Dram_Register_Read(0x20124);
    vIO32Write4BMsk(0x3009c,0,0xe0000000);
    ucDram_Register_Write(0x300d0,val);
  }
  else {
    uVar1 = 0xffffffff;
  }
  return uVar1;
}

U32 TestEngineCompare(DRAMC_CTX_T *p)
{
  uint uVar1;
  uint uVar2;

  uVar1 = p->test_pattern;
  if (uVar1 == 1) {
    if (p->fglow_freq_write_en == '\x01') {
      uVar1 = DramcEngine2(p,1,p->test2_1,p->test2_2,1,0,0,0);
    }
    else {
      uVar1 = DramcEngine2(p,0,p->test2_1,p->test2_2,1,0,0,0);
    }
  }
  else if (uVar1 == 0) {
    uVar1 = DramcEngine2(p,0,p->test2_1,p->test2_2,0,0,0,0);
  }
  else if (uVar1 == 5) {
    if (p->fglow_freq_write_en == '\x01') {
      uVar1 = DramcEngine2(p,1,p->test2_1,p->test2_2,2,0,0,0);
    }
    else {
      uVar1 = DramcEngine2(p,0,p->test2_1,p->test2_2,2,0,0,0);
    }
  }
  else if (uVar1 == 6) {
    uVar2 = DramcEngine2(p,0,p->test2_1,p->test2_2,1,0,0,0);
    uVar1 = DramcEngine2(p,0,p->test2_1,p->test2_2,2,0,0,0);
    uVar1 = uVar1 | uVar2;
  }
  else {
    uVar1 = DramcEngine2(p,0,p->test2_1,p->test2_2,1,0,0,0);
  }
  return uVar1;
}

void DramcModeRegWrite_PC3(DRAMC_CTX_T *p, U8 u1MRIdx, U32 u4Value)
{
  uint uVar1;

  vIO32Write4BMsk(0x3005c,(u1MRIdx & 0xff) << 0x15,0xe00000);
  vIO32Write4BMsk(0x3005c,u4Value << 8,0x1fff00);
  vIO32Write4BMsk(0x30060,1,1);
  prom_puts("Fire MRW command...\n");
  while (uVar1 = u4Dram_Register_Read(0x20088), (uVar1 & 1) == 0) {
    pause_polling(1);
  }
  prom_puts("ModeReg.");
  prom_print_dec(u1MRIdx & 0xff);
  prom_puts(", value.0x");
  prom_print_hex(u4Value,8);
  prom_puts(" done\n");
  vIO32Write4BMsk(0x30060,0,1);
  return;
}

void DramcModeRegWrite_PC4(DRAMC_CTX_T *p, U8 u1BGIdx, U8 u1MRIdx, U32 u4Value)
{
  uint uVar1;

  vIO32Write4BMsk(0x3005c,u1BGIdx << 0x1e,0xc0000000);
  vIO32Write4BMsk(0x3005c,(u1MRIdx & 0xff) << 0x15,0xe00000);
  vIO32Write4BMsk(0x3005c,u4Value << 8,0x1fff00);
  vIO32Write4BMsk(0x30060,1,1);
  while (uVar1 = u4Dram_Register_Read(0x20088), (uVar1 & 1) == 0) {
    pause_polling(1);
  }
  vIO32Write4BMsk(0x30060,0,1);
  return;
}

DRAM_STATUS_T DramcInit(DRAMC_CTX_T *p)
{
  uint uVar1;
  const ushort *puVar2;
  int iVar3;

  pause_polling(1);
  vIO32Write4BMsk(0x30000,0x80000000,0x80000000);
  pause_polling(1);
  vIO32Write4BMsk(0x30000,0,0x80000000);
  vIO32Write4BMsk(0x30040,0x3000000c,0x3000003f);
  vIO32Write4BMsk(0x401ac,0xe00,0x3f00);
  vIO32Write4BMsk(0x501ac,0xe00,0x3f00);
  vIO32Write4BMsk(0x40d14,0xe,0x3f3f);
  ucDram_Register_Write(0x40278,0);
  ucDram_Register_Write(0x40274,0xffffffff);
  ucDram_Register_Write(0x4027c,0xffffffff);
  ucDram_Register_Write(0x4028c,0x6003bf);
  ucDram_Register_Write(0x40294,0x13300000);
  ucDram_Register_Write(0x5028c,0x6003bf);
  ucDram_Register_Write(0x50294,0x222a2a00);
  if (p->dram_type == EN7580_TYPE_PCDDR4) {
    vIO32Write4BMsk(0x40d1c,0x10001,0xffff000f);
  }
  else if (p->dram_type == EN7580_TYPE_PCDDR3) {
    vIO32Write4BMsk(0x40d1c,1,0xffff000f);
  }
  vIO32Write4BMsk(0x40c1c,0x21,0x2f);
  vIO32Write4BMsk(0x40c9c,1,0xf);
  ucDram_Register_Write(0x405e4,7);
  ucDram_Register_Write(0x505e4,7);
  vIO32Write4BMsk(0x40008,0,1);
  vIO32Write4BMsk(0x40264,0x92920000,0xbfff0000);
  vIO32Write4BMsk(0x40268,0x120040,0x3f0060);
  vIO32Write4BMsk(0x40c14,0x500000,0x703f00);
  vIO32Write4BMsk(0x40c94,0x500000,0x703f00);
  pause_polling(1);
  vIO32Write4BMsk(0x400a4,0x10,0x10);
  vIO32Write4BMsk(0x40124,0x10,0x10);
  ucDram_Register_Write(0x40288,0x3f600);
  ucDram_Register_Write(0x50288,0xff7befdf);
  vIO32Write4BMsk(0x400a8,0x1010,0x7f7f);
  vIO32Write4BMsk(0x400ac,0x100e00,0x1f03f00);
  vIO32Write4BMsk(0x400b0,0x1001080,0x100d080);
  vIO32Write4BMsk(0x400ac,0x2000000,0x2000000);
  vIO32Write4BMsk(0x40128,0x1010,0x7f7f);
  vIO32Write4BMsk(0x4012c,0x100e00,0x1f03f00);
  vIO32Write4BMsk(0x40130,0x1001080,0x100d080);
  vIO32Write4BMsk(0x4012c,0x2000000,0x2000000);
  vIO32Write4BMsk(0x400a4,8,8);
  vIO32Write4BMsk(0x40124,8,8);
  vIO32Write4BMsk(0x401a4,8,8);
  vIO32Write4BMsk(0x401b8,0x80101,0x81f1f);
  vIO32Write4BMsk(0x4026c,0,0xff40);
  ucDram_Register_Write(0x40294,0x13400000);
  vIO32Write4BMsk(0x40270,0xfff0f0f0,0xfff0f0f0);
  DDRPhyPLLSetting_EN7580(p);
  vIO32Write4BMsk(0x400b4,0,0xff);
  vIO32Write4BMsk(0x40134,0,0xff);
  if (p->dram_type == EN7580_TYPE_PCDDR3) {
    vIO32Write4BMsk(0x40c04,0x5050c0c,0x1f1f1f1f);
    vIO32Write4BMsk(0x40c08,0x5050c0c,0x1f1f1f1f);
    vIO32Write4BMsk(0x40c84,0x5050c0c,0x1f1f1f1f);
    vIO32Write4BMsk(0x40c88,0x5050c0c,0x1f1f1f1f);
  }
  else if (p->dram_type == EN7580_TYPE_PCDDR4) {
    vIO32Write4BMsk(0x40c04,0xa0e0e,0x1f1f1f1f);
    vIO32Write4BMsk(0x40c08,0xa0e0e,0x1f1f1f1f);
    vIO32Write4BMsk(0x40c84,0xa0e0e,0x1f1f1f1f);
    vIO32Write4BMsk(0x40c88,0xa0e0e,0x1f1f1f1f);
  }
  vIO32Write4BMsk(0x400a4,4,4);
  vIO32Write4BMsk(0x400b0,0x10,0x1c);
  vIO32Write4BMsk(0x40124,4,4);
  vIO32Write4BMsk(0x40130,0x10,0x1c);
  vIO32Write4BMsk(0x400a0,0,0x33c000);
  vIO32Write4BMsk(0x40120,0,0x33c000);
  vIO32Write4BMsk_All(0x401a4,4,4);
  vIO32Write4BMsk_All(0x401b0,0x10,0x14);
  vIO32Write4BMsk(0x401a0,0,0x370000);
  vIO32Write4BMsk(0x401b4,0,0xfcf);
  if (p->dram_type == EN7580_TYPE_PCDDR3) {
    vIO32Write4BMsk(0x40d08,0xc0c,0x1f1f);
    vIO32Write4BMsk(0x40d04,0xc0c,0x1f1f);
  }
  else if (p->dram_type == EN7580_TYPE_PCDDR4) {
    vIO32Write4BMsk(0x40d08,0xe0e,0x1f1f);
    vIO32Write4BMsk(0x40d04,0xe0e,0x1f1f);
  }
  vIO32Write4BMsk(0x400a4,0x4e0,0x10084e2);
  vIO32Write4BMsk(0x40124,0x4e0,0x10084e2);
  if (p->dram_type == EN7580_TYPE_PCDDR3) {
    vIO32Write4BMsk(0x400c4,0x1000,0x7000);
    vIO32Write4BMsk(0x400a4,0,0x2000200);
    vIO32Write4BMsk(0x400b0,0,0x20);
    vIO32Write4BMsk(0x40130,0,0x20);
    vIO32Write4BMsk(0x40124,0,0x2000200);
    vIO32Write4BMsk_All(0x401b0,0,0x20);
  }
  else {
    vIO32Write4BMsk(0x400c4,0x2000,0x7000);
    vIO32Write4BMsk(0x400a4,0x2000200,0x2000200);
    vIO32Write4BMsk(0x400b0,0x20,0x20);
    vIO32Write4BMsk(0x40130,0x20,0x20);
    vIO32Write4BMsk(0x40124,0x2000200,0x2000200);
    vIO32Write4BMsk_All(0x401b0,0x20,0x20);
  }
  vIO32Write4BMsk(0x40c14,0,0x70000);
  vIO32Write4BMsk(0x40c94,0,0x70000);
  vIO32Write4BMsk(0x400c8,0x20,0x20);
  vIO32Write4BMsk(0x400c8,0,0x10000);
  vIO32Write4BMsk_All(0x401a4,0,2);
  vIO32Write4BMsk(0x401b8,0,0x10000);
  vIO32Write4BMsk(0x400b0,0x240,0x80000243);
  vIO32Write4BMsk(0x40130,0x240,0x80000243);
  vIO32Write4BMsk_All(0x401b0,0x240,0x80000243);
  vIO32Write4BMsk(0x400b8,0,0x10);
  vIO32Write4BMsk(0x40138,0,0x10);
  vIO32Write4BMsk(0x400ac,0x80,0x80);
  vIO32Write4BMsk(0x4012c,0x80,0x80);
  vIO32Write4BMsk(0x4012c,0x80,0x80);
  vIO32Write4BMsk(0x400ac,0x80,0x80);
  vIO32Write4BMsk(0x400c4,0x200,0xf30);
  vIO32Write4BMsk(0x40c14,0,0x40);
  vIO32Write4BMsk(0x40c94,0,0x40);
  vIO32Write4BMsk(0x40d14,0,0x40);
  if (p->dram_type == EN7580_TYPE_PCDDR3) {
    vIO32Write4BMsk(0x40c14,0x12,0x3f);
    vIO32Write4BMsk(0x40c94,0x12,0x3f);
    vIO32Write4BMsk(0x400b0,0x40000,0x50000);
    vIO32Write4BMsk(0x40130,0x40000,0x50000);
  }
  else {
    vIO32Write4BMsk(0x40c14,0x13,0x3f);
    vIO32Write4BMsk(0x40c94,0x13,0x3f);
    vIO32Write4BMsk(0x400b0,0x10000,0x50000);
    vIO32Write4BMsk(0x40130,0x10000,0x50000);
  }
  vIO32Write4BMsk(0x400ac,0x10000,0x10000);
  vIO32Write4BMsk(0x4012c,0x10000,0x10000);
  vIO32Write4BMsk(0x40264,0x924000,0xff6000);
  vIO32Write4BMsk(0x40264,0x80008000,0x80008000);
  vIO32Write4BMsk(0x400a4,0x100,0x100);
  vIO32Write4BMsk(0x40124,0x100,0x100);
  vIO32Write4BMsk(0x402a0,0x8100008c,0x8100008c);
  vIO32Write4BMsk(0x502a0,0x100008c,0x8100008c);
  vIO32Write4BMsk(0x401b8,0,0x100000);
  vIO32Write4BMsk(0x401b4,0,0x40);
  if (p->en_4bitMux != '\x01') {
    vIO32Write4BMsk(0x400a4,0,0xff0000);
    vIO32Write4BMsk(0x40124,0,0xff0000);
    vIO32Write4BMsk(0x400a0,0,0x1ff);
    vIO32Write4BMsk(0x40120,0x1ff,0x1ff);
  }
  if (p->dram_type == EN7580_TYPE_PCDDR4) {
    vIO32Write4BMsk(0x400a4,0x80000000,0x80000000);
    vIO32Write4BMsk(0x40124,0x80000000,0x80000000);
    vIO32Write4BMsk(0x400b0,0x20000,0xa0000);
    vIO32Write4BMsk(0x40130,0x20000,0xa0000);
  }
  else {
    vIO32Write4BMsk(0x400a4,0,0x80000000);
    vIO32Write4BMsk(0x40124,0,0x80000000);
    vIO32Write4BMsk(0x400b0,0x80000,0xa0000);
    vIO32Write4BMsk(0x40130,0x80000,0xa0000);
    vIO32Write4BMsk(0x401b0,0x40008,0x50008);
  }
  vIO32Write4BMsk(0x400a0,0,0x40000);
  vIO32Write4BMsk(0x40120,0,0x40000);
  vIO32Write4BMsk(0x308c8,0x7c0,0x7c0);
  vIO32Write4BMsk(0x3003c,0xffff,0xffff);
  vIO32Write4BMsk(0x30064,0x10000000,0x10000000);
  vIO32Write4BMsk(0x3000c,0,0x4000000);
  vIO32Write4BMsk(0x3000c,0x80000,0x82000);
  uVar1 = p->pinmux;
  if (uVar1 == 1) {
    vIO32Write4BMsk_All(0x402a0,0x100,0x300);
  }
  else if (uVar1 == 0) {
    vIO32Write4BMsk_All(0x402a0,0,0x300);
  }
  else if (uVar1 == 2) {
    vIO32Write4BMsk_All(0x402a0,0x200,0x300);
  }
  else if (uVar1 == 3) {
    vIO32Write4BMsk_All(0x402a0,0x300,0x300);
  }
  if (p->dram_type != EN7580_TYPE_PCDDR3) {
    if (p->dram_type != EN7580_TYPE_PCDDR4) {
      prom_puts("Not support this DDR type.\n");
      return 1;
    }
    vIO32Write4BMsk(0x40e08,0x8000800,0x3f3f3f3f);
    vIO32Write4BMsk(0x40e0c,0x8000800,0x3f3f3f3f);
    vIO32Write4BMsk(0x40e10,0x8000800,0x3f3f3f3f);
    vIO32Write4BMsk(0x40e14,0x8000800,0x3f3f3f3f);
    vIO32Write4BMsk(0x40e18,0x120a0800,0x7f7f3f3f);
    vIO32Write4BMsk(0x40e58,0x8000800,0x3f3f3f3f);
    vIO32Write4BMsk(0x40e5c,0x8000800,0x3f3f3f3f);
    vIO32Write4BMsk(0x40e60,0x8000800,0x3f3f3f3f);
    vIO32Write4BMsk(0x40e64,0x8000800,0x3f3f3f3f);
    vIO32Write4BMsk(0x40e68,0x120a0800,0x7f7f3f3f);
    ucDram_Register_Write(0x40e00,0x22222222);
    vIO32Write4BMsk(0x40e04,0,0xffff000f);
    ucDram_Register_Write(0x40f50,0x22222222);
    vIO32Write4BMsk(0x40e54,0,0xffff000f);
    vIO32Write4BMsk(0x40e1c,0xf0f00,0x3f3f3f00);
    vIO32Write4BMsk(0x40e6c,0xf0f00,0x3f3f3f00);
    vIO32Write4BMsk(0x402a0,0x8000008c,0xbefff4ff);
    vIO32Write4BMsk(0x30c54,0x200600,0xf33fffff);
    vIO32Write4BMsk_All(0x402a8,0x11351131,0xf37ff37f);
    vIO32Write4BMsk(0x300dc,0x1d007,0x33fff1bf);
    vIO32Write4BMsk(0x30210,0xe090601,0x3f3f3f01);
    vIO32Write4BMsk_All(0x4029c,0x3101020f,0xf7fdfbff);
    vIO32Write4BMsk(0x30840,0xa20810bf,0xfb0ff2ff);
    vIO32Write4BMsk(0x30860,p->odt_onoff & 1 | 0xc0070050,0xc87f00f3);
    vIO32Write4BMsk(0x3004c,0x25712000,0xfff77de7);
    vIO32Write4BMsk(0x30880,0,0x77777777);
    vIO32Write4BMsk(0x30884,0,0x7070777);
    vIO32Write4BMsk(0x30888,0,0x77777777);
    vIO32Write4BMsk(0x3088c,0,0x77777777);
    vIO32Write4BMsk(0x30890,0x10000001,0x77777777);
    vIO32Write4BMsk(0x30894,0,0x7070777);
    vIO32Write4BMsk(0x30898,0,0x77777777);
    vIO32Write4BMsk(0x3089c,0,0x77777777);
    vIO32Write4BMsk(0x308a0,0x22222222,0x77777777);
    vIO32Write4BMsk(0x308a4,0x11113333,0x77777777);
    vIO32Write4BMsk(0x30a2c,0x22222222,0x77777777);
    vIO32Write4BMsk(0x30a30,0x22222222,0x77777777);
    vIO32Write4BMsk(0x30a34,0x11113333,0x77777777);
    vIO32Write4BMsk(0x30a38,0x11113333,0x77777777);
    ucDram_Register_Write(0x30200,0xf0100000);
    vIO32Write4BMsk(0x30048,0x8000000,0xcf710000);
    vIO32Write4BMsk(0x30878,0xc0000000,0xfc000777);
    vIO32Write4BMsk(0x300d4,0x110,0x37a3f33);
    vIO32Write4BMsk(0x30050,0x30000700,0xf0000733);
    vIO32Write4BMsk(0x30054,0x6543b321,0xf777f777);
    ucDram_Register_Write(0x30000,0);
    if (p->data_width == 0x10) {
      vIO32Write4BMsk(0x30000,0x40012000,0x40012000);
    }
    else {
      vIO32Write4BMsk(0x30000,0x40112000,0x40112000);
    }
    vIO32Write4BMsk(0x30004,0x2001,0xf7df7ff);
    vIO32Write4BMsk(0x30008,0x81080000,0x8feb0001);
    vIO32Write4BMsk(0x3000c,0x26e02,0xcdfefffb);
    vIO32Write4BMsk(0x30010,0x80,0x3ff);
    vIO32Write4BMsk(0x30020,9,0x70ff110b);
    vIO32Write4BMsk(0x30038,0x80000106,0xcc3805f7);
    vIO32Write4BMsk(0x30040,0x3000000c,0x3000003f);
    vIO32Write4BMsk(0x3004c,0x25714001,0xfff77de7);
    vIO32Write4BMsk(0x30064,0x24000080,0xfe0808e0);
    vIO32Write4BMsk(0x300b0,0x4300000,0xfff02066);
    vIO32Write4BMsk(0x30200,0x200000,0x200000);
    vIO32Write4BMsk(0x30858,0x11100001,0xfffff173);
    vIO32Write4BMsk(0x30a04,0x10101010,0x7f7f7f7f);
    vIO32Write4BMsk(0x30a24,0x10101010,0x77777777);
    ucDram_Register_Write(0x30a28,0x6060606);
    vIO32Write4BMsk(0x30a28,0x10101010,0x77777777);
    vIO32Write4BMsk(0x30880,0,0x77777777);
    vIO32Write4BMsk(0x30884,0,0x7070777);
    vIO32Write4BMsk(0x30888,0,0x77777777);
    vIO32Write4BMsk(0x3088c,0,0x77777777);
    vIO32Write4BMsk(0x30890,0x10000111,0x77777777);
    vIO32Write4BMsk(0x30894,0x1000000,0x7070777);
    vIO32Write4BMsk(0x30898,0,0x77777777);
    vIO32Write4BMsk(0x3089c,0,0x77777777);
    vIO32Write4BMsk(0x40ec4,0xa00,0x3f003f3f);
    vIO32Write4BMsk(0x40fc4,0,0x3f003f3f);
    if (p->frequency == 0x640) {
      vIO32Write4BMsk(0x308a0,0x22222222,0x77777777);
      vIO32Write4BMsk(0x308a4,0x11113333,0x77777777);
      vIO32Write4BMsk(0x30a2c,0x22222222,0x77777777);
      vIO32Write4BMsk(0x30a30,0x22222222,0x77777777);
      vIO32Write4BMsk(0x30a34,0x11113333,0x77777777);
      vIO32Write4BMsk(0x30a38,0x11113333,0x77777777);
    }
    else if (p->frequency == 0x74a) {
      vIO32Write4BMsk(0x308a0,0x22223333,0x77777777);
      vIO32Write4BMsk(0x308a4,0x33331111,0x77777777);
      vIO32Write4BMsk(0x30a2c,0x22223333,0x77777777);
      vIO32Write4BMsk(0x30a30,0x22223333,0x77777777);
      vIO32Write4BMsk(0x30a34,0x33331111,0x77777777);
      vIO32Write4BMsk(0x30a38,0x33331111,0x77777777);
    }
    vIO32Write4BMsk(0x30b2c,0x22222222,0x77777777);
    vIO32Write4BMsk(0x30b30,0x22222222,0x77777777);
    vIO32Write4BMsk(0x30b34,0x11113333,0x77777777);
    vIO32Write4BMsk(0x30b38,0x33331111,0x77777777);
    vIO32Write4BMsk(0x40e1c,0xf0f00,0x3f3f3f00);
    vIO32Write4BMsk(0x40f1c,0xf0f00,0x3f3f3f00);
    vIO32Write4BMsk(0x40e6c,0xf0f00,0x3f3f3f00);
    vIO32Write4BMsk(0x40f6c,0xf0f00,0x3f3f3f00);
    vIO32Write4BMsk(0x30a1c,0x1111,0x77777777);
    vIO32Write4BMsk(0x30a20,0x3333,0x77777777);
    vIO32Write4BMsk(0x30844,0x30000d0f,0x7c001f1f);
    vIO32Write4BMsk(0x30a24,0x10101010,0x77777777);
    vIO32Write4BMsk(0x30a28,0x6060606,0x77777777);
    vIO32Write4BMsk(0x30b24,0x10101010,0x77777777);
    vIO32Write4BMsk(0x30b28,0x6060606,0x77777777);
    vIO32Write4BMsk(0x40e08,0xf0f0f0f,0x3f3f3f3f);
    vIO32Write4BMsk(0x40e0c,0xf0f0f0f,0x3f3f3f3f);
    vIO32Write4BMsk(0x40e10,0xf0f0f0f,0x3f3f3f3f);
    vIO32Write4BMsk(0x40e14,0xf0f0f0f,0x3f3f3f3f);
    vIO32Write4BMsk(0x40e18,0,0x7f7f3f3f);
    vIO32Write4BMsk(0x40f08,0xf0f0f0f,0x3f3f3f3f);
    vIO32Write4BMsk(0x40f0c,0xf0f0f0f,0x3f3f3f3f);
    vIO32Write4BMsk(0x40f10,0xf0f0f0f,0x3f3f3f3f);
    vIO32Write4BMsk(0x40f14,0xf0f0f0f,0x3f3f3f3f);
    vIO32Write4BMsk(0x40f18,0,0x7f7f3f3f);
    vIO32Write4BMsk(0x40e58,0xf0f0f0f,0x3f3f3f3f);
    vIO32Write4BMsk(0x40e5c,0xf0f0f0f,0x3f3f3f3f);
    vIO32Write4BMsk(0x40e60,0xf0f0f0f,0x3f3f3f3f);
    vIO32Write4BMsk(0x40e64,0xf0f0f0f,0x3f3f3f3f);
    vIO32Write4BMsk(0x40e68,0,0x7f7f3f3f);
    vIO32Write4BMsk(0x40f58,0xf0f0f0f,0x3f3f3f3f);
    vIO32Write4BMsk(0x40f5c,0xf0f0f0f,0x3f3f3f3f);
    vIO32Write4BMsk(0x40f60,0xf0f0f0f,0x3f3f3f3f);
    vIO32Write4BMsk(0x40f64,0xf0f0f0f,0x3f3f3f3f);
    vIO32Write4BMsk(0x40f68,0,0x7f7f3f3f);
    vIO32Write4BMsk(0x30858,0x30000001,0xf0000003);
    vIO32Write4BMsk(0x30a00,3,0xf);
    vIO32Write4BMsk(0x30b00,3,0xf);
    puVar2 = ac_timing[0];
    uVar1 = 0;
    while (((uint)*puVar2 != p->dram_type || ((uint)puVar2[1] != p->frequency))) {
      uVar1 = uVar1 + 1 & 0xff;
      puVar2 = puVar2 + 0x3a;
      if (uVar1 == 4) {
LAB_9fa33344:
        vIO32Write4BMsk(0x40004,0x10000,0x10000);
        pause_polling(500);
        vIO32Write4BMsk(0x30034,0x731414,0x77ffffff);
        pause_polling(0x14);
        vIO32Write4BMsk(0x30024,0x80002050,0xa000ffff);
        pause_polling(100);
        if (p->frequency == 0x640) {
          DramcModeRegWrite_PC4(p,0,2,0);
          DramcModeRegWrite_PC4(p,0,3,0);
          DramcModeRegWrite_PC4(p,0,1,0x103);
          DramcModeRegWrite_PC4(p,1,0,0);
          DramcModeRegWrite_PC4(p,1,1,0x400);
          DramcModeRegWrite_PC4(p,1,2,0x480);
          pause_polling(10);
          DramcModeRegWrite_PC4(p,1,2,0x4c8);
          pause_polling(10);
          DramcModeRegWrite_PC4(p,1,2,0x448);
          pause_polling(10);
          DramcModeRegWrite_PC4(p,0,0,0x310);
          pause_polling(100);
        }
        else if (p->frequency == 0x74a) {
          DramcModeRegWrite_PC4(p,0,2,8);
          DramcModeRegWrite_PC4(p,0,3,0);
          DramcModeRegWrite_PC4(p,0,1,0x103);
          DramcModeRegWrite_PC4(p,1,0,0);
          DramcModeRegWrite_PC4(p,1,1,0x400);
          DramcModeRegWrite_PC4(p,1,2,0x480);
          pause_polling(10);
          DramcModeRegWrite_PC4(p,1,2,0x4c8);
          pause_polling(10);
          DramcModeRegWrite_PC4(p,1,2,0x448);
          pause_polling(10);
          DramcModeRegWrite_PC4(p,0,0,0x520);
          pause_polling(100);
        }
        ucDram_Register_Write(0x3005c,0x40000);
        ucDram_Register_Write(0x30060,0);
        vIO32Write4BMsk(0x30060,0x10,0x10);
        pause_polling(10);
        vIO32Write4BMsk(0x30060,0,0x10);
        pause_polling(100);
        vIO32Write4BMsk(0x30004,0x702001,0xf7df7ff);
        vIO32Write4BMsk(0x30060,0x20,0x20);
        pause_polling(10);
        vIO32Write4BMsk(0x30060,0,0x20);
        ucDram_Register_Write(0x30074,0);
        vIO32Write4BMsk(0x30074,1,1);
        vIO32Write4BMsk(0x30004,0x100,0x100);
        vIO32Write4BMsk(0x30004,0x10000,0x10000);
        vIO32Write4BMsk(0x30860,p->odt_onoff << 1,2);
        vIO32Write4BMsk(0x30058,0xa56,0xc000ffff);
        vIO32Write4BMsk(0x3084c,0xff0000,0x1ff01ff);
        vIO32Write4BMsk(0x30064,0x40000c0,0xfe0808e0);
        vIO32Write4BMsk(0x30844,0xb00000,0x3ff0000);
        vIO32Write4BMsk(0x3004c,0x40000000,0x60000000);
        vIO32Write4BMsk(0x30048,0x40000000,0x40000000);
        vIO32Write4BMsk(0x3006c,0x20000,0x3ff00);
        vIO32Write4BMsk(0x30038,0x40000001,0x40000001);
        ucDram_Register_Write(0x30204,0x15e00);
        vIO32Write4BMsk(0x30094,0x100000,0xfffffff0);
        vIO32Write4BMsk(0x30098,0x4000,0xfffffff0);
        vIO32Write4BMsk(0x3009c,0x12000480,0xfff87fff);
        vIO32Write4BMsk(0x300dc,0x2000000,0x2000000);
        vIO32Write4BMsk(0x30004,0x2000020,0x2000020);
        vIO32Write4BMsk(0x3085c,0x30210000,0x33770000);
        vIO32Write4BMsk(0x3085c,0x20000,0x3f3fff3);
        ucDram_Register_Write(0x300a0,0x4080110d);
        vIO32Write4BMsk(0x300a0,0x4080110d,0xf3d3dfff);
        vIO32Write4BMsk(0x30050,0x21,0x21);
        ucDram_Register_Write(0x3001c,0);
        vIO32Write4BMsk(0x30004,0,1);
        vIO32Write4BMsk(0x30024,0,0x50);
        return 0;
      }
    }
    iVar3 = uVar1 * 0x74;
    if (uVar1 != 4) {
      vIO32Write4BMsk(0x30800,((ushort)ac_timing[uVar1][AC_TRCD] & 0xf) << 0x18 |
                              ((ushort)ac_timing[uVar1][AC_TRRD] & 7) << 0x10 |
                              (ushort)ac_timing[uVar1][AC_TWTR] & 0xf |
                              ((ushort)ac_timing[uVar1][AC_TWR] & 0x1f) << 8,0xf071f0f);
      vIO32Write4BMsk(0x30804,((ushort)ac_timing[uVar1][AC_TRC] & 0x1f) << 0x18 |
                              ((ushort)ac_timing[uVar1][AC_TRAS] & 0xf) << 0x10 |
                              (ushort)ac_timing[uVar1][AC_TRPAB] & 7 |
                              ((ushort)ac_timing[uVar1][AC_TRP] & 0xf) << 8,0x1f0f0f07);
      vIO32Write4BMsk(0x30808,((ushort)ac_timing[uVar1][AC_TFAW] & 0x1f) << 0x18 |
                              ((ushort)ac_timing[uVar1][AC_TRTP] & 7) << 8 |
                              (ushort)ac_timing[uVar1][AC_TXP] & 7 |
                              ((ushort)ac_timing[uVar1][AC_TRTW_ODT_OFF + p->odt_onoff] & 0xf)
                              << 0x10,0x1f0f0707);
      vIO32Write4BMsk(0x3080c,(uint)(byte)ac_timing[uVar1][AC_TRFC] << 0x10 |
                              (uint)(byte)ac_timing[uVar1][AC_TRFCPB],0xff00ff);
      vIO32Write4BMsk(0x30810,(ushort)ac_timing[uVar1][AC_TXREFCNT] & 0x3ff |
                              (uint)(ushort)ac_timing[uVar1][AC_TZQCS] << 0x18 |
                              (uint)(byte)ac_timing[uVar1][AC_REFCNT_FR_CLK] << 0x10,0xffff03ff);
      vIO32Write4BMsk(0x30814,((ushort)ac_timing[uVar1][AC_TWTPD] & 0x1f) << 8 |
                              (ushort)ac_timing[uVar1][AC_TRTPD] & 0x1f |
                              ((ushort)ac_timing[uVar1][AC_TMRR2W_ODT_OFF + p->odt_onoff] & 0xf)
                              << 0x18,0xf001f1f);
      vIO32Write4BMsk(0x30818,((ushort)ac_timing[uVar1][AC_TWRMPR] & 0xf) << 8 |
                              ((ushort)ac_timing[uVar1][AC_BGTWTR] & 0xf) << 4 |
                              (ushort)ac_timing[uVar1][AC_BGTCCD] & 3,0xff3);
      vIO32Write4BMsk(0x3081c,((ushort)ac_timing[uVar1][AC_XRTW2W] & 0xf) << 0x18 |
                              ((ushort)ac_timing[uVar1][AC_XRTW2R] & 7) << 0x10 |
                              (ushort)ac_timing[uVar1][AC_XRTR2R] & 0xf |
                              ((ushort)ac_timing[uVar1][AC_XRTR2W] & 0xf) << 8,0xf070f0f);
      vIO32Write4BMsk(0x30820,((ushort)ac_timing[uVar1][AC_BGTWTR_05T] & 1) << 0x17 |
                              ((ushort)ac_timing[uVar1][AC_BGTCCD_05T] & 1) << 0x16 |
                              (ushort)ac_timing[uVar1][AC_TRC_05T] & 1 |
                              ((ushort)ac_timing[uVar1][AC_TRP_05T] & 1) << 7 |
                              ((ushort)ac_timing[uVar1][AC_TRCD_05T] & 1) << 6 |
                              ((ushort)ac_timing[uVar1][AC_TWR_05T] & 1) << 10 |
                              ((ushort)ac_timing[uVar1][AC_TXP_05T] & 1) << 4,0xc004d1);
      vIO32Write4BMsk(0x30a00,(uint)(ushort)ac_timing[uVar1][AC_DQSINCTL_FOR_GATING],0xf);
    }
    goto LAB_9fa33344;
  }
  vIO32Write4BMsk(0x3004c,0x65714001,0x6ff17001);
  vIO32Write4BMsk(0x402a8,0,0xc);
  vIO32Write4BMsk(0x502a8,0,0xc);
  vIO32Write4BMsk(0x30c54,0x200600,0x200f00);
  vIO32Write4BMsk(0x300dc,0x1d007,0xffd03f);
  vIO32Write4BMsk(0x30210,0xe090601,0x3f3f3f01);
  if (p->data_width == 0x10) {
    vIO32Write4BMsk(0x30000,0x20003000,0x20003000);
  }
  else {
    vIO32Write4BMsk(0x30000,0x20103000,0x20103000);
  }
  vIO32Write4BMsk(0x4029c,0x101020f,0x101020f);
  vIO32Write4BMsk(0x5029c,0x101020f,0x101020f);
  vIO32Write4BMsk(0x30840,0xa20010bf,0xe200f0bf);
  vIO32Write4BMsk(0x30860,p->odt_onoff & 1 | 0xc0030050,0xc07f00f3);
  vIO32Write4BMsk(0x3004c,0x2000,0x40007001);
  vIO32Write4BMsk(0x30200,0x80000000,0xc0000000);
  vIO32Write4BMsk(0x30048,0x8000000,0x4f400000);
  vIO32Write4BMsk(0x30878,0xc0000000,0xc0000000);
  vIO32Write4BMsk(0x30034,4,4);
  vIO32Write4BMsk(0x30848,0x7320f,0x7ffff);
  vIO32Write4BMsk(0x308c8,0x2a000,0xfff83e);
  vIO32Write4BMsk(0x300d4,0,3);
  vIO32Write4BMsk(0x30050,0,2);
  vIO32Write4BMsk(0x30054,0x8000,0x8000);
  vIO32Write4BMsk(0x30880,0,0x77777777);
  vIO32Write4BMsk(0x30884,0,0x7070777);
  vIO32Write4BMsk(0x30888,0,0x77777777);
  vIO32Write4BMsk(0x3088c,0,0x77777777);
  vIO32Write4BMsk(0x30890,0x10000111,0x77770777);
  vIO32Write4BMsk(0x30894,0x1000000,0x7070777);
  vIO32Write4BMsk(0x30898,0,0x77777777);
  vIO32Write4BMsk(0x3089c,0,0x77777777);
  vIO32Write4BMsk(0x40ec4,0xa00,0x3f003f3f);
  vIO32Write4BMsk(0x40fc4,0,0x3f003f3f);
  if (p->frequency < 0x641) {
    vIO32Write4BMsk(0x308a0,0x11112222,0x77777777);
    vIO32Write4BMsk(0x308a4,0x33331111,0x77777777);
    vIO32Write4BMsk(0x30a2c,0x11112222,0x77777777);
    vIO32Write4BMsk(0x30a30,0x11112222,0x77777777);
    vIO32Write4BMsk(0x30a34,0x33331111,0x77777777);
    vIO32Write4BMsk(0x30a38,0x33331111,0x77777777);
    vIO32Write4BMsk(0x30b2c,0x11112222,0x77777777);
    vIO32Write4BMsk(0x30b30,0x11112222,0x77777777);
    vIO32Write4BMsk(0x30b34,0x33331111,0x77777777);
    vIO32Write4BMsk(0x30b38,0x33331111,0x77777777);
  }
  else if (p->frequency == 0x74a) {
    vIO32Write4BMsk(0x308a0,0x22222222,0x77777777);
    vIO32Write4BMsk(0x308a4,0x55553333,0x77777777);
    vIO32Write4BMsk(0x30a2c,0x22222222,0x77777777);
    vIO32Write4BMsk(0x30a30,0x22222222,0x77777777);
    vIO32Write4BMsk(0x30a34,0x55553333,0x77777777);
    vIO32Write4BMsk(0x30a38,0x55553333,0x77777777);
    vIO32Write4BMsk(0x30b2c,0x22222222,0x77777777);
    vIO32Write4BMsk(0x30b30,0x22222222,0x77777777);
    vIO32Write4BMsk(0x30b34,0x55553333,0x77777777);
    vIO32Write4BMsk(0x30b38,0x55553333,0x77777777);
  }
  vIO32Write4BMsk(0x40e1c,0xf0f00,0x3f3f3f00);
  vIO32Write4BMsk(0x40f1c,0xf0f00,0x3f3f3f00);
  vIO32Write4BMsk(0x40e6c,0xf0f00,0x3f3f3f00);
  vIO32Write4BMsk(0x40f6c,0xf0f00,0x3f3f3f00);
  vIO32Write4BMsk(0x30a1c,0x11111010,0x77777777);
  vIO32Write4BMsk(0x30a20,0x3030,0x77777777);
  vIO32Write4BMsk(0x30b1c,0x1111,0x77777777);
  vIO32Write4BMsk(0x30b20,0x3333,0x77777777);
  vIO32Write4BMsk(0x30844,0x2c000c0e,0x7c001f1f);
  vIO32Write4BMsk(0x30a24,0x22222222,0x77777777);
  vIO32Write4BMsk(0x30a28,0x20202020,0x77777777);
  vIO32Write4BMsk(0x30b24,0x22222222,0x77777777);
  vIO32Write4BMsk(0x30b28,0x20202020,0x77777777);
  vIO32Write4BMsk_All(0x40e08,0,0x3f3f3f3f);
  vIO32Write4BMsk_All(0x40e0c,0,0x3f3f3f3f);
  vIO32Write4BMsk_All(0x40e10,0,0x3f3f3f3f);
  vIO32Write4BMsk_All(0x40e14,0,0x3f3f3f3f);
  vIO32Write4BMsk_All(0x40e18,0,0x7f7f3f3f);
  vIO32Write4BMsk_All(0x40f08,0,0x3f3f3f3f);
  vIO32Write4BMsk_All(0x40f0c,0,0x3f3f3f3f);
  vIO32Write4BMsk_All(0x40f10,0,0x3f3f3f3f);
  vIO32Write4BMsk_All(0x40f14,0,0x3f3f3f3f);
  vIO32Write4BMsk_All(0x40f18,0,0x7f7f3f3f);
  vIO32Write4BMsk_All(0x40e58,0,0x3f3f3f3f);
  vIO32Write4BMsk_All(0x40e5c,0,0x3f3f3f3f);
  vIO32Write4BMsk_All(0x40e60,0,0x3f3f3f3f);
  vIO32Write4BMsk_All(0x40e64,0,0x3f3f3f3f);
  vIO32Write4BMsk_All(0x40e68,0,0x7f7f3f3f);
  vIO32Write4BMsk_All(0x40f58,0,0x3f3f3f3f);
  vIO32Write4BMsk_All(0x40f5c,0,0x3f3f3f3f);
  vIO32Write4BMsk_All(0x40f60,0,0x3f3f3f3f);
  vIO32Write4BMsk_All(0x40f64,0,0x3f3f3f3f);
  vIO32Write4BMsk_All(0x40f68,0,0x7f7f3f3f);
  vIO32Write4BMsk(0x30858,0x20000001,0xf0000003);
  vIO32Write4BMsk(0x30a00,2,0xf);
  vIO32Write4BMsk(0x30b00,2,0xf);
  prom_puts("[Dramc] PCDDR3 AC Timing update \n");
  puVar2 = ac_timing[0];
  if (p->frequency < 0x640) {
LAB_9fa32494:
    vIO32Write4BMsk(0x30800,(puVar2[0xc] & 0xf) << 0x18 | (puVar2[0xf] & 7) << 0x10 |
                            puVar2[0xe] & 0xf | (puVar2[0xd] & 0x1f) << 8,0xf071f0f);
    vIO32Write4BMsk(0x30804,(puVar2[7] & 0x1f) << 0x18 | (puVar2[4] & 0xf) << 0x10 | puVar2[6] & 7 |
                            (puVar2[5] & 0xf) << 8,0x1f0f0f07);
    vIO32Write4BMsk(0x30808,(puVar2[0x10] & 0x1f) << 0x18 | (puVar2[0xb] & 7) << 8 | puVar2[10] & 7
                            | (puVar2[p->odt_onoff + 0x11] & 0xf) << 0x10,0x1f0f0707);
    vIO32Write4BMsk(0x3080c,(uint)*(byte *)(puVar2 + 8) << 0x10 | (uint)*(byte *)(puVar2 + 9),
                    0xff00ff);
    vIO32Write4BMsk(0x30810,puVar2[0x15] & 0x3ff | (uint)puVar2[0x16] << 0x18 |
                            (uint)*(byte *)(puVar2 + 0x14) << 0x10,0xffff03ff);
    vIO32Write4BMsk(0x30814,(puVar2[0x18] & 0x1f) << 8 | puVar2[0x17] & 0x1f |
                            (puVar2[p->odt_onoff + 0x19] & 0xf) << 0x18,0xf001f1f);
    vIO32Write4BMsk(0x3081c,(puVar2[0x31] & 0xf) << 0x18 | (puVar2[0x33] & 0xf) << 8 |
                            puVar2[0x34] & 0xf | (puVar2[0x32] & 7) << 0x10,0xf070f0f);
    vIO32Write4BMsk(0x30a00,(uint)puVar2[0x36],0xf);
    vIO32Write4BMsk(0x30b00,(uint)puVar2[0x36],0xf);
  }
  else {
    uVar1 = 0;
    do {
      if (((uint)*puVar2 == p->dram_type) && (p->frequency == (uint)puVar2[1])) {
        puVar2 = ac_timing[0] + uVar1 * 0x3a;
        if (uVar1 != 4) goto LAB_9fa32494;
        break;
      }
      uVar1 = uVar1 + 1 & 0xff;
      puVar2 = puVar2 + 0x3a;
    } while (uVar1 != 4);
    prom_puts("Error: No match ACTiming found\n");
  }
  vIO32Write4BMsk(0x40004,0x10000,0x10000);
  pause_polling(500);
  vIO32Write4BMsk(0x30008,0x81080000,0x81080000);
  vIO32Write4BMsk(0x3000c,0x24f03,0x2cf13);
  vIO32Write4BMsk(0x3000c,0,1);
  vIO32Write4BMsk(0x30010,0x80,0xff);
  vIO32Write4BMsk(0x30020,9,0xb);
  vIO32Write4BMsk(0x30038,0x101,0x101);
  vIO32Write4BMsk(0x30040,0x20000000,0x20000000);
  vIO32Write4BMsk(0x3004c,0x5004001,0xf007001);
  vIO32Write4BMsk(0x30000,0x10000,0x10000);
  vIO32Write4BMsk(0x30064,0x4000000,0x4000040);
  vIO32Write4BMsk(0x300b0,0x4300000,0xfff00000);
  vIO32Write4BMsk(0x30200,0x80200000,0xc0200000);
  vIO32Write4BMsk(0x30034,0x414,0x4f4);
  vIO32Write4BMsk(0x30034,0x2000,0x2000);
  pause_polling(500);
  vIO32Write4BMsk(0x30024,0x80002050,0x80002050);
  pause_polling(100);
  uVar1 = p->frequency;
  if (uVar1 != 0x640) {
    if (uVar1 == 0x74a) {
      DramcModeRegWrite_PC3(p,2,0x20);
      pause_polling(100);
      DramcModeRegWrite_PC3(p,3,0);
      pause_polling(100);
      DramcModeRegWrite_PC3(p,1,0x40);
      pause_polling(100);
      DramcModeRegWrite_PC3(p,0,0x1114);
      pause_polling(100);
      goto LAB_9fa327e0;
    }
    if (uVar1 != 0x4b0) {
      DramcModeRegWrite_PC3(p,2,0x10);
      pause_polling(100);
      DramcModeRegWrite_PC3(p,3,0);
      pause_polling(100);
      DramcModeRegWrite_PC3(p,1,0x40);
      pause_polling(100);
      DramcModeRegWrite_PC3(p,0,0x1d60);
      pause_polling(100);
      goto LAB_9fa327e0;
    }
  }
  DramcModeRegWrite_PC3(p,2,0x18);
  pause_polling(100);
  DramcModeRegWrite_PC3(p,3,0);
  pause_polling(100);
  DramcModeRegWrite_PC3(p,1,0x40);
  pause_polling(100);
  DramcModeRegWrite_PC3(p,0,0x1d70);
  pause_polling(100);
LAB_9fa327e0:
  vIO32Write4BMsk(0x30034,0,0x2400);
  vIO32Write4BMsk(0x30004,0x700000,0x700000);
  vIO32Write4BMsk(0x3005c,0x40000,0x1fff00);
  vIO32Write4BMsk(0x30060,0x10,0x10);
  pause_polling(100);
  vIO32Write4BMsk(0x30060,0,0x10);
  vIO32Write4BMsk(0x30060,0x20,0x20);
  vIO32Write4BMsk(0x30060,0,0x20);
  vIO32Write4BMsk(0x30074,1,1);
  vIO32Write4BMsk(0x30004,0x100,0x100);
  vIO32Write4BMsk(0x30004,0x200,0x200);
  vIO32Write4BMsk(0x30860,p->odt_onoff << 1,2);
  vIO32Write4BMsk(0x30058,0x56,0xff);
  vIO32Write4BMsk(0x3084c,0xff0000,0x1ff0000);
  vIO32Write4BMsk(0x30064,0x4000000,0x24000000);
  vIO32Write4BMsk(0x30844,0xb00000,0x3ff0000);
  vIO32Write4BMsk(0x3004c,0x40000000,0x40000000);
  vIO32Write4BMsk(0x30048,0x40000000,0x40000000);
  vIO32Write4BMsk(0x30038,0x40000001,0x44000001);
  vIO32Write4BMsk(0x3020c,0x10002,0xf0002);
  vIO32Write4BMsk(0x30204,0xc00,0xc00);
  vIO32Write4BMsk(0x30094,0x100000,0xfffffff0);
  vIO32Write4BMsk(0x30098,0x4000,0xfffffff0);
  vIO32Write4BMsk(0x3009c,0x10000480,0x10000780);
  vIO32Write4BMsk(0x300dc,0x2000000,0x2000000);
  vIO32Write4BMsk(0x30004,0x4080020,0x4080020);
  vIO32Write4BMsk(0x3085c,0x30000000,0x30000000);
  vIO32Write4BMsk(0x300d0,0x20000,0x30000);
  vIO32Write4BMsk(0x300a0,0x40000000,0x70000000);
  vIO32Write4BMsk(0x30050,0x21,0x21);
  vIO32Write4BMsk(0x3001c,0,0x77000000);
  vIO32Write4BMsk(0x30004,0,1);
  vIO32Write4BMsk(0x30024,0,0x50);
  vIO32Write4BMsk(0x3004c,0,0x20000000);
  return 0;
}

void DramReInitForCalibration(DRAMC_CTX_T *p)
{
  return;
}

void DramcHWDQSGatingTracking_DVT(DRAMC_CTX_T *p)
{
  return;
}

void DramcHWGatingInit(DRAMC_CTX_T *p)
{
  return;
}

void DramcHWGatingOnOff(DRAMC_CTX_T *p, U8 u1OnOff)
{
  vIO32Write4BMsk_All(0x30200,0,0x1000000);
  vIO32Write4BMsk_All(0x30200,0,0x400000);
  return;
}
