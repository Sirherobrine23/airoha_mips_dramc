/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en7580_ddr.h"
static u32 regs[0x40000 / 4];
static u32 last_reg, ops, reads, writes, hash = 2166136261u, hash2 = 0x12345678u, seed;
static DRAMC_CTX_T ctx;
static void output(int ret);
static int sys_write(void *data, uint len)
{
 register int v0 __asm__("$2") = 4004;
 register int a0 __asm__("$4") = 1;
 register void *a1 __asm__("$5") = data;
 register uint a2 __asm__("$6") = len;
 __asm__ volatile("syscall" : "+r"(v0) : "r"(a0), "r"(a1), "r"(a2) : "$7", "memory");
 return v0;
}
__attribute__((noreturn)) static void finish(int ret)
{
 output(ret);
 register int v0 __asm__("$2") = 4001;
 register int a0 __asm__("$4") = 0;
 __asm__ volatile("syscall" : "+r"(v0) : "r"(a0) : "$7", "memory");
 __builtin_unreachable();
}
static void record(u32 kind, u32 reg, u32 val)
{
 last_reg = reg;
 hash = (hash ^ kind) * 16777619u;
 hash = (hash ^ reg) * 16777619u;
 hash = (hash ^ val) * 16777619u;
 hash2 = (hash2 << 7 | hash2 >> 25) + reg + val + kind;
 if (++ops > 2000000) finish(-999);
}
uint u4Dram_Register_Read(uint reg)
{
 uint val = 0;
 if (reg >= 0x20000 && reg < 0x60000) val = regs[(reg - 0x20000) / 4];
 if (reg == 0x20088 || reg == 0x20120) val = 0xffffffffu;
 if (reg == 0x20124) val = seed;
 if (reg == 0x20140 || reg == 0x20144) val = 0xffffffffu;
 reads++; record(1, reg, val); return val;
}
u8 ucDram_Register_Write(uint reg, uint val)
{
 if (reg >= 0x20000 && reg < 0x60000) regs[(reg - 0x20000) / 4] = val;
 writes++; record(2, reg, val); return 0;
}
void vIO32Write4BMsk(uint reg, uint val, uint mask)
{
 ucDram_Register_Write(reg, (u4Dram_Register_Read(reg) & ~mask) | (val & mask));
}
void vIO32Write4BMsk_All(uint reg, uint val, uint mask)
{
 uint count = 1;
 if ((reg & 0xf0000) > 0x3ffff) { reg = (reg & 0xffff) + 0x40000; count = 2; }
 for (uint i = 0; i < count; i++)
  ucDram_Register_Write(reg + i * 0x10000, (u4Dram_Register_Read(reg + i * 0x10000) & ~mask) | val);
}
void vIO32Write4B_All(uint reg, uint val)
{
 uint count = 1;
 if ((reg & 0xf0000) > 0x3ffff) { reg = (reg & 0xffff) + 0x40000; count = 2; }
 for (uint i = 0; i < count; i++) ucDram_Register_Write(reg + i * 0x10000, val);
}
#ifdef USE_ORIGINAL
DRAMC_CTX_T *pgCurrDramCtx;
#endif
void SaveCurrDramCtx(DRAMC_CTX_T *p) { pgCurrDramCtx = p; }
void pause_polling(int us) { record(3, 0, us); }
void prom_puts(const char *s) { (void)s; }
void prom_print_hex(ulong v, int l) { (void)v; (void)l; }
void prom_print_dec(ulong v) { (void)v; }
int prom_printf(const char *s, ...) { (void)s; return 0; }
void *memset(void *dst, int v, ulong len)
{
 byte *b = dst; while (len--) *b++ = v; return dst;
}
void *memcpy(void *dst, const void *src, ulong len)
{
 byte *d = dst;
 const byte *s = src;
 while (len--) *d++ = *s++;
 return dst;
}
static void output(int ret)
{
 u32 out[] = { ret, ops, reads, writes, hash, hash2, last_reg };
 sys_write(out, sizeof(out)); sys_write(&ctx, sizeof(ctx));
}
__attribute__((noinline)) static void poison_stack(u32 pattern)
{
 volatile u32 scratch[512];
 for (uint i = 0; i < 512; i++) scratch[i] = pattern;
}
int guard_test(int argc, char **argv)
{
 uint n = 0, profile = 0, mode = 0;
 if (argc > 1) for (char *p = argv[1]; *p; p++) n = n * 10 + *p - '0';
 if (argc > 2) for (char *p = argv[2]; *p; p++) profile = profile * 10 + *p - '0';
 seed = (profile >> 3) & 1 ? 0xffffu : 0;
 ctx.support_rank_num = 1; ctx.dram_type = profile & 1 ? 5 : 4;
 ctx.data_width = 16; ctx.test2_1 = 0x55000000; ctx.test2_2 = 0xaa000400;
 ctx.test_pattern = 5; ctx.frequency = profile & 2 ? 1866 : 1600;
 ctx.pinmux = profile & 1 ? 3 : 1; ctx.odt_onoff = (profile >> 2) & 1;
 ctx.vendor_id = 0x88; pgCurrDramCtx = &ctx;
 switch (n) {
 case 0: DDRPhyPLLSetting_EN7580(&ctx); break;
 case 1: ctx.frequency = 1200; DDRPhyPLLSetting_EN7580(&ctx); break;
 case 2: ctx.frequency = 1333; DDRPhyPLLSetting_EN7580(&ctx); break;
 case 3: mode = DramcInit(&ctx); break;
 case 4: vApplyConfigBeforeCalibration(&ctx); break;
 case 5: vApplyConfigAfterCalibration(&ctx); break;
 case 6: mode = DramcSwImpedanceCal(&ctx); break;
 case 7: mode = DramcWriteLeveling(&ctx); break;
 case 8: mode = DramcRxdqsGatingCal_PCDDR3(&ctx); break;
 case 9: mode = DramcRxdqsGatingCal_PCDDR4(&ctx); break;
 case 10: mode = DramcRxWindowPerbitCal(&ctx, 1); break;
 case 11: mode = DramcRxdatlatCal(&ctx); break;
 case 12: mode = DramcTxWindowPerbitCal(&ctx, 2); break;
 case 13: ctx.enable_tx_scan_vref = 1; mode = DramcTxVrefDQCalibration(&ctx); break;
 case 14: DramcModeRegWrite_PC3(&ctx, 1, 0x123); break;
 case 15: DramcModeRegWrite_PC4(&ctx, 0, 1, 0x123); break;
 case 16: DramcRunTimeConfig(&ctx); break;
 case 17: ctx.enable_rx_scan_vref = 1; mode = DramcRxWindowPerbitCal(&ctx, 1); break;
 case 18: mode = Round_Operation(profile * 77, profile % 5); break;
 case 19: MoveDramC_TX_DQS(&ctx, 0, -1); MoveDramC_TX_DQS_OEN(&ctx, 1, 2); MoveDramC_TX_DQ(&ctx, 0, -1); break;
 case 20: mode = DramcEngine2(&ctx, 0, ctx.test2_1, ctx.test2_2, 5, 0, 0, 0); break;
 case 21: DramcSelftestRun(&ctx); break;
 case 22: DramcHWGatingInit(&ctx); DramcHWGatingOnOff(&ctx, 1); break;
 case 23: poison_stack(profile & 16 ? 0x55555555 : 0xaaaaaaaa); vDramCalibrationAllChannel(&ctx); break;
 case 24: {
  DRAMC_CTX_T saved;
  u32 executed = ~0u, result = ~0u;
  vSetCalibrationResult(&ctx, DRAM_CALIBRATION_SW_IMPEDANCE, DRAM_FAIL);
  if (ctx.aru4CalResultFlag[0] != BIT(DRAM_CALIBRATION_SW_IMPEDANCE) ||
      ctx.aru4CalExecuteFlag[0] != BIT(DRAM_CALIBRATION_SW_IMPEDANCE)) return 10;
  vSetRank(&ctx, 1);
  if (u1GetRank(&ctx) != RANK_0) return 11;
  DramcSetRankEngine2(&ctx, 1);
  if (DramcRankSwap(&ctx, 1) != DRAM_FAIL || ops) return 18;
  ctx.rank = 1;
  saved = ctx;
  vSetCalibrationResult(&ctx, DRAM_CALIBRATION_GATING, DRAM_FAIL);
  for (uint i = 0; i < sizeof(ctx); i++)
   if (((u8 *)&saved)[i] != ((u8 *)&ctx)[i]) return 12;
  vGetCalibrationResult_All(&ctx, 1, &executed, &result);
  if (executed || result) return 13;
  ctx.rank = RANK_0;
  vSetCalibrationResult(&ctx, DRAM_CALIBRATION_SW_IMPEDANCE, DRAM_OK);
  if (ctx.aru4CalResultFlag[0] ||
      ctx.aru4CalExecuteFlag[0] != BIT(DRAM_CALIBRATION_SW_IMPEDANCE)) return 14;
  saved = ctx;
  vSetCalibrationResult(&ctx, 32, DRAM_FAIL);
  for (uint i = 0; i < sizeof(ctx); i++)
   if (((u8 *)&saved)[i] != ((u8 *)&ctx)[i]) return 15;
  break;
 }
 case 25: {
  vIO32WriteFldMulti(DRAMC_REG_DDRCONF0, P_Fld(0x89abcdefu, RG_FLD_FULL));
  if (u4Dram_Register_Read(DRAMC_REG_DDRCONF0) != 0x89abcdefu) return 16;
  vIO32WriteFldMulti(DRAMC_REG_DDRCONF0,
   P_Fld(0, DDRCONF0_DRAMC_SW_RST) | P_Fld(0xf, DDRCONF0_FIFOLEN));
  if (u4Dram_Register_Read(DRAMC_REG_DDRCONF0) != 0x09abcdffu) return 17;
  break;
 }
 default: return 5;
 }
 output(mode); return 0;
}
