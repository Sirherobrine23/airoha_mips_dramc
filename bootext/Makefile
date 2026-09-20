# SPDX-License-Identifier: GPL-2.0+
# Build the standalone EN7528 BootROM/XMODEM bootext image:
#   DRAMC @ 0x9fa30000 -> SRAM XMODEM chainloader @ 0x9fa35000 -> U-Boot.

SRCTREE ?= $(abspath $(CURDIR)/..)
OUT ?= $(SRCTREE)/out/en7528
SOC ?= en7528
HOST_TOOL ?= $(SRCTREE)/out/host/econet-image
UBOOT_LOAD_ADDR ?= 0x81000000
LLVM ?= 0
GNU_CROSS_COMPILE ?=

ifneq ($(SOC),en7528)
$(error bootext is currently only supported for en7528)
endif

CROSS_COMPILE ?= mipsel-linux-gnu-
TOOL_CC ?= $(CROSS_COMPILE)gcc
TOOL_LD ?= $(CROSS_COMPILE)ld
TOOL_OBJCOPY ?= $(CROSS_COMPILE)objcopy
TOOL_NM ?= $(CROSS_COMPILE)nm
TOOL_CC_TARGET ?=

BOOTEXT_BASE := 0x9fa30000
CHAIN_OFFSET := 0x00005000
CHAINLOADER_BASE := 0x9fa35000
BOOTEXT_SRAM_SIZE := 0x0000c000

BUILD := $(OUT)/.bootext
DDR_WORK := $(BUILD)/dramc-work
CHAIN_WORK := $(BUILD)/chainloader-work
DDR_IMAGE := $(BUILD)/dramc.bin
CHAIN_IMAGE := $(BUILD)/chainloader.bin
DDR_ELF := $(DDR_WORK)/.dramc/dramc.elf
OUTPUT ?= $(OUT)/bootext.bin

.PHONY: all clean check-layout
all: $(OUTPUT)

$(BUILD):
	@mkdir -p $@

$(DDR_IMAGE): | $(BUILD)
	+@$(MAKE) --no-print-directory -f $(SRCTREE)/dramc/Makefile \
		SRCTREE="$(SRCTREE)" OUT="$(DDR_WORK)" SOC=en7528 \
		CROSS_COMPILE="$(CROSS_COMPILE)" TOOL_CC="$(TOOL_CC)" \
		TOOL_LD="$(TOOL_LD)" TOOL_OBJCOPY="$(TOOL_OBJCOPY)" \
		TOOL_CC_TARGET="$(TOOL_CC_TARGET)" LLVM="$(LLVM)" \
		GNU_CROSS_COMPILE="$(GNU_CROSS_COMPILE)" \
		DDR_CPPFLAGS="-DBOOTEXT_CHAINLOAD_ENTRY=$(CHAINLOADER_BASE)" \
		OUTPUT="$(DDR_IMAGE)" all

$(CHAIN_IMAGE): | $(BUILD)
	+@$(MAKE) --no-print-directory -f $(SRCTREE)/chainloader/Makefile \
		SRCTREE="$(SRCTREE)" OUT="$(CHAIN_WORK)" SOC=en7528 \
		CROSS_COMPILE="$(CROSS_COMPILE)" TOOL_CC="$(TOOL_CC)" \
		TOOL_LD="$(TOOL_LD)" TOOL_OBJCOPY="$(TOOL_OBJCOPY)" TOOL_NM="$(TOOL_NM)" \
		TOOL_CC_TARGET="$(TOOL_CC_TARGET)" HOST_TOOL="$(HOST_TOOL)" \
		UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" CHAINLOADER_BASE="$(CHAINLOADER_BASE)" \
		OUTPUT="$(CHAIN_IMAGE)" all

check-layout: $(DDR_IMAGE) $(CHAIN_IMAGE)
	@set -eu; \
		end_hex=`$(TOOL_NM) -n "$(DDR_ELF)" | awk '$$3 == "_end" { print $$1; exit }'`; \
		test -n "$$end_hex" || { echo "error: cannot find EN7528 DRAMC _end" >&2; exit 1; }; \
		end=$$((0x$$end_hex)); chain=$$(( $(CHAINLOADER_BASE) )); limit=$$(( $(BOOTEXT_BASE) + $(BOOTEXT_SRAM_SIZE) )); \
		if [ $$end -gt $$chain ]; then \
			echo "error: EN7528 DRAMC runtime end 0x$$end_hex overlaps chainloader at $(CHAINLOADER_BASE)" >&2; exit 1; \
		fi; \
		chain_size=`wc -c < "$(CHAIN_IMAGE)"`; \
		if [ $$((chain + chain_size)) -gt $$limit ]; then \
			echo "error: EN7528 bootext chainloader exceeds FE SRAM window" >&2; exit 1; \
		fi

$(OUTPUT): $(DDR_IMAGE) $(CHAIN_IMAGE) check-layout $(HOST_TOOL)
	@mkdir -p $(dir $@)
	$(HOST_TOOL) bootext --dramc $(DDR_IMAGE) --chainloader $(CHAIN_IMAGE) \
		--chain-offset $(CHAIN_OFFSET) --max-size $(BOOTEXT_SRAM_SIZE) --output $@.tmp
	@mv -f $@.tmp $@

clean:
	@rm -rf $(BUILD) $(OUTPUT)
