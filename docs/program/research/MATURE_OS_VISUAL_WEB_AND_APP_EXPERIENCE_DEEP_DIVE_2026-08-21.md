# Mature OS visual, web, and application experience deep dive

Date: 2026-08-21
Scope: Brook, Astral, banan-os, LemonOS, SerenityOS, MollenOS, hhuOS, and duckOS
Method: design-system audit plus system-design audit, at the pinned revisions below
Intent: clean-room feature and behavior study for zlOS; no source code was imported

## Executive result

There is no single desktop here that zlOS should copy.

The strongest result is a deliberate combination:

- **SerenityOS owns the broadest complete native product system**: a semantic theme engine, reusable GUI toolkit, desktop shell, 51 active graphical applications, 13 games, settings surfaces, document and creation tools, and the only native browser/web engine in this group. Its process separation, portal services, declarative layouts, settings domains, and app-as-library-consumer strategy are more valuable than its deliberately retro default appearance.
- **LemonOS owns the cleanest small native desktop slice**: a centralized 13-role theme, rounded translucent shell, launcher metadata, task buttons, shared GUI widgets, generated IPC, compact system utilities and config/theme support, and coherent orange-on-charcoal visual identity. It demonstrates how little code is needed once shell, theme, registry, and app conventions line up.
- **duckOS owns the best small app-bundle seam**: `.app` directories carry names, executables, icons, hidden state, file extensions, and extension-specific icons. The same metadata powers its launcher, Files, Viewer, Editor, process monitor, and `open` command. Its five file-backed themes and reusable widgets are more systematic than its tiny asset set suggests.
- **banan-os owns the strongest tiny native interaction ideas**: a PATH-discovering fuzzy launcher, a live CPU/battery/audio/time bar, transparent rounded windows, pan/zoom image viewing with selectable interpolation, and an optional X compatibility bridge. It is a good feature sketch, not yet a complete product shell.
- **Brook owns the strongest compatibility-first rescue strategy**: small native Files/Edit/Console/Calculator/Player tools remain usable while Wayland and imported freedesktop metadata expose larger third-party applications. Its taskbar also surfaces compositor FPS and GPU-path state, making graphics evidence visible to the user.
- **Astral owns a compatibility distribution, not a native desktop design**: Xorg, FVWM/NSCDE, GTK, Qt, WebKitGTK, Wine, media frameworks, browsers, editors, and games provide extraordinary breadth, but almost all visual and application behavior belongs to upstream projects.
- **hhuOS owns a workload laboratory rather than a desktop**: its Lunar widgets, Pulsar game framework, framebuffer demos, emulators, 2D/3D games, audio probes, and graphics ports create a valuable proof ladder for hardware and libraries. It has no window manager, taskbar, launcher, Files, graphical editor, or browser.
- **MollenOS owns service/package architecture, not the checked-in GUI experience**: the polished launcher screenshot belongs to external `vali-userspace`. The current repository contributes generated IPC, supervised services, read-only application packs, and two native CLI utilities. Treating its screenshot as local implementation would be false.

The clean-room destination is therefore not “make zlOS look like Serenity.” It is:

1. define a semantic, versioned visual contract;
2. make one app manifest drive launching, opening, icons, search, permissions, restoration, and diagnostics;
3. put window, input, clipboard, audio, notifications, settings, file operations, networking, decoding, and web content behind bounded user-space services;
4. build the small daily-use suite first as shared-platform conformance clients;
5. add a split-process browser as a platform, not a monolithic demo;
6. keep graphics/performance/provider truth visible in normal UI;
7. prove keyboard, scale, contrast, locale, failure, restart, and persistence behavior, not just screenshots.

## Evidence contract and limits

This pass was read-only and did not build or boot any repository. Each statement is intentionally bounded:

| Mark | Meaning |
|---|---|
| `SRC` | Implementation exists in source at the pinned revision. |
| `BUILD` | The active build or registration graph reaches the target. |
| `ASSET` | Shipped image, font, icon, theme, wallpaper, metadata, or configuration exists. |
| `SHOT` | A repository screenshot was visually inspected. It proves only that somebody captured that state. |
| `DOC` | Documentation describes the behavior. It may be stale or aspirational. |
| `TEST` | A test or automation path exists. It was not executed in this pass. |
| `3P` | Behavior is supplied by a port, package, submodule, or separate repository. |
| `R?` | No fresh runtime proof was collected here. |

Pinned revisions:

| Repository | Revision |
|---|---:|
| Brook | `76c3155394d3` |
| Astral | `465437a97448` |
| banan-os | `c915c064e8e4` |
| LemonOS | `15f607e8be86` |
| SerenityOS | `3d83e4509fd2` |
| MollenOS | `f7d88d4d6f77` |
| hhuOS | `39bf35c6b98b` |
| duckOS | `3634e410c359` |

The companion [driver and application deep dive](MATURE_OS_DRIVERS_AND_APPS_DEEP_DIVE_2026-08-21.md) is the exhaustive target-count and low-level inventory. This report focuses on what those targets mean as a visible product and how the product contracts are implemented.

## Cross-system design-system matrix

| System | Visual source of truth | Fonts and scale | Icons/assets | Themes | Interaction state | Honest assessment |
|---|---|---|---|---|---|---|
| Brook | Kernel chrome constants plus repeated per-app dark palette | Kernel atlas; native apps embed 6x10 bitmaps; ported apps use fontconfig | Wallpaper, cursor theme, imported desktop icons | No central native theme service | Hover/focus/minimized/active states in shell; custom states per app | Visually coherent by convention, not by enforceable system |
| Astral | Upstream FVWM/NSCDE, GTK, Qt, X resources | Upstream font stacks | Upstream Adwaita/hicolor and app assets | NSCDE palette plus upstream themes | Owned by upstream toolkits/window managers | Broad and configurable, but Astral itself owns almost no design contract |
| banan-os | Hardcoded LibGUI/app colors; WindowServer config for background/radius | Fixed bitmap-oriented GUI | Small app-specific assets | Wallpaper and radius config, no semantic theme schema | Base widgets have hover; shell/apps implement additional states manually | Strong tiny visual identity; weak centralization and scale story |
| LemonOS | `Theme` semantic roles plus WM resource theme | Montserrat, Nunito, Source Code Pro | Wallpapers, cursors, resize cursors, app/menu/window icons | Default theme active; `classic.json` has stale incompatible schema | Focus, hover, active/inactive, pressed, selection | Best small coherent system; metrics and token vocabulary still narrow |
| SerenityOS | Theme INI roles, metrics, flags, alignments, paths, color schemes | Multiple installed fonts; GUI font database/settings | Large 16/32 icon vocabulary, cursors at 1x/2x, wallpapers, shadows | 23 theme files plus editor color schemes | Extensive widget/window/menu/selection/focus/disabled states | Most complete design system; retro default is only one skin |
| MollenOS | Not present in current repository | External userspace | Screenshot and docs only | External | External | Do not infer a local design system from external Vioarr imagery |
| hhuOS | `HhuTheme` per-widget state tables | Terminal 8x8 default | Workload/game assets, no desktop icon system | Runtime-replaceable `Theme`, one built-in HHU theme | Normal/hovered/pressed; widget focus bit | A compact widget skin, not a desktop design system |
| duckOS | Parsed `.thm` files with semantic colors/values/strings/images | Gohu 11 and 14 BDF; theme chooses normal/mono | Minimal app/filetype/cursor set; one wallpaper | default, light, hotdog, vapor, vapor_dark | Focused/unfocused title, raised/pressed widgets, hover, selection | Surprisingly real system under a deliberately small retro surface |

### Repeated visual strategies

1. **Semantic roles beat raw palette reuse.** LemonOS, SerenityOS, and duckOS can change many components coherently because widgets ask for roles such as window, accent, selection, title, or scrollbar. Brook's native tools repeat very similar Catppuccin-like values but cannot atomically change them.
2. **Metrics are part of a theme.** Serenity stores border radius, border thickness, title height, and button sizes alongside colors. Lemon stores three window metrics. duckOS stores button padding and progress height but hardcodes titlebar/border geometry in `Window.cpp`. Color-only theming leaves density and accessibility inconsistent.
3. **The asset vocabulary controls perceived completeness.** Serenity's icon and cursor coverage makes settings, file types, actions, devices, and applications legible. duckOS and LemonOS have sound primitives but visibly run out of distinct symbols. hhuOS has no desktop vocabulary at all.
4. **Motion is rare and mostly functional.** These systems favor immediate state changes, damage-driven redraw, hover, selection, window drag/resize, progress updates, and occasional flashing or bouncing. None provides a broad motion-token or reduced-motion system. That is a gap zlOS can own.
5. **A screenshot can conceal platform debt.** MollenOS's clean launcher is external. Astral's mature windows are upstream. Conversely, duckOS's simple screenshot conceals a meaningful theme/app/IPC design. Source ownership must stay explicit.

## Cross-system desktop and interaction matrix

| Capability | Brook | Astral | banan-os | LemonOS | SerenityOS | MollenOS | hhuOS | duckOS |
|---|---|---|---|---|---|---|---|---|
| Native desktop/window manager | Yes, kernel WM + Wayland bridge | No, Xorg/FVWM/NSCDE `3P` | Yes, WindowServer | Yes, LemonWM/WindowServer | Yes, WindowServer | External Vioarr `3P` | No | Yes, Pond |
| Taskbar/panel | Window buttons, Apps, terminal, FPS/GPU/clock | FVWM/NSCDE `3P` | CPU/battery/audio/time; no window list found | App/window buttons + CPU/memory | Window buttons, quick launch, tray/menu, show desktop | External | No | Apps + memory/CPU/time; no window list found |
| Launcher | Shortcut scripts + imported `.desktop` entries/icons | Upstream menus | PATH executable substring search | `.cfg` metadata, categories, search | App metadata, system menu, Run, quick launch | External screenshot | No | `.app` metadata list with icons |
| Window actions | Move, resize, min, max, close, task switching | Upstream | Move/resize/fullscreen/focus/close | Move/resize/minimize/close/task switching | Move/resize/min/max/tile/workspaces/flash/menus | External | No | Move/resize/focus/close/shadows |
| Workspaces | Not found | Upstream-dependent | Not found | Not found | Configurable grid | External/unknown | No | Not found |
| Notifications | No native center found | Upstream-dependent | No | No mature service found | NotificationServer + popup UI | External/unknown | No | No |
| Clipboard | Wayland data-device partial | X selections `3P` | ClipboardServer | Clipboard protocol | Clipboard service and GUI integration | External/unknown | Terminal/process dependent | No dedicated service found |
| Desktop persistence | Minimal | Upstream config/home | Wallpaper/radius config | WM/theme/menu config | ConfigServer + app settings/history/state files | Read-only packs/services | App-specific files | Config/theme + app files; no session restore |

### The shell behavior worth keeping

- Brook's `+` button makes “open a terminal” a first-class recovery action, while Apps imports freedesktop metadata. The taskbar's GPU path and compositor FPS labels turn a normally invisible systems fact into user-visible truth.
- banan-os's launcher needs no separate database: it scans executable files on PATH, deduplicates, sorts, and filters case-insensitively. That is an excellent fallback launcher even when the richer app registry is corrupt.
- LemonOS makes active-task click mean minimize and inactive-task click mean activate. It is compact and learnable.
- SerenityOS's quick launch, show-desktop behavior, workspace filtering, task context actions, and system menu together form the strongest complete navigation model.
- duckOS uses one app bundle model everywhere. This is more valuable than a visually sophisticated launcher fed by a separate, drifting registry.

## Browser and web result

| System | Browser path | Ownership | Architecture | Feature ceiling at this revision |
|---|---|---|---|---|
| Brook | NetSurf framebuffer package; Ladybird package/integration probes | `3P`, Brook integration glue | NetSurf libraries/assets; Wayland bridge; Ladybird launched through compatibility environment | Usable browsing is plausible from packaged engines; not a Brook-native web platform. Ladybird navigation script is `TEST`, not fresh runtime proof. |
| Astral | WebKitGTK MiniBrowser and Links recipes | `3P` | X11/GTK/WebKit stack; WebGL and WebAudio recipe options | Mature upstream engine surface, no Astral browser chrome or web service architecture |
| banan-os | Links and Lynx ports | `3P` | Terminal/port path; optional X bridge could host more | No native graphical browser or web engine |
| LemonOS | `Steal` HTTP/HTTPS downloader | Native downloader, not browser | URL parsing, sockets/OpenSSL, file output | No HTML/CSS/JS engine, browser chrome, history, tabs, storage, or web process model |
| SerenityOS | Browser + LibWeb/LibJS | Native | Browser chrome; WebContent helpers; RequestServer; LookupServer; ImageDecoder; WebWorker; portals and shared surfaces | Tabs, history, bookmarks, downloads, settings, inspector, storage tools, content filters, zoom/find/view-source/fullscreen, broad web APIs. Compatibility and some documented process goals remain incomplete. |
| MollenOS | None in current repository | — | — | External userspace may differ; not evidenced here |
| hhuOS | None | — | Network utilities only | No browser/web engine |
| duckOS | None | — | Network and image libraries do not form a web stack | No browser/web engine |

The most important correction is that “has a browser package” and “owns a browser platform” are different claims. Brook and Astral can expose mature upstream web software. Only Serenity owns chrome, engine, process helpers, settings, tests, storage UI, and platform integration in the audited tree.

## Browser architecture zlOS should take, without taking code

```text
Browser shell process
  tabs, address/search, history, bookmarks, downloads, permissions, recovery
        |
        +-- one bounded WebContent process per isolation policy
        |     HTML/CSS/DOM/layout/paint/JS/Wasm
        |     shared display lists or surfaces, never ambient framebuffer authority
        |
        +-- Request broker
        |     DNS, TLS, proxy, cookies, cache, downloads, quotas, cancellation
        |
        +-- disposable decoder workers
        |     image/font/media/PDF parse under low privilege
        |
        +-- storage broker
        |     origin-keyed cookies/local/session/IndexedDB with inspect/clear UI
        |
        +-- audio/video service and file-access portal
```

Serenity demonstrates the service shape, but its own process-architecture documentation says portions are aspirational. zlOS must prove the actual process graph, restart behavior, origin boundaries, handle revocation, decoder crash containment, and per-origin quotas at runtime.

## 1. Brook — native rescue desktop plus compatibility lane

Evidence: `SRC`, `BUILD`, `ASSET`, `SHOT`, `TEST`, `3P`, `R?`.

### Visual language

Brook's inspected desktop screenshot shows a modern colorful wallpaper, a thin dark bottom bar, and a dark navy terminal with a blue title strip. The native Files, Edit, Console, Player, and Calculator sources repeat a recognizable dark palette: deep navy/charcoal backgrounds, light blue-white text, muted slate, blue focus/accent, pink/red danger, and yellow warning. These values strongly resemble a Catppuccin-style palette, but there is no shared theme object or token file; each C program owns its constants.

The kernel WM separately hardcodes a 24-pixel titlebar, two-pixel border, blue focused title, dark inactive title, red close control, blue focus border, and 32-pixel bottom taskbar. It adds rounded taskbar buttons and circular window controls. The visual result can be coherent, but it is duplicated across kernel chrome, native tools, and third-party client-side decoration.

Font strategy is split. Kernel chrome uses a font atlas. Small native tools embed an ASCII 6x10 bitmap. Ported GTK/Qt/Wayland programs receive fontconfig and cursor paths. This supports compatibility but creates three typography and scale systems with no shared density or high-DPI contract.

### Desktop, windows, panel, and launcher

The kernel WM owns z-order, focus, move, edge/corner resize, minimize, maximize, close escalation, cascading placement, client- versus server-side decoration, and a per-window input ring. The compositor renders wallpaper, windows, chrome, taskbar, launcher, and alpha cursor in a damage-aware loop.

The bottom taskbar contains:

- **Apps**, opening a multi-column popup anchored above the bar;
- **+**, directly opening a terminal;
- responsive window-title buttons, including minimized state and active state;
- real clock/date;
- GPU/provider badge indicating software, virtio presentation, or confirmed host 3D state;
- compositor FPS.

The launcher merges Brook shortcut scripts with generated `applications.idx` entries. Its importer scans Nix-store `.desktop` files, ignores `NoDisplay`, extracts name, command, icon, and categories, converts icons to a kernel-consumable format, and refreshes after package install/remove. The launcher supports icons, hover, several columns, clipping, and wheel scrolling. It does not expose category navigation, semantic search, favorites, permissions, or keyboard-first search in the inspected kernel UI.

Wayland clients go through `waylandd`, which maps `xdg_toplevel` move, resize, maximize, minimize, popups, shared-memory buffers, cursor surfaces, and input into kernel WM operations. ARGB surfaces retain alpha, allowing client-side shadows. Data-device plumbing exists, but drag-and-drop is not established as a complete workflow in this pass.

### First-party workflows

- **brook-files**: 1024x700 two-pane browser with directory tree, toolbar/address, breadcrumbs, row sorting, status, scrolling, navigation/up, a tree toggle, and configurable file associations. It is a credible rescue file manager, but has no thumbnails, search, batch operations, trash, mounts UI, permissions UI, or durable view preferences.
- **brook-edit**: line-number gutter, current-line highlight, open/save, `Ctrl+S`, line duplication, find/next/Escape, status, and bounded in-memory line storage. It is a useful text rescue surface, not a language-aware editor.
- **brook-console**: an 8,192-line log ring with severity coloring, scrolling, and filtering. This is a standout systems UX: diagnostics are a normal app, not serial-only output.
- **brook-player**: FFmpeg decode, video into double-buffered Wayland shared memory, audio resampling to stereo 44.1-kHz signed 16-bit, fullscreen and auto-exit switches. It is a single-file player path, not a library/playlist/media-management product.
- **wayland-calc**: 240x320 keyboard/pointer calculator with a custom 4x5 button surface.
- **package and diagnostics tools**: native fetch/install/list/remove/update/configuration and hardware/network/graphics probes support recovery and integration, even when richer GUI ports fail.

### Web and third-party boundary

NetSurf is staged with its framebuffer UI assets and supporting DOM/CSS/image libraries. Its toolbar icon set includes back, forward, history, home, on-screen keyboard, reload, and stop. Ladybird and many other applications are external packages. Brook owns environment, launcher import, Wayland adaptation, and targeted integration scripts—not their UI implementations.

The Ladybird navigation automation drives two URLs and checks a former deadlock boundary. That is valuable `TEST` evidence of intended integration, but it was not executed here and does not prove present browser compatibility.

### Underneath and persistence

The visual stack is unusually kernel-heavy: window policy, chrome, input routing, launcher, app execution, taskbar, and wallpaper loading sit inside the kernel. Each userspace surface gets a virtual framebuffer mapping and input queue. Package installation regenerates launcher metadata. Native apps persist mostly through ordinary files/configuration; there is no cohesive settings or session-restoration service.

### Best ideas

- Keep minimal native rescue apps even when a compatibility lane carries richer software.
- Import upstream app metadata rather than maintaining a second launcher list.
- Put graphics provider and frame-rate truth in the normal shell.
- Make “new terminal” one click from any desktop failure state.
- Ship a user-facing filtered system log.

### Gaps and dead ends

- No central theme, scale, localization, accessibility, focus-ring, or reduced-motion contract.
- Fixed pixel dimensions and ASCII bitmap rendering constrain international text and high DPI.
- Kernel ownership of product policy creates a huge trusted surface.
- Launcher/search, drag-and-drop, notifications, settings, clipboard semantics, and app restoration remain shallow.
- The port catalogue creates breadth but cannot be counted as Brook-native app quality.

### Clean-room zlOS lesson

Take the rescue/compatibility split, metadata import, diagnostics app, and visible provider telemetry. Put the compositor and shell policy in restartable user-space services, share one token/font/icon system, and gate each compatibility feature with a real runtime receipt.

## 2. Astral — a distribution integration surface, not a first-party UX

Evidence: `SRC`, `BUILD`, `ASSET`, `SHOT`, `3P`, `R?`.

### What the screenshot actually proves

The inspected FVWM screenshot shows a galaxy wallpaper, xterm, xclock, xeyes, solitaire, and TyrQuake. That is a plausible usable X desktop. It does not prove Astral implemented those interfaces. The repository's native ownership is its boot/session glue, package recipes, a small administrative core, and integration configuration.

### Visual and shell choices

`startwm` launches X, waits for the display socket, merges X resources, starts an xterm login shell, then starts FVWM3. `startnscde` selects NSCDE. Astral supplies a wallpaper symlink into user homes and an NSCDE palette, while upstream FVWM/NSCDE own panels, menus, window frames, workspace behavior, shortcuts, and effects.

The result supports two directions:

- a lightweight configurable classic X/FVWM desktop;
- an intentionally retro CDE-like NSCDE desktop with generated palette.

GTK3/Adwaita/hicolor, Qt6/PyQt6, X resources, Mesa, SDL, and application-specific toolkits can all appear at once. This is powerful compatibility but not consistent first-party hierarchy, spacing, icons, or interaction.

### Applications and workflows

Astral has only five native administrative/trace/profile targets. Its user-visible breadth comes from 307 recipes, including:

- Xorg, xterm, xclock, xeyes, XFile, xNEdit, FVWM3, NSCDE, and screensavers;
- GTK3 and Qt6 application infrastructure;
- Mesa, SDL, GStreamer, and media/graphics libraries;
- WebKitGTK MiniBrowser and Links;
- Wine and compatibility libraries;
- DoomGeneric, TyrQuake, and other games/demos;
- developer, shell, network, archive, font, and Unix utility packages.

This is a valuable port laboratory. It is not accurate to say every recipe is necessarily part of the default image, and no recipe proves runtime on the pinned OS without a boot receipt.

### Web path

The WebKitGTK recipe enables MiniBrowser, WebGL, and WebAudio. Those switches establish build intent for upstream WebKit features, not Astral-owned browser chrome, isolation, settings, history, or standards compatibility. Links supplies a smaller independent browser path.

### Accessibility and localization

The recipe shelf includes `at-spi2-core`; Qt accessibility is enabled; GTK depends on accessibility infrastructure; gettext, iconv, ICU, and locale-related libraries are available. This is the strongest accessibility *package* story outside Serenity, but there is no Astral-authored assistive settings UI or verified screen-reader workflow in this checkout.

### Best ideas

- Use a compatibility distribution to discover which mature workflows users actually value before rebuilding them natively.
- Keep the native administrative/recovery core smaller than the port shelf.
- Offer a classic low-resource desktop path as a fallback.

### Gaps

- No Astral-owned visual system, app registry, desktop settings service, browser shell, accessibility experience, notification service, or cross-toolkit coherence.
- Wayland is not the active direction here.
- Many recipes disable tests or depend on upstream assumptions.
- The default-image boundary and present runtime state require independent proof.

### Clean-room zlOS lesson

Treat ports as product experiments and compatibility coverage, never as proof of native platform maturity. Wrap them in one app manifest, portal, theme-bridge, file-open, notification, clipboard, and crash-report contract so the boundary is explicit.

## 3. banan-os — tiny coherent native shell with sharp limits

Evidence: `SRC`, `BUILD`, `ASSET`, `SHOT`, `3P`, `R?`.

### Visual language

The screenshot shows a beach wallpaper and two translucent terminals. The active source supports alpha-blended windows, configurable background, configurable corner radius, focused decorations, and a dark translucent launcher. Individual apps hardcode colors. There is no shared semantic palette, spacing scale, typography scale, icon system, or file-backed theme schema.

LibGUI provides Button, Grid, Label, RoundedWidget, TextArea, Texture, MessageBox, Window, and base widgets. It is enough for compact tools, but launcher and parts of shell draw manually; ProgramLauncher even carries a FIXME to move to widgets. This creates duplicate state/rendering behavior.

### WindowServer and desktop behavior

WindowServer reads `$HOME/.config/WindowServer.conf` for background and corner radius. It owns direct keyboard/mouse input, window focus, move, resize, fullscreen, damage-driven rendering around 60 Hz, titlebars, and alpha blending. Fullscreen letterboxes/scales the focused window. An optional bouncing-window effect exists in source. If installed, `/usr/bin/xbanan` is launched as an X compatibility bridge.

The separate top TaskBar provides:

- aggregate CPU or per-core utilization, toggled by clicking;
- battery device state;
- AudioServer volume;
- wall time.

No task/window buttons were found in the inspected TaskBar source, so it is system telemetry rather than a complete task switcher.

ProgramLauncher opens a borderless translucent search surface. It scans every executable regular file on PATH, deduplicates and sorts names, filters with a case-insensitive substring, shows 15 rows with alternating backgrounds and a scrollbar, supports Up/Down, and executes with Enter. It is robust as a recovery launcher, but names are raw executables with no icons, descriptions, categories, aliases, permissions, or provenance.

### First-party workflows

- **Terminal**: PTY shell, terminal selection, `Ctrl+Shift+C/V` through ClipboardServer, bracketed paste, 8/24-bit ANSI color, and resize propagation to the PTY.
- **ImageViewer**: PNG/NetPBM-family loading through LibImage, drag-to-pan, wheel zoom centered around the cursor, and Space to toggle nearest/bilinear interpolation. The interpolation switch is an unusually good low-level visual inspection feature.
- **TaskBar**: live provider/system state described above.
- **ProgramLauncher**: executable discovery and keyboard-first filtering.
- **Audio/audioctl**: service-backed playback/control paths.
- **Snake** and small visual/test applications: native workload coverage.
- **Networking/administration**: DHCP, DNS lookup, HTTP server, shell/core utilities, and diagnostic programs.

There is no native graphical file manager, text editor, settings app, browser, document viewer, notification center, calendar, mail client, or media library.

### Services and third-party boundary

AudioServer, ClipboardServer, and WindowServer create a useful small process split. The 128-port shelf includes Links, Lynx, SDL2/3, GTK3/X, FFmpeg, games, shells, and utilities. `xbanan` is explicitly optional/external. Ports broaden applications but inherit the same theme/integration gap as Astral.

### Best ideas

- PATH scanning is the right emergency launcher fallback.
- Live CPU/battery/audio/time makes the bar information-dense without becoming a dashboard.
- Zoom-at-cursor and interpolation toggling make image inspection precise.
- A tiny server split is enough to establish clipboard/audio/window ownership early.
- An optional X bridge lets native and compatibility work progress independently.

### Gaps and risks

- No semantic theme, scale, accessibility, localization, rich icons, or keyboard-navigation contract.
- Launcher and shell manually redraw instead of consistently using LibGUI.
- TaskBar is not a task switcher.
- App discovery is filename-based, with no trusted manifest or permission boundary.
- WindowServer creates a broadly accessible socket; authentication/credential ownership is weak.
- No native web stack or daily-use productivity suite.

### Clean-room zlOS lesson

Keep a PATH fallback and live telemetry, but put normal discovery in signed app manifests. Make every shell surface use the shared widget/token system, and authenticate service connections with session-bound handles.

## 4. LemonOS — the best compact native desktop donor

Evidence: `SRC`, `BUILD`, `ASSET`, `SHOT`, `3P`, `R?`.

### Visual system

LemonOS has a real theme object with 13 semantic color roles: active/inactive window and text, background, container background, button, light/dark text, active/inactive/alternate foreground, and border. It also stores window corner radius, border thickness, and titlebar height. The default theme uses dark translucent containers, an orange accent, light text, and ten-pixel corners.

Its assets include eight wallpapers, normal/resize cursors, Montserrat, Nunito, Source Code Pro, window controls, app/menu icons at several sizes, and default/classic theme files. The shipped `classic.json` uses an older incompatible schema and appears stale at this revision. `lemonwm.json` contains a theme path that Main registers but does not consume; the WM loads a different fixed resource-theme path and supports a `SetSystemTheme` protocol. This is a configuration seam to repair, not evidence that theming is absent.

### Desktop and window behavior

LemonWM asynchronously loads wallpaper, falls back to teal, computes clipping/occlusion, supports transparent windows, damage regions, and an optional FPS cap/readout. WindowServer uses shared-memory double buffers and generated interfaces.

The WM draws rounded server-side decorations with close/minimize sprites, hover states, borders, resizing, focus, movement, minimization, and task representation. A concrete UI bug is visible in source: the minimize-hover path tests the close rectangle rather than the minimize rectangle, so the minimize hover image likely responds to the wrong hit area.

The top shell is a rounded translucent 36-pixel bar with:

- Lemon menu button;
- one button per window;
- active-task click to minimize, inactive-task click to activate;
- CPU and memory telemetry.

The menu is a 500x274 app browser. It reads `.cfg` metadata from `/system/lemon/menu`, including name, executable, icon, and category; searches case-insensitively across names/categories; and presents list/grid widgets. Twelve registered entries cover Audio Player, Device Manager, Doom, File Manager, GUI Test, Image Viewer, Lemon Monitor, Minesweeper, Snake, System Info, Terminal, and Text Editor. Other built programs without menu metadata are not launcher-visible, including Video Player, Run, Welcome, and several tests.

### GUI toolkit and shared workflows

LibGUI includes windows, menu and context-menu models, file dialogs, FileView, list/icon views, layout containers, buttons, text boxes, scrollbars, progress bars, and generated protocol stubs. Theme changes can be broadcast through WindowServer. This shared library is the reason the small app suite feels related.

Meaningful first-party experiences:

- **File Manager**: navigable filesystem view using shared FileView/icon machinery and shell file-open handling.
- **Text Editor**: open, save, save-as, line numbers, status, and Source Code Pro presentation.
- **Image Viewer**: file-open flow and image display/view controls.
- **Audio Player / Video Player**: FFmpeg-backed media paths; video is built but absent from launcher metadata.
- **Terminal**: shell workflow integrated with the desktop.
- **Device Manager**: hardware/device inventory rather than terminal-only enumeration.
- **Lemon Monitor**: resource/process inspection.
- **System Info**: compact OS/platform information.
- **Welcome**: launched automatically by Shell; onboarding is treated as a product surface.
- **Minesweeper**: difficulty menu and board interaction.
- **Snake**: native game/workload.
- **Doom**: ported game exposed through native app metadata.
- **Run**: command-launch utility, built but not normal menu surface.
- **Steal**: HTTP/HTTPS command-line downloader with URL parsing and OpenSSL; it is not a browser.
- **GUI tests/demos**: widget and rendering consumers that exercise platform behavior.

### Underneath and persistence

Generated InterfaceCompiler/IDL protocols structure client/server messages. Shared-memory buffers carry pixels. Shell interprets `open` actions through URLs, file extensions, and handlers. Theme, wallpaper, menu entries, and WM behavior come from resource/config files. There is no broad settings daemon or session-restoration graph; many preferences remain app/local-config concerns.

### Best ideas

- A 13-role theme plus three metrics is enough to create a distinctive shell.
- One small metadata directory drives a searchable icon launcher.
- Taskbar CPU/memory makes platform behavior visible.
- Theme updates are protocol events, not just startup configuration.
- Welcome, Device Manager, Monitor, and System Info make bring-up user-facing.

### Gaps

- No native browser/web engine.
- No accessibility tree, assistive settings, high-contrast proof, localization pipeline, or multiple keyboard-layout story in the inspected desktop.
- Theme vocabulary is narrow and version migration is already drifting.
- Several built apps have no launcher metadata.
- IPC relies on direct shared-memory/data structures without the isolation depth of Serenity.
- No isolated decoder/parser workers for hostile media.

### Clean-room zlOS lesson

Use LemonOS as the minimum coherent desktop contract: semantic theme, metadata launcher, shared FileView/dialogs, task buttons, device/monitor/welcome utilities, and generated protocols. Then harden every seam with handles, bounds, restart, accessibility, and schema migration.

## 5. SerenityOS — the only full native product platform in the set

Evidence: `SRC`, `BUILD`, `ASSET`, `SHOT`, `DOC`, `TEST`, `R?`.

### Visual system, not merely a retro skin

The inspected screenshot shows the familiar older Serenity desktop: textured wallpaper, gray beveled windows, a top menu/status region, taskbar, Terminal, Browser, and a notification. Treating that as the whole design system would miss most of the source.

There are 23 theme INI files at this revision: Basalt, Chillychilly, Coffee, Contrast, Cupertino, Dark, Default, Desert, Durrque, Faux Pas, Gruvbox Dark, Light, Nord, Olive, Plum, Pumpkin, Redmond variants, Scarlett, Silver, and Sunshine among them. Themes cover:

- active, inactive, moving, highlighted, and modal window colors;
- menu, base, button, text, link, selection, tooltip, tray, ruler, overlay, and syntax roles;
- alignments and boolean flags;
- border radius/thickness, titlebar height, title-button dimensions, and spacing metrics;
- asset paths for title icons, shadows, color schemes, and other chrome resources.

Editor color schemes are separate from desktop themes. Cursor themes include Default and Dark, 1x and 2x assets, resize variants, and animated wait. The icon collection has broad action, application, device, filetype, status, and settings coverage at common 16/32 sizes. Wallpapers and a substantial font collection complete the visual vocabulary.

LibGUI provides a large reusable component set: buttons, tabs, splitters, menus, toolbars, models/views, tables, trees, icon views, text editors, dialogs, color/font/file pickers, status bars, sliders, progress controls, graphs, property grids, and more. GML provides declarative layout resources. ThemeEditor and settings apps make the system inspectable and editable through its own UI.

### Desktop and navigation behavior

WindowServer owns composition, window frames, z-order, focus, movement, resizing, minimize/maximize, menu surfaces, cursors, workspaces, snapping/tiling-related geometry, flashing/attention, and client communication. Taskbar provides:

- per-window buttons filtered by current workspace;
- quick launch;
- system menu;
- system tray/status items;
- show desktop;
- task context behavior and access to system monitor/settings.

Workspaces form a configurable row/column grid. This is the only audited system with a clearly developed workspace model. Application metadata and LaunchServer support launching and associations. ConfigServer supplies typed-ish per-domain settings notifications so components do not all parse unrelated files independently.

### The active graphical suite

The active `Userland/Applications` build graph contains exactly 51 graphical applications. Their user-facing roles are:

#### Shell, orientation, and daily utility

- **About**: OS/version and credits identity.
- **Welcome**: first-run orientation and discovery.
- **Assistant**: searchable/helpful desktop assistant surface.
- **Run**: direct command/application launch.
- **Calculator**: normal desktop calculations.
- **Calendar** and **CalendarSettings**: date browsing plus preferences.
- **AnalogClock**, **ClockSettings**: clock display and clock configuration.
- **CharacterMap**: glyph discovery/copy workflow.
- **Screenshot**: region/window/screen capture.
- **Magnifier**: visual enlargement accessibility utility.

#### Files, documents, and creation

- **FileManager**: filesystem navigation, file operations, associations, properties, thumbnails/icons, and desktop integration.
- **TextEditor**: text editing through shared GUI/text libraries.
- **HexEditor**: binary inspection/editing.
- **ImageViewer**: multi-format viewing and image actions.
- **PDFViewer**: PDF document navigation/rendering.
- **PixelPaint**: raster creation/editing.
- **Presenter**: slide/presentation authoring and playback.
- **Spreadsheet**: grid/formula/document workflow.
- **FontEditor**: native font authoring.
- **ThemeEditor**: theme inspection and editing.
- **3DFileViewer**: model/file visualization.
- **SpaceAnalyzer**: visual storage-usage exploration.

#### Web, communication, and information

- **Browser** and **BrowserSettings**: full native web shell and preferences.
- **Mail** and **MailSettings**: email workflow and account/settings surface.
- **Maps** and **MapsSettings**: map browsing and provider/settings surface.
- **Weather**: weather lookup/presentation.
- **Help**: local documentation browser.
- **CertificateSettings**: trust/certificate administration.

#### Media

- **SoundPlayer**: audio playback.
- **VideoPlayer**: video playback.
- **Piano**: interactive audio/MIDI-like creative surface.

#### Development and diagnosis

- **Terminal** and **TerminalSettings**: shell plus profile/appearance behavior.
- **Debugger**: source/process debugging.
- **CrashReporter**: crash explanation and trace workflow.
- **SystemMonitor**: process, CPU, memory, and system inspection.
- **Profiler** is not in the 51 active Applications list but exists elsewhere as an optional developer tool; likewise HackStudio and SQLStudio must not be silently folded into the count.

#### System and account settings

- **Settings**: settings hub.
- **DisplaySettings**, **KeyboardSettings**, **MouseSettings**, **NetworkSettings**, **UsersSettings**, **GamesSettings**: focused configuration surfaces.
- **KeyboardMapper**: interactive keymap authoring/inspection.
- **PartitionEditor**: disk/partition administration.
- **Escalator**: privilege-elevation prompt/workflow.
- **MailSettings**, **MapsSettings**, **BrowserSettings**, **TerminalSettings**, **CalendarSettings**, **ClockSettings**, and **CertificateSettings** keep complex preferences out of main apps.

The 13 active games are BrickGame, Chess, ColorLines, FlappyBug, Flood, GameOfLife, Hearts, MasterWord, Minesweeper, Snake, Solitaire, Spider, and 2048. Together they exercise timing, input, menus, dialogs, board/grid layout, drag/drop, persistence, AI/opponents, animation, audio, and graphics. They are not just decoration; they are platform consumers.

Application metadata files enumerate additional demos/tools beyond the active 51, so “metadata exists” is not interchangeable with “active Applications build target.”

### Browser chrome and workflows

The native Browser source exposes:

- multiple tabs and optional vertical tabs;
- address/search box and configurable search engines;
- home/new-tab behavior;
- bookmarks bar with `bookmarks.json` persistence;
- history window;
- download views and progress;
- find in page;
- zoom in/out/reset;
- fullscreen;
- view source;
- DOM inspector and inspect-element entry points;
- browser task manager;
- storage inspector for cookies, local storage, and session storage, with deletion/clear operations;
- proxy mappings;
- preferred color scheme;
- watched content-filter list;
- watched autoplay allowlist;
- a separate BrowserSettings app;
- process reuse via ChromeProcess when opening links in an existing browser instance.

This is a product shell, not merely a webview demo.

### Web engine surface

LibWeb and its generated WebIDL/CSS inputs cover a broad engine surface: HTML parsing/tree building, DOM, CSS cascade/style/layout/paint, SVG, MathML, accessibility/ARIA structures, forms, selection, animation, events, Fetch/XHR, URL, cookies and storage, IndexedDB, workers/service workers, WebSockets, canvas, WebGL, WebAudio, media, clipboard, editing, navigation, and many browser APIs. LibJS supplies JavaScript, and WebAssembly modules exist.

Source presence does not equal standards conformance. The valuable engineering strategy is the decomposition and specification-oriented organization: generated bindings, explicit algorithms, focused web tests, and browser-facing debug tools.

### Process and service architecture

The active browser target depends on BrowserSettings, ImageDecoder, RequestServer, and WebContent. The broader system includes:

- **WebContent** for page/engine execution and painting into shared surfaces;
- **RequestServer** for network requests;
- **LookupServer** for name resolution;
- **ImageDecoder** as an isolated decode helper;
- **WebWorker** helpers;
- file-system access, audio, database, and other portals/services;
- pledge/unveil-style syscall/path restriction and process jailing.

Browser refuses to run as root and applies restrictions. Helper processes apply their own restrictions. However, the repository's process-architecture document explicitly says parts are aspirational, and the exact current allocation/reuse policy must be verified in execution. Phrase the evidence as “separate constrained helper processes exist,” not “perfect site isolation is proven.” Browser also has a FIXME around a standalone download manager, and its main process retains broad user-home access in places.

### Accessibility and localization

Concrete accessibility features include Magnifier and system color filters for protanopia/protanomaly, deuteranopia/deuteranomaly, tritanopia/tritanomaly, achromatopsia, and achromatomaly. Contrast theme and keyboard-capable GUI widgets add useful coverage. LibWeb contains accessibility semantics.

No complete desktop screen-reader workflow was established in this pass. Desktop strings remain broadly English even though LibLocale/LibUnicode and locale-heavy web functionality exist. Infrastructure should not be inflated into a finished localized desktop.

### Persistence and service ownership

ConfigServer domains, application settings, history/bookmark files, LaunchServer associations, clipboard/audio/notification services, SQL/database paths, and file-operation services create the strongest shared-platform model in the set. Apps consume common libraries and services rather than each reinventing parsers and system access.

### Best ideas

- Treat every first-party app as a conformance consumer of common libraries.
- Keep app settings separate when the main workflow would become cluttered.
- Make developer tools, crash reporting, storage inspection, and task management normal desktop experiences.
- Generate bindings/layout/protocol artifacts from declarative sources.
- Isolate network, content, decoding, and portal access before the browser is “finished.”
- Ship theme editing, accessibility filters, and high-contrast choices as platform features.

### Gaps and weakest claims

- Browser standards compatibility is incomplete; module breadth is not conformance.
- Process-architecture prose partly describes a destination rather than every current behavior.
- Full screen-reader and broad desktop localization workflows were not found.
- Some app metadata refers to optional/different build locations; counts must remain scoped.
- A broad suite increases maintenance and parser attack surface; isolation still needs runtime fault proof.

### Clean-room zlOS lesson

Take the shared-library/app-as-test strategy, service separation, declarative layout, semantic theme breadth, settings domains, browser tools, and creation suite. Re-specify all protocols and data models in zl, and make actual process isolation and restart receipts stronger than the documentation.

## 6. MollenOS — service and package donor; GUI is external

Evidence: `SRC`, `BUILD`, `DOC`, `SHOT`, `3P`, `R?`.

### Visual evidence ceiling

The inspected launcher screenshot is visually polished: full-screen wallpaper, centered translucent search/launcher panel, large app icons, user identity, date, and version. The documentation states that the complete experience requires separate `vali-userspace`, and Vioarr supplies the window manager, terminal, and launcher. Those visual features are not implemented in the current repository.

There is therefore no honest local audit of Vioarr theme tokens, fonts, icons, window behavior, browser, accessibility, localization, or app workflows from this checkout. The screenshot is `SHOT/DOC/3P`, not `SRC`.

### What the repository really contributes

The active tree builds ten modules, several service processes, and only two native user-facing utility apps:

- **osstat**: OS/system status reporting;
- **systat**: system/process status reporting.

Generated Gracht IPC connects services including deviced, filed, processd, served, sessiond, usbd, and netd; syslogd is disabled in the inspected graph. This creates separations for devices, files, processes, applications, sessions, USB, and network policy.

`served` consumes bundled application packs from `/data/setup`, installs/registers them, and removes setup input afterward. VaFS/app images provide read-only packaged bytes, and external application directories can be folded into the image. This is a useful installation and immutable-package mechanism, although read-only packaging is not process isolation.

### Browser, applications, and settings

No native browser or web engine exists in the current tree. No local graphical Files, editor, media player, settings center, game suite, notification UI, or accessibility surface is active here. Those may exist in external userspace; they were not imported into this evidence set.

### Best ideas

- Make app installation/registration a service with a defined pack format.
- Generate IPC stubs instead of hand-copying packet layouts.
- Separate session, process, device, file, network, and USB ownership early.
- Build immutable application images as a reproducible delivery unit.

### Gaps

- The attractive GUI cannot be audited or transferred from this repository.
- External userspace creates version/evidence drift.
- Only two native CLI utilities are current user-facing app evidence.
- Read-only packs need capabilities, per-process address spaces, quotas, signatures, updates, rollback, and revocation around them.

### Clean-room zlOS lesson

Take the service/package seams, not the screenshot. Keep zlOS's visual design self-owned and keep source, package, boot, and runtime evidence in the same repository receipt graph.

## 7. hhuOS — graphics, audio, game, and emulator proof laboratory

Evidence: `SRC`, `BUILD`, `ASSET`, `SHOT`, `3P`, `R?`.

### Visual identity and Lunar

The inspected widget screenshot is a minimal white/gray surface with cyan/blue controls and a bitmap font. Lunar has a real theme abstraction: every widget receives normal, hovered, and pressed `Style` values for widget background, border, accent, text, and text background. `HhuTheme` uses HHU corporate blues, ice blue, dark blue, black, and white. The current theme can be replaced at runtime.

Lunar includes:

- Button;
- Label with wrapping/truncation behavior;
- CheckBox;
- RadioButton and RadioButtonGroup;
- InputField with caret;
- Container;
- Border, Free, Grid, Horizontal, and Vertical layouts;
- mouse enter/exit/press/release/click and keyboard press/release/type listeners;
- focused/hovered/pressed state and dirty redraw.

The default typeface is Terminal 8x8. The theme styles only six widget families and has no external theme format, iconography, density scale, high-DPI model, motion system, or desktop chrome.

### No desktop shell

hhuOS is not a windowed desktop at this revision. There is no launcher, panel, taskbar, multiwindow compositor, notification service, graphical Files, settings center, graphical text editor, or browser. UI and games run as direct framebuffer/full-screen workloads or terminal applications.

This distinction makes it useful: hhuOS isolates “can the graphics/input/audio stack carry the workload?” from “does the desktop look polished?”

### First-party visual and media workloads

The active build graph has 45 applications. User-visible highlights include:

- **demo** modes for ant simulation, colors, fonts, keyboard, OpenGL, particles, polygons, sprites, and Lunar widgets;
- **view3d** for native 3D viewing;
- **battlespace**, a 3D space shooter with radar, scaling/performance control, movement, and missiles;
- **bug**, a Space-Invaders-like game;
- **dino** and **rogue3d**;
- PortableGL/TinyGL information, gears, triangle, and cube demos;
- **beep** and WAV **play** paths for SoundBlaster/audio exercise;
- **asciimate** for terminal ASCII animation;
- shells, core utilities, networking diagnostics, and system probes.

Pulsar provides game/workload structure: 2D components, collision, events, particles, 3D, and audio. This is a better donor for a zlOS workload SDK than for a desktop widget kit.

### Third-party experience boundary

ClassiCube, ClownMD, DoomGeneric, LiteNES, Peanut-GB, and QuakeGeneric are ports/submodules. Peanut-GB is especially useful as an integration workload because it combines Game Boy emulation, audio, save files, and palette switching. The gameplay belongs upstream; hhuOS owns adaptation to its platform.

Some downloadable media is intentionally omitted from redistributable builds. Asset provenance and default-image inclusion therefore need the same evidence discipline as executables.

### Accessibility, localization, web, persistence

No desktop accessibility or localization system was found. No browser/web stack exists. Persistence is per-application—such as emulator saves—rather than a shared document/settings/session platform.

### Best ideas

- Maintain a staged workload ladder: pixels → widgets → 2D → 3D → audio → emulator/game.
- Expose performance/scale controls inside demanding demos.
- Use games and emulators as subsystem integration tests with real human feedback.
- Keep a corporate-color theme table separate from widget behavior, even in a small library.

### Gaps

- Lunar lacks lists, trees, menus, dialogs, scroll views, file pickers, rich text, models, accessibility semantics, and desktop composition.
- Direct framebuffer authority does not scale to mutually isolated applications.
- Fixed bitmap fonts and English strings constrain internationalization and accessibility.
- Third-party workload success cannot substitute for first-party daily-use workflows.

### Clean-room zlOS lesson

Take the workload ladder and game/graphics framework ideas. Run the same workloads through the real zlOS compositor, input, audio, storage, and process boundaries so they become evidence for the platform rather than privileged demos.

## 8. duckOS — small retro desktop with real app and theme contracts

Evidence: `SRC`, `BUILD`, `ASSET`, `SHOT`, `3P`, `R?`.

### Visual system

The inspected screenshot shows a coherent dark desktop with a bottom taskbar, app menu, System Monitor, Doom, a 3D demo, and About. The source backs more of that coherence than the screenshot suggests.

LibUI parses `.thm` files with four value kinds:

- semantic colors;
- integer metrics;
- strings, including normal and monospace fonts;
- images, optionally tinted by foreground/background/accent.

The standard API exposes background, foreground, accent, window, focused/unfocused window title, derived shadow/highlight, button/text, and scrollbar roles; button padding and progress-bar height; font and mono font; and themed images. Five shipped themes are active assets:

- **default**: dark charcoal, white text, teal-blue accent;
- **light**: classic light gray/black;
- **hotdog**: red/yellow novelty high-intensity palette;
- **vapor**: light gray with cyan accent;
- **vapor_dark**: deep purple with pink and blue.

All select Gohu 11 for normal and mono text. Gohu 14 is also shipped and used by Calculator. The wallpaper set contains one duck image. Cursors include normal and four resize directions. Icons cover duck/system identity, missing icon, folder, default/text/audio file types, and a few app resources. This is a working token parser with a tiny vocabulary, not a full visual asset system.

Window decoration is classic beveled/gradient retro UI: 22-pixel titlebar, three-pixel border, title icon, centered title, raised close button, focus accent, and server-computed outer shadow. Theme metrics do not control titlebar/border size, so geometry still requires code changes. There is no live theme broadcast; configuration selects a theme during LibUI initialization.

### Pond compositor and window behavior

Pond is a separate service connected through LibRiver typed messaging. It owns root and child windows, z-order, focus, movement, resizing with directional cursors, clipping, dirty rectangles, shared double framebuffers, alpha, alpha hit testing, parent/menu relationships, minimum sizes, global-mouse cases, and optional shadows.

The shadow implementation samples window geometry into four edge buffers—the source calls it a “poor man's box-shadow.” It is visually effective but recomputation and large-window costs need measurement. Move and resize are distinct from focus; clicking normal windows focuses, draggable windows move to front, and edge proximity switches cursors/resizing.

The IPC surface includes open, destroy, move, resize, invalidate, font fetch, title, reparent, hints, bring-to-front, display info, focus, and minimum size, plus mouse/keyboard/window callbacks. It is much cleaner than direct global framebuffer access, though companion audits identify weak shared-memory ownership as a security regression shape.

### Sandbar and app menu

Sandbar is a borderless immovable bottom window. It provides:

- duck icon and **Apps** button;
- memory module;
- CPU module;
- wall-clock module;
- one-second updates.

AppMenu enumerates `.app` bundles, skips `hidden=true`, and renders each icon/name as a button. Choosing an app executes it and closes the menu. It does not search, categorize, favorite, expose recent documents, or show running windows. Sandbar also has no task/window button list, workspace model, tray, notification center, or show desktop.

### One metadata contract across the desktop

Every application bundle carries `app.conf` with name, executable, hidden state, supported extensions, and optional extension-specific icons. `libapp` finds bundles under one base path, loads icons, launches programs, resolves a handler for a file extension, and implements `open`. This same model drives:

- AppMenu visibility and labels;
- Files double-click;
- Editor registration for txt/conf/service/C/C++/theme files;
- Viewer registration for PNG/icon/WAV;
- 3demo registration for OBJ;
- process monitor app names/icons;
- filetype icon selection;
- command-line `open`.

This is the best compressed app-integration idea in the small systems.

### First-party applications and workflows

- **Files**: shared FileGridView rooted at `/`, shared navigation bar, app-derived icons, directory navigation, and double-click through `App::open`. The selection callback is empty. There is no rename/copy/move/delete/trash/search/properties/mount UI in its main workflow.
- **Editor**: open, save, save-as, FilePicker, `Ctrl+O`, `Ctrl+S`, `Ctrl+Shift+S`, word/character line-break modes, a one-megabyte read bound, extension registration, and title/icon updates. Write/open errors largely disappear from the UI, so data-loss/error communication needs work.
- **Viewer — image**: PNG/icon loading, centering, drag pan, wheel scaling clamped from 0.01x to 100x.
- **Viewer — audio**: WAV loading through LibSound, play/pause, stop, ±10-second seek, time label, and 60-Hz progress update through the Quack sound service.
- **Terminal**: LibTerm-backed shell window.
- **System Monitor**: CPU progress, kernel/cache/user memory breakdown, memory visualization, process table with icon/PID/name/virtual/physical/shared/state, and context actions Kill, Stop, Continue, Inspect. Inspector opens process details/memory maps in another window.
- **Calculator**: four-column arithmetic with digits, decimal, sign, delete, CE/C, four operators, equals, and divide-by-zero display.
- **Ducksweeper**: native mines board, reveal/flood, marking, elapsed state, win/loss flow.
- **Four in a Row**: two-player or CPU play, move hints, hover preview, status, win and reset.
- **3demo**: OBJ viewing/rendering through Lib3D and app/resource integration; also exercises registered app enumeration.
- **About**: uname-derived identity/version surface.
- **Uxn**: native Uxn virtual machine with 64-KiB memory, work/return stacks, core opcodes, console device, and ROM loading. It is an extensible tiny-computing surface more than a polished end-user app.

The ordinary shell `dsh`, 22 core utilities, init, DHCP client, Pond, and Quack complete the daily system base. Doom and SDL-based experiences are third-party ports.

### Accessibility, localization, persistence, and web

No accessibility tree, screen reader, color-filter UI, scalable type setting, alternate input settings, localization catalogue, or translated desktop workflow was found. Keyboard events and focused widgets exist, but that alone is not accessibility.

Theme/config files and user documents persist through the filesystem. App bundles are static registry entries. There is no settings daemon, session/window restoration, recent-document database, trash, or notification history. No browser or web engine exists.

### Best ideas

- One app bundle contract drives launcher, file associations, icons, process identity, and `open`.
- A small file-backed theme parser can support real personality variants.
- Process Monitor combines system state with app metadata and direct actions.
- Viewer unifies image and audio under file association without pretending they are the same widget.
- Typed window IPC and shared reusable FileView/FilePicker/Menu/TextView reduce per-app duplication.

### Concrete gaps and dead UI

- Files' selection callback is empty and file operations are absent.
- Sandbar cannot switch/minimize running tasks.
- AppMenu has no search and can overflow as the suite grows.
- Theme parser has a dormant/unused default-resource path variable and geometry remains hardcoded.
- Several theme files omit focused/unfocused title roles, which can produce default-zero behavior depending on lookup; schema validation/fallback is weak.
- Editor and app launch paths often fail without a user-facing error dialog.
- Uxn division has an explicit divide-by-zero TODO.
- No native web, accessibility, localization, or robust document persistence UX.

### Clean-room zlOS lesson

Take the app-bundle unification, typed Pond shape, process-aware monitor, and theme format. Add schema versioning/default inheritance, authenticated handles, search/categories, task buttons, error dialogs, settings/persistence, and a much larger accessible icon/type system.

## Complete cross-system app and service map

The table compresses the meaningful first-party user experience without hiding the full target inventories in the companion report.

| Product domain | Brook | Astral | banan-os | LemonOS | SerenityOS | MollenOS | hhuOS | duckOS |
|---|---|---|---|---|---|---|---|---|
| Files/documents | Files, Edit | XFile/xNEdit `3P` | CLI only | File Manager, Text Editor, Image Viewer | FileManager, TextEditor, HexEditor, ImageViewer, PDFViewer, 3DFileViewer, SpaceAnalyzer | filed service; external GUI | terminal/core tools | Files, Editor, Viewer, 3demo |
| Creation | Basic text | Upstream editors `3P` | None | Basic text | PixelPaint, Presenter, Spreadsheet, FontEditor, ThemeEditor, Piano | External | Workload/game creation libraries | Editor, Uxn runtime |
| Web/network UX | NetSurf/Ladybird `3P`, package tools | WebKitGTK/Links `3P` | Links/Lynx `3P`, native diagnostics | Steal downloader | Browser, BrowserSettings, Maps, Weather, Mail plus settings | netd; no local GUI | network diagnostics | DHCP/core networking; no browser |
| Media | FFmpeg Player | GStreamer/media apps `3P` | audio tools, ImageViewer | Audio Player, Video Player, ImgView | SoundPlayer, VideoPlayer, ImageViewer, PDFViewer | External | WAV/beep, graphics/audio demos, emulators | Viewer image/WAV, Quack, play |
| Settings/admin | Package/config/console/diagnostics | Native admin core + upstream tools | TaskBar/audioctl | Device Manager, Monitor, System Info, Welcome | Settings hub + display/input/network/users/cert/clock/calendar/games/mail/maps/browser/terminal settings, PartitionEditor, SystemMonitor | osstat, systat, services | system/probe apps | About, Monitor, Sandbar |
| Games | Quake-family and others mostly `3P` | Doom/Quake/etc `3P` | Snake + ports | Minesweeper, Snake, Doom `3P` | 13 native games | External | native 2D/3D games + six major ports | Ducksweeper, Four in a Row, 3demo, Doom `3P` |
| Platform services | Kernel WM, waylandd, package index | X stack `3P` | Window/Audio/Clipboard servers | WM, WindowServer, Shell, IDL | 29 services including window, launch, config, audio, notify, file ops, web/network/decode/database | device/file/process/app/session/USB/network | mostly direct OS APIs | Pond, Quack, init, DHCP |

## Accessibility and localization: the broad shared gap

| System | Concrete positive evidence | What is not established |
|---|---|---|
| Brook | Keyboard-capable native tools; readable severity colors | Semantic accessibility, screen reader, contrast themes, scalable text, localization |
| Astral | at-spi2 recipe; Qt accessibility enabled; GTK/ICU/gettext packages | Integrated assistive workflow and default-image/runtime proof |
| banan-os | Keyboard launcher and terminal clipboard | Accessibility tree, focus order, settings, alternate keyboard/locale |
| LemonOS | Shared widgets and theme roles | Assistive semantics, contrast proof, translation/input settings |
| SerenityOS | Magnifier, high-contrast theme, eight color-vision filters, keyboard-rich GUI, web ARIA | Full desktop screen reader and comprehensive localized desktop |
| MollenOS | External UI unknown | Any local GUI evidence |
| hhuOS | Widget focus and keyboard listeners | Desktop accessibility/localization entirely |
| duckOS | Focused widgets and multiple palettes | Semantic accessibility, contrast validation, translation, scalable type |

zlOS should treat this as a chance to lead rather than a final polish task. Every widget needs role/name/value/state/actions, focus geometry, keyboard activation, minimum target sizes, logical-order navigation, contrast checks, font scaling, locale-aware shaping/layout, bidirectionality, and reduced-motion behavior before the app suite expands.

## Destination design system for zlOS

### Token schema

The visual contract should be data-driven, versioned, inherited, and validated. Minimum semantic groups:

| Group | Required tokens |
|---|---|
| Color | canvas, surface levels, window active/inactive, chrome, text primary/secondary/disabled/inverse, accent, selection, link, focus, danger, warning, success, info, border, divider, shadow, tooltip, menu, tray, overlay |
| Typography | display/title/body/label/code families; size, line height, weight, tracking; fallback stacks; minimum legible size |
| Space | 1/2/3/4/6/8/12/16/24/32 logical-pixel scale, not ad hoc literals |
| Geometry | corner radii, border widths, titlebar/panel/control heights, touch targets, icon sizes |
| Elevation | window/menu/dialog/tooltip shadows and dimming |
| Motion | fast/normal/slow durations, easing, window/menu/notification transitions, reduced-motion zero/short variants |
| State | hover, pressed, checked, selected, focused, disabled, error, busy, attention |
| Assets | cursor family and scale variants, window buttons, action/status/device/file/app icons, wallpaper behavior |

Applications request semantic roles. They never own copies of global colors or window metrics. A theme package declares schema version and parent theme; missing optional roles inherit; missing required roles reject atomically and fall back to last-known-good. Contrast and target-size checks run at install time.

### Theme service contract

```text
settings/theme UI
   -> validate candidate package
   -> preview in isolated sample scene
   -> commit generation N atomically
theme service
   -> broadcast generation + changed token groups
toolkits/shell/apps
   -> resolve tokens by generation
   -> invalidate affected layout/paint only
```

Theme changes must survive service restart and never partially update the desktop. The protocol should expose preview, commit, rollback, current generation, system light/dark/high-contrast preference, scale, font scale, reduced motion, and per-session override.

### Typography and icons

Brook and hhuOS show why an embedded ASCII font remains useful for recovery. It must not become the desktop ceiling. zlOS needs:

- a tiny kernel/recovery bitmap font;
- a user-space shaper/rasterizer with Unicode fallback, combining marks, RTL, emoji policy, and font scale;
- deterministic font discovery/cache invalidation;
- 16/20/24/32/48/64 logical icon sizes with vector or multi-density raster sources;
- semantic icon names decoupled from filenames;
- high-contrast and monochrome-compatible icon forms;
- strict license/provenance records.

### Layout and responsive behavior

Borrow Serenity's declarative layouts, duckOS's compact Flex/Box/Grid primitives, and hhuOS's clear region layouts. Add logical pixels, constraints, intrinsic sizes, wrapping, baseline alignment, scroll containment, right-to-left mirroring, safe areas, and minimum/maximum bounds. Every first-party app must pass narrow, default, wide, 1x, 1.5x, and 2x screenshot/input checks.

## Destination app/platform architecture

```text
Session manager
  |
  +-- shell: desktop, taskbar, launcher, workspaces, notifications, quick settings
  +-- app registry: manifests, search index, MIME/actions, permissions, updates
  +-- settings service: versioned typed domains and subscriptions
  +-- window/compositor service: surfaces, input, focus, geometry, frames, capture
  +-- clipboard service: MIME offers, size/expiry policy, session ownership
  +-- file-operation service: copy/move/trash/undo/progress/conflict UI
  +-- document portal: open/save/recent with scoped handles
  +-- audio service: bounded rings, per-app volume, device failover
  +-- notification service: actions, grouping, quiet mode, durable history
  +-- network/request service: DNS/TLS/proxy/cache/downloads/quotas
  +-- decoder workers: image/font/media/PDF in disposable sandboxes
  +-- crash/recovery service: reports, app restart, unsaved-document recovery
  +-- accessibility service: tree, actions, focus, magnifier, filters, reader bridge
```

Each connection carries session identity and capabilities. IDs are opaque handles with generations. Every request has a maximum encoded size, deadline/cancellation, ownership check, and peer-death cleanup. Services publish readiness and health; supervision restarts them without silently preserving stale handles.

### Unified app manifest

duckOS supplies the compressed shape; Brook supplies freedesktop import; MollenOS supplies package/service lifecycle. A zlOS manifest should include:

- stable app ID, display name, description, categories, keywords, icon, executable;
- localized strings/resources;
- MIME types, file extensions, URL schemes, and named actions;
- requested capabilities and portals;
- singleton/multiwindow/background policy;
- minimum OS/API versions and dependencies;
- document/cache/config/state locations and quotas;
- update/signature/provenance information;
- declared crash-recovery and session-restore support;
- smoke-test command and expected visible receipt.

The launcher, `open`, Files, search, process monitor, taskbar, permissions UI, package manager, notification settings, crash reporter, and recent-documents service all consume the same registry. Imported `.desktop` data enters as explicitly untrusted compatibility metadata and is normalized, never executed as an arbitrary shell string.

## App-by-app zlOS feature plan

### Shell and launcher

Combine Serenity's workspaces/quick launch/show desktop, Lemon's task-button semantics, Brook's emergency terminal/provider telemetry, banan's keyboard-first fallback, and duckOS's single registry.

Required workflows:

- type-to-search apps, settings, files, actions, and help;
- category/app grid for discoverability;
- favorites, recent documents, and recent apps;
- taskbar buttons with grouped windows, progress/attention, minimize/restore, previews, and context actions;
- workspace switcher, keyboard shortcuts, move-window-to-workspace;
- notification/quick-settings area for network, sound, battery, devices, time, accessibility, and session actions;
- emergency “Terminal,” “Files,” “Monitor,” and “Safe mode” actions even if the registry fails;
- optional FPS/provider badges in developer mode, with provider status always available in Monitor.

### Files

Start from Lemon/Serenity shared models and duck/Brook navigation, then require:

- icon/list/columns views, sort/group, breadcrumbs/address, back/forward/up;
- search, preview, thumbnails through isolated decoders;
- copy/move/rename/link/trash/restore/delete with progress, conflict, cancel, retry, and undo;
- mount/device sidebar and eject/safely-remove;
- properties, permissions, ownership, hashes, file type and handler choice;
- scoped Open With and default-app management;
- crash-safe operation journal and resumed/rolled-back transfers.

### Editor

Brook/duck/Lemon establish the rescue baseline. The native product requires open/save/save-as, atomic save, encoding/newline choice, line numbers, search/replace, undo/redo, multi-document tabs, syntax highlighting, diagnostics, large-file mode, autosave/recovery, external-change detection, keyboard navigation, and accessible text semantics. “Language support missing” must remain distinct from “the platform cannot render/edit that text.”

### Viewer and media

Keep banan/duck cursor-centered zoom, pan, interpolation selection, and duck's unified file-open path. Add rotation, fit/actual size, metadata, next/previous directory navigation, animation, color management, and print/share actions. Audio/video need queues/playlists, scrub, volume, subtitles, tracks, fullscreen, recent position, device switch, and decoder crash containment.

### Monitor, Device Manager, logs, and crash reporting

Combine duck's process actions/memory maps, Lemon's Device Manager and Monitor, Brook's log filtering, and Serenity's SystemMonitor/CrashReporter.

The result must show:

- processes, services, providers, threads, CPU/memory/I/O, handles, IPC queues, windows, and permissions;
- hardware topology, current driver/provider, firmware, queues, errors, retries, reset/rebind, and hotplug history;
- per-service restart and failure receipts;
- structured logs with severity/component/time/session filters and export;
- crashes with symbolized trace, recent events, affected unsaved documents, restart/safe-mode action, and privacy review before sharing.

### Settings

Use a searchable hub with focused panels, like Serenity, but backed by one versioned domain service. Required panels: display/theme/type/scale; keyboard/layout/shortcuts; mouse/touch; sound; network/proxy/DNS; users/session/security; storage; applications/defaults/permissions; notifications; accessibility; date/time/locale; updates/packages; developer/system evidence.

### Welcome and Help

Take Lemon's automatic Welcome and Serenity's Welcome/Help. First boot should teach launcher, task switching, Files, updates, accessibility, recovery, and privacy. Help pages link into settings and expose actual system/version/provider state; stale docs fail a link/action test.

### Creation suite

Serenity proves small systems can own PixelPaint, Presenter, Spreadsheet, FontEditor, ThemeEditor, and Piano. zlOS should retain the full destination but stage it after document, undo, clipboard, dialogs, printing/export, fonts, and parser isolation are shared. Each creation app must produce a portable format and an atomic native document format with recovery.

### Games and workload lab

Use Serenity's native games for widget/platform coverage, hhuOS's workload ladder for graphics/audio/input evidence, and duck's small native games for approachable source. First-party games should exercise save data, controller remapping, scaling, audio focus, fullscreen, pause-on-focus-loss, accessibility, and deterministic replay. Ports remain separately attributed and sandboxed.

### Browser

Do not start with a single in-process HTML widget. Deliver in dependency order:

1. URL, Unicode, MIME, HTTP, TLS, certificate, cookie, cache, and download services with hostile-input tests;
2. browser shell with tabs/history/bookmarks/settings and a placeholder content process;
3. isolated HTML/CSS/DOM/layout/paint subset;
4. JavaScript in a separately bounded content process;
5. forms, accessibility tree, storage, workers, media, canvas, and developer tools;
6. compatibility expansion against versioned web-platform tests;
7. per-site isolation policy, crash/reload, process/task manager, storage/permission inspector.

Brook/Astral ports can provide an interim browser, but must use the same portals and app manifest so they validate the platform rather than bypass it.

## Cool-feature shortlist, ranked by zlOS value

| Priority | Feature | Donor | Why it matters |
|---:|---|---|---|
| 1 | One manifest drives launcher/open/icons/process identity | duckOS + Brook | Prevents registry drift and makes ports/native apps equally discoverable |
| 2 | Device/provider state visible in shell/Monitor | Brook + Lemon | Turns hardware claims into inspectable product truth |
| 3 | Separate web content/request/decode processes | SerenityOS | Browser faults become contained service failures |
| 4 | Searchable launcher plus PATH fallback | Lemon + banan-os | Rich discovery survives registry corruption |
| 5 | User-facing filtered system log | Brook | Makes debugging possible without serial console |
| 6 | Pan/zoom around cursor and interpolation switch | banan-os + duckOS | Small interaction that makes Viewer feel precise |
| 7 | Color-vision filters, Magnifier, Contrast theme | SerenityOS | Accessibility becomes a platform feature |
| 8 | Theme preview/edit with semantic roles | Serenity + Lemon + duck | Visual change is safe, coherent, and user-owned |
| 9 | Workload ladder through widgets, 3D, audio, emulators | hhuOS | Prevents a pretty shell from hiding broken subsystems |
| 10 | Immutable app packs behind an install service | MollenOS | Reproducible app delivery and rollback foundation |
| 11 | Active task click minimizes; inactive activates | LemonOS | Simple predictable task switching |
| 12 | Process Monitor knows app icons/names and memory maps | duckOS | Joins user concepts to systems reality |
| 13 | Browser storage/task/DOM inspectors | SerenityOS | Makes web behavior and privacy visible |
| 14 | Direct one-click emergency Terminal | Brook | Keeps the desktop recoverable |
| 15 | Uxn/tiny VM as a constrained creative runtime | duckOS | Interesting safe extension/workload surface if capability-bounded |

## Known negative patterns to turn into zlOS regression tests

- **Screenshot substitution:** Mollen visual claims must fail unless the implementing source/package/runtime receipt is in scope.
- **Port substitution:** Astral/Brook browser or game package must never be labeled native.
- **Metadata substitution:** Serenity optional `.af` entries and Lemon built-but-unregistered apps show that a manifest, build target, image inclusion, and launch receipt are separate states.
- **Theme drift:** Lemon's stale classic schema and duck's missing role values demand schema/version/fallback tests.
- **Hardcoded-token drift:** Brook native tools must be caught by a linter when global colors/metrics are duplicated.
- **Wrong-hit-region UI:** Lemon minimize hover should become a general test that visible hover, click target, and invoked action resolve to the same control.
- **Empty callback/dead selection:** duck Files selection demonstrates why every enabled action needs a reachable effect and error state.
- **Silent data failure:** duck Editor open/save errors demonstrate why document operations need atomicity and surfaced failure.
- **Registry growth failure:** simple launcher list/grid must be tested with zero, one, hundreds, long localized names, missing icons, duplicates, and malicious metadata.
- **Service authority failure:** world-accessible/global IDs and guessable shared-memory references in small servers become hostile-client tests.
- **Fixed-font failure:** ASCII-only and pixel-fixed UIs must be tested with Arabic, Hebrew, CJK, combining marks, emoji policy, long German/Finnish strings, and 200% scale.
- **Parser-in-app failure:** images/media/PDF/web payloads crash disposable workers, never the shell or owning document app.
- **Taskbar incompleteness:** banan and duck telemetry-only bars demonstrate the need for task-switch/minimize/workspace acceptance tests.

## Delivery order and proof gates

The target remains the full desktop/browser/application system. The order below reflects dependencies, not a deliberately crippled MVP.

### Phase 0 — evidence and visual contract

- Freeze current zlOS screenshots, app list, assets, interaction map, and provider evidence.
- Define token/theme/app-manifest schemas and evidence labels.
- Add screenshot scenes for every widget state, scale, contrast theme, and locale stress case.

Exit: schema validators reject missing/invalid roles; last-known-good theme recovery is demonstrated; existing UI evidence is preserved.

### Phase 1 — process-safe visual platform

- User-space compositor/window service, typed bounded surface/input protocol, theme/font/icon services, shared layout/widget library.
- Focus, move, resize, min/max/close, taskbar, launcher, keyboard navigation, and crash/restart handling.

Exit: hostile client cannot map another surface, spoof focus, overflow geometry, starve input, or retain handles after restart; screenshot/input matrix passes.

### Phase 2 — registry, settings, and daily core

- Signed app bundles; launcher search; file associations; settings domains; document portal; clipboard; notifications.
- Terminal, Files, Editor, Viewer, Calculator, Monitor, Device Manager, Logs, Welcome, Help, Screenshot.

Exit: install → discover → launch → open document → save atomically → crash/recover → uninstall is one reproducible receipt per app.

### Phase 3 — storage/media robustness

- File-operation service with journal/trash/undo; thumbnails/decoders; audio service and media players.
- Parser workers, device-loss recovery, bounded queues, and per-app volume.

Exit: corrupt inputs, decoder crashes, canceled operations, full disk, power-cut simulation, and audio-device removal produce bounded visible recovery.

### Phase 4 — web service foundation and interim compatibility browser

- DNS/TLS/request/cache/cookie/download services, file portal, certificate UI, proxy/network settings.
- Ported browser runs under the same restrictions while native shell/content work proceeds.

Exit: network failures, certificate errors, proxy, cancellation, download integrity, cookie/storage clearing, and browser-process crash have UI receipts.

### Phase 5 — native browser platform

- Split browser shell/content/worker architecture, native HTML/CSS/DOM/layout/paint/JS growth, storage and developer tools.

Exit: process graph and capabilities are inspected live; cross-origin probes, decoder faults, content hangs, crash/reload, history/bookmark persistence, keyboard/accessibility, and a versioned standards corpus pass.

### Phase 6 — creation suite and workload expansion

- Pixel editor, presenter, spreadsheet, theme/font tools, audio creation; native games and hhuOS-style workload lab; attributed ports.

Exit: documents round-trip, recover after crash, export portable formats, and pass scale/locale/accessibility checks; workloads exercise real isolated providers.

### Phase 7 — convergence and self-hosting

- Rebuild shell, services, apps, and browser components increasingly in zl; keep protocol/schema compatibility and reproducible packages.

Exit: clean-room rebuild produces the same manifest graph and behavioral receipts, then the system rebuilds the matching toolchain and itself inside zlOS.

## Exact first-party target appendix

This appendix makes counts auditable. Ports are named separately or linked to the companion exhaustive inventory.

### Brook

- Native desktop/server: `waylandd`, `brook-files`, `brook-edit`, `brook-player`, `brook-console`, `wayland-calc`, `brook-fbtest`.
- Static probes/demos: `bbtest`, `crash_test`, `demo`, `echotest`, `exectest`, `fibonacci`, `forktest`, `futexstress`, `hello`, `mandelbrot`, `memtest`, `pipetest`, `primes`, `schedstress`, `scm_rights_test`, `stresstest`, `symlink_test`, `syscheck`, `sysinfo`, `wayland_test`, `wmtest`; CoreMark is optional/external.
- Package path: `nix-search`, `nix-install`, `nix-fetch`, `nar-unpack`, index generation.
- Other native/adaptation tools: `sinetest`, `wavplay`, `mp3play`, profiler/crash/panic helpers; protocol packages `wayland-smoke`, `wayland-shm-smoke`, `wayland-xdg-smoke`, `wraptest`, `ladybird-tests`.
- NetSurf, Ladybird, Weston demos, Quake/Doom/GIMP/Mousepad are external or port/adaptation surfaces, not Brook-native applications.

### Astral

- Native runtime/admin: `mount`, `netd`, `netinfo`, `prof`, `systrace`.
- Distribution/session helpers: `astral-bootstrap`, `astral-chroot`, `startwm`, `startnscde`.
- `sndiod`, `sshd`, `xdm`, Xorg/FVWM/NSCDE, WebKitGTK/Links, GTK/Qt apps, Wine, games, editors, media and 307 recipes are third-party integration. The companion inventory records all 307 names.

### banan-os

The 62 default userspace names are: `audio`, `audioctl`, `AudioServer`, `bananfetch`, `basename`, `cat`, `cat-mmap`, `chmod`, `chown`, `ClipboardServer`, `cp`, `dd`, `dhcp-client`, `dirname`, `driver-install`, `DynamicLoader`, `echo`, `env`, `false`, `find`, `getopt`, `http-server`, `id`, `image`, `ImageViewer`, `init`, `kill`, `less`, `ln`, `loadfont`, `loadkeys`, `ls`, `meminfo`, `mkdir`, `mv`, `nologin`, `nslookup`, `poweroff`, `ProgramLauncher`, `pwd`, `resolver`, `rm`, `Shell`, `sleep`, `snake`, `sort`, `stat`, `sudo`, `sync`, `TaskBar`, `tee`, `Terminal`, `test`, `top`, `touch`, `tr`, `true`, `u8sum`, `uname`, `whoami`, `WindowServer`, and `yes`. The 128-port list is preserved in the companion report.

### LemonOS

- Five system targets: `init.lef`/Lemond, `netgov.lef`, `kmod.lef`, `login.lef`, `lemonwm.lef`; FTerm source is not build-reachable.
- 22 installed app executables: File Manager, Shell, `lsh`, Terminal, TextEdit, ImgView, AudioPlayer, VideoPlayer, LemonMonitor, SysInfo, DeviceManager, Run, Welcome, Minesweeper, Snake, JSONDump, `steal`, GraphicsTest, GUITest, IPCTest, SignalTest, `tests.lef`.
- Nine utilities: `cat`, `echo`, `rm`, `ls`, `uname`, `hexdump`, `ps`, `playaudio`, `lemonfetch`.

### SerenityOS

- 51 active graphical apps: `3DFileViewer`, `About`, `AnalogClock`, `Assistant`, `Browser`, `BrowserSettings`, `Calculator`, `Calendar`, `CalendarSettings`, `CertificateSettings`, `CharacterMap`, `ClockSettings`, `CrashReporter`, `Debugger`, `DisplaySettings`, `Escalator`, `FileManager`, `FontEditor`, `GamesSettings`, `Help`, `HexEditor`, `ImageViewer`, `KeyboardMapper`, `KeyboardSettings`, `Magnifier`, `Mail`, `MailSettings`, `Maps`, `MapsSettings`, `MouseSettings`, `NetworkSettings`, `PartitionEditor`, `PDFViewer`, `Piano`, `PixelPaint`, `Presenter`, `Run`, `Screenshot`, `Settings`, `SoundPlayer`, `SpaceAnalyzer`, `Spreadsheet`, `SystemMonitor`, `Terminal`, `TerminalSettings`, `TextEditor`, `ThemeEditor`, `UsersSettings`, `VideoPlayer`, `Weather`, `Welcome`.
- 13 active games: `BrickGame`, `Chess`, `ColorLines`, `FlappyBug`, `Flood`, `GameOfLife`, `Hearts`, `MasterWord`, `Minesweeper`, `Snake`, `Solitaire`, `Spider`, `2048`.
- 29 named service directories: `SystemServer`, `LoginServer`, `LaunchServer`, `ConfigServer`, `WindowServer`, `Taskbar`, `Clipboard`, `NotificationServer`, `AudioServer`, `FileOperation`, `FileSystemAccessServer`, `ImageDecoder`, `RequestServer`, `WebContent`, `WebWorker`, `WebDriver`, `NetworkServer`, `DHCPClient`, `LookupServer`, `DeviceMapper`, `KeyboardPreferenceLoader`, `CrashDaemon`, `ChessEngine`, `SQLServer`, `SSHServer`, `TelnetServer`, `EchoServer`, `WebServer`, `SpiceAgent`.
- The 223 utility targets are enumerated exactly in the companion report; optional/differently located metadata targets are not folded into the 51.

### MollenOS

- Native apps: `osstat`, `systat`.
- User-facing substrate services: `deviced`, `filed`, `processd`/`phoenix`, `served`, `sessiond`, `usbd`, `netd`; syslogd is disabled. Vioarr and full userspace are external.

### hhuOS

The 45 active applications are: `shell`; `cat`, `cp`, `date`, `echo`, `head`, `hexdump`, `kill`, `ls`, `mkdir`, `ps`, `pwd`, `rm`, `rmdir`, `touch`, `tree`; `mount`, `unmount`; `ip`, `ping`, `uecho`, `nettest`; `keyboard`; `shutdown`, `uptime`, `smbios`; `beep`, `play`; `ctest`, `membench`; `asciimate`; `demo`; `portablegl`, `tinygl`; `view3d`; `battlespace`, `bug`, `dino`, `rogue3d`; and third-party-integrated `classicube`, `clownmdemu`, `doom`, `litenes`, `peanut-gb`, `quake`.

### duckOS

- Four services: `init`, `pond`, `quack`, `dhcpclient`.
- 12 graphical/VM targets: `about`, `calculator`, `terminal`, `4inarow`, `ducksweeper`, `monitor`, `sandbar`, `files`, `editor`, `viewer`, `3demo`, `uxn`.
- Shell: `dsh`.
- 22 core utilities: `open`, `cat`, `chmod`, `chown`, `cp`, `echo`, `free`, `ln`, `ls`, `mkdir`, `mv`, `ps`, `pwd`, `rm`, `rmdir`, `touch`, `truncate`, `play`, `date`, `uname`, `kill`, `profile`.

## Final evidence ceiling

This pass establishes source ownership, active build/registration reachability, assets, configurations, documented intent, and screenshot appearance at the pinned revisions. It does **not** establish current bootability, pixel-perfect runtime, input correctness, browser compatibility, hardware acceleration, assistive usability, localization quality, persistence across cold boot, port installation, or security isolation.

The weakest links are:

1. MollenOS visual behavior is external.
2. Astral application behavior is overwhelmingly upstream.
3. Brook's large apps/browser paths are ports and integration tests, while its native UI is fixed-font and kernel-heavy.
4. Serenity's process architecture includes aspirational documentation and its broad Web modules are not conformance proof.
5. None of the eight received a fresh boot or app launch in this pass.

Those limits are not reasons to discard the ideas. They define the exact runtime gates zlOS must surpass.
