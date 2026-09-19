# SPDX-License-Identifier: GPL-2.0+
# Build the EN751221 BootROM XMODEM recovery chainloader.

SRCTREE ?= $(abspath $(CURDIR)/..)
OUT ?= $(SRCTREE)/out/$(SOC)
SOC ?=
UBOOT_LOAD_ADDR ?= 0x81000000
HOST_TOOL ?= $(SRCTREE)/out/host/econet-image

ifneq ($(SOC),en751221)
$(error recovery chainloader is only supported for en751221)
endif

CROSS_COMPILE ?= mips-linux-gnu-
TOOL_CC ?= $(CROSS_COMPILE)gcc
TOOL_LD ?= $(CROSS_COMPILE)ld
TOOL_OBJCOPY ?= $(CROSS_COMPILE)objcopy
TOOL_CC_TARGET ?=

BUILD := $(OUT)/.recovery-chainloader
OUTPUT ?= $(OUT)/$(SOC)-recovery-chainloader.bin
CFLAGS := -EB -mabi=32 -mips32r2 -msoft-float -mno-abicalls -fno-pic \
	-fno-pie -ffreestanding -fno-builtin -fno-stack-protector -Os -G0 -Wall
OBJS := $(BUILD)/start.o $(BUILD)/chainloader.o

.PHONY: all clean
all: $(OUTPUT)

$(BUILD):
	@mkdir -p $@

$(BUILD)/start.o: $(SRCTREE)/chainloader/start.S | $(BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(CFLAGS) -I$(SRCTREE)/include -c $< -o $@

$(BUILD)/chainloader.o: $(SRCTREE)/chainloader/chainloader.c | $(BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(CFLAGS) -I$(SRCTREE)/include \
		-DUBOOT_LOAD_ADDR=$(UBOOT_LOAD_ADDR) -c $< -o $@

$(BUILD)/chainloader.elf: $(OBJS) $(SRCTREE)/chainloader/chainloader.lds
	$(TOOL_LD) -EB -m elf32btsmip -T $(SRCTREE)/chainloader/chainloader.lds \
		-Map $(BUILD)/chainloader.map -o $@ $(OBJS)

$(OUTPUT): $(BUILD)/chainloader.elf $(HOST_TOOL)
	@mkdir -p $(dir $@)
	$(TOOL_OBJCOPY) -O binary $< $(BUILD)/chainloader.raw.bin
	$(HOST_TOOL) chainloader --image $(BUILD)/chainloader.raw.bin --output $@.tmp
	@mv -f $@.tmp $@

clean:
	@rm -rf $(BUILD) $(OUTPUT)
