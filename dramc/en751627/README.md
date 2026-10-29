# EN751627 C DRAMC port

The EN751627 payload is built from C sources, with a small assembly entry
to establish the SRAM stack and clear BSS. CP0/TLB operations use inline
assembly. No vendor binary, object file, encoded instruction array or
Ghidra export is linked into the firmware.

The reconstruction used the supplied EN7516 DRAMC V1.0 executable,
its relocatable objects and Ghidra exports. These reference artifacts
are not build inputs. The original raw initialized payload is
53,328 bytes with SHA-256
`a8ca71b613f1857741dde066ebbc5de16baa90ec5203e27d07a5d1705fcf123e`.

## Implementation

| File                  | Responsibility                                                           |
| --------------------- | ------------------------------------------------------------------------ |
| `entry.S`, `ddr.lds`  | Entry at `0x9fa30000`, SRAM stack and BSS initialization                 |
| `en7512_dramc_init.c` | Package/revision-specific DDR3 PLL, mode commands and timing setup       |
| `dramc.c`             | Calibration sequence, geometry probing, refresh timing and size metadata |
| `dramc_dq_dqs_cal.c`  | RX/TX per-bit training using sequential TA2 write/read tests             |
| `dramc_test.c`        | GDMA and TA1 tests                                                       |
| `system.c`            | CPU clock setup and strap metadata                                       |
| `runtime.c`           | Native 32-bit UART accesses and down-counter delays                      |
| `mmu.c`               | TLB invalidation and eight pairs of 64 MiB DRAM mappings                 |
| `main.c`              | eFuse setup, DRAM-stage lifecycle and return to boot2                    |

DLE and DQS gate-window calibration use the common sources under
`includes/dramc/`, selected by the local `en7512_soc.h`. EN751627 retains
its big-endian CPU, KSEG2 test address `0xc0080000`, original polling
limits and separate clock/timing initialization. The normal entry selects
DDR3 and derives QFP/BGA1 packaging from the hardware strap.

The original executable also contains an XMODEM flash-recovery menu. This
port implements the DRAM stage used by the repository's TCBoot flow:
after calibration it invalidates the temporary TLB entries and jumps to
the continuation address in `SYS_BOOT_JUMP` (`0xbfb00280`). Flash loading
and recovery remain in the repository's separate loader components.

The linker reserves the region above `0x9fa3b000` for the stack and rejects
runtime images that overlap it. The entry is at offset zero; the image
tool preserves it and only inserts the legacy `+0x280` jump when given an
old payload with a reserved zero prefix.

## Build and editor support

```sh
make LLVM=1 en751627 en751627-tcboot
make CROSS_COMPILE=mips64-linux-gnu- en751627 en751627-tcboot
make clangd CLANGD_SOC=en751627
```

Build outputs include `out/en751627/en751627-dramc.bin` and
`out/en751627/tcboot.bin`. The latter is a base image to which a final
payload is attached by the image builder. The SoC remains experimental
pending board validation.

The standalone SRAM boot extension and XMODEM receiver are documented in
[`bootext/en751627.md`](../../bootext/en751627.md). Build them with
`make en751627-bootext` using the same compiler selection.

## Hardware validation

A cold board boot and memory stress test are still required before
claiming hardware validation.
