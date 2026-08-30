# Current RTC feature classification

Date: 2026-08-29

This receipt records a shipped RTC decoder repair and maps its current host and
QEMU evidence into BT-029. It does not promote timezone, synchronization,
suspend behavior or physical clock accuracy.

## Repair

The previous decoder rejected broad numeric ranges but still accepted several
invalid hardware encodings:

- February 30, April 31 and February 29 in a non-leap year;
- hour zero in 12-hour mode;
- malformed BCD nibbles that decoded into plausible decimal values;
- a successful read that retained the previous failure reason.

Regression cases reproduced those failures before the shared decoder changed.
`rtc.c` now validates BCD digits before conversion, applies Gregorian
month-length/leap-year rules, enforces the 1 through 12 range in 12-hour mode,
rejects seconds outside 0 through 59 and clears the failure state on success.

## Current evidence

The exact current subject is the `build_identity` field in generated
`docs/program/FEATURE-STATUS.json` and the RTC receipt; this page does not
duplicate that volatile value.

The complete current host receipt reports 61 passes, 3 hardware skips, 12
explicit non-runs and 0 failures. Its `rtctest` member passes 50 assertions,
covering BCD/binary and 12/24-hour formats, UIP timeout, torn reads, absence,
invalid encodings/dates, leap years, epoch conversion and recovery state.

`kernel/docs/receipts/rtc-clock-qemu-2026-08-29.json` binds the exact current
`kernel.elf`, harness and build identity. Three controlled QEMU bases passed:

| Base | Guest RTC | Epoch | Boot offset |
|---|---|---:|---:|
| 2026-08-18T14:37:05 | 2026-08-18 14:37:05 | 1787063825 | 0 s |
| 2026-12-31T23:59:07 | 2026-12-31 23:59:07 | 1798761547 | 0 s |
| 2000-02-29T12:00:00 | 2000-02-29 12:00:00 | 951825600 | 0 s |

The receipt validator rejects a foreign build, foreign kernel, missing case,
printed-field/epoch disagreement and excessive boot offset.

## Classification

BT-029 moves from `PLANNED_UNPROVED` to `PARTIAL_CURRENT`. The current code and
receipts prove bounded CMOS acquisition and decoding. The feature remains
partial because hardware-clock timezone policy, clock synchronization, drift
and accuracy measurement, suspend/resume semantics, locale separation and a
physical RTC receipt are absent.

Exact current maturity counts remain in generated `program/FEATURE-STATUS.json`.

## Verification

```sh
python3 kernel/tools/generators/gen-build-identity.py --write --selftest
(cd kernel/tests/host && ./build.sh)
python3 kernel/tools/run/run-host-tests.py --run --selftest
kernel/tools/checks/verify-clock.sh
python3 tools/gen_feature_status.py --write --selftest
python3 tools/gen_feature_status.py --check --selftest
python3 tools/validate_master_program.py --self-test
tools/doc-check.sh
```
