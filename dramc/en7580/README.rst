EN7580 DDR calibration in C
==========================

``make en7580`` builds the C implementation in this directory. The PLL,
DDR3/DDR4 initialization, impedance, write leveling, gating, RX/TX windows,
latency, Vref, timing tables, register access, clocks and eFuse routines are
source code, not byte arrays containing executable instructions. Only the
entry point remains in ``entry.S``; ``spram.c`` uses inline assembly for
CP0/cache operations that C cannot express.

The payload executes at ``0x9fa30000``. ``ddr.lds`` still reserves the final
4 KiB for the stack and rejects code/data/BSS extending past ``0x9fa3b000``.
The target remains experimental pending validation on a physical board.

The EN7580 context has 88 bytes, frequency at offset 48, vendor ID at 52 and
pinmux at 64. Static assertions protect this ABI. DDR3/DDR4 identifiers are
4/5, and the DDR4 x16 pinmux is 3. These values intentionally differ from
the XX230v SDK header. The EN7580 PLL values and four AC timing rows are
preserved from its objects; the EN7523 PLL is not substituted.

Storage is represented by named globals, typed timing parameters and local
structures/arrays. Some internal calibration temporaries retain recovered
names where their original source name could not be established. Functions
that were empty in the vendor object remain empty; they are not placeholders
for missing implementations. The production preset uses one rank and x16.

Validation
----------

Build and run the normal image-format selftests::

  make en7580 test

An optional differential test requires GNU MIPS little-endian tools, Python 3,
``qemu-mipsel`` and the baseline commit in local Git history::

  make test-en7580-c
  CROSS_COMPILE=mipsel-linux-gnu- QEMU_MIPSEL=qemu-mipsel make test-en7580-c

The test rebuilds the original MIPS16 objects directly from commit
``9a0a8c9e747fd748ff20c31bb5627a406c9ad2b7``. Both versions run with the
same simulated register bank. It compares return values, operation counts,
two rolling hashes of ordered register reads/writes/delays, and every byte
of the 88-byte context. The 368 cases cover 23 entry paths under 16 profiles:
DDR3/DDR4, 1600/1866 MT/s, ODT off/on and test-engine pass/fail. Additional
PLL cases select 1200 and 1333 MT/s. TX/RX Vref-enabled paths are included.

Sixteen complete C calibration pipelines are also run with two different
stack patterns. This checks the intentional initialization correction below;
it is not a claim of equality with undefined original stack contents.

The port was checked with GCC 12.4.0, GNU binutils 2.42 and QEMU 8.2.2.
The complete EN7580 target and 43 existing image-format selftests passed.
That build produces a 41,440-byte DRAMC binary; its runtime including BSS
ends at ``0x9fa3a580`` (42,368 bytes), below the 45,056-byte SRAM limit.
These sizes describe the tested compiler and can change with toolchains.

These tests exercise the calibration algorithms, not real DDR signals or
clock/eFuse/cache hardware. The simulation returns immediate completion
status and constant comparison masks; it does not model analog training or
prove that every possible calibration window is correct. A board boot and
memory stress test remain necessary before treating the port as validated.

Intentional corrections
-----------------------

* ``DramcRxWindowPerbitCal(p, 0)`` returns failure. The object and XX230v
  source can use an uninitialized comparison mask in that unsupported
  configuration (DDR3_MPR_USED and DDR4_RDDQC_USED disabled). The production
  all-channel caller uses mode 1, which computes a test-engine result.
* DDR3 gating state and TX window state are initialized. The object can
  consume leftover stack bytes when no gating window is found, and in
  unused lane fields written by the TX finalization path. C writes defined
  zero defaults instead. Calibration failure flags are still recorded.
* eFuse extraction rejects invalid ranges and a null output buffer rather
  than permitting out-of-range access. Valid boot-time accesses keep the
  original bank-0 extraction and bank-1 repair behavior.
* The unused, uninitialized third argument to ``DramcRxdatlatScan`` is
  removed. The object never uses that argument.

Origin
------

The recovery follows the requested U-Boot interval from
``1a12c700b324709e7b996210991d03769eab85e8`` through
``cc706c324bfbf9121f0a76d9a0cc10e548edf06c`` (inclusive):

* ``f4c2de28301b72ef543958e524a2d33bb9b7ac7f`` introduced the encoded
  EN7580 assembly reconstructed from XGZ030 GPL objects.
* ``3e095fd9a10128692935cba27d23b9da08fa20c6`` added readable EN7528 C
  and EN751221 C reference files.
* ``2070dac83f586c42accceeb3b6fcdee00ee87428`` switched EN751221 to C.
* ``cc706c324bfbf9121f0a76d9a0cc10e548edf06c`` externalized the early
  boot sources, keeping the EN7580 encoded assembly.

U-Boot: https://sirherobrine23.com.br/airoha/u-boot

The standalone baseline is ``9a0a8c9e747fd748ff20c31bb5627a406c9ad2b7``.
Its ``dramc/en7580/reconstructed`` sources remain recoverable through Git;
neither those sources nor the intermediate assembly/C drafts are duplicated
alongside the active implementation.

XGZ030 reference: https://sirherobrine23.com.br/devices_gpl/xgz030_v1
at ``b3019cd03b879477feebc566193d8d32a0c55410``. The source directory is
``sdk/en7580/bootloader/Uboot/u-boot-2014.04-rc1/arch/arm/cpu/armv7/``
``en7523/ddr_cal/en7523``. It supplies shared algorithm/structure references,
but its EN7523 configuration is not a drop-in EN7580 implementation.

The supplied XX230v archive provides another vendor C reference. Its shared
EN7580 branches and important ABI differences are recorded in
``reference/xx230v-source-audit.rst``.
