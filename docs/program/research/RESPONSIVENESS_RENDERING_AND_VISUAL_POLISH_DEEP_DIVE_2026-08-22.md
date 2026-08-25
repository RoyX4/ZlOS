# Responsiveness, rendering, and visual-polish deep dive

Date: 2026-08-22

Scope: a fifth-pass, clean-room study of the 33 pinned starred repositories and the current zlOS desktop. This report answers two narrower questions than the earlier visual audit: **why does a system feel responsive instead of laggy, and what implementation machinery makes it look coherent rather than merely decorated?**

This is a source-and-evidence synthesis. A screenshot proves appearance at one instant, not input latency, frame pacing, reachability, or backend truth. A source path proves a mechanism exists, not that it meets a deadline. Measurements are only promoted when the workload, clock, artifact, sample count, and evidence lane are known.

## Short answer

The strongest systems do seven things together:

1. They retain expensive work instead of rebuilding it every frame.
2. They track damage and visibility so unchanged or hidden content is not painted.
3. They separate input, app work, composition, waiting, and present, then measure each phase.
4. They make animation time-based and discard stale visual states instead of replaying a backlog.
5. They keep blocking I/O, decoding, network work, and slow clients away from the compositor thread.
6. They use one semantic visual vocabulary for type, spacing, color, shape, states, and motion.
7. They never let a painted control, fake status value, or historical screenshot substitute for a working path.

“Looks good” and “does not lag” are the same product problem at the frame boundary. Blur, shadows, rich wallpaper, typography, and animation are good only when their cost is bounded, cached, invalidated correctly, and preserved under degraded hardware or memory pressure.

## Evidence vocabulary

| Label | Meaning |
|---|---|
| `[S]` | mechanism exists in the pinned source |
| `[R]` | mechanism is connected to the active build, image, init, or user route stated nearby |
| `[T]` | a deterministic test or benchmark targets it |
| `[QEMU]` | observed in a guest/emulator lane |
| `[H-host]` | host harness exercised physical hardware, without proving native-boot reachability |
| `[H-boot]` | observed from the native booted system on physical hardware |
| `[A]` | screenshot, video, font, icon, wallpaper, or other visual asset only |
| `[C]` | documentation or project claim only |
| `[F]` | concrete contradiction, false green, dead route, or simulated result |
| `[P]` | proposed destination behavior, not current proof |

## What “not laggy” actually means

Responsiveness is not an average FPS number. A system can average 60 FPS and still feel bad if it occasionally blocks for 120 ms, applies an old pointer event, animates at different speeds under load, or freezes while reading storage.

The product contract needs separate measures:

| Measure | User-visible failure | Required observation |
|---|---|---|
| input-to-present latency | pointer or key feels detached | timestamp input admission and the first present containing its result |
| frame time | motion hitches | measure every painted frame, not only averages |
| late-frame count | intermittent jank disappears in mean | count every frame beyond the active refresh deadline |
| worst gap | “smooth” run pauses once | record maximum inter-present gap and percentile distribution |
| queue age | system responds to old intent | carry sequence and timestamp through routing and rendering |
| damage efficiency | tiny change repaints the world | pixels visited, bytes copied, regions, windows, and causes |
| phase ownership | optimization guesses at the wrong layer | input, app tick, layout/paint, composition, wait, present |
| recovery latency | one app freezes desktop | deadline, cancellation, peer-death cleanup, fallback presentation |
| first useful frame | launch appears dead | placeholder/skeleton/cheap coherent frame before refinement |
| interaction stability | pointer/animation changes with load | velocity/time-normalized input and time-based animation |

The standing 60 Hz target is not merely “average below 16.67 ms.” Every painted frame and the peak must remain below 16.67 ms for the measured interaction, with zero hidden late frames. Higher-refresh displays require a negotiated deadline rather than a hard-coded 60 Hz assumption.

## End-to-end fast path

```text
device timestamp
  -> bounded input queue
  -> compositor-owned hit test / focus / grab
  -> authenticated app event
  -> app updates logical state
  -> app commits newest surface state + damage
  -> compositor coalesces damage and computes visibility
  -> retained wallpaper/chrome/client layers are reused
  -> only intersecting work is painted
  -> one atomic scene commit
  -> deadline-aware wait
  -> one present
  -> receipt ties frame to input sequence, work, damage, and artifact
```

No unbounded file read, DNS request, image decode, font discovery, process launch, log formatting, or client acknowledgment belongs on that path.

## Current zlOS: measured findings, not adjectives

The current zlOS evidence is stronger than the shelf on several narrow physical performance questions.

### Scanout mapping was the first dominant bottleneck

The original physical 2560×1440 framebuffer path was effectively uncacheable and measured about 111.59 MB/s. Remapping the framebuffer write-combining after IDT setup and before application-processor startup changed the measured slope to about 7,089 MB/s, 63.5 times the old value **[H-boot]**. Seven full-screen presents then had a 2.070 ms median; cursor-only input-to-present had a 7.827 ms median. The retained physical session recorded 6,503 valid phase samples with zero recorder drops.

That result changes the optimization order. Present is no longer the principal late-frame owner. On large 3.2–3.5 Mpixel damage, restoring the cached desktop takes about 9–13 ms and final present about 3 ms, while current chrome and client redraw can be far larger. The next work is retained client and shell surfaces, not speculative input rewrites or a GPU claim.

### The desktop already measures the correct phases

Current frame instrumentation separates input, app tick, compositor work, vblank wait, and present. It correlates input sequence/time, damaged pixels, presented bytes, framebuffer mapping, rectangle count, and late-frame records. This prevents “the cursor feels slow” from being answered by optimizing whichever loop is easiest to see.

### Cached static art is load-bearing

The detailed wallpaper has multiple gradients, radial glows, conic sweeps, a vignette, and dithering. Rebuilding it inside each damage rectangle is unaffordable. One 900×700 radial glow measured about 12.2 ms. The full-resolution dithered RGB565 wallpaper is therefore rendered once and reused; its cached paint is roughly 1.5 cycles per pixel. At 3840×2160 the two-byte cache is about 15.8 MiB, so cache admission and fallback are explicit rather than accidental.

A menu-sized blur measured about 7.5 ms cold and about 0.19 ms cached. This makes a stationary blurred menu reasonable and a dragged live blur unreasonable on a 16.67 ms budget. zlOS deliberately caches stationary effects and refuses or degrades when the effect arena cannot safely hold them.

### Clipping pixels is not enough; work must be clipped

The terminal once laid out and blitted glyphs for rows outside the active damage band, only for the framebuffer scissor to discard their pixels. Skipping those rows before layout/blit removed real work. Likewise, `fb_shadow` clipped to the screen but not to the active scissor, so every window visit walked the whole shadow band. Folding the scissor into loop bounds reduced one 600×460 shadow from 4.34 ms to 0.61 ms and improved a full desktop redraw 4.1×.

The attempted lookup-table optimization made the shadow 25% slower. This is an important rule: cache locality, extra loads, branch behavior, and setup cost can defeat an intuitively “faster” algorithm. Keep the measurement, not the cleverness.

### Damage ownership must be split

Current `wm_damage_win()` means screen pixels changed. A retained architecture needs at least two independent concepts:

- client content invalidation: the app must redraw its logical surface;
- screen damage: the compositor must recompose an exposed or moved region.

Moving, uncovering, focusing, or changing z-order should not force an unchanged app to re-run layout and draw. Resizing may use a scaled retained surface during motion, then rebuild once settled. Retained-surface memory must have a budget, deterministic eviction, and a correct immediate-mode fallback.

### Input needs temporal semantics

The pointer save-under is intentionally a small exact bounding box; the cost is negligible compared with whole-frame work. The dangerous issue is not its size but identity and time. Focus/grab ownership must survive across frames, and pointer acceleration based only on “delta per rendered frame” changes behavior when FPS changes. Acceleration must be based on event time or measured velocity, with raw input still available for games, accessibility tools, and diagnostics.

## The reusable performance machinery

### 1. Retained resources and retained scene state

Retain what is expensive and stable:

- wallpaper and decorative background fields;
- glyph atlases, shaping results, line breaking, and text runs;
- decoded and scaled images, icons, thumbnails, and color transforms;
- shadows, blur kernels/results, masks, rounded-corner coverage, and gradients;
- app content surfaces and separately retained shell/chrome layers;
- GPU textures/buffers/pipelines when a GPU backend exists;
- layout measurements and virtualized list cells.

Every cache needs a key, byte budget, owner, generation, invalidation reason, pressure behavior, and observable hit/miss/eviction counters. A cache without invalidation is a rendering bug; a cache without a limit is a reliability bug.

### 2. Damage, occlusion, and visibility

Damage should be bounded and area-aware. Merge overlapping or nearby rectangles when traversal overhead exceeds saved pixels; retain disjoint rectangles when a bounding box would explode work. Track why damage occurred: app content, move exposure, focus/chrome, pointer, animation, wallpaper, resize, or full fallback.

Before painting, intersect damage with:

- screen bounds;
- each layer/window bounds;
- clip and rounded masks;
- visible region after opaque occlusion;
- scroll viewport;
- actual widget/text/image subregion.

A fixed maximum region count needs a counted full-damage fallback. Silent region loss produces stale pixels; an unbounded list allows a malicious client to consume the compositor.

### 3. One commit and one present

Apps should submit immutable or generation-tagged state. The compositor should assemble one coherent scene snapshot and present once. It must not expose half-updated chrome and content, reuse a buffer before release, or block the entire desktop on one client.

Double buffering avoids tearing when producer and consumer overlap. Triple buffering can prevent producer stalls but increases queueing latency unless stale buffers are dropped. The destination contract is therefore “newest valid committed state,” not “render every queued frame.”

### 4. Deadline-aware pacing

Use the display’s negotiated refresh interval and a monotonic clock. Schedule backward from the present deadline using recent phase costs. If late, skip to the current time-derived animation state and record the miss. Never slow the animation clock or replay every intermediate frame.

Idle desktops should sleep until input, client damage, animation deadline, device change, or service event. A fixed 16 ms polling loop wastes CPU and battery; a fixed 10 ms loop is not equivalent to a 60 Hz deadline.

### 5. Slow work outside the compositor

The following need asynchronous workers or restricted services:

- wallpaper/image/font decode and resize;
- browser HTML/CSS/JS, network, TLS, media, and image decode;
- thumbnail generation and directory enumeration;
- application launch and package verification;
- filesystem flush, archive extraction, and removable-media discovery;
- logs, telemetry export, screenshot encoding, and remote-session transport.

The compositor may show a placeholder, progress state, previous stable result, or explicit failure. It must not synchronously wait for the result.

### 6. Bounded client protocols

Shared surfaces and IPC are performance tools only if ownership is safe. Descriptors must validate width, height, stride, format, total bytes, offsets, damage, generation, and access rights with checked arithmetic. Buffers need configure/ack, commit/release, peer-death cleanup, and per-client quotas. Global integer IDs, sender-unchecked refresh, and arbitrary shared framebuffer mappings are rejected.

### 7. Virtualization and incremental computation

Large lists, terminal scrollback, logs, file directories, browser documents, text files, and tables should create/shape/paint only the visible range plus a small look-ahead. Search/indexing can run incrementally with cancellation. Layout caches must invalidate only affected ancestors or ranges rather than rebuilding an entire tree.

### 8. Graceful degradation

Visual quality should adapt in a declared order under memory pressure, software rendering, low refresh, thermal limits, remote transport, or reduced-motion policy:

1. keep content, input, contrast, and focus feedback;
2. reduce animation frequency or duration;
3. use cached/static shadows and blur;
4. reduce shadow samples, backdrop resolution, and decorative particles;
5. replace transparency with opaque semantic surfaces;
6. fall back from GPU to software without changing app semantics.

Never remove error, focus, selection, progress, or accessibility states as a “performance optimization.”

## Why the best-looking systems look coherent

### Semantic tokens, not copied hex values

A coherent product has one versioned authority for semantic color roles, typography roles, spacing, radii, borders, elevation, motion, density, and focus treatment. Components consume `surface.panel`, `text.muted`, `border.focus`, or `space.3`; they do not independently choose near-matching colors.

This is why SerenityOS, LemonOS, duckOS, Chitti, and current zlOS are more reusable than systems where every application embeds a palette. Chitti extends theming beyond color into font, density, panes, wallpaper, transparency, cursor, syntax, and status placement. The gap is a previewable, reversible Settings contract rather than another preset.

### Typography does most of the visual work

Good typography requires role, size, weight, line height, baseline, shaping, fallback, hinting, and contrast consistency. One chrome font and one terminal font can create identity, as RustOS does with Rajdhani and Iosevka, but text still needs international fallback and semantic scaling. Pixel fonts can be charming at one scale and fail badly when enlarged.

Rendering quality needs glyph cache/atlas management, gamma-aware blending, correct clipping, subpixel policy where applicable, stable baseline alignment, and layout that survives longer translations and larger text.

### Shape and depth need a small grammar

Rounded corners, borders, inner highlights, shadows, translucency, and glass work when they express hierarchy. RustOS’s Aurora chrome is coherent because geometry, accents, radius, title height, padding, controls, and shadow steps repeat intentionally. Effects become visual noise when each app invents its own depth or when focus is distinguishable only by a subtle shadow.

Every depth level needs an opaque fallback and contrast proof. Scanout remains compositor-exclusive; translucent app surfaces do not gain access to what is behind other windows.

### States are part of appearance

Every interactive component needs default, hover, pressed, selected, focused, disabled, loading, error, empty, success, dragging, drop-target, and unavailable states where applicable. Dead buttons and simulated status values make even attractive UI feel cheap because users stop trusting feedback.

The concrete counterexamples matter: Cyjon’s menu routes to removed Debug and Task Manager programs; vib-OS paints a browser, Wi-Fi, ping, and command results without their backend; mission-control’s CSS/JS class mismatch can leave a connected indicator visually muted. These are visual-system failures because state and truth diverge.

### Motion must explain causality

Motion should communicate where an object came from, what changed, and whether work continues. Use short time-based transitions, reversible progress, and reduced-motion alternatives. Avoid animation that blocks input, moves targets unexpectedly, queues stale frames, or hides latency with an endless spinner.

### Layout rhythm beats decorative complexity

Consistent spacing, alignment, minimum sizes, hit targets, content density, and responsive reflow produce more perceived quality than another glow. Hard-coded fixed geometry fails at different display sizes, localization lengths, keyboard-only use, large text, and touch/coarse pointers.

## Strongest observed patterns by repository

| Repository | Responsiveness/polish mechanism worth studying | Exact evidence ceiling or rejection |
|---|---|---|
| banan-os | damage-driven WindowServer near 60 Hz; alpha, rounded corners; separated window/audio/clipboard services | redraw and theme semantics remain uneven; native suite is smaller than its ports |
| Cyjon/Fern-Night | changed-zone compositor, process-owned SHM buffers, compact proportional taskbar, per-pixel alpha | dead menu targets, no resize, weak protocol ownership; old screenshot unavailable |
| Brook | kernel WM damage, per-window input rings, virtual surfaces, ARGB; double-buffered Wayland video | kernel-heavy policy and duplicated visual palettes; freestanding fixtures are not runtime tests |
| AthenX | basic framebuffer/login/desktop presentation | accept-all credential validation and app/syscall stubs make visible trust false |
| LemonOS | occlusion/clipping/damage, async wallpaper, SHM double buffers, semantic theme roles, shared LibGUI | credential/process-authority defects; concrete minimize-hover geometry bug |
| Astral | broad build recipes and ordering metadata | no local polished desktop; ordering is not a reliable lifecycle DAG |
| MollenOS | protocol/service separation and broad modules | GUI is external; compiled xHCI/E1000/services can be unreachable; no local visual proof |
| snarkOS | bounded terminal/log cache and explicit operator-role/lifecycle UX | headless TUI, not a desktop rendering donor; zero-rate division defect |
| SerenityOS | centralized WindowServer, semantic themes/metrics, GML layouts, broad shared toolkit, browser process split | strongest full product reference; multiarch target tests are x86-64-only, other lanes build/boot |
| Hyper | deterministic boot-selection/configuration and assertion matrix | no interactive boot-menu polish in pinned source |
| TacOS | tiny recognizable full desktop and keyboard-driven pointer | unconditional small full redraw; no mature retained/damage architecture |
| Banana OS | broad modular packages/apps and classic desktop breadth | active build i386; user-selectable ring-0 ELF loading breaks app isolation |
| mission-control | responsive portal, explicit start/capacity/expiry/remote-input states | incomplete launch rollback/retry/data-plane authorization and a11y |
| duckOS | Pond service, dirty rectangles, SHM double buffers, alpha hit-test, semantic themes | weak fallback/authority; SHM ownership bypass; unmeasured shadow cost |
| Chitti | coherent editorial tokens; configurable theme/density/panes; bounded surfaces; staged browser progress | browser/UI in kernel, tiny surface DSL, incomplete package/kill/revocation semantics |
| MineBios | bounded game viewport/loop and raw-boot determinism | intentionally tiny/no desktop; scope makes responsiveness easier |
| unix-history-repo | historical provenance and evolution | dataset, not an interactive system |
| osdev-projects | compact learning UIs/drivers | educational fragments do not form one product/compositor contract |
| hhuOS | broad graphical/utility ideas and hardware stack | unsafe user MMIO/raw-pointer paths defeat isolation; evidence varies by component |
| RustOS | retained GPU scene, dirty coalescing, one present, async refined background, explicit Aurora geometry | best local small-desktop mechanism; thin consumer app set and prior test gates are candidly incomplete |
| Fudge | declarative UI tree over actor mailboxes, incremental tree changes, partial redraw | hard-coded visual system, stringly events, little automated UI proof |
| privilegeOS | upstream Linux supplies mature responsiveness machinery indirectly | driver/product breadth is external Linux configuration, not repo-implemented UI |
| MaslOS-2 | partial window refresh, separate app processes, reusable file-picker workflow | hard-coded styling and mixed-era assets; unsafe loader/IPC paths |
| freestanding-c-hdrs | exact generated toolchain provenance | no interactive/visual product |
| NyauxKC | framebuffer/boot substrate | boot wallpaper is Limine-only; no local polished desktop |
| Sapphire GPU | required/optional capability descriptors and ready/valid command thinking | incomplete hardware blocks/top-level; no desktop performance result |
| Skift | immutable/reactive UI model, 16 ms tick, merged damage, CPU double buffer, rich historical design exploration | current widgets/apps/browser fetched and absent; local adapter cannot prove exact visuals/a11y |
| NexiOS | bounded resource/scheduling questions and broad shell registration | diagnostic presentation; authority and revoke defects prevent unqualified adoption |
| vib-OS | cohesive shell hierarchy, dock, wallpaper preview, media controls, loading states | browser/network/status partly painted or simulated; monolithic title-dispatch and ring-0 apps |
| asm.fm | deterministic offline DSP and golden audio artifacts | not a realtime UI/audio-latency donor |
| mYOS | small direct framebuffer/shell workflows | disk waits can hang; no mature compositor/performance instrumentation |
| Zinnia | narrow DRM/evdev enablement for external userland | desktop screenshot belongs to external bootstrap; local console is not the shown desktop |
| protOS | userspace WM, partial-surface refresh, event topics, PTY-first terminal split | unsafe shared framebuffer/IPC/UAF paths; early XOR/no-resize UI |

## Browser responsiveness is a separate system

A browser feels fast when navigation, network, parsing, style, layout, paint, scripting, image/media decode, storage, and UI remain independently cancellable and observable.

The clean path is:

```text
Browser UI
  -> navigation broker and history
  -> restricted request/TLS/DNS service
  -> per-site content process
       -> streaming HTML parse
       -> incremental style/layout
       -> retained display list/layers
       -> worker/script budgets
  -> disposable image/media/font decoders
  -> compositor-owned shared surface
```

Required performance behaviors:

- show navigation state immediately and retain the previous safe page until commit;
- stream bounded bodies and parse progressively;
- cancel network, parser, layout, script, decode, and paint work on replacement navigation;
- isolate long scripts with time/instruction budgets and visible “page unresponsive” recovery;
- virtualize large documents and invalidate only affected style/layout subtrees;
- decode images off the UI/compositor thread, with dimension/output budgets;
- prioritize visible resources and defer below-fold media;
- implement back/forward from committed history, never from half-loaded state;
- use bounded per-origin caches, storage, workers, sockets, and surface memory;
- preserve accessibility tree and focus across incremental updates;
- emit navigation timing, long-task, layout, paint, dropped-frame, and crash receipts.

SerenityOS is the strongest full browser/process reference. Chitti supplies a useful staged, inspectable pipeline and agent-readable link/text tools, but its engine is kernel-resident and several web APIs are no-op or partial. Current zlOS has a bounded native browser path worth preserving, but the next architecture is the split above, not adding more parsing to the kernel. vib-OS’s painted Browser and Fudge’s raw HTTP client are explicit non-examples.

## Performance budgets for the destination

These are engineering budgets to prove, not claims about current completion.

| Interaction | Host/QEMU development gate | Native target gate |
|---|---|---|
| pointer/key to first affected present | full timestamp chain and no event reordering | distribution by device/display; no unexplained gaps |
| ordinary drag/resize | every painted frame within negotiated refresh interval | peak and percentile under physical refresh deadline |
| idle desktop | no polling repaint and zero unexplained damage | event-driven wake, power/CPU evidence |
| menu/launcher open | immediate state change; cached heavy effects | no cold effect computation on critical path |
| window expose/move | unchanged client does zero app redraw | retained-surface hit and correct occlusion receipt |
| scrolling text/list/files | visible-range work only | stable frame time across dataset growth |
| app launch | shell remains interactive; staged readiness | first-useful-frame, ready, and failure timing |
| browser navigation | cancel/restart all stages; content crash contained | interactive UI remains responsive during slow/hostile page |
| image/media open | decoder isolated and bounded | progressive/placeholder state; failure does not freeze session |
| memory pressure | deterministic cache eviction | no stale pixels, UAF, compositor death, or semantic loss |
| GPU hang/provider loss | bounded timeout and reset | software/GOP recovery remains usable |
| remote demo | local shell and transport have separate clocks | reconnect/expiry/end remain authoritative and bounded |

Numbers must be calibrated per hardware and refresh mode. The initial 60 Hz visual deadline remains 16.67 ms, but the real acceptance artifact records resolution, refresh, backend, CPU/GPU, mapping, workload, sample count, percentile, peak, late count, and exact image digest.

## Failure shapes to turn into regression tests

- one tiny pointer damage causes all windows and wallpaper to redraw;
- an obscured app performs layout/paint while another window moves above it;
- eight disjoint damage rectangles redraw the full dock eight times;
- a scissor rejects pixels but the text/layout/image loops still do full work;
- a cold blur, shadow, wallpaper glow, thumbnail, or font load enters the drag path;
- input acceleration changes when the renderer drops frames;
- queued animation frames replay after a stall instead of jumping to current time;
- triple buffering increases input latency by presenting stale complete frames;
- a client holds a buffer forever and blocks global presentation;
- width × height × stride overflow admits an undersized shared surface;
- cache eviction leaves a dangling layer or stale generation;
- memory pressure removes focus/error/a11y state instead of decoration;
- a browser decoder/script/network timeout freezes shell input;
- a pretty control reports success before backend commit;
- a launch icon exists while its executable/package route is absent;
- screenshot assets are accepted as performance or current-runtime proof;
- FPS average passes while a single visible 100 ms hitch is hidden;
- QEMU host contention is reported as target performance without qualification;
- hardware controller proof is copied to every class driver without a class-specific receipt.

## Clean-room zlOS architecture decision

The visual destination should keep the current lime/graphite identity and measured software fallback while changing the ownership model underneath:

1. Display Provider exclusively owns modeset, scanout, fences, and recovery.
2. Input Service timestamps and normalizes devices but preserves raw streams.
3. Session/Compositor owns focus, grabs, windows, surfaces, z-order, damage, occlusion, frame scheduling, and final present.
4. Client AppKit owns semantic widgets, local state, layout, retained drawing data, and accessibility nodes.
5. Font/Image/Media services own hostile parsing and reusable decoded assets.
6. Theme/Settings services own versioned semantic tokens, scale, locale, reduced motion, and rollback.
7. Apps own documents and intent, never scanout or global input.
8. Browser content, network, and decoders are separate restricted processes.
9. Telemetry attributes every frame and interaction without exposing private content.
10. GPU acceleration is an optional provider behind the same scene/surface contract; software remains the oracle and recovery path.

The first implementation target remains retained client and shell surfaces because current physical evidence says redraw dominates after the write-combining fix. The first visual target is not a redesign. It is preserving the existing identity while making its expensive pieces retained, semantic, scalable, accessible, and independently provable.

## Proof suite

Completion requires all of the following, with planted defects demonstrating that each gate can fail:

- deterministic renderer oracle for blend, clipping, transforms, text, damage, occlusion, rounded geometry, shadows, and cache generations;
- scene property tests for arbitrary move/resize/stack/damage sequences;
- protocol tests for malformed surfaces, stale handles, peer death, withheld releases, and quota exhaustion;
- frame attribution benchmark using shipping compositor/toolkit/render code;
- input-sequence-to-present correlation under background I/O, app stalls, browser load, and memory pressure;
- screenshot goldens at multiple scales, locales, themes, contrast modes, and reduced motion;
- keyboard-only and assistive-provider walkthroughs of every primitive and shell workflow;
- QEMU lanes for software, virtio-gpu where supported, resolutions, refresh policies, and forced provider loss;
- native receipts for the exact Intel/GOP path, input class, scanout, repeated mode change, suspend/resume or reset where supported;
- false-UI mutation tests: remove a route, lie about state, fail a backend commit, omit an icon, break a focus state, and require nonzero failure;
- browser hostile corpus, process-crash containment, navigation cancellation, storage quotas, and long-task recovery;
- independent review that names the worst frame, weakest evidence lane, and every skipped configuration.

## Bottom line

The shelf does not reveal a secret graphics trick. It reveals a discipline: retain stable work, bound every queue and parser, paint only visible damage, keep the compositor free of blocking work, present one coherent newest state, measure the worst frame, and make the visual language semantic enough that every app inherits it.

RustOS contributes the clearest small retained-scene/perceived-startup pattern; SerenityOS the broadest integrated desktop and browser architecture; LemonOS, duckOS, banan-os, Brook, Skift, Fudge, MaslOS, and protOS each contribute useful compositor/protocol seams; Chitti contributes a distinctive configurable product grammar; current zlOS contributes the strongest local measured physical framebuffer evidence. The counterexamples are just as important: a beautiful mock, dead launcher, simulated network status, fetched UI, or external screenshot is not a responsive product.

## Source trail

Primary local evidence and synthesis inputs:

- [`MATURE_OS_VISUAL_WEB_AND_APP_EXPERIENCE_DEEP_DIVE_2026-08-21.md`](MATURE_OS_VISUAL_WEB_AND_APP_EXPERIENCE_DEEP_DIVE_2026-08-21.md)
- [`ARCHITECTURE_OS_VISUAL_WEB_AND_APP_EXPERIENCE_DEEP_DIVE_2026-08-21.md`](ARCHITECTURE_OS_VISUAL_WEB_AND_APP_EXPERIENCE_DEEP_DIVE_2026-08-21.md)
- [`FOCUSED_REPOSITORIES_VISUAL_WEB_AND_APP_EXPERIENCE_DEEP_DIVE_2026-08-21.md`](FOCUSED_REPOSITORIES_VISUAL_WEB_AND_APP_EXPERIENCE_DEEP_DIVE_2026-08-21.md)
- [`ZLOS_CURRENT_VISUAL_WEB_AND_APP_EXPERIENCE_BASELINE_2026-08-21.md`](ZLOS_CURRENT_VISUAL_WEB_AND_APP_EXPERIENCE_BASELINE_2026-08-21.md)
- [`ALL_33_VISUAL_WEB_AND_APP_EXPERIENCE_MATRIX_2026-08-21.md`](ALL_33_VISUAL_WEB_AND_APP_EXPERIENCE_MATRIX_2026-08-21.md)
- [`CANONICAL_FEATURE_IMPLEMENTATION_CATALOG_2026-08-21.md`](CANONICAL_FEATURE_IMPLEMENTATION_CATALOG_2026-08-21.md)
- [`DRIVER_AND_APP_AUDIT_TAXONOMY_2026-08-21.md`](DRIVER_AND_APP_AUDIT_TAXONOMY_2026-08-21.md)
- current zlOS `kernel/HANDOFF.md`, `kernel/docs/visual-speed-northstar.md`, `kernel/docs/look-and-speed.md`, `kernel/docs/desktop-scale-and-effects.md`, `kernel/docs/evidence/desktop-feel.md`, and `kernel/docs/retained-window-surfaces.md` in `/home/roy/Documents/repos/zl-linux`

No upstream source was copied. No repository was built, booted, modified, committed, or pushed for this pass.
