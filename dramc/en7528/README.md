# EN7528 DDR calibration source

The EN7528 DDR stage executes from FE SRAM at `0x9fa30000` and targets
MIPS32r2 little-endian.  DRAMC V1.8 is implemented as readable freestanding C
in this directory and `../../includes/dramc`; the EN7528 build no longer consumes
`ddr/reconstructed`.

The full C path contains:

- `system.c`: system clock, eFuse shadow loading and package detection
- `mempll.c`: memory PLL setup and calibration
- `en7512_dramc_init.c`: initial EN7528 DRAMC programming
- `dramc.c`: calibration orchestration, size detection and TRFC
- `../../includes/dramc/dramc_dqs_gw_cal.c`: shared DQS gate-window calibration
- `dramc_dq_dqs_cal.c`: RX/TX per-bit DQ/DQS calibration
- `../../includes/dramc/dramc_dle_cal.c`: shared DLE calibration
- `dramc_test.c`: EN7528 DMA and TA1 routines
- `en7512_soc.h`: EN7528 parameters for the shared calibration code
- `main.c`: SRAM stage handoff

## Hardware validation

The complete no-reconstructed path completed DDR training from a standalone
SRAM bootext on a TP-Link XC220-G3 v1 / EN7528HU with 256 MiB DDR3.  The
validated path reported:

```
QFP IC
Xtal:1
DDR3 init.
DRAMC init done.
DDR leave: do_dqs_gw_calib_1 ret=0x00000000
DDR leave: do_sw_rx_dq_dqs_calib ret=0x00000000
DDR leave: do_dle_calib ret=0x00000000
DDR leave: do_sw_tx_dq_dqs_calib ret=0x00000000
DRAM size=256MB
ddr-900
7528DRAMC V1.8 (0)
```

The current `en7512_dramc_init.c` intentionally implements the validated
EN7528 QFP + DDR3 path.  KGD/BGA and DDR2 paths must be reconstructed and
validated separately before they are considered supported by the full-C
implementation.

Until the original vendor TLB setup is restored, DRAM training uses the direct
KSEG1 `0xa0080000` alias and bounded GDMA polling.

Use `make en7528` with a GNU MIPS little-endian toolchain, or
`make LLVM=1 en7528` with Clang/LLD.

The hardware run above predates the shared EN7512 calibration refactor.
See `../../includes/dramc/README.md` for its differential tests and the remaining
need for fresh board validation.
