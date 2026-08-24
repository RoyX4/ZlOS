# Universal proof and promotion gates

This file defines what “done” means for every phase, driver, service, app,
language feature, agent and release artifact.

## Evidence vocabulary

| Label | Meaning |
|---|---|
| `C` | claim exists in prose, UI or metadata |
| `S` | substantive source inspected |
| `R-build` | included in the active build graph |
| `R-image` | present in the shipped image/package |
| `R-init` | initialized or registered on target |
| `R-user` | real user route reaches it |
| `T-host` | deterministic host test executes real shared logic |
| `T-target` | test executes on the target architecture/system |
| `QEMU` | reproducible virtual-machine behavior |
| `H-harness` | host-side harness used with physical hardware |
| `H-native` | zlOS native boot and real hardware effect |
| `P` | partial, stub, disabled, disconnected or otherwise unproven |
| `F` | known false-green or contradictory evidence |
| `X` | deliberately rejected design/claim used only as a negative test |

Labels do not imply one another beyond the exact receipt. Physical proof of an
xHCI controller plus USB storage does not prove USB keyboard, audio or camera.
An Intel modeset host harness does not prove the path is callable during native
zlOS boot. An AArch64 build/boot does not prove target tests.

## Universal completion contract

Every item answers, in machine-readable form:

1. exact user/system outcome;
2. owning process/service/provider and source/package artifacts;
3. versioned inputs, outputs, persistent state and protocol;
4. required authority, delegation and live revocation;
5. byte/count/time/memory/CPU/I/O/connection limits;
6. start/readiness/stop/crash/recover/update/rollback lifecycle;
7. partial failure, timeout, cancellation, queue-full, peer/device loss behavior;
8. persistence scope, durability, migration, backup and retention where relevant;
9. keyboard, semantic accessibility, scale, contrast, motion and localization;
10. cache/asynchrony and latency/frame/operation budgets;
11. privacy, consent, secrets, redaction, provenance and audit;
12. source/build/image/init/user/host/QEMU/hardware evidence independently;
13. a planted mutation proving the verifier detects a missing or false field;
14. weakest remaining link.

“Not applicable” is explicit and justified; it cannot be silently omitted.

## Deterministic host gates

- normal, debug and sanitizer builds where supported;
- unit, property/model, corpus, fuzz and differential tests over production
  logic rather than copied test-only algorithms;
- allocation, map, queue, write, flush, provider, peer and lifecycle failure at
  every numbered step;
- checked arithmetic, zero/one/max/max+1, negative/sign-bit, wrap and overlap;
- test inventory parity: declared, compiled, registered, executed, skipped,
  failed and shipped;
- wrappers propagate the first failure and timeout is not success;
- one mutation per required registry/manifest/receipt field.

## Kernel and process hostile gates

- negative, `INT64_MIN`, maximum, gap and null syscall indices;
- invalid input and output pointers, first/last-byte unmapped, read-only output,
  one-past object, canonical overflow and fault during copyout;
- malformed/overlapping/noncanonical/W+X/zero-size/overflow ELF segments and
  wrong entry/header sizes;
- page-map failure at each page with exact PTE/range/refcount rollback;
- fork/exec/credential transition matrix, including normal-user attempts to
  obtain effective UID/GID 0;
- ordinary-process denial for raw physical/MMIO, DMA, mount, scanout/input grab,
  kill/debug and power operations;
- only offender termination while sibling, supervisor, desktop and logs survive;
- preemption, lock ordering, interrupt nesting, OOM and teardown stress.

## Capability and IPC hostile gates

- source lacks COPY/DERIVE while destination allows insert, and vice versa;
- depth and total-node derivation bounds at max/max+1;
- revoke across 9, 64 and larger sibling containers with no silent truncation;
- destination empty/full/last-slot and receiver-too-small;
- failure on every transferred handle preserves source/destination tables,
  queue bytes and object refs exactly;
- peer close before/at/after commit, timeout, cancel and duplicated/replayed ID;
- guessed/global object IDs, stale generations, unrelated aliases and grant-self;
- revoke serialized with spawn; killed parent/orchestrator leaves no live child
  or valid handle.

## Driver and hardware gates

- descriptor schema and required capability negotiation, rejecting unknown
  mandatory bits;
- match/admission/resource conflict and wrong revision/width/coherency;
- failure during every reserve/start step enters Aborting and releases all BARs,
  IRQs, DMA pages, domains, mappings, handles and endpoints;
- empty/full/wrap queues, late completion, cancel, timeout, reset and replay;
- hot-unplug during idle/I/O/interrupt/recovery and generation-safe rebind;
- IOMMU default-deny and attempted DMA outside granted pages; no-IOMMU paths are
  labelled non-isolated and revoke DMA before page reuse;
- host simulator/protocol oracle, QEMU device, then exact physical controller/
  class device first-use/reuse/reset/teardown/recovery;
- unsupported hardware degrades to named fallback or fails clearly.

## Storage and package gates

- corrupt partition/filesystem/archive/package lengths, counts, checksums,
  overlaps, cycles, traversal, symlinks and decompression bombs;
- disk full at metadata/data/journal boundaries and concurrent rename/unlink;
- power cut after every ordered write/flush; recover exactly old or new state;
- removable device loss with dirty cache, late completion and remount;
- install stages every file/tool/role/service/grant, validates all, then publishes
  atomically; failure at each stage restores exact prestate;
- uninstall stops processes, revokes handles, deregisters callbacks/tools/roles,
  removes package-owned files and preserves user data only by declared policy;
- signing key ID, trust chain, algorithm/policy version, rotation, revocation and
  anti-rollback proofs are separate from checksums.

## Display, input and performance gates

- software oracle versus each accelerated scene before promotion and after reset;
- no compositor allocation in frame commit; retained client/shell surfaces and
  bounded damage/occlusion remain pixel-correct under refusal/eviction;
- surface geometry/stride/format overflow, mapping refs, owner/grantee rights and
  owner/peer death;
- only compositor owns scanout; only focus/grab authority receives global input;
- one input timestamp flows route -> app -> damage -> commit -> present;
- typing, hover, drag, resize, scroll, stacked-window and animation distributions
  include p50/p95/p99/max and missed/lost/drop counters;
- workload, resolution, scale, backend, clock, sample count, machine and artifact
  identity accompany every performance number;
- QEMU/TCG is functional evidence, not physical latency evidence.

## Application and workflow gates

- registry exact name/ID/icon/manifest/entry/draw/event/tick/image/route/readiness/
  close fields, with one independent mutation each;
- empty/create/open/edit/save/export/share/close/reopen plus denied/offline/
  malformed/disk-full/dependency-loss/crash/restart;
- independently inspect files, messages, jobs or system state; a toast/label does
  not prove success;
- app crash or package removal leaves no process, surface, input grab, file grant,
  notification callback or background task;
- per-user state isolation and concurrent sessions;
- keyboard-only route and assistive-provider action replay.

## Browser and parser gates

- parser/layout/JavaScript/decoder fuzzing under sanitizers with minimized corpus;
- redirects, cancellation, caching, cookies/storage, downloads, malformed and
  partial responses from a deterministic origin server;
- kill every browser worker at each navigation/subresource/storage stage;
- cross-origin, mixed-content, certificate/name/time/root, site-permission and
  download-sandbox denial;
- fixed-width/scale/theme/locale page goldens and semantic-tree comparisons;
- memory/CPU/timer/process/network quotas and recovery after hostile pages;
- unsupported standards produce honest named behavior, never painted simulation.

## Visual, accessibility and localization gates

- one token authority and no undeclared visual literals in product components;
- every component state at supported scale/theme/contrast/locale/RTL;
- text/layout metrics plus regional color/palette/hue/structure comparisons;
- motion event/damage/frame sequence, reduced-motion behavior and deadline proof;
- semantic-tree validator, focus traversal, screen-reader journey and actions;
- 200% text/reflow without clipped critical actions, high contrast, captions,
  remapping and switch/dwell routes;
- locale numbers/dates/units/timezone, pluralization, bidi and IME;
- screenshots name artifact, mode, scale, theme, locale and evidence ceiling.

## Agent and operations gates

- every agent effect names current live authority, tool version, inputs, result,
  committed state and rollback/compensation;
- revoke-then-spawn, root parent kill, nth tool failure, forged receipt, secret
  exfiltration, budget exhaustion and independent reviewer rejection;
- public-demo action and VNC/WebSocket data planes require the same unguessable
  live lease authority;
- allocation/port/PID/overlay/launch/proxy failure at each step restores exact
  prestate; PID identity and process death precede slot reuse;
- CPU/RAM/PID/disk/time/egress quotas, graceful deadline, force-reap and zero
  orphans;
- update/release rollback, recovery boot and disaster restore rehearsed from
  immutable artifacts.

## Promotion record

Each promoted claim stores:

- feature/provider/service/app IDs and version;
- source commit plus dirty patch/config/tool/dependency digests;
- artifact/package/image/boot-origin identities;
- exact command/workload/corpus/seed and exit status;
- evidence label and environment/topology;
- measured results and raw log/screenshot/video/journal location;
- known exclusions, failures and weakest link;
- reviewer identity/result and rollback generation.

Promotion is per claim. A phase may advance with openly deferred profile items,
but neither the feature registry nor release UI may imply those items exist.
