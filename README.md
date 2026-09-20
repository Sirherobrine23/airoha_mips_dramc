# Airoha MIPS DRAM calibration and early chainload

Standalone early-boot sources for Airoha/EcoNet MIPS SoCs.  The repository
contains the DRAM controller initialization/calibration stages and the small
post-DRAM flash loader used to chainload U-Boot without keeping those sources in
the U-Boot tree. The packaging path accepts raw `u-boot.bin`, legacy
`u-boot.img`, and FIT images.

## Supported SoCs

- `en751221`
- `en751627` (DRAMC payload currently comes from the preserved vendor binary)
- `en7528`
- `en7580` (DRAMC reconstruction is still WIP)

## Build

The normal interface is deliberately per-SoC.

Build every SoC:

```sh
make all
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
```

That recovery chainloader is not the same thing as the normal `chainload.bin`:
`chainload.bin` runs after DRAM setup and reads the second-stage image from
flash, while the recovery image is used by the EN751221 internal BootROM over
XMODEM.

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

## Optional `tcboot.bin` packaging

The default `make <soc>` does **not** depend on a U-Boot source/build tree.
When a complete flash image is needed, point the optional packaging target at
one of the supported U-Boot formats:

```sh
# Raw U-Boot binary. Preferred for a minimal second stage.
make en751221-tcboot UBOOT_IMAGE=/path/to/u-boot.bin

# Legacy uImage. Kept on flash as a legacy image for compatibility.
make en751221-tcboot UBOOT_IMAGE=/path/to/u-boot.img

# FIT image. firmware/loadables are honored; U-Boot RAM FITs using
# config->kernel are also supported.
make en751221-tcboot UBOOT_IMAGE=/path/to/u-boot.itb
```

The same applies to `en751627-tcboot` and `en7528-tcboot`, or to the generic
form `make tcboot SOC=<soc> UBOOT_IMAGE=<file>`. Raw and FIT inputs are wrapped
in the small ECNT container (magic `ECNT`, exact size, load/entry and CRC32);
legacy `u-boot.img` is preserved as-is. For a normal FIT `firmware`/`loadables`
image, its load/entry metadata is honored. U-Boot-generated RAM FITs commonly
reference the U-Boot payload through `config->kernel`; in that compatibility
mode the SoC target load address (for EN751221, `0x81000000`) is authoritative,
because generated FIT metadata may describe an SPL convention rather than the
linked U-Boot ELF address. This keeps the post-DRAM flash loader small while
allowing all three input formats. The linked `u-boot` ELF is intentionally
rejected.

The result is `out/<soc>/tcboot.bin`. EN7580 TCBoot packaging remains WIP.

## Host image tool

The image finalizers are built as a native C utility at
`out/host/econet-image`.  The build no longer requires Python for image
packing or tests.  Its subcommands replace the former `tools/*.py` helpers:

```text
econet-image chainloader  # CRC table + XMODEM padding
econet-image flash        # build the 1 MiB TCBoot-compatible image
econet-image tcboot       # finalize legacy TCBoot/Binman images
econet-image selftest     # host-side regression tests
```

The normal `make <soc>` targets build the host utility automatically.

## Tests

Host-side image-layout tests do not require a MIPS toolchain:

```sh
make test
```

## Layout

- `dramc/<soc>/` — DRAM initialization/calibration source or preserved payload.
- `dramc/Makefile` — standalone DRAMC builder for all supported SoCs.
- `flash/` — flash reader, TCBoot startup stages and Makefile for both `chainload` and `tcboot`.
- `chainloader/` — EN751221 BootROM/XMODEM recovery chainloader and Makefile.
- `include/` — standalone early-boot headers; no U-Boot include tree required.
- `soc/*.mk` — per-SoC toolchain/capability metadata.
- `tools/` — only the native `econet-image.c` host image tool; the build path has no shell/Python helper scripts.
- `tpl/`, `spl/` — older U-Boot-integrated early-boot path retained as reference.
