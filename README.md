# Airoha MIPS DRAM calibration and bootext

Standalone early-boot sources for Airoha/EcoNet MIPS SoCs.  The repository
contains the DRAM controller initialization/calibration stages and the small
post-DRAM flash loader used to chainload an opaque payload without keeping those
sources in the bootloader tree. Final payload selection and packaging are kept
outside the firmware build (for example in the OpenWrt image pipeline).

## Supported SoCs

- `en751221`
- `en751627`
- `en7528`
- `en7580` (DRAMC port; experimental, hardware validation pending)

## Build

Builders add `includes/` to the header search path.
Shared EN751221 calibration sources and headers live in
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

## Code formatting

The root `.clang-format` defines a Linux-style layout with eight-column tabs,
a 100-column limit and function braces on a separate line. Include order,
comment text and comment line breaks are preserved; comment indentation is
adjusted to match the surrounding code. `make format` runs clang-format, then aligns
single-line `#define` values with tabs at a common eight-column stop, including
across blank lines. A declaration, comment or other directive ends the group.
Continued macros and definitions containing only comments retain their layout.
`SkipMacroDefinitionBody: true` lets clang-format preserve this alignment.
The editor must also display tabs as eight columns for the values to line up.
The repository's EditorConfig, VS Code and Zed settings use that width for C/C++
sources.

Install `clang-format` 20 or newer, then run:

```sh
make format
```

The formatting steps live in the `Makefile`; no separate Python script is needed.
`format` updates tracked `.c`, `.h`, `.cc`, `.cpp` and `.hpp` files in place.
Assembly, generated `includes/tmp_version.h` and sources under `blobs/` are
excluded.

To select a versioned executable, use `make format CLANG_FORMAT=clang-format-20`.
Editors supporting clang-format
automatically discover the root configuration and preserve macro alignment;
use `make format` to align newly added macros.

## Documentation

Project documentation uses Markdown (`.md`). New documentation must be written in Markdown.
