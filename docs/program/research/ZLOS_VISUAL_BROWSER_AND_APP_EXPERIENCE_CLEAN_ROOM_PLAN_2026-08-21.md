# zlOS visual, browser, and application experience: clean-room plan

Date: 2026-08-21

Status: destination architecture and delivery plan. This document specifies
behaviors and contracts derived from observed ideas; it does not import code
from any audited repository.

Inputs:

- [`ZLOS_CURRENT_VISUAL_WEB_AND_APP_EXPERIENCE_BASELINE_2026-08-21.md`](ZLOS_CURRENT_VISUAL_WEB_AND_APP_EXPERIENCE_BASELINE_2026-08-21.md)
- [`ALL_33_DRIVERS_AND_APPS_MATRIX_2026-08-21.md`](ALL_33_DRIVERS_AND_APPS_MATRIX_2026-08-21.md)
- the three visual/web/app deep dives produced by this research pass;
- [`DRIVER_AND_APP_CLEAN_ROOM_INTEGRATION_PLAN_2026-08-21.md`](DRIVER_AND_APP_CLEAN_ROOM_INTEGRATION_PLAN_2026-08-21.md)

## Outcome first

The destination is a recognizably zlOS desktop—not a clone—with:

- the current graphite/lime/grid identity, refined for legibility and scale;
- one semantic, accessible design system across shell, native apps, diagnostics,
  and browser chrome;
- a fast compositor and shell that survive app, decoder, provider, and browser-
  content failure;
- a browser whose useful bounded implementation grows behind site isolation,
  permission, storage, and network boundaries;
- every current application preserved as a real user workflow, not merely a
  catalogue tile;
- deliberate new productivity, communications, media, development, recovery,
  and accessibility families;
- measurable proof for appearance, input, workflow, service effects, recovery,
  and physical display behavior.

The project should not chase the largest external catalogue. It should extract
shared platform contracts so each additional app becomes cheaper, safer, and
more consistent.

## Product principles

1. **Identity before variety.** One visual language; themes change tokens, not
   component geometry or interaction meaning unpredictably.
2. **A screen is not a feature.** Every visible control maps to a typed action,
   progress state, result, failure, cancellation, and recovery path.
3. **Directness is the zlOS personality.** Show exact device, file, process,
   origin, and evidence state without turning every screen into a debug dump.
4. **Fast is part of the look.** A 200 ms animation that starts a frame late is
   wrong even if its final screenshot matches.
5. **Accessibility is structural.** Semantics, focus, scaling, contrast, motion,
   captions, and input alternatives are component contracts, not a late theme.
6. **Untrusted bytes never own the desktop.** Web, images, archives, fonts,
   media, documents, packages, and remote sessions are parsed outside the
   shell/compositor.
7. **Authority is visible.** Capture, microphone, camera, location, device,
   secret, file, and remote-control grants have user-facing ownership and
   revocation.
8. **Failure has a designed state.** Empty, loading, offline, degraded, denied,
   timed out, crashed, and recovered are first-class views.
9. **Evidence does not blur.** Source, build, screenshot, interactive QEMU,
   native physical behavior, and durable side effect remain distinct.

## End-state architecture

```text
zl design tokens + component/semantic schema + motion/type/icon assets
                              |
        +---------------------+----------------------+
        |                     |                      |
 Theme/Font/Icon       Accessibility/Locale    Input Method
 services              semantic tree           and shortcuts
        |                     |                      |
        +---------------------+----------------------+
                              |
                     Window/Surface protocol
                              |
                 Compositor + session shell
                    /         |          \
          app processes    portals      browser chrome
                |              |              |
     file/media/device   permission,      navigation broker
        services          chooser, etc.       |
                                         network/cache
                                              |
                                  isolated site/content processes
                                              |
                                    isolated decoder workers
```

Only the compositor owns scanout. Only the session/input policy owns focus and
global shortcuts. Apps own surfaces and content, not pixels on the display.
Service and process death invalidates generation-tagged handles and cannot leave
focus, capture, clipboard, drag, or modal ownership orphaned.

## 1. Design foundations

### 1.1 Semantic token schema

Retain the current source-of-truth approach, but version the schema and split
raw values from semantic use.

Required groups:

- color: canvas, sunken, content, raised, overlay, border, disabled, text
  levels, accent, link, selection, focus, success, warning, failure;
- typography: display, title, heading, body, label, caption, data, code, terminal;
- spacing: 1/2/3/4/6/8/12/16/24/32 design-space steps;
- radii: control, card, panel, window, circular;
- elevation: border, shadow, tint, blur-request/fallback behavior;
- motion: enter, exit, move, resize, press, attention, toast, background;
- density, scale, contrast, transparency, and reduced-motion profiles;
- z-order layers and interaction-state priorities.

Every token has an identifier, type, units, default, range, fallback, contrast
relationship, affected components, and evidence origin. Unknown mandatory token
schema fields are rejected. Theme installation validates the full set before
atomic publication.

### 1.2 Typography and language

One text stack must serve UI, browser, editor, terminal, documents, and
accessibility. Deliver:

- scalable glyph cache with strict memory budgets and eviction receipts;
- font fallback and coverage registry;
- shaping for combining marks, ligatures, bidirectional text and scripts;
- font metrics rather than bottom-aligned bitmap assumptions;
- user text scale independent from display density;
- monospace guarantees for terminal/editor/log/hex;
- locale-aware date, time, number and unit formatting;
- input-method composition and candidate surfaces;
- deterministic line-break, selection, caret and hit-test APIs.

The current three-atlas approximation remains as recovery fallback until the
new stack passes pixel and layout differential gates.

### 1.3 Icon and asset system

Use semantic icon names with vector/path or high-resolution raster sources,
compiled into bounded artifacts. Each icon declares optical sizes, stroke/fill
behavior, directionality, selected/disabled/attention variants, and fallback.
App bundles supply signed icons and screenshots through the package manifest;
shell never trusts raw decoder output.

Wallpaper and theme assets are content-addressed. Dynamic effects declare their
frame/memory budget and reduced-motion/static equivalent.

## 2. Component model

### 2.1 Component contract

Every component exposes:

```text
identity + semantic role + accessible name/description
value/range/selection + enabled/read-only/required/invalid
layout constraints + minimum touch target + overflow behavior
pointer/keyboard/touch/assistive actions
visual state + focus state + busy/progress/error state
localized content + test identifier
```

No user pointer, framebuffer address, or process-local object crosses the
protocol. Rendering uses immutable command/display lists or bounded scene
updates referencing owned resources.

### 2.2 Required primitive set

Text, icon, image, separator, button, icon button, split/menu button, checkbox,
radio, toggle, slider, stepper, text field, search field, password field,
multi-line editor, select/combo, segmented control, tabs, toolbar, status bar,
menu, popover, tooltip, toast, notification, modal, sheet, progress, meter,
badge, avatar, breadcrumb, tree, list, table, grid, card, chart, sidebar,
scroll view, split view, empty state, skeleton/loading state, and error panel.

Each primitive ships default, hover, focus-visible, active, selected, disabled,
busy, invalid, destructive, high-contrast, reduced-motion, and touch states.

### 2.3 Composite patterns

- file picker/save/open-with/share portals;
- command palette and global search;
- app launcher and catalogue;
- preferences page with staged apply/revert;
- data table with sort/filter/selection/virtualization;
- editor tabs, dirty-state and recovery;
- inspector/detail split view;
- permission prompt and persistent grant inspector;
- background-task/progress center;
- crash/restart/restore surface;
- onboarding/welcome and migration assistant;
- device arrival/removal and removable-media flow;
- authentication/elevation confirmation;
- download and package/update flow.

## 3. Accessibility contract

The accessibility service receives a bounded semantic tree, never scrapes
pixels. Nodes have stable generation-tagged IDs, parent/child order, role,
state, value, bounds, text ranges and actions. App updates are incremental and
quota-bound; malformed cycles or impossible bounds reject the update without
breaking the desktop.

Required user features:

- complete keyboard traversal and shortcut discovery;
- screen reader output plus speech/braille provider interfaces;
- magnifier, cursor scale, large text, contrast and color filters;
- reduced motion/transparency and animation pause;
- captions/transcripts and mono/balance audio controls;
- sticky/filter/slow keys and remapping;
- switch control and dwell alternatives;
- per-app accessibility permissions and privacy redaction.

Proof uses automated semantic snapshots, contrast checks, focus-order mutation,
keyboard-only journeys, 200% text, locale expansion, RTL, reduced motion, and
screen-reader action replay.

## 4. Shell and window experience

### 4.1 Session lifecycle

```text
Boot/Recovery -> Login -> SessionStarting -> Ready -> Locked
                                    |          |
                                    v          v
                               Degraded     Unlocking
                                    |
                         LoggingOut/ShuttingDown
```

Each transition has readiness dependencies, timeout, cancellation where safe,
reverse shutdown order, crash budget, and recovery UI. Lock revokes input and
surface visibility from the previous session; screenshot/capture and remote
leases cannot cross it.

### 4.2 Window and surface authority

A window/surface handle declares owner, grantees, dimensions, stride, format,
scale, color profile, memory quota, damage sequence, present mode and generation.
Rights separate map, draw, present, resize, embed, capture, share and destroy.
Only compositor holds scanout; only session policy assigns focus/input.

Admission checks integer overflow, total bytes, format, dimensions, quotas and
unknown required protocol bits. Resize is allocate/copy/commit or rollback.
Peer death releases surfaces, pointer capture, focus, drag, clipboard offers,
modal ownership and global-shortcut registrations.

### 4.3 Interaction set

Retain and polish current move, resize, maximize, minimize, snap, tabs,
Alt-Tab, Super snapping, workspaces, dock, Activities, catalogue, Run, toasts,
and system status. Add:

- overview with live but budgeted previews and accessible list equivalent;
- searchable app/window/file/action command palette;
- multi-monitor layouts, per-monitor scale, safe hotplug and workspace policy;
- drag/drop with typed offers, consent, cancellation and source death handling;
- notification center with source, urgency, privacy, actions and history;
- quick settings backed by real providers and explicit unavailable/degraded
  states;
- lock/login/power menus with complete authority checks;
- background jobs, downloads, recording/capture indicators and one-click
  revocation;
- crash surface that restores documents/session state without restart loops.

### 4.4 Performance budgets

Preserve the measured present and input telemetry. Define budgets separately
for input dispatch, scene update, raster, composition, present, app response and
animation. No average hides tail latency. At each supported resolution record
median, p95, p99, maximum, missed frames, dropped input, damage area, copied
bytes and backend identity.

Retained shell/client surfaces are the immediate performance step. GPU paths
must match software-rendered scene hashes before promotion. A decorative effect
that breaks the budget degrades predictably—reduce blur, animation or live
preview—not interaction correctness.

## 5. Application platform

### 5.1 Application manifest

Each bundle declares signed identity, publisher key and trust policy, version,
target architectures, zlOS ABI range, entry points, complete file hashes/sizes,
dependencies/conflicts, required and optional service capabilities, file/MIME
types, URL schemes, permissions with human descriptions, background roles,
resource budgets, localization, accessibility metadata, licenses, firmware/
asset provenance, migrations and rollback compatibility.

Install is stage -> authenticate -> validate every object -> resolve -> policy
admit -> atomically publish. Uninstall first unpublishes and revokes live
authority, stops processes, unregisters callbacks/services, then deletes
program data according to explicit user-data policy.

### 5.2 App lifecycle

```text
Verified -> Installed -> Launching -> Ready -> Background/Suspended
                              |          |             |
                              +--------> Stopping <----+
                                           |
                                      Exited/Crashed
                                           |
                                Restore/Restart/Quarantine
```

Readiness requires a surface and launch nonce from the correct process identity.
Close requests have a deadline; unsaved-state negotiation is bounded. Crash
loops trigger backoff/quarantine. Restore consumes versioned state and can be
declined if incompatible. Every exit proves process death and handle cleanup.

### 5.3 Portals and shared services

Applications do not receive ambient filesystem, device, clipboard, capture,
camera, microphone, location, secret, package, power or global-input access.
They request narrow handles through portals with user-visible ownership,
duration and revocation.

Core services: File/VFS, Settings, Secrets, Clipboard, Notification, Search,
Print, Media/Decoder, Audio, Camera/Capture, Network/Resolver, Package/Update,
Logger/Crash, Accessibility, Locale/Clock, Device/Power, and Agent Tool Broker.

## 6. Browser architecture and user experience

### 6.1 Process split

- **Browser UI:** tabs, address/search, history view, downloads, bookmarks,
  permissions, settings; no raw network or parser privilege.
- **Navigation broker:** request lifecycle, redirects, origin policy, response
  metadata, cancellation and download decisions.
- **Network service:** DNS, sockets, TLS, proxy, cache transport; secrets and
  trust policy stay outside content.
- **Site/content process:** HTML/CSS/layout/bounded JS/event loop for one site
  or origin group; disposable on compromise/crash.
- **Decoder workers:** image/font/media/document parsers with input length,
  output quotas and no file/network ambient rights.
- **Storage service:** origin-scoped cookies/cache/local data with quota,
  expiry, clearing and private-session behavior.

Every IPC envelope is typed/versioned and bounded by message count and bytes.
It carries authenticated peer identity, opaque correlation ID, deadline,
cancellation and explicit handle rights. Destination capacity is reserved
before committing a transfer.

### 6.2 Navigation state machine

```text
Idle -> Resolving -> Connecting -> Handshaking -> Requesting -> ResponseStarted
  ^          |            |              |              |
  |          +------------+--------------+--------------+
  |                         fail/cancel
  +---- Committed <- Parsing/LoadingSubresources <- ResponseAdmitted
             |
       Interactive/Complete
```

A navigation has one commit point. Before commit, the old page remains usable.
After commit, Back restores a history entry without confusing document and
subresource status. Redirect count, response bytes, decoded pixels, DOM nodes,
CSS rules, script steps, timers, storage and child processes are quota-bound.

### 6.3 Browser UI

Deliver tabs, new-tab/home, URL/search, back/forward/reload/stop, load progress,
site identity, permission indicator, zoom, find-in-page, downloads, bookmarks,
history, private windows, clear-data controls, crash/reload and responsive
errors. Security UI names the verified host, issuer/policy failure, mixed or
insecure state, and granted devices. A padlock is never shown from scheme text.

Downloads first land in quarantine through a File portal; filename, MIME,
declared length, received length, digest and provenance are visible. Opening is
a separate user action and permission decision.

### 6.4 Compatibility order

1. preserve current bounded pages, TLS, PNG, CSS layout, external styles and
   inline script proof after process separation;
2. DOM events, form controls/submission and dynamic media-query reflow;
3. origin/cookie/cache/storage and navigation correctness;
4. selector/pseudo-class completeness and text/layout fidelity;
5. external scripts/modules plus bounded task/microtask/timer loop;
6. additional image/font formats through decoder workers;
7. accessibility tree and browser-native find/selection/copy;
8. downloads, print/PDF and safe external-app handoff;
9. optional developer inspector for zlOS's supported subset.

Do not make full compatibility the gate for a useful secure browser. Each
subset is explicitly reported, tested against fixed local corpora, and mutation
checked.

## 7. App-family roadmap

### 7.1 Migrate current core apps first

1. About, System Monitor, System Info, Kernel Log, Services, Disk Usage,
   Network: become read-only clients of typed services and product provenance;
   privileged actions are separate capabilities.
2. Files, Text Editor, Image Viewer, Archive Manager, Hex Viewer, Regex Tester:
   exercise File portal, safe-save, bounded parsing, decoder workers, huge-input
   paging and recovery.
3. Terminal/Console/Run: exercise PTY, process trees, arguments/environment,
   exit, signals, launch authority and child cleanup. Define whether Console is
   a recovery role or retires into Terminal.
4. Settings, Clipboard, Clocks, Notes: exercise per-user state, schemas,
   persistence, notification and portal ownership.
5. Paint, Renderer, 3D, animation, Pointer, Font Atlas, Framebuffer: exercise
   graphics surfaces, input, capture permission and diagnostic receipts.
6. Menu and All Applications remain shell-owned registry views; utilities and
   all 24 games exercise deterministic app lifecycle, timers,
   input, storage, audio, quotas, pause/restore and crash isolation.
7. Browser: preserve current capability while moving into the process model.

### 7.2 Add platform-defining apps

- **Welcome/Onboarding:** account, locale, keyboard, accessibility, network,
  privacy and recovery key setup; resumable and safe to skip.
- **Device Manager:** device/provider identity, driver lifecycle, health,
  permissions and recovery; no ambient raw MMIO.
- **Software/Updates:** signed catalogue, permission diff, install/update/
  rollback/uninstall, provenance and restart requirements.
- **Crash/Recovery Center:** app/service failures, restart/restore, logs with
  redaction, safe mode and support bundle.
- **Accessibility Center:** live preview and per-user settings for every
  accessibility feature.
- **Notification/Background Tasks:** history, progress, privacy, cancellation
  and source controls.

### 7.3 Productivity and communications

- document reader/editor with versioned format, autosave, recovery and export;
- spreadsheet with bounded formulas, dependency cycles, deterministic recalc
  and hostile-document isolation;
- presentation editor/player with assets, speaker mode and safe full-screen;
- PDF/ebook viewer through isolated parser/renderer;
- mail, calendar, contacts and messaging behind account/secret/network services;
- print/scan applications over printer/scanner providers and a spool service,
  with job ownership, cancellation, page preview, privacy and device failure.

### 7.4 Media and creation

- media library/player with metadata and codec workers;
- audio recorder and mixer with visible capture ownership, bounded rings and
  recoverable projects;
- camera with preview/capture, explicit indicator and background denial;
- screen capture/recording portal with region/window consent and protected
  surface rules;
- richer Paint/vector/image tooling and a versioned project format;
- music/synth/sequencer using deterministic DSP goldens and AudioServer timing.

### 7.5 Development and automation

- source editor/IDE, compiler/build view, package manager, debugger, profiler,
  log/trace viewer and ABI/provenance inspector;
- agent/automation UI showing plan, requested tools, live attenuated grants,
  progress, receipts, child tasks, cancellation and rollback;
- agents remain ordinary processes; tool broker mediates each effect and a
  separate reviewer verifies high-risk receipts.

### 7.6 Recovery and public demo

The rescue UI is signed, read-only by default, names the target disk/operation,
requires confirmation for mutation, and produces backup/repair/restore receipts.

The public demo has authenticated control and VNC/WebSocket data planes, one
lease owner, transactional slot allocation and launch rollback, strict CPU/RAM/
PID/disk/network limits, process-identity/death proof before slot reuse, visible
timeout, and privacy cleanup.

## 8. Proof system

### Visual proof

- token-source chain and no stray literals;
- component matrix screenshots at every state, scale, contrast, locale and RTL;
- regional screenshot oracle for color/palette/hue/structure plus text/layout
  metrics;
- animation event/damage/frame sequence capture, not final frame only;
- software-versus-accelerated differential scenes;
- native physical display artifact/nonce/effect receipt separate from host
  harness and QEMU.

### Interaction proof

- keyboard-only and pointer/touch journeys;
- focus order, modal trap/escape, shortcut conflicts and input-owner death;
- resize/min/max/snap/workspace/multi-monitor/hotplug;
- drag/drop cancellation and source/destination death;
- app launch/readiness/close/restore with wrong PID, timeout and crash injection;
- mutation per registry field so a false-green catalogue is impossible.

### Workflow proof

For every app: empty -> create/open -> edit/action -> save/export/share -> close
-> reopen/restore, plus denied, offline, malformed input, disk full, dependency
loss, app crash and service restart. Validate side effects independently of UI
text. A “Saved” toast without durable bytes is failure.

### Browser proof

- parser/layout/JS/decoder fuzzing with sanitizers and deterministic seeds;
- local origin server corpus for redirects, cancellation, caching, cookies,
  mixed content, permissions, downloads and malformed responses;
- process kill at every navigation/subresource/storage stage;
- origin isolation and cross-origin denial;
- certificate/name/time/root/entropy failures;
- screenshot/layout oracle at fixed widths and text scales;
- memory/CPU/timer/process quotas and recovery after hostile pages.

### Accessibility proof

Semantic-tree validator, action replay, focus traversal, screen-reader journey,
200% text without clipped critical actions, contrast thresholds, high-contrast
snapshots, reduced-motion event traces, captions/transcripts and input remapping.

## 9. Dependency-ordered delivery

### Phase V0 — freeze truth

- exact current screenshot/source/interaction/app registry receipts;
- fix app ID/existence/route checker and mutation-prove it;
- classify current screenshots as current versus historical;
- version visual evidence vocabulary.

Exit: every named app and shell surface has one honest source/build/image/route/
workflow status, with no aggregate green hiding a failed field.

### Phase V1 — design and semantic core

- versioned token schema, scalable text, icons, component state matrix;
- semantic tree and focus/input contracts;
- locale, scale, contrast and reduced-motion profiles.

Exit: component gallery passes all state/scale/a11y oracles and can be consumed
by a process without direct framebuffer access.

### Phase V2 — surface, window, and session boundary

- process-safe surfaces, compositor ownership, shell/session lifecycle;
- typed event routing, workspaces, multi-monitor, notifications and portals;
- peer-death cleanup and performance budgets.

Exit: hostile demo app cannot draw outside its surface, steal input/scanout, or
leave authority after death; desktop remains responsive.

### Phase V3 — migrate diagnostic/core apps

- service-backed system apps, Files/Editor, Terminal, settings/state apps;
- safe save, document recovery, PTY/process and telemetry contracts.

Exit: each migrated app survives provider/service restart and proves real side
effects independently of pixels.

### Phase V4 — browser isolation

- browser UI/navigation/network/content/decoder/storage split;
- preserve current HTML/CSS/PNG/TLS/JS result;
- permission, identity, crash and quota UI.

Exit: hostile page or decoder crash cannot damage browser chrome or desktop;
current fixed corpus renders within declared differential and timing limits.

### Phase V5 — current catalogue completion

- migrate all utilities, diagnostics, graphics apps and 24 games;
- deterministic state, save/restore, audio and accessibility;
- app lifecycle/package manifests.

Exit: all current named apps launch from shipped image, reach ready nonce, pass
one full workflow, close cleanly, and survive injected crash.

### Phase V6 — product platform apps

- onboarding, device, package/update, crash/recovery, accessibility and tasks;
- productivity, print/scan, media/capture and communications foundations.

Exit: one complete end-to-end journey per family, including denial, failure,
restart and durable recovery.

### Phase V7 — developer, agent, rescue, and public-demo experiences

- self-hosted development views; capability-native agent UI/tool broker;
- rescue and remotely leased demo.

Exit: all external effects are attributed, consented, bounded, revocable and
independently evidenced; public sessions cannot observe or control one another.

### Phase V8 — hardware and polish promotion

- native physical display, input, audio, power and multi-monitor profiles;
- GPU composition only after software differential; sustained accessibility and
  latency gates.

Exit: the exact shipped artifact passes QEMU and named physical profiles with
separate receipts; no host harness is mislabeled as native boot.

## 10. What not to copy

- raw source or assets whose license/provenance does not permit it;
- catalogue breadth supplied only by external ports or fetched desktops;
- screenshots presented as reachability or service proof;
- direct framebuffer/input access, ambient global IDs, kernel-resident apps or
  parsers as the destination architecture;
- unauthenticated compositor, VNC, clipboard, shared-memory or management paths;
- “browser” claims based on a static page, and “modern web” claims based on a
  few CSS properties;
- accessibility reduced to color themes or magnification;
- install/update success before atomic publication and rollback proof;
- build-only tests or wrappers that mask failure;
- raw app count as a product goal.

## 11. Definition of done

The visual/browser/app program is complete when:

- zlOS retains its visual identity and all current user jobs while no ordinary
  app/browser parser owns kernel memory, scanout or ambient input;
- every component is semantic, keyboard-operable, scalable, localizable and
  tested in every state;
- every current named app is uniquely registered, shipped, launchable, ready,
  workflow-proven and cleanly tearable down;
- browser content, decoders, network secrets and origin storage are isolated and
  recoverable, while the current bounded web corpus still works;
- the new product families have complete user journeys and shared service
  contracts rather than bespoke privilege;
- visual, interaction, effect, recovery, QEMU and physical evidence are stored
  separately and cannot promote one another implicitly;
- failures explain what happened, preserve user data, revoke authority and
  leave the desktop usable.

## Weakest link and revisit point

This is a full destination plan, not an implementation estimate. It should be
revisited after V2 produces measured process-safe surface/event overhead and
after V4 runs the current browser corpus across real process boundaries. Those
measurements decide command-buffer granularity, site-process grouping and cache
sizes; they do not change the security or user-experience boundary.
