# SPDX-License-Identifier: GPL-2.0+

define Device/en751627
	EXPERIMENTAL := y
	SOC_CROSS_COMPILE := mips-linux-gnu-
	SOC_TRIPLE := mips-linux-gnu
	SOC_ENDIAN := big
	SOC_LD_EMULATION := elf32btsmip
	SOC_PAYLOAD_OFFSET := 0x00020000
	SOC_TCBOOT := y
	SOC_BOOTEXT := y
	TCBOOT_BOOT2 := y
	TCBOOT_MOVE_DATA_SRC := $$(SRCTREE)/flash/en751627/move_data.S
	TCBOOT_MOVE_DATA_LDS := $$(SRCTREE)/flash/en751627/move_data.lds
	TCBOOT_BOOT2_SRC := $$(SRCTREE)/flash/en751627/boot2.S
	TCBOOT_BOOT2_LDS := $$(SRCTREE)/flash/en751627/boot2.lds
	# Keep the early flash stages small before manufacturing data.
	TCBOOT_SIZE_OPT := -Oz

	DRAMC_DIR := $$(SRCTREE)/dramc/en7512/soc/en751627
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
		dramc_dqs_gw_cal.o \
		mmu.o
	DRAMC_OBJS := \
		entry.o \
		dramc_core.o \
		dramc_test.o \
		dramc_dle_cal.o \
		dramc_dq_dqs_cal.o \
		dramc_dqs_gw_cal.o \
		init.o \
		system.o \
		mmu.o \
		uart.o \
		timer.o

	CHAINLOADER_SOC_CPPFLAGS := -DCHAINLOADER_EN751627
	CHAINLOADER_SOC_OBJS := en751627.o
	CHAINLOADER_SFC_CPPFLAGS := -DECONET_SFC_TINY_WRITE_ONLY

	BOOTEXT_MODE := unified
	BOOTEXT_BASE := 0x9fa30000
	# Keep unified DRAMC + chainloader + CRC table + BSS below the DRAMC stack reserve.
	BOOTEXT_SRAM_SIZE := 0x0000b000
	BOOTEXT_SIZE_OPT := $$(if $$(filter 1,$$(LLVM)),-Oz,-Os)
endef

TARGET_DEVICES += en751627
