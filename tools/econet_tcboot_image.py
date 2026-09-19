#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
"""Finalize EcoNet/Airoha MIPS unsigned cold-boot images.

Binman fixes the placement of TPL, DDR calibration, SPL and U-Boot proper.
This helper fills the TCBoot fields which depend on the final SPL image size
and writes the vendor-style complemented CRC32 words.
"""

import argparse
import struct
import zlib
from pathlib import Path
from typing import Optional

IH_MAGIC = 0x27051956
IH_HEADER_SIZE = 0x40
BOOT_IMAGE_SIZE = 0x100000
TCBOOT_MAGIC_OFFSET = 0x0C
TCBOOT_MAGIC = b"6578"
CRC1_OFFSET = 0x1FFFC
CRC2_OFFSET = 0x3FDFC

SOCS = {
    "en751221": {
        "endian": "big",
        "spl_offset": 0x10000,
        "spl_limit": CRC1_OFFSET,
        "ddr_offset": 0x8000,
        "ddr_size": 0x4F70,
        "tcboot_len": 0,
        "spl_role": "bootram",
        "crc_offsets": (CRC1_OFFSET,),
        "minfo_offset": 0xFF00,
        "minfo_size": 0x100,
        "extended_header": False,
    },
    "en751627": {
        "endian": "big",
        "spl_offset": 0x10000,
        "spl_limit": CRC1_OFFSET,
        "ddr_offset": 0x2000,
        "ddr_size": 0xD050,
        "tcboot_len": 0x40000,
        "spl_role": "lzma",
        "crc_offsets": (CRC1_OFFSET, CRC2_OFFSET),
        "minfo_offset": 0x3FE00,
        "minfo_size": 0x200,
        "extended_header": True,
    },
    "en7528": {
        "endian": "little",
        "spl_offset": 0x10000,
        "spl_limit": 0x1FFF8,
        "ddr_offset": 0x4000,
        "ddr_size": 0xB000,
        "tcboot_len": 0x40000,
        "spl_role": "lzma",
        "crc_offsets": (CRC1_OFFSET, CRC2_OFFSET),
        "minfo_offset": 0x3FE00,
        "minfo_size": 0x200,
        "extended_header": True,
    },
    "en7580": {
        "endian": "little",
        "spl_offset": 0x10000,
        "spl_limit": 0x1FFF8,
        "ddr_offset": 0x4000,
        "ddr_size": 0xB000,
        "tcboot_len": 0x40000,
        "spl_role": "lzma",
        "crc_offsets": (CRC1_OFFSET, CRC2_OFFSET),
        "minfo_offset": 0x3FE00,
        "minfo_size": 0x200,
        "extended_header": True,
    },
}


def _put_u32(image: bytearray, offset: int, value: int, endian: str) -> None:
    image[offset:offset + 4] = value.to_bytes(4, endian)


def _get_spl_end(image: bytearray, cfg: dict) -> int:
    offset = cfg["spl_offset"]
    header = image[offset:offset + IH_HEADER_SIZE]
    if len(header) != IH_HEADER_SIZE:
        raise ValueError("SPL header lies outside the image")

    magic, = struct.unpack_from(">I", header, 0)
    if magic != IH_MAGIC:
        raise ValueError(
            f"SPL legacy image magic at {offset:#x} is {magic:#x}, "
            f"expected {IH_MAGIC:#x}"
        )

    payload_size, = struct.unpack_from(">I", header, 12)
    if not payload_size:
        raise ValueError("SPL legacy image has an empty payload")

    end = offset + IH_HEADER_SIZE + payload_size
    if end > cfg["spl_limit"]:
        raise ValueError(
            f"SPL ends at {end:#x}, beyond TCBoot limit "
            f"{cfg['spl_limit']:#x}"
        )
    return end


def _tcboot_crc(image: bytearray, end: int) -> int:
    return (~zlib.crc32(image[:end])) & 0xFFFFFFFF


def copy_manufacturing(image: bytearray, stock: bytes, cfg: dict) -> None:
    start = cfg["minfo_offset"]
    end = start + cfg["minfo_size"]
    if len(stock) < end:
        raise ValueError(
            f"stock bootloader is too small for manufacturing data: "
            f"{len(stock):#x} < {end:#x}"
        )
    image[start:end] = stock[start:end]


def finalize_image(image: bytearray, soc: str, stock: Optional[bytes] = None) -> None:
    try:
        cfg = SOCS[soc]
    except KeyError as exc:
        raise ValueError(f"unsupported SoC {soc!r}") from exc

    if len(image) != BOOT_IMAGE_SIZE:
        raise ValueError(
            f"combined image must be exactly {BOOT_IMAGE_SIZE:#x} bytes, "
            f"got {len(image):#x}"
        )
    if image[TCBOOT_MAGIC_OFFSET:TCBOOT_MAGIC_OFFSET + 4] != TCBOOT_MAGIC:
        raise ValueError("TCBoot magic '6578' is missing at offset 0x0c")

    if stock is not None:
        copy_manufacturing(image, stock, cfg)

    spl_end = _get_spl_end(image, cfg)
    endian = cfg["endian"]

    # tcboot_len and the stage interval fields are native-endian words.
    _put_u32(image, 0x08, cfg["tcboot_len"], endian)
    if cfg["spl_role"] == "bootram":
        _put_u32(image, 0x10, 0, endian)
        _put_u32(image, 0x14, 0, endian)
        _put_u32(image, 0x18, cfg["spl_offset"], endian)
        _put_u32(image, 0x1C, spl_end, endian)
    else:
        _put_u32(image, 0x10, cfg["spl_offset"], endian)
        _put_u32(image, 0x14, spl_end, endian)
        _put_u32(image, 0x18, 0, endian)
        _put_u32(image, 0x1C, 0, endian)

    if cfg["extended_header"]:
        _put_u32(image, 0x58, cfg["ddr_offset"], endian)
        _put_u32(image, 0x5C,
                 cfg["ddr_offset"] + cfg["ddr_size"], endian)

    # The vendor trx stage stores a complemented CRC32 in native endian.
    # CRC2 includes the already-finalized CRC1, matching stock 256 KiB images.
    for offset in cfg["crc_offsets"]:
        _put_u32(image, offset, _tcboot_crc(image, offset), endian)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--soc", required=True, choices=sorted(SOCS))
    parser.add_argument("--image", required=True, type=Path,
                        help="1 MiB combined Binman image")
    parser.add_argument("--output", type=Path,
                        help="output path; default overwrites --image")
    parser.add_argument("--stock", type=Path,
                        help="optional stock bootloader to copy MI/RFU data from")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    image = bytearray(args.image.read_bytes())
    stock = args.stock.read_bytes() if args.stock else None

    try:
        finalize_image(image, args.soc, stock)
    except ValueError as exc:
        raise SystemExit(f"econet tcboot: {exc}") from exc

    output = args.output or args.image
    output.write_bytes(image)


if __name__ == "__main__":
    main()
