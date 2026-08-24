#!/usr/bin/env python3
"""Fail closed when the full zlOS landing gate cannot run every boot route."""

from __future__ import annotations

import argparse
import shutil
from pathlib import Path


REQUIRED_COMMANDS = (
    "gcc",
    "ld",
    "objcopy",
    "nasm",
    "grub-mkrescue",
    "xorriso",
    "sgdisk",
    "mkfs.vfat",
    "mmd",
    "mcopy",
    "mdir",
    "qemu-system-i386",
    "qemu-system-x86_64",
)

REQUIRED_FILES = (
    Path("/usr/share/OVMF/OVMF_CODE_4M.fd"),
    Path("/usr/share/OVMF/OVMF_VARS_4M.fd"),
)


def missing_prereqs(commands: dict[str, bool], files: dict[str, bool]) -> list[str]:
    missing = [f"command:{name}" for name, present in commands.items() if not present]
    missing.extend(f"file:{name}" for name, present in files.items() if not present)
    return missing


def live_state() -> tuple[dict[str, bool], dict[str, bool]]:
    commands = {name: shutil.which(name) is not None for name in REQUIRED_COMMANDS}
    files = {str(path): path.is_file() for path in REQUIRED_FILES}
    return commands, files


def selftest() -> None:
    commands = {name: True for name in REQUIRED_COMMANDS}
    files = {str(path): True for path in REQUIRED_FILES}
    assert not missing_prereqs(commands, files)

    commands["qemu-system-x86_64"] = False
    assert missing_prereqs(commands, files) == ["command:qemu-system-x86_64"]
    commands["qemu-system-x86_64"] = True

    ovmf = str(REQUIRED_FILES[0])
    files[ovmf] = False
    assert missing_prereqs(commands, files) == [f"file:{ovmf}"]
    print("boot-prereqs selftest: caught missing-command and missing-firmware")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if args.selftest:
        selftest()

    commands, files = live_state()
    missing = missing_prereqs(commands, files)
    if missing:
        print("boot-prereqs: FAIL")
        for item in missing:
            print(f"  missing {item}")
        return 1
    print(
        "boot-prereqs: PASS: "
        f"{len(commands)} commands and {len(files)} firmware files"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
