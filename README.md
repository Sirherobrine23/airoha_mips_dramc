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
- `en7580` (DRAMC port; experimental, hardware validation pending)

## Build

Builders add `includes/` to the header search path.
Shared EN7512 calibration sources and headers live in
`includes/dramc/`; SoC-specific sources stay under `dramc/<soc>/`.

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

## Documentation

Project documentation uses Markdown (`.md`). New documentation must be written in Markdown.
