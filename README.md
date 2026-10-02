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
- `en7580` (DRAMC reconstruction is still WIP)

## Build

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

Build all default SoCs:

```sh
make all
```

Include non-default experimental SoCs in `all`:

```sh
make EXPERIMENTAL_SOCS=1 all
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
out/en751221/bootext.bin
```

`en7528` additionally builds a standalone BootROM/XMODEM boot extension:

```text
out/en7528/bootext.bin
```

The normal `chainload.bin` is the post-DRAM flash reader.  The EN751221
recovery helper instead runs from DRAM after the vendor/BootROM DDR stage.  The
EN7528 `bootext.bin` is different again: it is a single FE-SRAM image containing
the EN7528 DRAMC stage followed by an SRAM-resident XMODEM chainloader.

### EN751221 recovery with DDR reinitialization (experimental)

`make en751221-bootext` builds `out/en751221/bootext.bin`. Unlike EN7528,
the EN751221 BootROM downloads this file to **DRAM at 0x80009000**, not
directly to FE SRAM. A small bootstrap verifies the CRC table covering both
embedded payloads, enables PBUS access to FE memory, and copies them with
32-bit stores and readback verification:

```text
0x9fa32800  in-tree EN751221 DRAMC (entry at 0x9fa32a80)
            runtime state, including the DRAMC stack, ends below 0x9fa38000
0x9fa38000  SRAM XMODEM receiver and its DDR handoff entry
            code, CRC table and BSS must end below 0x9fa3c000
```

The SRAM entry uses no DRAM stack while the DRAMC runs. It supplies the
return address through `SCREG_WR0`, applies the post-calibration SLM,
arbiter and SMC setup from `flash/en751221/boot2.S`, then enters the normal
receiver with a fresh DRAM stack. The existing receiver's image formats,
menu and default chainload action are unchanged. **The menu also offers
flash writing: do not select it for a RAM-only recovery test.**

For this EN751221 bootext only, LLVM builds use `-Oz`; GCC keeps `-Os`.
The 1 KiB XMODEM packet buffer lives on the cached DRAM stack after calibration,
rather than occupying the receiver's limited SRAM BSS. Other recovery targets
retain their static buffer. The SRAM addresses, CRC coverage and overlap
checks are unchanged. Headroom remains limited, so these checks must stay
enabled for new compiler versions.

Run toolchain builds sequentially in separate, initially empty output
directories, then verify each set of artifacts, for example:

```sh
make -j1 O="$PWD/out/gcc" CROSS_COMPILE=/path/to/mips-linux-gnu- en751221
make O="$PWD/out/gcc" CROSS_COMPILE=/path/to/mips-linux-gnu- en751221-bootext-check
make -j1 O="$PWD/out/clang" LLVM=1 en751221
make O="$PWD/out/clang" LLVM=1 en751221-bootext-check
```

This is needed because the XR500v's BootROM-only DDR setup differed from
the flash path: missing clock metadata, different PLL/DRAMC setup, and
missing QDMA descriptor DONE writeback. An earlier DDR-stage chainloader
restored working Ethernet and Linux startup on that board.

The initial standalone implementation was tested in one cold BootROM cycle on
an Archer XR500v (EN7526G, 256 MiB DDR3). The wrapper CRC, SRAM readback and
receiver CRC passed; DRAMC reported calibration status 0; U-Boot loaded over
XMODEM, and a 9.3 MiB Linux initramfs transferred by TFTP with matching CRC.
Linux booted with the watchdog active, CPU 900 MHz and bus 225 MHz. An 8 MiB
synthetic download and three 8 MiB uploads matched SHA-256; the system ran
for over ten minutes without the previous QDMA completion warning or reset.
The board was then power-cycled back to its existing flash image.

That hardware run predates the packet-buffer/LLVM-footprint correction above.
The corrected revision has passed clean GCC 14.4 and Clang 19.1.7/21.1.8
builds and artifact checks. Its Clang 21.1.8 artifact has since passed one
cold BootROM recovery on the same XR500v: DDR/readback/receiver CRC checks,
raw U-Boot reception, TFTP, Linux userspace with its watchdog active, and
an 8 MiB download plus three 8 MiB uploads with matching SHA-256. The board
was returned to its existing flash image afterward. The tested artifact was
built at `35d600c` (before the CI-only rebase), SHA-256
`6c1fa7b578014e606de3654f18e0d67a8421ca87f37cab4adec7cbecf38afe57`.
This does not validate repeated recovery, prolonged stress, other boards,
or the newly built GCC/Clang 19 artifacts.

This is a single-board smoke test, not repeated-cold-boot or long-duration
validation. The tested second stage was raw `u-boot.bin`; the receiver's
other input formats and flash-writing menu were not hardware-tested in
this run. Keep a known-good recovery helper when trying it on another board.
The original DRAM-only `en751221-recovery-chainloader.bin` remains available
unchanged for comparison.

The C host selftests cover CRC tables, embedded payload corruption,
truncation and SRAM boundary rejection. After building, check the actual
images and linker symbols with the same host utility and the cross `nm`:

```sh
make test
make en751221-bootext-check CROSS_COMPILE=/path/to/mips-linux-gnu-
```

`en751221-bootext-check` reads existing artifacts; it does not rebuild or
modify firmware. Neither check runs firmware or replaces a BootROM hardware
test. No Python dependency is needed.

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

## Standalone `tcboot.bin`

The TCBoot build is intentionally independent from U-Boot (or any other final
payload). Build the boot component with:

```sh
make en751221-tcboot
make en751627-tcboot
make en7528-tcboot
# or
make tcboot SOC=en7528
```

The result is `out/<soc>/tcboot.bin`, a 128 KiB base image ending at flash
offset `0x20000`. It contains the BootROM stages, DRAM calibration payload and
the generic post-DRAM flash loader, but no U-Boot/FIT/kernel payload.

The loader understands only the small ECNT payload descriptor (v1 and v2).
It does not parse uImage, FIT or ELF. The image builder chooses an arbitrary payload, load
address and entry address, and wraps those opaque bytes before appending them:

```sh
out/host/econet-image tcboot \
    --boot out/en7528/tcboot.bin \
    --payload /path/to/payload.bin \
    --load 0x81000000 \
    --entry 0x81000000 \
    --output /tmp/tcboot-with-payload.bin
```

`econet-image tcboot` updates the TCBoot payload bounds in the BootROM header,
recalculates its CRC, and appends an ECNT descriptor plus the payload. It emits
only the used bytes; an outer image pipeline such as OpenWrt can pad the
artifact to the flash partition size afterwards. This keeps U-Boot/FIP/FIT
selection entirely in the OpenWrt image recipe rather than in this build.

Optional post-DRAM compression is selected when attaching the payload:

```sh
make LLVM=1 tcboot SOC=en7528
out/host/econet-image tcboot \
    --boot out/en7528/tcboot.bin \
    --payload /path/to/u-boot.bin \
    --load 0x81000000 --entry 0x81000000 \
    --compression lzma \
    --output /tmp/tcboot-with-payload.bin
```

Use `--compression gzip` for gzip, or `none` (the default) for the original
uncompressed ECNT v1 format. Always rebuild the TCBoot base with this loader
before attaching compressed payloads; old loaders cannot read ECNT v2.
Compression runs on the host via `gzip -n -9` or `xz --format=lzma` (LZMA1,
1 MiB dictionary, lc=3/lp=0/pb=2). The tool uses child processes with argv,
without a shell. The input is a raw executable payload, not a precompressed
file or a FIT/uImage container. XZ containers and LZMA2 are not supported.

Compressed payloads use the 48-byte ECNT v2 header. It stores the algorithm,
stored/unpacked sizes, CRC32 of each form and a CRC32 covering the whole
header. The loader rejects malformed sizes, unsupported algorithms, extra
stream data and incomplete streams, then checks the unpacked CRC before
executing. The payload entry is validated against its unpacked range.
Gzip also checks its own trailer CRC32/ISIZE. LZMA-alone accepts a known
unpacked size or an end marker with the unknown-size header; lc+lp must be
at most 4 and the declared dictionary at most 16 MiB.

The loader runs after DRAM calibration. Its stack remains at `0x8007fff0`,
its 64 KiB decoder workspace is at `0x80100000`, and compressed input is at
`0x80200000`. Output stays in `0x81000000..0x81ffffff`. These areas are
disjoint and fit the existing minimum 32 MiB DRAM window. The input and
output are accessed through uncached aliases, with cache maintenance before
execution. No runtime heap is used; LZMA uses the output as its dictionary.

EN751627's 53 KiB vendor DRAMC leaves too little flash space for both decoders
below mi.conf. Its dedicated linker layout places calibration below `0xff00`
and the post-DRAM loader at `0x10000`, still inside the 128 KiB base. The
existing loader/DRAMC offset fields describe both intervals; mi.conf/PAGE
and the boot CRC remain reserved. Other SoCs retain the existing layout.

Validation commands:

```sh
make test                    # host image/descriptor checks
make test-compression        # host decoder and packaging checks; Python 3, gzip, xz
make test-mips-compression   # Clang, LLD, qemu-mips and qemu-mipsel
```

The descriptor and payload must fit in the 1 MiB replacement bootloader region.
The current loader accepts payload destinations in `0x81000000..0x81ffffff`;
`entry` may differ from `load` but must point inside the loaded payload.

## EN7528 `bootext.bin`

The EN7528 BootROM recovery path needs the DRAM stage and the XMODEM
chainloader in one FE-SRAM image.  Build it explicitly with:

```sh
make en7528-bootext
```

It is also built by the normal `make en7528` target.  The current layout is:

```text
0x9fa30000  EN7528 DRAMC / calibration entry
            ... runtime DRAMC image/BSS ends before 0x9fa35000
0x9fa35000  SRAM XMODEM chainloader
            ... CRC self-check table
```

The composite file places the chainloader at file offset `0x5000`, pads the
result to an XMODEM 128-byte boundary, and currently fits inside the reserved
48 KiB FE-SRAM image window.  The bootext-specific DRAMC entry writes
`0x9fa35000` to `SYS_BOOT_JUMP` (`0xbfb00280`); after calibration
`start_spram(1)` follows that vendor handoff and enters the SRAM chainloader.
The chainloader then receives U-Boot into DRAM and accepts raw, legacy, FIT or
ECNT images.

## Host image tool

The image finalizers are built as a native C utility at
`out/host/econet-image`.  The build no longer requires Python for image
packing or tests.  Its subcommands replace the former `tools/*.py` helpers:

```text
econet-image chainloader      # CRC table + XMODEM padding
econet-image tcboot-base      # build payload-free 128 KiB TCBoot component
econet-image tcboot           # attach an opaque payload + ECNT descriptor
econet-image flash            # legacy one-shot image packer
econet-image tcboot-finalize  # finalize legacy TCBoot/Binman images
econet-image bootext          # compose EN7528 DRAMC + SRAM chainloader
econet-image selftest         # host-side regression tests
```

The normal `make <soc>` targets build the host utility automatically.

## Tests

Host-side image-layout tests do not require a MIPS toolchain:

```sh
make test
```

## Build system layout

The root `Makefile` is the public entry point.  A `make <soc>` invocation starts
one isolated internal build instance for that SoC, then includes the component
fragments below.  This keeps per-SoC variables from leaking into other targets
while following the same metadata + shared build-helper model used by OpenWrt.

- `base/compiler.mk` — shared GNU/LLVM toolchain selection and endian setup.
- `soc/Makefile`, `soc/*.mk` — SoC registry, capabilities and toolchain metadata.
- `dramc/<soc>/` — DRAM initialization/calibration source or preserved payload; each per-SoC `Makefile` only describes its DRAMC inputs/quirks.
- `dramc/Makefile` — DRAMC build fragment and common compile/link helpers.
- `flash/Makefile` — post-DRAM `chainload` and optional `tcboot` build fragment.
- `chainloader/Makefile` — SRAM/DRAM XMODEM recovery-chainloader build fragment.
- `bootext/Makefile`, `bootext/*.mk` — bootext build fragment plus per-SoC SRAM/layout metadata.
- `include/` — standalone early-boot headers; no U-Boot include tree required.
- `tools/` — the native `econet-image.c` host image tool; the build path has no shell/Python helper scripts.

## SDK TRX and secure headers (EN7580)

`econet-image` also handles the VX830v SDK's 256-byte TRX header and the
native-endian `SECURE_HEADER_V1` (276 bytes) / `SECURE_HEADER_V2` (2048 bytes).
The legacy V1 layout also uses version field `0`, as observed in the supplied
EN7580 dump; `--secure-version 0` and `sheader --version 0` select that encoding.
These secure headers are a different format from the big-endian ECNT payload
descriptor used by our post-DRAM loader. They cannot be interchanged.

The format offsets come from the supplied VX830v `tools/trx/{trx.c,trx.h}` and
SDK `global_inc/uapi/boot/sHeader.h`. Signed boot ranges follow the SDK's
`boot/bootrom/Makefile`, and payload length handling follows
`boot/bootrom/verify/main.c`. The SDK's `secure_header/sheader` implementation
was not included in the supplied archives. This is an independent format
implementation, not a source-level port of that missing program.

TRX and secure-header CRC32 use the SDK's no-final-XOR convention. The existing
ECNT descriptor continues to use conventional IEEE CRC32. All reserved TRX
bytes are initialized, unlike the vendor tool's uninitialized malloc buffer.

### Firmware TRX

```sh
make hosttools
out/host/econet-image trx \
    --kernel /path/to/linux.7z \
    --rootfs /path/to/rootfs.bin \
    --magic hdr2 --endian little \
    --firmware-version 2026.10 --model EN7580 \
    --load 0x81f00000 \
    --secure-version 1 --key /path/to/private.pem \
    --verify-key /path/to/public.pem \
    --output /tmp/tclinux-signed.bin
```

Use `--secure-version 2` for V2 or `--secure-version 0` for legacy V1.
Omitting the secure version creates an unsigned
TRX header. Optional `--romfile`, `--customer-version` and `--align` are
supported. Components are concatenated kernel/rootfs/romfile; alignment applies
to the total image and padding is counted in the last component. The signed
range is exactly the entire payload after `header_len`, including padding.
The three component lengths partition that range. Header magic is explicit
(`hdr0`/`hdr1`/`hdr2`/`hdr3`), independent of filenames.

The TRX header does not itself create an RSA signature: the secure header holds
that signature. V1 requires RSA-2048; V2 supports RSA-2048/3072/4096. The exponent
must be 65537. PKCS#1 v1.5 with SHA-256 is used. EN7580's verifier hashes chunks
of at most `0xfffff` bytes: one chunk uses SHA256(payload); multiple chunks use
SHA256(SHA256(chunk0) || SHA256(chunk1) || ...). A whole-file `openssl dgst
-sha256` signature is therefore incompatible with multi-chunk firmware.

Only PEM signing/verification invokes the installed `openssl` command, with
argv and temporary file descriptors; no shell or libcrypto build dependency is
used. The host implementation requires Linux `/proc/self/fd`. Private PEM keys
must be unencrypted (encrypted keys fail without prompting). Signing uses only
the key supplied by the caller; it does not create a key trusted by the device.

To use a signature already generated by the SDK, replace `--key` with
`--signature /path/to/image.sig`. Raw bytes or whitespace-separated hexadecimal
text are accepted (`--signature-format auto|raw|hex`). Use `--verify-key` to
check an imported signature against the exact payload before writing output.

### Prelinked boot image secure headers

The input image must already reserve the secure-header slots. This command
updates them in place; it never prepends a header, moves code, or changes the
BootROM stage offsets. In particular, it cannot convert our raw `tcboot.bin`
into a ROM-compatible secure-boot layout by shifting its bytes.

```sh
out/host/econet-image sheader \
    --image /path/to/tcboot-with-reserved-headers.bin \
    --boot-part 1 --version 1 --endian little \
    --key /path/to/private.pem --verify-key /path/to/public.pem \
    --output /tmp/tcboot-part1.bin
out/host/econet-image sheader \
    --image /tmp/tcboot-part1.bin \
    --boot-part 2 --version 1 --endian little \
    --key /path/to/private.pem --verify-key /path/to/public.pem \
    --output /tmp/tcboot-signed.bin
```

`--boot-part` requires a 256 KiB boot area. It selects headers at `0` and
`0x20000`, and signed ranges ending at `0x1fffc` and `0x3fdfc`, respectively.
`image_len` is the signed length plus four, as expected by `rsa_verify()`.
Both boot CRC slots are recalculated after each operation. The configuration
and reserved tail remain byte-identical. For other containers use `--offset`
and `--length` (the number of signed bytes after the header). Existing matching
headers can supply their version and signed size when those options are omitted.
This preserves an existing legacy version field of zero when re-signing it.
Empty/uninitialized slots require an explicit version and signed size.

V2 public-key wrapping and image encryption are not generated by these
commands. Existing IV/HMAC/AES mode, wrapped RSA modulus and reserved bytes are
preserved. `--secure-template` accepts an exact same-version/endian header;
`--rsa-pub` accepts 512 bytes of SDK key material, and `--rsa-info` accepts the
16-byte IV plus 32-byte HMAC at `+0x544`. These options are also accepted by
`trx`. Supply the correct prepared metadata/template for a ROM-consumed V2
header; the raw PEM public key is not a substitute for the wrapped key blob.
Signing covers the stored bytes, including ciphertext if already encrypted.
There is no automatic key provisioning or hardware secure-boot verification.

### SDK CRC and external signing helpers

```sh
# Equivalent to trx -t for a 256 KiB boot area (two chained CRC slots).
out/host/econet-image trx-crc \
    --image /path/to/tcboot.bin --boot-size 0x40000 \
    --endian little --output /tmp/tcboot-crc.bin
# Equivalent to appending the SDK CRC to RSA public-key/provisioning data.
out/host/econet-image trx-crc \
    --image /path/to/rsa_pub.bin --append --output /tmp/rsa_pub-crc.bin
# Export the exact 32-byte EN7580 digest for an external signer.
out/host/econet-image secure-digest \
    --image /path/to/payload.bin --output /tmp/payload.digest
```

`trx-crc --boot-size` also supports `0x10000` and `0x20000`. It preserves file
size and all bytes outside the CRC slots. `secure-digest` accepts `--offset`
and `--length` to select a nonempty payload range.

Build with `make hosttools`; `make test` runs the existing C host selftests.
Independent validation covered both byte orders, V1/V2 headers, chunk
boundaries, 2048/3072/4096-bit signatures, SDK signature imports, metadata
preservation and malformed inputs. Unsigned TRX output was also compared
byte-for-byte with the supplied VX830v tool, with its otherwise undefined
allocation/config buffers initialized. No Python files or runtime dependency
are added by this change. Booting the output on hardware remains a separate
validation step.

## Inspect existing images

```sh
make hosttools
out/host/econet-image inspect --image /path/to/tclinux.bin
out/host/econet-image inspect --image /path/to/tclinux-signed.bin --verify-key /path/to/public.pem
out/host/econet-image inspect --image /path/to/tcboot.bin --soc en7580
out/host/econet-image inspect --image /path/to/flash.bin --offset 0x20000 --format ecnt
```

`inspect` reads the input without modifying it. It prints TRX component
offsets/lengths, version strings, model and load address; secure-header version,
signed range, signature slot, SDK digest and V2 RSA/AES metadata; or the ECNT
loader's load/entry addresses, compression and stored/unpacked sizes. CRC
reports show both stored and calculated values, with `OK` or `BAD`.

`--format auto|trx|sheader|ecnt|tcboot` selects the parser. The default is `auto`.
TRX and secure-header byte order is inferred from magic/version unless
`--endian little|big` is supplied. ECNT loader descriptors always use big
endian. Legacy V1 version zero requires inference from the header CRC or a
uniquely bounded `image_len`; ambiguous inputs require an explicit `--endian`.
ECNT loader descriptors use version fields 1/2 only. Secure V1 accepts 0/1,
and secure V2 accepts 2, matching the layouts used by the supplied SDK/dump.
Both ECNT formats share their magic; use an explicit `--format` when
inspecting an ambiguous or damaged header. `--offset` selects an embedded
header; TCBoot inspection requires offset zero.

Raw TCBoot requires `--soc en751221|en751627|en7528|en7580` to select the
endian and boot CRC slots. `--endian` alone prints the raw header without
validating its boot CRCs. Two secure headers at zero and `0x20000` identify the
256 KiB secure boot layout automatically; otherwise select `--format tcboot`
explicitly for a secure boot image. Boot CRCs and any recognized ECNT loader
at `0x20000` are checked as appropriate to the selected layout.

Without `--verify-key`, RSA signatures are explicitly reported as
`NOT CHECKED`; a matching CRC does not prove signature validity. Verification
uses the supplied public PEM and the SDK chunked digest. V2's 512-byte signature
slot does not itself identify the RSA modulus size; this comes from the public
key. Key-material fingerprints identify opaque SDK blobs, not trusted keys.
ECNT inspection checks stored data, without decompression or validation of the
unpacked CRC. It does not verify whether the device trusts the supplied key.

Exit status is `0` when all applicable checks pass, `1` for malformed data,
CRC mismatches or failed signature verification, and `2` for command usage
errors. Missing files or invalid options may also return a nonzero status.
Run `make test-inspect` for the C fixture/CLI tests and `make test` for the
existing C host selftests. No Python files are added.
