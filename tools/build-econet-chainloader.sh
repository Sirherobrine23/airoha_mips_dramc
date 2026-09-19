#!/bin/sh
# SPDX-License-Identifier: GPL-2.0+
# Build the EN751221 BootROM XMODEM recovery chainloader.
set -eu

soc="${1:-}"
case "$soc" in
 en751221)
  endian=-EB
  emulation=elf32btsmip
  cross_default=mips-linux-gnu-
  ;;
 *)
  echo "unsupported recovery chainloader SoC: $soc" >&2
  exit 2
  ;;
esac

srctree="${srctree:-$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)}"
objtree="${objtree:-$srctree/out/$soc}"
UBOOT_LOAD_ADDR="${UBOOT_LOAD_ADDR:-0x81000000}"

cross="${CROSS_COMPILE:-$cross_default}"
cc="${TOOL_CC:-${cross}gcc}"
ld="${TOOL_LD:-${cross}ld}"
objcopy="${TOOL_OBJCOPY:-${cross}objcopy}"
cc_target="${TOOL_CC_TARGET:-}"

src="$srctree/chainloader"
build="$objtree/.recovery-chainloader"
out="${RECOVERY_CHAINLOADER_OUTPUT:-$objtree/${soc}-recovery-chainloader.bin}"
mkdir -p "$build" "$(dirname -- "$out")"

flags="$endian -mabi=32 -mips32r2 -msoft-float -mno-abicalls -fno-pic -fno-pie \
 -ffreestanding -fno-builtin -fno-stack-protector -Os -G0 -Wall"
"$cc" $cc_target $flags -I"$srctree/include" -c "$src/start.S" -o "$build/start.o"
"$cc" $cc_target $flags -I"$srctree/include" -DUBOOT_LOAD_ADDR="$UBOOT_LOAD_ADDR" \
 -c "$src/chainloader.c" -o "$build/chainloader.o"
"$ld" $endian -m "$emulation" -T "$src/chainloader.lds" \
 -Map "$build/chainloader.map" -o "$build/chainloader.elf" \
 "$build/start.o" "$build/chainloader.o"
"$objcopy" -O binary "$build/chainloader.elf" "$out"
"${PYTHON3:-python3}" "$srctree/tools/econet_chainloader_image.py" "$out"
