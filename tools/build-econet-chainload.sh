#!/bin/sh
# SPDX-License-Identifier: GPL-2.0+
# Build the small post-DRAM flash reader which loads an uncompressed U-Boot
# legacy image from flash and jumps to it.
set -eu

soc="${1:-}"
srctree="${srctree:-$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)}"
objtree="${objtree:-$srctree/out/$soc}"
UBOOT_LOAD_ADDR="${UBOOT_LOAD_ADDR:-0x81000000}"
UBOOT_OFFSET="${UBOOT_OFFSET:-0x00020000}"

case "$soc" in
 en751221)
  endian=-EB
  emulation=elf32btsmip
  cross_default=mips-linux-gnu-
  standalone_flags="-DECONET_NAND_PAGE_SHIFT=11"
  ;;
 en751627)
  endian=-EB
  emulation=elf32btsmip
  cross_default=mips-linux-gnu-
  standalone_flags=
  ;;
 en7528|en7580)
  endian=-EL
  emulation=elf32ltsmip
  cross_default=mipsel-linux-gnu-
  standalone_flags=
  ;;
 *)
  echo "usage: $0 {en751221|en751627|en7528|en7580}" >&2
  exit 2
  ;;
esac

cross="${CROSS_COMPILE:-$cross_default}"
cc="${TOOL_CC:-${cross}gcc}"
ld="${TOOL_LD:-${cross}ld}"
objcopy="${TOOL_OBJCOPY:-${cross}objcopy}"
cc_target="${TOOL_CC_TARGET:-}"

src="$srctree/flash"
build="$objtree/.chainload"
out="${CHAINLOAD_OUTPUT:-$objtree/${soc}-chainload.bin}"
mkdir -p "$build" "$(dirname -- "$out")"

flags="$endian -mabi=32 -mips32r2 -msoft-float -mno-abicalls -fno-pic -fno-pie \
 -ffreestanding -fno-builtin -fno-stack-protector -Os -G0 -Wall"

"$cc" $cc_target $flags -I"$src" -c "$src/loader-start.S" -o "$build/loader-start.o"
"$cc" $cc_target $flags -I"$src" -DUBOOT_LOAD_ADDR="$UBOOT_LOAD_ADDR" -DUBOOT_OFFSET="$UBOOT_OFFSET" \
 -c "$src/loader.c" -o "$build/loader.o"
"$cc" $cc_target $flags $standalone_flags -I"$src" \
 -c "$src/early_sfc.c" -o "$build/early_sfc.o"

"$ld" $endian -m "$emulation" -T "$src/loader.lds" \
 -Map "$build/chainload.map" -o "$build/chainload.elf" \
 "$build/loader-start.o" "$build/loader.o" "$build/early_sfc.o"
"$objcopy" -O binary "$build/chainload.elf" "$out.tmp"
mv -f "$out.tmp" "$out"
