# SPDX-License-Identifier: GPL-2.0+

define Device/en751221
	SOC_CROSS_COMPILE := mips-linux-gnu-
	SOC_TRIPLE := mips-linux-gnu
	SOC_ENDIAN := big
	SOC_LD_EMULATION := elf32btsmip
	SOC_PAYLOAD_OFFSET := 0x00020000
	SOC_TCBOOT := y
	SOC_RECOVERY_CHAINLOADER := y
	SOC_BOOTEXT := y

	DRAMC_LDS := $$(DRAMC_DIR)/ddr.lds
	DRAMC_INCLUDES := -I$$(DRAMC_DIR)
	DRAMC_OBJS := \
		head.o \
		setup.o \
		main.o \
		init.o \
		time.o \
		string.o \
		dramc.o \
		dramc_dq_dqs_cal.o \
		dramc_dle_cal.o \
		dramc_dqs_gw_cal.o \
		en7512_dramc_init.o \
		spram.o

	FLASH_STANDALONE_CPPFLAGS := -DECONET_NAND_PAGE_SHIFT=11
	TCBOOT_BOOT2 := y
	TCBOOT_MOVE_DATA_SRC := $$(SRCTREE)/flash/en751221/move_data.S
	TCBOOT_MOVE_DATA_LDS := $$(SRCTREE)/flash/en751221/move_data.lds
	TCBOOT_BOOT2_SRC := $$(SRCTREE)/flash/en751221/boot2.S
	TCBOOT_BOOT2_LDS := $$(SRCTREE)/flash/en751221/boot2.lds

	CHAINLOADER_BASE := 0x80009000
	CHAINLOADER_SFC_CPPFLAGS := -DECONET_NAND_PAGE_SHIFT=11 -DECONET_SFC_TINY_WRITE_ONLY

	BOOTEXT_MODE := checked-wrapper
	BOOTEXT_BASE := 0x80009000
	BOOTEXT_DDR_ENTRY := 0x9fa32a80
	BOOTEXT_CHAINLOADER_BASE := 0x9fa38000
	BOOTEXT_SRAM_LIMIT := 0x9fa3c000
	BOOTEXT_DDR_MAX_SIZE := 0x5800
	BOOTEXT_CHAIN_MAX_SIZE := 0x4000
	BOOTEXT_SIZE_OPT := $$(if $$(filter 1,$$(LLVM)),-Oz,-Os)
	BOOTEXT_CHAINLOADER_CPPFLAGS := -DEN751221_BOOTEXT
endef

# Check existing artifacts; intentionally does not rebuild firmware.
define Device/en751221/extra
$(call Device/Export,$(1)-bootext-check)
.PHONY: $(1)-bootext-check
$(1)-bootext-check: $$(HOST_TOOL)
	@mkdir -p "$$(BOOTEXT_BUILD)"
	$$(TOOL_NM) -n "$$(BOOTEXT_BUILD)/bootstrap.elf" > "$$(BOOTEXT_BUILD)/bootstrap.nm"
	$$(TOOL_NM) -n "$$(BOOTEXT_DDR_ELF)" > "$$(BOOTEXT_BUILD)/dramc.nm"
	$$(TOOL_NM) -n "$$(BOOTEXT_CHAIN_ELF)" > "$$(BOOTEXT_BUILD)/chainloader.nm"
	$$(HOST_TOOL) check-en751221-bootext \
		--image "$$(BOOTEXT_OUTPUT)" \
		--dramc "$$(BOOTEXT_DDR_IMAGE)" --chainloader "$$(BOOTEXT_CHAIN_IMAGE)" \
		--bootstrap-symbols "$$(BOOTEXT_BUILD)/bootstrap.nm" \
		--dramc-symbols "$$(BOOTEXT_BUILD)/dramc.nm" \
		--chainloader-symbols "$$(BOOTEXT_BUILD)/chainloader.nm"
endef

TARGET_DEVICES += en751221
