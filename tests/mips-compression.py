#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
"""Run freestanding decoder vectors under QEMU, in both MIPS byte orders.

Environment: CLANG, LLD, QEMU_MIPS, QEMU_MIPSEL (optional absolute tool paths).
"""
import gzip
import lzma
import os
from pathlib import Path
import random
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ['decompress.c', 'decompress-memory.c', 'decompress/LzmaDec.c',
           'decompress/inflate.c', 'decompress/inffast.c', 'decompress/inftrees.c',
           'decompress/adler32.c', 'decompress/crc32.c', 'decompress/zutil.c']


def array(name, data):
    return f'static const unsigned char {name}[] = {{' + ','.join(map(str, data)) + '};\n'


def main():
    raw = random.Random(751221).randbytes(4000) * 12 + b'MIPS' * 10000
    gz = gzip.compress(raw, mtime=0)
    lz = lzma.compress(raw, format=lzma.FORMAT_ALONE,
                       filters=[{'id': lzma.FILTER_LZMA1, 'dict_size': 1 << 20}])
    source = '#include "decompress.h"\n'
    source += array('expected', raw) + array('gz', gz) + array('lz', lz)
    source += '''
static unsigned char work[65536] __attribute__((aligned(8)));
static unsigned char output[sizeof(expected) + 32];
static int run(unsigned int algorithm, const unsigned char *src, unsigned int size)
{
    unsigned int i;
    for (i = 0; i < sizeof(output); i++) output[i] = 0x55;
    if (econet_decompress(algorithm, src, size, output + 16,
                          sizeof(expected), work, sizeof(work))) return 1;
    for (i = 0; i < sizeof(expected); i++)
        if (output[i + 16] != expected[i]) return 2;
    for (i = 0; i < 16; i++)
        if (output[i] != 0x55 || output[sizeof(expected) + 16 + i] != 0x55) return 3;
    if (!econet_decompress(algorithm, src, size - 1, output + 16,
                          sizeof(expected), work, sizeof(work))) return 4;
    if (!econet_decompress(algorithm, src, size, output + 16,
                          sizeof(expected) - 1, work, sizeof(work))) return 5;
    return 0;
}
void __attribute__((noreturn)) _start(void)
{
    int result = run(1, gz, sizeof(gz));
    if (!result) result = run(2, lz, sizeof(lz));
    register unsigned int code __asm__("$4") = result;
    register unsigned int syscall_nr __asm__("$2") = 4001;
    __asm__ volatile("syscall" : "+r"(syscall_nr) : "r"(code) : "memory");
    for (;;) {}
}
'''
    with tempfile.TemporaryDirectory() as directory:
        temp = Path(directory)
        harness = temp / 'vectors.c'
        harness.write_text(source)
        for endian, triple, emulation, qemu in [
                ('-EB', 'mips-linux-gnu', 'elf32btsmip', 'QEMU_MIPS'),
                ('-EL', 'mipsel-linux-gnu', 'elf32ltsmip', 'QEMU_MIPSEL')]:
            objects = []
            for i, src in enumerate([harness, *(ROOT / 'flash' / s for s in SOURCES)]):
                obj = temp / f'{i}.o'
                args = [os.environ.get('CLANG', 'clang'), '--target=' + triple, endian,
                        '-mabi=32', '-mips32r2', '-msoft-float', '-mno-abicalls',
                        '-fno-pic', '-fno-pie', '-ffreestanding', '-fno-builtin',
                        '-fno-stack-protector', '-Os', '-G0', '-DZ_SOLO', '-DNOBYFOUR',
                        '-D_LZMA_SIZE_OPT', '-ffunction-sections', '-fdata-sections',
                        '-I' + str(ROOT / 'flash/decompress'), '-I' + str(ROOT / 'flash'),
                        '-c', str(src), '-o', str(obj)]
                subprocess.run(args, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                objects.append(str(obj))
            elf = temp / 'vectors.elf'
            subprocess.run([os.environ.get('LLD', 'ld.lld'), '-m', emulation, '-static',
                            '--gc-sections', '-e', '_start', '-o', str(elf), *objects], check=True)
            subprocess.run([os.environ.get(qemu, 'qemu-' + triple.split('-')[0]), str(elf)],
                           check=True, timeout=30)
            print(f'{triple}: gzip/LZMA decode, bounds and truncation checks passed')


if __name__ == '__main__':
    main()
