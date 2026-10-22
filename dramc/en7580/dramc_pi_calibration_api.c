/* SPDX-License-Identifier: GPL-2.0-or-later */
/* EN7580 implementation recovered from the preserved vendor objects. */
#include "en7580_ddr.h"

u32 Round_Operation(u32 numerator, u32 denominator)
{
  if (!denominator) return 0xffff;
  u32 quotient = (numerator / denominator) & 0xffff;
  if ((int)(denominator * (quotient + 1) - numerator) <=
      (int)(numerator - quotient * denominator)) quotient = (quotient + 1) & 0xffff;
  return quotient;
}

void TxWinTransferDelayToUIPI(DRAMC_CTX_T *p, u32 delay, u32 small_backup,
                            u32 large_backup, u8 *large, u8 *small, u8 *pi)
{
  u32 ui = small_backup + (delay >> 6) * 2;
  (void)p;
  *pi = delay & 0x3f; *small = ui & 3; *large = large_backup + (ui >> 2);
}

void Set_RX_DQM_DelayLine_Phy_Byte(DRAMC_CTX_T *p,uint u1ByteIdx,int value)
{
  if (u1ByteIdx == 0) {
    vIO32Write4BMsk(0x40e18,(value & 0x3fU) << 8 | value & 0x3fU,0x3f3f);
  }
  else if (u1ByteIdx == 1) {
    vIO32Write4BMsk(0x40e68,(value & 0x3fU) << 8 | value & 0x3fU,0x3f3f);
  }
  return;
}

void Set_RX_DQ_DelayLine_Phy_Byte(DRAMC_CTX_T *p, u32 byte_idx, u8 *values)
{
  (void)p;
  if (byte_idx > 1) return;
  u32 base = 0x40e08 + byte_idx * 0x50;
  for (u32 pair = 0; pair < 4; pair++) {
    u32 lo = values[pair * 2] & 0x3f, hi = values[pair * 2 + 1] & 0x3f;
    vIO32Write4BMsk(base + pair * 4, lo | lo << 8 | hi << 16 | hi << 24, 0x3f3f3f3f);
  }
}

bool ExecuteMoveDramCDelay(DRAMC_CTX_T *p,uint *regs,int shift)
{
  uint uVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;

  if (*(char *)(regs + 1) == '\b') {
    uVar1 = u4Dram_Register_Read(*regs);
  }
  else {
    uVar1 = u4Dram_Register_Read(*regs);
    uVar2 = regs[1] >> 8;
    uVar1 = (uVar1 & (0xffffffffU >> (-(regs[1] >> 0x10) & 0x1f)) << (uVar2 & 0x1f)) >>
            (uVar2 & 0x1f);
  }
  if (*(char *)(regs + 3) == '\b') {
    uVar2 = u4Dram_Register_Read(regs[2]);
  }
  else {
    uVar2 = u4Dram_Register_Read(regs[2]);
    uVar4 = regs[3] >> 8;
    uVar2 = (uVar2 & (0xffffffffU >> (-(regs[3] >> 0x10) & 0x1f)) << (uVar4 & 0x1f)) >>
            (uVar4 & 0x1f);
  }
  iVar3 = shift + uVar2 * 4 + (uVar1 & 0xfffffffb);
  if (iVar3 < 0) {
    uVar2 = 0;
    uVar1 = 0;
  }
  else {
    uVar2 = iVar3 >> 2;
    uVar1 = iVar3 + uVar2 * -4;
  }
  uVar4 = regs[1];
  if ((uVar4 & 0xff) == 8) {
    ucDram_Register_Write(*regs,uVar1);
  }
  else {
    vIO32Write4BMsk(*regs,uVar1 << (uVar4 >> 8 & 0x1f),
                    (0xffffffffU >> (-(uVar4 >> 0x10) & 0x1f)) << (uVar4 >> 8 & 0x1f));
  }
  uVar1 = regs[3];
  if ((uVar1 & 0xff) == 8) {
    ucDram_Register_Write(regs[2],uVar2);
  }
  else {
    vIO32Write4BMsk(regs[2],uVar2 << (uVar1 >> 8 & 0x1f),
                    (0xffffffffU >> (-(uVar1 >> 0x10) & 0x1f)) << (uVar1 >> 8 & 0x1f));
  }
  return iVar3 < 0;
}

void MoveDramC_TX_DQS(DRAMC_CTX_T *p, u32 byte_idx, int shift)
{
  if (byte_idx > 1) return;
  u32 field = 0x3000b + byte_idx * 0x400;
  u32 regs[4] = { 0x308a4, field, 0x308a0, field };
  ExecuteMoveDramCDelay(p, regs, shift);
}

void MoveDramC_TX_DQS_OEN(DRAMC_CTX_T *p, u32 byte_idx, int shift)
{
  if (byte_idx > 1) return;
  u32 field = 0x3100d + byte_idx * 0x400;
  u32 regs[4] = { 0x308a4, field, 0x308a0, field };
  ExecuteMoveDramCDelay(p, regs, shift);
}

void MoveDramC_TX_DQ(DRAMC_CTX_T *p, u32 byte_idx, int shift)
{
  if (byte_idx > 1) return;
  for (u32 group = 0; group < 2; group++) {
    u32 small = group ? 0x30a34 : 0x30a38;
    u32 large = group ? 0x30a2c : 0x30a30;
    u32 field = 0x3000b + byte_idx * 0x400;
    u32 regs[4] = { small, field, large, field };
    ExecuteMoveDramCDelay(p, regs, shift);
    regs[1] = regs[3] = 0x3100d + byte_idx * 0x400;
    ExecuteMoveDramCDelay(p, regs, shift);
  }
}

void vSetDramMRWriteLevelingOnOff(DRAMC_CTX_T *p,uint u1OnOff)
{
  if (u1OnOff == 0) {
    u1MR2Value = u1MR2Value & 0x7f;
  }
  else {
    u1MR2Value = u1MR2Value | 0x80;
  }
  if (p->dram_type == EN7580_TYPE_PCDDR3) {
    DramcModeRegWrite_PC3(p,1,u1OnOff << 7 | 0x40);
  }
  else if ((p->dram_type == EN7580_TYPE_PCDDR4) && (DramcModeRegWrite_PC4(p,0,1,u1OnOff << 7 | 0x103), u1OnOff == 0))
  {
    if (p->frequency == 0x640) {
      DramcModeRegWrite_PC4(p,0,0,0x310);
    }
    else if (p->frequency == 0x74a) {
      DramcModeRegWrite_PC4(p,0,0,0x520);
    }
    pause_polling(100);
    vIO32Write4BMsk(0x308c0,0x10,0x10);
    vIO32Write4BMsk(0x308c0,0,8);
  }
  return;
}

void vSetRankNumber(DRAMC_CTX_T *p)
{
  p->support_rank_num = 1;
  return;
}

void vSetRank(DRAMC_CTX_T *p,uint ucRank)
{
  p->rank = ucRank & 0xff;
  return;
}

uint u1GetRank(DRAMC_CTX_T *p)
{
  return (uint)*(byte *)&p->rank;
}

void vSetCalibrationResult(DRAMC_CTX_T *p,uint calibration,uint result)
{
  uint uVar1;
  uint uVar2;

  uVar1 = p->rank;
  uVar2 = 1 << (calibration & 0x1f);
  (&p->cal_executed)[uVar1] = (&p->cal_executed)[uVar1] | uVar2;
  if ((result & 0xff) == 0) {
    (&p->cal_result)[uVar1] = ~uVar2 & (&p->cal_result)[uVar1];
    return;
  }
  (&p->cal_result)[uVar1] = uVar2 | (&p->cal_result)[uVar1];
  return;
}

void vGetCalibrationResult_All(DRAMC_CTX_T *p,uint u1Rank,uint *u4CalExecute,uint *u4CalResult)
{
  *u4CalExecute = (&p->cal_executed)[u1Rank & 0xff];
  *u4CalResult = (&p->cal_result)[u1Rank & 0xff];
  return;
}

void vPrintCalibrationBasicInfo(DRAMC_CTX_T *p)
{
  return;
}

void vApplyConfigAfterCalibration(DRAMC_CTX_T *p)
{
  vIO32Write4BMsk_All(0x400b0,0,3);
  vIO32Write4BMsk_All(0x40130,0,3);
  vIO32Write4BMsk_All(0x401b0,0,3);
  vIO32Write4BMsk_All(0x400b0,4,4);
  vIO32Write4BMsk_All(0x40130,4,4);
  vIO32Write4BMsk_All(0x401b0,4,4);
  vIO32Write4BMsk_All(0x3022c,0,0x3800000);
  vIO32Write4BMsk_All(0x40284,0,0xf);
  vIO32Write4BMsk_All(0x4029c,0,0x80000000);
  vIO32Write4BMsk_All(0x402a0,0,0x2000000);
  vIO32Write4BMsk_All(0x30064,0x20000000,0x20000000);
  vIO32Write4BMsk_All(0x300c8,0x11000000,0x11000000);
  vIO32Write4BMsk_All(0x300d0,0,0x700080);
  vIO32Write4BMsk_All(0x30024,0,0xf0);
  vIO32Write4BMsk_All(0x30038,0,0x4000000);
  vIO32Write4BMsk_All(0x401b8,0x200000,0x200000);
  pause_polling(2000);
  vIO32Write4BMsk_All(0x402a0,0,0x80000000);
  return;
}

void vApplyConfigBeforeCalibration(DRAMC_CTX_T *p)
{
  EnableDramcPhyDCM(p,0);
  vIO32Write4BMsk_All(0x308c8,0x3e,0x3e);
  vIO32Write4BMsk_All(0x3084c,0x1ff,0x1ff);
  vIO32Write4BMsk_All(0x30004,0,0x40000);
  vIO32Write4BMsk(0x30058,0,0xc0000000);
  vIO32Write4BMsk_All(0x308c0,p->DBI_W_onoff << 0x1d,0x20000000);
  vIO32Write4BMsk_All(0x40c1c,p->DBI_R_onoff << 7,0x80);
  vIO32Write4BMsk_All(0x40c9c,p->DBI_R_onoff << 7,0x80);
  vIO32Write4BMsk_All(0x30064,0x20000000,0x20000000);
  vIO32Write4BMsk_All(0x30064,0,0x80000000);
  vIO32Write4BMsk_All(0x30064,0,0x40000000);
  DramcHWGatingOnOff(p,0);
  vIO32Write4BMsk_All(0x3020c,0,0x10000000);
  vIO32Write4BMsk_All(0x402a0,0x80,0x80);
  vIO32Write4BMsk_All(0x3004c,0,0x40000);
  DramcHWGatingOnOff(p,0);
  return;
}

void DramPhyReset(DRAMC_CTX_T *p)
{
  uint val;
  uint val_00;
  uint val_01;

  val = u4Dram_Register_Read(0x30064);
  val_00 = u4Dram_Register_Read(0x300c8);
  val_01 = u4Dram_Register_Read(0x300d0);
  vIO32Write4BMsk_All(0x30064,0x20000000,0x20000000);
  vIO32Write4BMsk_All(0x300c8,0x11000000,0x11000000);
  vIO32Write4BMsk_All(0x300d0,0,0x700080);
  pause_polling(4);
  vIO32Write4BMsk_All(0x30000,1,1);
  vIO32Write4BMsk_All(0x402a0,2,2);
  vIO32Write4BMsk_All(0x400a4,0,0x110);
  vIO32Write4BMsk_All(0x40124,0,0x110);
  pause_polling(1);
  vIO32Write4BMsk_All(0x40124,0x110,0x110);
  vIO32Write4BMsk_All(0x400a4,0x110,0x110);
  vIO32Write4BMsk_All(0x402a0,0,2);
  vIO32Write4BMsk_All(0x30000,0,1);
  ucDram_Register_Write(0x30064,val);
  ucDram_Register_Write(0x300c8,val_00);
  ucDram_Register_Write(0x300d0,val_01);
  return;
}

void SetRxDqDqsDelay(DRAMC_CTX_T *p,int iDelay)
{
  struct {
    u8 dq_delay[8];
    int local_18;
  } state;
  uint uVar1;

  state.local_18 = iDelay;
  if (iDelay < 1) {
    uVar1 = (-iDelay & 0x7fU) << 0x10 | (-iDelay & 0x7fU) << 0x18;
    vIO32Write4BMsk(0x40e18,uVar1,0x7f7f0000);
    vIO32Write4BMsk(0x40e68,uVar1,0x7f7f0000);
    DramPhyReset(p);
  }
  else {
    uVar1 = 0;
    if (7 < p->data_width) {
      do {
        Set_RX_DQM_DelayLine_Phy_Byte(p,uVar1,(int)(char)state.local_18);
        uVar1 = uVar1 + 1 & 0xff;
      } while (uVar1 < p->data_width >> 3);
      if (7 < p->data_width) {
        uVar1 = 0;
        do {
          state.dq_delay[0] = (char)state.local_18;
          state.dq_delay[1] = (char)state.local_18;
          state.dq_delay[2] = (char)state.local_18;
          state.dq_delay[3] = (char)state.local_18;
          state.dq_delay[4] = (char)state.local_18;
          state.dq_delay[5] = (char)state.local_18;
          state.dq_delay[6] = (char)state.local_18;
          state.dq_delay[7] = (char)state.local_18;
          Set_RX_DQ_DelayLine_Phy_Byte(p,uVar1,(u8 *)&state.dq_delay[0]);
          uVar1 = uVar1 + 1 & 0xff;
        } while (uVar1 < p->data_width >> 3);
      }
    }
  }
  return;
}

void dle_factor_handler(DRAMC_CTX_T *p,uint curr_val)
{
  if (curr_val < 3) {
    curr_val = 3;
  }
  vIO32Write4BMsk(0x30844,curr_val & 0x1f |
                          (curr_val - 3 & 0x1f) << 0x1a | (curr_val - 2 & 0x1f) << 8,0x7c001f1f);
  DramPhyReset(p);
  return;
}

int DramcRankSwap(DRAMC_CTX_T *p,uint u1Rank)
{
  uint val;

  val = u1Rank & 0xff;
  vIO32Write4BMsk(0x30034,(val << 3 | p->support_rank_num < 2 ^ 1) << 4,0xf0);
  if (val == 0) {
    vIO32Write4BMsk(0x30034,0,8);
  }
  else {
    vIO32Write4BMsk(0x30034,8,8);
  }
  vIO32Write4BMsk(0x30034,val,3);
  return 0;
}

void DramcSwImpedanceSaveRegister(DRAMC_CTX_T *p)
{
  vIO32Write4BMsk(0x308ac,(sw_impedance[0] << 5 | sw_impedance[1]) << 0x14,0x3ff00000);
  vIO32Write4BMsk(0x308a8,(sw_impedance[0] << 5 | sw_impedance[1]) << 10,0xffc00);
  vIO32Write4BMsk_All(0x308ac,sw_impedance[0] << 5 | sw_impedance[1],0x3ff);
  vIO32Write4BMsk_All(0x308ac,(sw_impedance[0] << 5 | sw_impedance[1]) << 10,0xffc00);
  vIO32Write4BMsk(0x308b4,(sw_impedance[2] << 5 | sw_impedance[3]) << 0x14,0x3ff00000);
  vIO32Write4BMsk(0x308b0,(sw_impedance[2] << 5 | sw_impedance[3]) << 10,0xffc00);
  return;
}

int DramcSwImpedanceCal(DRAMC_CTX_T *p)
{
  uint uVar1;
  uint uVar2;
  uint uVar3;

  ucDram_Register_Write(0x40278,0);
  ucDram_Register_Write(0x4027c,0);
  ucDram_Register_Write(0x40274,0);
  vIO32Write4BMsk_All(0x3022c,0,0x80000000);
  vIO32Write4BMsk(0x4026c,0,0x40);
  vIO32Write4BMsk(0x3022c,0x3000000,0x3200000);
  vIO32Write4BMsk(0x40268,0x120000,0x3f0000);
  vIO32Write4BMsk(0x40008,0,10);
  vIO32Write4BMsk(0x40268,0x40,0x60);
  pause_polling(1);
  vIO32Write4BMsk(0x3022c,0x800000,0x800000);
  vIO32Write4BMsk(0x3022c,0x400000,0x400000);
  vIO32Write4BMsk(0x3022c,0,0x200000);
  vIO32Write4BMsk(0x30864,0,0xf9f0);
  vIO32Write4BMsk(0x4026c,0,0xff00);
  uVar3 = 0;
  while( true ) {
    vIO32Write4BMsk(0x30864,uVar3 << 4,0x1f0);
    pause_polling(1);
    uVar1 = u4Dram_Register_Read(0x40504);
    if ((uVar1 & 0x2000) != 0) break;
    uVar3 = uVar3 + 1;
    if (0x14 < uVar3) {
      uVar3 = 0xff;
      break;
    }
    if (uVar3 == 0x10) {
      uVar3 = 0x1b;
    }
  }
  vIO32Write4BMsk(0x3022c,0,0x400000);
  vIO32Write4BMsk(0x30864,(uVar3 & 0x1f) << 4,0xf9f0);
  vIO32Write4BMsk(0x4026c,0,0xff00);
  uVar1 = 0;
  while( true ) {
    vIO32Write4BMsk(0x30864,uVar1 << 0xb,0xf800);
    pause_polling(1);
    uVar2 = u4Dram_Register_Read(0x40504);
    if ((uVar2 & 0x2000) == 0) break;
    uVar1 = uVar1 + 1;
    if (0x1f < uVar1) goto LAB_9fa347d4;
    if (uVar1 == 0x10) {
      uVar1 = 0x1b;
    }
  }
  if ((uVar1 == 0xff) || (uVar3 == 0xff)) {
LAB_9fa347d4:
    uVar1 = 0xb;
    uVar3 = 0xb;
  }
  uVar2 = uVar3 * 0x2d & 0xffff;
  sw_impedance[0] = Round_Operation(uVar2,0x2d);
  uVar3 = uVar1 * 0x2d & 0xffff;
  sw_impedance[1] = Round_Operation(uVar3,0x2d);
  sw_impedance[2] = Round_Operation(uVar2,0xf0);
  sw_impedance[3] = Round_Operation(uVar3,0xf0);
  DramcSwImpedanceSaveRegister(p);
  vSetCalibrationResult(p,1,0);
  vIO32Write4BMsk(0x3022c,0,0x3800000);
  return 0;
}

void Another_Rank_CKE_low(DRAMC_CTX_T *p,uint on_off)
{
  return;
}

void O1PathOnOff(DRAMC_CTX_T *p,uint u1OnOff)
{
  if ((u1OnOff & 0xff) == 0) {
    vIO32Write4BMsk(0x400ac,0,0x20000);
    vIO32Write4BMsk(0x4012c,0,0x20000);
    vIO32Write4BMsk_All(0x401ac,0,0x20000);
    vIO32Write4BMsk(0x400a4,0,2);
    vIO32Write4BMsk(0x40124,0,2);
    vIO32Write4BMsk_All(0x401a4,0,2);
  }
  else {
    vIO32Write4BMsk(0x400ac,0x20000,0x20000);
    vIO32Write4BMsk(0x4012c,0x20000,0x20000);
    vIO32Write4BMsk_All(0x401ac,0x20000,0x20000);
    vIO32Write4BMsk(0x400a4,2,2);
    vIO32Write4BMsk(0x40124,2,2);
    vIO32Write4BMsk_All(0x401a4,2,2);
    pause_polling(1);
  }
  return;
}

int DramcWriteLeveling(DRAMC_CTX_T *p)
{
  struct {
    byte local_50[4];
    byte local_4c[4];
    u8 padding_8[8];
    int local_40[4];
    uint local_30;
    uint local_2c;
    uint local_28;
    uint local_24;
    uint local_20;
    uint local_1c;
    uint local_18;
  } state;
  int iVar1;
  uint uVar2;
  int iVar3;
  byte *pbVar4;
  uint uVar5;
  uint uVar6;
  int *piVar7;
  int local_48;
  int local_44;

  state.local_40[0] = 0;
  state.local_40[1] = 0;
  iVar1 = 1;
  if (p != (DRAMC_CTX_T *)0x0) {
    fgwrlevel_done = 0;
    state.local_30 = u1GetRank(p);
    DramcRankSwap(p,(uint)*(byte *)&p->rank);
    state.local_4c[0] = 0;
    state.local_4c[1] = 8;
    state.local_2c = u4Dram_Register_Read(0x3004c);
    state.local_28 = u4Dram_Register_Read(0x30064);
    state.local_24 = u4Dram_Register_Read(0x30038);
    state.local_20 = u4Dram_Register_Read(0x300bc);
    state.local_1c = u4Dram_Register_Read(0x30024);
    state.local_18 = u4Dram_Register_Read(0x308c0);
    vIO32Write4BMsk(0x400a4,2,2);
    vIO32Write4BMsk(0x40124,2,2);
    vIO32Write4BMsk(0x401a4,2,2);
    vIO32Write4BMsk(0x400b0,1,3);
    vIO32Write4BMsk(0x40130,1,3);
    vIO32Write4BMsk_All(0x401b0,1,3);
    vIO32Write4BMsk(0x3004c,0x20000000,0x20000000);
    vIO32Write4BMsk(0x30038,0x4000000,0x4000000);
    vIO32Write4BMsk(0x30038,0,0x40000000);
    vIO32Write4BMsk(0x30024,0x40,0x40);
    vIO32Write4BMsk(0x308c0,0x10,0x10);
    O1PathOnOff(p,1);
    vSetDramMRWriteLevelingOnOff(p,1);
    pause_polling(1);
    vIO32Write4BMsk(0x300bc,0xa00,0xf00);
    vIO32Write4BMsk(0x300bc,1,1);
    vIO32Write4BMsk(0x300bc,0x20,0x20);
    iVar1 = 0xf;
    if (p->data_width == 0x10) {
      iVar1 = 3;
    }
    vIO32Write4BMsk(0x300bc,iVar1 << 0x10,0xf0000);
    pause_polling(1);
    state.local_50[0] = 0;
    iVar1 = p->rank * 8;
    *(u32 *)((u8 *)wrlevel_dqs_final_delay + iVar1) = 0;
    state.local_50[1] = 0;
    *(u32 *)(((u8 *)wrlevel_dqs_final_delay + 4) + iVar1) = 0;
    uVar5 = p->data_width;
    state.local_40[2] = 0xf0;
    if (uVar5 == 0x10) {
      state.local_40[2] = 0xfc;
    }
    if ((&p->write_leveling_shift)[p->rank] == '\0') {
      state.local_40[3] = u1GetRank(p);
      if (7 < uVar5) {
        uVar5 = 0;
        do {
          MoveDramC_TX_DQS(p,uVar5,-1);
          MoveDramC_TX_DQS_OEN(p,uVar5,-1);
          vSetRank(p,0);
          MoveDramC_TX_DQ(p,uVar5,-1);
          vSetRank(p,state.local_40[3]);
          uVar5 = uVar5 + 1 & 0xff;
        } while (uVar5 < p->data_width >> 3);
      }
      (&p->write_leveling_shift)[p->rank] = '\x01';
      p->tx_perbit_init = '\0';
    }
    vIO32Write4BMsk(0x40e1c,0,0x3f000000);
    vIO32Write4BMsk(0x40e6c,0,0x3f000000);
    iVar1 = 0;
    do {
      if (iVar1 < 1) {
        vIO32Write4BMsk(0x40ec4,iVar1 * -0x1000000,0x3f000000);
      }
      else {
        vIO32Write4BMsk(0x40e1c,iVar1 << 0x18,0x3f000000);
        vIO32Write4BMsk(0x40e6c,iVar1 << 0x18,0x3f000000);
      }
      vIO32Write4BMsk(0x300bc,0x80,0x80);
      vIO32Write4BMsk(0x300bc,0,0x80);
      pause_polling(1);
      uVar5 = u4Dram_Register_Read(0x40528);
      pbVar4 = state.local_50;
      uVar2 = 0;
      do {
        uVar6 = (uVar5 & 0xffff) >> (state.local_4c[uVar2] & 0x1f) & 0xff;
        if (*pbVar4 == 0) {
          if (uVar6 == 0) {
            *pbVar4 = 1;
          }
        }
        else if (uVar6 == 0) {
          *pbVar4 = 1;
        }
        else {
          *pbVar4 = *pbVar4 + 1;
        }
        if (((state.local_40[2] >> (uVar2 & 0x1f) & 1U) == 0) &&
           ((uVar6 = (uint)*pbVar4, uVar6 == 8 || ((iVar1 == 0x3f && (1 < uVar6)))))) {
          *(uint *)((u8 *)wrlevel_dqs_final_delay + (p->rank * 2 + uVar2) * 4) = (iVar1 - uVar6) + 2;
          state.local_40[2] = (1 << (uVar2 & 0x1f) | state.local_40[2]) & 0xff;
        }
        uVar2 = uVar2 + 1;
        pbVar4 = pbVar4 + 1;
      } while (uVar2 != 2);
    } while ((state.local_40[2] != 0xff) && (iVar1 = iVar1 + 1, iVar1 != 0x40));
    if (state.local_40[2] == 0xff) {
      fgwrlevel_done = 1;
      vSetCalibrationResult(p,3,0);
    }
    else {
      vSetCalibrationResult(p,3,1);
    }
    iVar1 = *(int *)((u8 *)wrlevel_dqs_final_delay + p->rank * 8);
    if (0x1f < iVar1) {
      iVar1 = 0x1f;
    }
    iVar3 = *(int *)(((u8 *)wrlevel_dqs_final_delay + 4) + p->rank * 8);
    if (iVar1 < *(int *)(((u8 *)wrlevel_dqs_final_delay + 4) + p->rank * 8)) {
      iVar3 = iVar1;
    }
    uVar5 = 0;
    if (iVar3 < 1) {
      uVar5 = -iVar3;
      if ((int)uVar5 < 1) {
        uVar5 = 0;
      }
      else {
        uVar2 = u4Dram_Register_Read(0x40ec4);
        vIO32Write4BMsk(0x40ec4,(((uVar2 & 0x3f00) >> 8) + uVar5) * 0x100,0x3f00);
        vIO32Write4BMsk(0x40ec4,uVar5,0x3f);
      }
    }
    piVar7 = state.local_40;
    vIO32Write4BMsk(0x40ec4,uVar5 << 0x18,0x3f000000);
    u4Dram_Register_Read(0x308a0);
    u4Dram_Register_Read(0x308a4);
    iVar1 = p->rank * 8;
    *(uint *)((u8 *)wrlevel_dqs_final_delay + iVar1) = *(int *)((u8 *)wrlevel_dqs_final_delay + iVar1) + uVar5
    ;
    *(uint *)(((u8 *)wrlevel_dqs_final_delay + 4) + iVar1) = *(int *)(((u8 *)wrlevel_dqs_final_delay + 4) + iVar1) + uVar5;
    vSetDramMRWriteLevelingOnOff(p,0);
    ucDram_Register_Write(0x30024,state.local_1c);
    ucDram_Register_Write(0x3004c,state.local_2c);
    ucDram_Register_Write(0x30064,state.local_28);
    ucDram_Register_Write(0x30038,state.local_24);
    ucDram_Register_Write(0x300bc,state.local_20);
    ucDram_Register_Write(0x308c0,state.local_18);
    O1PathOnOff(p,0);
    uVar5 = 0;
    do {
      iVar1 = *(int *)((u8 *)wrlevel_dqs_final_delay + (p->rank * 2 + uVar5) * 4);
      if (iVar1 < 0x40) {
        *piVar7 = iVar1;
      }
      else {
        *piVar7 = iVar1 + -0x40;
        MoveDramC_TX_DQS(p,uVar5,2);
        MoveDramC_TX_DQS_OEN(p,uVar5,2);
      }
      uVar5 = uVar5 + 1;
      piVar7 = piVar7 + 1;
    } while (uVar5 != 2);
    uVar5 = p->rank;
    if ((int)uVar5 < 1) {
      do {
        vSetRank(p,uVar5 & 0xff);
        vIO32Write4BMsk(0x40e1c,state.local_40[0] << 0x18,0x3f000000);
        vIO32Write4BMsk(0x40e6c,state.local_40[1] << 0x18,0x3f000000);
        uVar5 = uVar5 + 1;
      } while (uVar5 != 1);
    }
    vSetRank(p,state.local_30);
    local_48 = (int)(*(int *)((u8 *)wrlevel_dqs_final_delay + p->rank * 8) + 0x10);
    if (0x3f < (int)local_48) {
      local_48 = (-48) + *(int *)((u8 *)wrlevel_dqs_final_delay + p->rank * 8);
      MoveDramC_TX_DQ(p,0,2);
    }
    local_44 = (int)(*(int *)(((u8 *)wrlevel_dqs_final_delay + 4) + p->rank * 8) + 0x10);
    if (0x3f < (int)local_44) {
      local_44 = (-48) + *(int *)(((u8 *)wrlevel_dqs_final_delay + 4) + p->rank * 8);
      MoveDramC_TX_DQ(p,1,2);
    }
    uVar5 = p->rank;
    if ((int)uVar5 < 1) {
      do {
        vSetRank(p,uVar5 & 0xff);
        vIO32Write4BMsk(0x40e1c,((uint)local_48 & 0x3f) << 8 | ((uint)local_48 & 0x3f) << 0x10,
                        0x3f3f00);
        vIO32Write4BMsk(0x40e6c,((uint)local_44 & 0x3f) << 0x10 | ((uint)local_44 & 0x3f) << 8,
                        0x3f3f00);
        uVar5 = uVar5 + 1;
      } while (uVar5 != 1);
    }
    vSetRank(p,state.local_30);
    vIO32Write4BMsk(0x400a4,0,2);
    vIO32Write4BMsk(0x40124,0,2);
    vIO32Write4BMsk(0x401a4,0,2);
    vIO32Write4BMsk(0x400b0,0,3);
    vIO32Write4BMsk(0x40130,0,3);
    vIO32Write4BMsk_All(0x401b0,0,3);
    DramcRankSwap(p,0);
    iVar1 = 0;
  }
  return iVar1;
}

int DramcRxdqsGatingCal_PCDDR3(DRAMC_CTX_T *p)
{
  struct {
    char local_a0[4];
    byte local_9c[4];
    char local_98[4];
    byte local_94[4];
    byte local_90[4];
    byte local_8c[4];
    char local_88[4];
    byte local_84[4];
    byte local_80[4];
    ushort local_7c;
    u8 padding_26[2];
    byte local_78[4];
    byte local_74[4];
    byte local_70[4];
    byte local_6c[4];
    byte local_68[4];
    byte local_64[4];
    byte local_60[4];
    byte local_5c[4];
    uint local_58[4];
    uint local_48;
    uint local_44;
    uint local_40;
    uint local_3c;
    uint local_38;
    uint local_34;
    uint local_30;
    uint local_2c;
    uint local_28;
    uint local_24;
    uint local_20;
    uint local_1c;
    uint local_18;
    uint local_14;
  } state = {0};
  int iVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  char *pcVar5;
  byte *pbVar6;
  byte *pbVar7;
  byte *pbVar8;
  uint uVar9;
  byte *pbVar10;
  byte bVar11;

  iVar1 = 1;
  if (p != (DRAMC_CTX_T *)0x0) {
    state.local_24 = u4Dram_Register_Read(0x30200);
    state.local_20 = u4Dram_Register_Read(0x30204);
    state.local_1c = u4Dram_Register_Read(0x30000);
    state.local_18 = u4Dram_Register_Read(0x30060);
    state.local_14 = u4Dram_Register_Read(0x3004c);
    vIO32Write4BMsk(0x3004c,0,0x40000);
    DramcHWGatingOnOff(p,0);
    vIO32Write4BMsk(0x30204,0x400,0x400);
    vIO32Write4BMsk(0x30204,0x800,0x800);
    vIO32Write4BMsk(0x30000,0,0x400000);
    vIO32Write4BMsk(0x30060,0x100,0x100);
    pause_polling(4);
    vIO32Write4BMsk(0x30060,0x200,0x200);
    pause_polling(1);
    vIO32Write4BMsk(0x30060,0,0x200);
    uVar2 = u1GetRank(p);
    vIO32Write4BMsk_All(0x402a0,uVar2 << 0x19,0x2000000);
    state.local_a0[0] = '\0';
    state.local_9c[0] = 0;
    state.local_8c[0] = 0;
    state.local_84[0] = 0;
    state.local_88[0] = '\0';
    state.local_80[0] = 0;
    state.local_a0[1] = 0;
    state.local_9c[1] = '\0';
    state.local_8c[1] = 0;
    state.local_84[1] = 0;
    state.local_88[1] = 0;
    state.local_80[1] = 0;
    state.local_44 = 2;
    if (0x534 < p->frequency) {
      state.local_44 = 5;
    }
    state.local_28 = state.local_44 + 0x18;
    uVar2 = u4Dram_Register_Read(0x30a00);
    vIO32Write4BMsk(0x30860,(uVar2 & 0xf) << 4,0xf0);
    state.local_58[2] = 0;
    state.local_58[3] = 4;
    do {
      state.local_40 = state.local_44 >> 3;
      state.local_3c = state.local_44 & 7;
      uVar2 = (int)(state.local_44 + 2) >> 3;
      state.local_48 = (state.local_44 + 2) % 8;
      uVar4 = (state.local_44 & 0xfffffff8) + state.local_3c;
      if (uVar4 < 4) {
        state.local_2c = 2;
        state.local_34 = 2;
        state.local_30 = 0;
        state.local_38 = 0;
      }
      else {
        uVar4 = uVar4 - 4;
        state.local_38 = uVar4 >> 3 & 0xff;
        state.local_30 = uVar4 + state.local_38 * -8 & 0xff;
        uVar4 = (uVar2 * 8 + state.local_48) - 4;
        state.local_34 = uVar4 >> 3 & 0xff;
        state.local_2c = uVar4 + state.local_34 * -8 & 0xff;
      }
      uVar4 = state.local_40 & 7;
      uVar2 = uVar2 & 7;
      vIO32Write4BMsk(0x30a24,uVar4 << 0x10 | uVar4 << 8 | uVar4 | uVar4 << 0x18 | uVar2 << 4 |
                              uVar2 << 0xc | uVar2 << 0x14 | uVar2 << 0x1c,0x77777777);
      uVar4 = state.local_3c & 7;
      uVar2 = state.local_48 & 7;
      vIO32Write4BMsk(0x30a28,uVar4 << 0x10 | uVar4 << 8 | uVar4 | uVar4 << 0x18 | uVar2 << 4 |
                              uVar2 << 0xc | uVar2 << 0x14 | uVar2 << 0x1c,0x77777777);
      uVar4 = state.local_38 & 7;
      uVar2 = state.local_34 & 7;
      vIO32Write4BMsk(0x30a1c,uVar4 << 0x10 | uVar4 << 8 | uVar4 | uVar4 << 0x18 | uVar2 << 4 |
                              uVar2 << 0xc | uVar2 << 0x14 | uVar2 << 0x1c,0x77777777);
      uVar2 = state.local_30 & 7;
      uVar4 = state.local_2c & 7;
      vIO32Write4BMsk(0x30a20,uVar2 << 0x10 | uVar2 << 8 | uVar2 | uVar2 << 0x18 | uVar4 << 4 |
                              uVar4 << 0xc | uVar4 << 0x14 | uVar4 << 0x1c,0x77777777);
      uVar2 = 0;
      do {
        ucDram_Register_Write(0x30a04,uVar2 << 0x10 | uVar2 << 8 | uVar2 | uVar2 << 0x18);
        DramPhyReset(p);
        vIO32Write4BMsk(0x30060,0x200,0x200);
        pause_polling(1);
        vIO32Write4BMsk(0x30060,0,0x200);
        DramcEngine2(p,1,0x55000000,0xaa000046,1,0,0,0);
        uVar4 = u4Dram_Register_Read(0x20408);
        state.local_58[1] = uVar4 >> 0x10;
        state.local_58[0] = uVar4 & 0xffff;
        pbVar6 = state.local_8c;
        pcVar5 = state.local_a0;
        pbVar7 = state.local_9c;
        uVar4 = 0;
        do {
          if (((int)state.local_58[2] >> (uVar4 & 0x1f) & 1U) == 0) {
            uVar9 = p->data_width;
            if (uVar9 == 0x10) {
              if (*(short *)(state.local_58 + uVar4) != 0x4646) {
LAB_9fa354be:
                if (*pcVar5 == '\x01') {
                  *pcVar5 = '\0';
                  bVar11 = *pbVar6;
                  if (*pbVar7 < bVar11) {
                    state.local_98[uVar4] = state.local_88[uVar4];
                    state.local_94[uVar4] = state.local_84[uVar4];
                    state.local_90[uVar4] = state.local_80[uVar4];
                    *pbVar7 = bVar11;
                    if (0x45 < bVar11) {
                      *pbVar6 = 0x40;
                      *pbVar7 = 0x40;
                    }
                    if (*pbVar6 * state.local_58[3] - 0x21 < 0x3f) {
                      state.local_58[2] = (1 << (uVar4 & 0x1f) | state.local_58[2]) & 0xff;
                    }
                    if (uVar9 == 0x10) {
                      if (state.local_58[2] == 3) {
                        state.local_44 = state.local_28;
                        uVar2 = 0x20;
                      }
                    }
                    else if ((uVar9 == 0x20) && (state.local_58[2] == 0xf)) {
                      state.local_44 = state.local_28;
                      uVar2 = 0x20;
                    }
                  }
                }
                goto LAB_9fa3527c;
              }
            }
            else if (*(short *)(state.local_58 + uVar4) != 0x2323) goto LAB_9fa354be;
            if (*pcVar5 == '\0') {
              *pcVar5 = '\x01';
              *pbVar6 = 0;
              state.local_88[uVar4] = (char)state.local_40;
              state.local_84[uVar4] = (byte)state.local_3c;
              state.local_80[uVar4] = (byte)uVar2;
            }
            state.local_58[3] = 1;
            if (*pcVar5 == '\x01') {
              *pbVar6 = *pbVar6 + 1;
            }
          }
LAB_9fa3527c:
          iVar1 = state.local_58[3];
          uVar4 = uVar4 + 1;
          pbVar6 = pbVar6 + 1;
          pcVar5 = pcVar5 + 1;
          pbVar7 = pbVar7 + 1;
        } while (uVar4 != 2);
        uVar2 = uVar2 + state.local_58[3] & 0xff;
      } while (uVar2 < 0x20);
      state.local_44 = state.local_44 + 1 & 0xff;
    } while (state.local_44 < state.local_28);
    vSetCalibrationResult(p,4,0);
    if (state.local_9c[0] == 0) {
      vSetCalibrationResult(p,4,1);
    }
    if (state.local_9c[1] == '\0') {
      vSetCalibrationResult(p,4,1);
    }
    iVar3 = 0;
    do {
      uVar2 = ((int)((uint)state.local_9c[iVar3] * iVar1) >> 1) + (uint)state.local_90[iVar3];
      *(byte *)((int)&state.local_7c + iVar3) = (byte)uVar2 & 0x1f;
      uVar2 = ((uVar2 & 0xff) >> 5) + (uint)state.local_94[iVar3];
      uVar4 = uVar2 & 7;
      state.local_78[iVar3] = (byte)uVar4;
      bVar11 = (char)((uVar2 & 0xff) >> 3) + state.local_98[iVar3];
      state.local_74[iVar3] = bVar11;
      uVar4 = uVar4 + 2;
      state.local_70[iVar3] = (byte)uVar4 & 7;
      state.local_6c[iVar3] = bVar11 + (char)(uVar4 >> 3);
      iVar3 = iVar3 + 1;
    } while (iVar3 != 2);
    ucDram_Register_Write(0x30200,state.local_24);
    ucDram_Register_Write(0x30204,state.local_20);
    ucDram_Register_Write(0x30000,state.local_1c);
    ucDram_Register_Write(0x30060,state.local_18);
    ucDram_Register_Write(0x3004c,state.local_14);
    vIO32Write4BMsk(0x30a24,(state.local_74[1] & 7) << 8 | (state.local_6c[0] & 7) << 4 | state.local_74[0] & 7 |
                            (state.local_6c[1] & 7) << 0xc,0x7777);
    vIO32Write4BMsk(0x30a28,(state.local_78[1] & 7) << 8 | (state.local_70[0] & 7) << 4 | state.local_78[0] & 7 |
                            (state.local_70[1] & 7) << 0xc,0x7777);
    pbVar10 = state.local_68;
    pbVar8 = state.local_64;
    pbVar7 = state.local_60;
    pbVar6 = state.local_5c;
    iVar1 = 0;
    do {
      bVar11 = state.local_74[iVar1] * '\b' + state.local_78[iVar1];
      if (bVar11 < 4) {
        *pbVar10 = 0;
        *pbVar8 = 0;
        *pbVar7 = 2;
        *pbVar6 = 2;
      }
      else {
        bVar11 = bVar11 - 4;
        *pbVar10 = bVar11 >> 3;
        *pbVar8 = bVar11 - (bVar11 & 0xf8);
        bVar11 = (state.local_70[iVar1] - 4) + state.local_6c[iVar1] * '\b';
        *pbVar7 = bVar11 >> 3;
        *pbVar6 = bVar11 - (bVar11 & 0xf8);
      }
      iVar1 = iVar1 + 1;
      pbVar10 = pbVar10 + 1;
      pbVar8 = pbVar8 + 1;
      pbVar7 = pbVar7 + 1;
      pbVar6 = pbVar6 + 1;
    } while (iVar1 != 2);
    vIO32Write4BMsk(0x30a1c,(state.local_68[1] & 7) << 8 | (state.local_60[0] & 7) << 4 | state.local_68[0] & 7 |
                            (state.local_60[1] & 7) << 0xc,0x7777);
    vIO32Write4BMsk(0x30a20,(state.local_64[1] & 7) << 8 | (state.local_5c[0] & 7) << 4 | state.local_64[0] & 7 |
                            (state.local_5c[1] & 7) << 0xc,0x7777);
    ucDram_Register_Write(0x30a04,(uint)state.local_7c);
    DramPhyReset(p);
    iVar1 = 0;
  }
  return iVar1;
}

void DramcGatingMode(DRAMC_CTX_T *p,uint u1Mode)
{
  vIO32Write4BMsk(0x400a4,0,0x110);
  vIO32Write4BMsk(0x40124,0,0x110);
  pause_polling(1);
  vIO32Write4BMsk(0x40124,0x110,0x110);
  vIO32Write4BMsk(0x400a4,0x110,0x110);
  return;
}

int DramcRxdqsGatingCal_PCDDR4(DRAMC_CTX_T *p)
{
  struct {
    char local_b8[4];
    byte local_b4[4];
    byte local_b0[4];
    ushort local_ac;
    u8 padding_e[2];
    byte local_a8[4];
    byte local_a4[4];
    byte local_a0[4];
    byte local_9c[4];
    byte local_98[4];
    byte local_94[4];
    byte local_90[4];
    byte local_8c[4];
    uint local_88[2];
    byte local_80[4];
    char local_7c[4];
    byte local_78[4];
    char local_74[4];
    byte local_70[4];
    byte local_6c[4];
    char local_68[4];
    byte local_64[4];
    byte local_60[4];
    byte local_5c[4];
    uint local_58;
    uint local_54;
    uint local_50;
    uint local_4c;
    uint local_48;
    uint local_44;
    uint local_40;
    uint local_3c;
    uint local_38;
    uint local_34;
    uint local_30;
    uint local_2c;
    uint local_28;
    uint local_24;
    uint local_20;
    uint local_1c;
    uint local_18;
    uint local_14;
  } state;
  bool bVar1;
  int iVar2;
  uint uVar3;
  char *pcVar4;
  char *pcVar5;
  byte *pbVar6;
  byte *pbVar7;
  byte *pbVar8;
  byte *pbVar9;
  byte bVar10;
  uint uVar11;

  iVar2 = 1;
  if (p != (DRAMC_CTX_T *)0x0) {
    state.local_30 = u4Dram_Register_Read(0x30200);
    state.local_2c = u4Dram_Register_Read(0x30000);
    state.local_28 = u4Dram_Register_Read(0x30060);
    state.local_24 = u4Dram_Register_Read(0x3004c);
    state.local_20 = u4Dram_Register_Read(0x400b0);
    state.local_1c = u4Dram_Register_Read(0x40130);
    state.local_18 = u4Dram_Register_Read(0x4029c);
    state.local_14 = u4Dram_Register_Read(0x400c4);
    DramcGatingMode(p,0);
    vIO32Write4BMsk(0x400c4,0x1000,0x1000);
    DramcModeRegWrite_PC4(p,1,0,0x800);
    vIO32Write4BMsk(0x3004c,0,0x40000);
    DramcHWGatingOnOff(p,0);
    vIO32Write4BMsk(0x30000,0,0x400000);
    vIO32Write4BMsk(0x30060,0x100,0x100);
    pause_polling(4);
    vIO32Write4BMsk(0x30060,0x200,0x200);
    pause_polling(1);
    vIO32Write4BMsk(0x30060,0,0x200);
    vIO32Write4BMsk(0x402a0,0,0x2000000);
    DramcEngine2Init(p,0x55000000,0xaa000023,1,0);
    vIO32Write4BMsk(0x4029c,0x100,0x300);
    state.local_b8[0] = '\0';
    state.local_b4[0] = 0;
    state.local_b0[0] = 0;
    state.local_80[0] = 0;
    state.local_7c[0] = '\0';
    state.local_78[0] = 0;
    state.local_68[0] = '\0';
    state.local_64[0] = 0;
    state.local_60[0] = 0;
    state.local_5c[0] = 0;
    state.local_74[0] = '\0';
    state.local_70[0] = 0;
    state.local_6c[0] = 0;
    state.local_b8[1] = 0;
    state.local_b4[1] = '\0';
    state.local_b0[1] = '\0';
    state.local_80[1] = 0;
    state.local_7c[1] = 0;
    state.local_78[1] = '\0';
    state.local_68[1] = 0;
    state.local_64[1] = 0;
    state.local_60[1] = 0;
    state.local_5c[1] = 0;
    state.local_74[1] = 0;
    state.local_70[1] = 0;
    state.local_6c[1] = 0;
    state.local_48 = 0xb;
    if ((p->frequency < 0x640) && (state.local_48 = 0xc, 0x534 < p->frequency)) {
      state.local_48 = 0x11;
    }
    state.local_34 = state.local_48 + 0x18;
    uVar3 = u4Dram_Register_Read(0x30a00);
    vIO32Write4BMsk(0x30860,(uVar3 & 0xf) << 4,0xf0);
    state.local_50 = 0;
    do {
      state.local_44 = state.local_48 >> 3;
      state.local_40 = state.local_48 & 7;
      uVar11 = (int)(state.local_48 + 4) >> 3;
      state.local_58 = (state.local_48 + 4) % 8;
      uVar3 = (state.local_48 & 0xfffffff8) + state.local_40;
      if (uVar3 < 0xb) {
        prom_puts(
                 "[DramcRxdqsGatingCal] Error: ucdly_coarse_large_RODT is already 0. RODT cannot be -11 UI\n"
                 );
        state.local_38 = 4;
        state.local_4c = 4;
        state.local_3c = 0;
        state.local_54 = 0;
      }
      else {
        state.local_54 = (uint)((-11) + uVar3) >> 3 & 0xff;
        state.local_3c = (uint)((-11) + uVar3 + state.local_54 * -8) & 0xff;
        state.local_4c = (uint)((-11) + uVar11 * 8 + state.local_58) >> 3 & 0xff;
        state.local_38 = (uint)((-11) + uVar11 * 8 + state.local_58 + state.local_4c * -8) & 0xff;
      }
      uVar3 = state.local_44 & 7;
      uVar11 = uVar11 & 7;
      vIO32Write4BMsk(0x30a24,uVar3 << 0x10 | uVar3 << 8 | uVar3 | uVar3 << 0x18 | uVar11 << 4 |
                              uVar11 << 0xc | uVar11 << 0x14 | uVar11 << 0x1c,0x77777777);
      uVar11 = state.local_40 & 7;
      uVar3 = state.local_58 & 7;
      vIO32Write4BMsk(0x30a28,uVar11 << 0x10 | uVar11 << 8 | uVar11 | uVar11 << 0x18 | uVar3 << 4 |
                              uVar3 << 0xc | uVar3 << 0x14 | uVar3 << 0x1c,0x77777777);
      uVar11 = state.local_54 & 7;
      uVar3 = state.local_4c & 7;
      vIO32Write4BMsk(0x30a1c,uVar11 << 0x10 | uVar11 << 8 | uVar11 | uVar11 << 0x18 | uVar3 << 4 |
                              uVar3 << 0xc | uVar3 << 0x14 | uVar3 << 0x1c,0x77777777);
      uVar3 = state.local_3c & 7;
      uVar11 = state.local_38 & 7;
      vIO32Write4BMsk(0x30a20,uVar3 << 0x10 | uVar3 << 8 | uVar3 | uVar3 << 0x18 | uVar11 << 4 |
                              uVar11 << 0xc | uVar11 << 0x14 | uVar11 << 0x1c,0x77777777);
      uVar3 = 0;
      do {
        ucDram_Register_Write(0x30a04,uVar3 << 0x10 | uVar3 << 8 | uVar3 | uVar3 << 0x18);
        DramPhyReset(p);
        vIO32Write4BMsk(0x30060,0x200,0x200);
        pause_polling(1);
        vIO32Write4BMsk(0x30060,0,0x200);
        DramcEngine2Run(p,1,1);
        state.local_58 = u4Dram_Register_Read(0x40510);
        state.local_58 = state.local_58 & 0xffff;
        state.local_54 = u4Dram_Register_Read(0x40514);
        state.local_54 = state.local_54 & 0xffff;
        uVar11 = u4Dram_Register_Read(0x40508);
        state.local_7c[0] = (char)((uVar11 & 0x200) >> 9);
        uVar11 = u4Dram_Register_Read(0x40508);
        state.local_80[0] = (byte)(uVar11 >> 8) & 1;
        uVar11 = u4Dram_Register_Read(0x4050c);
        state.local_7c[1] = (char)((uVar11 & 0x200) >> 9);
        uVar11 = u4Dram_Register_Read(0x4050c);
        state.local_80[1] = (byte)(uVar11 >> 8) & 1;
        uVar11 = u4Dram_Register_Read(0x20408);
        state.local_88[1] = uVar11 >> 0x10;
        state.local_88[0] = uVar11 & 0xffff;
        pbVar8 = state.local_5c;
        pcVar5 = state.local_7c;
        pbVar6 = state.local_b0;
        pcVar4 = state.local_b8;
        uVar11 = 0;
        do {
          if (((int)state.local_50 >> (uVar11 & 0x1f) & 1U) == 0) {
            bVar1 = false;
            if ((((state.local_58 | state.local_54) >> ((uVar11 & 3) << 3) & 0xff) == 0) &&
               (bVar1 = (state.local_88[uVar11] & 0xffff) == 0x2323, bVar1)) {
              if (*pcVar4 == '\0') {
                *pcVar4 = '\x01';
                *pbVar6 = 0;
              }
              if (*pcVar4 == '\x01') {
                *pbVar6 = *pbVar6 + 1;
              }
            }
            else if (*pcVar4 == '\x01') {
              *pcVar4 = '\0';
              *pbVar8 = 0;
              if (state.local_b4[uVar11] < *pbVar6) {
                state.local_b4[uVar11] = *pbVar6;
              }
            }
            if (state.local_80[uVar11] == 0) {
              if ((*pcVar5 == '\0') && (bVar1)) {
                *pbVar8 = 0;
              }
              else {
                if (*pcVar5 != '\x01') goto LAB_9fa359e6;
                if (bVar1) {
                  *pbVar8 = 0;
                }
              }
            }
            else {
LAB_9fa359e6:
              if (state.local_80[uVar11] == 1) {
                if ((*pcVar5 == '\x01') && (bVar1)) {
                  *pbVar8 = *pbVar8 + 1;
                  state.local_68[uVar11] = (char)state.local_44;
                  state.local_64[uVar11] = (byte)state.local_40;
                  state.local_60[uVar11] = (byte)uVar3;
                }
                else if ((*pcVar5 == '\0') && (bVar1)) {
                  bVar10 = *pbVar8;
                  if (4 < bVar10) {
                    if (state.local_78[uVar11] < bVar10) {
                      state.local_74[uVar11] = state.local_68[uVar11];
                      state.local_70[uVar11] = state.local_64[uVar11];
                      state.local_6c[uVar11] = state.local_60[uVar11];
                      state.local_78[uVar11] = bVar10;
                      state.local_50 = (1 << (uVar11 & 0x1f) | state.local_50) & 0xff;
                    }
                    if (state.local_50 == 3) {
                      state.local_48 = state.local_34;
                      uVar3 = 0x20;
                    }
                  }
                }
              }
            }
          }
          uVar11 = uVar11 + 1;
          pbVar8 = pbVar8 + 1;
          pcVar5 = pcVar5 + 1;
          pbVar6 = pbVar6 + 1;
          pcVar4 = pcVar4 + 1;
        } while (uVar11 != 2);
        uVar3 = uVar3 + 4 & 0xff;
      } while (uVar3 < 0x20);
      state.local_48 = state.local_48 + 1 & 0xff;
    } while (state.local_48 < state.local_34);
    DramcEngine2End(p);
    vSetCalibrationResult(p,4,0);
    if (state.local_78[0] == 0) {
      vSetCalibrationResult(p,4,1);
    }
    if ((state.local_b4[0] == 0) && (state.local_b0[0] != 0)) {
      state.local_b4[0] = 0x18;
    }
    if (state.local_78[1] == '\0') {
      vSetCalibrationResult(p,4,1);
    }
    if ((state.local_b4[1] == '\0') && (state.local_b0[1] != '\0')) {
      state.local_b4[1] = '\x18';
    }
    iVar2 = 0;
    do {
      *(byte *)((int)&state.local_ac + iVar2) = (byte)(state.local_6c[iVar2] + 0x10) & 0x1f;
      uVar3 = (uint)state.local_70[iVar2] + (state.local_6c[iVar2] + 0x10 >> 5);
      uVar11 = uVar3 & 7;
      state.local_a8[iVar2] = (byte)uVar11;
      bVar10 = state.local_74[iVar2] + (char)(uVar3 >> 3);
      state.local_a4[iVar2] = bVar10;
      uVar11 = uVar11 + 4;
      state.local_a0[iVar2] = (byte)uVar11 & 7;
      state.local_9c[iVar2] = bVar10 + (char)(uVar11 >> 3);
      iVar2 = iVar2 + 1;
    } while (iVar2 != 2);
    ucDram_Register_Write(0x30200,state.local_30);
    ucDram_Register_Write(0x30000,state.local_2c);
    ucDram_Register_Write(0x30060,state.local_28);
    ucDram_Register_Write(0x3004c,state.local_24);
    ucDram_Register_Write(0x400b0,state.local_20);
    ucDram_Register_Write(0x40130,state.local_1c);
    DramcGatingMode(p,1);
    ucDram_Register_Write(0x4029c,state.local_18);
    ucDram_Register_Write(0x400c4,state.local_14);
    DramcModeRegWrite_PC4(p,1,0,0);
    vIO32Write4BMsk(0x30a24,(state.local_a4[1] & 7) << 8 | (state.local_9c[0] & 7) << 4 | state.local_a4[0] & 7 |
                            (state.local_9c[1] & 7) << 0xc,0x7777);
    vIO32Write4BMsk(0x30a28,(state.local_a8[1] & 7) << 8 | (state.local_a0[0] & 7) << 4 | state.local_a8[0] & 7 |
                            (state.local_a0[1] & 7) << 0xc,0x7777);
    pbVar9 = state.local_98;
    pbVar7 = state.local_94;
    pbVar8 = state.local_90;
    pbVar6 = state.local_8c;
    iVar2 = 0;
    do {
      bVar10 = state.local_a4[iVar2] * '\b' + state.local_a8[iVar2];
      if (bVar10 < 0xb) {
        *pbVar9 = 0;
        *pbVar7 = 0;
        *pbVar8 = 4;
        *pbVar6 = 4;
      }
      else {
        bVar10 = bVar10 - 0xb;
        *pbVar9 = bVar10 >> 3;
        *pbVar7 = bVar10 - (bVar10 & 0xf8);
        bVar10 = (state.local_a0[iVar2] - 0xb) + state.local_9c[iVar2] * '\b';
        *pbVar8 = bVar10 >> 3;
        *pbVar6 = bVar10 - (bVar10 & 0xf8);
      }
      iVar2 = iVar2 + 1;
      pbVar9 = pbVar9 + 1;
      pbVar7 = pbVar7 + 1;
      pbVar8 = pbVar8 + 1;
      pbVar6 = pbVar6 + 1;
    } while (iVar2 != 2);
    vIO32Write4BMsk(0x30a1c,(state.local_98[1] & 7) << 8 | (state.local_90[0] & 7) << 4 | state.local_98[0] & 7 |
                            (state.local_90[1] & 7) << 0xc,0x7777);
    vIO32Write4BMsk(0x30a20,(state.local_94[1] & 7) << 8 | (state.local_8c[0] & 7) << 4 | state.local_94[0] & 7 |
                            (state.local_8c[1] & 7) << 0xc,0x7777);
    vIO32Write4BMsk(0x30874,p->odt_onoff & 1 | 0x1f20,0xfffd);
    ucDram_Register_Write(0x30a04,(uint)state.local_ac);
    DramPhyReset(p);
    iVar2 = 0;
  }
  return iVar2;
}

int DramcRxWindowPerbitCal(DRAMC_CTX_T *p,uint u1UseTestEngine)
{
  struct {
    ushort local_240[80];
    ushort local_1a0[80];
    ushort local_100[80];
    uint local_60[4];
    uint local_50[4];
    u8 dq_delay[8];
    uint local_38;
    uint local_34;
    uint local_30;
    u8 padding_214[4];
    uint local_28;
    uint local_24;
    uint local_20;
    int local_1c;
    uint local_18;
  } state;
  short sVar1;
  ushort uVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  uint puVar8;
  uint *puVar9;
  uint *puVar10;
  uint compare_bits = 0;
  int puVar11;
  int local_2c;

  /*
   * The XX230v SDK identifies this as u4err_value. With DDR3_MPR_USED=0,
   * its non-test-engine branch never assigns a comparison result; the
   * EN7580 object likewise keeps an incoming s0 value on that branch.
   * Reject that unsupported mode in the C implementation. Test-engine mode assigns
   * compare_bits on every scan; the initial zero only removes decompiler
   * register-preservation plumbing before the first comparison.
   * This guard is a deliberate correction, not a byte-equivalent recovery.
   */
  if (!p || !(u1UseTestEngine & 0xff))
    return 1;

  state.local_34 = u1UseTestEngine & 0xff;
  state.local_60[0] = 0;
  state.local_60[1] = 0;
  state.local_50[0] = 0;
  state.local_50[1] = 0;
  state.local_50[2] = 0;
  state.local_50[3] = 0;
  state.dq_delay[0] = 0;
  state.dq_delay[1] = 0;
  state.dq_delay[2] = 0;
  state.dq_delay[3] = 0;
  state.dq_delay[4] = 0;
  state.dq_delay[5] = 0;
  state.dq_delay[6] = 0;
  state.dq_delay[7] = 0;
  if (p == (DRAMC_CTX_T *)0x0) {
    return 1;
  }
  state.local_18 = u1GetRank(p);
  if (state.local_34 == 0) {
    vSetCalibrationResult(p,6,1);
  }
  else {
    vSetCalibrationResult(p,7,1);
    if ((p->enable_rx_scan_vref != '\0') && (p->rank == 0)) {
      state.local_1c = 1;
      state.local_28 = 0x20;
      uVar7 = 0;
      goto LAB_9fa35e00;
    }
  }
  state.local_1c = 0;
  state.local_28 = 0x13;
  uVar7 = 0x12;
LAB_9fa35e00:
  uVar6 = p->data_width;
  if (uVar6 != 0) {
    uVar3 = 0;
    do {
      state.local_240[uVar3 * 5] = 0x7fff;
      state.local_240[uVar3 * 5 + 1] = 0x7fff;
      state.local_240[uVar3 * 5 + 2] = 0x7fff;
      state.local_1a0[uVar3 * 5] = 0x7fff;
      state.local_1a0[uVar3 * 5 + 1] = 0x7fff;
      state.local_1a0[uVar3 * 5 + 2] = 0x7fff;
      state.local_100[uVar3 * 5] = 0x7fff;
      state.local_100[uVar3 * 5 + 1] = 0x7fff;
      state.local_100[uVar3 * 5 + 2] = 0x7fff;
      uVar3 = uVar3 + 1 & 0xff;
    } while (uVar3 < uVar6);
  }
  uVar6 = p->frequency;
  if (uVar6 < 0x640) {
    if (uVar6 < 0x535) {
      if (uVar6 < 0x42a) {
        local_2c = (int)0xffffff81;
      }
      else {
        local_2c = (-96);
      }
    }
    else {
      local_2c = (-70);
    }
  }
  else {
    local_2c = (-48);
  }
  if (uVar7 < state.local_28) {
    state.local_24 = 0;
    state.local_20 = 0;
    state.local_30 = uVar7;
    do {
      uVar6 = 0;
      uVar7 = compare_bits;
      if (7 < p->data_width) {
        do {
          state.local_38 = uVar7;
          Set_RX_DQM_DelayLine_Phy_Byte(p,uVar6,0);
          uVar6 = uVar6 + 1 & 0xff;
          uVar7 = state.local_38;
        } while (uVar6 < p->data_width >> 3);
        compare_bits = state.local_38;
        if (7 < p->data_width) {
          uVar7 = 0;
          do {
            Set_RX_DQ_DelayLine_Phy_Byte(p,uVar7,(u8 *)&state.dq_delay[0]);
            uVar7 = uVar7 + 1 & 0xff;
            compare_bits = state.local_38;
          } while (uVar7 < p->data_width >> 3);
        }
      }
      vIO32Write4BMsk(0x40c14,state.local_30,0x3f);
      vIO32Write4BMsk(0x40c94,state.local_30,0x3f);
      state.local_38 = 0;
      puVar11 = local_2c;
      do {
        SetRxDqDqsDelay(p,(int)puVar11);
        if (state.local_34 != 0) {
          compare_bits = TestEngineCompare(p);
        }
        uVar7 = p->data_width;
        uVar2 = (ushort)(uint)puVar11;
        if (uVar7 != 0) {
          uVar6 = 0;
          do {
            uVar3 = 1 << (uVar6 & 0x1f) & compare_bits;
            if (state.local_240[uVar6 * 5] == 0x7fff) {
              if (uVar3 == 0) {
                state.local_240[uVar6 * 5] = uVar2;
              }
            }
            else if (state.local_240[uVar6 * 5 + 1] == 0x7fff) {
              if (uVar3 == 0) {
                if (puVar11 == (int)0x3f) {
                  state.local_240[uVar6 * 5 + 1] = 0x3f;
                  iVar5 = 0x3f;
                  goto LAB_9fa35f5c;
                }
              }
              else {
                iVar5 = (int)(short)(uVar2 - 1);
                state.local_240[uVar6 * 5 + 1] = uVar2 - 1;
                if (iVar5 != 0x7fff) {
LAB_9fa35f5c:
                  if ((int)(short)state.local_100[uVar6 * 5 + 1] - (int)(short)state.local_100[uVar6 * 5] <=
                      iVar5 - (short)state.local_240[uVar6 * 5]) {
                    state.local_38 = state.local_38 | 1 << (uVar6 & 0x1f);
                    state.local_100[uVar6 * 5] = state.local_240[uVar6 * 5];
                    state.local_100[uVar6 * 5 + 1] = (ushort)iVar5;
                  }
                  state.local_240[uVar6 * 5] = 0x7fff;
                  state.local_240[uVar6 * 5 + 1] = 0x7fff;
                }
              }
            }
            uVar6 = uVar6 + 1 & 0xff;
          } while (uVar6 < uVar7);
          if (uVar7 == 0x10) {
            if (state.local_38 == 0xffff) {
LAB_9fa35fd2:
              if (state.local_34 == 0) {
                vSetCalibrationResult(p,6,0);
              }
              else {
                vSetCalibrationResult(p,7,0);
              }
              uVar7 = p->data_width;
              if (uVar7 == 0x10) {
                if ((compare_bits & 0xffff) == 0xffff) goto LAB_9fa3639c;
              }
              else if ((uVar7 == 0x20) && (compare_bits == 0xffffffff)) goto LAB_9fa3639c;
            }
          }
          else if ((uVar7 == 0x20) && (state.local_38 == 0xffffffff)) goto LAB_9fa35fd2;
        }
        puVar11 = (int)(int)(short)(uVar2 + 1);
      } while ((int)puVar11 < 0x40);
      uVar7 = p->data_width;
      if (uVar7 != 0) {
LAB_9fa3639c:
        uVar3 = 0;
        uVar6 = uVar3;
        do {
          uVar6 = uVar6 + ((uint)state.local_100[uVar3 * 5 + 1] - (uint)state.local_100[uVar3 * 5]) & 0xffff;
          uVar3 = uVar3 + 1 & 0xff;
        } while (uVar3 < uVar7);
        if (state.local_20 < uVar6) {
          if (state.local_1c != 0) {
            prom_puts("Better Rx Vref found.\n");
            prom_puts("Vref = ");
            prom_print_dec(state.local_30);
            prom_puts("\n");
            state.local_24 = state.local_30;
          }
          uVar7 = p->data_width;
          state.local_20 = uVar6;
          if (uVar7 == 0) goto LAB_9fa360be;
          uVar3 = 0;
          do {
            state.local_1a0[uVar3 * 5] = state.local_100[uVar3 * 5];
            state.local_1a0[uVar3 * 5 + 1] = state.local_100[uVar3 * 5 + 1];
            uVar3 = uVar3 + 1 & 0xff;
          } while (uVar3 < uVar7);
        }
        uVar7 = p->data_width;
        if (uVar7 != 0) {
          uVar6 = 0;
          do {
            state.local_100[uVar6 * 5] = 0x7fff;
            state.local_100[uVar6 * 5 + 1] = 0x7fff;
            uVar6 = uVar6 + 1 & 0xff;
          } while (uVar6 < uVar7);
        }
      }
LAB_9fa360be:
      state.local_30 = state.local_30 + 1 & 0xffff;
    } while (state.local_30 < state.local_28);
  }
  else {
    state.local_24 = 0;
  }
  uVar7 = p->data_width;
  if (uVar7 == 0) {
    puVar11 = (int)0xffff;
    state.local_34 = 0;
  }
  else {
    puVar11 = (int)0xffff;
    state.local_34 = 0;
    uVar6 = 0;
    state.local_34 = 0;
    do {
      puVar8 = (int)((uint)state.local_1a0[uVar6 * 5 + 1] - (uint)state.local_1a0[uVar6 * 5] & 0xffff);
      state.local_34 = (uint)(puVar8 + state.local_34) & 0xffff;
      if (puVar8 < puVar11) {
        puVar11 = puVar8;
      }
      uVar6 = uVar6 + 1 & 0xff;
    } while (uVar6 < uVar7);
    uVar6 = 0;
    do {
      state.local_1a0[uVar6 * 5 + 2] =
           (ushort)((int)(short)state.local_1a0[uVar6 * 5 + 1] + (int)(short)state.local_1a0[uVar6 * 5] >> 1);
      uVar6 = uVar6 + 1 & 0xff;
    } while (uVar6 < uVar7);
  }
  puVar9 = state.local_60;
  puVar10 = state.local_50;
  uVar7 = 0;
  do {
    uVar3 = uVar7 + 7 & 0xff;
    *puVar9 = 0x7f;
    uVar6 = uVar7;
    if (uVar3 < uVar7) {
LAB_9fa36192:
      *puVar9 = 0;
      uVar6 = 0;
      if (uVar7 <= uVar3) goto LAB_9fa361a2;
    }
    else {
      do {
        if ((int)(short)state.local_1a0[uVar6 * 5 + 2] < (int)*puVar9) {
          *puVar9 = (int)(short)state.local_1a0[uVar6 * 5 + 2];
        }
        uVar6 = uVar6 + 1 & 0xff;
      } while (uVar6 <= uVar3);
      if (0 < (int)*puVar9) goto LAB_9fa36192;
      *puVar9 = -*puVar9;
LAB_9fa361a2:
      sVar1 = *(short *)puVar9;
      uVar6 = 0;
      uVar4 = uVar7;
      do {
        state.local_1a0[uVar4 * 5 + 4] = sVar1 + state.local_1a0[uVar4 * 5 + 2];
        uVar6 = uVar6 + (ushort)(sVar1 + state.local_1a0[uVar4 * 5 + 2]) & 0xffff;
        uVar4 = uVar4 + 1 & 0xff;
      } while (uVar4 <= uVar3);
    }
    *puVar10 = uVar6 >> 3;
    puVar9 = puVar9 + 1;
    uVar7 = uVar7 + 8 & 0xff;
    puVar10 = puVar10 + 1;
    if (uVar7 == 0x10) {
      if (state.local_1c != 0) {
        vIO32Write4BMsk(0x40c14,state.local_24,0x3f);
        vIO32Write4BMsk(0x40c94,state.local_24,0x3f);
      }
      if (p->data_width != 0) {
        uVar7 = 0;
        do {
          uVar7 = uVar7 + 4 & 0xff;
        } while (uVar7 < p->data_width);
      }
      state.local_30 = (uint)*(byte *)&p->rank;
      if (state.local_30 == 0) {
        do {
          local_2c = puVar11;
          state.local_38 = state.local_30;
          vSetRank(p,state.local_30);
          if (7 < p->data_width) {
            uVar7 = 0;
            do {
              Set_RX_DQM_DelayLine_Phy_Byte(p,uVar7,(int)*(char *)(state.local_50 + uVar7));
              uVar7 = uVar7 + 1 & 0xff;
            } while (uVar7 < p->data_width >> 3);
          }
          vIO32Write4BMsk(0x40e18,(state.local_60[0] & 0x7f) << 0x10 | (state.local_60[0] & 0x7f) << 0x18,
                          0x7f7f0000);
          vIO32Write4BMsk(0x40e68,(state.local_60[1] & 0x7f) << 0x10 | (state.local_60[1] & 0x7f) << 0x18,
                          0x7f7f0000);
          puVar11 = local_2c;
        } while (state.local_38 == 0xff);
      }
      vSetRank(p,state.local_18);
      DramPhyReset(p);
      state.local_30 = (uint)*(byte *)&p->rank;
      if (state.local_30 == 0) {
        do {
          local_2c = puVar11;
          state.local_38 = state.local_30;
          vSetRank(p,state.local_30);
          if (7 < p->data_width) {
            uVar7 = 0;
            do {
              iVar5 = (uVar7 & 0x1f) * 8;
              state.dq_delay[0] = (char)state.local_1a0[(uVar7 & 0x1f) * 0x28 + 4];
              state.dq_delay[1] = (char)state.local_1a0[(iVar5 + 1) * 5 + 4];
              state.dq_delay[2] = (char)state.local_1a0[(iVar5 + 2) * 5 + 4];
              state.dq_delay[3] = (char)state.local_1a0[(iVar5 + 3) * 5 + 4];
              state.dq_delay[4] = (char)state.local_1a0[(iVar5 + 4) * 5 + 4];
              state.dq_delay[5] = (char)state.local_1a0[(iVar5 + 5) * 5 + 4];
              state.dq_delay[6] = (char)state.local_1a0[(iVar5 + 6) * 5 + 4];
              state.dq_delay[7] = (u8)state.local_1a0[(iVar5 + 7) * 5 + 4];
              Set_RX_DQ_DelayLine_Phy_Byte(p,uVar7,(u8 *)&state.dq_delay[0]);
              uVar7 = uVar7 + 1 & 0xff;
            } while (uVar7 < p->data_width >> 3);
          }
          puVar11 = local_2c;
        } while (state.local_38 == 0xff);
      }
      vSetRank(p,state.local_18);
      p->min_winsize = (ushort)(uint)puVar11;
      p->sum_winsize = (ushort)state.local_34;
      return 0;
    }
  } while( true );
}

uint DramcRxdatlatScan(DRAMC_CTX_T *p,uint max)
{
  struct {
    u8 dq_delay[8];
    uint local_30;
    int local_2c;
    uint local_28;
    uint local_24;
    uint local_20;
    uint local_1c;
    uint local_18;
  } state;
  uint uVar1;
  int iDelay;

  state.local_24 = 1;
  if (p != (DRAMC_CTX_T *)0x0) {
    state.local_1c = u4Dram_Register_Read(0x30844);
    state.local_18 = u4Dram_Register_Read(0x30844);
    state.local_30 = 5;
    state.local_24 = 0;
    state.local_2c = 0;
    state.local_20 = 0xff;
    do {
      state.local_28 = state.local_30 & 0xff;
      dle_factor_handler(p,state.local_30);
      if (max == 0) {
        uVar1 = TestEngineCompare(p);
      }
      else {
        state.dq_delay[0] = 0;
        state.dq_delay[1] = 0;
        state.dq_delay[2] = 0;
        state.dq_delay[3] = 0;
        state.dq_delay[4] = 0;
        state.dq_delay[5] = 0;
        state.dq_delay[6] = 0;
        state.dq_delay[7] = 0;
        Set_RX_DQM_DelayLine_Phy_Byte(p,0,0);
        Set_RX_DQM_DelayLine_Phy_Byte(p,1,0);
        Set_RX_DQ_DelayLine_Phy_Byte(p,0,(u8 *)&state.dq_delay[0]);
        Set_RX_DQ_DelayLine_Phy_Byte(p,1,(u8 *)&state.dq_delay[0]);
        iDelay = -0x20;
        do {
          SetRxDqDqsDelay(p,iDelay);
          uVar1 = TestEngineCompare(p);
          if (uVar1 == 0) break;
          iDelay = iDelay + 4;
        } while (iDelay != 0x24);
      }
      if (p->data_width == 0x10) {
        uVar1 = uVar1 & 0xffff;
      }
      if (uVar1 == 0) {
        if (state.local_2c == 0) {
          state.local_20 = state.local_28;
        }
        else if (state.local_2c != 1) goto LAB_9fa36498;
        state.local_24 = state.local_24 + 1 & 0xff;
        if (5 < state.local_24) break;
        state.local_2c = 1;
      }
      else if (state.local_2c == 1) {
        state.local_2c = 0xff;
      }
LAB_9fa36498:
      state.local_30 = state.local_30 + 1;
    } while (state.local_30 != 0x20);
    if (state.local_24 == 0) {
      (&aru1RxDatlatResult)[p->rank] = (byte)state.local_18 & 0x1f;
      ucDram_Register_Write(0x30844,state.local_1c);
      vSetCalibrationResult(p,5,1);
    }
    else {
      if (state.local_24 < 4) {
        uVar1 = state.local_20 + (state.local_24 >> 1);
      }
      else {
        uVar1 = state.local_20 + 2;
      }
      (&aru1RxDatlatResult)[p->rank] = (char)uVar1;
      dle_factor_handler(p,uVar1 & 0xff);
      vSetCalibrationResult(p,5,0);
    }
    vIO32Write4BMsk(0x30020,9,0xb);
  }
  return state.local_24;
}

int DramcRxdatlatCal(DRAMC_CTX_T *p)
{
  uint uVar1;

  uVar1 = DramcRxdatlatScan(p,0);
  if (uVar1 < 5) {
    DramcRxdatlatScan(p,1);
  }
  return 0;
}

int DramcTxWindowPerbitCal(DRAMC_CTX_T *p,int calType)
{
  struct {
    ushort local_258[80];
    ushort local_1b8[80];
    ushort local_118[80];
    byte local_78;
    byte local_77;
    byte local_76;
    byte local_75;
    byte local_74;
    u8 padding_1e5[3];
    u8 dqm_pi[4];
    u8 dqm_large[4];
    u8 dqm_small[4];
    u8 dqm_oen_large[4];
    u8 dqm_oen_small[4];
    u8 dq_pi[4];
    u8 dq_large[4];
    u8 dq_small[4];
    u8 dq_oen_large[4];
    u8 dq_oen_small[4];
    ushort local_48[8];
    uint local_38;
    uint local_34;
    u8 padding_228[8];
    uint local_28;
    uint local_24;
    uint local_20;
    uint local_1c;
    uint local_18;
  } state = {0};
  ushort uVar1;
  int iVar2;
  uint uVar3;
  ushort *puVar4;
  char cVar5;
  uint uVar6;
  ushort uVar7;
  uint uVar8;
  uint uVar9;
  ushort *puVar10;
  ushort *puVar11;
  ushort *puVar12;
  uint uVar13;
  char cVar14;
  uint min_width, saved_min;
  u8 *local_30;
  ushort *local_2c;
  uint min_backup;

  state.local_48[4] = 0;
  state.local_48[5] = 0;
  state.local_48[6] = 0xffff;
  state.local_48[7] = 0xffff;
  iVar2 = 1;
  if (p != (DRAMC_CTX_T *)0x0) {
    uVar13 = 0;
    state.local_18 = u1GetRank(p);
    if ((fgwrlevel_done != '\0') && (uVar6 = p->data_width >> 3, uVar6 != 0)) {
      uVar8 = 0xff;
      uVar3 = 0;
      do {
        if ((int)*(uint *)((u8 *)wrlevel_dqs_final_delay + (p->rank * 2 + uVar3) * 4) < (int)uVar8) {
          uVar8 = *(uint *)((u8 *)wrlevel_dqs_final_delay + (p->rank * 2 + uVar3) * 4) & 0xffff;
          uVar13 = uVar3;
        }
        uVar3 = uVar3 + 1 & 0xff;
      } while (uVar3 < uVar6);
    }
    if ((&p->tx_perbit_init)[p->rank] == '\0') {
      uVar6 = u4Dram_Register_Read(0x308a0);
      dq_ui_large_bak = (byte)(uVar6 >> (uVar13 << 2 & 0x1f)) & 7;
      uVar3 = (uVar13 + 4) * 4;
      dq_oen_ui_large_bak = (byte)(uVar6 >> (uVar3 & 0x1f)) & 7;
      state.local_38 = uVar13 << 2;
      uVar13 = u4Dram_Register_Read(0x308a4);
      dq_ui_small_bak = (byte)(uVar13 >> (state.local_38 & 0x1f)) & 7;
      dq_oen_ui_small_bak = (byte)(uVar13 >> (uVar3 & 0x1f)) & 7;
      (&p->tx_perbit_init)[p->rank] = '\x01';
    }
    vSetCalibrationResult(p,8,1);
    uVar13 = p->data_width;
    if (uVar13 != 0) {
      uVar6 = 0;
      do {
        state.local_258[uVar6 * 5] = 0x7fff;
        state.local_258[uVar6 * 5 + 1] = 0x7fff;
        state.local_1b8[uVar6 * 5] = 0x7fff;
        state.local_1b8[uVar6 * 5 + 1] = 0x7fff;
        uVar6 = uVar6 + 1 & 0xff;
      } while (uVar6 < uVar13);
    }
    uVar13 = 0;
    state.local_34 = 0;
    do {
      TxWinTransferDelayToUIPI
                (p,uVar13,(uint)dq_ui_small_bak,(uint)dq_ui_large_bak,(u8 *)&state.local_76,
                 (u8 *)&state.local_77,(u8 *)&state.local_78);
      TxWinTransferDelayToUIPI
                (p,uVar13,(uint)dq_oen_ui_small_bak,(uint)dq_oen_ui_large_bak,
                 (u8 *)&state.local_74,(u8 *)&state.local_75,(u8 *)&state.local_78);
      if ((calType == 0) || (calType == 2)) {
        uVar3 = state.local_76 & 7;
        uVar6 = state.local_74 & 7;
        vIO32Write4BMsk(0x30a2c,uVar3 << 4 | uVar3 << 8 | uVar3 | uVar3 << 0xc | uVar6 << 0x10 |
                                uVar6 << 0x14 | uVar6 << 0x18 | uVar6 << 0x1c,0x77777777);
        uVar6 = state.local_77 & 7;
        uVar3 = state.local_75 & 7;
        vIO32Write4BMsk(0x30a34,uVar6 << 4 | uVar6 << 8 | uVar6 | uVar6 << 0xc | uVar3 << 0x10 |
                                uVar3 << 0x14 | uVar3 << 0x18 | uVar3 << 0x1c,0x77777777);
      }
      state.local_38 = calType - 1;
      if (state.local_38 < 2) {
        uVar3 = state.local_76 & 7;
        uVar6 = state.local_74 & 7;
        vIO32Write4BMsk(0x30a30,uVar3 << 4 | uVar3 << 8 | uVar3 | uVar3 << 0xc | uVar6 << 0x10 |
                                uVar6 << 0x14 | uVar6 << 0x18 | uVar6 << 0x1c,0x77777777);
        uVar3 = state.local_77 & 7;
        uVar6 = state.local_75 & 7;
        vIO32Write4BMsk(0x30a38,uVar3 << 4 | uVar3 << 8 | uVar3 | uVar3 << 0xc | uVar6 << 0x10 |
                                uVar6 << 0x14 | uVar6 << 0x18 | uVar6 << 0x1c,0x77777777);
      }
      if ((calType == 0) || (calType == 2)) {
        vIO32Write4BMsk(0x40e1c,(state.local_78 & 0x3f) << 8,0x3f00);
        vIO32Write4BMsk(0x40e6c,(state.local_78 & 0x3f) << 8,0x3f00);
      }
      if (state.local_38 < 2) {
        vIO32Write4BMsk(0x40e1c,(state.local_78 & 0x3f) << 0x10,0x3f0000);
        vIO32Write4BMsk(0x40e6c,(state.local_78 & 0x3f) << 0x10,0x3f0000);
      }
      uVar6 = TestEngineCompare(p);
      uVar3 = p->data_width;
      if (uVar3 != 0) {
        uVar8 = 0;
        do {
          uVar9 = 1 << (uVar8 & 0x1f) & uVar6;
          if (state.local_258[uVar8 * 5] == 0x7fff) {
            if (uVar9 == 0) {
              state.local_258[uVar8 * 5] = (ushort)uVar13;
            }
          }
          else if (state.local_258[uVar8 * 5 + 1] == 0x7fff) {
            if (uVar9 == 0) {
              if ((uVar13 & 0xffff) == 0x80) {
                state.local_258[uVar8 * 5 + 1] = 0x80;
                iVar2 = 0x80;
                goto LAB_9fa3686c;
              }
            }
            else {
              uVar7 = (ushort)uVar13 - 1;
              iVar2 = (int)(short)uVar7;
              state.local_258[uVar8 * 5 + 1] = uVar7;
              if (iVar2 != 0x7fff) {
LAB_9fa3686c:
                if ((int)(short)state.local_1b8[uVar8 * 5 + 1] - (int)(short)state.local_1b8[uVar8 * 5] <=
                    iVar2 - (short)state.local_258[uVar8 * 5]) {
                  state.local_34 = state.local_34 | 1 << (uVar8 & 0x1f);
                  state.local_1b8[uVar8 * 5] = state.local_258[uVar8 * 5];
                  state.local_1b8[uVar8 * 5 + 1] = (ushort)iVar2;
                }
                state.local_258[uVar8 * 5] = 0x7fff;
                state.local_258[uVar8 * 5 + 1] = 0x7fff;
              }
            }
          }
          uVar8 = uVar8 + 1 & 0xff;
        } while (uVar8 < uVar3);
        if (uVar3 == 0x10) {
          if (state.local_34 == 0xffff) {
LAB_9fa368e2:
            vSetCalibrationResult(p,8,0);
            if (p->data_width == 0x10) {
              if ((uVar6 & 0xffff) == 0xffff) goto LAB_9fa36906;
            }
            else if ((p->data_width == 0x20) && (uVar6 == 0xffffffff)) goto LAB_9fa36906;
          }
        }
        else if ((uVar3 == 0x20) && (state.local_34 == 0xffffffff)) goto LAB_9fa368e2;
      }
      uVar13 = uVar13 + 1;
    } while (uVar13 != 0x81);
    if (p->data_width == 0) {
      min_width = 0xffff;
      state.local_1c = p->data_width;
    }
    else {
LAB_9fa36906:
      uVar3 = p->data_width;
      min_width = 0xffff;
      uVar13 = 0;
      state.local_1c = 0;
      uVar6 = 0;
      do {
        u32 width = (u16)(state.local_1b8[uVar13 * 5 + 1] - state.local_1b8[uVar13 * 5]);
        state.local_1b8[uVar13 * 5 + 3] = state.local_1b8[uVar13 * 5 + 1] - state.local_1b8[uVar13 * 5];
        if (width < min_width) {
          min_width = width;
        }
        uVar6 = uVar6 + width & 0xffff;
        uVar13 = uVar13 + 1 & 0xff;
      } while (uVar13 < uVar3);
      state.local_1c = uVar6;
    }
    puVar4 = state.local_48;
    puVar11 = state.local_48 + 2;
    puVar12 = state.local_48 + 6;
    local_2c = puVar12;
    puVar10 = state.local_48 + 4;
    cVar14 = '\0';
    do {
      *puVar4 = 0xffff;
      *puVar11 = 0;
      *puVar12 = 0xffff;
      *puVar10 = 0;
      cVar5 = '\0';
      do {
        uVar13 = (uint)(byte)(cVar5 + cVar14);
        uVar7 = state.local_1b8[uVar13 * 5];
        state.local_118[uVar13 * 5] = uVar7;
        uVar1 = state.local_1b8[uVar13 * 5 + 1];
        state.local_118[uVar13 * 5 + 1] = uVar1;
        state.local_118[uVar13 * 5 + 3] = state.local_1b8[uVar13 * 5 + 3];
        iVar2 = (int)(short)uVar7 + (int)(short)uVar1 >> 1;
        uVar7 = (ushort)iVar2;
        state.local_118[uVar13 * 5 + 2] = uVar7;
        if (iVar2 < (int)(uint)*puVar4) {
          *puVar4 = uVar7;
        }
        if ((int)(uint)*puVar11 < iVar2) {
          *puVar11 = uVar7;
        }
        if ((int)(uint)*puVar10 < (int)(short)state.local_118[uVar13 * 5]) {
          *puVar10 = state.local_118[uVar13 * 5];
        }
        if ((int)(short)state.local_118[uVar13 * 5 + 1] < (int)(uint)*puVar12) {
          *puVar12 = state.local_118[uVar13 * 5 + 1];
        }
        cVar5 = cVar5 + '\x01';
      } while (cVar5 != '\b');
      puVar4 = puVar4 + 1;
      puVar11 = puVar11 + 1;
      puVar12 = puVar12 + 1;
      puVar10 = puVar10 + 1;
      cVar14 = cVar14 + '\b';
    } while (cVar14 != '\x10');
    iVar2 = 0;
    min_backup = min_width;
    do {
      uVar13 = (uint)*local_2c + (uint)state.local_48[iVar2 + 4] >> 1;
      state.local_34 = (uint)dq_ui_small_bak;
      state.local_28 = (uint)dq_ui_large_bak;
      local_30 = (u8 *)(&state.dqm_pi[0] + iVar2);
      TxWinTransferDelayToUIPI
                (p,uVar13,state.local_34,state.local_28,(u8 *)(&state.dqm_large[0] + iVar2),(u8 *)(&state.dqm_small[0] + iVar2),
                 local_30);
      state.local_24 = (uint)dq_oen_ui_small_bak;
      state.local_20 = (uint)dq_oen_ui_large_bak;
      TxWinTransferDelayToUIPI
                (p,uVar13,state.local_24,state.local_20,(u8 *)(&state.dqm_oen_large[0] + iVar2),(u8 *)(&state.dqm_oen_small[0] + iVar2),
                 local_30);
      local_30 = (u8 *)(&state.dq_pi[0] + iVar2);
      TxWinTransferDelayToUIPI
                (p,uVar13,state.local_34,state.local_28,(u8 *)(&state.dq_large[0] + iVar2),(u8 *)(&state.dq_small[0] + iVar2),
                 local_30);
      TxWinTransferDelayToUIPI
                (p,uVar13,state.local_24,state.local_20,(u8 *)(&state.dq_oen_large[0] + iVar2),(u8 *)(&state.dq_oen_small[0] + iVar2),
                 local_30);
      iVar2 = iVar2 + 1;
      local_2c = local_2c + 1;
    } while (iVar2 != 2);
    state.local_34 = (uint)*(byte *)&p->rank;
    min_width = min_backup;
    if (state.local_34 == 0) {
      saved_min = min_backup;
      do {
        uVar13 = state.local_34;
        vSetRank(p,state.local_34);
        if ((calType == 0) || (calType == 2)) {
          vIO32Write4BMsk(0x30a2c,(state.dqm_large[1] & 7) << 4 | (state.dqm_oen_large[0] & 7) << 0x10 | state.dqm_large[0] & 7 |
                                  (state.dqm_oen_large[1] & 7) << 0x14,0x770077);
          vIO32Write4BMsk(0x30a34,(state.dqm_small[1] & 7) << 4 | (state.dqm_oen_small[0] & 7) << 0x10 | state.dqm_small[0] & 7 |
                                  (state.dqm_oen_small[1] & 7) << 0x14,0x770077);
          vIO32Write4BMsk(0x40e1c,(state.dqm_pi[0] & 0x3f) << 8,0x3f00);
          vIO32Write4BMsk(0x40e6c,(state.dqm_pi[1] & 0x3f) << 8,0x3f00);
        }
        if (state.local_38 < 2) {
          vIO32Write4BMsk(0x30a30,(state.dq_large[1] & 7) << 4 | (state.dq_large[2] & 7) << 8 | state.dq_large[0] & 7 |
                                  (state.dq_large[3] & 7) << 0xc | (state.dq_oen_large[0] & 7) << 0x10 |
                                  (state.dq_oen_large[1] & 7) << 0x14 | (state.dq_oen_large[2] & 7) << 0x18 |
                                  (state.dq_oen_large[3] & 7) << 0x1c,0x77777777);
          vIO32Write4BMsk(0x30a38,(state.dq_small[1] & 7) << 4 | (state.dq_small[2] & 7) << 8 | state.dq_small[0] & 7 |
                                  (state.dq_small[3] & 7) << 0xc | (state.dq_oen_small[0] & 7) << 0x10 |
                                  (state.dq_oen_small[1] & 7) << 0x14 | (state.dq_oen_small[2] & 7) << 0x18 |
                                  (state.dq_oen_small[3] & 7) << 0x1c,0x77777777);
          vIO32Write4BMsk(0x40e1c,(state.dq_pi[0] & 0x3f) << 0x10,0x3f0000);
          vIO32Write4BMsk(0x40e6c,(state.dq_pi[1] & 0x3f) << 0x10,0x3f0000);
        }
        min_width = saved_min;
      } while (uVar13 == 0xff);
    }
    vSetRank(p,state.local_18);
    p->min_winsize = (ushort)(uint)min_width;
    p->sum_winsize = (ushort)state.local_1c;
    iVar2 = 0;
  }
  return iVar2;
}

int DramcTxVrefDQCalibration(DRAMC_CTX_T *p)
{
  struct {
    ushort local_100[104];
    uint local_30;
    u8 padding_d4[4];
    uint local_28;
    uint local_24;
    uint local_20;
    uint local_1c;
    uint local_18;
  } state;
  ushort uVar1;
  int iVar2;
  ushort *puVar3;
  uint uVar4;
  uint uVar5;
  ushort *local_2c;

  state.local_24 = u4Dram_Register_Read(0x3004c);
  state.local_20 = u4Dram_Register_Read(0x30024);
  state.local_1c = u4Dram_Register_Read(0x30038);
  vIO32Write4BMsk(0x3004c,0x20000000,0x20000000);
  vIO32Write4BMsk(0x30038,0x4000000,0x4000000);
  vIO32Write4BMsk(0x30038,0,0x40000000);
  vIO32Write4BMsk(0x30024,0x40,0x40);
  iVar2 = 1;
  if (p != (DRAMC_CTX_T *)0x0) {
    state.local_18 = u1GetRank(p);
    if (p->dram_type == EN7580_TYPE_PCDDR4) {
      puVar3 = state.local_100;
      do {
        *puVar3 = 0x48;
        puVar3[1] = 0;
        puVar3 = puVar3 + 2;
      } while (puVar3 != state.local_100 + 0x66);
      vSetCalibrationResult(p,8,1);
      DramcModeRegWrite_PC4(p,1,2,0x480);
      pause_polling(100);
      local_2c = state.local_100 + 1;
      state.local_30 = 0;
      do {
        state.local_28 = state.local_30 | 0x40;
        DramcModeRegWrite_PC4(p,1,2,state.local_30 | 0x4c0);
        pause_polling(100);
        (&p->tx_perbit_init)[p->rank] = '\0';
        u4DRAMdebugLOgEnable = 0;
        u4DRAMdebugLOgEnable2 = 0;
        p->sum_winsize = 0;
        DramcTxWindowPerbitCal(p,2);
        uVar1 = p->sum_winsize;
        *local_2c = uVar1;
        u4DRAMdebugLOgEnable = 1;
        u4DRAMdebugLOgEnable2 = 0;
        if (uVar1 != 0) {
          local_2c[-1] = (ushort)state.local_28;
        }
        state.local_30 = state.local_30 + 1 & 0xffff;
        local_2c = local_2c + 2;
      } while (state.local_30 != 0x33);
      puVar3 = state.local_100 + 3;
      uVar5 = 0;
      uVar4 = 1;
      do {
        if (state.local_100[1] < *puVar3) {
          uVar5 = uVar4;
          state.local_100[1] = *puVar3;
        }
        uVar4 = uVar4 + 1 & 0xffff;
        puVar3 = puVar3 + 2;
      } while (uVar4 != 0x33);
      uVar1 = state.local_100[uVar5 * 2];
      DramcModeRegWrite_PC4(p,1,2,uVar1 | 0x480);
      pause_polling(100);
      DramcModeRegWrite_PC4(p,1,2,uVar1 | 0x400);
      pause_polling(100);
    }
    else {
      vSetCalibrationResult(p,8,1);
      DramcModeRegWrite_PC4(p,1,2,0x480);
      pause_polling(100);
    }
    ucDram_Register_Write(0x3004c,state.local_24);
    ucDram_Register_Write(0x30024,state.local_20);
    ucDram_Register_Write(0x30038,state.local_1c);
    vSetRank(p,state.local_18);
    iVar2 = 0;
  }
  return iVar2;
}

void DramcSelftestRun(DRAMC_CTX_T *p)
{
  uint uVar1;
  uint uVar2;

  vIO32Write4BMsk(0x30000,0,0x100000);
  ucDram_Register_Write(0x3009c,0x12000400);
  ucDram_Register_Write(0x30090,0x55aa);
  ucDram_Register_Write(0x30094,0);
  ucDram_Register_Write(0x30098,0x20000000);
  ucDram_Register_Write(0x3009c,0x92000400);
  prom_puts("[DramcSelftestRun] \n");
  while (uVar1 = u4Dram_Register_Read(0x20120), (uVar1 & 1) == 0) {
    pause_polling(1);
  }
  ucDram_Register_Write(0x3009c,0x12000400);
  pause_polling(1);
  ucDram_Register_Write(0x3009c,0x52000400);
  while (uVar1 = u4Dram_Register_Read(0x20120), (uVar1 & 1) == 0) {
    pause_polling(1);
  }
  prom_puts("[DramcSelftestRun] while end\n");
  pause_polling(1);
  uVar1 = u4Dram_Register_Read(0x20124);
  uVar2 = u4Dram_Register_Read(0x20120);
  ucDram_Register_Write(0x3009c,0x2000400);
  pause_polling(1);
  prom_puts("[DramcSelftestRun] result=");
  prom_print_dec((uVar2 & 0x10) >> 4);
  prom_puts("  u4result=0x");
  prom_print_hex(uVar1,8);
  prom_puts("\n");
  return;
}
