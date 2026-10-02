# SPDX-License-Identifier: GPL-2.0+

VERSION = 2026
PATCHLEVEL = 09
SUBLEVEL =
EXTRAVERSION =

SRCTREE := $(abspath $(CURDIR))
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

.PHONY: all clean test test-compression test-mips-compression help hosttools tcboot bootext recovery

all: $(BUILD_DEVICES) bootext recovery tcboot

hosttools: $(HOST_TOOL)

$(HOST_TOOL): $(SRCTREE)/tools/econet-image.c $(SRCTREE)/flash/ecnt.h
	@mkdir -p "$(dir $@)"
	@tmp="$@.$$$$.tmp"; \
		$(HOSTCC) $(HOSTCFLAGS) "$<" -o "$$tmp" && mv -f "$$tmp" "$@"

test: $(HOST_TOOL)
	@"$(HOST_TOOL)" selftest

test-compression: $(HOST_TOOL)
	@HOSTCC="$(HOSTCC)" python3 "$(SRCTREE)/tests/compression.py" "$(HOST_TOOL)"

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
