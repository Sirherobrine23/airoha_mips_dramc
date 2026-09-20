# SPDX-License-Identifier: GPL-2.0+

VERSION = 2026
PATCHLEVEL = 09
SUBLEVEL =
EXTRAVERSION =

GIT_COMMIT = $(shell git rev-parse --verify HEAD 2>/dev/null)
BUILD_DATE = $(shell date +"%d-%m-%Y %H:%M:%S %z")

__PLAIN_VERSION = $(VERSION)$(if $(PATCHLEVEL),.$(PATCHLEVEL)$(if $(SUBLEVEL),.$(SUBLEVEL)))$(EXTRAVERSION) - $(BUILD_DATE) $(GIT_COMMIT)
ifeq ($(PLAIN_VERSION),)
PLAIN_VERSION = $(__PLAIN_VERSION)
endif
export PLAIN_VERSION

SOCS := en751221 en751627 en7528
TCBOOT_SOCS := en751221 en751627 en7528
BOOTEXT_SOCS := en751627 en7528 en7580

ifeq ($(EXPERIMENTAL_SOCS),1)
SOCS += en7580
endif

O ?= $(CURDIR)/out
UBOOT_LOAD_ADDR ?= 0x81000000
LLVM ?= 0
HOSTCC ?= cc
HOSTCFLAGS ?= -O2 -Wall -Wextra -Werror -std=c11
HOST_TOOL := $(abspath $(O))/host/econet-image

.PHONY: clean all clean test help tcboot bootext $(SOCS) \
	$(addsuffix -tcboot,$(TCBOOT_SOCS)) $(addsuffix -bootext,$(BOOTEXT_SOCS)) \
	en751221-recovery hosttools

all: clean $(SOCS)

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
	@test -n "$(SOC)" || { echo "usage: make tcboot SOC=<en751221|en751627|en7528> UBOOT_IMAGE=/path/to/{u-boot.bin,u-boot.img,u-boot.itb}" >&2; exit 2; }
	+@$(MAKE) --no-print-directory -f "$(CURDIR)/soc/Makefile" \
		SRCTREE="$(CURDIR)" O="$(abspath $(O))" SOC="$(SOC)" \
		HOST_TOOL="$(HOST_TOOL)" UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" \
		UBOOT_IMAGE="$(UBOOT_IMAGE)" LLVM="$(LLVM)" tcboot

$(addsuffix -tcboot,$(TCBOOT_SOCS)): $(HOST_TOOL)
	+@$(MAKE) --no-print-directory -f "$(CURDIR)/soc/Makefile" \
		SRCTREE="$(CURDIR)" O="$(abspath $(O))" SOC="$(patsubst %-tcboot,%,$@)" \
		HOST_TOOL="$(HOST_TOOL)" UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" \
		UBOOT_IMAGE="$(UBOOT_IMAGE)" LLVM="$(LLVM)" tcboot

bootext: $(HOST_TOOL)
	@test -n "$(SOC)" || { echo "usage: make bootext SOC=en7528" >&2; exit 2; }
	+@$(MAKE) --no-print-directory -f "$(CURDIR)/soc/Makefile" \
		SRCTREE="$(CURDIR)" O="$(abspath $(O))" SOC="$(SOC)" \
		HOST_TOOL="$(HOST_TOOL)" UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" \
		LLVM="$(LLVM)" bootext

$(addsuffix -bootext,$(BOOTEXT_SOCS)): $(HOST_TOOL)
	+@$(MAKE) --no-print-directory -f "$(CURDIR)/soc/Makefile" \
		SRCTREE="$(CURDIR)" O="$(abspath $(O))" SOC="$(patsubst %-bootext,%,$@)" \
		HOST_TOOL="$(HOST_TOOL)" UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" \
		LLVM="$(LLVM)" bootext

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
		'EN7528 standalone SRAM bootext:' \
		'  make en7528-bootext' \
		'' \
		'Optional full tcboot.bin packaging:' \
		'  make en751221-tcboot UBOOT_IMAGE=/path/to/{u-boot.bin,u-boot.img,u-boot.itb}' \
		'' \
		'Host image tool:' \
		'  out/host/econet-image' \
		'' \
		'Outputs: out/<soc>/<soc>-dramc.bin and out/<soc>/<soc>-chainload.bin' \
		'EN7528 also emits out/en7528/bootext.bin'
