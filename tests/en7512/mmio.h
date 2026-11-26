/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef EN7512_TEST_MMIO_H
#define EN7512_TEST_MMIO_H

#ifdef USE_BASELINE
#ifdef PROFILE_EN751221
#include "en751221_ddr.h"
#include <dramc/regs.h>
#define DRAM_PROBE_BASE		DRAM_START
#define DQS_GW_COARSE_COUNT	DQS_GW_COARSE_MAX
#define DQS_GW_FINE_COUNT	DQS_GW_FINE_MAX
#define DQS_GW_WORDS_PER_COARSE	DQS_GW_LEN_PER_COARSE
#define DQS_GW_WORDS		DQS_GW_LEN
#else
#include "en7528_ddr.h"
#ifndef DRAMC_TEST_DM_CMP_CPT
#define DRAMC_TEST_DM_CMP_CPT	BIT(10)
#endif
#endif
#else
#include "dramc_common.h"
#endif

unsigned int test_read(unsigned int offset);
void test_write(unsigned int offset, unsigned int value);
void test_phy_reset(void);

#undef DRAMC_READ_REG
#undef DRAMC_WRITE_REG
#undef DRAMC_WRITE_SET
#undef DRAMC_WRITE_CLEAR
#define DRAMC_READ_REG(off)		test_read(off)
#define DRAMC_WRITE_REG(v, off)		test_write(off, v)
#define DRAMC_WRITE_SET(v, off)		test_write(off, test_read(off) | (v))
#define DRAMC_WRITE_CLEAR(v, off)	test_write(off, test_read(off) & ~(v))

/* Intercept PHY reset calls in both baseline and unified paths. */
#undef DDR_PHY_RESET_NEW
#define DDR_PHY_RESET_NEW()	test_phy_reset()
#undef dramc_phy_reset
#define dramc_phy_reset()		test_phy_reset()
#define en751221_ddr_phy_reset()	test_phy_reset()
#define en7528_ddr_phy_reset()		test_phy_reset()

/* Recreate the old test-table layouts when compiling the Git baseline. */
#ifdef USE_BASELINE
#ifdef PROFILE_EN751221
#define TEST_CASE_TYPE	test_case
#define TEST_CALLBACK	test_case
#else
#define TEST_CASE_TYPE	struct ddr_test_case
#define TEST_CALLBACK	fn
#endif
#else
#define TEST_CASE_TYPE	struct dramc_test_case
#define TEST_CALLBACK	fn
#endif

#endif
