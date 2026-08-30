# Visual-receipt feature classification - 2026-08-29

This receipt classifies EV-016 and EV-022 against the canonical 906-feature
ledger. It records the current QEMU capture and strict golden machinery without
confusing one default design path with complete visual coverage.

## Result

EV-016 and EV-022 remain `PARTIAL_CURRENT`. The exact current subject is the
`build_identity` field in generated `docs/program/FEATURE-STATUS.json` and the
bound visual receipt; this page does not duplicate that volatile value.

Two real QEMU boots exercised the exact current artifacts:

| Route | Artifact | Mode | Captures |
|---|---|---|---:|
| `grub-bios32` | `zlOS.iso` | 1920x1200 | 2 |
| `native-uefi64` | `zlOS-usb.img` | 1920x1200 | 2 |

Each boot reached `ready.` and the shell prompt with the embedded current build
identity. The runner captured the desktop, submitted the real `paint` command,
required a new compositor window report, then captured Paint open. All four
PNGs are nonblank and retain byte, RGB-pixel, size, route, artifact, QEMU-binary
and serial-log identities.

The native-UEFI route runs the registered USB image through a QEMU snapshot
layer. This matters because the guest's ZLLOG recorder legitimately writes to
its boot medium: an earlier writable capture changed the image after the
receipt had bound its pre-boot hash. The runner now hashes and sizes every
artifact before boot, rechecks it after QEMU exits, and the corrected receipt
records identical pre/post USB SHA-256 values. Their exact current value remains
in the bound visual receipt rather than being duplicated here.
Observation therefore cannot mutate the proof subject unnoticed.

## Strict goldens

The four captures are current exact-build goldens. Comparison requires exact
RGB equality outside four named live regions:

- advancing frame and wakeup telemetry at the top;
- the System Monitor live graph and counters;
- rail clock, uptime and emulated power state; and
- bottom memory/frame/uptime telemetry.

The checker rejects a missing or extra capture, dimension change, stable-region
pixel change, hidden variant gap or stale receipt. Its self-test paints a
64x64 magenta defect into a stable region and requires failure. A second
mutation inside the declared top telemetry mask must pass, proving the mask is
actually responsible for suppressing live noise rather than a global
tolerance.

## Evidence counts

```text
current QEMU routes:              2
current screenshots:              4
strict current goldens:            4
declared live-region masks:        4
historical unbound images:        41
complete variant dimensions:       0
open variant dimensions:           6
physical display receipts:         0
```

The visual registry preserves all 41 older images as unbound historical input
and labels only the four receipt-owned images current.

## Remaining proof

Scale, theme, locale, accessibility, UI-state and backend matrices are all
partial. There is no current video, no full six-route capture set, no physical
display comparison and no design approval. Claude's separate redesign worktree
was not read into or modified by this tranche; these images document only the
exact artifacts built in the isolated evidence branch.
