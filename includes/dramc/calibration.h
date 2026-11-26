/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef EN7512_CALIBRATION_H
#define EN7512_CALIBRATION_H

/* The device include path selects the SoC profile at build time. */
#include "en7512_soc.h"

struct dramc_test_case {
	int (*fn)(unsigned int start, unsigned int len, void *arg);
	unsigned int start;
	unsigned int len;
	void *arg;
};

int dramc_dma_test(unsigned int start, unsigned int len, void *arg);
void prom_puts(const char *s);

#endif
