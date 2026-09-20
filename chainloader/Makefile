# SPDX-License-Identifier: GPL-2.0+
# Build the XMODEM recovery/bootext chainloader.

SRCTREE ?= $(abspath $(CURDIR)/..)
OUT ?= $(SRCTREE)/out/$(SOC)
SOC ?=
UBOOT_LOAD_ADDR ?= 0x81000000
HOST_TOOL ?= $(SRCTREE)/out/host/econet-image

ifeq ($(SOC),en751221)
ENDIAN := -EB
EMULATION := elf32btsmip
CROSS_DEFAULT := mips-linux-gnu-
CHAINLOADER_BASE ?= 0x80009000
SOC_CPPFLAGS :=
SFC_CPPFLAGS := -DECONET_NAND_PAGE_SHIFT=11 -DECONET_SFC_TINY_WRITE_ONLY
else ifeq ($(SOC),en7528)
ENDIAN := -EL
EMULATION := elf32ltsmip
CROSS_DEFAULT := mipsel-linux-gnu-
CHAINLOADER_BASE ?= 0x9fa35000
SOC_CPPFLAGS := -DCHAINLOADER_EN7528
SFC_CPPFLAGS := -DECONET_SFC_TINY_WRITE_ONLY
else
$(error recovery/bootext chainloader is not supported for $(SOC))
endif

CROSS_COMPILE ?= $(CROSS_DEFAULT)
TOOL_CC ?= $(CROSS_COMPILE)gcc
TOOL_LD ?= $(CROSS_COMPILE)ld
TOOL_OBJCOPY ?= $(CROSS_COMPILE)objcopy
TOOL_NM ?= $(CROSS_COMPILE)nm
TOOL_CC_TARGET ?=

BUILD := $(OUT)/.recovery-chainloader
OUTPUT ?= $(OUT)/$(SOC)-recovery-chainloader.bin
CFLAGS := $(ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls -fno-pic \
	-fno-pie -ffreestanding -fno-builtin -fno-stack-protector -Os -G0 -Wall \
	$(SOC_CPPFLAGS)
OBJS := $(BUILD)/start.o $(BUILD)/chainloader.o $(BUILD)/flash-write.o $(BUILD)/early_sfc.o

.PHONY: all clean
all: $(OUTPUT)

$(BUILD):
	@mkdir -p $@

$(BUILD)/start.o: $(SRCTREE)/chainloader/start.S | $(BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(CFLAGS) -I$(SRCTREE)/include -c $< -o $@

$(BUILD)/chainloader.o: $(SRCTREE)/chainloader/chainloader.c $(SRCTREE)/chainloader/chainloader.h | $(BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(CFLAGS) -I$(SRCTREE)/include \
		-DUBOOT_LOAD_ADDR=$(UBOOT_LOAD_ADDR) -c $< -o $@

$(BUILD)/flash-write.o: $(SRCTREE)/chainloader/flash-write.c $(SRCTREE)/chainloader/chainloader.h $(SRCTREE)/flash/sfc.h | $(BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(CFLAGS) -I$(SRCTREE)/include -I$(SRCTREE)/flash \
		-DUBOOT_LOAD_ADDR=$(UBOOT_LOAD_ADDR) -c $< -o $@

$(BUILD)/early_sfc.o: $(SRCTREE)/flash/early_sfc.c $(SRCTREE)/flash/loader.h | $(BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(CFLAGS) $(SFC_CPPFLAGS) -I$(SRCTREE)/flash \
		-c $< -o $@

$(BUILD)/chainloader.elf: $(OBJS) $(SRCTREE)/chainloader/chainloader.lds
	$(TOOL_LD) $(ENDIAN) -m $(EMULATION) --defsym CHAINLOADER_BASE=$(CHAINLOADER_BASE) \
		-T $(SRCTREE)/chainloader/chainloader.lds \
		-Map $(BUILD)/chainloader.map -o $@ $(OBJS)

$(OUTPUT): $(BUILD)/chainloader.elf $(HOST_TOOL)
	@mkdir -p $(dir $@)
	$(TOOL_OBJCOPY) -O binary $< $(BUILD)/chainloader.raw.bin
	@set -eu; \
		start_hex=`$(TOOL_NM) -n $< | awk '$$3 == "__image_start" { print $$1; exit }'`; \
		check_hex=`$(TOOL_NM) -n $< | awk '$$3 == "__chk_start" { print $$1; exit }'`; \
		test -n "$$start_hex" -a -n "$$check_hex"; \
		check_off=$$((0x$$check_hex - 0x$$start_hex)); \
		$(HOST_TOOL) chainloader --image $(BUILD)/chainloader.raw.bin \
			--check-offset "$$check_off" --output $@.tmp
	@mv -f $@.tmp $@

clean:
	@rm -rf $(BUILD) $(OUTPUT)
