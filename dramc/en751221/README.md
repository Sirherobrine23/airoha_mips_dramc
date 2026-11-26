# EN751221 DDR calibration source

The EN751221 DDR calibration stage is built entirely from source.

The calibration routines recovered by Merbanan are combined with
the original GPL bootrom startup, UART, timer, string and SPRAM implementations.
Legacy BSP headers are not build dependencies; the required EN751221 register
interface is provided by `en751221_ddr.h` and `../../includes/dramc/regs.h`.
The DLE and DQS gate-window algorithms are shared with EN7528 under
`../../includes/dramc`; `en7512_soc.h` preserves the EN751221 search parameters.
`dramc_test.c` retains this port's DMA and TA1 routines.

The original stage executes from FE SRAM at `0x9fa32800`. Keep the source
order in `images/en751221.mk` synchronized with the early entry and
support code from the EN7512 V1.2.2 stage:

`head, setup, main, init, time, string, dramc, dramc_dq_dqs_cal, dramc_test,
dramc_dle_cal, dramc_dqs_gw_cal, en7512_dramc_init, spram`.

The vendor recovery baseline was a 20336-byte `spram.img`. A source build
using a newer compiler is not required to be byte-identical; the ABI, SRAM
VMA and hardware behaviour are the acceptance criteria.

The normal standalone build is `make en751221`.  The DRAM payload is
written to `out/en751221/en751221-dramc.bin` and is not a checked-in
firmware blob.

See `../../includes/dramc/README.md` for the shared-code differential test and its
hardware-validation limits.
