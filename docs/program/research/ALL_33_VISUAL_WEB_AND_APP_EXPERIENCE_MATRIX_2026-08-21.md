# All 33 starred repositories: visual, web, and application-experience matrix

Date: 2026-08-21

Scope: exactly the 33 repositories in the immutable source manifest, normalized
for visual design, desktop/shell interaction, browser/web reality, application
workflows, standout ideas, and truth boundaries.

Detailed evidence remains in:

- [`MATURE_OS_VISUAL_WEB_AND_APP_EXPERIENCE_DEEP_DIVE_2026-08-21.md`](MATURE_OS_VISUAL_WEB_AND_APP_EXPERIENCE_DEEP_DIVE_2026-08-21.md)
- [`ARCHITECTURE_OS_VISUAL_WEB_AND_APP_EXPERIENCE_DEEP_DIVE_2026-08-21.md`](ARCHITECTURE_OS_VISUAL_WEB_AND_APP_EXPERIENCE_DEEP_DIVE_2026-08-21.md)
- [`FOCUSED_REPOSITORIES_VISUAL_WEB_AND_APP_EXPERIENCE_DEEP_DIVE_2026-08-21.md`](FOCUSED_REPOSITORIES_VISUAL_WEB_AND_APP_EXPERIENCE_DEEP_DIVE_2026-08-21.md)
- [`ZLOS_CURRENT_VISUAL_WEB_AND_APP_EXPERIENCE_BASELINE_2026-08-21.md`](ZLOS_CURRENT_VISUAL_WEB_AND_APP_EXPERIENCE_BASELINE_2026-08-21.md)

## Evidence vocabulary

- `S`: substantive local source or manifest.
- `R`: connected to active build, boot, launcher, command, route, or package.
- `T`: relevant test/automation exists; not necessarily run in this pass.
- `A`: presentation asset or checked-in screenshot; not runtime proof.
- `V-prior`: separately recorded earlier build/runtime evidence.
- `3P`: third-party package, port, framework, asset, or compatibility product.
- `fetched`: pinned external project absent from the checkout.
- `external`: product or userland lives in another repository/system.
- `stub/mock`: visible or callable surface whose advertised behavior is fixed,
  simulated, incomplete, or absent.
- `dead`: implementation/launcher/asset is outside the active graph.
- `F`: concrete evidence contradicts the broad claim.

## Exact normalized ledger

| # | Repository | Visual/shell reality | Browser/web reality | App/workflow reality | Best clean-room idea | Required correction |
|---:|---|---|---|---|---|---|
| 1 | `Bananymous/banan-os` | `S/R/A`: compact native WindowServer, rounded translucent dark shell, fuzzy launcher, CPU/battery/audio/time panel | Links/Lynx are `3P`; optional X bridge, no native graphical engine | 62 native programs plus servers and 128 ports; terminal, image viewer, monitor, admin tools and tests; no native graphical file manager | PATH-scanning fallback launcher; pan/zoom viewer with interpolation; tiny coherent shell | Panel has no evidenced window list; design constants are duplicated; weak a11y/scale; credential flaws invalidate authority-sensitive UX |
| 2 | `CorruptedByCPU/Cyjon` | `S/R`: Fern-Night dark alpha desktop, Roboto/Mono, taskbar/menu/clock, drag/focus/minimize/Alt-Tab | None | Console/shell plus `cat`, `ls`, `free`, `ps`, `uptime`; removed apps remain referenced | Proportional taskbar allocation; Alt-anywhere drag; declarative app JSON; process-owned shared windows | Debug and Task Manager launch removed/unbuilt targets; no resize/themes/a11y/localization; unsafe IPC/shared buffers |
| 3 | `IanNorris/Brook` | `S/R/A`: native rescue desktop and Wayland bridge; repeated dark palette rather than central theme; taskbar shows FPS/GPU state | NetSurf framebuffer and Ladybird integration are `3P`, not Brook-native web platform | Small native Files/Edit/Console/Calculator/Player plus compatibility apps and probes | Keep native recovery tools while exposing a compatibility lane; make graphics truth visible | Three font/theme systems drift; host wrapper masks failures; compositor authority/user-pointer defects; package presence is not launch proof |
| 4 | `KingVentrix007/AthenX-2.0` | `S/R/A`: branded framebuffer login/terminal, diagnostics screens, small demos | No browser; UDP/TFTP teaching stack only | Shell/TUI/editor/manuals and small user command set | Discoverable diagnostic shell and readable failure presentation | Login accepts all credentials; “3D” is a demo; multitasking/syscalls and networking are incomplete; visual success cannot imply isolation |
| 5 | `LemonOSProject/LemonOS` | `S/R/A`: strongest compact native desktop; semantic 13-role theme, orange/charcoal identity, launcher/taskbar, shared GUI | Native HTTPS downloader only; no HTML/CSS/JS browser | 22 installed apps, nine utilities and system tools including Files, editor, media, DeviceManager, Monitor, SysInfo and Welcome; theme/config support is not a Settings app | Small coherent shell/theme/metadata/widget/IPC slice; active-task click minimizes | `classic.json` schema drift; wrong minimize-hover hit rectangle; no mature notifications/a11y/scale; broken credentials/process authority |
| 6 | `Mathewnd/Astral` | Desktop is overwhelmingly upstream Xorg/FVWM/NSCDE/GTK/Qt `3P` | WebKitGTK MiniBrowser and Links are `3P` recipes | Five local admin/trace tools; 307 recipes supply browsers, editors, media, Wine and games | Broad compatibility target map and distribution integration | Do not attribute upstream visual/app behavior to Astral; init and recipes lack strong runtime/failure evidence |
| 7 | `Meulengracht/MollenOS` | Polished launcher imagery belongs to external `vali-userspace`; no local design system | None locally | Generated services, immutable app packs and two native CLI utilities | Schema-generated service/package boundaries under a future GUI | External screenshot is not local UX; GUI/app claims must remain external; package/runtime cleanup defects persist |
| 8 | `ProvableHQ/snarkOS` | `S/R`: richest operator CLI/TUI; overview/log tabs, progress, peer/role/account views | REST/metrics endpoints, not a consumer browser | 21 role-oriented packages and operational commands | Secret-safe account flow; role-specific lifecycle, checkpoints and terminal observability | No desktop/app UX; peer-table mismatch and REST auth/rate defects; operator UI must reflect real bounded state |
| 9 | `SerenityOS/serenity` | `S/R/A/T`: deepest semantic themes, widgets, WindowServer, shell, settings, icons, workspaces and portals | Only full native browser+engine platform in mature group: Browser, LibWeb/LibJS, helpers, request/lookup/decoder/storage tooling | 51 graphical apps, 29 services, 13 games, 223 utilities; creation, documents, media, admin, accessibility | Shared app platform, declarative layouts, service ownership, parser isolation, browser architecture | Scale is far beyond direct adoption; retro appearance is a skin, not the lesson; multiarch/runtime evidence must remain lane-specific |
| 10 | `UltraOS/Hyper` | Config-file and log/error UX; no interactive boot picker/theme/menu | None | Boot configuration and assertion payloads, not app ecosystem | Typed boot configuration with deterministic default and executable scenario receipts | Multiple boot entries do not equal interactive menu; no keyboard chooser/countdown/background; runtime lanes differ |
| 11 | `UnmappedStack/TacOS` | `S/R/A`: blue wallpaper, pale-blue title bars, red close, drag/focus/close; keyboard-driven pointer | None | GTerm, Info, Doom, DIM terminal editor and small CLI | Keyboard-pointer fallback and tiny client/server window contract | Full redraw, no resize/minimize/themes/a11y; loader/syscall defects make UI boundary unsafe |
| 12 | `alexdboxall/Banana-Operating-System` | `S/R/A`: desktop, file cabinet, console, per-size icons, locale/keymap assets, file associations | No substantive browser; networking incomplete | Regedit, editors, utilities, games, photo/text/ZIP and package/CAB/installer flows | File associations, icon-size vocabulary and package/distribution UX | Current screenshot/runtime proof weak; ring-0 user-selected modules; license restrictive; many systems incomplete |
| 13 | `brainboxdotcc/mission-control` | `S/R/T`: real responsive web portal with capacity/status/deadline messaging and embedded noVNC | The portal is the web product; it brokers QEMU/noVNC rather than guest browsing | Visitor launch/queue/session/timeout plus admin lifecycle | Public-demo expectation setting, capacity-aware lease UI and disposable-session shape | VNC/WebSocket plane lacks equivalent auth; no explicit stop/reconnect; allocation/launch rollback and PID/death proof incomplete; a11y gaps |
| 14 | `byteduck/duckOS` | `S/R/A`: small retro Pond/Sandbar desktop, five file-backed themes, widgets, app bundles, menu/status bar | None | Files, Viewer, Editor, Terminal, monitor and utilities driven by `.app` metadata | One app-bundle model powers launcher, file types, icons and `open` | Panel has no evidenced window list; theme schema fallback weak; minimal a11y/localization; shared-memory authority flaw |
| 15 | `chittios/chitti` | `S/R/A`: warm editorial agent console, strong semantic palette, six themes, wallpaper/transparency/font/status config, panes/tabs/modals/toasts | Large local staged HTML/CSS/JS browser with storage, CORS, images, partial grid/SVG/MathML/WASM and explicit limits | Exact 48 manifest packages spanning agents, productivity, media, games, system and web | Capability-aware agent workspace; configurable theme breadth; inspectable browser; agent-readable links/actions | Kernel-resident orchestrator/browser enlarge TCB; app surface DSL lacks rich text/semantics; several browser APIs are no-op/stub; package/signature claims need correction |
| 16 | `cppsever/MineBios` | `S/R/V-prior`: one coherent mode-13h tile game | None | One bounded game loop with movement, collision, camera and interaction | Immediate teachable controls and bounded viewport workload | It is not an OS app suite or Minecraft clone; links are external/expired; no assertion-driven runtime gate |
| 17 | `dspinellis/unix-history-repo` | Git history is the interface; no native visual explorer | External Git/web tooling only | Research workflows through log/blame/diff/refs over complete history | Make feature lineage and provenance explorable in zlOS | Historical presence is not current functionality; dataset is not reconstruction pipeline; rights vary by lineage |
| 18 | `felipenlunkes/osdev-projects` | Markdown tables and external links | External sites only | Curated project catalogue, no implementation | Convert curation into typed, searchable, validated catalogue UX | No schema/filter/site/test/link validation; membership cannot be feature evidence |
| 19 | `hhuOS/hhuOS` | `S/R/A`: Lunar widget theme and rich visual workload lab; no desktop shell/window manager | None | Framebuffer/audio probes, emulators, 2D/3D games and graphics ports | Hardware/library proof ladder and game/media workload suite | No launcher/taskbar/Files/editor/browser; workload breadth is not app-platform completion; unsafe privileged operations |
| 20 | `ierwarf/rustos` | `S/R/A/T`: strongest local aesthetic—Aurora/glass, procedural starfield/icons, Wayland compositor, render scheduling | No browser app; glyph/icon references are not an engine | Shell, WayClick and qualification/diagnostic entries; very thin consumer catalogue | Render/compositor discipline, coherent tokens, async background and honest evidence | Theme/a11y/localization APIs absent; desktop product breadth is not present; runtime failures remain evidence ceiling |
| 21 | `jezze/fudge` | `S/R`: flat dark/rose desktop, Terminus font, mailbox-carried declarative widgets, draggable/right-resizable windows | Raw HTTP client/server only, not a browser | Seven GUI programs plus CLI/tools/demos/emulator | Declarative UI messages over typed service/mailbox boundaries | Hardcoded palette, almost no icons/themes/motion/a11y; storage/driver stubs; protocol authorization incomplete |
| 22 | `ktauchathuranga/privilegeos` | Colored rescue shell and guided high-stakes scripts | None | Drive inspect/mount/backup/restore/write/QEMU operations | Preview -> authorize -> act -> verify -> restore as rescue UX | “Verify” checks size, not bytes; brittle system-drive filtering, passwordless root and unpinned inputs; must become signed read-only-default rescue |
| 23 | `marceldobehere/MaslOS-2` | `S/R/A`: broad windowed desktop, launcher/taskbar/dialog/file chooser, dirty rectangles | No substantive browser platform | Roughly 30 apps/modules: explorer, terminals, editors, paint, calc, music, games/demos | App processes, conversation/stdin/stdout IPC, coherent desktop composition | Screenshots span eras; some launcher entries commented/unverified; unsafe loader/filesystem/C++ boundaries; AGPL means ideas only |
| 24 | `osdev0/freestanding-c-hdrs-gnu` | Generated headers/CI metadata are the only UX | None | Toolchain generation and compatibility workflow | Treat compiler input provenance and environment identity as a product surface | No end-user app/visual feature; generated headers do not prove ABI/runtime; transitive inputs/atomics vary |
| 25 | `rayanmargham/NyauxKC` | `S/R`: Limine-branded boot wallpaper then Flanterm logs | None | Kernel self-test/status only, no userspace apps | Carry branding into boot/recovery without mistaking it for desktop | Wallpaper is bootloader-owned; no shell/product UI; PCI/archive defects; no visual-system claim |
| 26 | `robotman2412/sapphire-gpu` | Documentation/protocol is the interface; no GUI/assets/screenshots | None | Simulation/developer workflow only | Machine-readable GPU capabilities/status feeding a Display Diagnostics app | No connected rasterizer/shader/compute product; empty/broken docs links; protocol claims are not visible GPU behavior |
| 27 | `skift-org/skift` | Local Strata compositor adapter and rich historical design lineage; current Karm/Hideo UI mostly `fetched` | Vaev advertised/fetched, absent locally | 15 launcher declarations locally; implementations external | Immutable/reactive UI mental model, damage loop, concise launcher taxonomy | Do not count fetched apps/browser as local; screenshots are dated lineage; channel transfer behavior is unsafe |
| 28 | `staycool1374-Ger/nexios` | `S/R/V-prior-build`: black framebuffer terminal, fixed status bar, Mandelbrot demo | None | Large command/diagnostic shell and core utilities, not desktop apps | Clear diagnostic presentation and explicit resource/status output | No desktop/window system/browser; status and claims can exceed runtime proof; capability/revoke defects are negative inputs |
| 29 | `viralcode/vib-OS` | `S/R/A/V-prior-build`: best-looking focused shell—macOS-inspired desktop, dock, windows, menus, media/file surfaces | Browser, curl/wget, ping and network indicators are painted/mock/simulated | Many embedded EL1 apps and disconnected userspace tree | Shell hierarchy, dock, context menus and cohesive media/file visual composition | Most important false-UI warning: visuals wildly exceed real networking/storage/process isolation; monolithic EL1 and timeout-as-success CI |
| 30 | `whispem/asm.fm` | No GUI; CLI creates audio artifacts | None | 25 deterministic offline synth/effect WAV programs | Golden DSP fixtures, A/B examples and reproducible sound as app-test assets | No player, waveform UI, real-time mixer/device/clock/backpressure; offline WAV is not AudioServer |
| 31 | `yashvi-soni-30/mYOS` | `S/R`: tiny VGA shell with prompt/history/color and persistent toy FS | None | help/clear/about/color/echo/reboot/halt/ls/touch/write/cat/rm/mkdir/cd/pwd | Small end-to-end command loop whose whole state is understandable | Prompt/path/scoping/scroll/color/storage behavior is weak; ATA can hang; no processes/tests/license |
| 32 | `zinnia-os/zinnia` | Local early framebuffer console and display/input substrate; attractive README desktop is external bootstrap | External only | Kernel plus arbitrary external `/init`; no local first-party app suite | Honest separation of kernel enablement from userland product | Do not attribute external wallpaper/shell/apps to Zinnia; local UI is console only; loader malformed input can panic |
| 33 | `Ellicode/protOS-kernel` | `S/R`: compact userspace WM/terminal with small chrome and shared buffers; unused wallpaper | None | shell, protofetch, `ls`, `read`, `panic`; editor unfinished | WM/event IPC + PTY terminal + process-owned surface refresh vertical slice | Raw pointers, racy IPC/UAF/ambient shared memory/global framebuffer; tiny chrome, no themes/a11y, full build fails |

## Design-system comparison

| Need | Strongest positive inputs | Useful partial inputs | zlOS decision |
|---|---|---|---|
| Coherent local identity | current zlOS, RustOS, ChittiOS | LemonOS, vib-OS visual composition | Keep lime/graphite/grid zlOS identity; do not import another skin |
| Semantic tokens/themes | SerenityOS, ChittiOS | LemonOS, duckOS | Version color/type/spacing/radius/elevation/motion/state schema; atomic preview/apply/rollback |
| Compact native components | LemonOS, duckOS, Fudge | current zlOS's 68-pattern toolkit | One process-safe semantic component library, not app-local colors/widgets |
| Typography/assets | SerenityOS, ChittiOS | RustOS, LemonOS | Scalable shaping/fallback plus semantic icon names and optical states |
| Motion | current zlOS has the clearest named curves | RustOS effects, Skift damage tick | Preserve measured curves; add interruption, reduced motion and frame receipts |
| Accessibility | SerenityOS is the clearest partial native input | almost every other system is a gap | Semantic tree, keyboard-complete components, scale/contrast/motion/input alternatives from the start |
| App metadata | duckOS, SerenityOS | LemonOS, Cyjon, ChittiOS | One signed manifest drives install, catalogue, icons, file types, permissions, launch and diagnostics |

## Desktop and interaction comparison

| Experience | Best source | Secondary lessons | Rejected pattern |
|---|---|---|---|
| Complete desktop navigation | SerenityOS | LemonOS compactness, RustOS visuals, current zlOS workspaces | Static screenshots or external desktops counted as local |
| Recovery/fallback shell | Brook | banan PATH launcher, current zlOS Terminal | Desktop failure with no simple local tool path |
| Window/compositor seam | RustOS, SerenityOS | Fudge declarative mailboxes, Cyjon/protOS surface IPC | Client scanout/global framebuffer/input ownership |
| Search/launch | ChittiOS command browser; banan fallback scan | Serenity app metadata, duck bundles | Dense ID ranges and dead launcher entries |
| Operator status | snarkOS | Brook FPS/GPU truth, current zlOS telemetry | UI counters disconnected from real provider state |
| High-stakes recovery | privilegeOS workflow shape | mission-control lifecycle | Success based on size/text rather than byte/process/effect verification |
| Remote demo | mission-control | Hyper assertion payloads, snarkOS operations | Control token without VNC data-plane auth or transactional rollback |

## Browser and web reality ladder

| Level | Repositories | Exact meaning |
|---:|---|---|
| 0 | most repositories | no browser or web product |
| 1 | LemonOS, Fudge, snarkOS | downloader/raw HTTP/REST/operator endpoints; not a browser |
| 2 | Brook, Astral, banan-os | third-party browser packages/compatibility; project does not own engine platform |
| 3 | mission-control | real browser-hosted remote-demo portal; web app, not guest browser engine |
| 4 | current zlOS | real bounded native browser: verified TLS, HTML/CSS/layout/PNG, flex/grid, external styles, bounded inline JS; trusted monolith and intentionally incomplete platform |
| 5 | ChittiOS | broad local staged browser and agent-readable controls; kernel-resident and explicitly partial/no-op in several APIs |
| 6 | SerenityOS | deepest native browser/web platform: chrome, engine, JS, content/network/decoder helpers, storage/settings/tools; still not universal compatibility or perfect isolation |

The zlOS target is not to leap from level 4 to “Chrome.” It is to preserve level
4 capability while first adopting level-6 process/service boundaries, then grow
compatibility in versioned, tested increments.

## Application-family donors

| Family | Primary donor ideas | Secondary ideas | zlOS result |
|---|---|---|---|
| Files/open/save | SerenityOS portals and apps; duckOS bundle/filetype seam | LemonOS, Brook, MaslOS | Handle-based Files/Editor/Viewer/Archive with safe-save and isolated decoders |
| Settings/personal state | SerenityOS domains; Chitti theme breadth | LemonOS theme, current zlOS settings | Schema-versioned per-user settings with live preview/revert and provider truth |
| System/device/admin | SerenityOS suite; snarkOS operator state | Brook GPU/FPS truth, current zlOS diagnostics | Read-only typed views plus separately authorized actions and recovery receipts |
| Creative/media | SerenityOS creation suite; hhuOS workload lab | Chitti packages, MaslOS apps, asm.fm goldens | Versioned projects, decoder/audio services, deterministic media proof |
| Games | current zlOS 24 games; hhuOS and Serenity workloads | Chitti, MaslOS, MineBios | Deterministic replay, timers/input/render/audio/save/crash/quota conformance suite |
| Agent/automation | ChittiOS visible plan/capability/app grammar | snarkOS roles, current zlOS future agents | Ordinary-process orchestrator, visible live grants, child cleanup and independent receipts |
| Productivity | SerenityOS document/spreadsheet/presentation apps | Chitti document packages | Explicit editor, sheet, presentation and PDF contracts; not an unbounded office clone |
| Print/scan | broad platform lesson, not a strong complete donor here | device/service patterns | Owned spool jobs, preview/cancel/privacy/device failure and scanner capture consent |
| Accessibility/localization | SerenityOS partial ecosystem | Chitti multilingual assets, TacOS keyboard pointer | System semantic tree, AT services, scalable shaping/locales and alternative inputs |

## Cool features worth carrying forward

1. ChittiOS's agent-aware searchable console, configurable status placement and
   progressive browser tools—but in user processes.
2. RustOS's procedural Aurora background, render scheduling and coherent glass
   effects—but adapted to zlOS's lime/grid identity and reduced-motion policy.
3. SerenityOS's one-platform breadth: themes, layouts, portals, creation apps,
   services and browser helpers.
4. LemonOS's tiny coherent theme/shell/task interaction model.
5. duckOS's single `.app` metadata source for launcher, file types, icons and
   opening.
6. banan-os's database-free PATH fallback launcher and image interpolation
   toggle.
7. Brook's compositor FPS/GPU-path truth in ordinary shell UI.
8. Fudge's declarative mailbox UI messages.
9. TacOS's keyboard-controlled pointer as a recovery/accessibility fallback.
10. Cyjon's proportional taskbar and Alt-anywhere drag, after removing dead
    entries and hardening ownership.
11. snarkOS's operator overview/log/account flows.
12. mission-control's public-session expectation/capacity UI, hardened across
    both control and VNC planes.
13. Unix-history provenance as an explorable Feature Receipts application.
14. asm.fm's deterministic audio examples as AudioServer golden fixtures.
15. MineBios's instantly understandable bounded game loop as an input/render
    smoke workload.

## Repeated false-UI and product hazards

- external/fetched/third-party desktop or browser presented as local;
- screenshot vintage omitted, so old or aspirational appearance becomes “now”;
- launcher/menu/catalogue item targets a missing, removed or unshipped program;
- visible success text without independent file/byte/process/device effect;
- telemetry or toggle not connected to the actual provider;
- mock browser/network/media panels painted over stubs;
- application count inferred from manifests, recipes or numeric ranges;
- status/permission icon derived from request text rather than verified state;
- app/server/process separation implied by labels while everything shares kernel
  authority;
- theme supports color but not typography, metrics, scale, contrast, motion or
  accessibility;
- sophisticated visuals conceal missing keyboard, focus, locale and recovery;
- build or timeout success promoted to interactive runtime proof.

Every one becomes a mutation or failure-injection case in the contract backlog.

## Direct zlOS consequences

1. Preserve the current graphite/lime/grid identity and design-token oracle;
   borrow structures and behaviors, not another repository's skin.
2. Repair the app registry before adding more tiles. Exact existence, package,
   route, ready nonce and teardown must all fail independently.
3. Complete semantic component states, scalable typography, accessibility and
   localization before multiplying app-specific UI.
4. Move surfaces, input, clipboard, files, settings, notifications, media and
   privileged operations behind typed handles and restartable services.
5. Migrate current diagnostics and file/editor apps first; they are the best
   service-contract probes. Then migrate all utilities and 24 games.
6. Split the current browser into chrome, navigation/network, content, decoder
   and origin-storage processes before pursuing broad compatibility.
7. Add onboarding, Device Manager, Software/Update, Crash/Recovery and
   Accessibility Center as platform-defining apps.
8. Then add explicit productivity, communications, media, print/scan,
   development, agent, rescue and public-demo families.
9. Certify workflows, not names: launch -> ready -> action -> durable effect ->
   close -> reopen/recover -> zero leaked authority.
10. Keep screenshot, host, QEMU and native physical evidence separate forever.

## Exact coverage receipt

The ledger contains 33 unique repositories and follows the source manifest's
exact order. The three deep dives retain every meaningful first-party app,
service, game, utility, visual asset family, browser route and product-boundary
finding, including the complete 48-package Chitti catalog, Serenity's active
graphical suite, all mature first-party target appendices, and the focused
repositories' exact app/utility/media inventories.

This matrix is exhaustive at the meaningful product/workflow level of the
pinned snapshots. It does not claim every vendored helper, historical branch,
external dependency, image asset or individual widget function was run.

## Weakest link

The pass is source-and-asset-heavy. Screenshots prove appearance at some point;
they do not prove current build reachability or interaction. No report here
upgrades an unrun desktop, browser, app or physical display path to runtime
proof. That limit is why the destination requires artifact-bound interaction,
side-effect, recovery and physical receipts instead of a visual maturity score.
