# SPDX-License-Identifier: GPL-2.0+

VERSION = 2026
PATCHLEVEL = 09
SUBLEVEL =
EXTRAVERSION =

GIT_COMMIT = $(shell git rev-parse --verify HEAD 2>/dev/null)
BUILD_DATE = $(shell date +"%d-%m-%Y %H:%M:%S %z")

PLAIN_VERSION = $(VERSION)$(if $(PATCHLEVEL),.$(PATCHLEVEL)$(if $(SUBLEVEL),.$(SUBLEVEL)))$(EXTRAVERSION) - $(BUILD_DATE) $(GIT_COMMIT)

SOCS := en751221 en751627 en7528
TCBOOT_SOCS := en751221 en751627 en7528
BOOTEXT_SOCS := en751221 en751627 en7528 en7580

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
		LLVM="$(LLVM)" PLAIN_VERSION="$(PLAIN_VERSION)" all

# Optional full TCBoot image packaging. This is intentionally not part of
# `all`: the normal per-SoC build emits DRAMC + chainload and therefore does
# not depend on a U-Boot build tree.
tcboot: $(HOST_TOOL)
	@test -n "$(SOC)" || { echo "usage: make tcboot SOC=<en751221|en751627|en7528> UBOOT_IMAGE=/path/to/{u-boot.bin,u-boot.img,u-boot.itb}" >&2; exit 2; }
	+@$(MAKE) --no-print-directory -f "$(CURDIR)/soc/Makefile" \
		SRCTREE="$(CURDIR)" O="$(abspath $(O))" SOC="$(SOC)" \
		HOST_TOOL="$(HOST_TOOL)" UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" \
		UBOOT_IMAGE="$(UBOOT_IMAGE)" LLVM="$(LLVM)" PLAIN_VERSION="$(PLAIN_VERSION)" tcboot

$(addsuffix -tcboot,$(TCBOOT_SOCS)): $(HOST_TOOL)
	+@$(MAKE) --no-print-directory -f "$(CURDIR)/soc/Makefile" \
		SRCTREE="$(CURDIR)" O="$(abspath $(O))" SOC="$(patsubst %-tcboot,%,$@)" \
		HOST_TOOL="$(HOST_TOOL)" UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" \
		UBOOT_IMAGE="$(UBOOT_IMAGE)" LLVM="$(LLVM)" PLAIN_VERSION="$(PLAIN_VERSION)" tcboot

bootext: $(HOST_TOOL)
	@test -n "$(SOC)" || { echo "usage: make bootext SOC=en7528" >&2; exit 2; }
	+@$(MAKE) --no-print-directory -f "$(CURDIR)/soc/Makefile" \
		SRCTREE="$(CURDIR)" O="$(abspath $(O))" SOC="$(SOC)" \
		HOST_TOOL="$(HOST_TOOL)" UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" \
		LLVM="$(LLVM)" PLAIN_VERSION="$(PLAIN_VERSION)" bootext

$(addsuffix -bootext,$(BOOTEXT_SOCS)): $(HOST_TOOL)
	+@$(MAKE) --no-print-directory -f "$(CURDIR)/soc/Makefile" \
		SRCTREE="$(CURDIR)" O="$(abspath $(O))" SOC="$(patsubst %-bootext,%,$@)" \
		HOST_TOOL="$(HOST_TOOL)" UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" \
		LLVM="$(LLVM)" PLAIN_VERSION="$(PLAIN_VERSION)" bootext

en751221-recovery: $(HOST_TOOL)
	+@$(MAKE) --no-print-directory -f "$(CURDIR)/soc/Makefile" \
		SRCTREE="$(CURDIR)" O="$(abspath $(O))" SOC=en751221 \
		HOST_TOOL="$(HOST_TOOL)" UBOOT_LOAD_ADDR="$(UBOOT_LOAD_ADDR)" \
		LLVM="$(LLVM)" PLAIN_VERSION="$(PLAIN_VERSION)" recovery

test: $(HOST_TOOL)
	@"$(HOST_TOOL)" selftest

# Check existing EN751221 artifacts without rebuilding firmware or needing Python.
.PHONY: en751221-bootext-check
EN751221_TEST_NM = $(if $(filter 1,$(LLVM)),llvm-nm,$(if $(CROSS_COMPILE),$(CROSS_COMPILE),mips-linux-gnu-)nm)
EN751221_TEST_DIR = $(abspath $(O))/en751221/.bootext
en751221-bootext-check: $(HOST_TOOL)
	$(EN751221_TEST_NM) -n "$(EN751221_TEST_DIR)/bootstrap.elf" > "$(EN751221_TEST_DIR)/bootstrap.nm"
	$(EN751221_TEST_NM) -n "$(EN751221_TEST_DIR)/dramc-work/.dramc/dramc.elf" > "$(EN751221_TEST_DIR)/dramc.nm"
	$(EN751221_TEST_NM) -n "$(EN751221_TEST_DIR)/chainloader-work/.recovery-chainloader/chainloader.elf" > "$(EN751221_TEST_DIR)/chainloader.nm"
	$(HOST_TOOL) check-en751221-bootext \
		--image "$(abspath $(O))/en751221/bootext.bin" \
		--dramc "$(EN751221_TEST_DIR)/dramc.bin" --chainloader "$(EN751221_TEST_DIR)/chainloader.bin" \
		--bootstrap-symbols "$(EN751221_TEST_DIR)/bootstrap.nm" \
		--dramc-symbols "$(EN751221_TEST_DIR)/dramc.nm" --chainloader-symbols "$(EN751221_TEST_DIR)/chainloader.nm"

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
