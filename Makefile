# SPDX-License-Identifier: GPL-2.0+

VERSION = 2026
PATCHLEVEL = 09
SUBLEVEL =
EXTRAVERSION =

SRCTREE := $(abspath $(CURDIR))
PROJECT_INCLUDES := -I"$(SRCTREE)/includes"
GIT_COMMIT = $(shell git -C "$(SRCTREE)" rev-parse --verify HEAD 2>/dev/null)
BUILD_DATE = $(shell date +"%d-%m-%Y %H:%M:%S %z")

__PLAIN_VERSION = $(VERSION)$(if $(PATCHLEVEL),.$(PATCHLEVEL)$(if $(SUBLEVEL),.$(SUBLEVEL)))$(EXTRAVERSION) - $(BUILD_DATE) $(GIT_COMMIT)
PLAIN_VERSION ?= $(__PLAIN_VERSION)
export PLAIN_VERSION

O ?= $(SRCTREE)/out
LLVM ?= 0
EXPERIMENTAL_SOCS ?= 0
HOSTCC ?= cc
HOSTCFLAGS ?= -O2 -Wall -Wextra -Werror -std=c11
CLANGD_SOC ?= en7528
PYTHON ?= python3
HOST_TOOL := $(abspath $(O))/host/econet-image
RAMBOOT_CROSS_COMPILE ?= mips64-linux-gnu-
RAMBOOT_BUILD := $(abspath $(O))/en7580/.ramboot
RAMBOOT_OUTPUT := $(abspath $(O))/en7580/ram-jump.bin
SRAM_COPY_OUTPUT := $(abspath $(O))/en7580/sram-copy.bin

# Keep SOC as a user-facing selector for `make <type> SOC=<soc>`.  Builders
# use DEVICE_SOC, so evaluating every image context cannot clobber this value.
REQUESTED_SOC := $(strip $(SOC))

include $(SRCTREE)/base/compiler.mk
include $(SRCTREE)/dramc/Makefile
include $(SRCTREE)/flash/Makefile
include $(SRCTREE)/chainloader/Makefile
include $(SRCTREE)/bootext/Makefile
include $(SRCTREE)/images/Makefile

.DEFAULT_GOAL := all

.PHONY: all clean test test-inspect test-compression test-mips-compression help hosttools tcboot bootext recovery en7580-ram-jump en7580-sram-copy

all: $(BUILD_DEVICES) bootext recovery tcboot

hosttools: $(HOST_TOOL)

en7580-ram-jump: $(RAMBOOT_OUTPUT)

en7580-sram-copy: en7580-bootext
	@set -eu; \
		mkdir -p "$(RAMBOOT_BUILD)"; \
		size=$$(wc -c < "$(abspath $(O))/en7580/bootext.bin"); \
		test $$((size % 4)) -eq 0 && test "$$size" -gt 0 && test "$$size" -le 45056 || { \
			echo 'EN7580 DRAMC image must be word-aligned and fit below 0x9fa3b000' >&2; exit 1; }; \
		$(RAMBOOT_CROSS_COMPILE)gcc -EL -mabi=32 -mips32r2 -mno-abicalls -fno-pic \
			-DSRAM_COPY_LENGTH=$$size -c "$(SRCTREE)/tools/en7580-sram-copy.S" \
			-o "$(RAMBOOT_BUILD)/sram-copy.o"; \
		$(RAMBOOT_CROSS_COMPILE)ld -EL -m elf32ltsmip \
			-T "$(SRCTREE)/tools/en7580-sram-copy.lds" \
			"$(RAMBOOT_BUILD)/sram-copy.o" -o "$(RAMBOOT_BUILD)/sram-copy.elf"; \
		$(RAMBOOT_CROSS_COMPILE)objcopy -O binary "$(RAMBOOT_BUILD)/sram-copy.elf" \
			"$(SRAM_COPY_OUTPUT)"

$(RAMBOOT_OUTPUT): $(SRCTREE)/tools/en7580-ram-jump.S $(SRCTREE)/tools/en7580-ram-jump.lds
	@mkdir -p "$(RAMBOOT_BUILD)"
	$(RAMBOOT_CROSS_COMPILE)gcc -EL -mabi=32 -mips32r2 -mno-abicalls -fno-pic \
		-c "$<" -o "$(RAMBOOT_BUILD)/ram-jump.o"
	$(RAMBOOT_CROSS_COMPILE)ld -EL -m elf32ltsmip \
		-T "$(SRCTREE)/tools/en7580-ram-jump.lds" \
		"$(RAMBOOT_BUILD)/ram-jump.o" -o "$(RAMBOOT_BUILD)/ram-jump.elf"
	$(RAMBOOT_CROSS_COMPILE)objcopy -O binary "$(RAMBOOT_BUILD)/ram-jump.elf" "$@"

.PHONY: clangd
clangd:
	@python3 "$(SRCTREE)/tools/clangd.py" --soc "$(CLANGD_SOC)"

$(HOST_TOOL): $(SRCTREE)/tools/econet-image.c $(SRCTREE)/flash/ecnt.h
	@mkdir -p "$(dir $@)"
	@tmp="$@.$$$$.tmp"; \
		$(HOSTCC) $(HOSTCFLAGS) $(PROJECT_INCLUDES) "$<" -o "$$tmp" && mv -f "$$tmp" "$@"

test: $(HOST_TOOL)
	@"$(HOST_TOOL)" selftest

test-inspect: $(HOST_TOOL)
	@$(HOSTCC) $(HOSTCFLAGS) $(PROJECT_INCLUDES) "$(SRCTREE)/tests/inspect.c" -o "$(abspath $(O))/host/test-inspect"
	@"$(abspath $(O))/host/test-inspect" "$(HOST_TOOL)"

test-compression: $(HOST_TOOL)
	@HOSTCC="$(HOSTCC)" $(PYTHON) -P "$(SRCTREE)/tests/compression.py" "$(HOST_TOOL)"

test-en7580-c:
	@LLVM="$(LLVM)" $(PYTHON) "$(SRCTREE)/tests/en7580/compare.py"

.PHONY: test-en7580-c

test-en7512:
	@LLVM="$(LLVM)" python3 "$(SRCTREE)/tests/en7512/compare.py"

.PHONY: test-en7512

test-mips-compression:
	@python3 "$(SRCTREE)/tests/mips-compression.py"

tcboot: $(addsuffix -tcboot,$(TARGET_TCBOOT))
bootext: $(addsuffix -bootext,$(TARGET_BOOTEXT))
recovery: $(addsuffix -recovery,$(TARGET_RECOVERY))

clean:
	@rm -rf "$(abspath $(O))"

help:
	@printf '%s\n' \
		'Build the default SoCs:' \
		'  make all' \
		'' \
		'Build one SoC:' \
		'  make en751221' \
		'  make en751627' \
		'  make en7528' \
		'  make en7580                 # experimental' \
		'' \
		'Include non-default experimental SoCs in all:' \
		'  make EXPERIMENTAL_SOCS=1 all' \
		'' \
		'Use LLVM/Clang:' \
		'  make LLVM=1 en7528' \
		'' \
		'Generate clangd editor commands (no firmware build):' \
		'  make clangd' \
		'  make clangd CLANGD_SOC=en751221' \
		'' \
		'Standalone boot extension:' \
		'  make en7528-bootext' \
		'  make en751627-bootext' \
		'  make en7580-bootext       # DDR diagnostic in SRAM' \
		'  make en7580-ram-jump      # tcboot RAM handoff to U-Boot' \
		'  make en7580-sram-copy     # tcboot SRAM DRAMC diagnostic handoff' \
		'  make bootext SOC=en7528' \
		'' \
		'Standalone TCBoot loader (payload is attached later by the image builder):' \
		'  make en751221-tcboot' \
		'  make tcboot SOC=en751221' \
		'' \
		'Host image tool:' \
		'  $(HOST_TOOL)'
