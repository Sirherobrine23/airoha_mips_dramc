#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
set -eu

soc="${1:-}"
srctree="${srctree:-$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)}"
objtree="${objtree:-$srctree/out/$soc}"

case "$soc" in
 en751221)
  dir="$srctree/dramc/en751221"
  endian=-EB
  emulation=elf32btsmip
  cc_default=mips-linux-gnu-
  out="${DDR_OUTPUT:-$objtree/en751221-dramc.bin}"
  # Preserve the link order of the known-good EN7512 V1.2.2 spram stage.
  source_order="head.S setup.c main.c init.c time.c string.c \
dramc.c dramc_dq_dqs_cal.c dramc_dle_cal.c dramc_dqs_gw_cal.c \
en7512_dramc_init.c spram.c"
  no_reconstructed=1
  ;;
 en751627)
  dir="$srctree/dramc/en751627"
  endian=-EB
  emulation=elf32btsmip
  cc_default=mips-linux-gnu-
  out="${DDR_OUTPUT:-$objtree/en751627-dramc.bin}"
  prebuilt="$dir/en751627_ddr.bin"
  ;;
 en7528)
  dir="$srctree/dramc/en7528"
  endian=-EL
  emulation=elf32ltsmip
  cc_default=mipsel-linux-gnu-
  out="${DDR_OUTPUT:-$objtree/en7528-dramc.bin}"
  readable="$dir/readable"
  no_reconstructed=1
  ;;
 en7580)
  dir="$srctree/dramc/en7580"
  endian=-EL
  emulation=elf32ltsmip
  cc_default=mipsel-linux-gnu-
  out="${DDR_OUTPUT:-$objtree/en7580-dramc.bin}"
  ;;
 *)
  echo "usage: $0 {en751221|en751627|en7528|en7580}" >&2
  exit 2
  ;;
esac

mkdir -p "$objtree" "$(dirname -- "$out")"

if [ -n "${prebuilt:-}" ]; then
 expected_size=$((0xd050))
 actual_size=$(wc -c < "$prebuilt")
 if [ "$actual_size" -ne "$expected_size" ]; then
  echo "error: EN751627 DDR payload has size $actual_size, expected $expected_size" >&2
  exit 1
 fi
 cp "$prebuilt" "$out.tmp"
 mv -f "$out.tmp" "$out"
 exit 0
fi

cross="${CROSS_COMPILE:-$cc_default}"
cc="${TOOL_CC:-${cross}gcc}"
ld="${TOOL_LD:-${cross}ld}"
objcopy="${TOOL_OBJCOPY:-${cross}objcopy}"
cc_target="${TOOL_CC_TARGET:-}"
build="$objtree/.dramc"
rm -rf "$build"
mkdir -p "$build"

cflags="$endian -mabi=32 -mips32r2 -msoft-float -mno-abicalls -fno-pic -fno-pie -ffreestanding -fno-builtin -Os -G0"

if [ -z "${no_reconstructed:-}" ]; then
 for src in "$dir"/reconstructed/*.S; do
  [ -f "$src" ] || continue
  if grep -nE '\.reloc.*,[[:space:]]*$' "$src" >&2; then
   echo "error: DDR source contains unresolved section relocations" >&2
   exit 1
  fi
 done
fi

objs=''

compile_asm()
{
 src="$1"
 [ -f "$src" ] || return 0
 obj="$build/$(basename "${src%.S}").o"
 "$cc" $cc_target $cflags -D__ASSEMBLY__ -I"$srctree/include" -I"$dir" \
  -x assembler-with-cpp -c "$src" -o "$obj"
 objs="$objs $obj"
}

compile_c()
{
 src="$1"
 [ -f "$src" ] || return 0
 obj="$build/$(basename "${src%.c}").o"
 "$cc" $cc_target $cflags -I"$srctree/include" -I"$dir" \
  -fomit-frame-pointer -fno-stack-protector \
  ${readable:+-I"$readable"} -c "$src" -o "$obj"
 objs="$objs $obj"
}

if [ -n "${source_order:-}" ]; then
 for name in $source_order; do
  case "$name" in
   *.S) compile_asm "$dir/$name" ;;
   *.c) compile_c "$dir/$name" ;;
   *) echo "error: unsupported EN751221 DDR source: $name" >&2; exit 1 ;;
  esac
 done
else
 if [ -z "${no_reconstructed:-}" ]; then
  if [ -n "${asm_order:-}" ]; then
   for name in $asm_order; do
    compile_asm "$dir/reconstructed/$name.S"
   done
  else
   for src in "$dir"/reconstructed/*.S; do
    [ -f "$src" ] || continue
    name=$(basename "${src%.S}")
    case " ${skip_reconstructed:-} " in
     *" $name "*) continue ;;
    esac
    compile_asm "$src"
   done
  fi
 fi
 for src in "$dir"/*.S; do
  [ -f "$src" ] || continue
  compile_asm "$src"
 done
 if [ -n "${readable:-}" ]; then
  for src in "$readable"/*.c; do
   [ -f "$src" ] || continue
   compile_c "$src"
  done
 fi
fi

if [ -f "$dir/glue.c" ]; then
 compile_c "$dir/glue.c"
fi

"$ld" "$endian" -m "$emulation" -T "$dir/ddr.lds" \
 -Map "$build/dramc.map" -o "$build/dramc.elf" $objs
"$objcopy" -O binary "$build/dramc.elf" "$out.tmp"
mv -f "$out.tmp" "$out"
