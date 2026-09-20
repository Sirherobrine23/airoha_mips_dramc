# SPDX-License-Identifier: GPL-2.0+

ifndef SOC
$(error SOC is required)
endif

SRCTREE ?= $(abspath $(CURDIR))
O ?= $(SRCTREE)/out
OUT := $(abspath $(O))/$(SOC)
UBOOT_LOAD_ADDR ?= 0x81000000
LLVM ?= 0
HOSTCC ?= cc
HOSTCFLAGS ?= -O2 -Wall -Wextra -Werror -std=c11
HOST_TOOL ?= $(abspath $(O))/host/econet-image

SOC_CONFIG := $(SRCTREE)/soc/$(SOC).mk
ifeq ($(wildcard $(SOC_CONFIG)),)
$(error unsupported SoC '$(SOC)')
endif
include $(SOC_CONFIG)

CROSS_COMPILE ?= $(SOC_CROSS_COMPILE)
GNU_CROSS_COMPILE ?= $(SOC_GNU_CROSS_COMPILE)

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
BOOTEXT_IMAGE := $(OUT)/bootext.bin

# Component invocations stay here so the target recipes remain declarative,
# similar to OpenWrt's Build/* pipeline helpers.
define Build/DRAMC
+$(MAKE) --no-print-directory -f "$(SRCTREE)/dramc/Makefile" \
	SRCTREE="$(SRCTREE)" OUT="$(OUT)" SOC="$(SOC)" \
	CROSS_COMPILE="$(CROSS_COMPILE)" TOOL_CC="$(TOOL_CC)" \
	TOOL_LD="$(TOOL_LD)" TOOL_OBJCOPY="$(TOOL_OBJCOPY)" \
	TOOL_CC_TARGET="$(TOOL_CC_TARGET)" LLVM="$(LLVM)" \
	GNU_CROSS_COMPILE="$(GNU_CROSS_COMPILE)" OUTPUT="$(DDR_IMAGE)" all
endef

define Build/Chainload
+$(MAKE) --no-print-directory -f "$(SRCTREE)/flash/Makefile" \
	SRCTREE="$(SRCTREE)" OUT="$(OUT)" SOC="$(SOC)" \
	CROSS_COMPILE="$(CROSS_COMPILE)" TOOL_CC="$(TOOL_CC)" \
	TOOL_LD="$(TOOL_LD)" TOOL_OBJCOPY="$(TOOL_OBJCOPY)" \
	TOOL_CC_TARGET="$(TOOL_CC_TARGET)" UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" \
	UBOOT_OFFSET="$(SOC_UBOOT_OFFSET)" CHAIN_OUTPUT="$(CHAINLOAD_IMAGE)" chainload
endef

define Build/Recovery
+$(MAKE) --no-print-directory -f "$(SRCTREE)/chainloader/Makefile" \
	SRCTREE="$(SRCTREE)" OUT="$(OUT)" SOC="$(SOC)" \
	CROSS_COMPILE="$(CROSS_COMPILE)" TOOL_CC="$(TOOL_CC)" \
	TOOL_LD="$(TOOL_LD)" TOOL_OBJCOPY="$(TOOL_OBJCOPY)" TOOL_NM="$(TOOL_NM)" \
	TOOL_CC_TARGET="$(TOOL_CC_TARGET)" UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" \
	HOST_TOOL="$(HOST_TOOL)" OUTPUT="$(RECOVERY_IMAGE)" all
endef

define Build/Bootext
+$(MAKE) --no-print-directory -f "$(SRCTREE)/bootext/Makefile" \
	SRCTREE="$(SRCTREE)" OUT="$(OUT)" SOC="$(SOC)" \
	CROSS_COMPILE="$(CROSS_COMPILE)" TOOL_CC="$(TOOL_CC)" \
	TOOL_LD="$(TOOL_LD)" TOOL_OBJCOPY="$(TOOL_OBJCOPY)" TOOL_NM="$(TOOL_NM)" \
	TOOL_CC_TARGET="$(TOOL_CC_TARGET)" HOST_TOOL="$(HOST_TOOL)" \
	UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" LLVM="$(LLVM)" \
	GNU_CROSS_COMPILE="$(GNU_CROSS_COMPILE)" OUTPUT="$(BOOTEXT_IMAGE)" all
endef

define Build/TCBoot
+$(MAKE) --no-print-directory -f "$(SRCTREE)/flash/Makefile" \
	SRCTREE="$(SRCTREE)" OUT="$(OUT)" SOC="$(SOC)" \
	CROSS_COMPILE="$(CROSS_COMPILE)" TOOL_CC="$(TOOL_CC)" \
	TOOL_LD="$(TOOL_LD)" TOOL_OBJCOPY="$(TOOL_OBJCOPY)" TOOL_NM="$(TOOL_NM)" \
	TOOL_CC_TARGET="$(TOOL_CC_TARGET)" HOST_TOOL="$(HOST_TOOL)" \
	UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" UBOOT_OFFSET="$(SOC_UBOOT_OFFSET)" \
	DDR_IMAGE="$(DDR_IMAGE)" UBOOT_IMAGE="$(abspath $(UBOOT_IMAGE))" \
	TCBOOT_OUTPUT="$(TCBOOT_IMAGE)" tcboot
endef

.PHONY: all dramc chainload recovery bootext tcboot clean

all: dramc chainload $(if $(filter y,$(SOC_RECOVERY_CHAINLOADER)),recovery) \
	$(if $(filter y,$(SOC_BOOTEXT)),bootext)

$(HOST_TOOL): $(SRCTREE)/tools/econet-image.c
	@mkdir -p "$(dir $@)"
	@tmp="$@.$$$$.tmp"; \
		$(HOSTCC) $(HOSTCFLAGS) "$<" -o "$$tmp" && mv -f "$$tmp" "$@"

dramc:
	@mkdir -p "$(OUT)"
	$(Build/DRAMC)
	@printf '  DRAMC      %s\n' "$(DDR_IMAGE)"

chainload:
	@mkdir -p "$(OUT)"
	$(Build/Chainload)
	@printf '  CHAINLOAD  %s\n' "$(CHAINLOAD_IMAGE)"

recovery: $(HOST_TOOL)
ifeq ($(SOC_RECOVERY_CHAINLOADER),y)
	@mkdir -p "$(OUT)"
	$(Build/Recovery)
	@printf '  RECOVERY   %s\n' "$(RECOVERY_IMAGE)"
else
	@echo "error: $(SOC) has no BootROM recovery chainloader" >&2; exit 2
endif

bootext: $(HOST_TOOL)
ifeq ($(SOC_BOOTEXT),y)
	@mkdir -p "$(OUT)"
	$(Build/Bootext)
	@printf '  BOOTEXT    %s\n' "$(BOOTEXT_IMAGE)"
else
	@echo "error: $(SOC) has no standalone bootext target" >&2; exit 2
endif

tcboot: dramc $(HOST_TOOL)
ifeq ($(SOC_TCBOOT),y)
	@test -n "$(UBOOT_IMAGE)" || { echo "error: UBOOT_IMAGE=<u-boot.bin|u-boot.img|u-boot.itb> is required" >&2; exit 2; }
	@test -f "$(UBOOT_IMAGE)" || { echo "error: U-Boot image not found: $(UBOOT_IMAGE)" >&2; exit 2; }
	$(Build/TCBoot)
	@printf '  TCBOOT     %s\n' "$(TCBOOT_IMAGE)"
else
	@echo "error: standalone tcboot packaging is not implemented for $(SOC)" >&2; exit 2
endif

clean:
	@rm -rf "$(OUT)"
