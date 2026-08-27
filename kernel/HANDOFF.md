# zlOS — where things stand

> **Authority notice (checked 2026-08-25):** use
> [`../docs/PROJECT-STATUS.md`](../docs/PROJECT-STATUS.md) for current repository
> and implementation truth, [`../docs/REMAINING-WORK.md`](../docs/REMAINING-WORK.md)
> for the immediate queue, and [`../docs/program/README.md`](../docs/program/README.md)
> for the complete product program. This file preserves measured hardware and
> implementation history, so later sections intentionally retain dated claims
> and reversals. T-8 in [`.ultra/TENSIONS.md`](../.ultra/TENSIONS.md) records
> the metadata chain that cannot currently regenerate as one build identity.

**The 64-bit BIOS+GRUB boot route is BROKEN and no gate watches it.** It stalls
after "keyboard on IRQ1" and never reaches "ready."; its sibling grub-uefi64
passes with the same kernel64.elf. Confirmed pre-existing by an A/B against
`7d1a11b` in a detached worktree - byte-identical failure signature.
`tools/preflight.sh` runs bios32, raw, iso and efi, and NOT verify-64, which is
why `grub-bios64`'s boot receipt still carries a PASS dated from `b8a00ec`.
Details, and why you must not simply add the gate to preflight, are in
[`../docs/evidence/grub-bios64-unwatched-2026-08-27.md`](../docs/evidence/grub-bios64-unwatched-2026-08-27.md).

The desktop's three edge reserves were written down ELEVEN times across wm.c,
snap.c, four host tests, four boot gates and the receipt writer, and by
2026-08-27 no two sets agreed - including a `72 * t->scale` in wm.c whose
comment cited a `dock_y()` that had been deleted. The full table, why every
gate was blind to it (several of the gates WERE copies), and what replaced it
are in
[`../docs/evidence/one-fact-many-copies-2026-08-27.md`](../docs/evidence/one-fact-many-copies-2026-08-27.md).
Read it before writing a number that describes the shell.

**`kernel/tests/host/run-all.sh` is how you run the host suite** - not
`build.sh` followed by running binaries by name. `build.sh` has `set -e` and
stops at the first link error, and the previous run's binaries stay on disk, so
that habit reports "all passed" from code that predates your change. It happened
in this session. run-all.sh deletes first and treats NOT BUILT as a failure.

About 125 tracked files were truncated to zero bytes on 2026-08-27 and committed
via `git add -A`; the recovery, and the fact that two simultaneous registered-claim
failures were the only thing that noticed, are in
[`../docs/evidence/truncation-incident-2026-08-27.md`](../docs/evidence/truncation-incident-2026-08-27.md).

The 2026-08-24 host freeze, the evidence that survived it, and the mandatory
resource-contained landing-gate procedure are recorded in
[`docs/host-freeze-and-gate-containment-2026-08-24.md`](docs/host-freeze-and-gate-containment-2026-08-24.md).
Do not run `gates/land-gate.sh` directly; it now refuses unrestricted starts.

The EV-018 structured-event envelope, bounded queue, wire contract, 37-check
host receipt and explicit target-integration gap are recorded in
[`docs/mp00-structured-event-trace-2026-08-24.md`](docs/mp00-structured-event-trace-2026-08-24.md).
It is host-proved core code, not yet a booted zlOS audit service.

The dated generated EV-026 decision/reversal/deprecation ledger and its exact
historical coverage gap are recorded in
[`docs/mp00-decision-ledger-2026-08-24.md`](docs/mp00-decision-ledger-2026-08-24.md).
It normalizes 19 high-impact records and indexes all 47 legacy labels without
pretending the remaining 42 labels already have normalized semantics.

The generated EV-027 unreleased changelog, compatibility facts, migrations,
known defects and recovery paths are recorded in
[`docs/mp00-release-notes-2026-08-24.md`](docs/mp00-release-notes-2026-08-24.md).
It has 17 change candidates and 12 blockers at its generation snapshot, but
deliberately zero published entries: there is still no versioned, signed or
authorized release.

The generated EV-028 read-only provenance model and self-contained evidence-room
HTML are recorded in
[`docs/mp00-provenance-viewer-2026-08-24.md`](docs/mp00-provenance-viewer-2026-08-24.md).
They expose exact origin, licenses, signatures, tests, health, 9 artifacts, 62
apps, 17 security claims and 17 changes. This is host-static, not a booted zlOS
app or signed/live portal.

The corrected reference-app registry and Maze pointer-route receipt is
[`docs/evidence/app-registry-truth-2026-08-22.md`](docs/evidence/app-registry-truth-2026-08-22.md).
It replaces the old aggregate `apps53.py` green that printed `Maze exists NO`
and hid the missing route behind blank catalogue ID 14.

The MP-00 host-test inventory and dated 62-target execution receipt is
[`docs/evidence/mp00/test-inventory-2026-08-22.md`](docs/evidence/mp00/test-inventory-2026-08-22.md).
It replaces filename-guessed execution with exact pass, hardware-skip,
instrument and manual-action states.

The exact nine-artifact build and six-route boot evidence is
[`docs/artifact-and-boot-route-registry-2026-08-22.md`](docs/artifact-and-boot-route-registry-2026-08-22.md).
Its generated registry separates build-only, parent-payload, QEMU-direct and
physical-unverified claims so one route can never promote another by name.

The exact top-level boot order is generated into
[`docs/init-registry-truth-2026-08-23.md`](docs/init-registry-truth-2026-08-23.md).
It binds 16 initialization stages and their dependencies/fallbacks to all six
promoted QEMU routes without pretending that route readiness is physical or
stage-specific provider proof.

The exact host tools and OVMF bytes behind those artifacts are locked in
[`docs/dependency-lock-truth-2026-08-23.md`](docs/dependency-lock-truth-2026-08-23.md).
The landing gate checks this lock and never silently refreshes it.
The four kernel ABI/link lanes and their 82 external compiler/sysroot headers
are bound separately in
[`docs/mp00-toolchain-manifest-2026-08-24.md`](docs/mp00-toolchain-manifest-2026-08-24.md).
The exact declared-input to logical-object to artifact reachability graph is
[`docs/mp00-build-graph-2026-08-24.md`](docs/mp00-build-graph-2026-08-24.md).

The exact dated 123-file build-input closure is reconstructable from the
deterministic archive described in
[`docs/mp00-source-snapshot-2026-08-24.md`](docs/mp00-source-snapshot-2026-08-24.md).
It is local and unsigned; it is not an off-host or whole-repository backup.

License/provenance truth is recorded in
[`docs/license-provenance-truth-2026-08-23.md`](docs/license-provenance-truth-2026-08-23.md).
There is currently no repository license file, so public release is explicitly
blocked rather than silently treating local files as permission to redistribute.

The dated joined front door for the MP-00 receipt set is
[`docs/mp00-evidence-registry-2026-08-23.md`](docs/mp00-evidence-registry-2026-08-23.md).
It is not a current projection: its upstream registries disagree on build
identity and regeneration fails closed under T-8. The dated verifier-canary,
failure-injection and hostile-corpus boundary is
[`docs/mp00-adversarial-registry-2026-08-23.md`](docs/mp00-adversarial-registry-2026-08-23.md).
The dated frame-performance receipt and its open regressions are
[`docs/evidence/mp00/benchmark-registry-2026-08-23.md`](docs/evidence/mp00/benchmark-registry-2026-08-23.md).
The current screenshot/visual asset boundary is
[`docs/mp00-visual-registry-2026-08-23.md`](docs/mp00-visual-registry-2026-08-23.md).
The current accessibility proof boundary is
[`docs/mp00-accessibility-registry-2026-08-23.md`](docs/mp00-accessibility-registry-2026-08-23.md).
The dated security claim boundary is
[`docs/mp00-security-registry-2026-08-23.md`](docs/mp00-security-registry-2026-08-23.md).
The dated crash/log/event boundary is
[`docs/mp00-observability-registry-2026-08-23.md`](docs/mp00-observability-registry-2026-08-23.md).
Its `PASS_WITH_OPEN_GAPS` result preserves physical, hardware-skip, non-run and
license blockers instead of flattening them into one green count.

Persistent named files and the Files app are documented in
[`docs/features/storage-and-files.md`](docs/features/storage-and-files.md). The old numbered RAM
file implementation and its callers are retired; Settings, editor, Files and
browser state use named `zlfs` files. Unknown disks are never auto-formatted by
opening an app: `format`/`mkfs` is the explicit destructive route. New zlfs v2
volumes use copy-on-write replacement and dual checksummed directory
generations; version-1 volumes remain mountable through the legacy path.
The implemented 64-bit Ring-3 image/syscall contract and its remaining process
gates are [`docs/architecture/system/user-process-abi.md`](docs/architecture/system/user-process-abi.md).
The automatic process/app -> syscall -> file/network/window -> result/error ->
exit audit contract and physical read-back procedure are
[`docs/architecture/system/automatic-system-audit.md`](docs/architecture/system/automatic-system-audit.md).
The complete performance/storage/process/network implementation receipt,
including every local gate and every still-open physical gate, is
[`docs/evidence/performance-architecture-implementation-2026-08-22.md`](docs/evidence/performance-architecture-implementation-2026-08-22.md).
The internal Intel AX201 Wi-Fi implementation/evidence ladder is
[`docs/drivers/network/ax201-wifi.md`](docs/drivers/network/ax201-wifi.md). Its read-only stage 0 and bounded
API-77 TLV/SHA-256 stage 1 pass 54/54 locally in the isolated checkout. They are
compiled into the current USB image but remain bounded/manual, have not run on
the ThinkPad, do not upload firmware, and are not a connection claim.

> **Latest physical Terminal/keyboard fix (2026-08-24):** the recovered
> ThinkPad boot contains 2,876/2,876 valid ZLLOG records with zero recorder or
> input-queue drops. The internal keyboard was PS/2 IRQ1; no USB HID keyboard
> was present. `diag` executed, but the trace then shows Right, Down and Delete
> key events reaching the app router and being discarded before Terminal; it
> also shows three successful `clear` submissions. Two owners were fixed:
> Terminal now has cursor insertion/deletion, Home/End and bounded Up/Down
> history, and every retained-surface redraw clears its clipped target before
> repainting so Backspace/clear cannot leave old glyph pixels. The permanent
> gate types nine lines through the emulated keyboard and verifies malformed
> command correction, history, results, prompts and pixels. Native UEFI is
> green (clear ink 39,685 -> 21); USB-keyboard-removed PS/2 is green (2,255 ->
> 53). The freshly flashed Imation serial `07B70D07914C6D7E` has clean GPT/FAT,
> matching local/physical EFI hashes and pristine dual-valid 512 MiB ZLLOG GUID
> `d129026f-37b4-4c76-bf83-2468383689a8`. This is locally/VM verified and ready
> for the physical retest; it is not yet a claim about the new image on the
> ThinkPad. Full evidence: [`docs/evidence/physical-input-and-recorder-diagnosis-2026-08-23.md`](docs/evidence/physical-input-and-recorder-diagnosis-2026-08-23.md).

> **Latest physical touchpad result/fix (2026-08-24):** the ThinkPad ZLLOG now
> proves the exact `8086:02e9`/`SYNA8006 06CB:CD8B` I2C stack reached live input,
> moved the zlOS cursor from `(131,334)` to `(620,509)`, and retained physical
> click edges. Transport, decoder and compositor routing are therefore physical
> zlOS successes; tap and two-finger scroll still need explicit confirmation.
> The same trace captured repeated `(3,1)` motion and accumulated `(48,16)`
> jumps: zlOS left the pad in relative report-2 compatibility mode and polled
> one non-zero delta repeatedly. Startup now sends HID feature report 4 value 3
> through the exact `SET_REPORT` packet, refuses to expose legacy mode if that
> switch fails, and gives touchpad motion Settings speed without the separate
> TrackPoint/mouse acceleration curve. `i2c` is now immediate read-only status;
> it no longer resets/re-probes the live bus or blocks the compositor for five
> seconds. The old physical journal is archived losslessly under
> `docs/evidence/exercises/2026-08-24/physical-touchpad-2026-08-24/`. Focused transport/decoder/input
> tests, 16 recorder tests, the broad host compile, four source builds, the
> ten-line native UEFI Terminal gate and full EFI gate are green. The corrected
> 512 MiB-history image is flashed/read back on Imation serial
> `07B70D07914C6D7E`: GPT/FAT clean, stage-0 hash
> `09d6543012006e7137f5a6fe11d33e87a2dbf64fffa5b5fe67687825d50b8ce3`,
> kernel and image ID
> `12117bd7c01613d797bf17212abb88da1795d0f0af78c837bf4d0ccc3cfd62aa`,
> pristine dual-valid ZLLOG GUID
> `346a00c9-3ee0-4534-8174-6b816427057a`. The remaining gate is physical feel,
> tap, scroll and non-blocking `i2c` confirmation. Full evidence and retest:
> [`docs/evidence/physical-input-and-recorder-diagnosis-2026-08-23.md`](docs/evidence/physical-input-and-recorder-diagnosis-2026-08-23.md).

> **Latest physical input/recorder fix (2026-08-23):** the visible desktop boot
> retained 860/860 USB records with zero drops. The first ZLLOG mount was a
> transient post-reset xHCI scan with no connected root port; automatic retry
> later mounted the same stick and the boot reported the journal active. The
> keyboard transport was healthy through PS/2 IRQ1. The Terminal lag owner was
> a full retained-client invalidation for every printable key. Printable input
> now refreshes only the prompt row, `diag` retries before reporting RAM-only,
> all local/EFI/graphical gates pass, and the fixed 512 MiB-history image is on
> Imation serial `07B70D07914C6D7E`. Exact evidence, hashes and the physical
> retest are in
> [`docs/evidence/physical-input-and-recorder-diagnosis-2026-08-23.md`](docs/evidence/physical-input-and-recorder-diagnosis-2026-08-23.md).

> **Latest physical boot boundary (2026-08-22):** USB ZLLOG recovered 719/719
> valid records with zero drops. Network discovery returned (210 -> 211), then
> the physical `8086:0d4f` I219 initializer was entered (212) and never returned
> (213 absent). One earlier xHCI timeout had successful recovery completions and
> was not the terminal fault. The generic 8254x reset path is now blocked for
> this PCH I219 before any MMIO write; milestone 216 records the deliberate
> quarantine and boot may continue. This does **not** claim I219 Ethernet works.
> The next physical gate is a visible desktop plus durable 213, 216, 206 and
> 207 milestones. Details: [`docs/architecture/system/automatic-system-audit.md`](docs/architecture/system/automatic-system-audit.md).
> The fourth image is already on the 3.6 GiB Imation stick: clean GPT/FAT,
> pristine 512 MiB ZLLOG GUID `156e16f3-53e4-48db-a1b6-3dd99cc664b7`, and
> local/USB EFI hashes match. `Boot0002` names ESP PARTUUID
> `42a1ea0f-1f29-4b61-9add-9d5b5ff8b1a1`; one-shot `BootNext: 0002` is armed.

Read this first in a new session. Everything below is verified, not remembered.

> **Current physical speed finding and fix (2026-08-21):** the latest complete
> ThinkPad journal measured the 2560x1440 framebuffer present at about 111.59
> MB/s with an effectively uncacheable mapping; full-screen copies took roughly
> 131-137 ms and dominated input/app work. The current tree retypes the existing
> identity mapping to write-combining after IDT setup and before AP startup, and
> keeps old/new software-cursor patches separate instead of copying their giant
> bounding box. Host tests and the UEFI QEMU cache-transition gate pass. The
> physical rerun is now complete: the live mapping reports write-combining,
> regression measures about 7,089 MB/s (63.5 times the old slope), and seven
> full-screen presents have a 2.070 ms median. Cursor-only input-to-present has
> a 7.827 ms median. Present is no longer the late-frame owner; repeated large
> compositor/draw work is now the measured target. Full
> evidence and limits: [`docs/current-speed-and-quality-diagnosis.md`](docs/current-speed-and-quality-diagnosis.md).
> The Imation stick serial `07B70D07914C6D7E` is already flashed with this
> build and a 512 MiB ZLLOG partition. Its GPT and FAT filesystem verify clean,
> both journal superblocks validate, and both EFI files match the build by
> SHA-256. The completed physical boot is retained in slot 0 with 6,503 valid
> records and zero drops.
> The physical phase build has now completed too: 5,967 valid records, 1,043
> phase samples and zero drops. Large 3.2-3.5 Mpixel damage regions that visit
> 12-13 windows spend about 130 ms in C chrome, 23-28 ms in zl app drawing,
> 9-13 ms restoring the cached desktop, and about 3 ms presenting. That trace
> selected retained client and shell surfaces. They are now implemented with
> bounded disjoint damage/occlusion, direct-render fallback, deadline/input
> work and a real-kernel four-core band oracle. The implementation contract
> remains [`docs/plans/retained-window-surfaces.md`](docs/plans/retained-window-surfaces.md);
> the target claim is still blocked on a fresh physical before/after journal.
> The broader, evidence-ranked execution order -- retained client **and** shell
> surfaces, damage/occlusion, deadline pacing, async storage, processes, real
> networking, then GPU composition -- is
> [`docs/plans/performance-architecture-roadmap.md`](docs/plans/performance-architecture-roadmap.md).

> **Final local execution evidence (2026-08-22):** BIOS/GRUB and native
> UEFI/GOP exercises each pass 32/32 workflows. `tools/checks/verify-efi.sh`
> passes normal
> and hostile 64-bit Ring 3, stage-0 persistence, WC cache transition and the
> forced fallback. `tools/probes/probe-smp.py` wakes 4/4 cores and finds 0
> differences in
> 175,224 serial/banded pixels. Virtio and e1000 networking each pass 18/18
> with DHCP, 20/20 ping, DNS and zero driver errors. These are VM/software
> receipts, not ThinkPad panel, I219 carrier or physical USB proof.
> `fstest` also cuts a v2 file replacement after every block write and
> cold-mounts exactly the old or new bytes; corrupting the newest directory
> generation falls back to the older complete generation.
> `tools/checks/verify-efi.sh` now also switches two independent Ring-3
> PML4/kernel-stack
> contexts across cooperative yield (`AB12`). PIT interrupts also preempt two
> non-yielding infinite-loop images and advance both (`PQ`), and a `#GP` in one
> process does not stop its sibling. Bounded PID IPC exchanges `hi`/`ok` across
> those address spaces and verifies sender IDs as `h1o2`. A persistent desktop
> spawn/reap service and the zl interpreter remain. The first owner-bound window/input ABI now opens a
> real WM client, presents bounded text, polls input `W`, closes, and leaves no
> leaked owner; pixel/configure/clipboard depth remains.
> TCP now fast-retransmits the first unacknowledged segment after three
> qualifying duplicate ACKs, without treating data ACKs or window changes as
> loss. Plain HTTP/1.0 can reuse an idle, fully acknowledged same-peer socket
> only after an explicit length-delimited `Connection: keep-alive` response.
> xHCI CDC-ECM now passes the full 18/18 DHCP, 20/20 ICMP and DNS gate in QEMU
> while USB storage, keyboard and tablet remain attached. Its bulk rings and
> four-buffer RX ownership are real driver code; physical tether/adapter proof
> is still open and is not inferred from QEMU.

> **Cloning the desktop mockup:**
> [`../docs/design/ds-clone.md`](../docs/design/ds-clone.md) is the one page for
> the ds-reference.html clone — the palette chain, the motion curves, the
> fidelity oracle, and an explicit list of what is APPROXIMATED rather than
> reproduced. Its companion
> [`docs/reference/ui/widgets.md`](docs/reference/ui/widgets.md) is the measured
> widget specification: 68 records, 178 line citations into the reference, and
> an index of the 18 places the reference contradicts itself. Read both before
> adding an app or a widget.
>
> Two facts from it that change what is possible, and are easy to waste hours
> rediscovering: the reference **cannot render without a shim** (its runtime was
> never delivered — `kernel/tests/refrender/` reimplements it), and **every game
> canvas in it is blank**, along with Renderer, Framebuffer, Console, Font Atlas
> and Image Viewer, because seven more of its modules are missing too. For
> those, only the shell can be cloned.

> **Workspaces are real as of 2026-08-20:**
> [`docs/desktop/workspaces.md`](docs/desktop/workspaces.md) — what filters (the paint walk,
> `wm_at`, the modal and focus walks, both taskbar loops), what deliberately
> does not (damage, the dock's running-accent), and why `wm_raise`/`wm_focus`
> PULL a window to the current workspace instead of refusing. Read it before
> touching `src/graphics/windowing/wm.c`'s z-order or `src/kernel.zl`'s island.
> It also records the trap
> that shapes the per-app oracle: **serial bytes are routed to the focus
> window**, so on a workspace with no windows the serial console is dead and
> you cannot switch to an empty workspace and then type a command to fill it.

> **Visual system status:** [`docs/visual-speed-northstar.md`](docs/visual-speed-northstar.md)
> records the isolated 2026-08-19 implementation pass, its host/QEMU evidence,
> and the exact target-hardware proof that remains. Keep visual polish and real
> application completeness as separate claims.

> **Current open work is not owned by this historical narrative.** Use
> [`../docs/REMAINING-WORK.md`](../docs/REMAINING-WORK.md) for the immediate
> sequence and [`../docs/program/README.md`](../docs/program/README.md) for the
> complete program. The
> [`2026-08-19 status audit`](../docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md)
> remains useful dated revalidation evidence, not the current queue.
> This file remains authoritative only for the measured hardware events it
> records. Its older narrative is **stale on five checkable points**, each
> corrected in that audit with a command: it
> says nothing in the kernel arms `lt_armed`
> (`src/drivers/display/intel.c:4232` does, reachable from
> `src/kernel.zl:1395`); it says there is no VBT parser (there is, and
> `intel_bringup_panel` calls `intel_vbt_find()`);
> it says `src/graphics/framebuffer/fb.c` has no clipping (lines 763-798 are the
> scissor, with four callers); it describes a boot fork `src/kernel.zl` no
> longer has; and it carries a
> northstar percentage its own source retracted.

## What this is

An OS written in `zl` (Roy's own language) at `~/Documents/repos/zl-linux/kernel/`.
Boots three ways with no GRUB: BIOS multiboot, our own 512-byte bootloader
(`raw_boot.asm`), and as a native UEFI application (`efi.c`).

All three entry families now converge on the versioned, sealed record described
in [`docs/architecture/boot/typed-boot-handover.md`](docs/architecture/boot/typed-boot-handover.md) before the ZL
kernel starts. Exact artifact/origin identity and automatic recovery selection
remain explicitly unclaimed until the next loader slice supplies them.

**The point of the project is the Intel display driver — the DPLL and a
cold-start modeset.** The laptop is a test PC. Optimise for that.

Where the firmware boundary actually sits, what a BIOS does that this kernel
already does for itself, and the two walls (an Intel DRAM-training blob, and
Boot Guard fused *on* on this laptop — measured): [`docs/concepts/what-is-a-bios.md`](docs/concepts/what-is-a-bios.md).

## The USB boot flight recorder works on the physical ThinkPad

The implementation and safety contract are in
[`docs/architecture/boot/persistent-boot-observer.md`](docs/architecture/boot/persistent-boot-observer.md); the
whole-runtime event/counter/sampling/burst/laboratory design is in
[`docs/architecture/system/always-on-telemetry.md`](docs/architecture/system/always-on-telemetry.md); the
typed lifecycle/operation/result layer is in
[`docs/architecture/system/automatic-system-audit.md`](docs/architecture/system/automatic-system-audit.md); the
priority order around it is in
[`../docs/EXECUTION-ROADMAP.md`](../docs/EXECUTION-ROADMAP.md). This is the
verified current state:

- `mkusb.sh` builds a 132 MiB image by default: a 62 MiB FAT32 `zlOS EFI`
  partition, a gap, then a bounded 64 MiB raw `ZLLOG` partition. `--log-mb`
  selects 64, 128, 256 or 512 MiB when more retained history is wanted. Journal
  initialization hashes the ESP before and after and fails if it changed. On a
  larger real device the same script first clears the final 1 MiB so a stale
  backup GPT from an older image cannot disagree with the new primary, moves
  the new backup GPT to the physical end, reinitializes only `ZLLOG` with the
  real device capacity, verifies the GPT and inspects the result; otherwise the
  image-sized superblock would be correctly refused.
- Firmware no longer jumps straight into the 2.7 MiB kernel. A canonical 20 KiB
  `EFI/BOOT/BOOTX64.EFI` witness prints immediately, replaces a bounded
  `EFI/ZLOS/WITNESS.LOG`, and chainloads the real `EFI/ZLOS/ZLOS.EFI`. This
  records exact pre-kernel UEFI status without any zlOS command. Both images
  are base-zero, 4 KiB-aligned EFI applications with relocations and no DLL
  flag; FAT now records the real LBA-2048 partition offset.
- xHCI mass storage has bounded caller-buffer `READ(10)` and `WRITE(10)`, CSW
  tag/status/residue validation, fixed- and descriptor-format `REQUEST SENSE`,
  `SYNCHRONIZE CACHE(10)` and one bounded Bulk-Only reset recovery. The fixed
  4 KiB staging window and discovered device capacity bound every request.
- `src/core/zllog.c` retains 4,096 fixed 64-byte records before storage exists,
  plus a
  separate reusable 64-cell interrupt/fault emergency lane. It tees the boot
  transcript only through `system ready`, redacts printable key identity, keeps
  button/wheel edges exact, samples ordinary pointer events/batches one in sixteen, records
  representative late painted frames and one healthy painted frame in sixty,
  and splits a frame into input, app tick, compositor, vblank-wait and present
  time. It also correlates input sequence/time, damage pixels, presented bytes,
  queue depth and missed deadlines.
- The recorder writes only after one GPT entry passes the custom type GUID,
  exact `ZLLOG` label, GPT CRCs, a supported 64..512 MiB range, partition unique GUID and
  CRC-valid journal-superblock checks. Every write is checked against the
  selected range again. The image hash is recorded for identification; the
  running kernel does not recompute `ZLOS.EFI`'s hash.
- Two CRC-protected 4 KiB superblocks publish 31, 63, 127 or 255 fixed 2 MiB
  journal slots according to the selected partition size. Records, slot
  checkpoints and alternate-superblock publication are
  ordered with explicit cache synchronization. Incomplete slots are recoverable
  after a torn metadata write; a clean exit marks the slot complete. A full
  segment is sealed and capture continues in the next one instead of stopping.
- `diag` reports active versus RAM-only mode, buffered/dropped records and the
  last error. `diag save` explicitly checkpoints and refuses cleanly if no
  exact target was mounted. A failed initial mount now retries automatically
  with bounded backoff, and a successful retry drains the RAM records without
  a command. The normal idle path checkpoints at most once per ten seconds,
  from normal context rather than an IRQ.
- `tools/zllog.py` accepts a whole GPT image/device or a raw journal partition,
  validates it without mounting, and reads or exports text, JSON and CSV. A
  storage refusal is decoded into the reason, MSC stage/name, port, slot and
  xHCI completion code instead of leaving only packed integers. A paired
  bounded record retains PORTSC, USBSTS and USBCMD, so a transient failure
  which later recovers still leaves the controller state that caused it.
  Sampled/late frames also retain damage-rectangle count and exact pixel area
  alongside the existing five phase timings.

Current host evidence, rerun 2026-08-22: `tools/test_zllog.py` is **15/15**;
`tests/host/zllog_e2e_test.py` is **5/5** across real shipping-writer containment,
rotation, mismatched identity, torn super/slot metadata and automatic recovery
from a transient MSC initialization failure; a fresh standalone
`msctest.c` build reports **0 failures** across command encoding, range bounds,
CSW/sense decoding and the complete 4 KiB staging copy. This proves the format,
writer state machine and xHCI command boundary.

The expanded always-on telemetry also has a current writable disposable
OVMF/xHCI proof: one COMPLETE boot, 351/351 valid records, global sequence
1..351, no extractor warnings and zero drops. It contains 46 command submits
and 46 completions; the final shell `quit` is records 335/336. USB pointer
press/release records 301/307 retain buttons 1/0 and input sequences 1/4.
Five frame-extension records carry nonzero input sequence and
input-to-present time. Display records decode both QEMU modes as uncacheable;
printable key identity is redacted and desktop command text is absent. Three
xHCI MMIO timeouts retain six decoded before/trigger snapshots. Exact
region comparisons kept the first MiB, the 63..64 MiB gap and the 128..132 MiB
tail byte-identical, while extracted BOOTX64.EFI and ZLOS.EFI also stayed
identical. This is QEMU/KVM evidence, not a ThinkPad latency measurement.

Physical v6 proof, 2026-08-21: the exact Imation stick mounted itself without
any zlOS command and retained a power-cut boot. Superblock B advanced to
generation 2 and points at slot 0; the extractor recovered **1,036/1,036
CRC-valid records, sequence 1..1036, zero recorder drops, zero input drops and
zero error/fatal records**. Record 214 is `STORAGE_READY` for LBA 131072 +
131072 at 512-byte blocks. The slot is correctly `WRITING`, not `COMPLETE`,
because the laptop was turned off rather than halted through zlOS. This is the
first physical proof of the whole chain: native UEFI kernel -> Intel xHCI ->
USB BOT/SCSI -> exact GPT/GUID/superblock validation -> durable ZLLOG writes.

The same run exposed one presentation defect in the diagnostics, not in the
journal. The first early scan refused before the later bounded retry mounted;
the boot line and `ZlBootDiag` NVRAM variable kept that first answer and said
RAM-only even while the raw journal was receiving records. The current tree
retries once after the final USB reporting pass and publishes a new
`storage-ready` NVRAM state only after the first durable checkpoint succeeds.
The original refusal remains in the journal, which preserves the useful
history without presenting it as the final state.

An earlier physical ThinkPad run left ZLLOG pristine even though the EFI
witness reached the native desktop handoff, but the separate firmware-variable
fallback worked on hardware. Its checksum-valid v1 record reported
`storage-refused / msc-init` while xHCI was running. The retained device was
port 10 `8087:0026` (Intel Bluetooth), not the Imation on Linux root port 4;
this proved the old global deepest-stage field was overwritten by a later
non-storage scan. Result 2 is `not-ready` and completion `0xffffffff` is the
not-attempted sentinel, so no Bulk-Only command had run.

The physical diagnostic image recorded a bounded v2 table for every attempted root port,
including separate enumeration and MSC stages/completions, USB identity,
candidate flag and raw PORTSC. A host regression models Imation port 4 and
Bluetooth port 10 and proves their boundaries cannot overwrite one another.
`tools/zlbootdiag.py` reads v1 through the current v4 format;
its suite is **5/5**. OVMF records `EFI_DIAG_ARM status=0` before exit.
`tests/host/efi_runtime_diag_test.py` additionally forces a wrong GPT label and
proves the post-exit current-format replacement and per-port table survive in OVMF's
variable store.

That v2 table has now isolated the physical root cause one layer further.
Imation `0718:067d` is port 4, high-speed ID 3, slot 1 and its attempted
configuration-header request completed with xHCI code 4 (`USB Transaction
Error`). Camera, fingerprint and Bluetooth independently failed at the same
EP0 boundary/code. Their first device-descriptor requests had succeeded, so
the disk-specific MSC/SCSI/GPT paths were never reached.

The publication order was one real defect, but the next physical run proved it
was not the whole root cause. `xhci_control_in()` now builds Setup with the
opposite cycle, writes the complete transfer, then flips only Setup's ownership
bit before the doorbell. The host regression proves normal and Link-TRB wraps
remain hidden until that commit. The first replacement image still stopped at
the configuration header: Imation timed out, while camera, fingerprint and
Bluetooth returned code 4 on their Setup TRBs. ZLLOG remained pristine. Do not
claim that atomic publication alone fixed physical enumeration.

Additional pre-rerun hardening is complete: EP0 and bulk completions match the
exact current TRB rather than any event on the same endpoint; PCIe doorbells
are flushed with a BAR readback; and the native EFI gate now enumerates xHCI
storage, keyboard and mouse together. `ZLDIAG3` retains the last bulk completion,
SCSI opcode, CSW status/residue, reset-recovery result, sense triplet and exact
EP0 Setup/Data/Status failure stage. The reader still accepts the physical v1
and v2 records. The three-device normal boot and forced-GPT fallback both pass.

The physical v3 result exposed three more concrete gaps. The driver used cached
spin counts as timeouts, while five million polls can expire in milliseconds;
it used only a compiler barrier where Linux uses a DMA write barrier; and it
made one descriptor attempt where Linux deliberately makes three. Control and
bulk waits now use PIT-backed real-time deadlines, EP0 publication and Event
TRB consumption use `sfence`/`lfence`, descriptors get three bounded attempts,
transaction errors use Reset Endpoint, and a true timeout uses Stop Endpoint
before replacing the dequeue ring. The stable v3 port entry now also carries
attempt count and recovery outcome. A host controller regression proves error
on the first Setup, reset + dequeue replacement, and success on attempt two.
All 32/64/EFI, exact-console, multi-device OVMF, xHCI, MSC and sanitizer gates
are green. Physical ZLLOG mount remains the required proof.

The resulting physical rerun did not mount ZLLOG, but it decisively narrowed
the failure. All four devices reached their successful Device descriptor, then
made all three Configuration-header attempts; every final event was cc4 on
Setup and every Reset Endpoint plus Set TR Dequeue recovery completed. The
Imation still reached no SCSI/BOT operation. Therefore retry count, endpoint
recovery and the old spin timeout are not the complete cause. The common EP0
ring/dequeue transition is now the highest-value boundary.

`ZLDIAG4` is built and host-verified for the next run. Its 104-byte per-port
entry preserves the raw three-TRB control TD, all four Transfer Event dwords,
pre-recovery output EP0 state/dequeue/DCS, software producer state, context
size, attempt number and first destination dword. The first failed port also
runs a bounded four-case request matrix whose byte completion codes distinguish
repeat-Device, alternate buffer/length, Configuration and 100 ms delayed
Configuration requests. The failure snapshot is copied before ring recovery
can erase it and before a later port can overwrite it. The reader remains
backward compatible with the real v1-v3 records; its suite is now **5/5**.

The adversarial host case now models Device on entries 0-2 followed by Config
Setup cc4 on entry 3 with EP0 Halted, then proves the trace survives Reset
Endpoint/ring replacement. Fresh evidence: `xhcitest` and `msctest` zero
failures, xHCI ASan/UBSan clean, parser 5/5, ZLLOG 9/9 plus E2E 3/3, all
32/64/EFI builds, exact BIOS transcript, kernel witness structure and native
multi-device OVMF/forced-refusal EFI gate green. The weakest link remains the
unrun physical v4 image; none of those host results says which request in the
matrix the Intel controller will accept.

The v4 image is now on the exact stable Imation by-id device and armed as
`BootNext=0002`. Independent post-write checks report a clean device-sized GPT,
clean FAT, unmounted partitions, pristine journal generations 1/0 and no boot
slots. Built and USB `ZLOS.EFI` both hash to
`053ac557706116a4419657977331848243b7f006bdb6162ebc135ff8cc3ce729`; the
entire 62 MiB ESP hashes identically in the image and on USB. ZLLOG unique GUID
is `a50b6779-e312-4f0e-8399-a3504b2b51a5` and the firmware entry names current
ESP PARTUUID `012ee330-58d4-448a-8f0a-c1014986dc56`. This proves prepared
media, not execution on Intel; the next ordinary restart is the physical gate.

That physical v4 gate has now run. The EFI variable is checksum-valid and the
USB journal is still pristine at superblock generations 1/0, so no SCSI/BOT
command was reached. Imation, camera, fingerprint and Bluetooth all completed
their Device descriptor and then failed all three Configuration-header
attempts with cc4 on the exact Setup TRB, residual 8 and output EP0 Halted at
that same dequeue pointer. Imation's post-failure matrix is `4/4/4/4`: repeat
Device, alternate buffer/length, Configuration, and delayed Configuration all
fail after the first Config error. This proves Reset Endpoint plus Set TR
Dequeue did not restore usable EP0; it does **not** prove those requests fail
on a pristine endpoint. Full-speed devices completed Device8 and Device18
before Config failed, so this is not simply “the second TD on the ring.”

`ZLDIAG5` closes that ambiguity and fixes the strongest standards gaps before
another physical run. `Address Device` now gets the 10 ms SET_ADDRESS recovery
interval used by Linux. Intel reset now waits for initial CNR, requires
HCHalted, clears RS/INTE/HSEE and performs the required 1 ms no-register-access
period after HCRST. IN Data TRBs set ISP, and an intermediate short Data event
can no longer complete a control request: only the matching Status success can.
The bounded global probe records the literal first successful Device request
and one immediate identical request before Config, including both full TDs,
events, EP0 state/dequeue, slot address/state and recovery result. The header
also records the firmware legacy-ownership registers and controller reset
states. Per-port v4 traces remain intact, and the reader accepts v1 through v5.

Host proof now includes an exact first-success/second-Setup-failure sequence,
a short-Data-then-Status sequence, trace survival across destructive recovery,
and a latch proving later ports cannot replace the first pair. Parser coverage
is **7/7**; `xhcitest`, its ASan/UBSan build and `msctest` report zero failures;
32-bit, 64-bit, EFI, exact-console, native multi-device OVMF and forced-runtime-
refusal gates are green. Physical v5 execution is still the remaining proof.

The verified v5 image is now flashed to the exact stable Imation by-id target
and armed as `BootNext=0002`. Independent read-back—not the flash script—shows
a clean device-sized GPT, no mounted USB partitions, valid pristine ZLLOG
superblocks at generations 1/0, and exact image/USB ESP SHA-256
`a5c0bcb6df9863b67bd57561617a46c9fc83b57643574b43290662820c01bec2`.
Built and USB `ZLOS.EFI` both hash to
`95bc371ae529dc13988caa51a29c0f9b99e22a38c8243139c26778f1d0e62531`.
ZLLOG's unique GUID is `6f8be994-dd1b-4912-9216-96204b904a99`; firmware
entry `Boot0002` names current ESP PARTUUID
`a9ef9679-e22c-4f1b-8e4e-0dd41b7a4d0a`. This is prepared-media proof only;
the next ordinary restart is the physical v5 execution gate.

That physical v5 gate has now run. ZLLOG remained pristine, but the
checksum-valid fallback closed the generic-ring hypotheses: Imation's first
Device18 request and its immediate identical twin both completed through their
Status TRBs on one live EP0 ring, with software enqueue advancing 3 -> 6.
The output Slot context remained Addressed at USB address 1. Camera,
fingerprint and Bluetooth also completed Device descriptors, then every device
failed Configuration-header Setup with cc4/residual 8. Firmware ownership was
cooperative and the controller reset states were valid. Therefore this is not
"the second TD", a fixed ring index, a lost address, malformed Config bytes,
or legacy SMM ownership. The v5 port trace retained attempt three after two
ring replacements, so it did not preserve the first clean Config attempt.

`ZLDIAG6` now preserves that missing first attempt before descriptor retries or
Reset Endpoint can overwrite it: the complete three-TRB TD, Transfer Event,
pre-recovery EP0 context, Slot address/state, individual Reset Endpoint and Set
TR Dequeue completion codes, and post-recovery EP0 context. The bounded
firmware header is 440 bytes and the worst-case variable is 3,668 bytes, kept
under a compile-time 4 KiB ceiling; the reader remains compatible with v1-v5.
The storage scan no longer calls a cached failed slot a retry. It explicitly
disables and forgets the slot, resets the port, allocates/addresses a fresh
device and retries Config. If that fails, one final bounded compatibility path
does an Address Device BSR=1 Device-descriptor preflight at USB address zero,
then resets and performs normal enumeration. Configured keyboard and pointer
ports are never reset by the storage rescan. Control waits are now one second,
and PCIe doorbells are flushed through readable USBSTS rather than reading the
write-only doorbell register.

Host regressions reproduce Device success followed by three Config failures,
prove the first event at the live-ring transition survives every destructive
recovery, retain both clean whole-enumeration outcomes, verify old-slot
invalidation and the BSR command bit, and reject later overwrites. Parser v6 is
8/8; `xhcitest`, its ASan/UBSan build, `msctest`, `inputtest` and `wmtest` are
green. The 32-bit, 64-bit and EFI builds, exact BIOS transcript, native
multi-device OVMF boot, forced-runtime-refusal EFI gate, journal 9/9 and torn-
write E2E 3/3 are also green.

The verified v6 image is now on the exact stable Imation by-id device and
`BootNext=0002` is armed. `sgdisk --verify` reports no problems; neither USB
partition is mounted; both journal superblocks are pristine and record the
real 7,570,752-block capacity. Built and USB `ZLOS.EFI` both hash to
`ea3bf7ea2b57788f26556da798dcd73aaaafa4d4fde1e3e7e4e58d6331341ada`;
stage zero hashes match at
`419278d402a8735dd9303fed3a3da5641d99fa7a7f12ab2521e661b7c21b4c35`,
and the complete ESP hashes match at
`a7abe4bd6ef8dd6a0c33b6508f576c7a7ba975932bc1767a59de03b5e1bda245`.
ZLLOG unique GUID is `fdd4994b-eb60-4755-ae47-2e745a698e11`; firmware entry
`Boot0002` names current ESP PARTUUID
`bac50c60-fb6a-403c-86d7-01dd31b52ef3`. This proves prepared media and
one-shot firmware routing. Only the ordinary restart can prove which clean
enumeration scheme the physical Intel controller accepts.

The preceding v3 replacement was flashed to the exact Imation stick and independently
read back on 2026-08-20. The device-sized GPT verifies clean, both pristine
ZLLOG superblocks validate, and the USB `ZLOS.EFI` SHA-256 is
`22ce18af7f6e61e58a7f52a6beb0fcda71ec8b4d010c422b5f4cfd77ff4044fd`,
identical to the build. `BootNext=0002` is bound to regenerated ESP GUID
`8be82f3d-3b75-4965-a72c-036f787f11b1`; the device is synced and unmounted.
This is prepared-media proof, not the still-pending Intel rerun.

The integration proof is real too: a fresh native OVMF UEFI64 image booted
twice over xHCI storage with `ZLLOG` active. After two clean boots the host
extractor found **407 globally contiguous CRC-valid records (`1..407`)**, zero
drops and zero warnings, including USB-tablet/button input, typed input and the
five frame phases. Exact region hashes showed bytes `0..64 MiB` and
`128..132 MiB` identical; only the `64..128 MiB` `ZLLOG` partition changed.
`tools/checks/verify-efi.sh` stayed green. This still does **not** prove that a
physical USB
controller/stick accepts and flushes the writes, or that the ThinkPad boot is
now fast.

Exact image and extraction loop, from the repository root:

```sh
cd kernel
./tools/images/mkusb.sh
./tools/checks/verify-efi.sh
cd ..
./tools/zllog.py inspect kernel/zlOS-usb.img
./tools/zllog.py read kernel/zlOS-usb.img --latest
./tools/zllog.py export kernel/zlOS-usb.img --all \
  --json /tmp/zllog.json --csv /tmp/zllog.csv --text /tmp/zllog.txt
python3 tools/test_zllog.py
python3 kernel/tests/host/zllog_e2e_test.py
```

The next manual step is the evidence that cannot be manufactured on the host:

```sh
cd kernel
./tools/images/mkusb.sh --boot-next /dev/sdX        # destructive; inspect the prompt's target
```

`mkusb.sh` automatically relocates the backup GPT, stamps the journal with the
physical device's actual capacity and prints the final `zllog.py inspect`
result. `--boot-next` also removes only stale exact-label `zlOS USB` firmware
entries, creates one bound to the newly generated partition GUID, verifies that
GUID, and sets a one-shot BootNext. Do not boot unless those checks succeed.
Then restart normally with the stick attached; no F12 choice or zlOS command is
required. Leave the desktop up for at least twenty seconds so automatic retry
and checkpointing can run, then shut down. Back in Linux, do not mount or
initialize the journal again:

```sh
cd /home/roy/Documents/repos/zl-linux
sudo ./tools/zllog.py inspect /dev/sdX
sudo ./tools/zllog.py read /dev/sdX --latest
sudo ./tools/zllog.py export /dev/sdX --latest \
  --json /tmp/thinkpad-zllog.json --text /tmp/thinkpad-zllog.txt
```

Until that transcript exists, physical ThinkPad/USB persistence and bare-metal
latency remain explicitly unverified.

## The development loop that matters

`kernel/tests/host/` compiles **the same `src/drivers/display/intel.c` that ships
in the kernel** as a
Linux program against the real GPU's PCI BAR. Seconds per iteration instead of
write-USB → reboot → read-screen.

```
cd kernel/tests/host
./gpu-dev.sh probe          # read everything (safe, i915 keeps running)
./gpu-dev.sh dump a.txt     # 300 registers
./gpu-dev.sh diff a.txt b.txt
sudo ./dpll_test 2 148500   # program an UNUSED DPLL and watch it lock
```

Map **8 MiB** of the BAR, not 16 — the kernel refuses the full BAR while i915
holds it. All display registers are under 1 MiB.

## Verified working on the real hardware (Comet Lake 8086:9B41)

- Panel: **2560×1440 on the eDP transcoder**, 2720×1481 total, X-tiled, stride 10240
- **DPLL programming PROVEN**: 720p/1080p/1440p/4K60 all computed, programmed,
  and **locked**, with the hardware restored exactly afterwards
- **AUX/DPCD works**: panel is DPCD 1.2, max 2.7 Gbps HBR, 4 lanes, **no rate table**
- Link training register offsets confirmed against the live trained link
- Panel power delays read from firmware: T1+T3 200 ms, **T10 50 ms**, **T11+T12 500 ms**
  (this line used to say "T9 50 ms" — wrong. `intel_pp_t9()` and `intel_pp_t10()`
  read each other's field: PP_OFF_DELAYS 28:16 is T10, 12:0 is T9. The 50 ms is
  T10. The real T9 is 260 ms and lives in VBT, not in this register.)
- DDI buffer translation table read off this machine (matches `skl_u_ddi_translations_edp`)

## Gen9 register traps — all of these cost real debugging time

| Trap | Truth |
|---|---|
| eDP TRANSCONF | **0x7F008**, not 0x6F008 (which is HSYNC and reads plausible) |
| eDP timings | **0x6F000**, not transcoder A at 0x60000 |
| PLANE_STRIDE units | 64 B linear, **512 B X-tiled**, 128 B Y-tiled |
| PP_CONTROL unlock key | **none on Gen9** — 0xABCD is pre-DDI only |
| Panel cycle delay | **PP_CONTROL[8:4]**; PP_DIVISOR is dead on CNP/CMP |
| Watermark fields | **lines 18:14, blocks 9:0** (narrow) |
| DP_TP_STATUS | **does not exist on DDI A** |
| DPLL enable regs | 0/1 = 0x46010/0x46014, **2/3 = 0x46040/0x46060** — not an array |
| DPLL0 | **feeds CDCLK. Never disable it.** |
| DPLL CFGCR | indexed from **DPLL1**, not DPLL0 (DPLL0 has none) |
| DPLL lock time | PRM says 5 ms; **measured ~80 ms** on a cold WRPLL |
| DP intermediate rates | 2.16/3.24/4.32 Gbps are **eDP-only**, need a DPCD rate table |

Retained research record with 13 resolved source conflicts:
`kernel/docs/evidence/display/gen9-modeset-plan.json`

## Stage 1 of that plan is DONE — every conflict settled on the real panel

`tests/host/modeset_test.c --survey` is read-only and runs with i915 up. **21 passed,
0 failed** (2026-08-17 — 18 for the four conflicts below, 3 more for the pixel
clock). It settles by measurement what the sources disagreed on:

| Was contested | Settled |
|---|---|
| C1 TRANSCONF | **0x7F008** reads `C0000000` (b31+b30). 0x6F008 decodes as hsync 2608..2640 — it really is TRANS_HSYNC |
| C2 PIPE_SRCSZ | **0x6001C** reads `09FF059F` = 2560×1440. 0x6F01C reads 0 |
| C5 unlock key | PP_CONTROL top half is `0000` — **no key**, as the plan said |
| C7 cycle delay | PP_CONTROL[8:4] = 6 → **T12 500 ms**. PP_DIVISOR ref divider 0, which the PRM forbids ⇒ dead register |

**The buf-trans table is `skl_u_trans_edp`, 10/10 exact.** That was the one fact
"only in VBT and not discoverable from any register" — it is discoverable, by
reading back what firmware programmed. The board is low-vswing eDP, **max vswing
level 3**, I_boost 0, and firmware has already set the balance-leg disable bit
(`DISPIO_CR_TX_BMU_CR0 = 08800000`) exactly as the plan's step 32 prescribes.

Also captured: **`saved_port_bits = 0x00000010`** (DDI_A_4_LANES set, port
reversal clear), CDCLK 337500 kHz, DPLL0 locked at rate_idx 1 (HBR).
`TRANS_DDI_FUNC_CTL = 82010006` — the plan predicted `0x82000006` + PHSYNC. Match.

**PSR is ON** (`EDP_PSR_CTL = 81F00406`). It must be cleared in Phase B step 4
before anything else, and it is why the frame counter reads `0 -> 0 = 0.0 Hz`
when the screen is idle.

## The pixel clock is MEASURED now: 241,690 kHz, 59.998 Hz

It never had been, and every bandwidth, watermark and link-rate number rested on
an assumed 60 Hz. It did **not** need PSR disabled. A DP link runs at a fixed
symbol rate, so the transcoder holds a ratio reconciling it with the pixel clock,
and `PIPE_LINK_M1/N1` *is* `pixel_clock : link_clock`. Read-only, exact, no
timer, correct while PSR is on.

Four independent sources agree, three of them registers this driver has to get
right anyway:

| Source | Says |
|---|---|
| `PIPE_LINK_M1/N1` = 0x72943 / 0x80000 × 270000 kHz | 241,690 kHz |
| `PIPE_DATA_M1/N1` = 0x7E55EF29 / 0x800000, via bpp and lanes | 241,690 kHz (delta 0) |
| Panel EDID detailed timing descriptor (`0x5E69` × 10 kHz) | 241,690 kHz |
| Frame counter, *with the screen busy* | 60.0 Hz |

So the assumed 60 Hz was right to 0.004% and **plan step 26 holds: 4 lanes @ HBR
is the only working point** (5,800,560 kbps needed; 4×RBR gives 5,184,000).
Confirmed with a real number rather than inherited from an assumption.

Panel is an **LG LP140QH2-SPD**, 309×174 mm, hsync **positive**, vsync negative —
which is exactly what `TRANS_DDI_FUNC_CTL = 0x82010006` encodes (b16 set, b17
clear). Another cross-check that landed.

**Do not trust the frame counter for this.** It is not reliably zero, it is
*intermittently* zero — 0.0 Hz idle, a correct 60.0 Hz with a terminal scrolling.
It passes in testing and returns 0 in the field.
`intel_pixel_clock_khz()` uses M/N first and keeps the counter only as the
fallback for a path with no M/N, which on this hardware means HDMI.

## M/N is implemented and verified bit-for-bit against firmware

Plan step 45 had **no implementation at all** — not a defective write path, an
absent one, and a hard blocker for Phase H. `intel_mn_compute()` /
`intel_mn_program()` now exist, and because firmware has already solved the same
problem for the same mode, the computation has a known-correct answer to be
checked against:

```
inputs: pixel 241690 kHz, link 270000 kHz symbol, 4 lanes, 24 bpp
  DATA_M1 (with TU)  firmware 7E55EF29   ours 7E55EF29   MATCH
  DATA_N1            firmware 00800000   ours 00800000   MATCH
  LINK_M1            firmware 00072943   ours 00072943   MATCH
  LINK_N1            firmware 00080000   ours 00080000   MATCH
```

M is **truncated**, not rounded — rounding misses. N is rounded *up* to a power
of two then capped (0x800000 data, 0x80000 link). `--survey` is 21/21 now.

## The rest of the pipe path — also absent, also now verified

Steps 46, 49–54 had **no code at all**. Each is a couple of lines and each one
wrong is a black screen with no error bit. Same method: compute, compare to what
firmware left. `./gpu-dev.sh probe` checks all of these every run.

```
TRANS_DDI_FUNC_CTL  firmware 82010006   ours 82010006   MATCH
TRANS_MSA_MISC      firmware 00000021   ours 00000021   MATCH
PIPE_MISC           firmware 00000000   ours 00000000   MATCH
WM_LINETIME         firmware 0000005B   ours 0000005B   MATCH
```

Three things the hardware corrected in the plan:

| Plan says | Hardware says |
|---|---|
| `WM_LINETIME` → **90** | **91** (`0x5B`). 2720×8000/241690 = 90.03 and i915 uses DIV_ROUND_**UP** — the plan truncated |
| cursor DDB 0..7, plane 8..891 | plane **0..858**, cursor **859..891** — inverted, and exactly fills 0..891 |
| cursor watermark 8 blocks | **13** blocks (`CUR_WM(0) = 8000000D`) — the plan's 8 is one short of what firmware asks |

Also settles plan uncertainty **#12**: `PLANE_WM` bit 30 (IGNORE_LINES) is **0**
in all eight of firmware's levels, so writing 0 is right. Firmware's real
per-level plane watermarks are 41/74/91/99/157/182/195/222 blocks — the plan
guessed "~21 blocks" for level 0 and suggested 256 as a safe margin; the real
number is 41 and 256 would have been a 6× over-allocation.

**C9 is still NOT settled.** Firmware's largest values (222 blocks, 11 lines) fit
inside *both* the narrow 18:14/9:0 and the wide 26:14/11:0 encodings, so this
data cannot distinguish them. Still using narrow, still on the plan's authority.

## The backlight was writing to the wrong registers

Two layouts exist and this code had the other one:

```
SKL / SPT   0xC8254 packs both: freq 31:16, duty 15:0
CNP / CMP   0xC8254 = freq, all 32 bits.  0xC8258 = duty, all 32 bits.   <- us
```

Measured: `FREQ 0x5EB2` = 24242 clocks of 24 MHz = **990 Hz**, `DUTY 0x556E` =
21870 = **90% brightness**. Under the packed reading, `intel_backlight_max()`
computed `0x5EB2 >> 16` = **0**, so `intel_backlight_set()` hit its `if (!max)`
and silently did nothing, and `intel_backlight_get()` returned the *period* as
the brightness.

It failed **safe** — max reading 0 meant it bailed before writing, so it never
corrupted the period. But it was dead code that looked live, and it is one of the
few write paths *not* behind `lt_armed`, so it would have been the first thing to
run for real.

## Still true, and the thing to fix next

**Nothing in the kernel ever arms `lt_armed`.** Only `tests/host/dpll_test.c` and
`tests/host/intel_probe.c` call `intel_link_train_arm()`. Every write path in
`src/drivers/display/intel.c` is unreachable from zlOS itself — the driver reads the display
correctly and cannot yet touch it.

## The ordered modeset exists now — 35 steps, and it can be read before it runs

`intel_modeset_run(port)` walks Phases B→H in the plan's order. Set the mode
first (`intel_modeset_set_from_hw()` takes it off the running panel), then run.

**`intel_modeset_dry(port)` walks the whole sequence writing nothing** — it needs
neither `lt_armed` nor a detached i915, so the order is reviewable on a live
desktop. `./gpu-dev.sh probe` prints it. That matters because the sequence is now
the *only* genuinely untested thing left: every primitive under it was checked
against what firmware programmed, but the order has never executed.

Dry mode is a macro, not a flag, for a real reason: C evaluates arguments
eagerly, so `ms_do(3, "...", intel_dc_states_block())` would touch hardware even
in a dry run. `MS_STEP` defers the call.

**Verified dry.** A full dry run followed by a register diff: all 25 registers
the sequence writes are byte-identical, and `PWR_WELL_CTL_DRIVER`,
`PWR_WELL_CTL_BIOS` and `DC_STATE_EN` are unchanged across two runs. The 22
registers that *did* move are i915's own — `PIPEDSL`, the frame and flip
counters, timestamps, a page-flipped `PLANE_SURF`, and `DDI_BUF_CTL` bit 7 which
is read-only idle status.

Failures report the plan step number, not "modeset failed".

Three things it needed that also did not exist:

- **A port enable/disable primitive.** `DDI_BUF_CTL_ENABLE` was defined and
  never written by anything, so `intel_link_train()` was writing training
  patterns into a port nothing had switched on. Plan implementation order #5
  calls this mandatory before any training attempt.
- **`intel_dbuf_enable()`** (step 10) and **`intel_cdclk_khz()`** (step 9).
- **`intel_iboost_set()`** (step 32), including the x4 rule — DDI A's upper
  lanes are driven by the DDI E field, so both halves need programming or two
  lanes sit on a different drive setting from the other two.

Two guards that were silently too narrow:

- `intel_pwr_well_enable()` rejected anything above index 3, which made **PW1
  (index 14) unrequestable** — plan step 6, a prerequisite for the whole display
  core. The REQ/STATE macros were always right; only the range check was wrong.
- `lt_armed` was declared halfway down the file, so a write path added above it
  failed to compile in a way that reads as "lt_armed is missing". Moved up with
  the other module state.

**Known deviation from the plan:** step 40 makes ONE training attempt and fails,
where the plan wants a bounded retry. Failing loudly is the plan's actual point,
and a correct retry must cycle the port first (4.3 #20) — worth adding once the
sequence has run once, not before.

Still missing: EDID over I2C-over-AUX (GMBUS does not serve eDP on DDI A).
`LINK_RATE_SET` Method B is **not** needed — this panel has no rate table.

## IT WORKS. The panel was lit by our own driver on 2026-08-17

`sudo ./modeset-run.sh --modeset` brought the ThinkPad's panel up from cold and
displayed a test pattern — a colour gradient with a white border, held for ten
seconds, confirmed by eye. All 34 steps green, then a clean teardown and the
desktop back with no power button.

```
*** PICTURE SHOULD BE ON SCREEN NOW - holding 10 s ***
underrun after 10 s: clear
teardown: clean
pipe off, panel off, port off
exit code: 0
```

**`underrun after 10 s: clear` is the second result.** Ten seconds of real
scanout with zero FIFO underruns, which validates the watermarks and the DDB
split derived from firmware — including the cursor's 13 blocks and the 0..858 /
859..891 division the plan had inverted.

### What the four failed attempts before it actually cost

Every one of them was a bug in the *harness or the checks*, not the modeset:

| Run | Reported | Truth |
|---|---|---|
| 1 | FAILED at step 56 | no framebuffer existed; armed a scanout of address 0 |
| 2 | 34/34 SUCCESS, dark | GGTT unreachable — 8 MiB map stops one byte short of it |
| 3 | FAILED at step 56 | plane WAS armed; `PLANE_SURFLIVE` returns address + status bits, so the equality check could never match |
| 4 | — | backlight enabled at **zero duty**: correct image on an unlit panel |

Run 1 also left the display half-configured and cost a hard power-off. That is
why `intel_modeset_teardown()` exists and why the harness calls it on the
success and failure paths both.

### The two mapping facts worth keeping

- The kernel will **not** serve one 16 MiB mapping of BAR0 — EINVAL, i915 or no
  i915. It will serve 8 MiB at offset 0 and 8 MiB at offset 8 MiB separately.
  Reserve 16 MiB with an anonymous `PROT_NONE` map and `MAP_FIXED` both halves
  into it; the driver then sees one contiguous BAR exactly as in the kernel.
- The framebuffer lives in **stolen memory** — `/proc/iomem` confirms
  `79800000-7d7fffff : Graphics Stolen Memory` inside `Reserved`, so it is never
  Linux RAM. Paint it through `resource2_wc`.

### Still true

**Nothing in the kernel arms `lt_armed`.** This runs from the host harness only.
zlOS itself still cannot light the panel — the driver can, and is proven to, but
the kernel has no caller. That is now the single thing between this and zlOS
booting on the ThinkPad with its own display.

## Historical display plan: `kernel/docs/archive/superseded/display-roadmap.md`

The decision (2026-08-17): **complete the display subsystem entirely before any
GPU work.** No ring buffers, no blitter, no execution engine until every item in
that roadmap is done.

Ordered by dependency, and the order is not the appealing one — the two most
interesting phases are blocked behind a parser and a grind:

```
0  close what is open      wire into zlOS, second modeset, LT retry,
                           EDID over AUX, X-tiled scanout
1  VBT parsing             unblocks 2 and 3; a parser, no hardware sequence
2  de-hardcode pipe/port   42 *_A registers; mechanical; blocks 3/4/5
3  HDMI, then external DP  HDMI first - no training, no panel power, no T12
4  hotplug                 zero lines today; needs a real interrupt path
5  planes, rotation, scaling, tiling
6  colour (gamma, CSC)
7  PSR / DRRS
8  audio over HDMI/DP      needs an HDA driver alongside
```

State measured rather than remembered: **one port, one pipe, one panel, one mode,
polled.** 42 `*_A` registers, no VBT parser, no HDMI port bring-up (the DPLL
clock math exists and is verified), and zero hotplug or interrupt code.

## What is actually a wall: `docs/concepts/what-is-actually-impossible.md`

**The project's thesis is that one person can do what is assumed to need a team,
so "too big for one person" is not a valid objection here — it is the hypothesis
under test.** That document re-grades the whole board against that bar. Summary:

- **Nothing on the board is impossible because of size.** Refuted by Terry Davis
  (TempleOS, ~10 yr solo), nakst (Essence, 9 yr solo), Kling, Bellard — and
  partly by this repo already.
- **The real walls are all one shape: a vendor holds a signing key, or the
  target is a competitor's shipping product.** There are about six, each with a
  legitimate route around: WiFi/CNVi (→ USB tether), Widevine (→ nothing to
  build), cellular, Bluetooth, Secure-Boot *distribution* (→ enrol your own key),
  GPU 3D (→ software rasterizer). **Modesetting is deliberately not on this
  list** — no blob, no signature, which is why it was reachable.
- **What kills solo OS projects is the breadth tax and stopping, not
  difficulty.** The golden-transcript gates and this file's verified-not-intended
  discipline are the defence. Don't let them slip.
- **Highest impressive-per-remaining-work item on the whole board:** zlOS booting
  the ThinkPad from `raw_boot.asm`, lighting its own panel at 2560×1440 through
  its own modeset, drawing its own desktop — no GRUB, no Linux, no blob. Blocked
  on **one missing caller**: nothing in the kernel arms `lt_armed`.

## The three "what about…" questions, answered: `docs/concepts/beyond-the-kernel.md`

Other languages on zlOS, the internet, and rewriting the C in zl. Asked and
measured 2026-08-17. Short version:

- **Other languages** — no heap, no ELF loader, no per-process address space
  (all four cores share one CR3, `src/arch/x86/smp.c:129`), and
  `fs_save`/`fs_load` is a RAM
  slot array, not a filesystem. Cheapest real win is hosting **zl's own
  interpreter** (1,900 lines) on zlOS; highest leverage is a **WASM interpreter**
  (~8k), which buys every language at once. POSIX ELF is a second project.
- **Internet** — zero network code today, but `lspci` says the ThinkPad has a
  real wired NIC (`8086:0d4f`, `Kernel driver in use: e1000e`). Wired is
  bounded: ~10k lines to HTTPS, of which TLS is 5–8k.
- **WiFi and Bluetooth** — Roy wants both. Plan: **`docs/plans/wireless-plan.md`**.
  That doc also **corrects two wrong claims** made earlier the same day in
  `docs/concepts/what-is-actually-impossible.md` and `docs/concepts/beyond-the-kernel.md`:
  - **Bluetooth is NOT a wall.** `lsusb` → `8087:0026 Intel AX201 Bluetooth`,
    **on USB**, `Class=Wireless / RF / Bluetooth` (224/1/1) — the standard class
    every BT dongle implements, and **HCI is a published Bluetooth SIG
    standard**, not a vendor protocol. Its endpoints are control + interrupt IN
    + bulk IN/OUT, and **`src/drivers/input/xhci.c` already implements all
    three**. ~6,800 lines
    to a working BT keyboard; **~2,000 to "zlOS lists nearby devices"**. Build
    against a **CSR dongle** (ROM firmware, no upload) before touching Intel's
    704 KB `ibt-0040-0041.sfi`.
  - **The iwlwifi blob is 1.3 MB, not 2–3 MB** (1,406,572 bytes, measured), and
    it is redistributable — so the AX201 is Kind-1 hard (~40k lines of
    undocumented protocol), not vendor-gated.
  - **WiFi is only gated if you let Intel pick the radio.** The **AR9271 USB
    dongle's firmware is 50 KB and open source** (`open-ath9k-htc-firmware`,
    already at `/lib/firmware/htc_9271.fw`). ~8,000 lines to WPA2 association;
    **~3,800 to "zlOS printed my SSID list"**. An ESP32 over UART is ~400 lines
    if you just want the network stack unblocked now.
  - Still true: **USB tethering or a USB NIC** is the shortest path to packets,
    and `src/drivers/input/xhci.c:1709 configure_bulk()` is why.
- **A browser** — **BUILT, and the estimate below was wrong in both directions.**
  It fetches `http://example.com/` by name off the real internet and renders it.
  ~4,657 lines across `browser.c html.c layout.c http.c tcp.c net.c dns.c
  virtio_net.c`, all in `SOURCES`, all gated. Not ~13,200 + ~10k — and **the
  gate was not a heap**: there is none, and no design constraint was relaxed to
  get here. `kernel/docs/browser-status.md` is the measured account, including
  what it refuses (HTTPS, JavaScript, CSS beyond the built-in stylesheet) and
  the two regressions the merge cost it. The reasoning below was right that
  "unbounded" describes a Chrome-compatible browser and not a document one;
  it was wrong about the price and about the blocker.
  *Original text, kept because the shape of the error is the reusable part:*
  `docs/archive/superseded/feature-catalogue-2026-08-17.md` says don't; **that call was wrong as
  stated** and `docs/concepts/beyond-the-kernel.md` §2b supersedes it. "Unbounded" is true of
  a Chrome-compatible browser, not of a *document* browser — Dillo, NetSurf and
  w3m have shipped that for decades. A document browser (HTML+CSS, no JS) is
  **~13,200 lines** plus the ~10k network stack. **The gate is a heap** (~300
  lines) — which means building one is a *decision to relax a stated design
  constraint*, not just work. Useful: the HTML→CSS→layout→paint branch needs no
  network and is testable in QEMU against local files.
  Also found: **the fonts are baked at build time into three fixed sizes only**
  (8×16, 16×32, 24×48, monospace, from DejaVu Sans Mono via `gen_hd_font.py`).
  There is **no runtime rasterizer** — see the correction below.
- **C → zl** — the blocker is the compiler, not the kernel. The kernel builds
  with `compile`, the backend the root README marks **ARCHIVED**. Current tree:
  `sizeof(Value)` is now 16 B, but builtins still dispatch through a
  **644-entry strcmp chain**, and every number is a `double` so **64-bit BARs
  and DMA addresses are not representable** — the project's own recurring bug class.
  `compilel` emits real `i64`… until you use a bitwise operator, at which point
  the return type degrades to `double` and every operand is boxed and dispatched
  by name. Historical pre-Value16 result for 10M iterations: **C 7 ms, zl
  arithmetic 4 ms, zl bitwise 999 ms**; rerun it before using those as current
  timings.

**The actionable part:** making `band/bor/bxor/shl/shr` native `i64` instructions
in `compilel.c` is ~1,500 lines of compiler work total with the other three
prerequisites, needs **no laptop, no panel, no hardware access**, cannot break a
running driver, and is testable on the host against the interpreter. It is the
only major item in that document with that property.

## The one thing blocking a cold-start modeset

It needs the display to itself. gnome-shell + Xwayland hold `/dev/dri/card0`,
so unbinding i915 under the live session kills the desktop, not just a console.

**Ask Roy to do this, then say go:**
```
Ctrl+Alt+F3
sudo systemctl stop lightdm
```
Recovery: `sudo systemctl start lightdm`.

## The desktop is the boot state now (2026-08-18)

`src/kernel.zl` ends in `if wm_avail() == 0 { ...text shell... } else { wm_session() }`.
With a framebuffer you get a compositor with the shell, System Monitor and
About open; without one - which is what `verify.sh` boots - the old text shell
runs unchanged and its transcript is still byte-identical to `golden.txt`.

Every demo is an app in a window: no `while` loop, no "press any key". Typing
`snake`, `paint`, `cube`, `anim`, `mouse` or `edit` opens one. The full account
of that run, including four things it found that no task list predicted, is
`kernel/docs/evidence/desktop-platform-run.md`.

## Everything else in the kernel

15 drivers, ~6,000 lines, all ours: `pci` `bga` `intel` `xhci` `efi` `apic`
`virtio_gpu` `cpu` `nvme` `sched` `smp` `i2c_hid` `input` + two SMP trampolines.

64-bit, 4 cores woken via INIT/SIPI, multitasking scheduler, NVMe persistence,
bounded read/write USB mass storage, a persistent USB boot journal, USB HID
keyboard, event-based input with modifiers and repeat, and a line editor with
history.

**Unproven:** `src/drivers/input/i2c_hid.c` (QEMU has no Intel LPSS I2C) and the
cold-start modeset.

Worse than unproven, on `src/drivers/input/i2c_hid.c`: it is a **transport with
no decoder**.
`i2c_hid_byte(i)` returns raw undecoded bytes. Nothing turns a touchpad report
into an x, a y and a button — that code does not exist yet.

## The desktop — BUILT, and it is what boots

The inversion landed 2026-08-18. `wm_frame()` is the top of the system on any
machine with a framebuffer; the shell is window 0 inside it, with a scrollback
and typed commands. On a machine with no framebuffer the old text shell runs
exactly as it always did, and `verify.sh` still diffs it byte-for-byte against
`golden.txt`.

**Verified by boot, not by assertion:** `shots/v10-now.png`. All four boot
paths green — `verify.sh`, `tools/checks/verify-raw.sh`,
`tools/checks/verify-efi.sh`, and the ISO.
`wmtest` 79 · `inputtest` 24 · `tritest` 9 · `fbbench` all green.

**Two probes drive the POINTER rather than the keyboard**, and they exist
because everything else in this repo types: `tools/probes/probe-dock.py` (hover,
press, launch, the menu, dismiss) and `tools/probes/probe-resize.py` (the corner
grows a window, the title bar still moves it). `tools/probes/probe-drag.py`
confirms a drag moves 18% of the
screen.

**A PATTERN WORTH KNOWING BEFORE YOU ADD ANYTHING HERE.** Five things in
`src/graphics/windowing/wm.c` and `src/graphics/framebuffer/fb.c` were complete,
correct, gated, and had **no caller at all**:
`WF_MODAL`, `wm_resize()`, `fb_blur_cache`, the whole animation timeline, and
`MOD_SUPER`. This repo is written mechanism-first and gated hard, which is
exactly what makes that easy to produce — a primitive arrives with tests, a
measurement and a design comment, and passes every check while being
unreachable. **A primitive is not done when it passes its test; it is done when
something calls it and a gate covers the call.** For visual work the assertion
has to check the PIXELS, not the state: `wm_anim_alpha()` reported a fade
correctly for hours while nothing drew one. See T-16 and
`docs/evidence/desktop-scale-and-effects.md` §5.

**Why the desktop looked small on a big screen, and where the effects went:**
`docs/evidence/desktop-scale-and-effects.md`. Short version: `ui()` was `cell_w() / 8`,
so it was 1 or 2 and never more, while the layout is written in 800 design
units - at 3840 wide that is 1920 units of space for an 800-unit design. It is
derived from the screen now (1..4) and the console cell is a separate question.

What the v10 pass added, with the numbers, is `docs/evidence/desktop-v10-plan.md` §8.
The five that matter most to somebody touching this next:

1. **Serial is an input source now**, not a thing the shell polls. `wm_frame()`
   reads `src/drivers/input/input.c`'s queue and nothing else, so a byte only
   `key_get()` could
   see was a byte the desktop could never see — every gate and probe in this
   repo would have gone blind the moment the compositor booted. `ser_rx()`
   probes the UART's scratch register first, because **an absent UART floats
   high**: the naive "is LSR bit 0 set" is true forever on the ThinkPad and
   would have injected an endless stream of 0xFF keystrokes on the one machine
   with no other diagnostic.
2. **C4 deleted the sticker-drag machinery** — `bg_buf` and `sp_buf` at 128 and
   160 MiB, `fb_bg_snapshot`/`fb_bg_restore`/`fb_grab`/`fb_stamp`. The back
   buffer moved into their 48 MiB and **now covers 3840×2160**: a whole-desktop
   redraw there went 44 ms → **9.71 ms**, and a full-screen fill 7.97 → 0.71
   cyc/px. Scene hashes byte-identical at every mode.
3. **`raw_boot.asm` loads a FIXED number of chunks.** It was 40 (1.25 MiB)
   against a 1.23 MiB kernel. A kernel over that limit is not a build error —
   it is silently truncated and jumped into. `tools/images/mkdisk.sh` refuses
   to build such
   an image now, and CHUNKS is 60.
4. **The wallpaper is a cached bitmap**, and that is arithmetic rather than
   taste: a translucent full-screen pass is 22 cyc/px ≈ 22 ms at 1920×1200, the
   compositor redraws the wallpaper inside *every* damage rectangle, and the
   v10 background is six such passes. Cached it is 1.5 cyc/px. At 4K it does
   not fit the arena, refuses, says so, and falls back to the plain gradient.
5. **THE COMPOSITOR COULD NOT SEE THE MOUSE, and no gate could have caught
   it.** zlOS drives two pointers - an absolute usb-tablet through
   `src/drivers/input/xhci.c` and a relative PS/2 mouse through
   `src/arch/x86/idt.c` - and the `mouse_x` builtin has preferred the tablet
   since it was written. `src/drivers/input/input.c`'s `pump_mouse()` read
   `idt_mouse_x()` **and nothing else**. While the shell owned the screen that
   was invisible, because the shell called `mouse_x()` directly. The moment
   `wm_frame()` became the top of the system the queue was the compositor's
   only source of pointer events, so on any machine with a tablet attached -
   which is what QEMU gives and what `tools/run/try.sh` attaches - **no EV_MOUSE was
   pushed at all**. No dragging, no clicking, no dock, no menu.

   Every gate in this repo drives zlOS by TYPING, and a dock that does nothing
   photographs identically to one that works. `tools/probes/probe-dock.py`
   exists because
   of this, and `inputtest` now asserts the preference directly.

6. **`console_mute()`** stops the console painting while the compositor owns the
   screen. The tee into term.c's scrollback and the write to COM1 both keep
   going — which is what keeps every gate reading exactly what it read before.

### The design docs, still worth reading

The shell owns the main loop and windows are a demo it launches. A real desktop
inverts that. Designed 2026-08-17:

- **`docs/visual-speed-northstar.md` — Roy's actual intent for the v10 HTML.**
  Read this first for visual work. The target is how polished the desktop looks
  and how fast it feels, not a demand to clone every simulated app before the
  comparison counts. It separates look, feel, machinery and application
  completeness, and records the 16.67 ms / `late = 0` interaction target.
- **`docs/DECISIONS.md` — every decision from 2026-08-17 in one page**, including
  the two that turned out wrong and why. Read this before the others. **#29–#33
  (2026-08-19) settle the kernel against `docs/design/zlOS-design-northstar.html`
  item by item** — the blur is gone and the reason is that it was disabling the
  wallpaper cache on the ThinkPad's 2560x1440 panel, not that it looked wrong.
- **`docs/archive/prompts/NEXT-PROMPT.md` — historical ranked queue, with the
  measurement that produced its old order.** Do not use it as the current
  queue. It also records the standing
  hazard nothing else states plainly: three to five agent sessions share this
  one checkout, and on 2026-08-19 that produced two simultaneous land gates,
  one session's commit sweeping another's in-progress edits, and load average
  15.
- **`docs/desktop/look-and-speed.md` — what a frame costs, what paces it, what is
  next.** The frame target (every frame AND the peak under 16.67 ms), the
  vsync survey per backend (one source exists,
  `src/drivers/display/intel.c`'s `PIPE_FRMCNT_A`, and
  it has zero callers), the blast radius of raising the PIT, and why SMP band
  rendering is one call that should not be made yet — 1.76x measured, not 4x,
  and two bands is slower than serial. **Two of its entries carry correction
  banners** — one claim in it was wrong and says so.
- `docs/guides/desktop-build-guide.md` — start here. What a compositor is, in plain
  words, and the build order.
- **`docs/archive/superseded/desktop-TODO.md` — historical ordered task list.**
  Its audited banner says not to work from the body; use `../docs/REMAINING-WORK.md`.
- `docs/archive/superseded/desktop-plan.md` — the decisions, the evidence, and the line numbers.
- `docs/desktop/desktop-toolkit.md` — **the layer that was missing from every earlier
  plan.** An **immediate-mode** toolkit (`ui.c`), chosen because a retained
  widget tree needs a heap and *is* a list of children — zlOS has neither.
  Widgets return whether they fired; nothing is allocated. Hit testing re-runs
  `app_draw` with drawing off, the same trick as `intel_modeset_dry()`.
- `docs/desktop/desktop-look.md` — why it looks blocky. **The renderer is not the
  problem** — real TrueType glyphs, subpixel LCD rendering and gamma-correct
  linear-light blending are all already in. **Read "TrueType" precisely:** the
  shapes come from DejaVu Sans Mono, but `gen_hd_font.py` rasterizes them **at
  build time** into three fixed coverage atlases — `font8x16`, `font16x32`,
  `font24x48`, monospace only. There is **no runtime rasterizer and no arbitrary
  size**. Fine for a desktop, a hard blocker for a browser. It is three
  resampling bugs, the
  worst being `fb_icon24` nearest-neighbour upscaling every icon at 2×
  (`src/graphics/framebuffer/fb.c:929`).
- `docs/desktop/desktop-polish-and-speed.md` — what makes a desktop look modern, and
  what it costs. Three facts up front: **three of four cores are parked** in
  `cli; hlt` forever (`src/arch/x86/smp.c:79`) so all drawing is single-core;
  **nothing measures a frame inside the kernel** (TSC exists in
  `src/arch/x86/cpu.c`, not exposed to
  zl); and the renderer is now benchmarked — see below.

- **`docs/archive/superseded/feature-catalogue-2026-08-17.md` — every feature found across ~15 hand-built OSes,
  organised by subsystem with a zlOS have/partial/none column.** 16 categories,
  from kernels to clipboard. Ends with a ranked shortlist of what is actually
  worth taking, and an explicit list of what is not (microkernels, capability
  security, attribute-indexed filesystems).
  **That list used to name filesystems, network stacks and browsers too, on the
  grounds that "each needs a heap or processes". All three shipped, and none of
  them needed either** — `src/fs/fs.c`, the files under `src/net/`, and
  `src/web/browser.c` use the same static arenas as the rest of the kernel. The
  premise was never tested against an attempt. The rows are corrected in
  `docs/archive/superseded/feature-catalogue-2026-08-17.md` §12 and §"Why a browser is in a category of its
  own"; `kernel/docs/browser-status.md` is the measured account.
  Top of the shortlist is still **the clip rectangle**, which now has *three*
  customers: compositor, toolkit, and the 3D rasterizer.
- `docs/research/os-landscape.md` — survey of ~13 hobby OSes, written for the **3D goal**.
  Headline: **SerenityOS runs Quake III on a software rasterizer, no GPU** —
  LibSoftGPU, 16×16 tiles, barycentric, SIMD. That is the path, and
  `src/graphics/framebuffer/fb3d.c` is
  its first step. Also: **Essence OS is one person since 2017** (nine years) with
  an animated software *vector* renderer; **Haiku is 25 years old and its Intel
  driver is modesetting only**, i.e. where `src/drivers/display/intel.c` is
  aiming; and **Redox has
  excellent systems engineering and a "sluggish, unpolished" UI** — a good
  desktop is not downstream of a good kernel.
- `docs/research/desktop-prior-art.md` — how TempleOS, SerenityOS, Essence, Haiku, Redox,
  Managarm and Asterinas do graphics. **Nobody in this space has GPU
  acceleration.** SerenityOS runs Half-Life on a SIMD *software* rasterizer.
  **Essence OS is the closest analogue** — one developer since 2017, software
  *vector* renderer **with animation**, window manager **in the kernel** (same
  choice as `docs/archive/superseded/desktop-plan.md`). TempleOS was 640×480/16 colours; **zlOS is
  already well past it**. Do not write a 3D driver — the *display* driver alone
  has cost a 13-conflict plan and an 86-defect audit.
  **Note:** `docs/archive/superseded/GRAPHICS_PLAN.md` (2026-08-03) says the GPU is reached
  via `opengl32.dll` FFI. That is the **Windows-hosted** plan and does not apply
  here — it has been annotated. On zlOS, 3D means a software rasterizer, and
  `src/graphics/framebuffer/fb3d.c` is its first step.

  **On "can we just take Linux's driver":** no, and the doc measures why on this
  machine. `i915.ko` is **11.2 MB** uncompressed and ~100K lines; Mesa's Intel
  Vulkan driver is **24.3 MB**; the **entire zlOS kernel is 1.07 MB** and 11,374
  hand-written lines. FreeBSD runs i915 only via **LinuxKPI** — it emulates the
  Linux kernel API rather than porting the driver, and i915 assumes GEM, TTM,
  dma-buf/dma_fence locking, workqueues and a heap, none of which zlOS has by
  design. **`src/drivers/display/intel.c` already does the correct thing: borrow
  Linux's knowledge, not its code.** Also:
  `src/drivers/display/virtio_gpu.c:314` disables virgl on purpose — enabling
  it would give real 3D in QEMU only, never on the laptop.
- `docs/archive/superseded/desktop-northstar-feasibility.md` — can zlOS run the `~/zl OS v10.dc.html`
  mockup? Keep its layer breakdown and gap list; **ignore every percentage in
  it.** Successive 95%, 20%, 65% and 35% figures each collapsed different
  layers and each misled. `ui.c` is a layout engine and `ui_list_row` expresses
  a list without a list type. **Separating "the language cannot" from "nobody
  has written it yet" is the whole lesson**, and getting it wrong costs the
  wrong fix.

## The renderer is benchmarked: `tests/host/fbbench.c`

**This dev box IS the test laptop** — `i7-10510U`, Comet Lake-U, same chip family
as the `8086:9B41` graphics. So `src/graphics/framebuffer/fb.c` timed here runs
on the real target CPU.

`fbbench` compiles the **shipping `src/graphics/framebuffer/fb.c` unmodified**
at the kernel's own `-O2`
and `mmap`s the three fixed physical addresses fb.c hardcodes. No sudo.

```
cd kernel/tests/host && ./build.sh && ./fbbench
```

Baseline measured 2026-08-17, whole desktop with 3 windows:
**19.98 ms @ 1920×1200, 26.80 ms @ 2560×1440** — a 50 and 37 fps ceiling with
nothing else running.

It also puts the resolution cliff on a stopwatch: a full-screen gradient costs
2.93 cyc/px with the back buffer on and **7.97 cyc/px** with it off, and
`bg_snapshot` returns in **0.00 ms** at 2560×1440 because `bg_ok = 0` — that is
window dragging, doing nothing, measured. And it is a **floor**: the harness uses
ordinary RAM for VRAM, so real hardware pays the 30–50× VRAM read on top.

### `fb_shadow` fixed — 4.1× on a full desktop redraw

`fb_shadow` darkened the window's **entire footprint**, and the caller drew the
window on top of ~90% of it. It now skips the covered rectangle, inset by
`SHADOW_SKIP_INSET` so the rounded corners keep their shadow.

| | before | after |
|---|---|---|
| shadow 600×460 | 4.34 ms | **0.61 ms** |
| one window, full chrome | 5.12 ms | **0.90 ms** |
| whole desktop @1920×1200 | 19.98 ms | **4.88 ms** |

**Verified pixel-identical** (FNV hash of the whole back buffer) and `verify.sh`
passes. **A static instruction count first pointed at a per-pixel divide in that
same loop — tabling it made the shadow 25% SLOWER.** The loop is not
arithmetic-bound. Measure, then optimise, then measure again.

Two things found while planning that you need before believing anything about
the desktop on real hardware:

1. **At 2560×1440 the back buffer switches itself off**
   (`src/graphics/framebuffer/fb.c:155` —
   `BACK_MAX` is `1920*1200`), and it takes subpixel text, fast pixel readback
   and **window dragging** with it, silently. The ThinkPad panel is 2560×1440.
   Verified by reading; **not yet observed** — zlOS has never booted on it.
2. **`src/graphics/framebuffer/fb.c` has no clipping.** Every primitive clips
   to the screen and nothing
   else, so there is no way to repaint part of the screen. That, not the window
   code, is what blocks a real compositor.
## Arrow keys reached no application until 2026-08-18, and PS/2 was innocent

Reported as "arrows are not delivered to apps", with a correct measurement
behind it: in the browser, injecting qcode `spc` scrolled and `down` did
nothing, though `browser_key()` handles both. The suspected cause was the
`0xE0` prefix in the PS/2 decode — which fits the evidence exactly, and is
wrong. `tests/host/inputtest.c` drives the real `src/drivers/input/input.c` and the PS/2 path
decodes all nine extended keys correctly, before any change.

**The keyboard in the repro was the USB one.** `tools/run/try.sh` attaches `-device
usb-kbd` and QEMU routes typing to it once it exists.

The USB path decoded keys to **characters**. There is no character for Up, so
`hid_to_ascii()` returned 0 — and 0 already means "nothing was typed". The key
vanished with no error at any layer. Adding arrow cases to that function cannot
fix it: there is nothing to return. The transport had to change shape.

| Was | Is |
|---|---|
| `xhci_key()` → a character; `input_poll()` pushes `EV_CHAR` | `xhci_key_event()` → packed `press`/`mods`/HID usage; `src/drivers/input/input.c` translates |
| a keymap in `src/drivers/input/xhci.c` and another in `src/drivers/input/input.c` | printable USB keys map to a set-1 scancode and go through the **existing** `to_char()` |
| USB sent presses only | releases too — without them `key_down[]` never clears and repeat never stops |
| USB handled shift and nothing else | ctrl, caps and super, because it is the same `to_char()` — **Ctrl+W was dead on an external keyboard** |
| a held shift alone was invisible | `xhci_kbd_mods()` publishes the live bitmap |

`src/graphics/windowing/wm.c` was never involved; `route_key()` forwards every
event type already.

`key_down[]` now has **three** regions — PS/2, PS/2-extended, and USB at
`0x200 + usage`. A USB `'a'` is usage 0x04 and a PS/2 F9 is scancode 0x04, so a
shared slot means releasing one un-holds the other.

`xhci_key()` still exists, still returns a character, and reads its **own**
queue: `src/kernel.zl` calls the `usb_key` builtin in two places and compares the
result against 13 and 27. Two queues also stop the shell and the compositor
stealing each other's keystrokes.

Why it hid so long: **a test that asks "did a key work" passes on the broken
code**, because most keys did. `tests/host/inputtest.c` asserts on event *type and
code*, and asserts parity — the same key must produce the same event from either
keyboard. No GPU, no root, no QEMU, milliseconds:

```
cd kernel/tests/host && ./build.sh && ./inputtest
```

Full write-up: `docs/reference/system/input-stack.md`.

## How a harness types a command — `docs/desktop/typing-into-the-compositor.md`

**Serial reaches the compositor.** `src/drivers/input/input.c` feeds COM1 into
the same event
queue as PS/2 and USB (`SERIAL, the third source`), which is what kept every
gate in this repo working when the desktop became the boot state.

What does not work is sending a bare character. The shell is a window whose
input is a **line**: `src/graphics/windowing/term.c` buffers printable characters and only on Enter
echoes the line and looks the first **word** up in its table. So `windows`,
not `w` — `w` was a command in the old text shell and is now one character in
a buffer.

**A single character produces no serial output at all**, because the echo lives
in the Enter branch. That silence is identical to a dropped key, and reading it
as one is how two probes came to document "serial cannot reach the compositor"
on branches whose own `src/drivers/input/input.c` says otherwise. Measured 2026-08-19: `-k w`
changes 1 225 pixels, all inside the prompt line, with the `w` visibly sitting
in the buffer.

`tools/probes/probe-shot.py -k` takes a command, submits it, and waits for
`src/graphics/windowing/term.c` to echo
the line back — the one marker that proves it was taken, on either wire. It
exits non-zero when the echo never comes, instead of photographing a frame no
command ran in, which is what it used to do.

## zlOS keeps things now — `docs/evidence/system-track.md`

Files had no names and nothing survived a reboot. **zlfs** (`src/fs/fs.c`) is a
superblock, a flat directory of 32 named entries, and files as contiguous runs
on the NVMe disk. `src/arch/x86/rtc.c` reads the CMOS clock, so the header shows
a real time instead of uptime — and the header has stopped drawing "net up",
which claimed a network driver this tree does not contain.
`src/graphics/windowing/clip.c`, `src/graphics/ui/snap.c` and
`src/graphics/ui/notify.c` are the clipboard, window snapping and toasts.

The desktop path is documented in
[`docs/features/storage-and-files.md`](docs/features/storage-and-files.md). Files mounts zlfs on
open and creates, opens and deletes entries by name. zlEDIT has a disk-backed
mode with Ctrl+S, ESC save-and-close and clipboard copy/paste; `edit <n>` keeps
the old RAM slots only as a compatibility path.

There are now two power-cycle checks. `tools/checks/verify-disk.sh` boots three
times against one image and requires its counter to go 1 → 2 → 3.
`tools/probes/probe-files.py` drives the
real Files/editor UI, kills QEMU, boots a new process on the same NVMe image and
requires the named file's exact editor pixels and byte count to survive.

The write path was reviewed adversarially — a fresh agent told to lose a file,
proving each claim by running it — and it found **six data-loss defects** in
code that already had 63 passing assertions, including one where the comment
asserted an invariant the code did not hold. All six are fixed with regressions
that fail on the old code. The full account, and what is deliberately left
undone in `src/graphics/windowing/wm.c`, is in
[`docs/evidence/system-track.md`](docs/evidence/system-track.md).

## The recurring bug class — check this FIRST

Six times now: **a DMA buffer outside guest RAM, on top of another buffer, or an
address truncated to 32 bits.** Symptoms look like protocol bugs.

- **zlOS needs 1 GiB of RAM minimum.** See "The RAM floor" below — this was
  `-m 256` until 2026-08-20 and the change is a deliberate trade, not a drift
- `u32 reg = xop + OFFSET` where xop is a 64-bit BAR → **reads correct, writes
  vanish**, 64-bit build only
- Every driver now ships a `*_ram_ok()` probe

**The sixth was caught by reading, not by running, and that is the lesson.**
`src/drivers/input/i2c_hid.c` had `HID_BUF` at `0x0C900000` — 9 MiB inside the
16 MiB arena `src/graphics/framebuffer/fb.c` hands out for cached blurs, and
inside the span `back` occupied before
the compositor moved it. Two owners, one address range, neither aware of the
other. It had never been *observed* because it could not be: QEMU has no Intel
LPSS I2C controller, so the touchpad driver only runs on the laptop, which is
also the only machine with a panel big enough to make the framebuffer reach.
The two halves of the bug were never on the same machine as a working test.

The map is now **[`kernel/src/arch/x86/memmap.h`](src/arch/x86/memmap.h)** — declared once, with every
owner asserting its own extent against its neighbours at compile time. It
replaced a comment in `src/graphics/framebuffer/fb.c` that carried the list and
told you not to trust
it* ("do not take this list on trust, re-grep it"). That instruction was the
admission; the list was already wrong when it was written.

```
cd kernel/tests/host && ./memmap-guard-test.sh    # seconds, no QEMU, no hardware
```

**That paragraph said "12 checks" and the script was scoring 10 passed, 2
failed** — verified against a clean `git archive HEAD` tree, not the working
copy. Both failures were the same staleness: `HI_APSTK` (the AP stacks) was
inserted between `HI_BACK` and `HI_SCHED`, `BACK_LIMIT` became
`HI_APSTK - HI_BACK` = 40 MiB, and neither the break that perturbs it nor the
literal that pins it followed. So the script whose whole job is to prove
guards are not decorations had a decoration of its own, and its own failure
message — *"the map was broken and NOTHING complained"* — was on screen for
anyone who ran it.

It is **20 checks** now, all green: one baseline compile of ten owners, ten
deliberate breaks each refused by the build (including replays of both real
collisions), and nine files whose rebased addresses are proven identical to the
literals they replaced.
**A `_Static_assert` nobody has watched fail is a decoration, not a guard** —
that is what the negative half of that script is for.
**And the corollary nobody had written down: NOT ONE GATE passes `-m`.**
`verify.sh`, `tools/checks/verify-raw.sh` and `tools/checks/verify-iso.sh` all
boot QEMU's default, which
is **measured** at exactly 128 MiB (`query-memory-size-summary` says
`base-memory: 134217728`). So on every gate this project runs, the whole
high-RAM map is unbacked, and **a new fixed buffer placed above 128 MiB is dead
code that will still pass review**. That is why the program arena is at 8 MiB.
It is also why the browser's storage region (`HI_DOM`) is at **80 MiB** and not
at some round number above `HI_BACK`: 80..96 MiB is under the 128 MiB every
gate actually boots with, so `verify.sh` exercises it rather than stepping
around it.

## The RAM floor — 1 GiB, and what it cost

Fixed 2026-08-20. The paragraph directly above was true when written and is now
the *history* of this section, kept because the shape is worth recognising.

`memmap.h`'s `HI_TOP` is one promise: **the smallest guest zlOS claims to boot
on.** Every DMA buffer sits below it because below it is the only memory we have
said exists. Until now, nothing compared that promise to what the gates
actually booted, and all three of these were simultaneously true:

| what | `-m` it passed | against `HI_TOP` = 256 MiB |
|---|---|---|
| `verify.sh`, `tools/checks/verify-raw.sh`, `tools/checks/verify-iso.sh`, `run.sh` | **none** → 128 MiB | top **half** of the map unbacked |
| `tools/checks/verify-disk.sh`, `tools/checks/verify-clock.sh` | 512 | fine |
| `tools/checks/verify-efi.sh`, `tools/probes/exercise.py`, `tools/run/try.sh` | 1G | 4× the asserted ceiling |

`HI_TOP` is now **`0x40000000`, 1 GiB**, and every QEMU in the tree passes
`-m 1G`. `kernel/tools/checks/check-ram.sh` is the gate: it reads `HI_TOP` out of
`src/arch/x86/memmap.h`,
finds every QEMU launch in every `.sh` and `.py` here, and fails if any of them
passes less than that or passes no `-m` at all. Static — no build, no QEMU, so
it cannot fail because the host is busy. Validated against three planted
defects (a gate with `-m` removed, a gate lowered to `-m 512`, and a brand-new
gate written without one); all three go red, the clean tree goes green.

**The trade, stated rather than implied: zlOS no longer claims to boot on a
256 MB machine.** Nothing in the tree needed 1 GiB — the highest thing claimed
is virtio-gpu's framebuffer ending at 255 MiB — so this buys room, not
correctness. It is worth it because 256 MiB was never a hardware limit; it was
`-m 256` written down once and then asserted against forever, and it had already
started blocking work that has real use for the space above it.

Two consequences worth knowing:

- **The program arena's justification changed.** `src/core/arena.c` sits at 8 MiB
  *because* everything above 128 MiB used to be unbacked on every gate. It is
  still at 8 MiB — moving a live region buys nothing — but it is no longer
  forced there, and the space above the map is now genuinely allocatable.
- **`tools/checks/check-himap.sh` needed widening in the same commit.** Its
  literal-scanner
  matched `0x0` followed by seven hex digits, which cannot express any value at
  or above `0x10000000`. With `HI_TOP` at 1 GiB it would have gone on comparing
  against a ceiling of `0x40000000` while structurally unable to see two thirds
  of the span — reporting a region as covered that it could not look at. That
  is the shape `docs/GUARDS-THAT-DID-NOT-GUARD.md` exists for, and it would have
  been introduced *by* the fix.

The full map — every base and end re-grepped from the file that owns it, the
kernel image end measured, the arithmetic for where a new buffer may go, and one
collision `src/graphics/framebuffer/fb.c`'s map does not list (the SMP AP
stacks at 168 MiB, inside
`sp_buf`'s declared span) — is `kernel/docs/reference/system/memory-map.md`.

## Two silent faults hid five sixths of the desktop (2026-08-20)

"All Applications" is the only route to 47 of the 53 apps. It did not open, and
once it did it drew twelve tiles and then killed the machine. Neither failure
produced a single line of diagnostic anywhere, and both are the same shape: a
call that succeeded at doing the wrong thing.

**1. `APP_CATALOG` was 13. So is `APP_FILES`.** The catalog's id was chosen at
what was then one past `APP_EDIT`; `APP_FILES` was added later and took it.
`reg_open()` asks wm which app each open window is showing, and the desktop
opens a Files window at boot — so `apps`, the dock's grid button and the start
menu all reached `reg_open(13)`, matched the Files window already on screen,
raised it, and **returned 1**. Success, no new window, nothing printed. It
misrouted the other way too: `app_event()` tested `id >= APP_CATALOG` above its
`id == APP_FILES` arm, so every key and click aimed at the Files window went to
`cat_event()`.

**2. `sys2_mono()` and `sys3_mono()` never existed.** `reg_mono()` has
forwarded to them since the slice files were written. **zl resolves a call by
NAME AT RUNTIME**, so an undefined function is not a link error — `build.sh`'s
"undefined symbols: 0" is `nm` counting C symbols, and a zl call site is a
string. It falls through the builtin table in `freestanding/runtime_kernel.c`
to `kfatal("builtin not available in the kernel subset")`, which **halts the
machine**. sys2's six apps are the ones with no icon, so the catalog asks them
for a monogram: scrolling to Kernel Log drew that tile's background and its
category stripe and then froze zlOS. From outside, an empty window.

The same class was sitting on the ThinkPad panel path: `key()`, called one
statement before `panel_console()` moves the console onto the panel our own
driver has just lit. Nothing defines `key()` either.

**Two static checks now cover both, each validated in both directions:**

```
cd kernel
./tools/checks/check-appids.py --selftest   # replants APP_CATALOG = 13, requires a failure
./tools/checks/check-zlcalls.py             # every call site vs. the fn set and the builtins
```

`tools/checks/check-zlcalls.py` reads the builtin names out of
`src/runtime/interp_kernel.c`'s own
`streq(name, "...")` table rather than transcribing them, and it found `key()`
on its first run. **A `_Static_assert` nobody has watched fail is a decoration**
— that rule applies to these too, which is what `--selftest` is for.

## Verify before believing anything

```
cd kernel
./tools/checks/check-appids.py  # no two apps share an id (static, instant)
./tools/checks/check-zlcalls.py # every zl call resolves - an undefined one HALTS the kernel
./tools/checks/check-memmap.sh  # hand-placed buffers do not overlap (static, instant)
./verify.sh        # BIOS golden transcript
./tools/checks/verify-raw.sh    # our own bootloader
./tools/checks/verify-efi.sh    # zlOS as its OWN UEFI application - the ThinkPad's path
./tools/checks/verify-iso.sh    # BIOS and UEFI through GRUB
./tools/probes/probe-dock.py    # the POINTER: hover, press, launch, the menu, dismiss
./tools/probes/probe-resize.py  # the corner grows a window; the title bar still moves it
./tools/probes/probe-drag.py --grab 1500,125 --drop 700,700
./tools/run/try.sh serial       # drive it from the terminal
cd tests/host && ./build.sh && ./wmtest && ./inputtest && ./fbbench && ./tritest
```

**The `probe-*.py` scripts that drive the POINTER are not optional extras.**
Every other gate here types, and the compositor's entire pointer path - drag,
click-to-focus, the close box, the dock, the menu - was dead for hours while
all of them stayed green. See T-15.

`tools/checks/check-memmap.sh` parses the fixed addresses out of `src/kernel.zl` and derives
their sizes from the same constants, so bumping `FS_SLOT` or `HIST_N` re-runs
the arithmetic. It exists because `LINE_BUF`/`HIST_BUF` had been placed inside
`FS_DATA`'s slots 7 and 8: editing RAM file 7 or 8 overwrote the shell's input
line and history ring, and typing at the prompt overwrote those two files.
There is no heap, so nothing catches this at runtime. It reads source and does
arithmetic — no build, no QEMU, so it cannot fail because the host is busy.

The checker now discovers the current map, derives scratch/name sizes from
their owning constants, and has a mutation self-test that plants a duplicate
address and requires rejection. This closes the old `DISK_SCRATCH` blind spot;
new memory owners still need to use a discoverable declaration shape.
**`./build.sh` DOES NOT REBUILD WHAT THE PROBES BOOT, and this will cost you an
afternoon.** `build.sh` produces `kernel.elf`. Every `probe-*.py` boots
`zlOS.iso` (`tools/probes/exercise.py:280` `qemu_argv` → `-cdrom zlOS.iso`),
which is made by
`tools/images/mkiso.sh` and only by that script. `tools/probes/exercise.py:273`
`build()` runs it for you —
so a probe run WITHOUT `--no-build` is honest, and `./build.sh && ./probe-x.py
--no-build` silently tests the kernel you had before your edit.

Measured, the hard way: three consecutive diagnostics of a real bug were run
against a stale ISO. A fix that worked looked like it had failed, was reverted,
and the hunt moved to the wrong subsystem. The tell was a `term_say` printed
*before* another one appearing in the log *after* it — impossible in the source,
and the only explanation was that the source was not what was running.

**If a diagnostic result is impossible, check what you actually booted before
you check anything else.**

## The exec track — running code the kernel was not built with

Three docs, all written from measurement rather than intent:

- `kernel/docs/reference/system/memory-map.md` — every fixed physical address, re-grepped from the
  file that owns it, the kernel image end measured, and **the fact that no gate
  passes `-m` so every address above 128 MiB is unbacked under all of them**.
  Also two collisions `src/graphics/framebuffer/fb.c`'s map does not list.
- `kernel/docs/architecture/system/exec-kill-path.md` — how a program that will not stop is stopped:
  a step budget and a depth cap at `eval`/`exec`, a `longjmp` boundary instead of
  `exit(1)`, and why it is deliberately not the timer interrupt.
- `kernel/docs/DECISIONS.md` §"The exec track" — the level choice (**Level 1,
  ASSUMED not chosen**), and why the boot log still says "no heap".

Gates the exec track added, cheapest first:

```
cd kernel/tests/host && ./build.sh
./arenatest        # the program arena's ceiling      62 checks, no QEMU
./exectest         # `run`, with a fake filesystem    44 checks, no QEMU
./exectest-nofs    # `run`, as it actually ships      32 checks, no QEMU
./killtest.sh      # adversarial: can a script wedge the machine?  14 cases
cd ../.. && ./tools/probes/probe-run.py # `run` in the real compositor
./verify.sh          # BIOS golden transcript
./tools/checks/verify-raw.sh      # our own bootloader - text leg AND framebuffer leg
./tools/checks/verify-efi.sh      # the NATIVE UEFI application (buildefi.sh's output)
./tools/checks/verify-iso.sh      # BIOS and UEFI through GRUB
./tools/checks/verify-sources.sh  # one source list really does reach all four builds
./tools/run/try.sh serial         # drive it from the terminal
```

**`tools/checks/verify-efi.sh` and `tools/checks/verify-sources.sh` are new
(2026-08-18).** Before them,
`buildefi.sh` built `BOOTX64.EFI` and nothing ever booted it - which is how the
address truncation in `boot/efi.c` survived, since
`tools/checks/verify-iso.sh`'s UEFI leg boots the *multiboot* kernel and
`boot/efi.c` is not in that binary.

The desktop has its own probes, all of which boot the real thing:

```
./tools/probes/probe-term.py    type five commands into the shell, assert each result
./tools/probes/probe-apps.py    five apps in five windows, running at once
./tools/probes/probe-snake.py   snake keeps playing while another window is dragged
./tools/probes/probe-smp.py     band rendering on 4 real cores draws identical pixels
./tools/probes/probe-frame.py   the frame timer is a measurement, not a number
./tools/probes/probe-edit.py    the editor: a window, typing, ESC saves and closes
python3 ./tools/probes/probe-files.py  Files + zlEDIT survive a killed QEMU and cold boot
./tools/probes/probe-drag.py --no-tablet     a window really moves
```

`tools/run/try.sh` GUI mode is **verified working** (2026-08-17). It was booting
`-kernel kernel.elf`, and QEMU's own multiboot loader never supplies the
framebuffer tag — it prints `multiboot knows VBE. we don't` — so `console_init()`
never reached `fb_setup()` and every graphical demo answered "needs the
framebuffer console". It boots `-cdrom zlOS.iso` now; GRUB supplies the tag.
Confirmed by QMP screendump: 1920×1200, desktop with windows, taskbar and a
System Monitor reading the real CPU. `virtio-gpu-pci` is still attached so `y`
keeps its own device.

**If you need `-kernel` for a fast text-only loop, that path is fine** — it boots,
the shell responds, and `verify.sh` gates it. It just cannot do graphics.
## MP-00 dependency/SBOM closure slice — 2026-08-24

`kernel/metadata/dependency-lock.json` is now `zlos.host-dependency-lock.v2`. It retains
the original 15 invoked command and two firmware identities, and joins 91 exact
runtime files to 156 recursively installed binary packages. Each package records
binary version/architecture, source package/version, raw and resolved dependency
edges, and local archive custody. The generator batches package ownership and
installed metadata discovery and rejects missing commands, binary drift, wrong
aliases, missing firmware, missing packages, unresolved dependencies, invented
source custody and missing build identity.

Evidence ceiling: `PASS_WITH_OPEN_SUPPLY_GAPS`. No package source archive is
retained and no offline rebuild has been run. This materially advances EV-004
but does not complete it or EV-005. The dependent toolchain manifest, build graph
and joined evidence registry were regenerated after the lock changed.

EV-008 is closed for the current batch. `kernel/metadata/wrapper-registry.json` hashes
and classifies all 141 current shebang wrappers, including 17 legacy/action/
probe policy gaps that are explicitly not landing authority. The only supported
entry remains `gates/run-land-gate-contained.sh start`; `check-land-gate.py`
proves 71 mandatory seams, including memory-map mutation and UI-scale
contracts, and rejects masked child-failure and masked final-exit mutations.
The adversarial registry is 19/19 canaries caught. A stale zero-byte
`kernel64.elf` was discovered by CAN-006 and rebuilt under a two-core,
low-priority limit; its final 2,702,592-byte output matches the registered hash.

EV-026's largest local gap is also closed. The decision generator now derives
normalized semantics for all 47 legacy labels directly from their numbered
source sections, retaining the exact heading, line, section hash, feature scope
and evidence ceiling. Zero legacy semantics remain open. EV-026 stays partial
because repository-wide history outside `kernel/docs/DECISIONS.md`, independent
approval, automatic migration and a booted history viewer are still absent.

EV-014 distribution work is implemented but not promoted. `run-benchmarks.py`
can now produce a v2 receipt with seven independent raw samples and p50/p95/
peak summaries. The first low-priority, two-core run was terminated after the
host load rose above the containment tolerance; it never replaced the receipt.
The exact historical v1 receipt remains valid through its frozen runner hash
and still states `NOT_RECORDED` percentiles/peak, four over-budget metrics and
zero native-target measurements. Run v2 only on an actually idle host.
