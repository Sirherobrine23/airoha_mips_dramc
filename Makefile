# SPDX-License-Identifier: GPL-2.0+

VERSION := 2026
PATCHLEVEL := 09
SUBLEVEL := 19

SOCS := en751221 en751627 en7528 en7580
TCBOOT_SOCS := en751221 en751627 en7528

O ?= $(CURDIR)/out
UBOOT_LOAD_ADDR ?= 0x81000000
LLVM ?= 0
HOSTCC ?= cc
HOSTCFLAGS ?= -O2 -Wall -Wextra -Werror -std=c11
HOST_TOOL := $(abspath $(O))/host/econet-image

.PHONY: all clean test help tcboot $(SOCS) \
	$(addsuffix -tcboot,$(TCBOOT_SOCS)) en751221-recovery hosttools

all: $(SOCS)

hosttools: $(HOST_TOOL)

$(HOST_TOOL): $(CURDIR)/tools/econet-image.c
	@mkdir -p "$(dir $@)"
	@tmp="$@.$$$$.tmp"; \
		$(HOSTCC) $(HOSTCFLAGS) "$<" -o "$$tmp" && mv -f "$$tmp" "$@"

$(SOCS): $(HOST_TOOL)
	+@$(MAKE) --no-print-directory -f "$(CURDIR)/soc/Makefile" \
		SRCTREE="$(CURDIR)" O="$(abspath $(O))" SOC="$@" \
		HOST_TOOL="$(HOST_TOOL)" UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" \
		LLVM="$(LLVM)" all

# Optional full TCBoot image packaging. This is intentionally not part of
# `all`: the normal per-SoC build emits DRAMC + chainload and therefore does
# not depend on a U-Boot build tree.
tcboot: $(HOST_TOOL)
	@test -n "$(SOC)" || { echo "usage: make tcboot SOC=<en751221|en751627|en7528> UBOOT_IMAGE=/path/to/u-boot.img" >&2; exit 2; }
	+@$(MAKE) --no-print-directory -f "$(CURDIR)/soc/Makefile" \
		SRCTREE="$(CURDIR)" O="$(abspath $(O))" SOC="$(SOC)" \
		HOST_TOOL="$(HOST_TOOL)" UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" \
		UBOOT_IMAGE="$(UBOOT_IMAGE)" LLVM="$(LLVM)" tcboot

$(addsuffix -tcboot,$(TCBOOT_SOCS)): $(HOST_TOOL)
	+@$(MAKE) --no-print-directory -f "$(CURDIR)/soc/Makefile" \
		SRCTREE="$(CURDIR)" O="$(abspath $(O))" SOC="$(patsubst %-tcboot,%,$@)" \
		HOST_TOOL="$(HOST_TOOL)" UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" \
		UBOOT_IMAGE="$(UBOOT_IMAGE)" LLVM="$(LLVM)" tcboot

en751221-recovery: $(HOST_TOOL)
	+@$(MAKE) --no-print-directory -f "$(CURDIR)/soc/Makefile" \
		SRCTREE="$(CURDIR)" O="$(abspath $(O))" SOC=en751221 \
		HOST_TOOL="$(HOST_TOOL)" UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" \
		LLVM="$(LLVM)" recovery

test: $(HOST_TOOL)
	@"$(HOST_TOOL)" selftest

clean:
	@rm -rf "$(abspath $(O))"

help:
	@printf '%s\n' \
		'Build all supported SoCs:' \
		'  make all' \
		'' \
		'Build DRAMC + chainload for one SoC:' \
		'  make en751221' \
		'  make en751627' \
		'  make en7528' \
		'  make en7580' \
		'' \
		'Use LLVM/Clang instead of the GNU cross toolchain:' \
		'  make LLVM=1 en7528' \
		'' \
		'Optional full tcboot.bin packaging:' \
		'  make en751221-tcboot UBOOT_IMAGE=/path/to/u-boot.img' \
		'' \
		'Host image tool:' \
		'  out/host/econet-image' \
		'' \
		'Outputs: out/<soc>/<soc>-dramc.bin and out/<soc>/<soc>-chainload.bin'
