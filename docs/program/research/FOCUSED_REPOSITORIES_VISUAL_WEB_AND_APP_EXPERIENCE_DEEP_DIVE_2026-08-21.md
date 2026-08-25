# Focused repositories: visual, web, and application experience deep dive

**Date:** 2026-08-21
**Scope:** the 16 focused/supporting starred repositories, audited as user experiences rather than as lists of claimed features.
**Purpose:** extract clean-room product, interaction, visual-system, application, browser, media, game, and operations ideas for zlOS without copying implementation.

This report complements `FOCUSED_REPOSITORIES_DRIVERS_AND_APPS_DEEP_DIVE_2026-08-21.md`. That report answers what hardware and application surfaces exist. This one asks a stricter set of questions:

1. What can a person actually see or do?
2. Which source path makes that interaction work?
3. Is it reachable from the shipped build/start graph?
4. Is there runtime, screenshot, test, or only claim evidence?
5. Which visible controls are decorative, simulated, stubbed, dead, external, or misleading?
6. What can zlOS learn at the product-system level without reusing code?

## Evidence contract

- **[C] Claim:** README, roadmap, marketing text, comment, or UI copy.
- **[S] Source:** concrete implementation exists in the local snapshot.
- **[R] Reachable:** connected to a default build, boot, command, route, launcher, or package graph.
- **[T] Test:** automated check specifically exercises the behavior.
- **[A] Asset:** repository screenshot, image, font, icon, audio, or other presentation asset.
- **[V-prior] Prior validation:** build/runtime evidence recorded by the companion audit; not rerun for this visual pass.
- **[stub] Stub/simulation:** interface exists but the underlying operation is fixed, fake, incomplete, or always fails.
- **[dead] Dead surface:** asset or implementation exists but is not connected to the current route/build.
- **[external] External UX:** a linked site/tool/application, not a feature implemented by the repository.
- **[risk] Source-level defect/risk:** visible or operational behavior is likely wrong, but was not runtime-reproduced in this pass.

“Present in source” is not “working in the image.” “Shown in a screenshot” is not “current.” “A network stack exists” is not “the Browser uses it.” “A video mode is selected” is not “there is a GUI.”

This was a source-and-asset audit. No new QEMU run or build was used to upgrade source evidence to runtime proof. Existing build/runtime statements are marked **[V-prior]**.

## Frozen snapshots and rights boundary

| Repository | Audited commit | Rights signal | Clean-room consequence |
|---|---:|---|---|
| `robotman2412/sapphire-gpu` | `575efe…` | CERN-OHL-P-2.0 | Learn from protocol/product framing; do not transplant HDL/source. |
| `UltraOS/Hyper` | `d4f446…` | MIT | Ideas are usable; still design independently. |
| `ProvableHQ/snarkOS` | `a8e2e3…` | Apache-2.0 | Treat as operator-UX reference, not OS UI code. |
| `dspinellis/unix-history-repo` | `d0d404…` | mixed historical rights | Use history/provenance method only. |
| `felipenlunkes/osdev-projects` | `0736c9…` | BSD-3-Clause | Catalogue concept only. |
| `marceldobehere/MaslOS-2` | `154d42…` | AGPL-3.0 | Strong copyleft: observe workflows, independently implement. |
| `alexdboxall/Banana-Operating-System` | `f46354…` | CC BY-NC plus notices | Do not copy; extract only product ideas and system boundaries. |
| `KingVentrix007/AthenX-2.0` | `e67344…` | GPL-3 root; README badge conflicts | Treat all implementation as non-donor. |
| `ktauchathuranga/privilegeos` | `1563a3…` | MIT wrapper; upstream tools retain rights | Copy neither scripts nor bundled tools blindly. |
| `yashvi-soni-30/mYOS` | `20bf15…` | no root license found | No code reuse. |
| `cppsever/MineBios` | `ca9831…` | Apache-2.0 | Learn from bounded interaction loop. |
| `viralcode/vib-OS` | `73b642…` | README says MIT; no root license found | Treat implementation as unavailable for reuse. |
| `whispem/asm.fm` | `a55729…` | MIT | Audio concepts remain preferable to source reuse. |
| `brainboxdotcc/mission-control` | `a6b7ec…` | Composer says MIT; no root license found | Rebuild portal/lease architecture independently. |
| `osdev0/freestanding-c-hdrs-gnu` | `cdd9b3…` | GPL-3 plus GCC exception | Generated toolchain artifact, not UI donor. |
| `Ellicode/protOS-kernel` | `daa97b…` | MIT | Useful architecture reference; still avoid source-shaped cloning. |

## Executive truth table

| Repository | Real user-facing experience | Strongest visible idea | Largest truth boundary |
|---|---|---|---|
| Sapphire | Developer reads a GPU protocol/spec and runs simulations | Capability/status schema for a future display diagnostics app | No rasterizer, shader, compute UI, or finished GPU behind the broad claims |
| Hyper | Administrator writes `hyper.cfg`; boot logs/errors report selection | Small typed boot configuration with deterministic default | Multiple entries are not an interactive boot menu |
| snarkOS | Rich CLI plus two-tab terminal operator dashboard | Secret-safe account flow and operator overview/logs | REST exists, but there is no product browser GUI; one peer-table source mismatch |
| Unix history | Git queries over historical Unix source | Feature provenance as an explorable product surface | Repository supplies no visual explorer |
| osdev-projects | README tables and external links | Curated project catalogue | No site, filters, schema, generation, or link validation |
| MaslOS-2 | Windowed desktop with launcher, taskbar, productivity apps, games, media | Separate app processes, file dialogs, IPC, dirty rectangles | Screenshots span eras; commented launcher entries and unverified app reachability matter |
| Banana OS | Desktop, file manager, console, editors, utilities, games | File associations, per-size icons, packages, locale/keymap assets | Strong source inventory, weak current screenshot/runtime proof |
| AthenX | Framebuffer terminal/login, command suite, debug screens, demos | Discoverable diagnostics shell and readable failure screen | Authentication returns success unconditionally; “3D” is a demo, not a GPU stack |
| privilegeOS | Colored rescue shell and guided destructive/recovery scripts | Preview → authorize → act → verify → restore workflow | “Verification” checks device size, not written bytes; system-drive filtering is brittle |
| mYOS | Tiny VGA shell with persistent toy filesystem | Small complete command loop with history | Color parsing, prompt path, scrolling, scoping, and storage limits contradict polish |
| MineBios | One bootable block-placement game | Immediate, teachable control loop | Not an OS suite or Minecraft clone; editor links are external/expired |
| vib-OS | Cohesive macOS-inspired desktop with many painted apps | Shell hierarchy, dock, media/file flows, context menu | Browser/curl/wget/ping/network indicators are mock or simulated; architecture is monolithic |
| asm.fm | CLI builds offline WAV synthesizer examples | Deterministic audio fixtures and A/B signal examples | No player, real-time audio path, browser UI, or visualization |
| mission-control | Real responsive web portal controlling disposable QEMU sessions | Capacity-aware, deadline-aware, mobile-capable public demo | Retry/state/rollback/auth/accessibility gaps; no explicit stop/reconnect path |
| freestanding headers | Toolchain maintainer consumes generated headers/CI output | Provenance and reproducible environment metadata | No application, visual asset, or end-user UI |
| protOS | Userspace WM, terminal, shell, small utilities | WM/event IPC + PTY terminal + shared surface refresh | Very small window chrome; unused wallpaper; unsafe shared-memory/IPC boundaries |

## Experience-system map

The repositories cover five different products. Treating them all as “operating systems with apps” would erase the useful distinctions.

| Product class | Repositories | Primary user | Primary interface |
|---|---|---|---|
| Desktop/application OS experiments | MaslOS-2, Banana OS, vib-OS, protOS | desktop user/developer | framebuffer compositor, windows, applications |
| Console/shell OS experiments | AthenX, mYOS | developer/learner | VGA/framebuffer terminal and commands |
| Single-purpose bootable experience | MineBios, privilegeOS | player / recovery operator | game loop / root rescue shell |
| Developer/operator product | Sapphire, Hyper, snarkOS, asm.fm, freestanding headers | implementer/operator | docs, config, CLI, TUI, files, CI |
| Web/corpus/catalogue surface | mission-control, Unix history, osdev-projects | visitor/researcher | web portal / Git / Markdown |

The reusable architecture is therefore not “copy the prettiest desktop.” It is a chain:

```text
visual language
  -> shell and navigation
      -> window/application lifecycle
          -> app workflows and shared services
              -> hardware/network/storage capability
                  -> honest states, tests, and receipts
```

vib-OS is strongest at the first two layers and weak at truth underneath. protOS and MaslOS expose more of the application/process boundary. mission-control is strongest at remote delivery and session expectation-setting. snarkOS is strongest at operator feedback. No single repository completes the chain.

---

## 1. `robotman2412/sapphire-gpu`: documentation is the interface

### What the user actually experiences

There is no desktop, control panel, graphical demo, media app, game, browser, icon set, theme, or bundled screenshot. The real experience is a developer reading Markdown protocol documents and running hardware simulations **[S]**.

The documentation presents command/status concepts that could eventually support a human-facing GPU inspector: serial command framing, command/result statuses, memory operations, and advertised capabilities. This is useful information architecture, not a visual product.

### How it works underneath

- Commands are described as a device protocol, not user actions.
- Hardware simulation/build targets are the validation surface **[V-prior]**.
- The README’s broad 2D/3D, rasterization, shader, and compute direction outpaces connected implementation **[C/S]**.
- `docs/command_summary.md` links to `phys_serial.md`, but the actual file is `docs/cmd/serial.md` **[risk]**.
- `docs/compute.md` and `docs/drawing.md` are essentially section headings rather than complete behavioral references **[stub]**.

### Visual/design-system assessment

There is no visual system to audit. Documentation navigation itself is inconsistent: broken path naming and aspirational empty sections make capability discovery less trustworthy.

### Transferable clean-room idea

Make graphics capability discoverable by machines first, then render it in a zlOS **Display Diagnostics** app:

- adapter and output identity;
- current mode and supported modes;
- framebuffer/scanout allocation;
- command/capability versions;
- last command status and structured error;
- software fallback state;
- a self-test card whose result links to a receipt.

The important lesson is not Sapphire’s visual appearance—it has none—but that the UI should consume the same structured capability/status contract as the driver.

---

## 2. `UltraOS/Hyper`: a configuration UX, not a boot-picker UX

### What the user actually experiences

Hyper’s human workflow is:

1. build/install the bootloader;
2. author `hyper.cfg`;
3. define one or more entries;
4. choose `default-entry`, or let the first entry win;
5. read firmware/serial logs and errors during boot.

There is no interactive chooser, countdown, keyboard selection, themed background, icon, font, or boot-menu screenshot **[S]**. “Multiple loadable entries” must not be translated into “boot menu.” `loader/loader.c` parses and automatically selects an entry **[S/R]**.

### Underlying workflow

- The config syntax is a simplified YAML-like tree with strings, integers, booleans, nulls, objects, and entry sections.
- Path origin prefixes give configuration a compact way to address boot-volume resources.
- Video-mode selection negotiates handoff information for the kernel; it does not render a GUI.
- Errors and assertions are the principal observability surface.
- The parser has limits, including no general string escaping; this is administrator-facing debt, not just parser trivia.

### Operations UX assessment

Good:

- deterministic fallback to the first entry;
- explicit entry naming and defaults;
- config near the boot artifacts;
- direct errors rather than silent fallback.

Missing:

- config validation command with line/column diagnostics;
- interactive recovery entry selection;
- safe-mode/edit-current-entry path;
- visible timeout/default indication;
- per-entry metadata such as icon, description, kernel version, last-known-good;
- boot receipt telling the OS exactly why an entry/mode was selected.

### Transferable clean-room idea

Separate zlOS’s boot experience into a deterministic loader contract and an optional presentation layer. The loader should work headlessly; the picker should consume the same parsed entry model. That prevents a pretty menu from becoming a second, inconsistent source of boot logic.

---

## 3. `ProvableHQ/snarkOS`: the strongest terminal operator UX

### Real surfaces

snarkOS is not a desktop OS. Its user-facing product is a styled command-line application, a terminal dashboard, structured node logs, and a REST operator surface **[S/R]**.

Top-level commands include account management, clean, developer operations, node start, and update. Developer flows cover decrypt, deploy, execute, scan, and private transfer. Account flows cover creation/import, signing/verifying, vanity search, and key persistence.

### Account and secret UX

The account commands demonstrate unusually deliberate terminal interaction:

- hidden password input;
- discreet private-key presentation in an alternate screen;
- ownership/permission checks, including mode `0600` expectations;
- explicit sign/verify workflows;
- styled help with visually distinct headers, literals, invalid values, and errors;
- human-readable and machine-usable output paths.

The key transferable behavior is **progressive disclosure of secrets**. A normal log should say that a secret exists; a deliberate, temporary view may reveal it; persistence must apply restrictive permissions; automation receives structured output only when explicitly requested.

### Node TUI

The Ratatui interface is a real two-tab operator console **[S/R]**:

- alternate-screen/raw terminal mode and mouse capture;
- one-second update tick;
- left/right tab switching;
- Escape to stop;
- **Overview:** latest block, sync status, peers, and help;
- **Logs:** bounded 128-entry cache;
- startup banner, role/warning output, updater status, colored progress, and transaction confirmation.

The UI is backed by live node state rather than painted sample numbers. However:

- peer table headers and layout define three columns (`IP`, `State`, `Node Type`), while row construction emits four values including `last_seen` **[risk]**; the fourth value may be clipped or misrendered;
- CDN synchronization occurs before the TUI starts, and source contains a TODO to expose that progress earlier **[S]**;
- Escape-to-stop is concise but deserves a confirmation/first-press warning for long-running validator state;
- secret/JWT material should never be repeated in ordinary logs;
- color and symbol choices need non-color equivalents and accessibility review.

### Browser/web boundary

The REST server is a backend surface, not a product web dashboard. `node/bft/examples/assets/index.html` is an example asset, not evidence of a browser application. There is no bundled HTML operator console, wallet GUI, or graphical account manager.

### Transferable clean-room ideas

- One zlOS **System Monitor** with Overview and Logs, backed by real kernel service telemetry.
- Role-aware command help and consistent severity styling.
- Bounded log retention in the UI with export to a receipt file.
- Explicit sync/startup phases before the main dashboard appears.
- Temporary secret views that disappear without polluting scrollback.
- Human and JSON modes generated from the same typed result.

---

## 4. `dspinellis/unix-history-repo`: provenance can be a product feature

### Actual experience

This repository is a historical Git corpus, not an operating system or application collection. Its real user interface is Git:

- search commits;
- browse branches/tags;
- follow a file through renames;
- blame a line;
- compare eras;
- inspect merges and authorship.

The companion audit counted the local corpus at roughly 849,000 commits **[V-prior]**. This pass does not restate that as a current upstream figure.

### What is absent

There is no bundled web explorer, timeline, query UI, screenshot set, theme, graph renderer, app launcher, or end-user documentation browser. Git hosting visualizations are **[external]**.

### Transferable clean-room idea

zlOS should make provenance visible at the feature level:

- “when did NVMe first boot?”
- “which commit introduced resize?”
- “what runtime receipts prove this app release?”
- “what subsystem owns this setting?”
- “which architecture epochs changed this API?”

A **History/Receipts** app could join source milestones, build receipts, boot screenshots, device traces, and user-visible releases into an explorable timeline. The donor idea is historical queryability, not a visual design.

---

## 5. `felipenlunkes/osdev-projects`: a catalogue trapped in Markdown

### Actual experience

The repository is a curated README of OS-development projects and links. The workflow is scroll, read table rows, use browser find, and open external repositories **[S/external]**.

There is no application, static site, schema, search UI, filter, screenshot gallery, automatic categorization, health indicator, or link checker.

### Information-design value

The catalogue establishes that one concise surface can answer “what else exists?” It does not solve:

- whether a project still builds;
- architecture/platform support;
- latest validation date;
- GUI versus console maturity;
- license compatibility;
- feature proof;
- screenshot recency;
- dead links.

### Transferable clean-room idea

Build zlOS’s internal feature/app registry from structured manifests, then generate both docs and UI. Filters should include subsystem, maturity, reachability, last verified commit, hardware/runtime proof, license provenance, and owner. Ctrl-F over Markdown can remain a fallback, not the primary interface.

---

## 6. `marceldobehere/MaslOS-2`: broad desktop workflows and real app separation

### What the screenshots show

Repository assets show several eras rather than one guaranteed current release **[A]**:

- early gray/purple desktop and debug/status panels;
- later photo wallpaper and more conventional windows;
- tiled desktop demos with terminal, paint, games, and utilities;
- DOOM;
- Minesweeper;
- window-manager progress images and a terminal GIF.

They prove that those visual states existed, not that every pictured application launches from the audited commit.

### Desktop shell

The desktop and window manager have real source paths under `modules/desktop`, `modules/startMenu`, `libm/gui`, `libm/window`, and `libm/wmStuff` **[S/R]**.

The shell includes:

- a blue taskbar;
- an `M` start button;
- per-window task items with icons;
- selected and hover states;
- task items that shrink between roughly 60 and 200 pixels;
- ellipsized titles;
- date/time from the RTC;
- a start menu excluded from the normal task list.

The start menu is a fixed 200×400, titleless, non-movable/non-resizable black window with hard-coded colored text buttons. Active launch buttons cover Pong.maab, Mandelbrot.maab, Alert.maab, terminal, Explorer, Notepad, Image Viewer, Paint, TicTacToe, Flappy Rocc, DOOM, and Minesweeper. Task Manager, Music Player, and Magnifier entries are commented out **[dead]**.

### Window/app model

Windows support movement, resize, show/hide, title/border colors, and titlebar/border toggles. User processes communicate with the desktop through IPC and can request partial window updates **[S]**. Dirty-rectangle refresh is a meaningful performance/architecture idea, not merely a visual detail.

The GUI library supplies buttons, canvases, text, text fields, boxes, rectangles, and related primitives. Styling is primarily hard-coded inside apps/components rather than resolved from design tokens.

### Meaningful application/workflow inventory

Program directories in the snapshot are:

`3DKub`, `audioTest`, `calcRocc`, `connectFour`, `doom`, `empty`, `explorer`, `flappyRocc`, `fsTest`, `goofyTetris`, `helloTest`, `imgView`, `logger`, `maab`, `microEdit`, `minesweeper`, `miniWinTest`, `msgTest`, `musicPlayer`, `notepad`, `openFileExplorer`, `paint`, `saveFileExplorer`, `shell`, `stdioTest`, `threadTest`, `threadTest2`, `ticTacToe`, `winTest`, and `winTest2` **[S]**.

Important real workflows:

- **Notepad:** edit, Load, Save, and Save As through separate dialog processes and the VFS.
- **Paint:** Open, Save, Save As, Clear, canvas drawing, brush size, and a 28-color MS-Paint-like palette.
- **Explorer/open/save explorers:** file browsing and service-like picker behavior.
- **Image viewer:** image-file presentation.
- **microEdit:** line numbers/gutter, syntax highlighting, and bracket matching.
- **Minesweeper:** Easy 9×9/10, Medium 16×16/40, Hard 22×22/120; flags, timer, restart, and custom PSF font.
- **Music/audio:** WAV assets and AC97-connected experiments; existence is stronger than default-launch proof.
- **Games/demos:** DOOM, Pong, TicTacToe, Flappy Rocc, Connect Four, Tetris, 3D cube, Mandelbrot.
- **Developer/test apps:** filesystem, message, stdio, window, and thread exercises; these are validation utilities, not finished consumer apps.

### Visual-system assessment

Strengths:

- recognizable taskbar/start-menu/window mental model;
- clear app separation;
- custom fonts and icons/images;
- enough state distinction to make focus and selection visible;
- many end-to-end workflows rather than painted panels.

Weaknesses:

- hard-coded colors and geometry instead of semantic tokens;
- visual inconsistency between historical eras and individual apps;
- fixed launcher manifest;
- no evident DPI, localization, high-contrast, reduced-motion, or screen-reader model;
- icon/state conventions are not documented as a component contract;
- commented launcher items risk confusing source inventory with reachable product.

### Transferable clean-room ideas

The strongest donor is application architecture:

- separate app modules/processes;
- conversation IDs and standard-I/O-like IPC;
- a shared file-picker service;
- app windows backed by partial surface refresh;
- file-based apps that implement the whole open/edit/save cycle;
- tiny validation apps for each GUI/service contract.

zlOS should retain those boundaries while replacing fixed colors/geometry with tokens, manifests, accessibility semantics, and receipts.

---

## 7. `alexdboxall/Banana-Operating-System`: packages, associations, icons, and a surprisingly broad suite

### Desktop shell and presentation assets

`applications/desktop/desktop/Main.cpp` registers itself as the desktop, creates a 28-pixel taskbar, loads a wallpaper, enumerates desktop/package content, tracks selection, and opens files according to registry associations **[S/R]**.

The repository carries 8×8, 16×16, and 64×64 BMP icons for GUI applications, plus system cursors, colored icons, wallpapers, keymaps, locale data, registry configuration, and `desktop.ini`-style metadata **[A]**. This is the focused set’s strongest explicit multi-size icon discipline.

The visual component library under `libraries/userdraw` includes top-level windows, buttons, text fields, fonts, loaded bitmaps, regions, and window creation. `NTextField` includes cursor, selection, and horizontal scrolling logic **[S]**.

### Applications and real workflows

The source tree contains these user-facing app roots **[S]**:

- **Cabinet:** file manager; title follows current directory and entries are enumerated from the filesystem.
- **Regedit:** registry editor opening `SYSTEM.REG`.
- **command + conhost:** shell/command processor hosted in a 640×440 non-resizable console window.
- **desktop:** wallpaper, desktop icons, taskbar, selection, file associations.
- **Minesweeper:** Easy 10×10, Normal 15×15, Hard 25×15; win/loss icon state and restart controls.
- **Solitaire:** real card, deal, tableau, foundation, discard, and holding-pile model rather than a screenshot mock.
- **Photo Viewer:** opens an image argument into a size-matched viewer window.
- **RAM Viewer:** small memory-inspection window.
- **Sentences (`newgui`):** document-style text editor with icon toolbar and file input; some actions use fixed `C:/test.doc`, reducing it from a complete Save As experience.
- **Typewriter:** simpler editor with new/open/save-style toolbar; save uses fixed `C:/test.txt` **[stub-quality]**.
- **`te`:** terminal-oriented text editor.
- **ZIP:** CLI compression/decompression, create, append, delete, list, aliases, and compression-level selection.
- **cliptest:** userspace window/clipboard experiment rather than a mature user app.

The package tree distinguishes `banana`, development, kernel, PCI, system, and wallpapers content. CAB/package handling gives application distribution a system-level shape rather than making every app a kernel fixture.

### How shell-to-app behavior works

- Desktop entries and file icons are loaded from installed content.
- Registry keys map extensions/types to open programs and icons.
- Executables and common formats receive explicit fallbacks.
- Per-size application icons support desktop, small chrome, and larger launch representations.
- GUI applications use shared `N*` components and event messages.

### Evidence and design limitations

- Source coverage is much stronger than current screenshot coverage; do not claim a polished current Banana desktop from assets alone.
- Several projects retain Visual Studio artifacts, object files, and hard-coded Windows-like paths; this is a historical development environment as much as a clean modern app SDK.
- Fixed save destinations in editor demos make toolbar affordances look more complete than the workflow.
- The companion audit documents unsafe kernel/ring-0 loading boundaries **[V-prior]**; a familiar window is not process isolation.
- No browser/web application is present.
- No unified accessibility, theme, scaling, or localization component contract was found, despite locale/keymap data.

### Transferable clean-room ideas

- application manifests own icons at deliberate sizes;
- file associations are data, not switch statements in each app;
- packages declare desktop entries, formats, and system resources;
- locale and keyboard maps live beside other system assets;
- the desktop asks the registry/package model how to open a file;
- basic games are valuable end-to-end event/render/state tests.

---

## 8. `KingVentrix007/AthenX-2.0`: a visually branded diagnostic shell with a bypassed login

### Visible experience

AthenX presents a framebuffer-oriented console rather than a windowed desktop. Assets/screenshots show **[A]**:

- login screen;
- main terminal;
- file handling;
- PCI lists;
- stack trace output;
- a blue “BOSD” skull failure screen;
- logo and background/gradient treatments.

These are meaningful visual states, but no current runtime was performed in this pass.

### Command/application surface

The kernel-connected command handler exposes a broad workflow set **[S]**:

- help/manual, time, echo, clear/background/logo;
- serial and shutdown;
- drive, error, log, PCI, stack, and driver inspection;
- file commands: `cat`, `pwd`, `touch`, `ls`, `cd`, `rm`, `mkdir`, update;
- install/grab/load/fim paths;
- format/table operations;
- song/audio interaction;
- 3D, RLE, filesystem, executable/raw, and edit demos.

Manual pages in `utils/man.c` make this more discoverable than an undocumented debug monitor.

There are userspace directories for amix, bin, byte, cat, edit/editor/kilo, shell, sim, tar, TCC, and tests. Presence does not establish that each is built, installable, and usable in the audited image; the companion audit records incomplete multitasking/syscall reachability **[V-prior]**.

### How the visuals work

- VESA/VGA/framebuffer paths paint the console and branded backgrounds.
- Bitmap/font assets render text and images.
- PS/2 input drives the shell.
- Debug commands read kernel data directly.
- BOSD/stack screens turn kernel failure state into a legible operator surface.

### False/stub/dead boundaries

- `validate_credentials` returns `0` before the actual comparison logic **[S/stub]**. The login is decorative, not an authentication boundary.
- The 3D command/demo is not evidence of a general GPU, scene, window, or application rendering stack.
- Networking covers lower-level UDP/DHCP/ARP/TFTP paths, not a browser.
- TFTP write is TODO; size handling fetches the whole file **[S]**.
- A logo/background command is a presentation feature, not a theme system.
- Source and screenshots may represent different stages.

### Transferable clean-room ideas

- a discoverable `inspect` namespace for logs, PCI, drivers, stack, disks, and errors;
- a readable crash screen with error ID, stack summary, and receipt destination;
- visual branding during console-only phases;
- manual pages shipped with every command;
- simple diagnostic demos that clearly label their scope.

Never present a login surface until it controls access. A false lock screen is worse than no lock screen.

---

## 9. `ktauchathuranga/privilegeos`: high-stakes operations UX, with a false-green verifier

### Actual product

privilegeOS is a Linux/BusyBox rescue environment, not a desktop OS. Its own visible experience is boot text, a colored/ASCII banner, and an automatic root shell. Etcher, Rufus, and other graphical flashing tools mentioned in documentation are **[external]**.

The included workflows cover:

- `getdrives` discovery;
- `getadmin` recovery/modification flow;
- `putadmin` restoration flow;
- mount/network/power aliases and help;
- USB image writing through `boot.sh`.

### Operations interaction design

The scripts use consistent INFO/WARN/ERROR/SUCCESS language and deliberate gates:

- list target devices;
- display source and destination;
- warn that data will be destroyed;
- unmount the target;
- compare image/device sizes;
- require the exact confirmation `YES`;
- run `dd` with progress;
- write a log;
- provide restoration guidance.

The privilege workflows similarly include legal/authorization confirmation, partition discovery, mounts, backup handling, validation, and a paired restoration command.

### Critical truth defects

- The “Verifying write” phase checks only that the whole target block-device size is at least the image size **[S/risk]**. It does not compare bytes or a hash and cannot detect a corrupted/partial write.
- System-drive exclusion strips trailing digits from the device reported by `df /` **[S/risk]**. That model is brittle for NVMe/MMC names such as `/dev/nvme0n1p2`, so the safety claim is not robust.
- Size/startup figures in README copy were not measured in this pass **[C]**.
- The shell starts as root; attractive warnings do not replace capability isolation.

### Transferable clean-room pattern

For any destructive zlOS operation:

```text
resolve exact target
  -> show immutable action preview
  -> prove target is not system/root media
  -> require typed confirmation
  -> create recovery point
  -> perform bounded operation
  -> verify actual content/state
  -> emit receipt and restoration route
```

Use byte/hash verification, stable device identity, and a reversible journal. The pattern transfers; the specific credential-manipulation workflow does not.

---

## 10. `yashvi-soni-30/mYOS`: tiny persistent shell, many visible limits

### Actual experience

mYOS is a VGA text shell with keyboard editing, ten-command history, and a tiny ATA-backed filesystem **[S/R]**. Commands include:

- `help`, `clear`, `about`, `color`, `reboot`, `halt`;
- `ls`, `touch`, `write`, `cat`, `rm`, `mkdir`, `cd`, `pwd`;
- echo behavior;
- left/right editing and history navigation.

There is no desktop, app launcher, mouse UI, browser, image viewer, font/theme asset system, game, or web UI.

### What works conceptually

The project makes a complete loop visible: boot, prompt, edit command, change directory, persist small files, reboot/halt. This is useful as a teaching and smoke-test environment.

### Concrete UX/implementation defects

- Prompt redraw hard-codes `YOS:/ > ` after editing even when the current directory changed; `pwd` can disagree with the prompt **[S/risk]**.
- `color <0-15>` parses only one character (`input[6]-'0'`), so 10–15 do not work as advertised **[S/risk]**.
- Screen overflow wraps the cursor to the top rather than scrolling; scrolling is explicitly future work **[stub]**.
- Filesystem capacity is four entries, names are 16 bytes, and data is 32 bytes.
- file write/read/remove searches can match a same-named used entry without fully respecting parent/current-directory/type **[S/risk]**.
- ATA readiness waits lack timeout/DRQ/error handling **[S/risk]**; a frozen shell is a user-facing failure.

### Transferable clean-room idea

Keep one tiny “safe shell” that works when the desktop does not. It should share the real VFS and command registry, display the actual path, scroll correctly, time out devices, and expose a receipt command. Minimal must mean narrow, not dishonest.

---

## 11. `cppsever/MineBios`: one coherent game, not an app ecosystem

### Actual experience

MineBios is a single 16-bit bootable assembly game. A repository screenshot shows the block view running in QEMU **[A]**. Controls are documented:

- `W`: back;
- `E`: forward;
- Space: jump/fly;
- arrows: move the selection cursor;
- Enter: place a block;
- Delete: remove a block.

### Underlying experience loop

The VGA renderer draws a fixed tile/chunk view, player sprite, selection border, blocks, and basic movement/physics **[S/R]**. The camera/block model is small and fixed enough to remain understandable.

### Scope boundary

There is no inventory, crafting, save/load, world generator, settings, multiplayer, app launcher, browser, or general OS shell. “Interaction” means selecting, placing, and removing tiles. It should not be described as a Minecraft-class experience.

Linked image/map editor sites are **[external]**, and the referenced domains are expired/unavailable in this snapshot’s research context. The content pipeline is manual: generate data elsewhere, then paste it into assembly.

### Transferable clean-room ideas

- a demo should teach its complete control set on one screen;
- a game is a high-value input/render/timing test;
- content should be data-driven and editable by a first-party tool;
- the smallest satisfying loop is better than ten decorative app icons.

For zlOS, a built-in block/sprite playground could validate keyboard repeat, pointer capture, frame pacing, audio, file saves, and gamepad APIs without being positioned as a full game platform.

---

## 12. `viralcode/vib-OS`: the best-looking shell and the most important false-UI warning

### Visible design direction

vib-OS is the focused set’s most cohesive visual presentation **[A/S]**:

- photographic wallpaper;
- top menu/status bar;
- dock;
- traffic-light window controls;
- draggable/resizable overlapping windows;
- focus/z-order;
- context menu;
- loading screen;
- wallpaper chooser;
- file manager, image viewer, terminal, settings, games, and DOOM screenshots.

The ten dock labels in source are `Term`, `Files`, `Calc`, `Notes`, `Set`, `Clock`, `DOOM`, `Snake`, `Help`, and `Web` **[S]**.

### Real workflows

#### Files

The file manager reads the VFS and renders directory items in a grid. Its toolbar exposes Back, New Folder, New File, and Rename; handlers cover path state and create/rename/delete/open behavior **[S/R]**. This is more than a painted folder window.

#### Notes

Notepad loads and saves through the VFS and includes toolbar/status/gutter/line-number behavior **[S/R]**.

#### Image viewer

The viewer has PNG/JPEG decoding, previous/next file navigation in the containing folder, zoom, rotation, fullscreen, and panning **[S]**. Screenshots support the visible flow **[A]**.

#### Snake and DOOM

Snake has real game state, controls, and score. DOOM is loaded as an ELF at a fixed address and called inside kernel context through a KAPI **[S/R]**. The DOOM experience exists, but it is not a protected userspace application.

#### Other shell-embedded apps

Calculator, Clock, Settings, About, Help, browser, and parts of terminal behavior are dispatched by title-prefix branches inside `kernel/gui/window.c`. They are app-shaped panels inside a monolith, not a mature app/process/component framework.

### Design-system assessment

Strong:

- clear desktop hierarchy;
- repeated window chrome;
- dock labels and icons;
- focused/selected/context states;
- content-appropriate toolbars;
- preview-oriented wallpaper settings;
- media navigation and loading states;
- screenshots communicate a consistent product idea.

Weak:

- visual constants and colors are hard-coded rather than semantic tokens;
- pixel font/icon language has no documented scale/state/accessibility contract;
- title-prefix dispatch couples identity, rendering, and behavior;
- status content is often fixed presentation rather than observed system state;
- duplicated static Wi-Fi drawing code suggests no reusable status component;
- top clock reads PL031 RTC but hard-codes timezone `-5` **[S/risk]**.

### Browser and network: painted, not implemented

The Browser is a mock **[S/stub]**:

- fixed address `http://vib-os.org`;
- fixed welcome page/status/IP text;
- fake links;
- no editable URL, history, navigation, HTTP fetch, parser, DOM, CSS, layout, or page paint pipeline.

The repository’s own roadmap says basic rendering/no full HTML parser and still lists browser implementation as TODO **[C]**.

Terminal `curl`/`wget` prints a hard-coded HTTP response/HTML string rather than performing a transaction **[S/stub]**. `ping` is simulated, and the application KAPI `net_ping` returns `-1` **[S/stub/V-prior]**. The top-bar Wi-Fi symbol is explicitly static “connected.” Settings says “Network: Not connected,” “Audio: Disabled,” and fixed `1024x768`; About prints fixed architecture/display/memory-style strings rather than a settings/telemetry model.

A separate network stack existing in source does not connect these visible browser/status features to reality.

### Other claim/reachability boundaries

- Broad README claims around process management, TCP/IP, ext4/APFS, audio, and GUI maturity include drift/stubs recorded in the companion audit **[C/V-prior]**.
- The userspace tree is not equivalent to the current shell app graph.
- DOOM’s execution path contradicts any implication of safe application isolation.
- Attractive screenshots do not prove each control’s backend.

### Transferable clean-room ideas

Take the product composition, not the imitation/macOS styling:

- shell regions with stable roles;
- launcher with clear app names;
- files open in the appropriate app;
- media navigation and transformations;
- wallpaper preview before apply;
- context menus near the target;
- visible loading/empty/error states;
- simple games as input/frame tests.

Reject the architecture that makes an app a title-string branch and a feature a painted answer.

---

## 13. `whispem/asm.fm`: audio as a deterministic command-line artifact

### Actual experience

asm.fm is neither an OS nor an audio application UI. It is a collection of 25 x86-64 NASM programs that write valid 16-bit mono 44.1 kHz WAV data to standard output **[S]**.

The workflow is:

```sh
./bin/beep > out.wav
aplay out.wav
```

The audio player is **[external]**. There is no bundled GUI player, waveform, spectrum view, sequencer, live keyboard, browser demo, or real-time audio driver.

### Exact experiment inventory

The snapshot contains:

`autowah`, `beep`, `bitcrusher`, `chorus`, `delay`, `distortion`, `envelope`, `filter_sweep`, `fm`, `lowpass`, `melody`, `melody_hard`, `melody_smooth`, `oscillators`, `polyphony`, `polyphony_adsr`, `pwm`, `reverb`, `ringmod`, `scale`, `supersaw`, `tempo`, `tremolo`, `tremolo_strong`, and `vibrato` **[S]**.

Together they demonstrate oscillators, scales/melody, polyphony, timing, envelopes, modulation, filters, delay/reverb/chorus, distortion/bit crushing, FM, ring modulation, PWM, and timbral variation.

### Transferable clean-room ideas

- Make every audio primitive produce a deterministic offline reference WAV.
- Keep paired “hard” and “smooth” examples for regression listening and waveform comparison.
- Add a zlOS **Sound Lab** that can play, pause, render to file, display waveform/spectrum, and compare expected versus device-captured output.
- Separate synthesis correctness from the real-time driver/mixer path.

This repository is a donor for audio test pedagogy, not for desktop appearance.

---

## 14. `brainboxdotcc/mission-control`: the real web application and remote-demo donor

### What the public visitor sees

mission-control is a responsive web portal for launching temporary QEMU sessions through noVNC **[S/R]**. It is not an OS; it is the delivery and operations wrapper around one.

The landing page includes:

- product branding/logo/tagline;
- explanation that the session is a real temporary OS;
- expected startup time;
- slot availability or full-capacity warning;
- Start Session button with disabled/“Starting…” state;
- Learn More;
- focus/stuck-key advice;
- duration, idle timeout, fresh-state, and concurrency expectations;
- responsive two-column layout.

The session page includes:

- connection status pill;
- server-authoritative time remaining;
- progress indicator that changes near 25% and 10%;
- expiry warning;
- noVNC canvas;
- mobile/coarse-pointer soft keyboard controls.

### Visual system

CSS defines a small semantic token set: background, panel, text, muted, border, danger, and success/OK colors. Cards, buttons, tags, alerts, status dots, and responsive grids form an actual—if compact—web design system **[S]**. It uses a system font rather than imported typography.

The design is restrained and task-specific. It avoids pretending the website is the guest OS; the browser chrome frames the remote session and explains its limits.

### Interaction details

Client code includes **[S]**:

- noVNC `RFB` construction;
- viewport scaling and remote resize;
- connection state updates;
- touch/mobile soft keyboard bar;
- text-input key bridging;
- Ctrl/Alt toggles and Escape;
- VisualViewport repositioning;
- server-authoritative countdown refreshed by heartbeat;
- bounded local countdown between heartbeats;
- input activity touch/lease renewal;
- best-effort Beacon release on exit.

### Service architecture underneath

The server connects the visible session state to:

- slot allocation under a lock;
- lease/session token;
- disposable overlay creation;
- QEMU launch;
- noVNC WebSocket path;
- reaper/expiry cleanup;
- admin CLI for status, leases, kill, slot kill, capacity setting, and reap;
- JSON output for operations **[S/R]**.

This is the most important systems lesson in the focused set: demo UX is not a screenshot page. It is capacity, lease, isolation, expiry, remote input, failure recovery, and cleanup presented as one product.

### Concrete defects/gaps

- CSS defines `.dot.ok`, but JS adds `.good`; connected status may retain the muted dot rather than green **[S/risk]**.
- `align-items: centre` is invalid CSS, so intended tag alignment is ignored **[S/risk]**.
- OpenGraph/Twitter image metadata appears to yield the description variable instead of a dedicated image value **[S/risk]**.
- Session touch logic is duplicated: inline Blade heartbeat/activity code and imported JS use different intervals, creating redundant requests/listeners **[S/risk]**.
- The retry loop returns after constructing `RFB`; construction does not await a successful connection, so it does not provide meaningful failed/disconnected reconnection **[S/risk]**.
- Session allocation lacks a full database transaction boundary **[V-prior]**.
- Overlay/QEMU launch failure after lease allocation has no complete rollback path **[V-prior]**.
- The visible VNC/WebSocket data-plane authorization boundary is not clearly enforced at this repository layer **[V-prior/risk]**.
- Start request performs launch synchronously before redirect; the page promises a startup duration but provides no staged live progress.
- There is no explicit End Session button, reconnect control, queue/waitlist, or useful recovery CTA.
- The remote display needs an accessible name/instructions and non-color state equivalents.
- Stuck-key advice is helpful, but focus capture/release should be discoverable and deterministic.

### Transferable clean-room architecture

For a public zlOS demo:

```text
visitor portal
  -> capacity and policy check
  -> transactional lease
  -> disposable disk overlay
  -> supervised QEMU process
  -> authenticated one-session WebSocket/VNC route
  -> explicit starting/ready/reconnecting/failed/expiring/ended states
  -> end/release action
  -> reaper plus immutable session receipt
```

Add a queue only if capacity warrants it. Never expose “Start” until the service can account for partial launch, reconnect, cleanup, and data-plane authorization.

---

## 15. `osdev0/freestanding-c-hdrs-gnu`: toolchain provenance is the entire UX

### Actual experience

The repository contains generated freestanding GNU C headers for i686, x86_64, AArch64, LoongArch64, RISC-V 64, and m68k, plus CI generation/commit automation **[S]**. It has no README, end-user app, command shell, desktop, screenshot, icon, theme, font, browser, or interactive configuration surface.

The user is a toolchain maintainer consuming a known header snapshot or inspecting CI output.

### Transferable clean-room idea

Turn invisible build context into visible diagnostics:

- target triple;
- compiler build/version;
- sysroot/header provenance;
- generated-at commit;
- supported language/runtime features;
- ABI flags;
- reproducibility hash.

zlOS’s SDK/About/Diagnostics surfaces should read this metadata from the build artifact rather than embedding hand-written version strings. The repo supplies no visual reference.

---

## 16. `Ellicode/protOS-kernel`: small userspace window manager and PTY-first app model

### Desktop/WM experience

`protowm` is a real userspace window manager **[S/R]**. It:

- maps the framebuffer;
- subscribes to PS/2 mouse and keyboard IPC topics;
- receives window create/refresh events;
- draws gradient titlebars, borders, drop shadows, fonts, and cursor;
- previews drag movement with an XOR outline;
- raises a window after interaction;
- forwards keyboard events;
- launches Terminal with Ctrl+T;
- exits with Ctrl+Q.

It does not implement close, minimize, maximize, resize, taskbar, launcher, app menu, notifications, or settings. The root background is a solid color. `xp.bmp` exists but wallpaper loading is commented out **[A/dead]**.

Assets include cursor BMP, the unused XP wallpaper, and four `.fmp` fonts **[A]**.

### Terminal and shell workflow

The Terminal creates a 500×300 window, opens a PTY through `/dev/ptymx`, launches the Corgi shell, renders a scrolling cell grid, forwards keyboard events, waits for asynchronous readable notifications, and requests window refresh **[S/R]**.

Corgi displays the current directory, implements `cd`, `exit`, and `clear`, then resolves other commands from `/system/bin/<name>` **[S/R]**. This is a clean conceptual split between terminal emulator, pseudo-terminal, shell, and utilities.

### Utility/app inventory

- **`ls`:** optional path, colored file/folder markers, aligned size output, and a friendly empty-directory state.
- **`read`:** character, hexadecimal, decimal, and hex+ASCII file views.
- **`edit`:** subscribes to keyboard events and echoes typed characters until `Q`; it does not load/edit/save a file **[stub]**.
- **`protofetch`:** ASCII logo plus OS, architecture, memory use, and color swatches read from `/dev/about`.
- **`panic`:** explicitly warns, accepts y/n up to five attempts, then invokes a user-requested panic.
- **`terminal`:** graphical PTY terminal.
- **`protowm`:** window manager.
- **`corgi`:** shell/launcher.

### How visible features work underneath

Applications and the WM coordinate through IPC topics and mapped/shared surfaces. Window refresh messages bound the part of the framebuffer that needs repainting. Input travels through event subscriptions; the terminal uses PTY semantics rather than special-casing shell drawing.

### Safety and completeness boundary

- The companion audit identifies raw user-pointer, unbounded/racy IPC, use-after-free, and shared-framebuffer isolation risks **[V-prior]**.
- A userspace WM is not automatically a secure compositor.
- XOR drag outline and no resize/close controls make this an early interaction model.
- The wallpaper asset is dead.
- `edit` is a key-event echo demo, not an editor.
- There is no browser/web engine, media player, image viewer, package UI, or settings app.

### Transferable clean-room ideas

- keep terminal emulator, PTY, shell, and commands separate;
- use event topics for lifecycle/input notifications with typed payloads;
- make partial-surface refresh explicit;
- develop a tiny WM first, but require close/resize/focus ownership before calling it a desktop;
- expose `/dev/about`-style structured facts to a system-info app;
- treat every shared buffer/event boundary as hostile and capability-scoped.

---

## Cross-repository visual and application matrix

Legend: **real** = source-connected workflow; **partial** = meaningful but limited; **mock** = painted/simulated; **asset** = visual evidence only; **none** = absent from the audited product.

| Repository | Desktop/windowing | Files/editing | Media/creative | Games | Browser/web | Ops/admin |
|---|---|---|---|---|---|---|
| Sapphire | none | docs only | none | none | none | protocol/simulation |
| Hyper | none | config | none | none | none | boot config/logs |
| snarkOS | terminal TUI | account/config files | none | none | REST backend only | real, strong |
| Unix history | none | Git corpus | none | none | external host only | provenance queries |
| osdev-projects | none | Markdown table | none | none | external links only | catalogue maintenance |
| MaslOS-2 | real | real file/editor suite | paint/image/audio partial-real | broad real/source set | none | debug/test apps |
| Banana OS | real/source | real file/editor suite | photo viewer | Minesweeper/Solitaire | none | registry/RAM/console |
| AthenX | console visual shell | kernel commands/editors partial | song/logo/demos | 3D demo | none | strong debug shell |
| privilegeOS | none | rescue scripts | none | none | external download tools | real, high-stakes CLI |
| mYOS | text shell | tiny persistent FS | none | none | none | basic commands |
| MineBios | single game view | none | tile art | real single game | none | none |
| vib-OS | real-looking shell | real VFS apps | image viewer, wallpaper | Snake/DOOM | **mock** | fixed settings/about |
| asm.fm | none | WAV artifacts | real offline synthesis | none | none | build/CLI |
| mission-control | web frame around VM | none in portal | remote display | guest-dependent | real portal/noVNC | real lease/admin UX |
| freestanding headers | none | generated headers | none | none | none | CI/provenance |
| protOS | partial-real WM | shell/read/ls; edit mock | none | none | none | protofetch/panic |

## Design-system maturity comparison

| System | Tokens | Components | State model | Assets | Accessibility | Main debt |
|---|---|---|---|---|---|---|
| mission-control web | small semantic CSS palette | cards, buttons, tags, alerts, status | launch/disabled/connected/time warning | logo/web assets | responsive + mobile input; incomplete semantics | CSS/JS class mismatch, recovery/retry, labels |
| vib-OS | hard-coded palette | repeated shell chrome, app-specific drawing | focus/drag/context/loading | wallpaper/icons/font/screenshots | no coherent model found | mock telemetry/browser; title-prefix monolith |
| MaslOS-2 | hard-coded per shell/app | GUI primitives and window model | focus/hover/select/resize/hidden | fonts/images/audio/screenshots | no coherent model found | fixed geometry/launcher, inconsistent eras |
| Banana OS | hard-coded/legacy visual choices | shared `N*` userdraw widgets | window/text selection/events | multi-size icons, cursors, wallpaper, locale/keymaps | text selection exists; system contract absent | screenshot drift, fixed paths, isolation |
| protOS | constants, no token layer | WM/window/terminal primitives | drag/focus/raise/refresh | cursor, fonts, dead wallpaper | no coherent model found | missing window controls and unsafe surfaces |
| snarkOS TUI | Ratatui styles | tabs, tables, gauges/log list | sync/running/log/stop | banner | keyboard path, color dependence | pre-TUI sync, table mismatch, secret/log care |
| AthenX | background/color/font helpers | console/manual/debug screens | command/error/crash | logo/background/screenshots | no coherent model found | auth bypass, kernel coupling, source drift |
| other nine | none/minimal | none relevant | command/config/docs states | sparse/none | not applicable or absent | their product is not a visual desktop |

## Browser/web reality ladder

The word “browser” hides four different layers. None of the focused OS repositories implements the full stack.

```text
1. browser-shaped pixels
2. URL/navigation/session model
3. network fetch: DNS + TCP + TLS + HTTP
4. document engine: parser + DOM + CSS + layout + paint + input
5. isolation, storage, downloads, permissions, accessibility, developer diagnostics
```

- **vib-OS:** layer 1 only. Its address, page, status, links, curl/wget, ping, and Wi-Fi presentation are fixed/simulated.
- **mission-control:** a real host web app and noVNC client. It delivers a remote OS, but it is not a guest OS browser.
- **snarkOS:** REST/API backend; no product browser client.
- **AthenX:** lower-level network protocols/TFTP; no browser.
- **Hyper:** video handoff; no browser.
- **All others:** none.

For zlOS, never call a painted page “Browser.” Name intermediate milestones precisely:

1. `WebView Demo` — static bundled document only;
2. `HTTP Viewer` — real URL/fetch, text response, no HTML layout;
3. `Document Browser` — HTML/CSS subset with navigation;
4. `Browser` — interactive documents with history, storage, TLS policy, isolation, accessibility, and downloads.

Each milestone must display the transport/parser/rendering limits in About/Diagnostics.

## Media, creative, and game lessons

### Audio

- asm.fm provides the clearest deterministic synthesis fixtures.
- MaslOS connects user-facing music/audio experiments to WAV assets and AC97 paths.
- AthenX exposes a song command but not a mature audio product.
- vib-OS visibly says audio disabled while broader source/claims imply more; the UI must report capability, not aspiration.

zlOS should have one shared audio model:

```text
decoder/synth -> mixer -> stream buffer -> device backend
                         -> capture/fixture verifier
                         -> Sound Lab waveform/spectrum UI
```

### Images and paint

- vib-OS has the deepest image-viewer interactions: folder navigation, zoom, rotate, pan, fullscreen.
- MaslOS Paint supplies a complete small creative loop: palette, brush, canvas, open/save.
- Banana Photo Viewer demonstrates file-association launch and size-matched viewing.

Unify these through an image document contract: decoder capabilities, EXIF/orientation, non-destructive view transform, explicit modified state, Save versus Export, and memory-limit/error states.

### Games

- MineBios: smallest complete loop and control teaching.
- MaslOS: broad game portfolio exercises windows, timers, storage, audio, mouse, and framebuffer behavior.
- Banana: Minesweeper/Solitaire exercise nontrivial state and pointer interactions.
- vib-OS: Snake is a shell-embedded game; DOOM exercises a large port but exposes isolation debt.

Games should be test clients of stable APIs, not privileged kernel branches. A game receipt can capture frame timing, input latency, audio underruns, save integrity, and crash isolation.

## Application architecture comparison

| Model | Example | Benefit | Cost/failure mode | zlOS decision |
|---|---|---|---|---|
| Title-string dispatch in compositor/kernel | vib-OS | fast to draw many app-shaped demos | identity collisions, monolith, fake backends, no isolation | reject |
| Separate program modules using desktop IPC | MaslOS-2 | real lifecycle and service boundaries | ad-hoc protocol/versioning/style | retain concept, type/version it |
| Package + registry association | Banana OS | data-driven file opening and resources | legacy paths and unsafe loading | retain manifests/associations, replace loader |
| Userspace WM + shared surfaces + event topics | protOS | clear decomposition and PTY terminal | unsafe buffer/IPC ownership | retain capability-scoped version |
| Kernel command handler | AthenX | diagnostics available early | everything coupled to kernel privilege | keep recovery shell, move apps/services out |
| Host web portal + VM supervisor | mission-control | public reach and disposable sessions | two security domains and lifecycle complexity | separate portal/supervisor/data plane explicitly |

## Clean-room zlOS experience architecture

### 1. System facts and evidence plane

Every visible capability must originate in a typed fact, not copied UI text:

```text
driver/service state
  -> typed capability/status API
  -> app view-model
  -> component state
  -> receipt/test link
```

Examples:

- Wi-Fi icon reads link state, signal, interface, and last error.
- About reads build/runtime facts.
- Display settings reads actual modes and current scanout.
- Browser reports which fetch/parser/layout features are active.
- Audio panel reports codec, mixer, underruns, and muted/disabled reason.

### 2. Visual foundations

Define semantic tokens before app-specific pixels:

- surfaces: desktop, panel, window, elevated, inset;
- text: primary, secondary, disabled, inverse;
- actions: accent, hover, pressed, focus, selected;
- status: success, warning, danger, info, disconnected;
- spacing, radii, border, shadow, type scale, icon sizes;
- density/DPI and minimum touch target;
- high contrast and reduced motion;
- focus ring that survives every theme.

Ship light/dark/high-contrast themes by resolving tokens, not branching app paint code.

### 3. Shell

The shell owns:

- wallpaper/workspaces;
- launcher/search;
- task switching and window list;
- status area backed by system facts;
- notifications;
- lock/session state;
- accessibility/focus model;
- application lifecycle and crash surfaces.

It does not own calculator, browser, editor, or game internals.

### 4. Window/compositor contract

Borrow the proven conceptual overlap from MaslOS/protOS/Banana:

- create/map/close/resize/focus;
- app-owned surface buffers;
- bounded damage regions;
- typed input events;
- compositor-owned chrome and focus;
- capability-scoped clipboard, drag/drop, file picker, notifications;
- protocol version and disconnect semantics;
- memory and event-queue limits;
- screenshot/inspection metadata for tests.

Never allow arbitrary app pointers, unbounded queues, or direct global framebuffer ownership.

### 5. Application manifest and SDK

Each app declares:

- stable ID, name, executable, version;
- icons at deliberate sizes or scalable source;
- file types/actions;
- requested capabilities;
- single/multi-window behavior;
- settings schema;
- accessibility label/description;
- package/license/provenance;
- launch/health receipt tests.

Shared SDK services: app window, menus/toolbars, text input, file picker, VFS document, clipboard, image/audio decode, network request, settings, notifications, logging, and help.

### 6. First-party app set

Build complete workflows, not icon count:

1. **Files:** browse, sort, select, rename, create, move/copy, trash/recover, properties, mount/eject, open-with.
2. **Text:** new/open/edit/dirty state/save/save-as/recovery/search/line-column/encoding errors.
3. **Terminal:** PTY, scrollback, selection/copy, resize, shell launch, process exit/status.
4. **Image:** decode, folder navigation, zoom/pan/rotate/fullscreen, metadata, export.
5. **Paint:** canvas, palette, brush, undo, open/save/export.
6. **Settings:** facts and mutations use the same typed service; unsupported controls are absent or explicitly unavailable.
7. **System Monitor:** Overview/Processes/Devices/Logs/Receipts; use snarkOS’s operational clarity.
8. **Diagnostics:** display, input, storage, network, audio self-tests with evidence.
9. **Browser:** staged honestly according to the reality ladder above.
10. **Game/Sound Lab:** stable API test clients with deterministic fixtures.

### 7. Real browser architecture

```text
browser UI
  -> navigation/history/download model
  -> URL + DNS + connection + TLS + HTTP service
  -> streaming body/cache
  -> HTML tokenizer/tree builder
  -> CSS parser/cascade
  -> style/layout/paint tree
  -> compositor surface/input/accessibility tree
  -> sandboxed content process and permission broker
```

Start with static local documents only if it is named that way. Do not fake address bars, network indicators, response bodies, IPs, or links.

### 8. Remote demonstration architecture

Use mission-control’s product boundary, hardened:

- web portal is outside the guest;
- transactional lease/capacity allocation;
- supervised QEMU and disposable overlay;
- per-session authenticated remote-display route;
- explicit lifecycle state machine;
- reconnection with backoff and real connection observation;
- End Session and release receipt;
- keyboard-capture and mobile controls;
- cleanup/reaper idempotence;
- no secret tokens in ordinary page/log output.

## Priority integration plan

### Phase A — make current truth renderable

1. Define typed system facts for display, input, storage, network, audio, build, and app state.
2. Remove hand-written/fixed About and Settings values.
3. Define evidence labels in developer builds: live, simulated, unsupported, stale, disconnected.
4. Add receipt IDs to crash, driver test, and settings failures.

**Exit gate:** every visible status has a source API and a test that changes it.

### Phase B — visual foundations and shell contract

1. Token file and resolver.
2. Type/icon/spacing/state specification.
3. Window protocol: create, damage, focus, resize, close.
4. Keyboard focus order, pointer capture, high-contrast, scaling.
5. App manifest/launcher and file-association registry.

**Exit gate:** two separately built apps can launch, resize, focus, close, theme, and fail without compositor corruption.

### Phase C — complete document workflows

Build Files, Text, Terminal, Image, and Paint around shared file picker/VFS/clipboard. Require dirty-state and recovery behavior. Avoid a dozen one-screen demos until these are end-to-end.

**Exit gate:** create → edit → save → reboot → reopen works, and failed writes never silently lose the original.

### Phase D — diagnostics, monitor, and media

1. Overview/Logs/Devices/Receipts monitor.
2. Display/Input/Storage/Network/Audio diagnostics.
3. Deterministic audio fixtures and Sound Lab.
4. One game that exercises stable APIs from a normal process.

**Exit gate:** visual states are driven by injected real transitions, and hardware failures produce actionable receipts.

### Phase E — browser in named capability increments

1. local document viewer;
2. HTTP text viewer;
3. HTML/CSS subset renderer;
4. navigation/history/downloads;
5. isolation/storage/permissions/accessibility.

**Exit gate:** no mock transport or fixed content exists behind a production-looking control.

### Phase F — public remote demo

Build the portal only after the guest experience is worth showing. Treat lease, overlay, QEMU, remote data plane, auth, countdown, reconnect, termination, and reap as one tested state machine.

**Exit gate:** forced failure at every transition leaves no leaked lease/process/overlay and tells the visitor what happened.

## False-UI prevention gates

For every visible feature, record:

| Question | Required evidence |
|---|---|
| Does the control accept real input? | interaction test, not screenshot |
| Does state come from the claimed subsystem? | typed dependency trace |
| Can success and failure both be induced? | deterministic test for both |
| Is the app reachable? | build/package/launcher graph |
| Does it survive reboot/relaunch when claimed? | persistence test |
| Is a screenshot current? | commit/build ID embedded in receipt |
| Is the data local or external? | provenance and network boundary |
| Is it safe across process boundaries? | capability/memory/event validation |
| Is it accessible without color/mouse? | keyboard, semantics, contrast checks |
| Is it really a browser/network/audio/setting? | end-to-end backend proof, not painted text |

Specific regression tests suggested by this audit:

- connected/disconnected status changes the actual semantic token and accessible text;
- peer table column/header counts match;
- changing directory changes the prompt;
- values 0–15 round-trip through color settings;
- device target resolver handles SATA, NVMe, MMC, partitions, symlinks, and root media;
- write verification corrupts one byte and must fail;
- launch failure after lease allocation rolls back every resource;
- remote client reconnect waits for connection events rather than constructor success;
- Browser address navigation performs observed DNS/TCP/TLS/HTTP work;
- editor Save As writes the selected path, not a fixed demo file;
- login failure denies access and never falls through;
- dead assets and commented launcher items are excluded from generated product inventory.

## Best ideas worth carrying forward

1. **mission-control:** make public access a capacity-aware, time-bounded product with explicit lifecycle.
2. **snarkOS:** present live system state in one concise Overview/Logs operator console.
3. **MaslOS-2:** build apps as separate modules with IPC, file dialogs, and damaged-region refresh.
4. **Banana OS:** package app metadata, multi-size icons, associations, locale/keymap assets.
5. **vib-OS:** compose a coherent shell with clear hierarchy, media flows, dock, context states, and wallpaper preview—but connect every pixel to reality.
6. **protOS:** preserve the PTY/terminal/shell separation and userspace WM direction.
7. **privilegeOS:** use action preview, typed confirmation, restoration, and receipts for destructive operations; replace its weak verification.
8. **AthenX:** make diagnostics/manual/crash information legible during early boot.
9. **asm.fm:** keep deterministic audio fixtures beside the real mixer/driver.
10. **Unix history:** make provenance and proof explorable instead of burying it in chat/docs.
11. **MineBios:** ship one understandable complete loop before expanding breadth.
12. **Hyper/Sapphire/freestanding headers:** expose boot, device, and toolchain facts as structured data that presentation layers consume.

## Reproducibility trail: highest-value local evidence

All paths below are relative to the named repository inside `/home/roy/Documents/repos/zl-starred-sources/`. They are navigation pointers, not an exhaustive list of every file read.

| Repository directory | Highest-value evidence paths |
|---|---|
| `robotman2412__sapphire-gpu` | `README.md`; `docs/command_summary.md`; `docs/cmd/serial.md`; `docs/compute.md`; `docs/drawing.md`; simulation/build files referenced by the companion audit |
| `UltraOS__Hyper` | `README.md`; `loader/loader.c`; configuration parser/source and sample `hyper.cfg`; video and logging paths |
| `ProvableHQ__snarkOS` | `cli/src/commands/`; account/developer/start command modules; Ratatui display modules; REST/node startup paths; `node/bft/examples/assets/index.html` |
| `dspinellis__unix-history-repo` | Git object/ref/commit graph itself; repository documentation |
| `felipenlunkes__osdev-projects` | `README.md` catalogue tables and outbound links |
| `marceldobehere__MaslOS-2` | `modules/desktop/`; `modules/startMenu/`; `libm/gui/`; `libm/window/`; `libm/wmStuff/`; `programs/`; `images/demo.png`; `images/doom-test.png`; `images/minesweeper.PNG`; `images/wm 2.png` through `wm 7.png`; `images/terminal 1.gif` |
| `alexdboxall__Banana-Operating-System` | `applications/desktop/desktop/Main.cpp`; all `applications/*`; `libraries/userdraw/`; `packages/`; `sysroot/`; application `icon8x8.bmp`, `icon16x16.bmp`, and `icon64x64.bmp` assets |
| `KingVentrix007__AthenX-2.0` | `ui/shell/cmdhandler.c`; `utils/man.c`; credential-validation source; VESA/VGA/image/font paths; `images/Log in screen.png`; `images/Main terminal.png`; `images/File handling.png`; PCI/stack/BOSD images |
| `ktauchathuranga__privilegeos` | `README.md`; `boot.sh`; `getadmin.sh`; `putadmin.sh`; initramfs/banner/alias configuration |
| `yashvi-soni-30__mYOS` | shell/keyboard/VGA source; filesystem and ATA source; README command claims |
| `cppsever__MineBios` | `README.md`; boot/game assembly; QEMU screenshot asset; generated map/image data paths |
| `viralcode__vib-OS` | `kernel/gui/window.c`; `kernel/apps/launcher.c`; VFS/file/image/network paths; `kernel/assets/dock_icons/`; `kernel/assets/icons/`; `kernel/media/bootstrap_images/`; every file under `screenshots/` |
| `whispem__asm.fm` | `README.md`; all 25 root `*.asm` files; generated `bin/` programs |
| `brainboxdotcc__mission-control` | `resources/views/try/index.blade.php`; `resources/views/try/session.blade.php`; `resources/css/app.css`; `resources/js/mission-control/session.js`; `app/Services/LeaseAllocator.php`; `app/Services/VmLauncher.php`; `app/Http/Controllers/SessionApiController.php`; `app/Console/Commands/` |
| `osdev0__freestanding-c-hdrs-gnu` | architecture header directories; `.github/workflows/`; license files |
| `Ellicode__protOS-kernel` | `programs/protowm/`; `programs/terminal/`; `programs/corgi/`; `programs/ls/`; `programs/read/`; `programs/edit/`; `programs/protofetch/`; `programs/panic/`; `initramfs/`; cursor/wallpaper/`.fmp` assets |

## Final conclusion

The focused set does not contain a complete desktop/browser/app blueprint.

- **vib-OS** supplies the clearest visual north star but also the strongest warning: polished pixels can conceal mock browser/network/settings behavior and a monolithic architecture.
- **MaslOS-2**, **Banana OS**, and **protOS** supply better application/process/service shapes, each with security, consistency, or completeness debt.
- **mission-control** supplies the real web and remote-demo experience, including the operational machinery a public demo actually needs.
- **snarkOS** supplies the best live operator UX.
- The remaining repositories contribute bounded lessons in recovery, audio, provenance, catalogue design, boot configuration, diagnostics, and small interactive loops; they should not be mislabeled as desktop donors.

The clean-room synthesis for zlOS is therefore:

> Take vib-OS’s coherence, MaslOS’s app separation, Banana’s manifests/assets, protOS’s PTY/event boundaries, snarkOS’s operational truth, mission-control’s delivery lifecycle, privilegeOS’s explicit risk gates, asm.fm’s deterministic media fixtures, and Unix history’s provenance—then require every visible state to terminate in a real typed subsystem and a receipt.

That is how zlOS gets “all the cool features” without becoming a gallery of controls whose backends do not exist.
