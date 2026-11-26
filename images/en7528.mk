# SPDX-License-Identifier: GPL-2.0+

define Device/en7528
	SOC_CROSS_COMPILE := mipsel-linux-gnu-
	SOC_TRIPLE := mipsel-linux-gnu
	SOC_ENDIAN := little
	SOC_LD_EMULATION := elf32ltsmip
	SOC_PAYLOAD_OFFSET := 0x00020000
	SOC_TCBOOT := y
	TCBOOT_BOOT2 := y
	TCBOOT_MOVE_DATA_SRC := $$(SRCTREE)/flash/en7528/move_data.S
	TCBOOT_MOVE_DATA_LDS := $$(SRCTREE)/flash/en7528/move_data.lds
	TCBOOT_BOOT2_SRC := $$(SRCTREE)/flash/en7528/boot2.S
	TCBOOT_BOOT2_LDS := $$(SRCTREE)/flash/en7528/boot2.lds
	SOC_BOOTEXT := y

	DRAMC_DIR := $$(SRCTREE)/dramc/en7512/soc/en7528
	DRAMC_SHARED_DIR := $$(SRCTREE)/dramc/en7512/common
	DRAMC_LDS := $$(DRAMC_SHARED_DIR)/ddr.lds
	DRAMC_INCLUDES := -I$$(DRAMC_DIR) -I$$(DRAMC_SHARED_DIR)
	DRAMC_LDFLAGS := --gc-sections
	DRAMC_COMMON_OBJS := uart.o timer.o
	DRAMC_SHARED_OBJS := \
		entry.o \
		dramc_core.o \
		dramc_test.o \
		dramc_dle_cal.o \
		dramc_dq_dqs_cal.o \
		dramc_dqs_gw_cal.o
	DRAMC_OBJS := \
		entry.o \
		dramc_core.o \
		dramc_test.o \
		dramc_dle_cal.o \
		dramc_dq_dqs_cal.o \
		dramc_dqs_gw_cal.o \
		init.o \
		mempll.o \
		system.o \
		uart.o \
		timer.o

	CHAINLOADER_SOC_CPPFLAGS := -DCHAINLOADER_EN7528
	CHAINLOADER_SFC_CPPFLAGS := -DECONET_SFC_TINY_WRITE_ONLY

	BOOTEXT_MODE := unified
	BOOTEXT_BASE := 0x9fa30000
	BOOTEXT_SRAM_SIZE := 0x0000c000
	BOOTEXT_SIZE_OPT := $$(if $$(filter 1,$$(LLVM)),-Oz,-Os)
endef

TARGET_DEVICES += en7528
