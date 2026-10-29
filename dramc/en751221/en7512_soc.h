/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef EN7512_SOC_H
#define EN7512_SOC_H

#include "en751221_ddr.h"

#define DRAM_PROBE_BASE			DRAM_START
#define DQS_GW_COARSE_COUNT		DQS_GW_COARSE_MAX
#define DQS_GW_FINE_COUNT		DQS_GW_FINE_MAX
#define DQS_GW_WORDS_PER_COARSE	DQS_GW_LEN_PER_COARSE
#define DQS_GW_WORDS			DQS_GW_LEN
#define DQS_GW_TEST_LENGTH		0xff
#define dramc_phy_reset()		en751221_ddr_phy_reset()

#endif
