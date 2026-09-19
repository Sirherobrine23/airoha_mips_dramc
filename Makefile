# SPDX-License-Identifier: GPL-2.0+

VERSION := 2026
PATCHLEVEL := 09
SUBLEVEL := 19

SOCS := en751221 en751627 en7528 en7580
TCBOOT_SOCS := en751221 en751627 en7528

O ?= $(CURDIR)/out
PYTHON3 ?= python3
UBOOT_LOAD_ADDR ?= 0x81000000
LLVM ?= 0

.PHONY: all clean test help tcboot $(SOCS) \
	$(addsuffix -tcboot,$(TCBOOT_SOCS)) en751221-recovery

all: $(SOCS)

$(SOCS):
	+@$(MAKE) --no-print-directory -f "$(CURDIR)/soc/Makefile" \
		SRCTREE="$(CURDIR)" O="$(abspath $(O))" SOC="$@" \
		PYTHON3="$(PYTHON3)" UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" \
		LLVM="$(LLVM)" all

# Optional full TCBoot image packaging.  This is intentionally not part of
# `all`: the normal per-SoC build only emits DRAMC + the standalone chainload
# stage and therefore does not depend on a U-Boot build tree.
tcboot:
	@test -n "$(SOC)" || { echo "usage: make tcboot SOC=<en751221|en751627|en7528> UBOOT_IMAGE=/path/to/u-boot.img" >&2; exit 2; }
	+@$(MAKE) --no-print-directory -f "$(CURDIR)/soc/Makefile" \
		SRCTREE="$(CURDIR)" O="$(abspath $(O))" SOC="$(SOC)" \
		PYTHON3="$(PYTHON3)" UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" \
		UBOOT_IMAGE="$(UBOOT_IMAGE)" LLVM="$(LLVM)" tcboot

$(addsuffix -tcboot,$(TCBOOT_SOCS)):
	+@$(MAKE) --no-print-directory -f "$(CURDIR)/soc/Makefile" \
		SRCTREE="$(CURDIR)" O="$(abspath $(O))" SOC="$(patsubst %-tcboot,%,$@)" \
		PYTHON3="$(PYTHON3)" UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" \
		UBOOT_IMAGE="$(UBOOT_IMAGE)" LLVM="$(LLVM)" tcboot

en751221-recovery:
	+@$(MAKE) --no-print-directory -f "$(CURDIR)/soc/Makefile" \
		SRCTREE="$(CURDIR)" O="$(abspath $(O))" SOC=en751221 \
		PYTHON3="$(PYTHON3)" UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" \
		LLVM="$(LLVM)" recovery

test:
	@PYTHONPATH="$(CURDIR)/tools" "$(PYTHON3)" -m unittest discover \
		-s "$(CURDIR)/tools" -p 'test_*.py' -v

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
		'Outputs: out/<soc>/<soc>-dramc.bin and out/<soc>/<soc>-chainload.bin'
