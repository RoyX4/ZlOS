# Application registry truth repair — 2026-08-22

This is the MP-00/MP-01 implementation receipt for the reference-app catalogue
false green.

## Reproduced failure before the fix

On branch base `b8a00ec45ac1f9f955ba79ed63f0072540067d4e`, this command exited zero:

```sh
python3 kernel/tests/host/apps53.py
```

Its own table said:

```text
reference apps: 53      REG_FIRST=14 REG_LAST=69   reg_count()=47
26  Maze ... id 34  exists NO ... dock/shell/boot
all 53 reference apps resolve to an id in the tree
```

The count stayed plausible because two errors cancelled numerically:

- `APP_MAZE = 34`, but `reg_exists()` rejected the whole range 34 through 39;
- `REG_FIRST = 14`, but no app owned ID 14, so the dense catalogue included a
  blank tile.

The checker only failed when a name was absent. It printed existence, size and
icon failures but never added them to its failure set, and it never checked the
inverse question “does every dense catalogue ID name a real reference app?”

## Implementation

- `REG_FIRST` is now 15, the first real registry app (`APP_CLIP`).
- the unallocated gap is 35 through 39; ID 34 is Maze.
- `apps53.py` fails independently on identity, existence, size, icon, route and
  blank/extra dense IDs.
- `--selftest` plants five in-memory mutations: missing existence, blank ID,
  missing size, missing icon and missing identity. Each must be rejected.
- the oracle's source contract now agrees on `REG_FIRST = 15`.
- `probe-catalog.py` no longer clicks only tile zero. It parses Maze's dense
  index from the real registry, scrolls to it with the pointer and requires a
  new compositor window.
- `gates/land-gate.sh` now runs the duplicate-ID self-test and the full
  app-registry coverage self-test.
- `kernel.zl::app_name()` is now the canonical identity seam for IDs 0 through
  13 and All Applications; it delegates registry IDs to `reg_name()`.
- `gen-app-manifest.py` generates/checks `app-manifest.json` from `app_name`,
  `reg_name`, every slice name table, `reg_exists` and the reference categories.
  The manifest contains all 61 current named implementations, the separate
  catalogue surface and exactly 24 games. Its evidence ceiling explicitly says
  static source/build identity, not runtime or physical proof.
- the generator derives the app-module set from `kernel.zl`'s actual imports,
  rejects any unimported `apps_*.zl` file or missing imported file, records the
  four build routes, and rejects conflicting names for the same ID instead of
  allowing a first definition to hide later drift.
- the landing gate checks manifest freshness and mutation-proves missing entry,
  duplicate ID, duplicate name, missing required field/source and conflicting
  identity failures.
- the same generator emits `app_manifest_embed.zl`. Every kernel build imports
  it and the running image prints schema, entry count and the SHA-256 of the
  exact host manifest. The check follows the real recursive import graph;
  `check-zlcalls.py` was corrected because its old “every zl file” claim only
  globbed `apps_*.zl` and initially ignored the generated module.
- `gen-build-identity.py` inventories 121 shared build inputs: the active zl
  import closure, declared C sources, kernel/freestanding headers, entry/linker
  files, all four route scripts, the zl compiler binary and exact host-tool
  versions. It records Git head, branch and dirty state and emits
  `build_identity_embed.zl`; all four kernel build scripts reject stale app or
  build identities before compilation.
- the running kernel reports the build ID and Git head/dirty state on two
  bounded lines. The first implementation used one 150-character zl literal;
  the compiler truncated it, the QEMU receipt found zero valid identities, and
  the build was rejected. Both lines are now independently required.
- `wm.c` emits a serial-only, versioned lifecycle event for open, first draw
  (`ready`) and close. The tuple `(slot, app, generation)` must stay stable and
  the live-window count must return to its pre-open value.
- the terminal dock path now recreates a closed boot terminal. The old path
  retained a dead slot and only tried to raise it; QEMU reproduced the dock
  click timing out after Ctrl+W before the shared create/reopen path was added.
- `dock_app(slot)` is the single source mapping for all nine dock identities;
  both the UI and the route probe consume it.
- the graphical terminal now maps both `.` and `mount` to the kernel's existing
  zlfs/NVMe command 46. The help and kernel dispatch already advertised `.`,
  but the terminal word table omitted it, making the path unreachable after
  the compositor became the boot state.
- the hybrid ISO and native UEFI USB image are now reproducible. Two unchanged
  builds originally produced identical kernels but different ISO/USB bytes.
  `SOURCE_DATE_EPOCH` alone did not reach GRUB's hidden wall-clock filename,
  random GPT identifiers, FAT serials or directory timestamps. The recipes now
  use explicit xorriso dates, content-derived GPT/FAT identities, invariant FAT
  formatting, PE `/Brepro`, and a build-local clock wrapper scoped only to the
  packaging children. They never change the system clock or kernel build.
- `check-reproducible-build.py` performs two complete recipe runs and requires
  byte-for-byte equality of all nine outputs: three ELFs, the EFI application,
  both 32-bit and 64-bit GRUB ISOs, raw disk, USB disk and boot-media identity
  metadata. Its self-test mutates every artifact, removes one, and rejects stale
  or corrupted resume snapshots. A durable JSON receipt binds all hashes to the
  build identity and the exact checker source.

## Deterministic evidence

```text
$ python3 kernel/tools/checks/check-appids.py --selftest
selftest: planted APP_CATALOG = 13, caught as APP_CATALOG (apps_registry.zl), APP_FILES (kernel.zl)
check-appids: no duplicate app ids

$ python3 kernel/tests/host/apps53.py --selftest
reference apps: 53      REG_FIRST=15 REG_LAST=69   reg_count()=47
...
26  Maze ... id 34  exists yes  size yes  icon yes  catalog-static
...
selftest: caught missing-existence, blank-extra-id, missing-size, missing-icon, missing-identity
all 53 reference apps have complete static registry fields; catalog has no blank ids

$ ./kernel/tools/checks/check-zlcalls.py
check-zlcalls: 861 zl functions, 643 builtins, every call site resolves

$ cd kernel && python3 gen-app-manifest.py --check --selftest
app-manifest selftest: caught missing-entry, duplicate-id, duplicate-name, missing-field, missing-source, identity-conflict
app-manifest: PASS: 61 named implementations + 1 catalogue surface, 24 games

$ cd kernel && python3 check-reproducible-build.py --check --selftest
reproducible-build selftest: caught byte drift in all 9 artifacts, a missing artifact, and stale resume snapshots
reproducible-build: kernel.elf sha256=65dfe49e3ce7b7f4d714590df9c51d5c41ea6ffb602068f170c528a2919a360a
reproducible-build: kernel64.elf sha256=9799d74f80f7ea9db527a3965ae212e581658fe96f26f3318ecd88277a722516
reproducible-build: kernel_raw.elf sha256=2135fb1cf9a79deac26d5837b8c92cd733f8f5b904ca404de3e578906fc36348
reproducible-build: BOOTX64.EFI sha256=9f3b3b5fd90d35d1adb90543ecbd660ed6e5582657f199dca537a6fe544a09fe
reproducible-build: zlOS.iso sha256=8ea44217a7aecf51f7f799dab6ec98d509c688d045aaabb218c4a4b03cb4d8de
reproducible-build: zlOS64.iso sha256=4ce7c4691748e0558faa07207de8c3bd22cf0cba6aef2d71103367ebf8a6b1fb
reproducible-build: zlOS.img sha256=72de4e361a6db29a0c1e7706a37b6833b4f78eb40f146354583f9565ba5aaf8b
reproducible-build: zlOS-usb.img sha256=7ce06c9246354bf6603e554f1d8607c0cb0ab0aacccc5af35ed6f5119d8b924d
reproducible-build: boot-media-ids.json sha256=7a11f3239f9eabcbf664f95b70fd3b19bd76c7dace16b5d5a04d42a2f11d7957
reproducible-build: PASS: two independent recipe runs match
```

All four source-consuming build routes passed from the isolated worktree:

```text
32-bit BIOS native-source image: kernel.elf 4582476 bytes, undefined symbols 0, multiboot header present
64-bit:      kernel64.elf 2692168 bytes, undefined symbols 0, ELF64
native UEFI: BOOTX64.EFI 2706944 bytes
raw loader:  zlOS.img 6291968 bytes; boot sector 512 bytes; kernel 4454540 bytes; 1793 KiB headroom
```

## QEMU identity, route and lifecycle evidence

The final route and lifecycle sweeps booted the unmodified tracked source at its
native 1920x1200 layout. This is real pointer/compositor/QEMU evidence for the
corrected catalogue path. Earlier 1280x800 sweeps used a temporary modeset
source variant; they were discarded because the restored source hash did not
identify the bytes that were actually compiled. `zlosboot`'s provenance paths
now use `how="native"`; visual-only variants suppress canonical identity lines.

The final deterministic 32-bit ISO has SHA-256
`ca67eacab5d0c4913a40f12307b7998ac7f0fcd743773544d0a1552838550f46`.
The separate exact-kernel64 ISO has SHA-256
`3ea3ea875fdfaa60f0754f5e782111b9a04acf32d8188a92ed8ffdf3a5d16ab7`.
At boot it reported manifest SHA-256
`a1c4ec15e88d709e59096dce8f08b1b6e37adee91148d706c439f59d3d2d9da7`
and route-neutral build-input identity
`85027b159c9a594045c2f900e5971bb3408dd418dd61a373625425fba9030d13`.
Three independently generated receipts all name that exact artifact:

- `docs/receipts/app-lifecycle-qemu-2026-08-22.json`: all 47 catalogue apps
  completed open -> first compositor draw -> close; every cycle preserved its
  slot/generation identity and returned live windows from 5 to 4.
- `docs/receipts/app-routes-qemu-2026-08-22.json`: three boot-open identities,
  all nine dock routes, all seven shell-word routes, Menu and All Applications
  passed their applicable open/ready/close contracts.
- `docs/receipts/run-qemu-2026-08-22.json`: Run ID 7 distinguished no filename,
  no filesystem and no-such-file states; `.` formatted/mounted a fresh zlfs
  volume; the second Run reused the existing window; Terminal stayed live; and
  Run closed without leaking a window.

Together these receipts give route plus first-draw evidence for all 61 current
named implementations and the separate All Applications surface. The 24 games
are included in those 61. This is not a claim that every internal workflow was
completed: `ready` means the compositor actually invoked the app's draw hook,
not that every control, persistence path or failure mode works.

`gen-app-evidence.py` joins the generated identity manifest and all three
receipts into `app-evidence.json`. It verifies current source hashes, a single
artifact hash, exact ID/name agreement, lifecycle identity, teardown counts,
47/47 catalogue results, duplicate-Run rejection and the boot-reported manifest
digest. All 62 identities now carry shipped-manifest membership for this exact
tested image. It also requires the boot-reported whole-build ID on all six
named boot routes. Its mutation tests prove that artifact mismatch, missing
readiness, an unproved identity, identity drift, a duplicate Run window and a
missing shipped-manifest receipt or wrong build identity each turn the gate red. `--verify-artifact` also
requires the currently present ISO to be the tested bytes; the landing gate
omits only that volatile check because later boot stages legitimately rebuild
the ISO.

The same manifest identity was runtime-proved on six boot paths across four
distinct artifacts, all retained in `app-evidence.json`:

- BIOS via GRUB/multiboot: deterministic `zlOS.iso`, all app routes and
  `app-manifest-grub-bios32-qemu-2026-08-22.json`;
- UEFI via GRUB/multiboot: `app-manifest-grub-uefi32-qemu-2026-08-22.json`;
- BIOS via GRUB/multiboot64: exact `kernel64.elf` in `zlOS64.iso`, owned
  long-mode switch, delayed `fib 20 = 6765`, and
  `app-manifest-grub-bios64-qemu-2026-08-22.json`;
- UEFI via GRUB/multiboot64: the same exact 64-bit ISO and kernel, owned
  long-mode switch, delayed `fib 20 = 6765`, and
  `app-manifest-grub-uefi64-qemu-2026-08-22.json`;
- raw BIOS via our 512-byte loader, no GRUB: `app-manifest-raw-bios-qemu-2026-08-22.json`;
- native 64-bit UEFI application, no GRUB/bootloader: `app-manifest-native-uefi64-qemu-2026-08-22.json`.

All six boot receipts bind the artifact, transient boot log, exact boot-origin
line, QEMU version, manifest, generated embed, kernel, harness and receipt-writer
hashes. The evidence registry rejects a planted wrong manifest on either boot
route. This is QEMU runtime proof, not physical firmware or panel proof.

## Verification not claimed

- The whole `gates/land-gate.sh` was not rerun; its new registry, identity and
  reproducibility stages were run directly and mutation-proved.
- No physical ThinkPad boot was performed.
- The successful Run receipt proves the loader's refusal ladder, not successful
  loading/execution of a valid external program.
- QEMU input/display evidence does not promote USB keyboard, pointer or display
  to native physical-hardware evidence.
- The worktree begins at committed `b8a00ec`; it does not silently incorporate
  the concurrent uncommitted performance/process/network batch in the shared
  checkout.

The current monolithic image-membership step is complete. Future process/package
membership belongs to MP-11. The next current-suite step is to replace the
coarse first-draw ceiling with workflow contracts one app family at a time.
