# SPDX-License-Identifier: GPL-2.0+

define Device/en7580
	DEVICE_DEFAULT := n
	EXPERIMENTAL := y
	SOC_CROSS_COMPILE := mipsel-linux-gnu-
	SOC_TRIPLE := mipsel-linux-gnu
	SOC_ENDIAN := little
	SOC_LD_EMULATION := elf32ltsmip
	SOC_PAYLOAD_OFFSET := 0x00060000
	SOC_BOOTEXT := y

	DRAMC_LDS := $$(DRAMC_DIR)/ddr.lds
	DRAMC_INCLUDES := -I$$(DRAMC_DIR)
	# Clang -Os exceeds the SRAM budget; preserve the stack reservation.
	DDR_CPPFLAGS := -fno-strict-aliasing $$(if $$(filter 1,$$(LLVM)),-Oz)
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

	BOOTEXT_MODE := dramc-only
	BOOTEXT_BASE := 0x9fa30000
	BOOTEXT_SRAM_LIMIT := 0x9fa3b000
	BOOTEXT_SIZE_OPT := $$(if $$(filter 1,$$(LLVM)),-Oz,-Os)
	BOOTEXT_DRAMC_CPPFLAGS := -fno-strict-aliasing -DEN7580_BOOTEXT
endef

TARGET_DEVICES += en7580
