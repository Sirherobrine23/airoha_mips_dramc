#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Generate editor-only compile commands without building firmware."""
import argparse
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--soc", choices=("en751221", "en7528"), default="en7528",
                        help="profile for shared sources (default: en7528)")
    args = parser.parse_args()
    output = ROOT / "out/clangd"
    generated = output / "includes"
    generated.mkdir(parents=True, exist_ok=True)
    (generated / "tmp_version.h").write_text(
        '/* Editor-only metadata; firmware builds generate their own header. */\n'
        '#define PLAIN_VERSION "clangd"\n'
        '#define CC_VERSION_STRING "clang"\n'
        '#define LD_VERSION_STRING "ld.lld"\n')

    entries = []
    for directory in ("dramc", "flash", "chainloader", "bootext", "includes", "tools", "tests"):
        for source in sorted((ROOT / directory).rglob("*")):
            if source.suffix not in (".c", ".h") or source.name == "tmp_version.h":
                continue
            relative = source.relative_to(ROOT)
            # Preserved binary/decompiler references are not build sources.
            if "blobs" in relative.parts:
                continue
            host = relative.parts[0] == "tools" or relative == Path("tests/inspect.c")
            soc = args.soc
            for candidate in ("en751221", "en751627", "en7528", "en7580"):
                if candidate in relative.parts:
                    soc = candidate
                    break
            flags = ["clang", "-std=gnu11", "-Wall"]
            if not host:
                big = soc in ("en751221", "en751627")
                flags += ["--target=" + ("mips-linux-gnu" if big else "mipsel-linux-gnu"),
                          "-EB" if big else "-EL", "-mabi=32", "-mips32r2", "-msoft-float",
                          "-mno-abicalls", "-fno-pic", "-fno-pie", "-ffreestanding",
                          "-fno-builtin", "-fno-stack-protector", "-G0"]
            for include in (generated, ROOT / "includes", ROOT / "dramc" / soc,
                            ROOT / "includes/dramc", ROOT / "flash", ROOT / "chainloader"):
                flags += ["-I" + str(include)]
            if relative.parts[0] == "flash":
                flags += ["-I" + str(ROOT / "flash/decompress"),
                          "-DZ_SOLO", "-DNOBYFOUR", "-D_LZMA_SIZE_OPT"]
            if relative.parts[0] == "chainloader" and soc == "en7528":
                flags += ["-DCHAINLOADER_EN7528"]
            if relative.parts[:2] == ("tests", "en7512"):
                flags += ["-include", str(ROOT / "tests/en7512/mmio.h")]
                if soc == "en751221":
                    flags += ["-DPROFILE_EN751221"]
            flags += ["-x", "c", "-c", str(source)]
            entries.append({"directory": str(ROOT), "file": str(source), "arguments": flags})

    database = output / "compile_commands.json"
    database.write_text(json.dumps(entries, indent=2) + "\n")
    print(f"clangd: {len(entries)} entries; shared profile {args.soc}; {database}")


if __name__ == "__main__":
    main()
