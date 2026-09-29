# SPDX-License-Identifier: GPL-2.0+

define Device/en7528
	SOC_CROSS_COMPILE := mipsel-linux-gnu-
	SOC_TRIPLE := mipsel-linux-gnu
	SOC_ENDIAN := little
	SOC_LD_EMULATION := elf32ltsmip
	SOC_PAYLOAD_OFFSET := 0x00020000
	SOC_TCBOOT := y
	SOC_BOOTEXT := y

	DRAMC_LDS := $$(DRAMC_DIR)/ddr.lds
	DRAMC_INCLUDES := -I$$(DRAMC_DIR)
	DRAMC_OBJS := \
		entry.o \
		dramc.o \
		dramc_dle_cal.o \
		dramc_dq_dqs_cal.o \
		dramc_dqs_gw_cal.o \
		en7512_dramc_init.o \
		main.o \
		mempll.o \
		system.o \
		glue.o

	CHAINLOADER_BASE := 0x9fa35000
	CHAINLOADER_SOC_CPPFLAGS := -DCHAINLOADER_EN7528
	CHAINLOADER_SFC_CPPFLAGS := -DECONET_SFC_TINY_WRITE_ONLY

	BOOTEXT_MODE := composite
	BOOTEXT_BASE := 0x9fa30000
	BOOTEXT_CHAIN_OFFSET := 0x00005000
	BOOTEXT_CHAINLOADER_BASE := 0x9fa35000
	BOOTEXT_SRAM_SIZE := 0x0000c000
	BOOTEXT_DRAMC_CPPFLAGS := -DBOOTEXT_CHAINLOAD_ENTRY=$$(BOOTEXT_CHAINLOADER_BASE)
endef

TARGET_DEVICES += en7528
