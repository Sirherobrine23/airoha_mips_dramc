# SPDX-License-Identifier: GPL-2.0+
# Generic standalone DRAM initialization/calibration build framework.
#
# SoC-wide properties live in soc/<soc>.mk.  Source lists and DRAMC-specific
# quirks live in dramc/<soc>/Makefile.  Keep this file free of per-SoC source
# lists so adding/changing a DRAMC implementation does not grow a central
# ifeq ladder.

DRAMC_MAKEFILE := $(lastword $(MAKEFILE_LIST))
SRCTREE ?= $(abspath $(dir $(DRAMC_MAKEFILE))/..)
OUT ?= $(SRCTREE)/out/$(SOC)
SOC ?=
OUTPUT ?= $(OUT)/$(SOC)-dramc.bin
LLVM ?= 0

SOC_CONFIG := $(SRCTREE)/soc/$(SOC).mk
ifeq ($(wildcard $(SOC_CONFIG)),)
$(error unsupported SOC '$(SOC)')
endif
include $(SOC_CONFIG)

DRAMC_DIR := $(SRCTREE)/dramc/$(SOC)
DRAMC_CONFIG := $(DRAMC_DIR)/Makefile
ifeq ($(wildcard $(DRAMC_CONFIG)),)
$(error missing DRAMC description '$(DRAMC_CONFIG)')
endif
include $(DRAMC_CONFIG)

ifeq ($(SOC_ENDIAN),big)
ENDIAN := -EB
else ifeq ($(SOC_ENDIAN),little)
ENDIAN := -EL
else
$(error unsupported SOC_ENDIAN '$(SOC_ENDIAN)' for $(SOC))
endif

CROSS_COMPILE ?= $(SOC_CROSS_COMPILE)
ifeq ($(LLVM),1)
TOOL_CC ?= clang
TOOL_LD ?= ld.lld
TOOL_OBJCOPY ?= llvm-objcopy
TOOL_CC_TARGET ?= --target=$(SOC_TRIPLE)
else
TOOL_CC ?= $(CROSS_COMPILE)gcc
TOOL_LD ?= $(CROSS_COMPILE)ld
TOOL_OBJCOPY ?= $(CROSS_COMPILE)objcopy
TOOL_CC_TARGET ?=
endif
DDR_CPPFLAGS ?=
SIZE_OPT ?= -Os

# Used only by the EN7580 reconstructed MIPS16 sources when LLVM=1.  A
# mips64-linux-gnu gcc can emit both -EB and -EL O32 objects, so one fallback
# toolchain is enough for the four SoCs supported by this repository.
GNU_CROSS_COMPILE ?= $(if $(SOC_GNU_CROSS_COMPILE),$(SOC_GNU_CROSS_COMPILE),mips64-linux-gnu-)
GNU_CC ?= $(GNU_CROSS_COMPILE)gcc

BUILD := $(OUT)/.dramc
DRAMC_TARGET_OBJS := $(addprefix $(BUILD)/,$(DRAMC_OBJS))
DRAMC_GNU_ASM_TARGETS := $(addprefix $(BUILD)/,$(DRAMC_GNU_ASM_OBJS))

CFLAGS := $(ENDIAN) -mabi=32 -mips32r2 -msoft-float -mno-abicalls \
	-fno-pic -fno-pie -ffreestanding -fno-builtin $(SIZE_OPT) -G0
CFLAGS_C := $(CFLAGS) -fomit-frame-pointer -fno-stack-protector $(DDR_CPPFLAGS)
ASFLAGS := $(CFLAGS) -D__ASSEMBLY__ $(DDR_CPPFLAGS)
CPPFLAGS := -I$(SRCTREE)/include $(DRAMC_INCLUDES)

define filechk_version.h
	(echo \#define PLAIN_VERSION \"$(PLAIN_VERSION)\"; \
	echo \#define CC_VERSION_STRING \"$$(LC_ALL=C $(TOOL_CC) --version | head -n 1)\"; \
	echo \#define LD_VERSION_STRING \"$$(LC_ALL=C $(TOOL_LD) --version | head -n 1)\"; )
endef

# Keep command recipes centralized, similar to OpenWrt's Build/* helpers.
define Build/Compile/C
	$(call filechk_version.h) > $(SRCTREE)/include/tmp_version.h
	@mkdir -p $(dir $@)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(CFLAGS_C) $(CPPFLAGS) -c $< -o $@
endef

define Build/Compile/ASM
	@mkdir -p $(dir $@)
	$(TOOL_CC) $(TOOL_CC_TARGET) $(ASFLAGS) $(CPPFLAGS) \
		-x assembler-with-cpp -c $< -o $@
endef

define Build/Compile/GNUASM
	@mkdir -p $(dir $@)
	$(GNU_CC) $(ASFLAGS) $(CPPFLAGS) -x assembler-with-cpp -c $< -o $@
endef

define Build/Link
	$(TOOL_LD) $(ENDIAN) -m $(SOC_LD_EMULATION) -T $(DRAMC_LDS) \
		-Map $(BUILD)/dramc.map -o $@ $(DRAMC_TARGET_OBJS)
endef

define Build/Objcopy
	@mkdir -p $(dir $@)
	$(TOOL_OBJCOPY) -O binary $< $@.tmp
	@mv -f $@.tmp $@
endef

define Build/CopyPrebuilt
	@size=$$(wc -c < "$<"); \
		test "$$size" -eq "$(DRAMC_PREBUILT_SIZE)" || { \
			echo "error: $(SOC) DRAMC payload has size $$size, expected $(DRAMC_PREBUILT_SIZE)" >&2; \
			exit 1; \
		}
	@mkdir -p $(dir $@)
	@cp "$<" "$@.tmp"
	@mv -f "$@.tmp" "$@"
endef

.PHONY: all clean check-relocs
all: $(OUTPUT)

$(DRAMC_TARGET_OBJS): $(DRAMC_MAKEFILE) $(SOC_CONFIG) $(DRAMC_CONFIG)

$(BUILD):
	@mkdir -p $@

ifdef DRAMC_PREBUILT

$(OUTPUT): $(DRAMC_PREBUILT)
	$(Build/CopyPrebuilt)

else

# Generic rules preserve subdirectories in object names, e.g.
# readable/foo.c -> $(BUILD)/readable/foo.o.
$(BUILD)/%.o: $(DRAMC_DIR)/%.c
	$(Build/Compile/C)

$(BUILD)/%.o: $(DRAMC_DIR)/%.S
	$(Build/Compile/ASM)

ifeq ($(DRAMC_CHECK_RELOCS),y)
check-relocs:
	@if grep -nE '\.reloc.*,[[:space:]]*$$' $(DRAMC_RELOC_SRCS); then \
		echo "error: DRAMC source contains unresolved section relocations" >&2; \
		exit 1; \
	fi
else
check-relocs:
	@:
endif

# Override only the reconstructed EN7580 assembly objects under LLVM.  LLD
# can consume the resulting GNU objects; only LLVM IAS lacks R_MIPS16_26.
ifeq ($(LLVM),1)
ifneq ($(strip $(DRAMC_GNU_ASM_TARGETS)),)
$(DRAMC_GNU_ASM_TARGETS): $(BUILD)/%.o: $(DRAMC_DIR)/%.S | check-relocs
	$(Build/Compile/GNUASM)
endif
endif

$(BUILD)/dramc.elf: $(DRAMC_TARGET_OBJS) $(DRAMC_LDS) | check-relocs
	$(Build/Link)

$(OUTPUT): $(BUILD)/dramc.elf
	$(Build/Objcopy)

endif

clean:
	@rm -rf $(BUILD) $(OUTPUT)
