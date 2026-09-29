# Airoha MIPS DRAM calibration and early chainload

Standalone early-boot sources for Airoha/EcoNet MIPS SoCs.  The repository
contains the DRAM controller initialization/calibration stages and the small
post-DRAM flash loader used to chainload an opaque payload without keeping those
sources in the bootloader tree. Final payload selection and packaging are kept
outside the firmware build (for example in the OpenWrt image pipeline).

## Supported SoCs

- `en751221`
- `en751627` (DRAMC payload currently comes from the preserved vendor binary)
- `en7528`
- `en7580` (DRAMC reconstruction is still WIP)

## Build

The normal interface is deliberately per-SoC. The top-level `Makefile` is only
the build dispatcher: it defines global inputs, includes the builders and then
registers the device contexts from `images/*.mk`. SoC-specific state belongs in
`images/<soc>.mk`; the component Makefiles only expose builder macros such as
`dramc/compile`, `flash/compile`, `chainload/compile` and `bootext/compile`.

Before each `Device/<soc>` is evaluated, `images/Makefile` resets every
context-owned variable with `Device/Init`. The resulting context (including the
toolchain selected by `compiler/prepare`) is then snapshotted as target-specific
variables before the next device is evaluated. This allows multiple SoCs and
secondary contexts such as bootext DRAMC/chainloader to coexist in the same
parallel make invocation without inheriting state from the previous device.

Build all default SoCs:

```sh
make all
```

Include non-default experimental SoCs in `all`:

```sh
make EXPERIMENTAL_SOCS=1 all
```

Build only one SoC:

```sh
make en751221
make en751627
make en7528
make en7580
```

Each SoC target builds the two independent pieces under `out/<soc>/`:

```text
out/<soc>/<soc>-dramc.bin
out/<soc>/<soc>-chainload.bin
```

`en751221` additionally builds the BootROM/XMODEM recovery helper:

```text
out/en751221/en751221-recovery-chainloader.bin
out/en751221/bootext.bin
```

`en7528` additionally builds a standalone BootROM/XMODEM boot extension:

```text
out/en7528/bootext.bin
```

The normal `chainload.bin` is the post-DRAM flash reader.  The EN751221
recovery helper instead runs from DRAM after the vendor/BootROM DDR stage.  The
EN7528 `bootext.bin` is different again: it is a single FE-SRAM image containing
the EN7528 DRAMC stage followed by an SRAM-resident XMODEM chainloader.

### EN751221 recovery with DDR reinitialization (experimental)

`make en751221-bootext` builds `out/en751221/bootext.bin`. Unlike EN7528,
the EN751221 BootROM downloads this file to **DRAM at 0x80009000**, not
directly to FE SRAM. A small bootstrap verifies the CRC table covering both
embedded payloads, enables PBUS access to FE memory, and copies them with
32-bit stores and readback verification:

```text
0x9fa32800  in-tree EN751221 DRAMC (entry at 0x9fa32a80)
            runtime state, including the DRAMC stack, ends below 0x9fa38000
0x9fa38000  SRAM XMODEM receiver and its DDR handoff entry
            code, CRC table and BSS must end below 0x9fa3c000
```

The SRAM entry uses no DRAM stack while the DRAMC runs. It supplies the
return address through `SCREG_WR0`, applies the post-calibration SLM,
arbiter and SMC setup from `flash/en751221/boot2.S`, then enters the normal
receiver with a fresh DRAM stack. The existing receiver's image formats,
menu and default chainload action are unchanged. **The menu also offers
flash writing: do not select it for a RAM-only recovery test.**

For this EN751221 bootext only, LLVM builds use `-Oz`; GCC keeps `-Os`.
The 1 KiB XMODEM packet buffer lives on the cached DRAM stack after calibration,
rather than occupying the receiver's limited SRAM BSS. Other recovery targets
retain their static buffer. The SRAM addresses, CRC coverage and overlap
checks are unchanged. Headroom remains limited, so these checks must stay
enabled for new compiler versions.

Run toolchain builds sequentially in separate, initially empty output
directories, then verify each set of artifacts, for example:

```sh
make -j1 O="$PWD/out/gcc" CROSS_COMPILE=/path/to/mips-linux-gnu- en751221
make O="$PWD/out/gcc" CROSS_COMPILE=/path/to/mips-linux-gnu- en751221-bootext-check
make -j1 O="$PWD/out/clang" LLVM=1 en751221
make O="$PWD/out/clang" LLVM=1 en751221-bootext-check
```

This is needed because the XR500v's BootROM-only DDR setup differed from
the flash path: missing clock metadata, different PLL/DRAMC setup, and
missing QDMA descriptor DONE writeback. An earlier DDR-stage chainloader
restored working Ethernet and Linux startup on that board.

The initial standalone implementation was tested in one cold BootROM cycle on
an Archer XR500v (EN7526G, 256 MiB DDR3). The wrapper CRC, SRAM readback and
receiver CRC passed; DRAMC reported calibration status 0; U-Boot loaded over
XMODEM, and a 9.3 MiB Linux initramfs transferred by TFTP with matching CRC.
Linux booted with the watchdog active, CPU 900 MHz and bus 225 MHz. An 8 MiB
synthetic download and three 8 MiB uploads matched SHA-256; the system ran
for over ten minutes without the previous QDMA completion warning or reset.
The board was then power-cycled back to its existing flash image.

That hardware run predates the packet-buffer/LLVM-footprint correction above.
The corrected revision has passed clean GCC 14.4 and Clang 19.1.7/21.1.8
builds and artifact checks. Its Clang 21.1.8 artifact has since passed one
cold BootROM recovery on the same XR500v: DDR/readback/receiver CRC checks,
raw U-Boot reception, TFTP, Linux userspace with its watchdog active, and
an 8 MiB download plus three 8 MiB uploads with matching SHA-256. The board
was returned to its existing flash image afterward. The tested artifact was
built at `35d600c` (before the CI-only rebase), SHA-256
`6c1fa7b578014e606de3654f18e0d67a8421ca87f37cab4adec7cbecf38afe57`.
This does not validate repeated recovery, prolonged stress, other boards,
or the newly built GCC/Clang 19 artifacts.

This is a single-board smoke test, not repeated-cold-boot or long-duration
validation. The tested second stage was raw `u-boot.bin`; the receiver's
other input formats and flash-writing menu were not hardware-tested in
this run. Keep a known-good recovery helper when trying it on another board.
The original DRAM-only `en751221-recovery-chainloader.bin` remains available
unchanged for comparison.

The C host selftests cover CRC tables, embedded payload corruption,
truncation and SRAM boundary rejection. After building, check the actual
images and linker symbols with the same host utility and the cross `nm`:

```sh
make test
make en751221-bootext-check CROSS_COMPILE=/path/to/mips-linux-gnu-
```

`en751221-bootext-check` reads existing artifacts; it does not rebuild or
modify firmware. Neither check runs firmware or replaces a BootROM hardware
test. No Python dependency is needed.

### GNU cross toolchain

The defaults are:

- big-endian: `mips-linux-gnu-`
- little-endian: `mipsel-linux-gnu-`

Override a single-SoC build in the usual way:

```sh
make en7528 CROSS_COMPILE=/path/to/mipsel-linux-gnu-
```

### LLVM/Clang

For the SoCs whose reconstructed assembly is accepted by LLVM's MIPS
assembler:

```sh
make LLVM=1 en751221
make LLVM=1 en751627
make LLVM=1 en7528
```

The current EN7580 reconstruction uses `R_MIPS16_26` `.reloc` directives which
LLVM's integrated MIPS assembler does not implement; build that DRAMC payload
with GNU binutils for now.  Its standalone chainload stage itself is LLVM-safe.

## Standalone `tcboot.bin`

The TCBoot build is intentionally independent from U-Boot (or any other final
payload). Build the boot component with:

```sh
make en751221-tcboot
make en751627-tcboot
make en7528-tcboot
# or
make tcboot SOC=en7528
```

The result is `out/<soc>/tcboot.bin`, a 128 KiB base image ending at flash
offset `0x20000`. It contains the BootROM stages, DRAM calibration payload and
the generic post-DRAM flash loader, but no U-Boot/FIT/kernel payload.

The loader understands only the small ECNT payload descriptor. It does not
parse uImage, FIT or ELF. The image builder chooses an arbitrary payload, load
address and entry address, and wraps those opaque bytes before appending them:

```sh
out/host/econet-image tcboot \
    --boot out/en7528/tcboot.bin \
    --payload /path/to/payload.bin \
    --load 0x81000000 \
    --entry 0x81000000 \
    --output /tmp/tcboot-with-payload.bin
```

`econet-image tcboot` updates the TCBoot payload bounds in the BootROM header,
recalculates its CRC, and appends an ECNT descriptor plus the payload. It emits
only the used bytes; an outer image pipeline such as OpenWrt can pad the
artifact to the flash partition size afterwards. This keeps U-Boot/FIP/FIT
selection entirely in the OpenWrt image recipe rather than in this build.

The descriptor and payload must fit in the 1 MiB replacement bootloader region.
The current loader accepts payload destinations in `0x81000000..0x81ffffff`;
`entry` may differ from `load` but must point inside the loaded payload.

## EN7528 `bootext.bin`

The EN7528 BootROM recovery path needs the DRAM stage and the XMODEM
chainloader in one FE-SRAM image.  Build it explicitly with:

```sh
make en7528-bootext
```

It is also built by the normal `make en7528` target.  The current layout is:

```text
0x9fa30000  EN7528 DRAMC / calibration entry
            ... runtime DRAMC image/BSS ends before 0x9fa35000
0x9fa35000  SRAM XMODEM chainloader
            ... CRC self-check table
```

The composite file places the chainloader at file offset `0x5000`, pads the
result to an XMODEM 128-byte boundary, and currently fits inside the reserved
48 KiB FE-SRAM image window.  The bootext-specific DRAMC entry writes
`0x9fa35000` to `SYS_BOOT_JUMP` (`0xbfb00280`); after calibration
`start_spram(1)` follows that vendor handoff and enters the SRAM chainloader.
The chainloader then receives U-Boot into DRAM and accepts raw, legacy, FIT or
ECNT images.

## Host image tool

The image finalizers are built as a native C utility at
`out/host/econet-image`.  The build no longer requires Python for image
packing or tests.  Its subcommands replace the former `tools/*.py` helpers:

```text
econet-image chainloader      # CRC table + XMODEM padding
econet-image tcboot-base      # build payload-free 128 KiB TCBoot component
econet-image tcboot           # attach an opaque payload + ECNT descriptor
econet-image flash            # legacy one-shot image packer
econet-image tcboot-finalize  # finalize legacy TCBoot/Binman images
econet-image bootext          # compose EN7528 DRAMC + SRAM chainloader
econet-image selftest         # host-side regression tests
```

The normal `make <soc>` targets build the host utility automatically.

## Tests

Host-side image-layout tests do not require a MIPS toolchain:

```sh
make test
```

## Build system layout

The root `Makefile` is the public entry point.  A `make <soc>` invocation starts
one isolated internal build instance for that SoC, then includes the component
fragments below.  This keeps per-SoC variables from leaking into other targets
while following the same metadata + shared build-helper model used by OpenWrt.

- `base/compiler.mk` — shared GNU/LLVM toolchain selection and endian setup.
- `soc/Makefile`, `soc/*.mk` — SoC registry, capabilities and toolchain metadata.
- `dramc/<soc>/` — DRAM initialization/calibration source or preserved payload; each per-SoC `Makefile` only describes its DRAMC inputs/quirks.
- `dramc/Makefile` — DRAMC build fragment and common compile/link helpers.
- `flash/Makefile` — post-DRAM `chainload` and optional `tcboot` build fragment.
- `chainloader/Makefile` — SRAM/DRAM XMODEM recovery-chainloader build fragment.
- `bootext/Makefile`, `bootext/*.mk` — bootext build fragment plus per-SoC SRAM/layout metadata.
- `include/` — standalone early-boot headers; no U-Boot include tree required.
- `tools/` — the native `econet-image.c` host image tool; the build path has no shell/Python helper scripts.
