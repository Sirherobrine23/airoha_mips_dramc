#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
"""Exercise the actual loader decoders and the host image packer."""
import ctypes
import gzip
import lzma
import os
from pathlib import Path
import random
import struct
import subprocess
import sys
import tempfile
import zlib

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ['decompress.c', 'decompress/LzmaDec.c', 'decompress/inflate.c',
           'decompress/inffast.c', 'decompress/inftrees.c', 'decompress/adler32.c',
           'decompress/crc32.c', 'decompress/zutil.c']
FLAGS = ['-DZ_SOLO', '-DNOBYFOUR', '-D_LZMA_SIZE_OPT',
         '-I' + str(ROOT / 'flash/decompress'), '-I' + str(ROOT / 'flash')]


def main():
    tool = Path(sys.argv[1]).resolve()
    count = 0
    with tempfile.TemporaryDirectory() as directory:
        temp = Path(directory)
        libpath = temp / 'decoder.so'
        subprocess.run([os.environ.get('HOSTCC', 'cc'), '-shared', '-fPIC', '-O2',
                        *FLAGS, *(str(ROOT / 'flash' / s) for s in SOURCES),
                        '-o', str(libpath)], check=True)
        library = ctypes.CDLL(str(libpath))
        decode = library.econet_decompress
        decode.argtypes = [ctypes.c_uint32, ctypes.c_void_p, ctypes.c_size_t,
                           ctypes.c_void_p, ctypes.c_size_t, ctypes.c_void_p,
                           ctypes.c_size_t]
        decode.restype = ctypes.c_int

        def check(algorithm, packed, raw, good=True, output_size=None, work_size=65536):
            nonlocal count
            size = len(raw) if output_size is None else output_size
            src = ctypes.create_string_buffer(packed)
            # Canaries bracket the destination, including failure paths.
            storage = ctypes.create_string_buffer(b'X' * (size + 32), size + 32)
            work = ctypes.create_string_buffer(65536)
            result = decode(algorithm, src, len(packed),
                            ctypes.addressof(storage) + 16, size, work, work_size)
            assert (result == 0) == good, (algorithm, len(packed), size, result, good)
            assert storage.raw[:16] == b'X' * 16
            assert storage.raw[16 + size:] == b'X' * 16
            if good:
                assert storage.raw[16:16 + size] == raw
            count += 1

        rng = random.Random(7528)
        datasets = [b'A', bytes(range(256)), b'U-Boot\0' * 70000,
                    rng.randbytes(150000), b'\0' * 1100000,
                    (rng.randbytes(32768) + b'MIPS') * 6]
        for raw in datasets:
            gz = gzip.compress(raw, mtime=0)
            lz = lzma.compress(raw, format=lzma.FORMAT_ALONE,
                               filters=[{'id': lzma.FILTER_LZMA1, 'dict_size': 1 << 20}])
            for alg, packed in [(1, gz), (2, lz)]:
                check(alg, packed, raw)
                for cut in [1, 5, len(packed) - 1]:
                    check(alg, packed[:cut], raw, False)
                check(alg, packed + b'garbage', raw, False)
                check(alg, packed, raw, False, output_size=len(raw) + 1)
                if len(raw) > 1:
                    check(alg, packed, raw, False, output_size=len(raw) - 1)
                check(alg, packed, raw, False, work_size=8)
            # gzip trailer corruption and concatenation are rejected.
            bad = bytearray(gz)
            bad[-8] ^= 1
            check(1, bytes(bad), raw, False)
            check(1, gz + gz, raw + raw, False)
            # Known LZMA output size is supported; mismatched size is rejected.
            known = lz[:5] + struct.pack('<Q', len(raw)) + lz[13:]
            check(2, known, raw)
            bad = lz[:5] + struct.pack('<Q', len(raw) + 1) + lz[13:]
            check(2, bad, raw, False)
            bad = bytes([255]) + lz[1:]
            check(2, bad, raw, False)
            bad = lz[:1] + struct.pack('<I', 1 << 25) + lz[5:]
            check(2, bad, raw, False)
            check(3, lz, raw, False)
        # DEFLATE stored, fixed-Huffman and dynamic-Huffman blocks.
        raw = datasets[2]
        for level, strategy in [(0, zlib.Z_DEFAULT_STRATEGY), (6, zlib.Z_FIXED),
                                (9, zlib.Z_DEFAULT_STRATEGY)]:
            compressor = zlib.compressobj(level, zlib.DEFLATED, 31, 8, strategy)
            check(1, compressor.compress(raw) + compressor.flush(), raw)
        # Gzip optional FNAME/FCOMMENT/FHCRC fields.
        raw = datasets[2]
        gz = gzip.compress(raw, mtime=0)
        header = gz[:3] + bytes([0x1a]) + gz[4:10] + b'u-boot.bin\0comment\0'
        header += struct.pack('<H', zlib.crc32(header) & 0xffff)
        check(1, header + gz[10:], raw)
        bad = bytearray(header)
        bad[-1] ^= 1
        check(1, bytes(bad) + gz[10:], raw, False)
        # Random stream mutations must never escape the caller's buffers.
        for alg, packed in [(1, gz), (2, lzma.compress(raw, format=lzma.FORMAT_ALONE))]:
            for _ in range(100):
                bad = bytearray(packed)
                bad[rng.randrange(len(bad))] ^= 1 << rng.randrange(8)
                src = ctypes.create_string_buffer(bytes(bad))
                storage = ctypes.create_string_buffer(b'X' * (len(raw) + 32), len(raw) + 32)
                work = ctypes.create_string_buffer(65536)
                decode(alg, src, len(bad), ctypes.addressof(storage) + 16,
                       len(raw), work, 65536)
                assert storage.raw[:16] == storage.raw[-16:] == b'X' * 16
                count += 1
        # Exercise packaging with both native-endian BootROM headers.
        for endian in ['<', '>']:
            base = bytearray(0x20000)
            base[12:16] = b'6578'
            struct.pack_into(endian + 'II', base, 24, 0x20000, 0x20000)
            struct.pack_into(endian + 'I', base, 0x1fffc,
                             zlib.crc32(base[:0x1fffc]) ^ 0xffffffff)
            boot = temp / 'base.bin'
            boot.write_bytes(base)
            payload = temp / 'payload.bin'
            payload.write_bytes(datasets[4])
            for name, alg in [('gzip', 1), ('lzma', 2)]:
                image = temp / (name + '.bin')
                args = [str(tool), 'tcboot', '--boot', str(boot), '--payload', str(payload),
                        '--load', '0x81000000', '--entry', '0x81000040',
                        '--compression', name, '--output', str(image)]
                subprocess.run(args, check=True)
                data = image.read_bytes()
                h = data[0x20000:0x20030]
                fields = struct.unpack('>12I', h)
                assert fields[:3] == (0x45434e54, 2, 48)
                assert fields[8:] == (alg, len(datasets[4]), zlib.crc32(datasets[4]), 0)
                assert zlib.crc32(h[:28] + b'\0' * 4 + h[32:]) == fields[7]
                stored = data[0x20030:]
                assert zlib.crc32(stored) == fields[6]
                assert len(stored) == fields[3]
                assert struct.unpack_from(endian + 'II', data, 24) == (0x20000, len(data))
                assert struct.unpack_from(endian + 'I', data, 0x1fffc)[0] == (
                    zlib.crc32(data[:0x1fffc]) ^ 0xffffffff)
                check(alg, stored, datasets[4])
                # Deterministic bytes, despite subprocess-based compression.
                subprocess.run(args, check=True)
                assert image.read_bytes() == data
            payload.write_bytes(datasets[1])
            image = temp / 'none.bin'
            subprocess.run([str(tool), 'tcboot', '--boot', str(boot), '--payload', str(payload),
                            '--load', '0x81000000', '--entry', '0x81000000',
                            '--output', str(image)], check=True)
            data = image.read_bytes()
            assert struct.unpack_from('>II', data, 0x20000) == (0x45434e54, 1)
            assert data[0x20020:] == datasets[1]
            count += 1
        # EN751627's late-loader layout and manufacturing-data exclusions.
        stages = bytearray(0x18000)
        stages[0x10000:0x18000] = b'L' * 0x8000
        (temp / 'stages.bin').write_bytes(stages)
        symbols = {'move_data': (0x280, 0x700), 'boot2': (0x700, 0x1600),
                   'spram': (0x1600, 0xe650), 'lzma': (0x10000, 0x18000)}
        def write_symbols():
            (temp / 'stages.nm').write_text(''.join(
                f'{0xbfc00000 + value:08x} T __{name}_{suffix}\n'
                for name, interval in symbols.items()
                for suffix, value in zip(['start', 'end'], interval)))
        write_symbols()
        (temp / 'minfo.bin').write_bytes(b'M' * 256)
        args = [str(tool), 'tcboot-base', '--soc', 'en751627',
                '--stages', str(temp / 'stages.bin'), '--symbols', str(temp / 'stages.nm'),
                '--minfo', str(temp / 'minfo.bin'), '--output', str(temp / 'late.bin')]
        subprocess.run(args, check=True, stdout=subprocess.PIPE)
        base = (temp / 'late.bin').read_bytes()
        assert len(base) == 0x20000
        assert struct.unpack_from('>II', base, 16) == symbols['lzma']
        assert struct.unpack_from('>II', base, 0x58) == symbols['spram']
        assert base[0xff00:0xffb0] == b'M' * 176
        assert base[0x10000:0x18000] == stages[0x10000:0x18000]
        count += 1
        for interval in [(0xfe00, 0x18000), (0x8000, 0x10000), (0xe000, 0xf000)]:
            symbols['lzma'] = interval
            write_symbols()
            result = subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            assert result.returncode != 0
            count += 1
        print(f'compression: {count} decoder/packaging checks passed')


if __name__ == '__main__':
    main()
