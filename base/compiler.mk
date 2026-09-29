# SPDX-License-Identifier: GPL-2.0+
# Toolchain context.  This macro is evaluated once for every Device/<soc>.
# Do not use ?= here: every device must replace the previous device context.

# Command-line variables still have GNU make's normal higher precedence, so a
# caller may explicitly override CROSS_COMPILE/TOOL_CC/etc.
define compiler/prepare
CROSS_COMPILE := $$(SOC_CROSS_COMPILE)
GNU_CROSS_COMPILE := $$(if $$(SOC_GNU_CROSS_COMPILE),$$(SOC_GNU_CROSS_COMPILE),mips64-linux-gnu-)
SIZE_OPT := -Os

ifeq ($$(SOC_ENDIAN),big)
TARGET_ENDIAN := -EB
else ifeq ($$(SOC_ENDIAN),little)
TARGET_ENDIAN := -EL
else
$$(error unsupported SOC_ENDIAN '$$(SOC_ENDIAN)' for $$(DEVICE_SOC))
endif

ifeq ($$(LLVM),1)
TOOL_CC := clang
TOOL_LD := ld.lld
TOOL_OBJCOPY := llvm-objcopy
TOOL_NM := $$(shell command -v llvm-nm >/dev/null 2>&1 && echo llvm-nm || echo nm)
TOOL_CC_TARGET := --target=$$(SOC_TRIPLE)
else
TOOL_CC := $$(CROSS_COMPILE)gcc
TOOL_LD := $$(CROSS_COMPILE)ld
TOOL_OBJCOPY := $$(CROSS_COMPILE)objcopy
TOOL_NM := $$(CROSS_COMPILE)nm
TOOL_CC_TARGET :=
endif

GNU_CC := $$(GNU_CROSS_COMPILE)gcc
endef
