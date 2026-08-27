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

KERNEL_ROOT = Path(__file__).resolve().parents[2]
REQUIRED_SCRIPTS = (
    Path("tools/images/mkiso.sh"),
    Path("verify.sh"),
    Path("tools/checks/verify-iso.sh"),
    Path("tools/checks/verify-64.sh"),
    Path("tools/checks/verify-efi.sh"),
    Path("tools/checks/verify-raw.sh"),
    Path("tools/checks/verify-disk.sh"),
    Path("tools/checks/verify-clock.sh"),
    Path("tools/checks/verify-net.sh"),
)


def missing_prereqs(
    commands: dict[str, bool],
    files: dict[str, bool],
    scripts: dict[str, bool],
) -> list[str]:
    missing = [f"command:{name}" for name, present in commands.items() if not present]
    missing.extend(f"file:{name}" for name, present in files.items() if not present)
    missing.extend(
        f"executable-script:{name}" for name, executable in scripts.items() if not executable
    )
    return missing


def live_state() -> tuple[dict[str, bool], dict[str, bool], dict[str, bool]]:
    commands = {name: shutil.which(name) is not None for name in REQUIRED_COMMANDS}
    files = {str(path): path.is_file() for path in REQUIRED_FILES}
    scripts = {
        str(path): (KERNEL_ROOT / path).is_file() and (KERNEL_ROOT / path).stat().st_mode & 0o111 != 0
        for path in REQUIRED_SCRIPTS
    }
    return commands, files, scripts


def selftest() -> None:
    commands = {name: True for name in REQUIRED_COMMANDS}
    files = {str(path): True for path in REQUIRED_FILES}
    scripts = {str(path): True for path in REQUIRED_SCRIPTS}
    assert not missing_prereqs(commands, files, scripts)

    commands["qemu-system-x86_64"] = False
    assert missing_prereqs(commands, files, scripts) == ["command:qemu-system-x86_64"]
    commands["qemu-system-x86_64"] = True

    ovmf = str(REQUIRED_FILES[0])
    files[ovmf] = False
    assert missing_prereqs(commands, files, scripts) == [f"file:{ovmf}"]
    files[ovmf] = True

    script = str(REQUIRED_SCRIPTS[0])
    scripts[script] = False
    assert missing_prereqs(commands, files, scripts) == [f"executable-script:{script}"]
    print(
        "boot-prereqs selftest: caught missing-command, missing-firmware, "
        "and non-executable-script"
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if args.selftest:
        selftest()

    commands, files, scripts = live_state()
    missing = missing_prereqs(commands, files, scripts)
    if missing:
        print("boot-prereqs: FAIL")
        for item in missing:
            print(f"  missing {item}")
        return 1
    print(
        "boot-prereqs: PASS: "
        f"{len(commands)} commands, {len(files)} firmware files, and "
        f"{len(scripts)} executable scripts"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
