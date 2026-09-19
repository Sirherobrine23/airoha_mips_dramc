#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
"""Merge EN751221 manufacturing information into a finalized U-Boot image."""

import argparse
from pathlib import Path

from econet_tcboot_image import finalize_image


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "copy the 256-byte 128 KiB TCBoot manufacturing block at 0xff00 "
            "from a stock EN751221 bootloader and refresh the TCBoot CRC"
        )
    )
    parser.add_argument("--image", required=True, type=Path,
                        help="u-boot-en7512.bin input")
    parser.add_argument("--stock", required=True, type=Path,
                        help="stock TCBoot image containing manufacturing data")
    parser.add_argument("--output", required=True, type=Path,
                        help="output image")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    image = bytearray(args.image.read_bytes())
    stock = args.stock.read_bytes()

    try:
        finalize_image(image, "en751221", stock)
    except ValueError as exc:
        raise SystemExit(f"en7512 image: {exc}") from exc

    args.output.write_bytes(image)


if __name__ == "__main__":
    main()
