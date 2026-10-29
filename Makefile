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
HOST_TOOL := $(abspath $(O))/host/econet-image

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

.PHONY: all clean test test-inspect test-compression test-mips-compression help hosttools tcboot bootext recovery

all: $(BUILD_DEVICES) bootext recovery tcboot

hosttools: $(HOST_TOOL)

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
	@HOSTCC="$(HOSTCC)" python3 "$(SRCTREE)/tests/compression.py" "$(HOST_TOOL)"

test-en7580-c:
	@python3 "$(SRCTREE)/tests/en7580/compare.py"

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
		'  make bootext SOC=en7528' \
		'' \
		'Standalone TCBoot loader (payload is attached later by the image builder):' \
		'  make en751221-tcboot' \
		'  make tcboot SOC=en751221' \
		'' \
		'Host image tool:' \
		'  $(HOST_TOOL)'
