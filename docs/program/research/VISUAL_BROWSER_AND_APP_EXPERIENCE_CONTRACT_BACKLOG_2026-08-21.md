# Visual, browser, and application experience contract backlog

Date: 2026-08-21

Purpose: convert the clean-room visual/browser/app plan into independently
implementable and independently falsifiable work. Contract IDs are stable;
delivery order is dependency order, not aesthetic priority.

This backlog is a **dependency-ordered overlay, not a standalone foundation**.
It deliberately does not duplicate kernel, driver, process, IPC, filesystem,
network, package, supervisor, device, or service contracts already defined in:

- [`IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md`](IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md)
- [`DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md`](DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md)

When a VX contract names one of those foundations, the canonical prerequisite
is:

| VX dependency phrase | Canonical prerequisite contracts |
|---|---|
| process, user boundary, handle or typed IPC | `P3.1`–`P3.4`, `P4.1`–`P4.3`; `DA-17`, `DA-18` |
| supervisor, app/process lifecycle, session/auth | `P4.4`, `P4.5`; `DA-19`, `DA-20`, `DA-25`, `DA-26` |
| display, compositor, surface or input provider | `P5.3`; `DA-09`, `DA-10`, `DA-20` |
| evidence, telemetry, logger or crash service | `P0.1`–`P0.4`, `P5.1`; `DA-01`, `DA-03`, `DA-08`, `DA-08O`, `DA-25N` |
| File/VFS, file portal, restricted parser or decoder | `P5.2`, `P5.6`; `DA-21`, `DA-21F` |
| settings/configuration | `DA-20C` |
| Network/Resolver/TLS, Secrets or Clock | `P5.5`, `P6.1`–`P6.3`; `DA-13`, `DA-15`, `DA-22` plus the session-owned secret store required by `P4.5` |
| package, manifest, install/update or launch | `P5.7`, `P7.1`–`P7.4`; `DA-23`, `DA-23L`, `DA-24` |
| PTY/process tree | `P3.1`–`P3.4`, `P4.5`; `DA-17`, `DA-18`, `DA-25` |
| AudioServer/media provider | `P5.4`; `DA-14` |
| device/power/camera/capture provider | `DA-04`–`DA-08`, `DA-15`, `DA-16`, `DA-26` |
| boot, QEMU, physical or contradiction promotion | `DA-36`, `DA-40`–`DA-44` |
| agent tool broker or public-demo operations | `P10.1`–`P10.5`, `P11.1`–`P11.5`; `DA-35`, `DA-38` |

Those contracts must be complete before the consuming VX contract starts. The
VX ordering only resolves dependencies inside this visual/product overlay.

Each contract must leave four receipts:

1. artifact/source identity;
2. deterministic host/protocol proof;
3. target/QEMU interaction and failure proof where applicable;
4. cleanup/rollback proof showing no leaked process, handle, grant, surface,
   file, device ownership, or false success.

## Global rules

- No contract is complete because a screen exists.
- Every user action has authority, state, timeout/cancellation, result and
  failure semantics.
- Every visual component has semantic/accessibility state.
- Every app workflow validates the durable or external effect separately from
  the UI message.
- Every parser/decoder accepts `(bytes, length)` and produces bounded output.
- Every transferred object is an opaque generation-tagged handle.
- Every aggregate check reports and fails on every required subfield; mutation
  proof must show each field can independently make the check fail.
- Host, screenshot, QEMU and physical evidence remain separate.

## Dependency map

```text
VX-00 truth/evidence
  -> VX-01 registry truth
  -> VX-02 tokens -> VX-03 theme -> VX-04 type -> VX-05 icons -> VX-06 motion
  -> VX-07 semantics -> VX-08 components -> VX-09 layout/localization
  -> VX-10 input/focus
  -> VX-11 surfaces -> VX-12 compositor -> VX-13 session/shell -> VX-14 windows
  -> VX-15 notifications -> VX-16 portals -> VX-17 manifest -> VX-18 lifecycle
  -> VX-19 catalogue
  -> current app migrations VX-20..25
  -> browser split VX-26..33
  -> product app families VX-34..44
  -> agent/recovery/demo VX-45..47
  -> proof/promotion VX-48..53
```

## Phase 0 — truth and evidence

### VX-00 — Visual/app/browser evidence registry

**Depends on:** existing source manifest and build artifact receipts.

**Deliver:** a machine-readable record per feature with repository/commit,
source path, build/image/init/user-route, screenshot vintage, host/QEMU/physical
run, exact behavior, date, artifact digest and known contradiction.

**Invariants:** evidence is per behavior; no transitive promotion; historical
screenshots are labeled; source names do not imply function.

**Proof:** seed one false source-only, old-screenshot, build-only, QEMU-only and
adjacent-hardware promotion; the validator rejects all five.

### VX-01 — Current application registry repair and oracle

**Depends on:** VX-00.

**Deliver:** generated single-source registry for every app/shell surface with
unique ID, exact name, family, icon, size constraints, entry point, draw/event/
tick requirements, package/image membership and launch routes.

**Invariants:** no blank IDs; no duplicate ID/name; every declared route reaches
the same app; dense numeric ranges never imply existence.

**Proof:** independently remove/mutate ID, name, icon, size, constructor, draw,
event, tick, package, catalogue, dock, shell and launch-ready nonce. Every
mutation fails. Reproduce and then eliminate the Maze/blank-ID false green.

## Phase 1 — visual foundation

### VX-02 — Versioned semantic token schema

**Depends on:** VX-00.

**Deliver:** generated color/type/spacing/radius/elevation/motion/layer/density
schema with defaults, ranges, fallback and semantic relationships.

**Invariants:** raw literals are confined to token artifacts; unknown mandatory
fields reject; semantic state colors do not silently follow theme accent;
focus/selection/error roles remain distinguishable.

**Proof:** source scan, schema round-trip, missing/duplicate/wrong-type/unknown-
required cases, every accent contrast pair and one-literal mutation.

### VX-03 — Atomic theme and appearance service

**Depends on:** VX-02.

**Deliver:** per-user theme profile, accent, contrast, transparency, density,
wallpaper and scale settings; preview/apply/revert; signed asset manifest;
atomic persistence and rollback.

**Invariants:** partial theme never publishes; failed preview expires; broken
theme falls back without losing user data; lock/recovery has safe built-in
theme.

**Proof:** failure-inject every write and publish step; reboot during apply;
malformed assets; contrast-invalid theme; provider death during preview.

### VX-04 — Scalable text, shaping and glyph cache

**Depends on:** VX-02.

**Deliver:** UI/reading/code/terminal roles, scalable rasterization, metrics,
fallback, shaping, bidirectionality, combining marks, selection/caret hit tests,
bounded cache and deterministic eviction.

**Invariants:** no zero/negative metrics; terminal monospace stays exact;
fallback cannot recurse forever; cache exhaustion degrades, never corrupts;
text scale is independent of display scale.

**Proof:** multilingual corpus, RTL, combining/emoji fallback, huge glyph churn,
200%/300% text, cache failure injection, browser/editor/terminal differential.

### VX-05 — Semantic icon and asset pipeline

**Depends on:** VX-02, VX-03.

**Deliver:** named icons with optical sizes/states/directionality, bounded build
artifact, signed app assets and decoder-worker boundary.

**Invariants:** atlas position is not API; missing icons get explicit fallback;
untrusted images never parse in shell; selected/disabled/attention states keep
meaning at every scale.

**Proof:** missing/duplicate name, oversized/decompression bomb, RTL icon,
1x/1.25x/1.5x/2x/3x snapshots, decoder crash and fallback rendering.

### VX-06 — Motion engine and reduced-motion policy

**Depends on:** VX-02.

**Deliver:** named curves/durations, monotonic clock scheduling, interrupt/
reverse/retarget behavior, damage generation and static/reduced alternatives.

**Invariants:** PIT/frequency changes do not alter duration; final state is exact;
interruption cannot leak grabs/modal/focus; no infinite attention animation;
reduced motion removes nonessential travel.

**Proof:** fixed tick vectors, dropped-frame simulation, mid-animation reverse,
window death, reduced-motion traces and damage-mutation check.

## Phase 2 — components, semantics, and input

### VX-07 — Accessibility tree, assistive providers and control center

**Depends on:** VX-02; process/IPC handle foundation; `DA-09` display provider
for magnification/color transforms, `DA-10` input provider for remapping and
alternative input, `DA-20C` settings service for profiles, and `DA-14`
AudioServer/provider seam for speech and caption-audio integration. Early host
proof may use deterministic fake providers, but target completion requires the
named canonical contracts.

**Deliver:** versioned tree updates with stable IDs, roles, names, descriptions,
states, values, bounds, text ranges, actions and privacy/redaction markers;
screen-reader output with speech and braille provider interfaces; magnifier,
cursor scale, large-text, high-contrast and color-filter services; caption and
transcript routing; sticky/filter/slow keys, remapping, switch control and dwell
input; and an Accessibility Center that previews, persists, reverts and reports
availability of every feature.

**Invariants:** tree is acyclic and owned; updates are bounded/atomic; peer death
removes subtree; action checks current handle and generation; protected content
is not exposed; assistive-provider death leaves keyboard recovery and visible
fallback; lock/session/privacy policy redacts speech, braille, captions and
history; input transformations are ordered, reversible and never trap the user
without an emergency reset path.

**Proof:** cycle/alias/stale ID, oversized update, peer death, focus/action replay,
redaction, a foundation semantic-node fixture, full screen-reader action
journey, speech/braille provider crash, magnifier and color-filter screenshots,
live captions, switch/dwell control, sticky/filter/slow-key timing vectors,
remapping conflict, lock privacy, reboot persistence, failed preview rollback
and emergency recovery with no pointer device.

### VX-08 — Primitive component library

**Depends on:** VX-02, VX-04, VX-05, VX-07.

**Deliver:** the complete primitive set named in the clean-room plan, each with
layout, rendering, events, semantics and state model.

**Invariants:** disabled cannot fire; busy actions deduplicate or reject;
focus-visible differs from pointer hover; invalid state is named and announced;
minimum touch target and clipping rules hold.

**Proof:** state matrix screenshot and semantic snapshot for every primitive;
screen-reader action coverage over that snapshot; keyboard/pointer/touch
actions; hostile labels; locale expansion; mutation of every required state.

### VX-09 — Responsive layout, scale and localization

**Depends on:** VX-04, VX-08.

**Deliver:** constraints, intrinsic sizing, wrapping, scroll/virtualization,
per-monitor fractional scale, RTL mirroring and locale/time/number/unit services.

**Invariants:** checked geometry; no critical action clips at 200% text;
resizing never yields negative/overflow dimensions; focus order follows logical
order; virtualization preserves semantic identity.

**Proof:** narrow/wide/tall/4K matrices, 30–100% longer strings, RTL, rapid scale
and monitor transitions, huge list and integer-boundary fuzzing.

### VX-10 — Focus, shortcuts and input ownership

**Depends on:** VX-07, VX-08, input service.

**Deliver:** keyboard traversal, focus scopes, modal trap/escape, accelerators,
global shortcuts, pointer capture, drag ownership, IME and shortcut discovery.

**Invariants:** one focus owner per seat; only session policy owns global keys;
peer death releases capture; hidden/disabled controls cannot focus; signal/input
events are bounded and loss-accounted.

**Proof:** focus-order replay, conflicting shortcut, modal nesting, app crash
while grabbed, device disconnect, IME composition and keyboard-only journeys.

## Phase 3 — surfaces, compositor, and shell

### VX-11 — Window/surface protocol

**Depends on:** process/IPC handles, VX-02, VX-10.

**Deliver:** generation-tagged window/surface handles; map/draw/present/resize/
embed/capture/share/destroy rights; bounded buffers, damage and frame callbacks.

**Invariants:** compositor-exclusive scanout; checked width*height*stride;
destination capacity reserved before transfer; resize commits atomically;
revocation precedes memory reuse.

**Proof:** overflow/noncanonical formats, quota exhaustion, nth-buffer failure,
stale handle, owner death, capture denial, resize rollback and forged present.

### VX-12 — Compositor scene and recovery

**Depends on:** VX-05, VX-06, VX-11.

**Deliver:** scene ownership, z-order, clipping, damage, occlusion, software
oracle, backend interface, cursor, scanout, fence/deadline and reset fallback.

**Invariants:** client cannot draw outside surface; protected surfaces cannot be
captured; late frame/fence cannot target reused surface; backend failure returns
to software/GOP without losing session control.

**Proof:** randomized scene differential, overlapping/transparent/rotated cases,
backend hang/reset, stale fence, cursor edge, capture policy, sustained frame
budgets at supported resolutions.

### VX-13 — Session shell and lifecycle

**Depends on:** VX-03, VX-07..12, session/auth service.

**Deliver:** login/lock/unlock/logout, top island, dock, Activities/overview,
catalogue, Run/command palette, workspaces, status, quick settings and power UI.

**Invariants:** lock hides/revokes prior content; readiness waits for dependencies;
degraded providers are visible; logout reverses startup and proves process death;
global actions require exact authority.

**Proof:** dependency timeout/crash, lock during capture/drag/modal, failed login,
session restart, orphan-window check, ordinary-app shutdown denial and full
keyboard route.

### VX-14 — Window interaction and multi-monitor

**Depends on:** VX-12, VX-13.

**Deliver:** move/resize/min/max/snap, tabs, Alt-Tab, workspaces, overview,
multi-monitor placement, per-monitor scale, hotplug and restore.

**Invariants:** visible recovery point remains; snap/restore geometry survives
scale changes; window never becomes irretrievable; modal/focus ownership stays
within workspace/session policy.

**Proof:** geometry boundaries, rapid hotplug, monitor removal mid-drag, scale
transition, workspace switching with modal, app death and persisted restore.

### VX-15 — Notifications, quick settings and background tasks

**Depends on:** VX-13, portal foundation.

**Deliver:** source-attributed toasts/history, urgency/privacy/actions, progress,
cancel, recording/capture indicators and actual provider-backed toggles.

**Invariants:** lock-screen redaction; app cannot spoof system source; action
handle expires; progress owner death closes or marks failed; unavailable setting
never appears as successful toggle.

**Proof:** spoof, flood/quota, stale action, provider loss, lock privacy, cancel
race and restart/history persistence.

## Phase 4 — app platform

### VX-16 — Permission portals and chooser patterns

**Depends on:** VX-07, VX-10, handle authority, service layer.

**Deliver:** open/save/open-with/share, clipboard, secrets, notification,
capture, camera, microphone, device and location portals.

**Invariants:** grant is narrow, user-visible, revocable and bound to live app
identity; cancel returns no handle; selected resource cannot change under name;
session/peer death revokes transient grants.

**Proof:** confused-deputy attempts, stale dialog/app, selection swap, cancel/
timeout, revoke in use, lock/logout and wrong-app handle reuse.

### VX-17 — Application manifest, install and update

**Depends on:** package/service contract, VX-05, VX-16.

**Deliver:** canonical signed manifest and atomic install/update/rollback/
uninstall including files, assets, entry points, services, permissions,
localizations, licenses and data policy.

**Invariants:** authenticate before trust; traversal/link/bomb/script input rejected;
unknown required capability rejects; publication is all-or-nothing; uninstall
revokes/stops/unregisters before delete.

**Proof:** failure at every stage, key revoke/rotation/rollback, wrong arch/ABI,
dependency conflict, crash/reboot recovery, live app during update and exact
pre/post object comparison.

### VX-18 — Application lifecycle, crash and restore

**Depends on:** VX-11, VX-17, supervisor.

**Deliver:** launch/readiness/background/suspend/stop/exit/crash/restore/
quarantine state machine and versioned restore snapshots.

**Invariants:** ready nonce matches process identity; close deadline is bounded;
unsaved negotiation cannot stall session forever; crash loop backs off; exit
proves all child processes/handles/grants/surfaces gone.

**Proof:** wrong PID/nonce, start timeout, crash at every state, kill parent/
child, incompatible restore, repeated crash, logout deadline and leak census.

### VX-19 — App catalogue/search/provenance

**Depends on:** VX-01, VX-17, VX-18.

**Deliver:** generated searchable catalogue with categories, capabilities,
provenance, install/launch state, accessibility metadata and launch receipts.

**Invariants:** catalogue is derived from shipped admitted artifacts; no static
dense-range inference; hidden/disabled/incompatible apps explain why; search
cannot launch ungranted operations.

**Proof:** every registry mutation from VX-01, package removal/update, corrupt
icon, incompatible app, launch crash and provenance mismatch.

## Phase 5 — current application migrations

### VX-20 — System-observability app family

**Apps:** About, System Monitor, System Info, Kernel Log, Services, Disk Usage,
Network, Benchmark, Framebuffer, Font Atlas, Pointer.

**Depends on:** VX-18, typed telemetry/logger/device services.

**Deliver:** read-only snapshots/streams, filters, copy/export, exact source/
backend/time, degraded state and separately authorized actions.

**Proof:** provider restart, counter wrap, missing sensor, redaction, huge log,
stale action, ordinary-user privileged denial and receipt verification.

### VX-21 — Files, editor and data-inspection family

**Apps:** Files, Text Editor, Sticky Notes, Regex Tester, Text Diff, Checksum,
Hex Viewer, Archive Manager, Image Viewer.

**Depends on:** VX-16, File service, decoder workers.

**Deliver:** handle-based open/save, safe-save transaction, dirty/undo/recovery,
huge-file paging, search, hostile archive/image isolation and recent items.

**Proof:** disk full, rename/flush failure, app/decoder crash, malformed input,
symlink/traversal/bomb, concurrent change, revoke mid-save and reboot recovery.

### VX-22 — Terminal, console and launch family

**Apps:** Terminal, Console (`tty1`), Run.

**Depends on:** VX-18, PTY/process/session services.

**Deliver:** PTY tabs, process tree, resize/signals/exit, scroll/search/copy,
command parsing and explicit recovery-console role or retirement migration.

**Proof:** hostile arguments/environment, child fork/exec failure, terminal
death, signal permissions including signal 0, huge output/backpressure, resize,
logout and no orphan children.

### VX-23 — Settings and personal-state family

**Apps:** Settings, Clipboard, Clocks & Timers, Calculator, Base Converter, Unit
Converter, Colour Picker, Keyboard Tester.

**Depends on:** VX-03, VX-15, VX-16, per-user settings service.

**Deliver:** staged preferences, typed clipboard, alarms, history/precision,
screen-pick permission, physical/logical key inspection and schema migration.

**Proof:** failed persistence, reboot, locale/timezone, suspend timer, clipboard
owner death/privacy, capture denial, layout change and invalid numeric input.

### VX-24 — Creative/rendering family

**Apps:** Paint, Renderer, 3D, zlOS animation.

**Depends on:** VX-11, VX-12, VX-16, File/Media services.

**Deliver:** versioned project state, undo, tools/selection/layers as appropriate,
safe import/export, reusable scene/render interface and deterministic demo mode.

**Proof:** huge canvas/mesh, decoder failure, save crash, backend reset, undo
round-trip, software/GPU differential and project migration.

### VX-25 — Games conformance suite

**Apps:** all 24 named games.

**Depends on:** VX-18, input, audio and user-data services.

**Deliver:** deterministic seed/replay, pause/resume, save/high score, audio cues,
resource budgets, accessibility alternatives and one shared game-runtime seam.

**Proof:** rule goldens, launch every shipped game, replay hash, background
throttle, input loss, audio loss, crash/restore, quota, close teardown and the
repaired Maze route.

## Phase 6 — browser

### VX-26 — Browser IPC and process topology

**Depends on:** VX-07, VX-11, VX-16, VX-18.

**Deliver:** browser UI, navigation broker, network, site content, decoder and
origin-storage roles with generated bounded schemas.

**Invariants:** content has no ambient file/device/secret access; sender identity
and endpoint authority checked; quotas/backpressure/deadlines/cancel/peer death
defined; no pointer crosses IPC.

**Proof:** oversized/unknown messages, queue full, nth-handle transfer failure,
peer death at every request stage, forged origin/identity and leak census.

### VX-27 — Navigation and history transaction

**Depends on:** VX-26.

**Deliver:** one navigation ID and commit point across resolve/connect/TLS/
request/redirect/response/parse/subresources; back/forward/reload/stop/history.

**Invariants:** old page remains before commit; subresource state cannot replace
document status; redirect and history bounds; cancel is idempotent; stale
completion cannot commit.

**Proof:** cancel/race/redirect loops, Back during load, document then failing
stylesheet/image, wrong navigation ID, content crash and restored history.

### VX-28 — Browser network, trust, cache and storage

**Depends on:** VX-26, Network/Secrets/Clock services.

**Deliver:** origin-aware requests, DNS/TCP/TLS, trust UI data, cookies/cache/
storage quotas, private mode, clear/site-data and download quarantine metadata.

**Invariants:** padlock requires verified name/time/root/policy; origin partitioning;
private data is destroyed; cache never bypasses current auth/validation;
downloads do not execute on arrival.

**Proof:** certificate matrix, bad clock/entropy/root, cross-origin storage,
cache poisoning/revalidation, quota, private cleanup, corrupt partial download.

### VX-29 — Site content runtime

**Depends on:** VX-26, VX-27.

**Deliver:** preserve current HTML/CSS/layout/PNG/bounded JS, then add DOM events,
forms, dynamic reflow and bounded task/timer loop.

**Invariants:** input lengths and output quotas; parser overflow says so;
script step/time/memory limits; layout arithmetic checked; origin-bound APIs;
crash cannot affect browser UI or another site.

**Proof:** current corpus differential, parser/layout/JS fuzz, DOM/event mutation,
timer storm, narrow/huge viewports, site-process crash and cross-site denial.

### VX-30 — Decoder worker family

**Depends on:** VX-26.

**Deliver:** image/font/media/document worker protocol with length-first input,
format sniff/admission, decoded-size quota, cancellation and sealed output.

**Invariants:** decoder has no network/file handles; declared dimensions cannot
overflow; partial output never publishes as success; process death invalidates
output handles.

**Proof:** malformed/truncated/bomb/huge dimension corpus, cancellation, worker
crash, output mismatch and repeated decode leak check.

### VX-31 — Browser chrome and permission UX

**Depends on:** VX-07..10, VX-26..30.

**Deliver:** tabs, address/search, progress/stop, identity, permission indicator,
zoom/find, downloads, bookmarks/history, private mode, errors and crash reload.

**Invariants:** displayed URL/origin follows committed navigation; page cannot
draw/spoof browser chrome; permission ownership/revocation visible; keyboard and
screen-reader complete.

**Proof:** spoof pages, fullscreen/capture, long IDN/RTL URL, permission revoke,
download failure, tab/site crash, keyboard-only and 200% text.

### VX-32 — Web compatibility ladder

**Depends on:** VX-29.

**Deliver:** versioned supported-feature manifest and fixed local corpora for
forms/events, selectors/media, script modules, formats and accessibility.

**Invariants:** unsupported is explicit; no silent widening; external corpus
version/digest pinned; compatibility growth cannot weaken sandbox or quotas.

**Proof:** positive/negative/mutation cases per feature; browser exposes exact
engine profile in diagnostics.

### VX-33 — Browser end-to-end and hostile-page gate

**Depends on:** VX-26..32.

**Deliver:** repeatable host and QEMU origin servers, packet fault injection,
screenshot oracle, process/resource telemetry and crash recovery matrix.

**Proof:** real current Wikipedia-like fixed corpus, redirects, loss/reorder,
TLS failures, huge pages, malicious parsers/scripts, content/decoder/network
death and continued desktop/browser-chrome responsiveness.

## Phase 7 — new product application families

### VX-34 — Welcome, account and onboarding

**Depends on:** VX-13, VX-16, account/locale/network/accessibility services.

**Deliver:** resumable setup for account, locale, keyboard, accessibility,
network, privacy and recovery; safe skip and later re-entry.

**Proof:** offline setup, failed persistence, reboot every step, assistive-only
journey, existing account and migration.

### VX-35 — Device and permission manager

**Depends on:** VX-20, device/session authority.

**Deliver:** device/provider identity, health, lifecycle, driver/provenance,
permissions, recovery and removable media.

**Proof:** provider crash/rebind, device removal, stale generation, ordinary-user
MMIO/power/mount denial and recovery receipts.

### VX-36 — Software, update and rollback UI

**Depends on:** VX-17.

**Deliver:** signed catalogue, provenance, permission/version diff, progress,
cancel, update/rollback/uninstall and restart state.

**Proof:** signature/key revoke, dependency conflict, network/disk failure,
reboot mid-update, live app, rollback and exact package object census.

### VX-37 — Crash and recovery center

**Depends on:** VX-18, logger/crash service.

**Deliver:** source-attributed failures, redacted detail, restart/restore, safe
mode, quarantine, support bundle and state-recovery choice.

**Proof:** spoof/redaction, crash loop, incompatible snapshot, logger loss,
support-bundle permission and desktop continuity.

### VX-38 — Document and PDF family

**Depends on:** VX-21, VX-30.

**Deliver:** document reader/editor and isolated PDF/ebook reader with autosave,
recovery, search, annotations, print/export and accessibility.

**Proof:** hostile document/PDF, huge pages, missing fonts, disk full, decoder
crash, recovery, semantic reading order and print cancellation.

### VX-39 — Spreadsheet

**Depends on:** VX-21.

**Deliver:** cells/formulas/dependency graph, deterministic recalculation,
import/export, charts, undo/autosave/recovery and accessibility grid.

**Proof:** cycles, precision/errors, huge sparse sheet, formula bombs, hostile
import, crash during recalc/save and semantic cell navigation.

### VX-40 — Presentation

**Depends on:** VX-21, VX-24.

**Deliver:** slide editor/player, assets, templates, notes, speaker/display mode,
export and recovery.

**Proof:** external-display loss, hostile assets, missing font, full-screen
authority, autosave crash, reduced motion and keyboard-only delivery.

### VX-41 — Print, spool and scan

**Depends on:** VX-16, provider/service contracts.

**Deliver:** printer/scanner discovery, page setup/preview, owned spool jobs,
cancel/retry, progress, scan preview/crop/save and privacy indicators.

**Proof:** device removal, jam/error, wrong owner cancel, huge job, spool restart,
partial scan, capture denial and retained/removed data policy.

### VX-42 — Communications and personal information

**Depends on:** Browser/Network, Secrets, Notification, Locale services.

**Deliver:** account broker and Mail/Calendar/Contacts/Messaging clients with
offline queue, sync conflicts, attachments and notification privacy.

**Proof:** credential revoke, hostile message/attachment, clock/timezone,
duplicate/reordered sync, offline edit, conflict and account removal cleanup.

### VX-43 — Media, audio and creation

**Depends on:** AudioServer, VX-30, VX-24.

**Deliver:** library/player, recorder, mixer, synth/sequencer and richer image
creation with bounded streams, projects and permission state.

**Proof:** codec crash, xrun/device loss, sample-rate negotiation, saturation
goldens, microphone revoke, save/recovery and background behavior.

### VX-44 — Camera and capture

**Depends on:** VX-16, media/device services.

**Deliver:** camera preview/capture and screen/window/region recording portal,
visible ownership indicator, protected-surface policy and output workflow.

**Proof:** lock/background denial, indicator cannot be hidden, device removal,
quota/disk full, protected content, app death and revocation.

## Phase 8 — development, agents, rescue and demo

### VX-45 — Development workbench

**Depends on:** VX-21, VX-22, compiler/package/debug services.

**Deliver:** source editor, build graph, diagnostics, debugger, profiler, trace,
package and provenance views as ordinary apps.

**Proof:** hostile project, build cancel, compiler crash, debug authority, huge
trace, source recovery and reproducible artifact receipt.

### VX-46 — Agent and automation experience

**Depends on:** VX-16, VX-18, tool broker and capability system.

**Deliver:** visible plan, requested grants, child tasks, progress, receipts,
cancel/revoke, rollback and independent review; no ring-0 orchestrator.

**Invariants:** child authority derives from live parent handles at commit;
revoke serialized with spawn; termination makes every handle stale; model text
cannot bypass broker policy.

**Proof:** revoke-then-spawn, kill root orchestrator, stale manifest grant,
tool timeout/partial failure, consent expiry and no orphan privileged child.

### VX-47 — Rescue and public-demo experiences

**Depends on:** VX-13, VX-17, operations/service contracts.

**Deliver:** signed read-only rescue UI plus transactional leased QEMU/noVNC
demo with authenticated control and data planes, containment and cleanup.

**Proof:** wrong-disk confirmation, backup/restore, interrupted repair, lease
theft, enumerable socket, launch failure rollback, PID reuse, process death,
resource/egress limits and cross-session privacy.

## Phase 9 — system-wide proof and promotion

### VX-48 — Visual-regression oracle

**Depends on:** VX-02..14.

**Deliver:** component and whole-desktop regional corpus; color/palette/hue/
structure/text/layout metrics; tolerances calibrated against renderer variance.

**Proof:** planted one-pixel structure, wrong accent, wrong font size, missing
focus, clipped text and stale screenshot defects each fail the expected metric.

### VX-49 — Interaction replay and mutation gate

**Depends on:** VX-10, VX-13..25.

**Deliver:** deterministic input/event replay with screenshots, semantic trees,
focus owner, app state and side-effect assertions.

**Proof:** one mutation per required route/state/effect; no dense-ID inference;
lost/double/reordered input and process death.

### VX-50 — App workflow certification matrix

**Depends on:** VX-18..47.

**Deliver:** per app: install -> launch -> ready -> empty/open/create -> action/
edit -> save/export/share -> close -> reopen/restore plus denial/failure/recovery.

**Invariants:** UI result and underlying effect checked separately; every app
has keyboard/accessibility route; cleanup census is zero.

**Proof:** automated matrix plus bounded manual/physical journeys for visuals
and device interactions that cannot be host-proven.

### VX-51 — Performance and responsiveness gate

**Depends on:** VX-12, VX-18, VX-33.

**Deliver:** stage timing for input, IPC, app, scene, raster, composition,
present, browser parse/layout/paint; median/p95/p99/max and missed/lost counts.

**Proof:** supported resolution/scale/backend matrix, app/browser load, damaged-
area extremes, provider restart and effect degradation without input failure.

### VX-52 — QEMU product matrix

**Depends on:** VX-50, boot assertion matrix.

**Deliver:** exact artifact across BIOS/UEFI/media/resolution/input/network/
storage/audio profiles with launch/workflow/close and fault scenarios.

**Proof:** artifact digest and nonce, negative exits, panic absence, durable
side-effect checks and post-workload filesystem integrity.

### VX-53 — Physical promotion matrix

**Depends on:** VX-51, VX-52, driver/hardware contracts.

**Deliver:** named hardware profiles for display, keyboard, pointer, storage,
network, audio, power and multi-monitor; native artifact/effect receipts.

**Invariants:** host harness does not count as native boot; controller proof does
not promote every class; fallback remains available; DMA isolation state is
honest when no IOMMU exists.

**Proof:** cold/repeated boot, exact user journey, unplug/replug/reset/suspend,
provider recovery, durable log and physical effect evidence.

## Completion accounting

This backlog contains **54 contracts, VX-00 through VX-53**. Completion is
contract-by-contract and evidence-level-specific. A later contract may ship a
useful partial product without weakening an earlier security, accessibility,
or truth invariant.

The weakest external dependency is process-safe surfaces plus typed IPC: they determine
how quickly the current kernel-resident desktop and browser can migrate without
regressing responsiveness. Measurements may tune batching and cache sizes, but
must not erase the ownership boundaries.
