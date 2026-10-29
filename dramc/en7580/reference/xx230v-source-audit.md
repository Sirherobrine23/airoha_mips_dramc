# XX230v DRAMC source comparison

Input: `tplink-xx230v_v1-main.tar.gz` supplied with this recovery.
SHA-256:

```text
0a57ee3df2113443f21f98d16c0ecef9642d38af39fd1be572244484a13a83db
```

The relevant source root inside the archive is:

```text
tplink-xx230v_v1/sdk/en7529/bootloader/Uboot/u-boot-2014.04-rc1/
  arch/arm/cpu/armv7/en7523/ddr_cal/en7523/
```

These files are useful vendor C references for the shared DRAMC algorithms,
register field names and original local-variable/structure names. They are
an EN7523 configuration with surviving EN7580 branches; they are not a
drop-in C replacement for the EN7580 MIPS objects.

## Evidence of shared EN7580 code

`dramc_pi_calibration_api.c` has an explicit
`#elif (fcFOR_CHIP_ID == EN7580)` branch in `DramcWriteLeveling`. It
programs `WRITE_LEV_DQSBX_G` to `0xa`, then enables write leveling and
`WRITE_LEV_CBTMASKDQSOE`. The EN7580 object performs the corresponding
masked writes at virtual register `0x300bc`:

```text
value 0x00000a00, mask 0x00000f00
value 0x00000001, mask 0x00000001
value 0x00000020, mask 0x00000020
```

`dramc_register.h` uses the `_EN7580_REGISTER_H_` guard, and the
ASIC physical register bases match the physical addresses represented by
the EN7580 object's KSEG1 aliases:

| Block | XX230v ASIC | EN7580 KSEG1 |
| --- | --- | --- |
| DRAMC NAO | 0x1fb22000 | 0xbfb22000 |
| DRAMC AO | 0x1fb24000 | 0xbfb24000 |
| DDRPHY B01 | 0x1fb28000 | 0xbfb28000 |
| DDRPHY CMD | 0x1fb26000 | 0xbfb26000 |

`hal_io.c` and `x_hal_io.h` match the earlier XGZ030 GPL C reference
byte-for-byte after line-ending normalization. The remaining source files
differ; in particular, XX230v adds DDR2, package and crystal-selection work.

## ABI and configuration differences

The uploaded `dramc_pi_api.h` selects `fcFOR_CHIP_ID EN7523`. The PLL
function is `DDRPhyPLLSetting_EN7523`, and this revision enables DDR2 and
DDR3. `ecnt_system.c` only mentions the EN7580 clock/setup functions in
commented-out calls within the EN7523 setup path.

The following XX230v values were checked with the GNU MIPS o32 compiler;
the EN7580 values come from its preserved objects and recovered context:

| Property | EN7580 | XX230v |
| --- | --- | --- |
| sizeof(DRAMC_CTX_T) | 88 | 92 |
| frequency offset | 48 | 48 |
| clk_src offset | absent | 52 |
| vendor_id offset | 52 | 56 |
| pinmux offset | 64 | 68 |
| TYPE_PCDDR3 | 4 | 5 |
| TYPE_PCDDR4 | 5 | 6 |
| DDR4 x16 pinmux value | 3 | 5 |

XX230v inserts `TYPE_PCDDR2` before DDR3 and adds `clk_src` to the
context. Its pinmux enum also inserts DDR2/package modes. Importing these
definitions unchanged would reinterpret both EN7580 context fields and
the preserved timing-table type identifiers. The recovered header now
names the EN7580 DDR3/DDR4 values and checks its critical field offsets.

Changing only `fcFOR_CHIP_ID` is insufficient: the uploaded pinmux enum
and physical-base table are themselves guarded by the EN7523 selector.
Each shared routine must be audited under the EN7580 ABI/configuration.

## RDDQC finding

The apparent inherited-register input in `DramcRxWindowPerbitCal` has a
source-level explanation. The uploaded routine declares `u4err_value`
without an initializer. It assigns it from `TestEngineCompare` when
`u1UseTestEngine` is nonzero. The alternative assignment exists only for
DDR3 with `COMPILE_PCDDR3 && DDR3_MPR_USED`; the supplied header sets
`DDR3_MPR_USED` to zero, and also sets `DDR4_RDDQC_USED` to zero.

Thus mode zero can reach the per-bit comparison without a newly computed
error mask. The EN7580 instructions exhibit that same missing assignment.
The C implementation rejects this unsupported mode explicitly. Its production
all-channel entry calls `DramcRxWindowPerbitCal(p, 1)`, preserving the normal
test-engine calibration path. This is an intentional correction to undefined
original behavior, not a claim of binary identity.

The active implementation uses typed state and timing tables and preserves
the EN7580 PLL/DDR4 values. See the parent README for the differential tests,
additional initialization corrections and hardware-validation limits.
