#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Compare shared EN7512 calibration against the pre-merge MIPS sources."""
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
BASELINE = "6896ba27a1c111d550adc785fcfad76cd7894b13"
PREFIX = os.environ.get("CROSS_COMPILE", "mips64-linux-gnu-")
LLVM = os.environ.get("LLVM", "0") == "1"


def run(args):
    return subprocess.run(args, check=True, stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE, timeout=60).stdout


def expected_gating(coarse, fine, profile, first, last):
    if profile == 0:
        return False
    if profile == 1:
        return True
    if profile == 2:
        return coarse == first and 31 <= fine <= 33
    if profile == 3:
        return coarse == last and 62 <= fine <= 66
    if profile == 4:
        return coarse in (first, last) and 10 <= fine < 20
    if profile == 5:
        return coarse == first + 1 and (3 <= fine < 7 or 40 <= fine < 44)
    if profile == 6:
        return coarse == last and fine == 95
    if profile == 7:
        return coarse == last and fine == 0
    if profile == 8:
        return (coarse * 97 + fine * 13) % 17 < 8
    return coarse == first + 2 and 20 <= fine < 40


def check_result(data, endian, mode, profile, first, last):
    values = struct.unpack(endian + "12I", data[:48])
    result, first_result, index, tap, coarse, fine = values[:6]
    if mode == 0:
        selected = {0: 0, 1: 1, 2: 1, 3: 15, 4: 0, 5: 2, 6: 8, 7: 9}[profile]
        assert (index, tap) == (selected, selected), (profile, values)
        assert result == (0 if selected else 0xffffffff), (profile, values)
    else:
        rows = [(c, [f for f in range(96)
                     if expected_gating(c, f, profile, first, last)])
                for c in range(first, last + 1)]
        best, passes = max(rows, key=lambda row: (len(row[1]), row[0]))
        if passes:
            assert (coarse, fine) == (best, passes[0] + len(passes) // 2), values
        else:
            assert (coarse, fine) == (0, 0), values
        assert first_result == (0 if passes else 0xffffffff), values
        assert result == (0xffffffff if profile == 9 else first_result), values


def main():
    compiler = "clang" if LLVM else PREFIX + "gcc"
    linker = "ld.lld" if LLVM else PREFIX + "ld"
    for tool in (compiler, linker, "qemu-mips", "qemu-mipsel", "git"):
        if not shutil.which(tool):
            raise SystemExit(f"required tool not found: {tool}")
    cases = 0
    with tempfile.TemporaryDirectory(prefix="en7512-calibration-") as temporary:
        work = Path(temporary)
        for soc, endian, flag, emulation, qemu, first, last in (
            ("en751221", ">", "-EB", "elf32btsmip", "qemu-mips", 0, 31),
            ("en7528", "<", "-EL", "elf32ltsmip", "qemu-mipsel", 6, 20),
        ):
            executables = []
            for baseline in (True, False):
                target = work / (soc + ("-baseline" if baseline else "-shared"))
                target.mkdir()
                sources = []
                for stem in ("dramc_dle_cal", "dramc_dqs_gw_cal"):
                    if baseline:
                        source = target / (stem + ".c")
                        source.write_bytes(run([
                            "git", "-C", str(ROOT), "show",
                            f"{BASELINE}:dramc/{soc}/{stem}.c"]))
                    else:
                        source = ROOT / "includes/dramc" / (stem + ".c")
                    sources.append(source)
                sources += [ROOT / "tests/en7512/trace.c", ROOT / "tests/en7512/start.S"]
                if not baseline:
                    sources.insert(0, ROOT / "dramc" / soc / "dramc_test.c")
                flags = [flag, "-mabi=32", "-mips32r2", "-msoft-float", "-mno-abicalls",
                         "-fno-pic", "-fno-pie", "-ffreestanding", "-fno-builtin",
                         "-fno-stack-protector", "-Os", "-G0", "-std=gnu11",
                         "-I" + str(ROOT / "dramc" / soc),
                         "-I" + str(ROOT / "includes"),
                         "-I" + str(ROOT / "includes/dramc"),
                         "-include", str(ROOT / "tests/en7512/mmio.h")]
                if LLVM:
                    flags += ["--target=" + ("mips-linux-gnu" if endian == ">" else "mipsel-linux-gnu")]
                if soc == "en751221":
                    flags += ["-DPROFILE_EN751221"]
                if baseline:
                    flags += ["-DUSE_BASELINE"]
                objects = []
                for source in sources:
                    obj = target / (source.stem + ".o")
                    compile_flags = flags.copy()
                    if source.suffix == ".S":
                        position = compile_flags.index("-include")
                        del compile_flags[position:position + 2]
                    run([compiler, *compile_flags, "-c", str(source), "-o", str(obj)])
                    objects.append(str(obj))
                executable = target / "calibration.elf"
                run([linker, flag, "-m", emulation, "-static", "-e", "_start",
                     "-o", str(executable), *objects])
                executables.append(executable)
            for mode, count in ((0, 8), (1, 10)):
                for profile in range(count):
                    outputs = [run([qemu, str(exe), str(mode), str(profile)])
                               for exe in executables]
                    if outputs[0] != outputs[1]:
                        raise AssertionError(f"trace mismatch: {soc} mode={mode} profile={profile}")
                    check_result(outputs[1], endian, mode, profile, first, last)
                    cases += 1
            print(f"{soc}: 18 matching traces, selected taps and failure handling passed")
    print(f"EN7512 calibration: {cases} differential cases passed")


if __name__ == "__main__":
    try:
        main()
    except subprocess.CalledProcessError as error:
        raise SystemExit(error.stderr.decode(errors="replace")) from error
