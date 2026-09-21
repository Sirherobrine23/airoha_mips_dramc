# SPDX-License-Identifier: GPL-2.0+
# BootROM loads the checked DRAM wrapper at 0x80009000. It relocates the
# in-tree DRAMC and XMODEM receiver into disjoint FE-SRAM ranges.
CROSS_COMPILE ?= mips-linux-gnu-
TOOL_CC ?= $(CROSS_COMPILE)gcc
TOOL_LD ?= $(CROSS_COMPILE)ld
TOOL_OBJCOPY ?= $(CROSS_COMPILE)objcopy
TOOL_NM ?= $(CROSS_COMPILE)nm
TOOL_CC_TARGET ?=
BUILD := $(OUT)/.bootext
DDR_WORK := $(BUILD)/dramc-work
CHAIN_WORK := $(BUILD)/chainloader-work
DDR_IMAGE := $(BUILD)/dramc.bin
CHAIN_IMAGE := $(BUILD)/chainloader.bin
DDR_ELF := $(DDR_WORK)/.dramc/dramc.elf
CHAIN_ELF := $(CHAIN_WORK)/.recovery-chainloader/chainloader.elf
OUTPUT ?= $(OUT)/bootext.bin
# Clang -Os exceeds the fixed SRAM slots. Keep GCC's -Os setting and use
# Clang's size-first mode for these bootext-specific component builds.
BOOTEXT_SIZE_OPT := $(if $(filter 1,$(LLVM)),-Oz,-Os)
FLAGS := -EB -mabi=32 -mips32r2 -msoft-float -mno-abicalls -fno-pic \
	-fno-pie -ffreestanding -fno-builtin -fno-stack-protector $(BOOTEXT_SIZE_OPT) -G0 -Wall
OBJS := $(BUILD)/start.o $(BUILD)/bootstrap.o $(BUILD)/uart.o $(BUILD)/blobs.o

.PHONY: all clean dramc chainloader check-layout
all: $(OUTPUT)

$(OBJS): $(SRCTREE)/bootext/en751221.mk

$(BUILD):
	@mkdir -p $@

dramc: | $(BUILD)
	+$(MAKE) --no-print-directory -f $(SRCTREE)/dramc/Makefile \
		SRCTREE="$(SRCTREE)" OUT="$(DDR_WORK)" SOC=en751221 \
		CROSS_COMPILE="$(CROSS_COMPILE)" TOOL_CC="$(TOOL_CC)" \
		TOOL_LD="$(TOOL_LD)" TOOL_OBJCOPY="$(TOOL_OBJCOPY)" \
		TOOL_CC_TARGET="$(TOOL_CC_TARGET)" LLVM="$(LLVM)" \
		SIZE_OPT="$(BOOTEXT_SIZE_OPT)" OUTPUT="$(DDR_IMAGE)" all

chainloader: | $(BUILD)
	+$(MAKE) --no-print-directory -f $(SRCTREE)/chainloader/Makefile \
		SRCTREE="$(SRCTREE)" OUT="$(CHAIN_WORK)" SOC=en751221 \
		CROSS_COMPILE="$(CROSS_COMPILE)" TOOL_CC="$(TOOL_CC)" \
		TOOL_LD="$(TOOL_LD)" TOOL_OBJCOPY="$(TOOL_OBJCOPY)" TOOL_NM="$(TOOL_NM)" \
		TOOL_CC_TARGET="$(TOOL_CC_TARGET)" HOST_TOOL="$(HOST_TOOL)" \
		UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" CHAINLOADER_BASE=0x9fa38000 \
		CHAINLOADER_CPPFLAGS=-DEN751221_BOOTEXT SIZE_OPT="$(BOOTEXT_SIZE_OPT)" \
		OUTPUT="$(CHAIN_IMAGE)" all

check-layout: dramc chainloader
	@set -eu; \
		ddr_end=`$(TOOL_NM) -n "$(DDR_ELF)" | awk '$$3 == "_end" {print $$1}'`; \
		ddr_start=`$(TOOL_NM) -n "$(DDR_ELF)" | awk '$$3 == "start" {print $$1}'`; \
		chain_end=`$(TOOL_NM) -n "$(CHAIN_ELF)" | awk '$$3 == "__image_end" {print $$1}'`; \
		test -n "$$ddr_end" -a -n "$$ddr_start" -a -n "$$chain_end"; \
		test $$((0x$$ddr_start)) -eq $$((0x9fa32a80)) || { echo 'unexpected DRAMC entry'; exit 1; }; \
		test $$((0x$$ddr_end)) -le $$((0x9fa38000)) || { echo 'DRAMC runtime overlaps SRAM receiver'; exit 1; }; \
		test $$((0x$$chain_end)) -le $$((0x9fa3c000)) || { echo 'receiver exceeds FE SRAM window'; exit 1; }; \
		test $$(wc -c < "$(DDR_IMAGE)") -le $$((0x5800)); \
		test $$(wc -c < "$(CHAIN_IMAGE)") -le $$((0x4000))

$(BUILD)/start.o: $(SRCTREE)/chainloader/start.S | $(BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(FLAGS) -c $< -o $@
$(BUILD)/bootstrap.o: $(SRCTREE)/bootext/en751221-bootstrap.c $(SRCTREE)/chainloader/chainloader.h | $(BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(FLAGS) -I$(SRCTREE)/include -I$(SRCTREE)/chainloader -c $< -o $@
$(BUILD)/uart.o: $(SRCTREE)/chainloader/uart.c $(SRCTREE)/chainloader/chainloader.h | $(BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(FLAGS) -I$(SRCTREE)/include -I$(SRCTREE)/chainloader -c $< -o $@
$(BUILD)/blobs.o: $(SRCTREE)/bootext/en751221-blobs.S check-layout | $(BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(FLAGS) -DSRAM_STAGE_BIN='"$(CHAIN_IMAGE)"' \
		-DDDR_STAGE_BIN='"$(DDR_IMAGE)"' -c $< -o $@
$(BUILD)/bootstrap.elf: $(OBJS) $(SRCTREE)/chainloader/chainloader.lds
	$(TOOL_LD) -EB -m elf32btsmip --defsym CHAINLOADER_BASE=0x80009000 \
		-T $(SRCTREE)/chainloader/chainloader.lds -Map $(BUILD)/bootstrap.map -o $@ $(OBJS)
$(OUTPUT): $(BUILD)/bootstrap.elf $(HOST_TOOL)
	$(TOOL_OBJCOPY) -O binary $< $(BUILD)/bootstrap.raw.bin
	@set -eu; \
		check=`$(TOOL_NM) -n $< | awk '$$3 == "__chk_start" {print $$1}'`; \
		test -n "$$check"; \
		$(HOST_TOOL) chainloader --image $(BUILD)/bootstrap.raw.bin \
			--check-offset $$((0x$$check - 0x80009000)) --output $@.tmp
	@mv -f $@.tmp $@
clean:
	@rm -rf $(BUILD) $(OUTPUT)
