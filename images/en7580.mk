# SPDX-License-Identifier: GPL-2.0+

define Device/en7580
	DEVICE_DEFAULT := n
	EXPERIMENTAL := y
	SOC_CROSS_COMPILE := mipsel-linux-gnu-
	SOC_TRIPLE := mipsel-linux-gnu
	SOC_ENDIAN := little
	SOC_LD_EMULATION := elf32ltsmip
	SOC_PAYLOAD_OFFSET := 0x00060000

	DRAMC_LDS := $$(DRAMC_DIR)/ddr.lds
	DRAMC_INCLUDES := -I$$(DRAMC_DIR)
	DDR_CPPFLAGS := -fno-strict-aliasing
	DRAMC_OBJS := \
		entry.o \
		dramc.o \
		dramc_pi_basic_api.o \
		dramc_pi_calibration_api.o \
		dramc_pi_main.o \
		hal_io.o \
		main.o \
		pkgId.o \
		spram.o \
		system.o \
		data.o \
		glue.o
endef

TARGET_DEVICES += en7580
