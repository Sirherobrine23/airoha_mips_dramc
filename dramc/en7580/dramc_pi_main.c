/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "en7580_ddr.h"
void vDramCalibrationAllChannel(DRAMC_CTX_T *p)
{
	vSetRank(p, 0);
	if ((p->support_rank_num == 1 || p->support_rank_num == 2) && p->dram_type != 2)
		DramcWriteLeveling(p);
	if (p->dram_type == EN7580_TYPE_PCDDR3)
		DramcRxdqsGatingCal_PCDDR3(p);
	else
		DramcRxdqsGatingCal_PCDDR4(p);
	DramcRxWindowPerbitCal(p, 1);
	DramcRxdatlatCal(p);
	if (p->dram_type == EN7580_TYPE_PCDDR4)
		DramcTxVrefDQCalibration(p);
	DramcTxWindowPerbitCal(p, 2);
	vSetRank(p, 0);
	v4bitPinmuxEnableVerify(p);
}
void Dramc_DDR_Reserved_Mode_setting(void)
{
	DRAMC_CTX_T unused;
	reg_write(0x10000000, reg_read(0x10000000) | BIT(1));
	/* DramPhyReset uses fixed addresses, not context fields. */
	DramPhyReset(&unused);
}
static void prepare_context(DRAMC_CTX_T *p, u32 type)
{
	p->support_rank_num = 1; p->rank = 0; p->dram_type = type;
	p->dram_cbt_mode = 0; p->DBI_R_onoff = 0; p->DBI_W_onoff = 0;
	p->package = 1; p->data_width = 16;
	p->test2_1 = 0x55000000; p->test2_2 = 0xaa000400; p->test_pattern = 5;
	p->vendor_id = 0x88; p->density = 0;
	p->fglow_freq_write_en = 0; p->ssc_en = 0; p->en_4bitMux = 0;
	p->enable_rx_scan_vref = type == EN7580_TYPE_PCDDR4;
	p->enable_tx_scan_vref = 0; p->write_leveling_shift = 0;
	p->tx_perbit_init = 0; p->asymmetric = 0;
	p->min_winsize = 0; p->sum_winsize = 0;
}
static void print_configuration(DRAMC_CTX_T *p)
{
	prom_puts("dram_type = "); prom_print_dec(p->dram_type);
	prom_puts(", speed = "); prom_print_dec(p->frequency); prom_puts("\n");
}
static void calibrate(DRAMC_CTX_T *p)
{
	Global_Option_Init(p); DramcSwImpedanceCal(p); DramcInit(p);
	vApplyConfigBeforeCalibration(p); vDramCalibrationAllChannel(p);
	vApplyConfigAfterCalibration(p);
}
void DPI_SW_main_PCDDR3(void)
{
	DRAMC_CTX_T *p = &g_dramc_config;
	prepare_context(p, EN7580_TYPE_PCDDR3);
	p->frequency = (reg_read(0xbfb00240) & 3) ? 1200 : 1866;
	p->pinmux = 1; p->odt_onoff = p->frequency >= 1600;
	print_configuration(p); calibrate(p);
	prom_puts("Start Selftest Now...\n"); DramcSelftestRun(p);
}
void DPI_SW_main_PCDDR4(void)
{
	DRAMC_CTX_T *p = &g_dramc_config;
	prepare_context(p, EN7580_TYPE_PCDDR4);
	p->frequency = 1866; p->pinmux = 3; p->odt_onoff = 1;
	print_configuration(p); vPrintCalibrationBasicInfo(p); calibrate(p);
}
