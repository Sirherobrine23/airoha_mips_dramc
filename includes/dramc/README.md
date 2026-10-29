# Shared EN7512 DRAMC calibration

EN751221 and EN7528 build the same DLE and DQS gate-window translation
units from this directory. `regs.h` also provides their common register
offsets and test-agent bit definitions. The device include path selects
`en7512_soc.h` from the corresponding SoC directory; no runtime chip
detection or indirect MMIO operations are added to firmware.

The shared files live under `includes/dramc/`. Builders expose the
repository's `includes/` directory globally, so SoC headers can include
`<dramc/regs.h>` and `<asm/io.h>` without relative paths. Shared source
objects are selected explicitly by `DRAMC_SHARED_DIR` and
`DRAMC_SHARED_OBJS` in each image definition.

The family relationship is corroborated by the supplied vendor SDK at
`boot/bootrom/ddr_cal_en7512`. Its Makefile selects EN7512, EN7521,
EN7516, EN7527 and EN7528 within that platform, and its `inc/dramc.h`
contains common register definitions and variant-specific calibration
parameters. The EN7516 and EN7528 demo objects have identical DLE and
gate-window instruction words after endian normalization, including
equivalent symbolic relocations. Their RX/TX and initialization objects
differ and are not unified here.

## Preserved differences

| Parameter | EN751221 | EN7528 |
| --- | --- | --- |
| CPU byte order | Big endian | Little endian |
| DQS coarse search | 0..31 | 6..20 |
| DQS fine search | 0..95 | 0..95 |
| Gating test length | 0xff | 0x0f |
| Expected gating counter | 0xf8f8f8f8 | 0x78787878 |
| DRAM test CPU address | 0xa0080000 | 0xa0080000 |
| DMA/TA1 polling limit | 0x7fffffff | 0x00100000 |

Each `dramc_test.c` retains its original DMA access qualifiers, polling
and TA1 code. Clock/PLL, eFuse/package detection, DRAM geometry, initial
timings and RX/TX per-bit training remain in the SoC directories.
EN751627 still uses its preserved vendor payload; EN7580 uses its separate
DDR3/DDR4 implementation.

The shared algorithms retain the original selection rules: DLE chooses
the second tap of the first adjacent passing pair; gating chooses the
coarse row with the most passing taps, preferring the later row on ties,
then adds half the pass count to the first passing fine tap. Disjoint
windows and repeated-calibration state intentionally retain the old
behaviour rather than adopting a different window-selection policy.

## Validation

Run the differential test with GNU MIPS tools and QEMU:

```sh
make test-en7512
CROSS_COMPILE=mips64-linux-gnu- make test-en7512
```

Or use the installed Clang/LLD with the same QEMU interpreters:

```sh
make LLVM=1 test-en7512
```

The test reads the pre-merge sources from Git commit
`6896ba27a1c111d550adc785fcfad76cd7894b13` and compares them with the
shared implementation under MIPS big- and little-endian execution. Both
paths use identical simulated register and DMA responses. The 36 cases
compare return values, selected taps, result bitmaps, final registers and
two hashes of ordered MMIO reads/writes, delays and DMA-test requests.
They also assert expected selections independently for empty/full,
boundary-crossing, tied, disjoint and repeated calibration windows.

The model completes test-agent requests immediately and substitutes a
deterministic DMA result. It does not exercise analog DDR signals, actual
DMA/cache ordering or timeout expiry. Compilation and simulated agreement
do not replace a fresh board boot and memory stress test after this merge.
