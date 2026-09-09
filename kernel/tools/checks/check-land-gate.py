#!/usr/bin/env python3
"""Prove that deleting or conditionally hiding a landing authority is red."""

from __future__ import annotations

import argparse
import re
import shlex
from pathlib import Path


GATE = Path(__file__).resolve().parents[3] / "gates" / "land-gate.sh"
VERIFY_NET = Path(__file__).resolve().parent / "verify-net.sh"

REQUIRED_SNIPPETS = (
    'run "mandatory boot prerequisites"',
    'run "contained gate launcher contract"',
    'run "hosted gate launcher contract"',
    'python3 tools/checks/check-land-gate.py --selftest',
    'run "QEMU crash classifier"',
    'run "wrapper inventory write"',
    'run "wrapper inventory check"',
    'python3 tools/checks/check-build-contract.py --selftest',
    'run "host dependency lock write"',
    'python3 tools/generators/gen-dependency-lock.py --check --selftest',
    'run "offline dependency archive receipt refresh"',
    'python3 tools/checks/verify-dependency-archives.py --receipt-check --selftest',
    'run "license/provenance truth write"',
    'python3 tools/generators/gen-license-registry.py --check --selftest',
    'run "build input identity"',
    'run "toolchain manifest write"',
    'run "toolchain manifest check"',
    'run "build graph write"',
    'run "build graph check"',
    'run "source snapshot write"',
    'run "source snapshot check"',
    'run "kernel raw"',
    'run "kernel ELF permissions"',
    'run "SOURCES recovery selftest"',
    'run "SOURCES coverage"',
    'run "host test inventory write"',
    'run "host test inventory check"',
    'run "host tests execute"',
    'run "host benchmark receipt"',
    'run "host build benchmark receipt"',
    'run "zl call sites"',
    'run "zl generated dispatch"',
    'run "address-space registry write"',
    'run "address-space registry check"',
    'run "memory map"',
    'run "memory map mutation"',
    'run "memory-map mirrors"',
    'run "memory-map mirror canary"',
    'run "UI scale contract"',
    'run "unique app ids"',
    'run "app registry coverage"',
    'run "application manifest"',
    'run "app lifecycle verifier"',
    'run "reproducible artifact verifier"',
    'run "high-RAM map"',
    'FAIL (reverse SOURCES: kernel/SOURCES is missing)',
    'src/graphics/fonts/font_big.c|src/graphics/icons/icons_rgb.c)',
    'run "reproducible kernel and ISO"',
    'tools/images/mkiso.sh verify.sh tools/checks/verify-iso.sh',
    'tools/checks/verify-64.sh tools/checks/verify-efi.sh',
    'tools/checks/verify-raw.sh tools/checks/verify-disk.sh',
    'tools/checks/verify-clock.sh tools/checks/verify-net.sh',
    'run "final canonical ISO"',
    'run "CPU fault capture QEMU"',
    'run "CPU fault capture native UEFI64 QEMU"',
    'run "CPU GP error-code capture native UEFI64 QEMU"',
    'run "CPU double-fault IST capture native UEFI64 QEMU"',
    'run "CPU double-fault task-gate capture BIOS32 QEMU"',
    'run "USB keyboard re-plug native UEFI64 QEMU"',
    'run "app routes QEMU"',
    'run "rail register QEMU"',
    'python3 tools/probes/probe-rail.py --no-build',
    'run "47-app lifecycle QEMU"',
    'run "Run route QEMU"',
    'python3 tools/probes/probe-run.py --no-build',
    'run "persistent user-process command QEMU"',
    'python3 tools/probes/probe-user-process.py --no-build',
    'run "normal-exit user-process command QEMU"',
    'python3 tools/probes/probe-user-process-exit.py --no-build',
    'run "sleeping user-process command QEMU"',
    'python3 tools/probes/probe-user-process-exit.py --no-build --sleep',
    'run "userspace child fault and wait QEMU"',
    'run "userspace child signed exit and wait QEMU"',
    'run "userspace live orphan adoption QEMU"',
    'run "userspace terminal orphan adoption QEMU"',
    'python3 tools/probes/probe-user-spawn-wait.py --no-build',
    'python3 tools/probes/probe-user-spawn-wait.py --no-build --normal-exit',
    'run "page-table QEMU receipt check"',
    'run "physical allocator QEMU receipt check"',
    'run "application evidence registry write"',
    'run "application evidence registry check"',
    'run "hardware receipt plan write"',
    'run "hardware receipt plan check"',
    'run "artifact and boot-route registry write"',
    'run "artifact and boot-route registry check"',
    'run "final build graph artifact rebind write"',
    'run "final build graph artifact rebind check"',
    'run "initialization registry write"',
    'run "initialization registry check"',
    'run "adversarial registry write"',
    'run "adversarial registry check"',
    'run "host benchmark receipt check"',
    'run "host build benchmark receipt check"',
    'run "performance regression registry write"',
    'run "performance regression registry check"',
    'run "current visual receipt write"',
    'run "visual golden registry write"',
    'run "current visual receipt check"',
    'run "visual golden registry check"',
    'run "visual evidence registry write"',
    'run "visual evidence registry check"',
    'run "accessibility proof registry write"',
    'run "accessibility proof registry check"',
    'run "security claim registry write"',
    'run "security claim registry check"',
    'run "decision ledger write"',
    'run "decision ledger check"',
    'run "event trace host receipt write"',
    'run "event trace host receipt check"',
    'run "event schema registry write"',
    'run "event schema registry check"',
    'run "observability registry write"',
    'run "observability registry check"',
    'run "release notes write"',
    'run "release notes check"',
    'run "provenance viewer write"',
    'run "provenance viewer check"',
    'run "joined evidence registry write"',
    'run "joined evidence registry check"',
    'run "906 feature status write"',
    'run "906 feature status check"',
    'run "906 partial closure write"',
    'run "906 partial closure check"',
    'run "906 master program"',
    'resource admission stayed blocked for $((waits * 30)) seconds',
    'await_guard || exit 2',
)

OPTIONAL_AUTHORITY = re.compile(
    r'\[ -[fx] "\$WT/kernel/(?!SOURCES)[^"\n]+" \](?:\s*\\\s*)?(?:&&)?'
)
OPTIONAL_BOOT = re.compile(r'\[ -x "\$WT/kernel/\$g" \] \|\| continue')
HOST_BUILD_BENCHMARK_GUARD = re.compile(
    r'run "host benchmark receipt".*?await_guard \|\| exit 2\s*'
    r'run "host build benchmark receipt"',
    re.S,
)


BIOS32_DOUBLE_FAULT = "CPU double-fault task-gate capture BIOS32 QEMU"
USB_REPLUG = "USB keyboard re-plug native UEFI64 QEMU"
USB_REPLUG_COMMAND = "python3 tools/checks/verify-usb-replug.py --run --route native-uefi64 --no-build"


PROCESS_SCENARIOS = (
    ("userspace child fault and wait QEMU", False, None),
    ("userspace child signed exit and wait QEMU", True, None),
    ("userspace live orphan adoption QEMU", True, "parent-first"),
    ("userspace terminal orphan adoption QEMU", False, "child-first"),
)


def failures(source: str, verify_net: str | None = None) -> list[str]:
    if verify_net is None:
        verify_net = VERIFY_NET.read_text()
    # a mandatory invocation that survives only inside a `#` comment is not
    # an invocation (every snippet passed commented out, 2026-09-04)
    code = "\n".join(
        line for line in source.splitlines() if not line.lstrip().startswith("#")
    )
    errors = [
        f"missing mandatory invocation: {snippet}"
        for snippet in REQUIRED_SNIPPETS
        if snippet not in code
    ]
    logical_lines = code.replace("\\\n", " ").splitlines()
    crash_commands = [line for line in logical_lines
                      if line.startswith('run "' + BIOS32_DOUBLE_FAULT + '"')]
    if len(crash_commands) != 1 or shlex.split(crash_commands[0])[3:] != [
            "python3", "tools/checks/verify-crash.py", "--run", "--route", "bios32",
            "--fault", "double-fault", "--no-build", "--selftest"]:
        errors.append("BIOS32 double-fault gate is missing, repeated or selects the wrong mode")
    usb_commands = [line for line in logical_lines
                    if line.startswith('run "' + USB_REPLUG + '"')]
    if len(usb_commands) != 1 or shlex.split(usb_commands[0])[2:] != [
            "$WT/kernel", *shlex.split(USB_REPLUG_COMMAND)]:
        errors.append("USB re-plug gate is missing, repeated or selects the wrong runtime route")
    for title, normal_exit, orphan_order in PROCESS_SCENARIOS:
        commands = [line for line in logical_lines if line.startswith('run "' + title + '"')]
        if len(commands) != 1:
            errors.append("missing or repeated process boot scenario: " + title)
            continue
        arguments = shlex.split(commands[0])
        orders = [arguments[index + 1] if index + 1 < len(arguments) else ""
                  for index, arg in enumerate(arguments) if arg == "--orphan-order"]
        expected_orders = [] if orphan_order is None else [orphan_order]
        if (arguments[3:5] != ["python3", "tools/probes/probe-user-spawn-wait.py"] or
                "--no-build" not in arguments or "--fixtures-only" in arguments or
                ("--normal-exit" in arguments) != normal_exit or orders != expected_orders or
                any(arg.startswith("--orphan-order=") for arg in arguments)):
            errors.append("process boot scenario is downgraded or selects the wrong fixture: " + title)
    if OPTIONAL_AUTHORITY.search(source):
        errors.append("kernel authority is hidden behind an existence guard")
    if OPTIONAL_BOOT.search(source):
        errors.append("named boot script can be skipped when absent")
    if not HOST_BUILD_BENCHMARK_GUARD.search(source):
        errors.append("host build benchmark lacks its own quiet-host admission guard")
    if "python3 tools/probes/probe-net.py --fetch" not in verify_net:
        errors.append("network boot gate does not use the synchronized fetch probe")
    if ".NEq" in verify_net:
        errors.append("network boot gate enqueues dependent commands together")
    for required in (
        "out=$( cd \"$dir\" && \"$@\" 2>&1 ); rc=$?",
        'echo ">>> FAIL ($label) exit=$rc"\n    FAIL=$((FAIL+1))',
        "exit $FAIL",
    ):
        if required not in source:
            errors.append(f"landing aggregator does not preserve child failure: {required}")
    return errors


def expect_failure(source: str, label: str, verify_net: str | None = None) -> None:
    if not failures(source, verify_net):
        raise AssertionError(f"mutation escaped: {label}")


def selftest(source: str) -> None:
    verify_net = VERIFY_NET.read_text()
    expect_failure(
        source.replace('run "build input identity"', '# removed identity gate', 1),
        "deleted-verifier",
    )
    # the invocation kept verbatim but commented out is the same defect
    expect_failure(
        source.replace('run "build input identity"', '# run "build input identity"', 1),
        "commented-out-verifier",
    )
    expect_failure(
        source.replace(
            'run "app routes QEMU"',
            '[ -f "$WT/kernel/probe-app-routes.py" ] && run "app routes QEMU"',
            1,
        ),
        "optional-verifier",
    )
    expect_failure(
        source.replace(
            'FAIL (reverse SOURCES: kernel/SOURCES is missing)',
            'missing SOURCES ignored',
            1,
        ),
        "missing-sources-manifest",
    )
    expect_failure(
        source.replace(
            'src/graphics/fonts/font_big.c|src/graphics/icons/icons_rgb.c)',
            'src/graphics/fonts/font_big.c)',
            1,
        ),
        "deleted-generated-data-classification",
    )
    expect_failure(
        source.replace(
            'tools/checks/verify-64.sh tools/checks/verify-efi.sh',
            'tools/checks/verify-efi.sh',
            1,
        ),
        "deleted-boot-route",
    )
    expect_failure(
        source.replace('run "kernel raw"', '# removed raw build', 1),
        "deleted-raw-build-before-permission-check",
    )
    expect_failure(
        source.replace(
            'run "final build graph artifact rebind check"',
            '# removed final graph rebind check',
            1,
        ),
        "deleted-final-graph-rebind",
    )
    expect_failure(
        source.replace(
            'run "physical allocator QEMU receipt check"',
            '# removed physical allocator receipt check',
            1,
        ),
        "deleted-physical-allocator-receipt-check",
    )
    expect_failure(
        source.replace('run "rail register QEMU"', '# removed rail gate', 1),
        "deleted-rail-gate",
    )
    expect_failure(
        source.replace(
            'run "persistent user-process command QEMU"',
            '# removed persistent user-process command gate',
            1,
        ),
        "deleted-user-process-command-gate",
    )
    expect_failure(
        source.replace(
            'run "normal-exit user-process command QEMU"',
            '# removed normal-exit user-process command gate',
            1,
        ),
        "deleted-normal-exit-user-process-command-gate",
    )
    expect_failure(
        source.replace(
            'run "sleeping user-process command QEMU"',
            '# removed sleeping user-process command gate',
            1,
        ),
        "deleted-sleeping-user-process-command-gate",
    )
    expect_failure(source.replace('run "' + BIOS32_DOUBLE_FAULT + '"',
                                  '# removed BIOS32 double-fault gate', 1),
                   "deleted-bios32-double-fault-gate")
    crash_command = ("python3 tools/checks/verify-crash.py --run --route bios32 "
                     "--fault double-fault --no-build --selftest")
    for old, new in (("--route bios32", "--route native-uefi64"),
                     ("--fault double-fault", "--fault ud2"), (" --run", "")):
        expect_failure(source.replace(crash_command, crash_command.replace(old, new), 1),
                       "wrong-bios32-double-fault-mode-" + old.strip())
    usb_title = 'run "' + USB_REPLUG + '"'
    expect_failure(source.replace(usb_title, '# removed USB re-plug gate', 1),
                   "deleted-usb-replug-gate")
    for old, new in ((" --run", ""), ("native-uefi64", "bios32"),
                     ("verify-usb-replug.py", "verify-crash.py")):
        expect_failure(source.replace(USB_REPLUG_COMMAND,
                                      USB_REPLUG_COMMAND.replace(old, new), 1),
                       "wrong-usb-replug-mode-" + old.strip())
    expect_failure(source.replace(usb_title + ' "$WT/kernel"', usb_title + ' "$WT"', 1),
                   "wrong-usb-replug-directory")
    expect_failure(source + '\n' + usb_title + ' "$WT/kernel" ' + USB_REPLUG_COMMAND + '\n',
                   "duplicated-usb-replug-gate")
    for title, _normal_exit, _orphan_order in PROCESS_SCENARIOS:
        expect_failure(source.replace('run "' + title + '"', '# removed ' + title, 1),
                       "deleted-" + title.replace(" ", "-"))
    expect_failure(source.replace("--orphan-order child-first", "--orphan-order parent-first", 1),
                   "wrong-terminal-orphan-order")
    for flag in ("--fixtures-only", "--normal-exit", "--orphan-order parent-first"):
        expect_failure(source.replace("probe-user-spawn-wait.py --no-build",
                                      "probe-user-spawn-wait.py --no-build " + flag, 1),
                       "wrong-spawn-boot-mode-" + flag)
    expect_failure(
        source.replace(
            'run "host benchmark receipt" "$WT/kernel" python3 tools/run/run-benchmarks.py --run --selftest\n'
            '# The frame benchmark can occupy the host long enough for another task to\n'
            '# resume. Admit the independently measured build distribution separately.\n'
            'await_guard || exit 2\n'
            'run "host build benchmark receipt"',
            'run "host benchmark receipt" "$WT/kernel" python3 tools/run/run-benchmarks.py --run --selftest\n'
            'run "host build benchmark receipt"',
            1,
        ),
        "deleted-host-build-benchmark-guard",
    )
    expect_failure(
        source.replace(
            'resource admission stayed blocked for $((waits * 30)) seconds',
            'resource admission waits forever',
            1,
        ),
        "deleted-bounded-resource-admission",
    )
    expect_failure(
        source.replace(
            'run "CPU double-fault IST capture native UEFI64 QEMU"',
            '# removed double-fault IST gate',
            1,
        ),
        "deleted-double-fault-gate",
    )
    expect_failure(
        source,
        "deleted-synchronized-network-fetch",
        verify_net.replace(
            "python3 tools/probes/probe-net.py --fetch",
            "python3 tools/probes/probe-net.py",
            1,
        ),
    )
    expect_failure(
        source,
        "restored-network-command-race",
        verify_net + "\nKEYS='.NEq'\n",
    )
    expect_failure(source.replace("exit $FAIL", "exit 0", 1), "masked-final-exit")
    expect_failure(
        source.replace(
            'echo ">>> FAIL ($label) exit=$rc"\n    FAIL=$((FAIL+1))',
            'echo ">>> FAIL ($label) exit=$rc"\n    : # failure ignored',
            1,
        ),
        "masked-child-failure",
    )
    print(
        "land-gate selftest: caught deleted-verifier, optional-verifier, "
        "missing-SOURCES, deleted-generated-data-classification, deleted-boot-route, "
        "deleted-raw-build-before-permission-check, "
        "deleted-final-graph-rebind, deleted-physical-allocator-receipt-check, "
        "deleted-rail-gate, deleted-user-process-command-gate, "
        "deleted-normal-exit-user-process-command-gate, "
        "deleted-sleeping-user-process-command-gate, "
        "deleted-usb-replug-gate, wrong-usb-replug-mode, "
        "wrong-usb-replug-directory, duplicated-usb-replug-gate, "
        "deleted-userspace-child-fault-and-wait-QEMU, "
        "deleted-userspace-child-signed-exit-and-wait-QEMU, "
        "deleted-userspace-live-orphan-adoption-QEMU, deleted-userspace-terminal-orphan-adoption-QEMU, "
        "wrong-terminal-orphan-order, wrong-spawn-boot-mode-orphan-order, "
        "wrong-spawn-boot-mode-fixtures-only, wrong-spawn-boot-mode-normal-exit, "
        "deleted-double-fault-gate, deleted-bios32-double-fault-gate, "
        "wrong-bios32-double-fault-route/fault/run-mode, "
        "deleted-bounded-resource-admission, "
        "deleted-synchronized-network-fetch, "
        "restored-network-command-race, masked-final-exit and masked-child-failure"
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    source = GATE.read_text()
    if args.selftest:
        selftest(source)
    errors = failures(source)
    if errors:
        print("land-gate closure: FAIL")
        for error in errors:
            print(f"  {error}")
        return 1
    print(f"land-gate closure: PASS: {len(REQUIRED_SNIPPETS)} mandatory seams")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
