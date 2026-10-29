/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef EN7512_TEST_MMIO_H
#define EN7512_TEST_MMIO_H

#include "en7512_soc.h"

unsigned int test_read(unsigned int offset);
void test_write(unsigned int offset, unsigned int value);
void test_phy_reset(void);

#undef DRAMC_READ_REG
#undef DRAMC_WRITE_REG
#undef DRAMC_WRITE_SET
#undef DRAMC_WRITE_CLEAR
#define DRAMC_READ_REG(off)	test_read(off)
#define DRAMC_WRITE_REG(v, off)	test_write(off, v)
#define DRAMC_WRITE_SET(v, off)	test_write(off, test_read(off) | (v))
#define DRAMC_WRITE_CLEAR(v, off)	test_write(off, test_read(off) & ~(v))

/* The production inline reset functions were expanded before interception. */
#undef DDR_PHY_RESET_NEW
#define DDR_PHY_RESET_NEW()	test_phy_reset()
#define en751221_ddr_phy_reset()	test_phy_reset()
#define en7528_ddr_phy_reset()	test_phy_reset()

/* Recreate the old test-table layouts when compiling the Git baseline. */
#ifdef USE_BASELINE
#ifdef PROFILE_EN751221
typedef struct {
	int (*test_case)(unsigned int, unsigned int, void *);
	unsigned int start, range;
	void *ext_arg;
} test_case;
#define TEST_CASE_TYPE test_case
#define TEST_CALLBACK test_case
#else
struct ddr_test_case {
	int (*fn)(unsigned int, unsigned int, void *);
	unsigned int start, len;
	void *arg;
};
#define TEST_CASE_TYPE struct ddr_test_case
#define TEST_CALLBACK fn
#endif
#else
#include "calibration.h"
#define TEST_CASE_TYPE struct dramc_test_case
#define TEST_CALLBACK fn
#endif

#endif
