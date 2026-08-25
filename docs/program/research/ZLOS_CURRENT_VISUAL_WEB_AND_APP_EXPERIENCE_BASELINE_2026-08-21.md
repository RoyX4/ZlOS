# Current zlOS visual, web, and application-experience baseline

Date: 2026-08-21

Scope: the active shared checkout at `/home/roy/Documents/repos/zl-linux`, read
against current source, its design-system specification, current screenshots,
browser records, app registry, and the already validated driver/application
baseline. This is a read-only clean-room destination audit. No zlOS source was
changed and no kernel build or boot was run.

## Why this baseline exists

The external-repository audit can only be useful if it distinguishes four
different statements:

1. a screenshot or mock-up contains a visual;
2. source can draw that visual;
3. a real user route can manipulate it;
4. the underlying data, service, or device operation is real.

Those are not interchangeable. A beautiful Services window does not prove
restartable services. A Browser window does not prove an isolated web-content
process. A catalogue tile does not prove the app can launch. This baseline is
the destination-side control that prevents those category errors.

## Evidence vocabulary

- `[S]`: substantive current source exists.
- `[R-build]`: source is in the active build inventory.
- `[R-init]`: reached by the normal boot/desktop path.
- `[R-user]`: a concrete pointer, keyboard, shell, dock, catalogue, or app route
  reaches the behavior.
- `[T]`: a deterministic host or target gate exists.
- `[QEMU]`: the behavior is recorded in an emulator scenario.
- `[H]`: the exact behavior, not merely an adjacent driver, has native physical
  evidence.
- `[Shot-current]`: a current screenshot was inspected during this pass.
- `[Shot-old]`: an older screenshot was inspected but is not current product
  truth.
- `[P]`: partial, approximated, coupled, or below the claimed product boundary.
- `[F]`: evidence contradicts the broader claim.
- `[Mock]`: visible in a reference or model, not a working zlOS behavior.

Evidence attaches to a behavior. It does not automatically propagate from a
controller to a device class, from a game rule to its desktop launch route, or
from a window to the service it depicts.

## Current visual direction

The current direction is not the older navy/cyan desktop shown in
`docs/evidence/visual-diffs/after-merge-help.png`, `look-2560.png`, or `dock-menu.png`. Those
screenshots remain useful interaction history, but the live reference-derived
system is a dark graphite depth ladder with a single lime accent.

`kernel/shots/current-critique.png` was inspected in this pass and shows the
new direction: a near-black grid-and-glow wallpaper, a compact floating top
island, dark windows with subtle depth, lime focus/telemetry accents, and a
small translucent-looking dock. It also honestly shows unfinished product
polish: overlapping windows hide content, title and body type are small at the
captured resolution, the dock icons are low-detail, and the central window's
empty content dominates the composition.

### Palette and depth `[S][R-build][T][Shot-current]`

`kernel/design.h` is the single token authority. Current roles are:

- eight ordered surfaces from `#07080a` through `#474b50`;
- seven text levels from `#eef0f2` through `#5c6167`;
- default accent `#b8e838`, with bright/link/pale derivatives;
- semantic success `#a9e34b`, warning `#f5b93c`, and failure `#ff6a50`;
- named intermediate surfaces for tabs, wells, cards, headings, games, body,
  and unfocused title bars;
- computed dark-or-light ink on arbitrary accent colors.

The important design rule is structural: adjacent surfaces climb one depth
step. Color is not decoration pasted onto arbitrary panels. Focus is a faint
lime ring and glow, while unfocused chrome steps down and loses the glow.
`hosttest/palette.c` checks the reference -> token -> theme chain so two
simultaneous palettes cannot silently return.

### Shape, elevation, and wallpaper `[S][R-build][T][P]`

The token set contains radii from 4 to 20 pixels, with 11/12 pixels as the
common component shape and 16 pixels for windows, dock, and top island. It also
records a 20-pixel vertical / 50-pixel blur / 72% window shadow and the
reference blur radii.

The current renderer can draw rounded rectangles, alpha blends, gradients,
shadows, generated icons, supersampled corners, and a cached wallpaper. It
cannot reproduce a browser-style live `backdrop-filter` under arbitrary rounded
content. The present approximation works over the deliberately low-frequency
wallpaper; it is not general blur parity.

The current wallpaper is a strong identity move: grid, diagonal shadow forms,
green atmospheric glows, and a slow sweep. It avoids generic wallpaper art and
makes the accent part of the environment. It needs quieter contrast beneath
text-heavy windows and a reduced-motion/static mode.

### Typography `[S][R-build][T][P]`

zlOS has proportional and monospace paths, real bold atlases, synthesized
oblique, antialiased/subpixel text, gamma-correct blending, clipping, and
multiple browser heading sizes. Terminal text stays monospace while ordinary
UI text is proportional.

The reference specifies eight desktop sizes from 9 to 12.5 pixels, including
half-pixel steps, but the current bitmap-atlas runtime maps them to three
available atlases. Letter spacing is absent. Browser headings reach six
relative sizes, although large headings may be bilinear enlargements and look
softer than native glyphs. Baselines assume one atlas family. These are visible
system-wide limits, not per-app cleanup.

Destination requirements:

- one scalable shaping/rasterization service or library used by shell, apps,
  browser, terminal, and accessibility;
- explicit UI, reading, code, and large-display roles rather than raw pixels;
- fallback, shaping, bidirectionality, combining marks, emoji/symbol, and
  localization coverage;
- per-monitor fractional scaling and text zoom independent of layout zoom;
- contrast, large-text, and reduced-motion profiles.

### Icons and imagery `[S][R-build][R-user][P]`

The desktop has an icon atlas and generated glyphs for shell and app surfaces.
Current app registry entries map most added apps to atlas glyphs and retain
two-letter fallbacks. Existing screenshots show the transition from blocky,
large dock imagery toward the current compact monochrome treatment.

This is enough for a coherent bring-up desktop, not a mature icon system. The
destination needs optical sizes, selected/disabled/badged states, semantic
names independent of atlas positions, high-DPI rendering, app-owned icons,
and an icon fallback that is visibly a fallback rather than an unexplained
initialism.

### Motion `[S][R-build][T][R-user]`

`ease.c`/`ease.h` implement fixed-point cubic Bézier timing rather than one
generic smoothstep:

| Motion | Duration | Purpose |
|---|---:|---|
| window `zwin` | 200 ms | open/settle from 96.5% and +10 px |
| pop `zpop` | 80/100/110 ms | menus, chips, tiles |
| press `zpress` | 250 ms | tactile press dip |
| pulse `zpulse` | 1,000/2,600 ms | attention/activity |
| overlay `zov` | 160 ms | modal/overview settle |
| toast `ztoast` | 160 ms | notification entrance |
| wallpaper `zsweep` | 7,000 ms | ambient linear sweep |

Window, overlay, toast, ghost, and wallpaper animation paths are present in
`wm.c`. Animation correctness is not inferred from compiling: the reference
work records a prior failure where handlers changed state without damaging the
window, so nothing repainted. The destination requires animation-state tests,
frame-time receipts, interruption/reversal behavior, and reduced motion.

## Component and interaction system

### Implemented component families `[S][R-build][T]`

The current `ui.c` and `uikit.c` surface is much larger than a handful of
buttons. It includes:

- flowing row/column placement, density-aware metrics, hit testing, focus
  indexing, activation, and focus rings;
- labels, separators, buttons, destructive/primary treatments, icon buttons,
  pills, toggles, sliders, segmented controls, and multiple tab styles;
- toolbar/status rows, shared column grids, sortable headers, list rows,
  sidebars, nav rows, stat strips, meters, progress bars, mini bars, and disk
  segment bars;
- monospace/log/hex/editor rows, cards, key/value grids, sparklines, badges,
  status dots, and text inputs;
- popovers, menus, modals, toasts, notifications, a command palette, empty
  states, calculator keypad, clock readouts, and shell chrome.

`kernel/docs/reference/ui/widgets.md` records 68 widget patterns and 18 inconsistencies in
the external reference. zlOS deliberately corrects several: ink contrast on
the lime accent, CPU chart clipping, and header/row track drift.

### Component gaps

The current API is predominantly immediate-mode drawing in trusted kernel
space. It does not yet constitute an application-platform contract. Missing or
incomplete concerns include:

- explicit disabled, loading, busy, validation-error, read-only, pressed,
  hover, keyboard-focus, touch-focus, and destructive-confirmation states for
  every interactive component;
- semantic roles/names/values/actions and an accessibility tree;
- pointer capture, drag cancellation, touch targets, gestures, IME, selection,
  undo, clipboard types, and robust text editing;
- responsive constraints and overflow policies independent of one reference
  resolution;
- bidirectional layout, localization expansion, plural/time/date formatting;
- process-safe rendering commands and surface damage rather than direct global
  framebuffer drawing.

The original reference has almost no hover or keyboard-focus language. Any
such behavior in zlOS is product design, not fidelity work, and should be
specified and tested as such.

## Desktop and window experience

### Real current interactions `[S][R-build][R-init][R-user][T]`

`wm.c` contains actual window open/close, focus/raise, minimize, move, resize,
maximize, modal ownership, tabs, z-order, damage, repaint, toasts, drag ghosts,
snap previews, keyboard focus cycling, Super-key snapping, double-click title
maximize, wheel routing, and frame telemetry. Workspace routing is documented
and current, not merely a visual mock.

The shell includes:

- a compact top island with Activities, clock, health/status indicators, and
  session control affordance;
- an application dock with open/running indicators;
- an All Applications catalogue and a Run surface;
- focus distinction, window controls, resize grips, drag/move/resize, snap,
  minimize/maximize/close, Alt-Tab/focus cycling, and workspaces;
- settings for input and appearance, including a persistence path;
- system monitor, notifications/toasts, quick/status concepts, and telemetry.

### Current proof ceiling

The inspected current screenshot proves composition and rendering, not every
interaction. Older screenshots prove earlier states only. `wmtest`, feel tests,
animation tests, palette tests, screenshot oracles, and QEMU probes cover
different layers and none substitutes for a native physical interaction trace.

At present, much of the desktop and every built-in app share kernel memory and
draw directly. A crash, hang, or malformed document can therefore affect the
desktop boundary in ways a normal app should not. The clean-room target keeps
the visible behavior but moves compositor, shell, services, browser content,
and applications into explicit processes with bounded IPC and revocable
surface/input authority.

## Browser and web experience

### What the current browser really does `[S][R-build][R-user][T][QEMU]`

The browser is a real bounded product path, not a screenshot or static page:

- URL field, address-versus-search interpretation, Back, eight-entry history,
  click navigation, keyboard input, selection, scrolling, resize/reflow, and
  explicit status/error text;
- DNS, TCP, HTTP, verified TLS 1.3/X.509 and a boot-time network path;
- HTML recovery parsing, links, headings, paragraphs, lists, code/pre,
  emphasis, div/span, images, tables, and entities;
- CSS cascade/specificity, selectors subset, box model, padding/borders,
  backgrounds, widths/min/max, centering, overflow clipping, floats,
  positioning, flexbox, grid, and a bounded `@media` subset;
- PNG from network and inline data URIs;
- external stylesheets fetched before images;
- bounded inline JavaScript evaluated after parse, with `document.write`
  output appended once and one reparse;
- explicit on-screen overflow/refusal/failure reporting rather than silent
  corruption.

The current recorded end-to-end result fetched a real Wikipedia page by name
over verified TLS, rendered 120 KB of body with images/flex/grid, and exposed
the result through the Browser app. Source comments and an older section of
`browser-status.md` still say JavaScript or HTTPS is refused; live source and
the later completion audit supersede those stale statements.

### Honest web limits `[P]`

This is intentionally not Chrome compatibility:

- one trusted in-kernel browser/global state, not browser/UI/network/content/
  decoder processes;
- fixed-capacity arenas and explicit truncation rather than general virtual
  memory allocation;
- limited selector/pseudo-class support and no complete modern web platform;
- media queries evaluated at stylesheet parse time, not dynamically on every
  resize;
- no full DOM mutation/event loop, module loader, fetch/XHR, storage, cookies,
  downloads, permissions, extensions, accessibility tree, devtools, printing,
  tabs-as-isolated-sites, profiles, bookmarks, password manager, or sandbox;
- bounded inline scripts only; external script loading and compatibility with
  modern application frameworks are not established;
- PNG is the substantive image path; broad image/video/audio codecs and color
  management are absent;
- HTTP/1.0 is deliberate; HTTP/1.1/2/3 behavior is not present;
- synthesized oblique and atlas scaling remain visible typographic limits.

The next milestone should not be “add random CSS until more sites look less
wrong.” It should create browser process boundaries, a navigation/request
contract, origin/site isolation, cancellation/deadlines, cache/storage policy,
permissions UI, accessibility semantics, and reproducible web-platform tests.

### Browser product flow to preserve

The best current quality is honesty. A malformed URL, missing network, refused
content type, failed certificate, parser overflow, script error, or unsupported
path gets a named state. Preserve that directness while upgrading the UX:

```text
new tab/home
  -> type URL or search
  -> resolve/connect/TLS/request with cancellable progress
  -> commit navigation once
  -> parse/style/layout/paint in bounded content process
  -> load ordered subresources
  -> interactive document with origin-bound capabilities
  -> back/forward/reload/download/open-external decisions
```

Every transition needs a user-visible status, cancellation, peer-death result,
and evidence receipt. A padlock means verified identity and current policy; it
must never mean merely encrypted transport.

## Complete current application inventory and user jobs

There are 61 named implementations plus the All Applications catalogue in the
current source inventory. That number is source inventory, not a launchability
claim: the known registry defect makes Maze source-present but not catalogue-
reachable and leaves one blank substitute tile.

### Shell and core product surfaces

| App/surface | User job and visible workflow | Underlying reality / next boundary |
|---|---|---|
| Terminal | issue commands, read output, launch tools/apps | real shell route, kernel-resident; migrate to PTY/session process |
| System Monitor | inspect CPU, memory, frame and system state | real counters; consume typed telemetry rather than globals |
| About | inspect product/system identity | real window; add immutable build/provenance receipt |
| Menu | open the shell application/task menu | real shell-owned surface rather than an ordinary destination app; replace static IDs with generated admitted-app metadata |
| Browser | navigate and read bounded web content | real stack above; split into broker/content/network/decoder processes |
| Settings | change pointer/visual/system preferences | real controls and persistence; add schema, rollback, per-user profiles |
| Run | launch by command/name | real launcher; make parsing and launch authority explicit |
| All Applications | browse and launch catalogue | real registry UI, currently false-green for Maze/blank ID |
| Files | list/open/edit named files | real zlfs path; move to VFS/file-service handles |
| Text Editor | edit text and save named files | real save path; needs selection, undo, encoding, recovery, multi-document state |

### Creative, graphics, and inspection

| App | Current user-visible idea | Product completion needed |
|---|---|---|
| Paint | direct pixel drawing | tools/layers/selection/history/file formats/export and crash recovery |
| 3D | rotating software-rendered cube | reusable scene/render API, input/camera model, asset loading |
| zlOS animation | branded motion demonstration | convert into boot/demo/visual regression fixture |
| Pointer | visualize pointer input | per-device events, acceleration curve inspector, touch/stylus separation |
| Renderer | inspect rendering primitives/scene | backend comparison and correctness/performance receipts |
| Framebuffer | inspect capture/scanout | privileged capture portal with privacy/authority |
| Font Atlas | inspect shipped glyph atlas | coverage/search/metrics/fallback diagnostics |
| Image Viewer | open and inspect an image | decoder isolation, zoom/pan/rotate/metadata/color/profile/error UX |
| Colour Picker | inspect/select/copy a color | screen-pick authority, formats, palette/history and contrast check |

### Utility workflows

| App | User job | Important completion boundary |
|---|---|---|
| Clipboard | inspect/copy session clipboard | MIME offers, ownership, expiry, privacy, explicit grants |
| Regex Tester | test a pattern against text | time/memory limits, highlighting, dialect label, safe failure |
| Base Converter | convert number bases | arbitrary precision/overflow/error semantics, copy actions |
| Text Diff | compare two text inputs | file handles, scalable algorithm, syntax/word/line modes |
| Checksum | hash input/file | algorithm identity, streaming file handle, verification workflow |
| Unit Converter | convert typed quantities | unit metadata, precision, localization, favorites/history |
| Sticky Notes | capture small persistent notes | durable per-user data, autosave, recovery, privacy |
| Keyboard Tester | visualize key events | physical/logical key distinction, layouts, repeat/modifiers, privacy |
| Benchmark | run and report workload | exact machine/backend/workload identity and comparable receipts |
| Calculator | perform arithmetic | expression history, keyboard entry, precision and error states |
| Clocks & Timers | clock, countdown, laps | monotonic/wall-clock split, alarms/notifications, suspend behavior |

### System and diagnostic workflows

| App | User job | Important completion boundary |
|---|---|---|
| System Info | inspect hardware/software identity | consolidated read-only inventory service and copy/export |
| Kernel Log | inspect system events | filtered/redacted logger stream, privilege and persistent receipts |
| Hex Viewer | inspect file/device bytes | read-only default, huge-file paging, offset/search, guarded write mode |
| Console (`tty1`) | use a low-level console | define consolidation/retirement versus Terminal and stable PTY identity |
| Disk Usage | see space and largest consumers | VFS snapshot semantics, permissions, cancellation and removable media |
| Services | inspect logical system providers | real supervisor state/actions, not labels over in-kernel modules |
| Archive Manager | list/extract/create archives | decoder sandbox, traversal/bomb/link checks, transactional extraction |
| Network | inspect interface/address/link state | network service, connection diagnostics, Wi-Fi/VPN later |

### Games and interactive fixtures

The 24 named games are Snake, Word Guess, Tic-Tac-Toe, Nim, Tower of Hanoi,
Lights Out, Connect Four, Maze, Tetris, Pong, Breakout, Minesweeper, 2048,
Conway's Life, Asteroids, Invaders, 15 Puzzle, Reversi, Simon, Sokoban,
Flappy, Missile Command, Blackjack, and Frogger.

They collectively exercise keyboard/pointer routing, focus, timers, animation,
collision/rules, deterministic randomness, grids, text, overlays, score/state,
and repaint. Their rules have meaningful host coverage, but those checks do
not prove every desktop launch route or rendered interaction. Destination use:

- deterministic seed and replay log;
- pause/resume/background/throttle semantics;
- save/high score as user-scoped storage;
- audio cues through AudioServer, not direct device writes;
- frame and input latency budgets;
- process/resource quotas and crash containment;
- accessibility alternatives for color, timing, motion, and keyboard controls.

### Current registry contradiction `[F]`

`APP_MAZE = 34`, while `reg_exists()` rejects IDs 34 through 39. The catalogue
checker prints `exists NO` but still exits successfully, and ID 14 is treated as
existing without a named implementation. The visible total stays plausible
because the blank tile substitutes for Maze. Therefore:

- app source inventory: 61 named implementations;
- catalogue completeness: not proven;
- Maze rules/source: real;
- Maze human launch path: contradicted by current registry;
- “all 53 reference apps resolve” gate: false green.

A replacement gate must mutation-test every required field: valid unique ID,
exact name, icon, size, constructor, draw/event/tick routes, shipped-image
membership, human launch route, launch-ready nonce, and clean close/teardown.

## Missing product app families

The current suite is unusually broad for a from-scratch OS, but it is strongest
at diagnostics, compact utilities, demos, and games. Product gaps include:

- login, lock, user/session/elevation, account, and permission surfaces;
- notification center/history, clipboard portal, file chooser, share/open-with,
  secrets, and app permission dashboards;
- mail, calendar, contacts, messaging, and communications accounts;
- PDF/ebook/document reading plus document, spreadsheet, and presentation
  creation;
- media library/player, audio recording/mixing, camera, and screen recording;
- package/store/update/rollback UI with provenance and permissions;
- print queue, printer setup, scanning, and document capture;
- debugger/profiler/source/package-development environment;
- accessibility center, screen reader, magnifier, switch control, captions,
  remapping, high contrast, large type, and reduced motion;
- localization, keyboard layouts, input methods, timezone/locale formatting;
- battery, thermal, power profiles, suspend/resume, device and removable-media
  management;
- safe remote/public-demo/rescue experiences.

## Best current zlOS ideas to protect

1. **One measurable token authority.** The palette is derived and gated, not
   duplicated by eye.
2. **Visual fidelity as several measurements.** The 62-region oracle separates
   color, palette, hue, and structure; one whole-screen score cannot hide a
   local regression.
3. **Motion is source and state, not a screenshot.** Curves, durations, damage,
   and frame timing are independently inspectable.
4. **The browser is deliberately bounded and honest.** It names refusals and
   truncation instead of pretending to be a complete modern engine.
5. **Apps double as platform probes.** Games, diagnostics, and utilities expose
   input, rendering, storage, networking, timing, and service-contract gaps.
6. **Software rendering remains the oracle.** Accelerated paths can be compared
   against a known-correct implementation.
7. **The product has a visual identity.** The lime/graphite/grid direction is
   recognizably zlOS and should not be diluted into a generic desktop clone.

## Clean-room destination boundary

```text
design tokens + semantic component specifications
                    |
font/icon/theme/localization/accessibility services
                    |
compositor + shell + window/surface/input authority
                    |
typed bounded IPC + app lifecycle + portals
          /---------+----------\
 browser chrome                ordinary applications
     |                                |
 navigation/network broker      file/media/device services
     |
 origin-isolated content + decoder processes
```

The external repositories can contribute interaction patterns, component
ideas, app workflows, browser architecture, negative cases, and proof methods.
They do not replace zlOS's identity, current apps, current browser, or physical
hardware spine, and their source is not copied.

## Immediate integration priorities

1. Repair and mutation-proof the app registry before treating catalogue size as
   product evidence.
2. Freeze the visual token/component schema and add accessibility semantics and
   complete interaction states.
3. Move shell/apps from direct framebuffer/global-state access to surfaces,
   typed events, handles, and restartable services.
4. Split the browser into chrome, navigation/network, content, and decoder
   boundaries before expanding compatibility aggressively.
5. Build core portals: file chooser, open/share, clipboard, notification,
   secrets, capture, permissions, and package/update.
6. Complete typography/scaling/localization, because every screen inherits the
   current atlas and spacing limits.
7. Turn every app family into a user-journey contract with empty/loading/error/
   offline/recovery/accessibility states and deterministic proof.
8. Keep current screenshot, interaction, host, QEMU, and physical evidence
   separate in every future claim.

## Weakest link

The baseline can inspect current source, current screenshots, and recorded
gates, but it did not boot the dirty shared checkout. The exact current
launchability and appearance of every app therefore remain below direct runtime
verification. The known false-green registry demonstrates why that distinction
must remain explicit.
