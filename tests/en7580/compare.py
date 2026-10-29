#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Compare the pre-header C port with the current EN7580 calibration under QEMU."""
import os
from pathlib import Path
import shlex
import shutil
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
BASELINE = "535fa3d57767621d62fdbea13eb202cc213a2992"
SOURCES = ("dramc_pi_basic_api.c", "dramc_pi_calibration_api.c", "dramc_pi_main.c", "data.c")


def run(command, **kwargs):
    return subprocess.run(command, check=True, **kwargs)


def build(directory, baseline, llvm):
    directory.mkdir()
    for name in (*SOURCES, "en7580_ddr.h"):
        target = directory / name
        if baseline:
            target.write_bytes(subprocess.check_output(
                ["git", "show", f"{BASELINE}:dramc/en7580/{name}"], cwd=ROOT))
        else:
            shutil.copyfile(ROOT / "dramc/en7580" / name, target)
    for name in ("trace.c", "start.S"):
        target = directory / name
        if baseline:
            target.write_bytes(subprocess.check_output(
                ["git", "show", f"{BASELINE}:tests/en7580/{name}"], cwd=ROOT))
        else:
            shutil.copyfile(ROOT / "tests/en7580" / name, target)
    cross = os.environ.get("CROSS_COMPILE", "mips64-linux-gnu-")
    cc = ["clang", "--target=mipsel-linux-gnu"] if llvm else [cross + "gcc"]
    ld = "ld.lld" if llvm else cross + "ld"
    flags = ["-EL", "-mabi=32", "-mips32r2", "-msoft-float", "-mno-abicalls",
             "-fno-pic", "-fno-pie", "-ffreestanding", "-fno-builtin",
             "-fno-stack-protector", "-fno-strict-aliasing", "-G0", "-Oz" if llvm else "-Os", "-std=gnu11",
             "-I" + str(directory), "-I" + str(ROOT / "includes")]
    if llvm:
        flags += ["-Wno-parentheses", "-Wno-shift-op-parentheses"]
    objects = []
    for source in (*SOURCES, "trace.c", "start.S"):
        obj = directory / (Path(source).stem + ".o")
        run(cc + flags + ["-c", str(directory / source), "-o", str(obj)])
        objects.append(str(obj))
    executable = directory / "trace.elf"
    run([ld, "-EL", "-m", "elf32ltsmip", "-static", "-e", "_start",
         "-Ttext", "0x00401000", "-o", str(executable), *objects])
    return executable


def execute(qemu, executable, case, profile):
    result = run([*qemu, str(executable), str(case), str(profile)],
                 stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=30)
    if len(result.stdout) != 116:
        raise AssertionError(f"case {case}/{profile}: unexpected output size {len(result.stdout)}")
    assert struct.unpack_from("<i", result.stdout)[0] != -999, "model operation limit"
    return result.stdout


def main():
    llvm = os.environ.get("LLVM", "0") == "1"
    qemu = shlex.split(os.environ.get("QEMU_MIPSEL", "qemu-mipsel"))
    with tempfile.TemporaryDirectory(prefix="en7580-header-check-") as work:
        old = build(Path(work) / "before", True, llvm)
        new = build(Path(work) / "after", False, llvm)
        count = 0
        for case in range(24):
            for profile in range(32 if case == 23 else 16):
                before = execute(qemu, old, case, profile)
                after = execute(qemu, new, case, profile)
                assert before == after, (case, profile, before.hex(), after.hex())
                count += 1
        for case in (24, 25):
            data = execute(qemu, new, case, 0)
            assert struct.unpack_from("<I", data)[0] == 0
        print(f"EN7580: {count} pre/post-header C traces and contexts matched; "
              "rank bounds and full-width field checks passed")


if __name__ == "__main__":
    main()
