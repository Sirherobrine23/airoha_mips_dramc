/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef EN7580_DDR_H
#define EN7580_DDR_H
#include <dramc/en7580/dramc_pi_api.h>
#include <dramc/en7580/x_hal_io.h>
#include <dramc/en7580/dramc_register.h>

typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint, ulong;
typedef void code(void);
#define true 1
#define false 0

#define EN7580_TYPE_PCDDR3 TYPE_PCDDR3
#define EN7580_TYPE_PCDDR4 TYPE_PCDDR4

/* The supplied EN7580 header matches the recovered single-rank object ABI.
 * The separate XX230v EN7523 context has a different layout and enum IDs.
 */
_Static_assert(RANK_MAX == 1, "EN7580 port supports one rank");
_Static_assert(DQ_DATA_WIDTH == 16 && DQS_NUMBER == 2, "EN7580 port uses x16 data");
_Static_assert(sizeof(DRAMC_CTX_T) == 88, "EN7580 vendor context ABI");
_Static_assert(TYPE_PCDDR3 == 4 && TYPE_PCDDR4 == 5, "EN7580 DRAM type IDs");
_Static_assert(PIN_MUX_TYPE_DDR4X16 == 3, "EN7580 DDR4 pinmux ID");
#define CTX_OFFSET(member, value) \
    _Static_assert(__builtin_offsetof(DRAMC_CTX_T, member) == (value), "EN7580 " #member " offset")
CTX_OFFSET(frequency, 48);
CTX_OFFSET(vendor_id, 52);
CTX_OFFSET(pinmux, 64);
CTX_OFFSET(aru4CalResultFlag, 76);
CTX_OFFSET(aru4CalExecuteFlag, 80);
CTX_OFFSET(arfgWriteLevelingInitShif, 84);
CTX_OFFSET(fgTXPerbifInit, 85);
#undef CTX_OFFSET

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
u32 DdrUpdateACTimingReg(void);
void Set_MRR_Pinmux_Mapping(DRAMC_CTX_T *p);
int SscEnable(DRAMC_CTX_T *p);
void DDRPhyPLLSetting_EN7580(DRAMC_CTX_T *p);
void HwSaveForSR(DRAMC_CTX_T *p);
void DramReInitForCalibration(DRAMC_CTX_T *p);
void DramcHWDQSGatingTracking_DVT(DRAMC_CTX_T *p);
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
void vSetCalibrationResult(DRAMC_CTX_T *p,uint calibration,uint result);
void vPrintCalibrationBasicInfo(DRAMC_CTX_T *p);
void SetRxDqDqsDelay(DRAMC_CTX_T *p,int iDelay);
void dle_factor_handler(DRAMC_CTX_T *p,uint curr_val);
void DramcSwImpedanceSaveRegister(DRAMC_CTX_T *p);
void Another_Rank_CKE_low(DRAMC_CTX_T *p,uint on_off);
void O1PathOnOff(DRAMC_CTX_T *p,uint u1OnOff);
void DramcGatingMode(DRAMC_CTX_T *p,uint u1Mode);
uint DramcRxdatlatScan(DRAMC_CTX_T *p,uint max);
void DramcSelftestRun(DRAMC_CTX_T *p);
void vDramCalibrationAllChannel(DRAMC_CTX_T *p);
void DPI_SW_main_PCDDR3(void);
void DPI_SW_main_PCDDR4(void);
uint u4RegBaseAddrTraslate(uint reg);
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
