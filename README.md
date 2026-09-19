# Airoha MIPS DRAM calibration and early chainload

Standalone early-boot sources for Airoha/EcoNet MIPS SoCs.  The repository
contains the DRAM controller initialization/calibration stages and the small
post-DRAM flash loader used to chainload U-Boot (or another compatible legacy
uImage) without keeping those sources in the U-Boot tree.

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
an already-built uncompressed MIPS `u-boot.img`:

```sh
make en751221-tcboot UBOOT_IMAGE=/path/to/u-boot.img
make en751627-tcboot UBOOT_IMAGE=/path/to/u-boot.img
make en7528-tcboot UBOOT_IMAGE=/path/to/u-boot.img
```

or equivalently:

```sh
make tcboot SOC=en7528 UBOOT_IMAGE=/path/to/u-boot.img
```

The result is `out/<soc>/tcboot.bin`.  EN7580 TCBoot packaging remains WIP.

## Tests

Host-side image-layout tests do not require a MIPS toolchain:

```sh
make test
```

## Layout

- `dramc/<soc>/` — DRAM initialization/calibration source or preserved payload.
- `flash/` — standalone flash reader and TCBoot startup stages.
- `chainloader/` — EN751221 BootROM/XMODEM recovery chainloader.
- `include/` — standalone early-boot headers; no U-Boot include tree required.
- `soc/*.mk` — per-SoC toolchain/capability metadata.
- `tools/` — standalone builders and host-side image tools.
- `tpl/`, `spl/` — older U-Boot-integrated early-boot path retained as reference.
