# SPDX-License-Identifier: GPL-2.0+

ifndef SOC
$(error SOC is required)
endif

SRCTREE ?= $(abspath $(CURDIR))
O ?= $(SRCTREE)/out
OUT := $(abspath $(O))/$(SOC)
PYTHON3 ?= python3
UBOOT_LOAD_ADDR ?= 0x81000000
LLVM ?= 0

SOC_CONFIG := $(SRCTREE)/soc/$(SOC).mk
ifeq ($(wildcard $(SOC_CONFIG)),)
$(error unsupported SoC '$(SOC)')
endif
include $(SOC_CONFIG)

CROSS_COMPILE ?= $(SOC_CROSS_COMPILE)

ifeq ($(LLVM),1)
TOOL_CC ?= clang
TOOL_LD ?= ld.lld
TOOL_OBJCOPY ?= llvm-objcopy
TOOL_NM ?= $(shell command -v llvm-nm >/dev/null 2>&1 && echo llvm-nm || echo nm)
TOOL_CC_TARGET ?= --target=$(SOC_TRIPLE)
else
TOOL_CC ?= $(CROSS_COMPILE)gcc
TOOL_LD ?= $(CROSS_COMPILE)ld
TOOL_OBJCOPY ?= $(CROSS_COMPILE)objcopy
TOOL_NM ?= $(CROSS_COMPILE)nm
TOOL_CC_TARGET ?=
endif

DDR_IMAGE := $(OUT)/$(SOC)-dramc.bin
CHAINLOAD_IMAGE := $(OUT)/$(SOC)-chainload.bin
RECOVERY_IMAGE := $(OUT)/$(SOC)-recovery-chainloader.bin
TCBOOT_IMAGE := $(OUT)/tcboot.bin

TOOL_ENV = \
	srctree="$(SRCTREE)" objtree="$(OUT)" \
	CROSS_COMPILE="$(CROSS_COMPILE)" \
	TOOL_CC="$(TOOL_CC)" TOOL_LD="$(TOOL_LD)" \
	TOOL_OBJCOPY="$(TOOL_OBJCOPY)" TOOL_NM="$(TOOL_NM)" \
	TOOL_CC_TARGET="$(TOOL_CC_TARGET)" \
	PYTHON3="$(PYTHON3)" UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" \
	UBOOT_OFFSET="$(SOC_UBOOT_OFFSET)"

.PHONY: all dramc chainload recovery tcboot clean

all: dramc chainload $(if $(filter y,$(SOC_RECOVERY_CHAINLOADER)),recovery)

dramc:
	@mkdir -p "$(OUT)"
	@$(TOOL_ENV) DDR_OUTPUT="$(DDR_IMAGE)" \
		"$(SRCTREE)/tools/build-econet-ddr.sh" "$(SOC)"
	@printf '  DRAMC      %s\n' "$(DDR_IMAGE)"

chainload:
	@mkdir -p "$(OUT)"
	@$(TOOL_ENV) CHAINLOAD_OUTPUT="$(CHAINLOAD_IMAGE)" \
		"$(SRCTREE)/tools/build-econet-chainload.sh" "$(SOC)"
	@printf '  CHAINLOAD  %s\n' "$(CHAINLOAD_IMAGE)"

recovery:
ifeq ($(SOC_RECOVERY_CHAINLOADER),y)
	@mkdir -p "$(OUT)"
	@$(TOOL_ENV) RECOVERY_CHAINLOADER_OUTPUT="$(RECOVERY_IMAGE)" \
		"$(SRCTREE)/tools/build-econet-chainloader.sh" "$(SOC)"
	@printf '  RECOVERY   %s\n' "$(RECOVERY_IMAGE)"
else
	@echo "error: $(SOC) has no BootROM recovery chainloader" >&2; exit 2
endif

tcboot: dramc
ifeq ($(SOC_TCBOOT),y)
	@test -n "$(UBOOT_IMAGE)" || { echo "error: UBOOT_IMAGE=/path/to/u-boot.img is required" >&2; exit 2; }
	@test -f "$(UBOOT_IMAGE)" || { echo "error: U-Boot image not found: $(UBOOT_IMAGE)" >&2; exit 2; }
	@$(TOOL_ENV) DDR_IMAGE="$(DDR_IMAGE)" \
		UBOOT_IMAGE="$(abspath $(UBOOT_IMAGE))" TCBOOT_OUTPUT="$(TCBOOT_IMAGE)" \
		"$(SRCTREE)/tools/build-econet-flash.sh" "$(SOC)"
	@printf '  TCBOOT     %s\n' "$(TCBOOT_IMAGE)"
else
	@echo "error: standalone tcboot packaging is not implemented for $(SOC)" >&2; exit 2
endif

clean:
	@rm -rf "$(OUT)"
