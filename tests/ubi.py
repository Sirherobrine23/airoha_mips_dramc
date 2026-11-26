#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
"""Host and QEMU MIPS tests for the freestanding UBI loader and bounded decoder."""
import ctypes
import gzip
import lzma
import os
from pathlib import Path
import random
import shutil
import struct
import subprocess
import sys
import tempfile
import zlib

ROOT = Path(__file__).resolve().parents[1]
DECOMP_SOURCES = [
    'decompress.c', 'decompress-memory.c',
    'decompress/LzmaDec.c', 'decompress/inflate.c', 'decompress/inffast.c',
    'decompress/inftrees.c', 'decompress/adler32.c', 'decompress/crc32.c',
    'decompress/zutil.c',
]
DECOMP_FLAGS = [
    '-DZ_SOLO', '-DNOBYFOUR', '-D_LZMA_SIZE_OPT',
    '-I' + str(ROOT / 'flash/decompress'), '-I' + str(ROOT / 'flash'),
]

UBI_EC_MAGIC = 0x55424923
UBI_VID_MAGIC = 0x55424921
UBI_LAYOUT_VOL_ID = 0x7fffefff


def ubi_crc32(data: bytes) -> int:
    """Linux UBI CRC-32 (IEEE 802.3 without final inversion)."""
    return zlib.crc32(data) ^ 0xffffffff


def run_ubinize(temp: Path, name: str, peb_size: int, min_io: int, volumes: list[dict]) -> bytes:
    """Generate a real UBI image using mtd-utils ubinize."""
    cfg_lines = []
    for idx, vol in enumerate(volumes):
        data_file = temp / f'{name}_vol{idx}.bin'
        data_file.write_bytes(vol['data'])
        cfg_lines.append(f'[vol{idx}]')
        cfg_lines.append('mode=ubi')
        cfg_lines.append(f'image={data_file}')
        cfg_lines.append(f'vol_id={vol.get("vol_id", idx)}')
        cfg_lines.append(f'vol_type={vol.get("vol_type", "static")}')
        cfg_lines.append(f'vol_name={vol["name"]}')
        if 'vol_size' in vol:
            cfg_lines.append(f'vol_size={vol["vol_size"]}')
        cfg_lines.append('')
    cfg_path = temp / f'{name}.cfg'
    out_path = temp / f'{name}.ubi'
    cfg_path.write_text('\n'.join(cfg_lines))
    subprocess.run(
        ['ubinize', '-o', str(out_path), '-p', str(peb_size), '-m', str(min_io), str(cfg_path)],
        check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    )
    return out_path.read_bytes()


def patch_vid_hdr(peb: bytearray, vid_off: int, *, sqnum=None, copy_flag=None, data_crc=None,
                  data_size=None):
    if copy_flag is not None:
        peb[vid_off + 6] = copy_flag
    if data_size is not None:
        struct.pack_into('>I', peb, vid_off + 20, data_size)
    if data_crc is not None:
        struct.pack_into('>I', peb, vid_off + 32, data_crc)
    if sqnum is not None:
        struct.pack_into('>Q', peb, vid_off + 40, sqnum)
    crc = ubi_crc32(bytes(peb[vid_off:vid_off + 60]))
    struct.pack_into('>I', peb, vid_off + 60, crc)


def find_pebs(ubi: bytes, peb_size: int):
    pebs = []
    for off in range(0, len(ubi), peb_size):
        chunk = ubi[off:off + peb_size]
        if len(chunk) < 64 or struct.unpack_from('>I', chunk, 0)[0] != UBI_EC_MAGIC:
            continue
        vid_off, data_off = struct.unpack_from('>II', chunk, 16)
        vid = chunk[vid_off:vid_off + 64]
        if struct.unpack_from('>I', vid, 0)[0] == UBI_VID_MAGIC:
            vol_id, lnum = struct.unpack_from('>II', vid, 8)
            sqnum = struct.unpack_from('>Q', vid, 40)[0]
            pebs.append((off, vid_off, data_off, vol_id, lnum, sqnum))
    return pebs


class UbiInfo(ctypes.Structure):
    _fields_ = [
        ('peb_size', ctypes.c_uint32),
        ('leb_size', ctypes.c_uint32),
        ('usable_leb_size', ctypes.c_uint32),
        ('scanned_pebs', ctypes.c_uint32),
        ('vol_id', ctypes.c_uint32),
        ('data_size', ctypes.c_uint32),
        ('vol_type', ctypes.c_uint8),
        ('vol_name', ctypes.c_char * 32),
    ]


READ_CB = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_uint32, ctypes.c_void_p, ctypes.c_size_t,
                           ctypes.c_void_p)


def build_host_lib(temp: Path) -> ctypes.CDLL:
    cc = os.environ.get('HOSTCC', 'cc')
    libpath = temp / 'ubi_test.so'
    strict_srcs = [ROOT / 'flash/ubi.c', ROOT / 'flash/decompress.c']
    vendor_srcs = [
        ROOT / 'flash' / s for s in DECOMP_SOURCES
        if s not in ('decompress.c', 'decompress-memory.c')
    ]
    objs = []
    for idx, src in enumerate(strict_srcs):
        obj = temp / f'strict_{idx}.o'
        subprocess.run(
            [cc, '-fPIC', '-O2', '-std=gnu11', '-Wall', '-Wextra', '-Werror',
             *DECOMP_FLAGS, '-c', str(src), '-o', str(obj)],
            check=True,
        )
        objs.append(str(obj))
    for idx, src in enumerate(vendor_srcs):
        obj = temp / f'vendor_{idx}.o'
        subprocess.run(
            [cc, '-fPIC', '-O2', '-std=gnu11', *DECOMP_FLAGS, '-c', str(src), '-o', str(obj)],
            check=True,
        )
        objs.append(str(obj))
    subprocess.run([cc, '-shared', *objs, '-o', str(libpath)], check=True)
    lib = ctypes.CDLL(str(libpath))
    lib.econet_ubi_load_volume.argtypes = [
        READ_CB, ctypes.c_void_p, ctypes.c_uint32, ctypes.c_uint32,
        ctypes.POINTER(ctypes.c_char_p), ctypes.c_size_t,
        ctypes.c_void_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t,
        ctypes.POINTER(UbiInfo),
    ]
    lib.econet_ubi_load_volume.restype = ctypes.c_int
    lib.econet_decompress_bounded.argtypes = [
        ctypes.c_uint32, ctypes.c_void_p, ctypes.c_size_t,
        ctypes.c_void_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_size_t),
        ctypes.c_void_p, ctypes.c_size_t,
    ]
    lib.econet_decompress_bounded.restype = ctypes.c_int
    return lib


def call_ubi(lib, flash_bytes: bytes, ubi_offset: int, max_scan: int = 0x10000000,
             names=(b'u-boot', b'uboot', b'bootloader'), wrap_size: int = 0,
             dst_cap: int = 1024 * 1024):
    read_count = 0

    def reader(off, dst, length, _ctx):
        nonlocal read_count
        read_count += 1
        out = bytearray(length)
        for i in range(length):
            addr = off + i
            if wrap_size:
                addr %= wrap_size
            out[i] = flash_bytes[addr] if addr < len(flash_bytes) else 0xff
        ctypes.memmove(dst, bytes(out), length)
        return 0

    cb = READ_CB(reader)
    arr = (ctypes.c_char_p * len(names))(*names)
    dst_buf = ctypes.create_string_buffer(dst_cap)
    work_buf = ctypes.create_string_buffer(65536 + 8)
    work_addr = (ctypes.addressof(work_buf) + 7) & ~7
    info = UbiInfo()
    ret = lib.econet_ubi_load_volume(
        cb, None, ubi_offset, max_scan, arr, len(names),
        dst_buf, dst_cap, ctypes.c_void_p(work_addr), 65536, ctypes.byref(info),
    )
    return ret, info, dst_buf.raw[:info.data_size] if ret == 0 else b'', read_count


def run_host_tests(lib, temp: Path) -> int:
    rng = random.Random(75287512)
    checks = 0
    boot_hdr = b'\x10\x00\x00\x0f' + b'TCBOOT_HEADER_01' + b'\x00' * (0x20000 - 20)

    # 1. Dynamic PEB detection across 64K (NOR), 128K (2K NAND), 256K (4K NAND), static + dynamic.
    for peb_size, min_io, vtype, vname in [
        (0x10000, 64, 'static', 'u-boot'),
        (0x20000, 2048, 'static', 'u-boot'),
        (0x20000, 2048, 'dynamic', 'uboot'),
        (0x40000, 4096, 'static', 'bootloader'),
    ]:
        payload = rng.randbytes(peb_size * 2 + 1337)
        vols = [
            {'name': 'kernel', 'vol_type': 'dynamic', 'vol_size': peb_size, 'data': b'K' * 4096},
            {'name': vname, 'vol_type': vtype, 'vol_size': len(payload) + peb_size, 'data': payload},
        ]
        ubi = run_ubinize(temp, f'geom_{peb_size}_{vtype}', peb_size, min_io, vols)
        offset = peb_size
        flash = (boot_hdr[:offset].ljust(offset, b'\x00')) + ubi
        ret, info, data, _ = call_ubi(lib, flash, offset)
        assert ret == 0, (peb_size, vtype, vname, ret)
        assert info.peb_size == peb_size, (info.peb_size, peb_size)
        assert info.vol_name.decode() == vname
        if vtype == 'static':
            assert data == payload
        else:
            assert data[:len(payload)] == payload
        checks += 1

    # 2. Priority ordering: "u-boot" must win over "bootloader" even with a higher vol_id.
    p_bl = rng.randbytes(4096)
    p_ub = rng.randbytes(8192)
    ubi = run_ubinize(temp, 'prio', 0x20000, 2048, [
        {'name': 'bootloader', 'vol_id': 0, 'vol_type': 'static', 'data': p_bl},
        {'name': 'u-boot', 'vol_id': 1, 'vol_type': 'static', 'data': p_ub},
    ])
    ret, info, data, _ = call_ubi(lib, boot_hdr + ubi, 0x20000)
    assert ret == 0 and info.vol_name == b'u-boot' and info.vol_id == 1 and data == p_ub
    checks += 1

    # 3. Bad block at PEB 0 + bad block mid-partition.
    p_bad = rng.randbytes(0x25000)
    ubi = run_ubinize(temp, 'badblocks', 0x20000, 2048, [
        {'name': 'u-boot', 'vol_type': 'static', 'data': p_bad},
    ])
    bad_peb = b'\x00' * 0x20000
    erased_peb = b'\xff' * 0x20000
    # Insert bad PEB before PEB 0 and erased PEB between PEB 0 and PEB 1.
    mangled = bad_peb + ubi[:0x20000] + erased_peb + ubi[0x20000:]
    ret, info, data, _ = call_ubi(lib, boot_hdr + mangled, 0x20000)
    assert ret == 0 and info.peb_size == 0x20000 and data == p_bad
    checks += 1

    # 4. Duplicate LEB with higher sqnum + interrupted copy_flag=1 with bad data_crc.
    p_orig = rng.randbytes(0x28000)
    ubi = run_ubinize(temp, 'sqnum', 0x20000, 2048, [
        {'name': 'u-boot', 'vol_type': 'static', 'data': p_orig},
    ])
    pebs = find_pebs(ubi, 0x20000)
    leb0 = [p for p in pebs if p[3] == 0 and p[4] == 0][0]
    off0, vid_off, data_off, _, _, sq0 = leb0
    leb_len = 0x20000 - data_off

    # Newer valid copy of LEB 0 with modified bytes.
    new_leb0_data = rng.randbytes(leb_len)
    newer_peb = bytearray(ubi[off0:off0 + 0x20000])
    newer_peb[data_off:data_off + leb_len] = new_leb0_data
    patch_vid_hdr(newer_peb, vid_off, sqnum=sq0 + 100, copy_flag=1,
                  data_size=leb_len, data_crc=ubi_crc32(new_leb0_data))

    # Even newer interrupted copy (copy_flag=1, bad data_crc) placed BEFORE the valid one!
    corrupt_peb = bytearray(newer_peb)
    corrupt_peb[data_off] ^= 0x55
    patch_vid_hdr(corrupt_peb, vid_off, sqnum=sq0 + 200, copy_flag=1,
                  data_size=leb_len, data_crc=ubi_crc32(new_leb0_data))

    combined = ubi + bytes(corrupt_peb) + bytes(newer_peb)
    ret, info, data, _ = call_ubi(lib, boot_hdr + combined, 0x20000)
    assert ret == 0
    assert data == new_leb0_data + p_orig[leb_len:]
    checks += 1

    # 5. Corrupted layout volume copy 0 falls back to copy 1.
    ubi_vtbl = bytearray(ubi)
    layout0 = [p for p in pebs if p[3] == UBI_LAYOUT_VOL_ID and p[4] == 0][0]
    ubi_vtbl[layout0[0] + layout0[2] + 5] ^= 0xff  # corrupt vtbl copy 0 record CRC
    ret, info, data, _ = call_ubi(lib, boot_hdr + bytes(ubi_vtbl), 0x20000)
    assert ret == 0 and data == p_orig
    checks += 1

    # 6. upd_marker=1 on "u-boot" causes loader to ignore incomplete volume and pick "uboot".
    p_good = rng.randbytes(4096)
    ubi_upd = bytearray(run_ubinize(temp, 'upd', 0x20000, 2048, [
        {'name': 'u-boot', 'vol_id': 0, 'vol_type': 'static', 'data': b'BAD' * 500},
        {'name': 'uboot', 'vol_id': 1, 'vol_type': 'static', 'data': p_good},
    ]))
    for off, _, doff, vid, _, _ in find_pebs(bytes(ubi_upd), 0x20000):
        if vid == UBI_LAYOUT_VOL_ID:
            rec_off = off + doff  # vol_id 0 record
            ubi_upd[rec_off + 13] = 1  # upd_marker = 1
            crc = ubi_crc32(bytes(ubi_upd[rec_off:rec_off + 168]))
            struct.pack_into('>I', ubi_upd, rec_off + 168, crc)
    ret, info, data, _ = call_ubi(lib, boot_hdr + bytes(ubi_upd), 0x20000)
    assert ret == 0 and info.vol_name == b'uboot' and data == p_good
    checks += 1

    # 7. Missing bootloader volume -> ECONET_UBI_NO_VOLUME (2); non-UBI -> ECONET_UBI_NO_UBI (1).
    ubi_no_bl = run_ubinize(temp, 'nobl', 0x20000, 2048, [
        {'name': 'rootfs', 'vol_type': 'dynamic', 'vol_size': 0x20000, 'data': b'R' * 4096},
    ])
    ret, _, _, _ = call_ubi(lib, boot_hdr + ubi_no_bl, 0x20000)
    assert ret == 2, ret
    ret, _, _, _ = call_ubi(lib, boot_hdr + b'ECNT' + b'\x00' * 0x40000, 0x20000)
    assert ret == 1, ret
    checks += 2

    # 8. Multi-offset probing simulation: UBI at 0x40000 (256K PEB) and 0x100000 (128K/256K PEB)
    #    with ECNT present at 0x20000, verifying probe window does not cross into 0x100000.
    for ubi_off, peb_size, min_io in [
        (0x40000, 0x40000, 4096),
        (0x100000, 0x20000, 2048),
        (0x100000, 0x40000, 4096),
    ]:
        p_multi = rng.randbytes(0x12345)
        ubi_m = run_ubinize(temp, f'multi_{ubi_off:x}_{peb_size:x}', peb_size, min_io, [
            {'name': 'u-boot', 'vol_type': 'static', 'data': p_multi},
        ])
        prefix = bytearray(b'\xff' * ubi_off)
        prefix[:len(boot_hdr)] = boot_hdr
        if ubi_off == 0x100000:
            prefix[0x20000:0x20004] = b'ECNT'
        flash_m = bytes(prefix) + ubi_m
        # Earlier offsets must not falsely match 0x100000; target offset must succeed cleanly.
        if ubi_off == 0x100000:
            ret_40, _, _, _ = call_ubi(lib, flash_m, 0x40000)
            assert ret_40 == 1, ret_40
        else:
            ret_20, _, _, _ = call_ubi(lib, flash_m, 0x20000)
            assert ret_20 != 0, ret_20
        ret_ok, info_ok, data_ok, _ = call_ubi(lib, flash_m, ubi_off)
        assert ret_ok == 0 and info_ok.peb_size == peb_size and data_ok == p_multi
        checks += 1

    # 9. Wrap-around detection on a full 16 MiB chip: scanner must stop at 16 MiB!
    wrap_size = 16 * 1024 * 1024
    # Fill entire 16 MiB - 128 KiB with valid UBI EC headers (repeating the first EC header).
    ec_hdr = ubi[:0x20000]
    empty_ec = ec_hdr[:64] + b'\xff' * (0x20000 - 64)
    full_ubi = bytearray(boot_hdr)
    full_ubi += ubi
    while len(full_ubi) < wrap_size:
        full_ubi += empty_ec
    ret, info, data, _ = call_ubi(lib, bytes(full_ubi), 0x20000, max_scan=64 * 1024 * 1024,
                                  wrap_size=wrap_size)
    assert ret == 0 and data == p_orig
    assert info.scanned_pebs == (wrap_size - 0x20000) // 0x20000, info.scanned_pebs
    checks += 1

    # 9. Bounded decompression (gzip + LZMA known & unknown size) + real mkimage uImage check.
    raw = (rng.randbytes(4096) + b'U-Boot MIPS ') * 10
    gz = gzip.compress(raw, mtime=0)
    lz_unknown = lzma.compress(raw, format=lzma.FORMAT_ALONE,
                               filters=[{'id': lzma.FILTER_LZMA1, 'dict_size': 1 << 20}])
    lz_known = lz_unknown[:5] + struct.pack('<Q', len(raw)) + lz_unknown[13:]
    for alg, packed in [(1, gz), (2, lz_unknown), (2, lz_known)]:
        out_buf = ctypes.create_string_buffer(len(raw) + 4096)
        work_buf = ctypes.create_string_buffer(65536 + 8)
        work_addr = (ctypes.addressof(work_buf) + 7) & ~7
        out_len = ctypes.c_size_t(0)
        rc = lib.econet_decompress_bounded(
            alg, packed, len(packed), out_buf, len(raw) + 4096,
            ctypes.byref(out_len), ctypes.c_void_p(work_addr), 65536,
        )
        assert rc == 0 and out_len.value == len(raw) and out_buf.raw[:len(raw)] == raw
        # Buffer too small must fail cleanly.
        rc = lib.econet_decompress_bounded(
            alg, packed, len(packed), out_buf, len(raw) - 1,
            ctypes.byref(out_len), ctypes.c_void_p(work_addr), 65536,
        )
        assert rc != 0
        checks += 2

    if shutil.which('mkimage'):
        for comp, blob in [('none', raw), ('gzip', gz), ('lzma', lz_known)]:
            in_p = temp / f'uboot_{comp}.bin'
            out_p = temp / f'uboot_{comp}.uimage'
            in_p.write_bytes(blob)
            subprocess.run([
                'mkimage', '-A', 'mips', '-O', 'u-boot', '-T', 'firmware', '-C', comp,
                '-a', '0x81000000', '-e', '0x81000000', '-n', 'test',
                '-d', str(in_p), str(out_p),
            ], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            uimg = out_p.read_bytes()
            ubi_uimg = run_ubinize(temp, f'uimg_{comp}', 0x20000, 2048, [
                {'name': 'u-boot', 'vol_type': 'static', 'data': uimg},
            ])
            ret, info, data, _ = call_ubi(lib, boot_hdr + ubi_uimg, 0x20000)
            assert ret == 0 and data == uimg
            assert struct.unpack_from('>I', data, 0)[0] == 0x27051956
            if comp != 'none':
                alg = 1 if comp == 'gzip' else 2
                out_buf = ctypes.create_string_buffer(len(raw) + 4096)
                work_buf = ctypes.create_string_buffer(65536 + 8)
                work_addr = (ctypes.addressof(work_buf) + 7) & ~7
                out_len = ctypes.c_size_t(0)
                rc = lib.econet_decompress_bounded(
                    alg, data[64:], len(data) - 64, out_buf, len(raw) + 4096,
                    ctypes.byref(out_len), ctypes.c_void_p(work_addr), 65536,
                )
                assert rc == 0 and out_len.value == len(raw) and out_buf.raw[:len(raw)] == raw
            checks += 1

    return checks


def run_mips_qemu_tests(temp: Path) -> int:
    rng = random.Random(75122175)
    payload = rng.randbytes(0x21500)
    ubi = run_ubinize(temp, 'mips_qemu', 0x20000, 2048, [
        {'name': 'kernel', 'vol_type': 'dynamic', 'vol_size': 0x20000, 'data': b'K' * 1024},
        {'name': 'u-boot', 'vol_type': 'static', 'data': payload},
    ])
    flash = b'\x10\x00\x00\x0f' + b'\x00' * (0x20000 - 4) + ubi
    gz = gzip.compress(payload, mtime=0)

    def c_array(name: str, data: bytes) -> str:
        return f'static const unsigned char {name}[] = {{' + ','.join(map(str, data)) + '};\n'

    harness_src = '#include "ubi.h"\n#include "decompress.h"\n'
    harness_src += c_array('flash_img', flash)
    harness_src += c_array('expected', payload)
    harness_src += c_array('gz_data', gz)
    harness_src += '''
static unsigned char ubi_work[UBI_WORK_SIZE] __attribute__((aligned(8)));
static unsigned char dec_work[ECONET_DECOMP_WORK_SIZE] __attribute__((aligned(8)));
static unsigned char out_buf[sizeof(expected) + 4096];
static const char *const vol_names[] = { "u-boot", "uboot", "bootloader" };

static int mem_read(uint32_t offset, void *dst, size_t len, void *ctx)
{
    uint8_t *p = dst;
    (void)ctx;
    for (size_t i = 0; i < len; i++) {
        uint32_t addr = offset + (uint32_t)i;
        p[i] = addr < sizeof(flash_img) ? flash_img[addr] : 0xffU;
    }
    return 0;
}

static int run_tests(void)
{
    struct econet_ubi_info info;
    size_t dec_len = 0;
    int ret = econet_ubi_load_volume(mem_read, NULL, 0x20000U, 0x1000000U,
                                     vol_names, 3, out_buf, sizeof(out_buf),
                                     ubi_work, sizeof(ubi_work), &info);
    if (ret != ECONET_UBI_OK)
        return 1;
    if (info.peb_size != 0x20000U || info.data_size != sizeof(expected))
        return 2;
    for (size_t i = 0; i < sizeof(expected); i++) {
        if (out_buf[i] != expected[i])
            return 3;
    }
    if (econet_decompress_bounded(ECONET_COMP_GZIP, gz_data, sizeof(gz_data),
                                  out_buf, sizeof(out_buf), &dec_len,
                                  dec_work, sizeof(dec_work)))
        return 4;
    if (dec_len != sizeof(expected))
        return 5;
    for (size_t i = 0; i < sizeof(expected); i++) {
        if (out_buf[i] != expected[i])
            return 6;
    }
    return 0;
}

void __attribute__((noreturn)) _start(void)
{
    int result = run_tests();
    register unsigned int code __asm__("$4") = (unsigned int)result;
    register unsigned int syscall_nr __asm__("$2") = 4001;
    __asm__ volatile("syscall" : "+r"(syscall_nr) : "r"(code) : "memory");
    for (;;) {}
}
'''
    harness = temp / 'mips_ubi_harness.c'
    harness.write_text(harness_src)

    count = 0
    for endian, triple, emulation, qemu_env in [
        ('-EB', 'mips-linux-gnu', 'elf32btsmip', 'QEMU_MIPS'),
        ('-EL', 'mipsel-linux-gnu', 'elf32ltsmip', 'QEMU_MIPSEL'),
    ]:
        qemu = os.environ.get(qemu_env, 'qemu-' + triple.split('-')[0])
        if not shutil.which(qemu):
            continue
        objs = []
        strict_sources = [harness, ROOT / 'flash/ubi.c', ROOT / 'flash/decompress.c']
        vendor_sources = [
            ROOT / 'flash' / s for s in DECOMP_SOURCES if s != 'decompress.c'
        ]
        base_flags = [
            os.environ.get('CLANG', 'clang'), '--target=' + triple, endian,
            '-mabi=32', '-mips32r2', '-msoft-float', '-mno-abicalls',
            '-fno-pic', '-fno-pie', '-ffreestanding', '-fno-builtin',
            '-fno-stack-protector', '-Os', '-G0', '-std=gnu11',
            '-ffunction-sections', '-fdata-sections', *DECOMP_FLAGS,
        ]
        for i, src in enumerate(strict_sources):
            obj = temp / f'{triple}_s{i}.o'
            subprocess.run([*base_flags, '-Wall', '-Wextra', '-Werror',
                            '-c', str(src), '-o', str(obj)], check=True)
            objs.append(str(obj))
        for i, src in enumerate(vendor_sources):
            obj = temp / f'{triple}_v{i}.o'
            subprocess.run([*base_flags, '-c', str(src), '-o', str(obj)], check=True)
            objs.append(str(obj))
        elf = temp / f'{triple}_ubi.elf'
        subprocess.run([
            os.environ.get('LLD', 'ld.lld'), '-m', emulation, '-static',
            '--gc-sections', '-e', '_start', '-o', str(elf), *objs,
        ], check=True)
        subprocess.run([qemu, str(elf)], check=True, timeout=30)
        count += 1
    return count


def main():
    if not shutil.which('ubinize'):
        print('ubi: skipped (ubinize not found)')
        sys.exit(0)
    with tempfile.TemporaryDirectory() as directory:
        temp = Path(directory)
        lib = build_host_lib(temp)
        host_checks = run_host_tests(lib, temp)
        qemu_checks = run_mips_qemu_tests(temp)
        print(f'ubi: {host_checks} host checks and {qemu_checks} MIPS QEMU targets passed')


if __name__ == '__main__':
    main()
