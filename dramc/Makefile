# SPDX-License-Identifier: GPL-2.0+
# Build the standalone DRAM initialization/calibration payload.

SRCTREE ?= $(abspath $(CURDIR)/..)
OUT ?= $(SRCTREE)/out/$(SOC)
SOC ?=
OUTPUT ?= $(OUT)/$(SOC)-dramc.bin

ifeq ($(SOC),en751221)
ENDIAN := -EB
EMULATION := elf32btsmip
CROSS_DEFAULT := mips-linux-gnu-
SOC_DIR := $(SRCTREE)/dramc/en751221
DDR_LDS := $(SOC_DIR)/ddr.lds
ORDERED_OBJS := \
	$(OUT)/.dramc/head.o \
	$(OUT)/.dramc/setup.o \
	$(OUT)/.dramc/main.o \
	$(OUT)/.dramc/init.o \
	$(OUT)/.dramc/time.o \
	$(OUT)/.dramc/string.o \
	$(OUT)/.dramc/dramc.o \
	$(OUT)/.dramc/dramc_dq_dqs_cal.o \
	$(OUT)/.dramc/dramc_dle_cal.o \
	$(OUT)/.dramc/dramc_dqs_gw_cal.o \
	$(OUT)/.dramc/en7512_dramc_init.o \
	$(OUT)/.dramc/spram.o
else ifeq ($(SOC),en751627)
ENDIAN := -EB
EMULATION := elf32btsmip
CROSS_DEFAULT := mips-linux-gnu-
SOC_DIR := $(SRCTREE)/dramc/en751627
PREBUILT := $(SOC_DIR)/en751627_ddr.bin
PREBUILT_SIZE := 53328
else ifeq ($(SOC),en7528)
ENDIAN := -EL
EMULATION := elf32ltsmip
CROSS_DEFAULT := mipsel-linux-gnu-
SOC_DIR := $(SRCTREE)/dramc/en7528
READABLE_DIR := $(SOC_DIR)/readable
DDR_LDS := $(SOC_DIR)/ddr.lds
READABLE_C_SRCS := $(sort $(wildcard $(READABLE_DIR)/*.c))
READABLE_OBJS := $(patsubst $(READABLE_DIR)/%.c,$(OUT)/.dramc/readable/%.o,$(READABLE_C_SRCS))
ORDERED_OBJS := $(OUT)/.dramc/entry.o $(READABLE_OBJS) $(OUT)/.dramc/glue.o
else ifeq ($(SOC),en7580)
ENDIAN := -EL
EMULATION := elf32ltsmip
CROSS_DEFAULT := mipsel-linux-gnu-
SOC_DIR := $(SRCTREE)/dramc/en7580
RECON_DIR := $(SOC_DIR)/reconstructed
DDR_LDS := $(SOC_DIR)/ddr.lds
RECON_ASM_SRCS := $(sort $(wildcard $(RECON_DIR)/*.S))
RECON_OBJS := $(patsubst $(RECON_DIR)/%.S,$(OUT)/.dramc/reconstructed/%.o,$(RECON_ASM_SRCS))
ORDERED_OBJS := $(RECON_OBJS) $(OUT)/.dramc/entry.o $(OUT)/.dramc/glue.o
else
$(error unsupported SOC '$(SOC)')
endif

CROSS_COMPILE ?= $(CROSS_DEFAULT)
TOOL_CC ?= $(CROSS_COMPILE)gcc
TOOL_LD ?= $(CROSS_COMPILE)ld
TOOL_OBJCOPY ?= $(CROSS_COMPILE)objcopy
TOOL_CC_TARGET ?=

BUILD := $(OUT)/.dramc
CFLAGS := $(ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
	-fno-pic -fno-pie -ffreestanding -fno-builtin -Os -G0
CFLAGS_C := $(CFLAGS) -fomit-frame-pointer -fno-stack-protector
ASFLAGS := $(CFLAGS) -D__ASSEMBLY__

.PHONY: all clean check-relocs
all: $(OUTPUT)

$(BUILD):
	@mkdir -p $@

$(BUILD)/readable $(BUILD)/reconstructed:
	@mkdir -p $@

ifeq ($(SOC),en751627)
$(OUTPUT): $(PREBUILT)
	@size=$$(wc -c < "$<"); \
		test "$$size" -eq "$(PREBUILT_SIZE)" || { \
			echo "error: EN751627 DDR payload has size $$size, expected $(PREBUILT_SIZE)" >&2; \
			exit 1; \
		}
	@mkdir -p $(dir $@)
	@cp "$<" "$@.tmp"
	@mv -f "$@.tmp" "$@"
else

ifeq ($(SOC),en751221)
$(BUILD)/%.o: $(SOC_DIR)/%.S | $(BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(ASFLAGS) -I$(SRCTREE)/include -I$(SOC_DIR) \
		-x assembler-with-cpp -c $< -o $@

$(BUILD)/%.o: $(SOC_DIR)/%.c | $(BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(CFLAGS_C) -I$(SRCTREE)/include -I$(SOC_DIR) \
		-c $< -o $@
endif

ifeq ($(SOC),en7528)
$(BUILD)/entry.o: $(SOC_DIR)/entry.S | $(BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(ASFLAGS) -I$(SRCTREE)/include -I$(SOC_DIR) \
		-x assembler-with-cpp -c $< -o $@

$(BUILD)/readable/%.o: $(READABLE_DIR)/%.c | $(BUILD)/readable
	$(TOOL_CC) $(TOOL_CC_TARGET) $(CFLAGS_C) -I$(SRCTREE)/include -I$(SOC_DIR) \
		-I$(READABLE_DIR) -c $< -o $@

$(BUILD)/glue.o: $(SOC_DIR)/glue.c | $(BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(CFLAGS_C) -I$(SRCTREE)/include -I$(SOC_DIR) \
		-I$(READABLE_DIR) -c $< -o $@
endif

ifeq ($(SOC),en7580)
check-relocs:
	@if grep -nE '\.reloc.*,[[:space:]]*$$' $(RECON_ASM_SRCS); then \
		echo "error: DDR source contains unresolved section relocations" >&2; \
		exit 1; \
	fi

$(BUILD)/reconstructed/%.o: $(RECON_DIR)/%.S | $(BUILD)/reconstructed check-relocs
	$(TOOL_CC) $(TOOL_CC_TARGET) $(ASFLAGS) -I$(SRCTREE)/include -I$(SOC_DIR) \
		-x assembler-with-cpp -c $< -o $@

$(BUILD)/entry.o: $(SOC_DIR)/entry.S | $(BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(ASFLAGS) -I$(SRCTREE)/include -I$(SOC_DIR) \
		-x assembler-with-cpp -c $< -o $@

$(BUILD)/glue.o: $(SOC_DIR)/glue.c | $(BUILD)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(CFLAGS_C) -I$(SRCTREE)/include -I$(SOC_DIR) \
		-c $< -o $@
endif

$(BUILD)/dramc.elf: $(ORDERED_OBJS) $(DDR_LDS)
	$(TOOL_LD) $(ENDIAN) -m $(EMULATION) -T $(DDR_LDS) \
		-Map $(BUILD)/dramc.map -o $@ $(ORDERED_OBJS)

$(OUTPUT): $(BUILD)/dramc.elf
	@mkdir -p $(dir $@)
	$(TOOL_OBJCOPY) -O binary $< $@.tmp
	@mv -f $@.tmp $@
endif

clean:
	@rm -rf $(BUILD) $(OUTPUT)
