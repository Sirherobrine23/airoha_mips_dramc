/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef EN7580_DDR_H
#define EN7580_DDR_H
typedef unsigned char u8, byte;
typedef unsigned short u16, ushort;
typedef unsigned int u32, uint, ulong;
typedef unsigned long long u64;
typedef _Bool bool;
typedef void code(void);
#define true 1
#define false 0
/* EN7580 object ABI: the XX230v variant inserts TYPE_PCDDR2 before DDR3. */
enum en7580_dram_type {
  EN7580_TYPE_PCDDR3 = 4,
  EN7580_TYPE_PCDDR4 = 5,
};
typedef struct DRAMC_CTX_T {
  uint support_rank_num, rank, dram_type, odt_onoff, dram_cbt_mode;
  uint DBI_R_onoff, DBI_W_onoff, package, data_width;
  uint test2_1, test2_2, test_pattern, frequency;
  byte vendor_id, pad53; ushort density;
  byte fglow_freq_write_en, ssc_en, en_4bitMux, enable_rx_scan_vref, enable_tx_scan_vref;
  byte pad61[3]; uint pinmux; byte asymmetric, pad69;
  ushort min_winsize, sum_winsize, pad74;
  uint cal_result, cal_executed;
  byte write_leveling_shift, tx_perbit_init, pad86[2];
} DRAMC_CTX_T;
_Static_assert(sizeof(DRAMC_CTX_T) == 88, "EN7580 vendor context ABI");
/* XX230v adds clk_src at offset 52 and moves these fields by four bytes. */
_Static_assert(__builtin_offsetof(DRAMC_CTX_T, frequency) == 48,
               "EN7580 frequency offset");
_Static_assert(__builtin_offsetof(DRAMC_CTX_T, vendor_id) == 52,
               "EN7580 vendor ID offset");
_Static_assert(__builtin_offsetof(DRAMC_CTX_T, pinmux) == 64,
               "EN7580 pinmux offset");

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define BIT(n) (1u << (n))
static inline u32 reg_read(u32 addr) { return *(volatile u32 *)addr; }
static inline void reg_write(u32 addr, u32 value) { *(volatile u32 *)addr = value; }
__attribute__((noreturn)) static inline void trap(uint code) { (void)code; __builtin_trap(); }
extern DRAMC_CTX_T g_dramc_config;
extern DRAMC_CTX_T *pgCurrDramCtx;
extern u32 detected_dram_type, dram_size, sw_impedance[4], uiReg0D0h;
extern u8 fgwrlevel_done, aru1RxDatlatResult;
extern u32 wrlevel_dqs_final_delay[2];
extern u8 dq_ui_large_bak, dq_ui_small_bak, dq_oen_ui_large_bak, dq_oen_ui_small_bak;
extern u8 u1MR2Value;
extern u32 u4DRAMdebugLOgEnable, u4DRAMdebugLOgEnable2;
enum en7580_ac_parameter {
  AC_DRAM_TYPE,
  AC_FREQUENCY,
  AC_DBI_ONOFF,
  AC_BYTE_MODE,
  AC_TRAS,
  AC_TRP,
  AC_TRPAB,
  AC_TRC,
  AC_TRFC,
  AC_TRFCPB,
  AC_TXP,
  AC_TRTP,
  AC_TRCD,
  AC_TWR,
  AC_TWTR,
  AC_TRRD,
  AC_TFAW,
  AC_TRTW_ODT_OFF,
  AC_TRTW_ODT_ON,
  AC_REFCNT,
  AC_REFCNT_FR_CLK,
  AC_TXREFCNT,
  AC_TZQCS,
  AC_TRTPD,
  AC_TWTPD,
  AC_TMRR2W_ODT_OFF,
  AC_TMRR2W_ODT_ON,
  AC_TWRMPR,
  AC_BGTWTR,
  AC_BGTCCD,
  AC_TRAS_05T,
  AC_TRP_05T,
  AC_TRPAB_05T,
  AC_TRC_05T,
  AC_TRFC_05T,
  AC_TRFCPB_05T,
  AC_TXP_05T,
  AC_TRTP_05T,
  AC_TRCD_05T,
  AC_TWR_05T,
  AC_TWTR_05T,
  AC_TRRD_05T,
  AC_TFAW_05T,
  AC_TRTW_05T_ODT_OFF,
  AC_TRTW_05T_ODT_ON,
  AC_BGTWTR_05T,
  AC_BGTCCD_05T,
  AC_TRTPD_05T,
  AC_TWTPD_05T,
  AC_XRTW2W,
  AC_XRTW2R,
  AC_XRTR2W,
  AC_XRTR2R,
  AC_DMCATRAIN_INTV,
  AC_DQSINCTL_FOR_GATING,
  AC_DATLAT,
  AC_MODE_REG_WL,
  AC_MODE_REG_RL,
  AC_PARAMETER_COUNT
};
extern const u16 ac_timing[4][AC_PARAMETER_COUNT];
int prom_printf_s(char *str,...);
uint calculate_dram_size(void);
void dramc_reg_dump(void);
int main(void);
void v4bitPinmuxEnableVerify(DRAMC_CTX_T *p);
u32 DdrUpdateACTimingReg(void);
void Set_MRR_Pinmux_Mapping(DRAMC_CTX_T *p);
void Global_Option_Init(DRAMC_CTX_T *p);
int SscEnable(DRAMC_CTX_T *p);
void DDRPhyPLLSetting_EN7580(DRAMC_CTX_T *p);
void TransferToSPMControl(DRAMC_CTX_T *p);
void Switch26MHzDisableDummyReadRefreshAllBank(DRAMC_CTX_T *p);
void EnableDramcPhyDCM(DRAMC_CTX_T *p,uint bEn);
void HwSaveForSR(DRAMC_CTX_T *p);
void DramcRunTimeConfig(DRAMC_CTX_T *p);
void DramcSetRankEngine2(DRAMC_CTX_T *p,uint u1RankSel);
uint DramcEngine1(DRAMC_CTX_T *p,uint test2_1,uint test2_2,int loopforever,uint period);
int DramcEngine2Init(DRAMC_CTX_T *p,uint test2_1,uint test2_2,uint testaudpat,uint log2loopcount);
uint DramcEngine2Run(DRAMC_CTX_T *p,int wr,uint testaudpat);
void DramcEngine2End(DRAMC_CTX_T *p);
uint DramcEngine2(DRAMC_CTX_T *p,int wr,uint test2_1,uint test2_2,uint testaudpat,int loopforever,
                 uint period,uint log2loopcount);
uint TestEngineCompare(DRAMC_CTX_T *p);
void DramcModeRegWrite_PC3(DRAMC_CTX_T *p,uint u1MRIdx,uint u4Value);
void DramcModeRegWrite_PC4(DRAMC_CTX_T *p,uint u1BGIdx,uint u1MRIdx,uint u4Value);
int DramcInit(DRAMC_CTX_T *p);
void DramReInitForCalibration(DRAMC_CTX_T *p);
void DramcHWDQSGatingTracking_DVT(DRAMC_CTX_T *p);
void DramcHWGatingInit(DRAMC_CTX_T *p);
void DramcHWGatingOnOff(DRAMC_CTX_T *p,uint u1OnOff);
uint Round_Operation(uint A,uint B);
void TxWinTransferDelayToUIPI
               (DRAMC_CTX_T *p,uint uiDelay,uint u1UISmallBak,uint u1UILargeBak,u8 *pu1UILarge,
               u8 *pu1UISmall,u8 *pu1PI);
void Set_RX_DQM_DelayLine_Phy_Byte(DRAMC_CTX_T *p,uint u1ByteIdx,int value);
void Set_RX_DQ_DelayLine_Phy_Byte(DRAMC_CTX_T *p,uint byte_idx,u8 *values);
bool ExecuteMoveDramCDelay(DRAMC_CTX_T *p,uint *regs,int shift);
void MoveDramC_TX_DQS(DRAMC_CTX_T *p,uint u1ByteIdx,int iShiftUI);
void MoveDramC_TX_DQS_OEN(DRAMC_CTX_T *p,uint u1ByteIdx,int iShiftUI);
void MoveDramC_TX_DQ(DRAMC_CTX_T *p,uint u1ByteIdx,int iShiftUI);
void vSetDramMRWriteLevelingOnOff(DRAMC_CTX_T *p,uint u1OnOff);
void vSetRankNumber(DRAMC_CTX_T *p);
void vSetRank(DRAMC_CTX_T *p,uint ucRank);
uint u1GetRank(DRAMC_CTX_T *p);
void vSetCalibrationResult(DRAMC_CTX_T *p,uint calibration,uint result);
void vGetCalibrationResult_All(DRAMC_CTX_T *p,uint u1Rank,uint *u4CalExecute,uint *u4CalResult);
void vPrintCalibrationBasicInfo(DRAMC_CTX_T *p);
void vApplyConfigAfterCalibration(DRAMC_CTX_T *p);
void vApplyConfigBeforeCalibration(DRAMC_CTX_T *p);
void DramPhyReset(DRAMC_CTX_T *p);
void SetRxDqDqsDelay(DRAMC_CTX_T *p,int iDelay);
void dle_factor_handler(DRAMC_CTX_T *p,uint curr_val);
int DramcRankSwap(DRAMC_CTX_T *p,uint u1Rank);
void DramcSwImpedanceSaveRegister(DRAMC_CTX_T *p);
int DramcSwImpedanceCal(DRAMC_CTX_T *p);
void Another_Rank_CKE_low(DRAMC_CTX_T *p,uint on_off);
void O1PathOnOff(DRAMC_CTX_T *p,uint u1OnOff);
int DramcWriteLeveling(DRAMC_CTX_T *p);
int DramcRxdqsGatingCal_PCDDR3(DRAMC_CTX_T *p);
void DramcGatingMode(DRAMC_CTX_T *p,uint u1Mode);
int DramcRxdqsGatingCal_PCDDR4(DRAMC_CTX_T *p);
int DramcRxWindowPerbitCal(DRAMC_CTX_T *p,uint u1UseTestEngine);
uint DramcRxdatlatScan(DRAMC_CTX_T *p,uint max);
int DramcRxdatlatCal(DRAMC_CTX_T *p);
int DramcTxWindowPerbitCal(DRAMC_CTX_T *p,int calType);
int DramcTxVrefDQCalibration(DRAMC_CTX_T *p);
void DramcSelftestRun(DRAMC_CTX_T *p);
void vDramCalibrationAllChannel(DRAMC_CTX_T *p);
void Dramc_DDR_Reserved_Mode_setting(void);
void DPI_SW_main_PCDDR3(void);
void DPI_SW_main_PCDDR4(void);
void SaveCurrDramCtx(DRAMC_CTX_T *p);
uint u4RegBaseAddrTraslate(uint reg);
uint u4Dram_Register_Read(uint reg);
uint ucDram_Register_Write(uint reg,uint val);
void vIO32Write4BMsk(uint reg,uint val,uint mask);
void vIO32Write4B_All(uint reg,uint val);
void vIO32Write4BMsk_All(uint reg,uint val,uint mask);
void start_spram(int calibrate);
u32 efuse_disable(uint param_1);
u32 get_row_col(int param_1,int *param_2,int *param_3);
void efuse_clear_and_enable_int(int param_1);
u32 efuse_read_word(uint param_1,int param_2,u32 *param_3);
int read_efuse(int bank);
int efuse_read_data(int bit,int len,volatile void *buf);
void en7580_setup_clk(uint param_1);
void set_spi_ctrler_ecc(void);
void init_system(int bootrom_ext);
void prom_puts(const char *str);
void prom_print_hex(ulong val, int len);
void prom_print_dec(ulong val);
int get_SYS_HCLK(void);
void pause_polling(int usec);
void time_polling_init(void);
int spram_preprocess(void);
int spram_postprocess(void);
int prom_printf(const char *fmt, ...);
void *memset(void *dst, int value, ulong len);
#endif
