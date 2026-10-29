# EN7580 DDR calibration in C

`make en7580` builds the C implementation in this directory. The PLL,
DDR3/DDR4 initialization, impedance, write leveling, gating, RX/TX windows,
latency, Vref, timing tables, register access, clocks and eFuse routines are
source code, not byte arrays containing executable instructions. Only the
entry point remains in `entry.S`; `spram.c` uses inline assembly for
CP0/cache operations that C cannot express.

The payload executes at `0x9fa30000`. `ddr.lds` still reserves the final
4 KiB for the stack and rejects code/data/BSS extending past `0x9fa3b000`.
The cold-boot target remains experimental. A warm DRAMC calibration through
the stock tcboot was validated on a physical C6500XK, as described below.

The EN7580 context has 88 bytes, frequency at offset 48, vendor ID at 52 and
pinmux at 64. Static assertions protect this ABI. DDR3/DDR4 identifiers are
4/5, and the DDR4 x16 pinmux is 3. These values intentionally differ from
the XX230v SDK header. The EN7580 PLL values and four AC timing rows are
preserved from its objects; the EN7523 PLL is not substituted.

Storage is represented by named globals, typed timing parameters and local
structures/arrays. Some internal calibration temporaries retain recovered
names where their original source name could not be established. Functions
that were empty in the vendor object remain empty; they are not placeholders
for missing implementations. The production preset uses one rank and x16.

## EN7580 header integration

The active port includes `dramc_pi_api.h`, `dramc_register.h` and
`x_hal_io.h` from `includes/dramc/en7580/`. Unlike the separate XX230v
EN7523 reference, these headers select EN7580 and match the recovered
88-byte context. The context and shared function declarations now come
from those headers rather than a duplicated reconstructed definition.
Static assertions check the single-rank x16 preset, type/pinmux IDs and
critical context offsets, including the result and execution arrays.
The HAL physical-bank table uses the register header's KSEG1 bases.

The following integration corrections are explicit:

- Shared APIs use the vendor argument/return widths, enums and byte-sized
  status values where specified, including `ucDram_Register_Write`.
- Calibration results and write-leveling/TX state use the named arrays
  in `DRAMC_CTX_T`. Rank setters and result accessors reject/ignore an
  unsupported rank; invalid result reads produce zero flags. Rank 1
  previously addressed unrelated fields through scalar-pointer arithmetic.
- Invalid calibration IDs are ignored before shifting a bit mask.
- `P_Fld` uses the field mask directly, avoiding undefined `1 << 32` in
  full-word fields and signed shifts into bit 31.
- Clang uses `-Oz` for this SoC because `-Os` exceeds the SRAM allocation.
  The linker continues to enforce the original stack reservation.

The original PLL values, timing rows, DDR3/DDR4 choices and calibration
selection rules are retained. The supplied header is a bring-up preset;
including it does not add dual-rank, MPR/RDDQC or alternative calibration
flows that are absent from the recovered implementation.

## Validation

```sh
make CROSS_COMPILE=mips64-linux-gnu- en7580 test
make LLVM=1 en7580
make CROSS_COMPILE=mips64-linux-gnu- test-en7580-c
make LLVM=1 test-en7580-c
```

The optional regression test requires Python 3, QEMU's `qemu-mipsel`
interpreter and the selected compiler/linker. It reads the pre-integration
C sources from Git commit `535fa3d57767621d62fdbea13eb202cc213a2992` and
compares them with the current port under identical simulated register
responses. It runs 400 cases, comparing return values, read/write/delay
counts, two ordered-operation hashes and all 88 context bytes. Cases
cover DDR3/DDR4, frequency/ODT choices, engine pass/fail and Vref paths;
the complete pipeline is exercised with two stack patterns. Additional
checks exercise invalid-rank protection and full-word/high-bit field
updates.

This test compares the C implementations before and after header
integration, not the original vendor machine code. It does not exercise
physical DDR signals, DMA/cache ordering, actual clocks/eFuses or timeout
expiry. A board boot and memory stress test remain necessary before
claiming hardware validation.

## Intentional corrections

* `DramcRxWindowPerbitCal(p, 0)` returns failure. The object and XX230v
  source can use an uninitialized comparison mask in that unsupported
  configuration (DDR3_MPR_USED and DDR4_RDDQC_USED disabled). The production
  all-channel caller uses mode 1, which computes a test-engine result.
* DDR3 gating state and TX window state are initialized. The object can
  consume leftover stack bytes when no gating window is found, and in
  unused lane fields written by the TX finalization path. C writes defined
  zero defaults instead. Calibration failure flags are still recorded.
* eFuse extraction rejects invalid ranges and a null output buffer rather
  than permitting out-of-range access. Valid boot-time accesses keep the
  original bank-0 extraction and bank-1 repair behavior.
* The unused, uninitialized third argument to `DramcRxdatlatScan` is
  removed. The object never uses that argument.

## Origin

The recovery follows the requested U-Boot interval from
`1a12c700b324709e7b996210991d03769eab85e8` through
`cc706c324bfbf9121f0a76d9a0cc10e548edf06c` (inclusive):

- `f4c2de28301b72ef543958e524a2d33bb9b7ac7f` introduced the encoded EN7580 assembly reconstructed from XGZ030 GPL objects.
- `3e095fd9a10128692935cba27d23b9da08fa20c6` added readable EN7528 C and EN751221 C reference files.
- `2070dac83f586c42accceeb3b6fcdee00ee87428` switched EN751221 to C.
- `cc706c324bfbf9121f0a76d9a0cc10e548edf06c` externalized the early boot sources, keeping the EN7580 encoded assembly.

U-Boot: https://sirherobrine23.com.br/airoha/u-boot

The standalone baseline is `9a0a8c9e747fd748ff20c31bb5627a406c9ad2b7`.
Its `dramc/en7580/reconstructed` sources remain recoverable through Git;
neither those sources nor the intermediate assembly/C drafts are duplicated
alongside the active implementation.

XGZ030 reference: https://sirherobrine23.com.br/devices_gpl/xgz030_v1
at `b3019cd03b879477feebc566193d8d32a0c55410`. The source directory is
`sdk/en7580/bootloader/Uboot/u-boot-2014.04-rc1/arch/arm/cpu/armv7/`
`en7523/ddr_cal/en7523`. It supplies shared algorithm/structure references,
but its EN7523 configuration is not a drop-in EN7580 implementation.

The supplied XX230v archive provides another vendor C reference. Its shared
EN7580 branches and important ABI differences are recorded in
`reference/xx230v-source-audit.md`.

## Secure Boot

The experimental `en7580-bootext` target produces a raw DDR diagnostic image
at `out/en7580/bootext.bin`. It enters at `0x9fa30000`, supplies its own SRAM
stack, clears BSS and runs calibration using the vendor's bootext clock path.
It prints `C-DRAMC start`, the normal calibration output, then either
`C-DRAMC done` or `C-DRAMC size fail` and stays in SRAM. It does not access the
flash writer. The builder checks the entry, SRAM runtime end and binary size.

This raw image has no ECNT secure header or device signature. Its acceptance
by a secure-mode BootROM is a hardware question; a successful build or XMODEM
transfer cannot establish that the ROM will execute it. The vendor
`bootext.ram` from the XGZ030 dump has a V1 ECNT header and a different entry
layout, so it is not interchangeable with this probe. The recovery must be
observed on UART before interpreting any result.

On the EN7580 board tested through `en7523ctl` port1 at 115200 baud, holding
GPIO0 while powering on produced `done`. Sending the raw 41,024-byte probe
after `x` with XMODEM-1K completed at the transport layer, then the recovery
reported `received len=0000a400` and
`Check bootext.ram secure header CRC error:f65d86ef`. Neither `C-DRAMC`
marker appeared. This proves that the raw image was rejected before the C
DRAMC ran; it does not establish whether a correctly formatted but unsigned
image would pass the subsequent signature check. Testing cold DDR calibration
through this route requires a bootext image accepted by the board's secure
boot policy.

## RAM boot through the stock tcboot

On the Quantum/Axon C6500XK, normal boot (without GPIO0 held) reaches the
stock `bldr>` prompt. Its `xmdm <addr> <len>` command receives bytes into RAM
and `jump <addr>` executes them without another image-signature check. This
provides a volatile boot route after the signed BootROM and DDR stages have
already run. It does not test this repository's cold DDR calibration.

The cached `0x81000000` jump is unsafe immediately after XMODEM reception:
the first live probe reached an exception at that address despite a correct
RAM dump. Receiving a 96-byte UART probe at the uncached `0xa1000000` alias
and jumping there printed `EN7580 RAM execution OK`. For a linked U-Boot image,
`tools/en7580-ram-jump.S` runs at uncached `0xa0f00000`, invalidates the
instruction cache over `0x81000000..0x8107ffff`, then enters U-Boot at
`0x81000000`.

Build the handoff stage with:

```sh
make CROSS_COMPILE=mips64-linux-gnu- en7580-ram-jump
```

It produces `out/en7580/ram-jump.bin`. The live test used a
475,876-byte little-endian EN7580 U-Boot built from the
`en7580_xgz030_defconfig` profile at `0x81000000`; this minimal profile uses
an in-memory environment. It ran on the C6500XK and reached the console and
Ethernet, while the kernel FIT supplied the C6500XK device tree. Keep the
U-Boot binary below 512 KiB for the receiver length and cache range below.
From a clean checkout of the local U-Boot tree, reproduce that image with:

```sh
make O=/tmp/en7580-uboot-ram CROSS_COMPILE=mips64-linux-gnu- en7580_xgz030_defconfig
make O=/tmp/en7580-uboot-ram CROSS_COMPILE=mips64-linux-gnu- -j8 u-boot.bin
cp /tmp/en7580-uboot-ram/u-boot.bin out/en7580/u-boot-ram.bin
```

With `en7523ctl` controlling `port1` at 115200 baud and `bldr>` active:

```sh
en7523ctl --config /mnt/Data/airoha_en7523/en7523ctl/config.json \
  xmodem-send --1k --pre-send 'xmdm a1000000 80000' port1 \
  out/en7580/u-boot-ram.bin
en7523ctl --config /mnt/Data/airoha_en7523/en7523ctl/config.json \
  xmodem-send --pre-send 'xmdm a0f00000 1000' port1 \
  out/en7580/ram-jump.bin
```

At the stock console, run `jump a0f00000`. On the tested board this produced
`U-Boot 2026.07`, `SoC: EcoNet EN7580`, `DRAM: 448 MiB`, and the U-Boot prompt.
The U-Boot binary in `out/` is a local build artifact, so rebuild or copy it
there when cleaning the repository output.

The C6500XK OpenWrt initramfs FIT in the local OpenWrt build was loaded over
Ethernet from the host's TFTP server. On the tested setup the host used
`192.168.1.10/24`, served the `openwrt/bin/targets/airoha` directory, and a
LAN cable connected the board to the host. Run these commands at U-Boot,
without `saveenv` or any flash command:

```text
setenv ipaddr 192.168.1.1
setenv serverip 192.168.1.10
ping 192.168.1.10
tftpboot 0x88000000 en7580/openwrt-airoha-en7580-quantum_c6500xk-v1-initramfs-kernel.bin
iminfo 0x88000000
bootm 0x88000000
```

The 65,606,836-byte FIT passed `iminfo` CRC32/SHA1 checks. Linux 6.18.55
identified `Quantum/Axon C6500XK v1`, reached the OpenWrt shell, and mounted
`/` as `tmpfs`. This establishes a working unsigned RAM boot using the stock
tcboot's unrestricted RAM commands. It does not change the board's BootROM
verification, eFuses, or flash contents.

## Warm DDR calibration from tcboot

The stock tcboot disables FE SRAM access by clearing bit 0 of `0xbfb00958`.
Receiving the DRAMC image directly into FE SRAM with `xmdm` produced corrupted
words or stopped after the first XMODEM block. The tested route receives the
image at uncached DRAM address `0xa1100000`, then runs
`tools/en7580-sram-copy.S` from `0xa1000000`. That stage enables FE SRAM,
copies and reads back every 32-bit word at `0xbfa30000`, invalidates the
instruction cache for the cached alias, and returns to `bldr>`. It writes
`0x600d600d` to `0xa1001000` on success or `0xbadbad00` on mismatch.

Build the image and its size-matched copy stage together:

```sh
make CROSS_COMPILE=mips64-linux-gnu- en7580-sram-copy
en7523ctl --config /mnt/Data/airoha_en7523/en7523ctl/config.json \
  xmodem-send --1k --pre-send 'xmdm a1100000 b000' port1 \
  out/en7580/bootext.bin
en7523ctl --config /mnt/Data/airoha_en7523/en7523ctl/config.json \
  xmodem-send --pre-send 'xmdm a1000000 1000' port1 \
  out/en7580/sram-copy.bin
```

Then run the following at `bldr>`; only jump to the DRAMC image when the
status word matches:

```text
jump a1000000
memrl a1001000
  A1001000: 600D600D
jump bfa30000
```

On the C6500XK, the 41,104-byte diagnostic entered FE SRAM and printed
`C-DRAMC start`, DDR3 at 1866, self-test `result=0 u4result=0x00000000`,
`DRAM size=512MB`, and `C-DRAMC done`. The diagnostic then stays in SRAM;
power-cycle to return to the stock boot chain. Its root filesystem and tcboot
state are not preserved across the DDR reinitialization. No flash command is
used.

The first SRAM run reached `Calculate size.` and then restarted before the
final marker. The original size probe used a KSEG2 address requiring the
vendor's TLB map. The diagnostic build now uses the direct uncached KSEG1
alias for the DDR3 address range and bounds a failed probe. This change is
limited to `EN7580_BOOTEXT`; the normal DRAMC build retains its original
address. The successful warm run verifies this board's DDR3 calibration and
size probe, but it does not verify a cold BootROM handoff or a long memory
stress test.

### v1

WIP

### v2

WIP
