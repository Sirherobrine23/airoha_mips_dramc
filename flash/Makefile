# SPDX-License-Identifier: GPL-2.0+
# Build the post-DRAM chainloader and optional complete TCBoot-compatible image.

SRCTREE ?= $(abspath $(CURDIR)/..)
OUT ?= $(SRCTREE)/out/$(SOC)
SOC ?=
UBOOT_LOAD_ADDR ?= 0x81000000
UBOOT_OFFSET ?= 0x00020000
HOST_TOOL ?= $(abspath $(OUT)/../host/econet-image)

ifeq ($(SOC),en751221)
ENDIAN := -EB
EMULATION := elf32btsmip
CROSS_DEFAULT := mips-linux-gnu-
STANDALONE_FLAGS := -DECONET_NAND_PAGE_SHIFT=11
TCBOOT_SUPPORTED := y
else ifeq ($(SOC),en751627)
ENDIAN := -EB
EMULATION := elf32btsmip
CROSS_DEFAULT := mips-linux-gnu-
STANDALONE_FLAGS :=
TCBOOT_SUPPORTED := y
else ifeq ($(SOC),en7528)
ENDIAN := -EL
EMULATION := elf32ltsmip
CROSS_DEFAULT := mipsel-linux-gnu-
STANDALONE_FLAGS :=
TCBOOT_SUPPORTED := y
else ifeq ($(SOC),en7580)
ENDIAN := -EL
EMULATION := elf32ltsmip
CROSS_DEFAULT := mipsel-linux-gnu-
STANDALONE_FLAGS :=
TCBOOT_SUPPORTED := n
else
$(error unsupported SOC '$(SOC)')
endif

CROSS_COMPILE ?= $(CROSS_DEFAULT)
TOOL_CC ?= $(CROSS_COMPILE)gcc
TOOL_LD ?= $(CROSS_COMPILE)ld
TOOL_OBJCOPY ?= $(CROSS_COMPILE)objcopy
TOOL_NM ?= $(CROSS_COMPILE)nm
TOOL_CC_TARGET ?=

CHAIN_BUILD := $(OUT)/.chainload
CHAIN_OUTPUT ?= $(OUT)/$(SOC)-chainload.bin
TCBOOT_BUILD := $(OUT)/.tcboot
TCBOOT_OUTPUT ?= $(OUT)/tcboot.bin
DDR_IMAGE ?= $(OUT)/$(SOC)-dramc.bin
UBOOT_IMAGE ?=

COMMON_FLAGS := $(ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
	-fno-pic -fno-pie -ffreestanding -fno-builtin -fno-stack-protector -Os -G0
CHAIN_CFLAGS := $(COMMON_FLAGS) -Wall
TCBOOT_CFLAGS := $(COMMON_FLAGS)

CHAIN_OBJS := $(CHAIN_BUILD)/loader-start.o $(CHAIN_BUILD)/loader.o $(CHAIN_BUILD)/early_sfc.o
TCBOOT_LOADER_OBJS := $(TCBOOT_BUILD)/loader-start.o $(TCBOOT_BUILD)/loader.o $(TCBOOT_BUILD)/flash.o
TCBOOT_STAGE_OBJS := $(TCBOOT_BUILD)/start.o $(TCBOOT_BUILD)/payloads.o

.PHONY: all chainload tcboot clean FORCE
all: chainload
chainload: $(CHAIN_OUTPUT)

$(CHAIN_BUILD) $(TCBOOT_BUILD):
	@mkdir -p $@

$(CHAIN_BUILD)/loader-start.o: $(SRCTREE)/flash/loader-start.S | $(CHAIN_BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(CHAIN_CFLAGS) -I$(SRCTREE)/flash -c $< -o $@

$(CHAIN_BUILD)/loader.o: $(SRCTREE)/flash/loader.c $(SRCTREE)/flash/loader.h | $(CHAIN_BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(CHAIN_CFLAGS) -I$(SRCTREE)/flash \
		-DUBOOT_LOAD_ADDR=$(UBOOT_LOAD_ADDR) -DUBOOT_OFFSET=$(UBOOT_OFFSET) \
		-c $< -o $@

$(CHAIN_BUILD)/early_sfc.o: $(SRCTREE)/flash/early_sfc.c | $(CHAIN_BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(CHAIN_CFLAGS) $(STANDALONE_FLAGS) \
		-I$(SRCTREE)/flash -c $< -o $@

$(CHAIN_BUILD)/chainload.elf: $(CHAIN_OBJS) $(SRCTREE)/flash/loader.lds
	$(TOOL_LD) $(ENDIAN) -m $(EMULATION) -T $(SRCTREE)/flash/loader.lds \
		-Map $(CHAIN_BUILD)/chainload.map -o $@ $(CHAIN_OBJS)

$(CHAIN_OUTPUT): $(CHAIN_BUILD)/chainload.elf
	@mkdir -p $(dir $@)
	$(TOOL_OBJCOPY) -O binary $< $@.tmp
	@mv -f $@.tmp $@

ifeq ($(TCBOOT_SUPPORTED),y)
tcboot: FORCE
	@test -n "$(UBOOT_IMAGE)" || { echo "error: UBOOT_IMAGE=/path/to/u-boot.img is required" >&2; exit 2; }
	@test -f "$(UBOOT_IMAGE)" || { echo "error: U-Boot image not found: $(UBOOT_IMAGE)" >&2; exit 2; }
	@test -f "$(DDR_IMAGE)" || { echo "error: DRAMC image not found: $(DDR_IMAGE)" >&2; exit 2; }
	@test -x "$(HOST_TOOL)" || { echo "error: host image tool not found: $(HOST_TOOL)" >&2; exit 2; }
	+@$(MAKE) --no-print-directory -f $(lastword $(MAKEFILE_LIST)) \
		SRCTREE="$(SRCTREE)" OUT="$(OUT)" SOC="$(SOC)" \
		CROSS_COMPILE="$(CROSS_COMPILE)" TOOL_CC="$(TOOL_CC)" TOOL_LD="$(TOOL_LD)" \
		TOOL_OBJCOPY="$(TOOL_OBJCOPY)" TOOL_NM="$(TOOL_NM)" TOOL_CC_TARGET="$(TOOL_CC_TARGET)" \
		UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" UBOOT_IMAGE="$(abspath $(UBOOT_IMAGE))" \
		DDR_IMAGE="$(abspath $(DDR_IMAGE))" HOST_TOOL="$(HOST_TOOL)" TCBOOT_OUTPUT="$(TCBOOT_OUTPUT)" \
		__tcboot
else
tcboot:
	@echo "error: standalone tcboot packaging is not implemented for $(SOC)" >&2; exit 2
endif

.PHONY: __tcboot
__tcboot: $(TCBOOT_OUTPUT)

$(TCBOOT_BUILD)/loader-start.o: $(SRCTREE)/flash/loader-start.S | $(TCBOOT_BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TCBOOT_CFLAGS) -I$(SRCTREE)/flash -c $< -o $@

$(TCBOOT_BUILD)/loader.o: $(SRCTREE)/flash/loader.c $(SRCTREE)/flash/loader.h | $(TCBOOT_BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TCBOOT_CFLAGS) -I$(SRCTREE)/flash \
		-DUBOOT_LOAD_ADDR=$(UBOOT_LOAD_ADDR) -c $< -o $@

$(TCBOOT_BUILD)/flash.o: $(SRCTREE)/flash/early_sfc.c | $(TCBOOT_BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TCBOOT_CFLAGS) $(STANDALONE_FLAGS) \
		-I$(SRCTREE)/flash -c $< -o $@

$(TCBOOT_BUILD)/loader.elf: $(TCBOOT_LOADER_OBJS) $(SRCTREE)/flash/loader.lds
	$(TOOL_LD) $(ENDIAN) -m $(EMULATION) -T $(SRCTREE)/flash/loader.lds \
		-Map $(TCBOOT_BUILD)/loader.map -o $@ $(TCBOOT_LOADER_OBJS)

$(TCBOOT_BUILD)/loader.bin: $(TCBOOT_BUILD)/loader.elf
	$(TOOL_OBJCOPY) -O binary $< $@

$(TCBOOT_BUILD)/start.o: $(SRCTREE)/flash/$(SOC)/start.S | $(TCBOOT_BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TCBOOT_CFLAGS) -D__ASSEMBLY__ \
		-I$(SRCTREE)/include -c $< -o $@

$(TCBOOT_BUILD)/stages.o: $(SRCTREE)/flash/$(SOC)/stages.S | $(TCBOOT_BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TCBOOT_CFLAGS) -I$(SRCTREE)/include -c $< -o $@

ifeq ($(SOC),en751221)
$(TCBOOT_BUILD)/move_data.o: $(SRCTREE)/flash/en751221/move_data.S | $(TCBOOT_BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TCBOOT_CFLAGS) -D__ASSEMBLY__ \
		-I$(SRCTREE)/include -c $< -o $@

$(TCBOOT_BUILD)/boot2.o: $(SRCTREE)/flash/en751221/boot2.S | $(TCBOOT_BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(TCBOOT_CFLAGS) -D__ASSEMBLY__ \
		-I$(SRCTREE)/include -c $< -o $@

$(TCBOOT_BUILD)/move_data.elf: $(TCBOOT_BUILD)/move_data.o $(SRCTREE)/flash/en751221/move_data.lds
	$(TOOL_LD) $(ENDIAN) -m $(EMULATION) -G 0 -static -n -nostdlib --build-id=none \
		-T $(SRCTREE)/flash/en751221/move_data.lds -Map $(TCBOOT_BUILD)/move_data.map \
		-o $@ $(TCBOOT_BUILD)/move_data.o

$(TCBOOT_BUILD)/boot2.elf: $(TCBOOT_BUILD)/boot2.o $(SRCTREE)/flash/en751221/boot2.lds
	$(TOOL_LD) $(ENDIAN) -m $(EMULATION) -G 0 -static -n -nostdlib --build-id=none \
		-T $(SRCTREE)/flash/en751221/boot2.lds -Map $(TCBOOT_BUILD)/boot2.map \
		-o $@ $(TCBOOT_BUILD)/boot2.o

$(TCBOOT_BUILD)/move_data.bin: $(TCBOOT_BUILD)/move_data.elf
	$(TOOL_OBJCOPY) -O binary -j .text $< $@

$(TCBOOT_BUILD)/boot2.bin: $(TCBOOT_BUILD)/boot2.elf
	$(TOOL_OBJCOPY) -O binary -j .text $< $@

$(TCBOOT_BUILD)/payloads.o: $(TCBOOT_BUILD)/stages.o $(TCBOOT_BUILD)/move_data.bin \
		$(TCBOOT_BUILD)/boot2.bin $(TCBOOT_BUILD)/loader.bin $(DDR_IMAGE)
	$(TOOL_OBJCOPY) \
		--add-section .move_data=$(TCBOOT_BUILD)/move_data.bin \
		--set-section-flags .move_data=alloc,load,readonly,code \
		--add-section .boot2=$(TCBOOT_BUILD)/boot2.bin \
		--set-section-flags .boot2=alloc,load,readonly,code \
		--add-section .loader=$(TCBOOT_BUILD)/loader.bin \
		--set-section-flags .loader=alloc,load,readonly,data \
		--add-section .spram=$(DDR_IMAGE) \
		--set-section-flags .spram=alloc,load,readonly,data \
		$(TCBOOT_BUILD)/stages.o $@
else
$(TCBOOT_BUILD)/payloads.o: $(TCBOOT_BUILD)/stages.o $(TCBOOT_BUILD)/loader.bin $(DDR_IMAGE)
	$(TOOL_OBJCOPY) \
		--add-section .loader=$(TCBOOT_BUILD)/loader.bin \
		--set-section-flags .loader=alloc,load,readonly,data \
		--add-section .spram=$(DDR_IMAGE) \
		--set-section-flags .spram=alloc,load,readonly,data \
		$(TCBOOT_BUILD)/stages.o $@
endif

$(TCBOOT_BUILD)/flash.elf: $(TCBOOT_STAGE_OBJS) $(SRCTREE)/flash/flash.lds
	$(TOOL_LD) $(ENDIAN) -m $(EMULATION) -T $(SRCTREE)/flash/flash.lds \
		-Map $(TCBOOT_BUILD)/flash.map -o $@ $(TCBOOT_STAGE_OBJS)

$(TCBOOT_BUILD)/flash.bin: $(TCBOOT_BUILD)/flash.elf
	$(TOOL_OBJCOPY) -O binary $< $@

$(TCBOOT_BUILD)/flash.nm: $(TCBOOT_BUILD)/flash.elf
	$(TOOL_NM) -n $< >$@

$(TCBOOT_OUTPUT): $(TCBOOT_BUILD)/flash.bin $(TCBOOT_BUILD)/flash.nm $(UBOOT_IMAGE) $(HOST_TOOL)
	@mkdir -p $(dir $@)
	$(HOST_TOOL) flash --soc $(SOC) --stages $(TCBOOT_BUILD)/flash.bin \
		--symbols $(TCBOOT_BUILD)/flash.nm --uboot $(UBOOT_IMAGE) \
		--load $(UBOOT_LOAD_ADDR) --output $@

FORCE:

clean:
	@rm -rf $(CHAIN_BUILD) $(TCBOOT_BUILD) $(CHAIN_OUTPUT) $(TCBOOT_OUTPUT)
