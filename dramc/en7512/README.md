# Unified EN7512-family DRAMC (`en751221`, `en751627`, `en7528`)

EN751221, EN751627, and EN7528 share the same EN7512 DRAMC controller IP and
are built from this unified directory using compile-time SoC profiles
(`dramc/en7512/soc/<soc>/en7512_soc.h`), with no runtime chip-dispatch overhead.

## Layout

- `common/`:
  - `dramc_common.h`: common register accessors, constants, `ddr_phy_reset()`,
    and SoC hook prototypes
  - `dramc_core.c`: `dramc_calib()`, `check_column_bank()`, size probing,
    `set_TRFC()`, register dump, shared `dram_spcmd()` / `dram_mempll_finish()`,
    and `dramc_main()`
  - `dramc_dqs_gw_cal.c`: DQS gate-window calibration
  - `dramc_dle_cal.c`: DLE calibration
  - `dramc_dq_dqs_cal.c`: RX/TX per-bit DQ/DQS training (`DRAMC_TA2_SPLIT_RW`
    selects EN751627's sequential TA2 write/read path)
  - `dramc_test.c`: GDMA and TA1 tests
  - `entry.S`, `ddr.lds`: shared `0x9fa30000` SRAM startup (EN751627, EN7528)
  - `mmu.c`: KSEG2 1 GiB TLB mapping (EN751627)
- `soc/en751221/`: `en7512_soc.h`, `head.S` (`0x9fa32800` + `0x280` prefix),
  `ddr.lds`, `system.c`, `init.c`
- `soc/en751627/`: `en7512_soc.h`, `system.c`, `init.c`
- `soc/en7528/`: `en7512_soc.h`, `system.c`, `mempll.c`, `init.c`

UART and timer services come from the repository's shared `common/uart.c` and
`common/timer.c`.

## Preserved SoC parameters

| Parameter               | EN751221   | EN751627   | EN7528        |
| ----------------------- | ---------- | ---------- | ------------- |
| CPU byte order          | Big endian | Big endian | Little endian |
| DQS coarse search       | 0..31      | 6..20      | 6..20         |
| DQS fine search         | 0..95      | 0..95      | 0..95         |
| Gating test length      | 0xff       | 0x0f       | 0x0f          |
| Expected gating counter | 0xf8f8f8f8 | 0x78787878 | 0x78787878    |
| DRAM test CPU address   | 0xa0080000 | 0xc0080000 | 0xa0080000    |
| DMA/TA1 polling limit   | 0x7fffffff | 0x7fffffff | 0x00100000    |
| RX/TX TA2 mode          | Combined   | Split W/R  | Combined      |
| Max DRAM size probe     | 512 MiB    | 1024 MiB   | 1024 MiB      |

## Validation

Run the differential test against the pre-merge baseline:

```sh
make test-en7512
make LLVM=1 test-en7512
```
