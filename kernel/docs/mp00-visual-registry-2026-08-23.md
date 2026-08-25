# MP-00 visual evidence registry

`kernel/metadata/visual-registry.json` hashes and measures all 41 PNG/PPM visual assets
currently present in the implementation worktree. It pairs the 20 named BIOS
and UEFI exercise scenarios without assuming that equal names mean equal pixels.

The result is `INVENTORY_WITH_OPEN_GAPS`. All 41 retained images are prior
QEMU/visual assets unbound to the current build identity; no host-render output
is currently retained. They are useful prior assets, not current
visual-regression proof.

Scale, theme, locale, accessibility, UI-state and backend coverage all remain
open because the assets lack the required scenario metadata and current
artifact binding. The self-test catches a missing asset, invented build binding,
missing dimensions, hidden variant gap and unearned completion claim.
