# SPDX-License-Identifier: GPL-2.0+

# Add cross build with clang and gcc
# 
# the build will be for a single SoC or for all of them—an Openwrt style package build

VERSION := 2026
PATCH := 09
SUBPATCH := 19

O?=out/

PHONY := all
all: clean

clean:
	@rm -rf $(O)
