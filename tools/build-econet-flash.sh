#!/bin/sh
# SPDX-License-Identifier: GPL-2.0+
# Compose a complete TCBoot-compatible image around a prebuilt U-Boot uImage.
set -eu

soc="${1:-}"
standalone_flags=
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
  ;;
en7528)
  endian=-EL
  emulation=elf32ltsmip
  cross_default=mipsel-linux-gnu-
  ;;
*)
  echo "unsupported tcboot SoC: $soc" >&2
  exit 2
  ;;
esac

srctree="${srctree:-$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)}"
objtree="${objtree:-$srctree/out/$soc}"
UBOOT_LOAD_ADDR="${UBOOT_LOAD_ADDR:-0x81000000}"
UBOOT_IMAGE="${UBOOT_IMAGE:-$objtree/u-boot.img}"
DDR_IMAGE="${DDR_IMAGE:-$objtree/${soc}-dramc.bin}"
TCBOOT_OUTPUT="${TCBOOT_OUTPUT:-$objtree/tcboot.bin}"

[ -f "$UBOOT_IMAGE" ] || { echo "error: U-Boot image not found: $UBOOT_IMAGE" >&2; exit 2; }
[ -f "$DDR_IMAGE" ] || { echo "error: DRAMC image not found: $DDR_IMAGE" >&2; exit 2; }

cross="${CROSS_COMPILE:-$cross_default}"
cc="${TOOL_CC:-${cross}gcc}"
ld="${TOOL_LD:-${cross}ld}"
objcopy="${TOOL_OBJCOPY:-${cross}objcopy}"
nm="${TOOL_NM:-${cross}nm}"
cc_target="${TOOL_CC_TARGET:-}"

src="$srctree/flash"
build="$objtree/.tcboot"
mkdir -p "$build" "$(dirname -- "$TCBOOT_OUTPUT")"
flags="$endian -mabi=32 -mips32r2 -msoft-float -mno-abicalls -fno-pic -fno-pie -ffreestanding -fno-builtin -fno-stack-protector -Os -G0"

"$cc" $cc_target $flags -I"$src" -c "$src/loader-start.S" -o "$build/loader-start.o"
"$cc" $cc_target $flags -I"$src" -DUBOOT_LOAD_ADDR="$UBOOT_LOAD_ADDR" -c "$src/loader.c" -o "$build/loader.o"
"$cc" $cc_target $flags $standalone_flags -I"$src" -c "$src/early_sfc.c" -o "$build/flash.o"
"$ld" $endian -m "$emulation" -T "$src/loader.lds" -Map "$build/loader.map" \
  -o "$build/loader.elf" "$build/loader-start.o" "$build/loader.o" "$build/flash.o"
"$objcopy" -O binary "$build/loader.elf" "$build/loader.bin"

"$cc" $cc_target $flags -D__ASSEMBLY__ -I"$srctree/include" -c "$src/$soc/start.S" -o "$build/start.o"
"$cc" $cc_target $flags -I"$srctree/include" -c "$src/$soc/stages.S" -o "$build/stages.o"

if [ "$soc" = en751221 ]; then
  for stage in move_data boot2; do
    "$cc" $cc_target $flags -D__ASSEMBLY__ -I"$srctree/include" \
      -c "$src/$soc/$stage.S" -o "$build/$stage.o"

    "$ld" $endian -m "$emulation" -G 0 -static -n -nostdlib \
      --build-id=none -T "$src/$soc/$stage.lds" \
      -Map "$build/$stage.map" -o "$build/$stage.elf" "$build/$stage.o"

    "$objcopy" -O binary -j .text "$build/$stage.elf" "$build/$stage.bin"
  done

  "$objcopy" \
    --add-section .move_data="$build/move_data.bin" \
    --set-section-flags .move_data=alloc,load,readonly,code \
    --add-section .boot2="$build/boot2.bin" \
    --set-section-flags .boot2=alloc,load,readonly,code \
    --add-section .loader="$build/loader.bin" \
    --set-section-flags .loader=alloc,load,readonly,data \
    --add-section .spram="$DDR_IMAGE" \
    --set-section-flags .spram=alloc,load,readonly,data \
    "$build/stages.o" "$build/payloads.o"
else
  "$objcopy" \
    --add-section .loader="$build/loader.bin" \
    --set-section-flags .loader=alloc,load,readonly,data \
    --add-section .spram="$DDR_IMAGE" \
    --set-section-flags .spram=alloc,load,readonly,data \
    "$build/stages.o" "$build/payloads.o"
fi

"$ld" $endian -m "$emulation" -T "$src/flash.lds" -Map "$build/flash.map" \
  -o "$build/flash.elf" "$build/start.o" "$build/payloads.o"
"$objcopy" -O binary "$build/flash.elf" "$build/flash.bin"
"$nm" -n "$build/flash.elf" >"$build/flash.nm"
"${PYTHON3:-python3}" "$srctree/tools/econet_flash_image.py" \
  --soc "$soc" --stages "$build/flash.bin" --symbols "$build/flash.nm" \
  --uboot "$UBOOT_IMAGE" --load "$UBOOT_LOAD_ADDR" --output "$TCBOOT_OUTPUT"
