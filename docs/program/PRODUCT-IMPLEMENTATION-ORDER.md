# zlOS product-first implementation order

Date: 2026-08-24

This is the human execution view of the complete 906-feature program. It does
not replace [`PHASES.md`](PHASES.md), the machine-checked dependency graph, or
the specialized driver/application/browser backlogs. It prevents evidence work
from being mistaken for product delivery.

## Rule

The main queue must deliver a visible or load-bearing zlOS capability. Evidence,
receipts, mutation tests and provenance are completion conditions attached to
that capability. They are not a separate product milestone unless they directly
fix a false green, data-loss risk, unsafe release path or host-safety problem.

```text
implement useful behavior -> test its contract -> boot it -> measure it
                          -> prove required hardware -> retain the receipt
```

## Product waves

### Wave 1 — make the current system fast and dependable

Preserve everything already working. Finish the measured rendering/input path:
retained client content and chrome, bounded damage/occlusion, glyph/icon caches,
deadline-aware pacing, low-latency input, asynchronous storage, idle power and
safe software fallbacks. Repair current app routes and persistence defects as
they are found. Do not add decorative breadth while ordinary interaction still
misses the target frame budget.

Product exit: the physical target can boot, drag, type, scroll, open/close apps,
use Files/editor/browser and persist data without dropped input or unexplained
long frames.

### Wave 2 — finish the owned boot and recovery system

Keep BIOS/raw, GRUB/Multiboot and native UEFI acquisition routes, but converge
them on one bounded zlOS boot manager and typed immutable handover. Add exact
current/previous/recovery/debug/assertion generations, hostile parser handling,
early diagnostics, firmware inventory and atomic rollback. Boot code acquires
and verifies the system; it does not become a second kernel.

Product exit: current, previous and recovery generations are selectable and
identifiable; malformed or mismatched kernels/configuration fail closed; both
BIOS and UEFI paths reach the same kernel contract.

### Wave 3 — real 64-bit processes and fault containment

Deliver per-process address spaces, guarded stacks, safe usercopy, robust ELF
admission, exec/threads/shared memory, timer preemption, scheduling, signals,
wait/teardown and memory pressure. Preserve the cooperative path only as a
diagnostic fallback until the preemptive path is proved stronger.

Product exit: multiple ordinary 64-bit programs run concurrently; one hostile
or crashing program cannot read kernel/device memory or kill the desktop.

### Wave 4 — handles, typed IPC, supervision and sessions

Replace ambient IDs and raw-pointer messages with generation-checked handles,
explicit rights and bounded versioned zlIDL protocols. Add transactional handle
transfer, backpressure, cancellation, deadlines, peer-death cleanup, a service
supervisor, users, credentials, sessions, secrets and portal-mediated grants.

Product exit: drivers, services and applications can live in separate processes
with explicit authority, bounded failure and restartable lifecycles.

### Wave 5 — storage, driver framework and real hardware breadth

Unify device discovery, resource admission, start/recovery/removal and rollback
behind common provider contracts. Deepen asynchronous block I/O, VFS, zlfs,
mounts, permissions, file-backed memory, removable media and transactional
packages. Then add normalized hardware families rather than copying one driver
per research repository: NVMe/AHCI/USB storage; I219/e1000/RTL/USB networking;
GOP/BGA/virtio/VMware/Intel display; PS/2/USB/I2C/tablet input; audio, camera,
sensors, power and virtual devices.

Product exit: a failed or removed device leaks no BAR, IRQ, DMA page, handle or
published endpoint; storage remains old-or-new across interruption; selected
physical providers complete real work rather than only enumerate.

### Wave 6 — compositor, input, audio and the shared design system

Move scanout, windows, surfaces, focus and global input policy behind owned
services once Waves 3-5 can host them safely. Complete multi-display,
workspaces, scaling, color, text shaping, themes, components, animations,
reduced motion, screen-reader semantics, magnification, high contrast,
localization and input methods. Add AudioServer and capture portals with bounded
shared rings and privacy indicators.

Product exit: the desktop can recover from provider/service failure, every
primitive works by keyboard and semantics, and visual polish remains within the
frame/input budgets from Wave 1.

### Wave 7 — application platform and migration of everything current

Deliver the stable process ABI, AppKit/UI SDK, manifests, launcher, package
lifecycle and shared Files/Settings/Clipboard/Notifications/Open/Save/Print/
Share/Secrets portals. Add document models, undo/redo, autosave/recovery,
MIME/thumbnail/search and isolated decoder workers. Then migrate every one of
the 63 current named implementations, All Applications and all 24 games out of
kernel ownership without deleting a proven fallback early.

Product exit: every current app has a real install/launch/use/save/close/crash/
restart/update/remove lifecycle, not merely a menu tile or first painted frame.

### Wave 8 — complete browser, broad applications and compatibility

Split browser UI, network, renderer, storage and decoder responsibilities into
restricted processes. Deepen HTML/CSS/layout, JavaScript, navigation, downloads,
permissions, accessibility and crash recovery. Build the selected productivity,
office, PDF, media, creative, communication, PIM, administration, accessibility,
learning and developer applications on the shared platform. Add native ports,
selected unmodified Linux workloads, selected unmodified Windows workloads,
Windows/Linux virtual machines, containers and CPU/device emulation only behind
the same authority and resource contracts.

Product exit: hostile web/content workers cannot own files, devices, secrets or
the desktop; complete representative workflows work in each application family;
the compatibility UI states exactly which Windows/Linux programs, APIs and guest
workflows are proved instead of claiming blanket compatibility.

### Wave 9 — zl development, agents, operations and self-hosting

Finish the system ABI and zl compiler/runtime/SDK inside zlOS; add debugger,
profiler, package/build tools and compatibility headers. Add capability-bounded
agent plans and tool brokerage only after processes, handles, packages and
portals exist. Finish recovery media, updates, remote demo/management and release
operations. Finally rebuild the matching zlOS toolchain and system inside zlOS.

Product exit: a clean machine can recover/install the system; zlOS can build the
declared matching generation inside itself; agent and remote actions cannot
bypass normal authority; release and rollback use exact signed generations.

## Immediate queue

The next product implementation slice is Wave 1, not completion of the old
23-row evidence batch:

1. rerun the contained current build/QEMU suite only when the host is idle;
2. preserve the current boot/app/storage/network baseline;
3. measure the current physical compositor/drawing owner;
4. implement the smallest retained-chrome/glyph/damage change that removes that
   measured owner while keeping the software renderer as oracle;
5. verify app open/close, typing, drag, scroll, Files/editor/browser persistence;
6. then begin the Wave 2 boot-manager vertical slice and Wave 3 process work in
   dependency-safe parallel seams.

The physical ThinkPad, legal licence choices, independent security review and
release signing remain honest external gates. They must not block safe local
implementation, and local implementation must not fabricate their approval.

## Mapping to the canonical phases

| Product wave | Canonical phases |
|---|---|
| 1 | MP-01 plus current performance work |
| 2 | MP-02 |
| 3 | MP-03 |
| 4 | MP-04 |
| 5 | MP-05, MP-06, MP-08, MP-09, MP-19 |
| 6 | MP-07, MP-09, MP-10 |
| 7 | MP-11, MP-12 |
| 8 | MP-13, MP-14, MP-15 |
| 9 | MP-16, MP-17, MP-18, MP-20 |

The detailed feature, driver, service and application inventories remain the
scope authority. This document changes execution emphasis, not destination
breadth.
