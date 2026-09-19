#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
"""Host-side format checks: python3 -m unittest discover -s tools -p test_econet_flash_image.py."""
import struct
import unittest
import zlib
from econet_flash_image import pack, CRC, PAYLOAD, LIMIT


class FlashImageTest(unittest.TestCase):
    def setUp(self):
        self.stages = bytearray(0x9000)
        self.stages[:8] = b'\x0b\xf0\x00\x18\0\0\0\0'
        self.symbols = {}
        for name, a, b in [('move_data', 0x280, 0x768),
                           ('boot2', 0x768, 0x1660),
                           ('lzma', 0x1660, 0x2000),
                           ('spram', 0x2000, 0x9000)]:
            self.symbols[f'__{name}_start'] = 0xbfc00000 + a
            self.symbols[f'__{name}_end'] = 0xbfc00000 + b
        self.data = b'U-Boot payload\0' * 17
        self.header = bytearray(64)
        struct.pack_into('>7I', self.header, 0, 0x27051956, 0, 0, len(self.data),
                         0x81000000, 0x81000000, zlib.crc32(self.data))
        self.header[28:32] = bytes((17, 5, 5, 0))
        self.update_header_crc()

    def update_header_crc(self):
        self.header[4:8] = bytes(4)
        struct.pack_into('>I', self.header, 4, zlib.crc32(self.header))

    def build(self, soc='en751627', minfo=None):
        return pack(self.stages, self.symbols, self.header + self.data,
                    soc, 0x81000000, minfo)

    def test_both_endian_layouts(self):
        for soc, endian, marker in [('en751627', 'big', b'PAGE'),
                                     ('en7528', 'little', b'EGAP')]:
            with self.subTest(soc=soc):
                image = self.build(soc)
                self.assertEqual(len(image), LIMIT)
                self.assertEqual(image[:8], self.stages[:8])
                self.assertEqual(image[PAYLOAD:PAYLOAD+64+len(self.data)], self.header+self.data)
                self.assertEqual(int.from_bytes(image[8:12], endian), PAYLOAD)
                self.assertEqual(image[0xffb0:0xffb4], marker)
                self.assertEqual(int.from_bytes(image[CRC:CRC+4], endian),
                                 zlib.crc32(image[:CRC]) ^ 0xffffffff)
                self.assertEqual(image[0x1660:0x2000], self.stages[0x1660:0x2000])
                if soc == 'en7528':
                    self.assertEqual(image[0x1fff8:CRC], marker)
                else:
                    self.assertEqual(image[0x2000:0x2008], struct.pack('>II', 0x0be8c0a0, 0))

    def test_bad_data_crc(self):
        self.data = bytes([self.data[0] ^ 1]) + self.data[1:]
        with self.assertRaisesRegex(ValueError, 'payload size/CRC'):
            self.build()

    def test_bad_header_crc(self):
        self.header[4] ^= 1
        with self.assertRaisesRegex(ValueError, 'header/CRC'):
            self.build()

    def test_compressed_payload_rejected(self):
        self.header[31] = 3
        self.update_header_crc()
        with self.assertRaisesRegex(ValueError, 'uncompressed'):
            self.build()

    def test_stage_overlap_rejected(self):
        self.symbols['__move_data_end'] = 0xbfc00800
        with self.assertRaisesRegex(ValueError, 'first-page'):
            self.build()

    def test_manufacturing_preserved_except_required_marker(self):
        image = self.build(minfo=b'\xa5' * 256)
        self.assertEqual(image[0xff00:0xffb0], b'\xa5' * 176)
        self.assertEqual(image[0xffb4:0x10000], b'\xa5' * 76)
        with self.assertRaisesRegex(ValueError, '256 bytes'):
            self.build(minfo=b'')

    def test_size_limits(self):
        self.stages = bytes(0xff00)
        with self.assertRaisesRegex(ValueError, 'mi.conf'):
            self.build()
        self.stages = bytes(0x9000)
        self.data = bytes(LIMIT - PAYLOAD)
        with self.assertRaisesRegex(ValueError, '896 KiB'):
            self.build()


if __name__ == '__main__':
    unittest.main()
