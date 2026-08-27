# Complete application and experience registry

This registry names every current zlOS application/game and every selected
destination product family. Equivalent calculators, editors or file managers
from multiple donor repositories become one stronger zlOS product; their useful
workflows and negative tests are merged into its contract. Third-party ports
remain explicit packages under MP-15 rather than being falsely claimed as
first-party zlOS applications.

Every app inherits MP-10/11: isolated process, admitted signed manifest,
generated registry identity, bounded resources, semantic accessibility,
keyboard operation, scale/localization, portal-only privileged effects,
empty/loading/error/offline/denied/recovery states, crash-loop containment,
autosave where stateful, and deterministic launch/close/update/uninstall proof.

For data apps the minimum real journey is:

```text
empty -> create or open -> edit/action -> save/export/share -> close -> reopen
```

The same journey is tested with malformed input, denied authority, dependency
loss, disk full, cancellation, process crash and service restart. UI text alone
never proves the side effect.

## Current shell and core surfaces — preserve and migrate

| ID | Current name | Primary phase | Destination |
|---|---|---|---|
| CUR-001 | Terminal | MP-12 | PTY-backed process terminal and shell client |
| CUR-002 | System Monitor | MP-12 | typed telemetry, process/service/device controls |
| CUR-003 | About | MP-12 | immutable build, provenance, licenses and health |
| CUR-004 | Menu | MP-12 | shell-owned generated admitted-app menu |
| CUR-005 | Browser | MP-13 | split browser UI/network/content/decoder platform |
| CUR-006 | Settings | MP-12 | schema-driven per-user/system settings and rollback |
| CUR-007 | Run | MP-12 | bounded parser/resolver with explicit launch authority |
| CUR-008 | All Applications | MP-12 | exact generated catalogue; fix Maze/blank-ID false green |
| CUR-009 | Files | MP-12 | VFS/file-portal manager with search/removable/trash |
| CUR-010 | Text Editor | MP-12 | multi-document text editor with undo/recovery/encoding |
| CUR-039 | System | MP-12 | PRESSWORK system pane backed by typed providers |
| CUR-040 | Type | MP-12 | PRESSWORK typography pane with persistent accessible settings |

## Current creative, graphics and inspection apps — preserve and deepen

| ID | Current name | Primary phase | Destination |
|---|---|---|---|
| CUR-011 | Paint | MP-12 | tools, selection, layers, history, formats and recovery |
| CUR-012 | 3D | MP-12 | renderer/scene/camera/asset fixture and creation seed |
| CUR-013 | zlOS animation | MP-12 | motion, branding and visual-regression fixture |
| CUR-014 | Pointer | MP-12 | per-device input/acceleration/latency inspector |
| CUR-015 | Renderer | MP-12 | software/hardware correctness and performance inspector |
| CUR-016 | Framebuffer | MP-12 | authority-mediated capture/scanout inspector |
| CUR-017 | Font Atlas | MP-12 | glyph coverage/shaping/fallback/metrics inspector |
| CUR-018 | Image Viewer | MP-12 | isolated decode, zoom/pan/rotate/metadata/color |
| CUR-019 | Colour Picker | MP-12 | screen-pick portal, palettes, formats and contrast |

## Current utility apps — preserve and deepen

| ID | Current name | Primary phase | Destination |
|---|---|---|---|
| CUR-020 | Clipboard | MP-12 | session MIME offers/history/privacy broker client |
| CUR-021 | Regex Tester | MP-12 | bounded dialect-labelled pattern workbench |
| CUR-022 | Base Converter | MP-12 | exact-width/arbitrary-precision conversion and errors |
| CUR-023 | Text Diff | MP-12 | scalable text/file compare and merge modes |
| CUR-024 | Checksum | MP-12 | streaming hash and verification workflow |
| CUR-025 | Unit Converter | MP-12 | typed units, precision, locale, favorites/history |
| CUR-026 | Sticky Notes | MP-12 | per-user durable private notes and recovery |
| CUR-027 | Keyboard Tester | MP-12 | physical/logical keys, layouts/modifiers/repeat |
| CUR-028 | Benchmark | MP-12 | artifact/workload/backend-bound comparable receipts |
| CUR-029 | Calculator | MP-12 | expression history, precision, keyboard and modes |
| CUR-030 | Clocks & Timers | MP-12 | clocks, stopwatch, countdown and background alarms |

## Current system and diagnostic apps — preserve and migrate

| ID | Current name | Primary phase | Destination |
|---|---|---|---|
| CUR-031 | System Info | MP-12 | consolidated read-only hardware/software inventory |
| CUR-032 | Kernel Log | MP-12 | filtered/redacted persistent event stream |
| CUR-033 | Hex Viewer | MP-12 | huge-file paging/search, read-only default, guarded edit |
| CUR-034 | Console (tty1) | MP-12 | low-level recovery console, explicitly distinct from Terminal |
| CUR-035 | Disk Usage | MP-12 | snapshot/cancellable storage analysis and permissions |
| CUR-036 | Services | MP-12 | real supervisor state, health and authorized actions |
| CUR-037 | Archive Manager | MP-12 | isolated parser and transactional extraction/creation |
| CUR-038 | Network | MP-12 | interfaces, routes, addresses, diagnostics and radios |

The 40 non-game entries above include `All Applications`. Excluding that
separate catalogue surface leaves 39 named non-game implementations; together
with the 24 games below they total the 63 current named implementations.

## All 24 current games — preserve as product and platform probes

| ID | Game | Primary phase | Main platform proof |
|---|---|---|---|
| GAME-001 | Snake | MP-12 | grid, timing, keyboard, deterministic replay |
| GAME-002 | Word Guess | MP-12 | text input, localization and state |
| GAME-003 | Tic-Tac-Toe | MP-12 | pointer/keyboard focus and rules |
| GAME-004 | Nim | MP-12 | deterministic rules and accessibility |
| GAME-005 | Tower of Hanoi | MP-12 | drag/keyboard, animation and undo |
| GAME-006 | Lights Out | MP-12 | grid input, contrast and replay |
| GAME-007 | Connect Four | MP-12 | board, turn state and animations |
| GAME-008 | Maze | MP-12 | repair launch route; generation/pathfinding/input |
| GAME-009 | Tetris | MP-12 | frame pacing, repeat, collision and persistence |
| GAME-010 | Pong | MP-12 | continuous input, collision and audio timing |
| GAME-011 | Breakout | MP-12 | damage, collision, particles and audio |
| GAME-012 | Minesweeper | MP-12 | pointer/keyboard, grids and state restore |
| GAME-013 | 2048 | MP-12 | gestures/keyboard, animation and undo |
| GAME-014 | Conway's Life | MP-12 | large grid, simulation rate and pause |
| GAME-015 | Asteroids | MP-12 | vector rendering, rotation, collision and sound |
| GAME-016 | Invaders | MP-12 | sprites, pacing, collision and state |
| GAME-017 | 15 Puzzle | MP-12 | grid, keyboard and solvable generation |
| GAME-018 | Reversi | MP-12 | board rules and keyboard accessibility |
| GAME-019 | Simon | MP-12 | audio/visual cues, timing and reduced-motion mode |
| GAME-020 | Sokoban | MP-12 | level data, undo and persistence |
| GAME-021 | Flappy | MP-12 | low-latency input, physics and replay |
| GAME-022 | Missile Command | MP-12 | pointer, multiple objects and audio |
| GAME-023 | Blackjack | MP-12 | cards, deterministic randomness and state |
| GAME-024 | Frogger | MP-12 | moving hazards, pacing and collision |

Every game gets deterministic seed/replay, pause/background throttling,
user-scoped save/high score, AudioServer cues, process quotas, crash containment,
keyboard operation and alternatives for color/timing/motion. A rules host test
does not prove a shipped launch route or rendered interaction.

## Shell, onboarding and product-control applications

| ID | App | Primary phase | Complete user outcome |
|---|---|---|---|
| APP-001 | Welcome/Onboarding | MP-14 | resumable account, locale, input, a11y, network, privacy and recovery setup |
| APP-002 | Login/Greeter | MP-07 | accessible authenticated session selection |
| APP-003 | Lock Screen | MP-07 | secure attention, notification privacy and unlock |
| APP-004 | Notification Center | MP-14 | history, grouping, actions, source controls and quiet mode |
| APP-005 | Background Tasks | MP-14 | progress, pause/cancel, budgets and source identity |
| APP-006 | Permissions/Privacy Dashboard | MP-14 | inspect/change live app grants and history |
| APP-007 | User and Account Manager | MP-14 | users, credentials, groups, sessions and recovery |
| APP-008 | Accessibility Center | MP-10 | live preview/control for every assistive feature |
| APP-009 | Appearance Center | MP-10 | theme/type/scale/motion/color preview and rollback |
| APP-010 | Device Manager | MP-14 | provider identity, health, authority, recovery and receipts |
| APP-011 | Software Center | MP-14 | browse/install/update/remove with provenance and permissions |
| APP-012 | Update and Rollback Center | MP-14 | generations, staged update, restart and recovery |
| APP-013 | Crash/Recovery Center | MP-14 | failures, restore, logs, safe mode and support bundle |
| APP-014 | Power/Battery Center | MP-14 | battery/thermal/profile/suspend/shutdown state |
| APP-015 | Removable Media Center | MP-14 | identify, mount, format, eject, repair and privacy |
| APP-016 | Feature/Provenance Viewer | MP-18 | inspect implementation, version, source, license and evidence |

## Files, documents, office and knowledge work

| ID | App | Primary phase | Complete user outcome |
|---|---|---|---|
| APP-020 | Advanced File Manager | MP-14 | tabs, search, preview, copy/move/trash, mounts and conflicts |
| APP-021 | Document Viewer | MP-14 | paginated documents, navigation, search and annotations |
| APP-022 | PDF Viewer | MP-14 | isolated PDF rendering, forms/links/search/print |
| APP-023 | Ebook Reader | MP-14 | library, reflow, bookmarks, annotations and accessibility |
| APP-024 | Word Processor | MP-14 | styled documents, layout, comments, autosave and export |
| APP-025 | Spreadsheet | MP-14 | bounded formulas, cycles, deterministic recalc, charts/import/export |
| APP-026 | Presentation Editor/Player | MP-14 | slides, assets, speaker view, full-screen and export |
| APP-027 | Notes/Knowledge Base | MP-14 | linked notes, tags, search, attachments and sync-ready conflicts |
| APP-028 | Markdown Editor/Preview | MP-14 | source/preview, safe links/assets and export |
| APP-029 | PDF/Document Annotator | MP-14 | highlights, ink, comments, signatures and non-destructive save |
| APP-030 | OCR/Document Capture | MP-14 | scan/image import, language models, correction and export |
| APP-031 | Archive/Compression Workbench | MP-14 | large archive browse/create/extract with safe recovery |
| APP-032 | Search | MP-14 | system/content search respecting permissions and cancellation |
| APP-033 | Dictionary/Thesaurus | MP-14 | local language data, definitions and writing integration |
| APP-034 | Font Manager | MP-14 | preview/install/remove/license/conflict and fallback impact |
| APP-035 | Print Queue and Printer Setup | MP-14 | devices, previews, jobs, status, retry/cancel and privacy |
| APP-036 | Scanner | MP-14 | device, preview, region, quality, capture and document handoff |

## Media, creation and entertainment applications

| ID | App | Primary phase | Complete user outcome |
|---|---|---|---|
| APP-040 | Media Library/Player | MP-14 | library, playlists, metadata, audio/video playback and resume |
| APP-041 | Music Player | MP-14 | albums/playlists/queue/equalizer/metadata |
| APP-042 | Video Player | MP-14 | seek, subtitles, tracks, full-screen and sync |
| APP-043 | Audio Recorder | MP-14 | consent, meters, pause, project recovery and export |
| APP-044 | Audio Mixer/Patchbay | MP-14 | per-app/device routing, levels, mute and latency |
| APP-045 | Music/Synth/Sequencer | MP-14 | deterministic DSP, instruments, timeline and export |
| APP-046 | Camera | MP-14 | permission, preview, capture, settings and gallery handoff |
| APP-047 | Screen Capture | MP-14 | region/window/display consent and protected-surface policy |
| APP-048 | Screen Recorder | MP-14 | capture, audio choice, indicators, pause and recovery |
| APP-049 | Advanced Raster Editor | MP-14 | layers, masks, transforms, brushes, color and project format |
| APP-050 | Vector Graphics Editor | MP-14 | paths/shapes/text/layers/import/export |
| APP-051 | 3D Model/Scene Viewer | MP-14 | assets, camera, materials, lighting and renderer fallback |
| APP-052 | Animation Editor/Player | MP-14 | timeline, keyframes, easing, preview and export |
| APP-053 | Video Editor | MP-14 | timeline, clips, audio, effects, proxy/cache and recovery |
| APP-054 | Image Metadata/Batch Tool | MP-14 | inspect/edit metadata, convert/resize and reversible batch |
| APP-055 | Voice/Podcast Tool | MP-14 | multitrack record/edit, noise processing and export |

## Communication, personal information and internet apps

| ID | App | Primary phase | Complete user outcome |
|---|---|---|---|
| APP-060 | Mail | MP-14 | accounts, folders, compose, attachments, search, offline and security |
| APP-061 | Calendar | MP-14 | calendars, recurrence, invitations, reminders and timezone |
| APP-062 | Contacts | MP-14 | people/groups, import/export, permissions and account sync |
| APP-063 | Tasks/Reminders | MP-14 | lists, due/recurring work, notifications and completion history |
| APP-064 | Messaging/Chat | MP-14 | conversations, attachments, presence, offline and privacy |
| APP-065 | Calls/Conference | MP-14 | audio/video calls, device/permission controls and network recovery |
| APP-066 | RSS/News Reader | MP-14 | feeds, offline cache, reader mode and provenance |
| APP-067 | Weather | MP-14 | consented location/manual places, cache and source/time identity |
| APP-068 | Maps | MP-14 | places, pan/zoom, routes, offline policy and location privacy |
| APP-069 | Download Manager | MP-13 | progress, resume, verification, destination and quarantine |
| APP-070 | Remote Desktop Client | MP-14 | authenticated sessions, display/input/clipboard controls |
| APP-071 | SSH Client | MP-15 | host-key verification, credentials, PTY and sessions |

## Development, debugging and automation applications

| ID | App | Primary phase | Complete user outcome |
|---|---|---|---|
| APP-080 | Source Editor/IDE | MP-15 | projects, language services, build/test/debug and recovery |
| APP-081 | Compiler/Build Monitor | MP-16 | graphs, diagnostics, artifacts, cache and reproducibility |
| APP-082 | Debugger | MP-15 | scoped attach, breakpoints, memory/registers and symbols |
| APP-083 | Profiler | MP-15 | CPU/memory/I/O/frame traces, comparisons and export |
| APP-084 | Log/Trace Viewer | MP-15 | queries, correlations, redaction and saved views |
| APP-085 | Package/Port Development | MP-15 | recipe, patch, build, test, stage and provenance |
| APP-086 | ABI/Provenance Inspector | MP-16 | binaries, sections, symbols, dependencies and source identity |
| APP-087 | Disk/Filesystem Inspector | MP-15 | volumes, blocks, fs structures and safe repair staging |
| APP-088 | Network Packet Inspector | MP-15 | scoped capture, decode, filters and export |
| APP-089 | API/Protocol Workbench | MP-15 | typed request/response, schemas, timing and hostile cases |
| APP-090 | Agent/Automation Center | MP-17 | plans, tools, live grants, children, progress, receipts and cancel |
| APP-091 | Workflow Editor | MP-17 | triggers, typed steps, conditions, budgets, history and rollback |
| APP-092 | Scheduled Tasks | MP-17 | user-owned schedules, next runs, failures and disable/delete |
| APP-093 | Skill/Plugin Manager | MP-17 | trust, permissions, install/update/revoke/uninstall |

## Recovery, demo and support experiences

| ID | App | Primary phase | Complete user outcome |
|---|---|---|---|
| APP-100 | Rescue Desktop | MP-18 | signed read-only-first diagnostics and repair entry |
| APP-101 | Backup/Restore | MP-18 | select, verify, restore, dry run and rollback |
| APP-102 | Boot Generation Manager | MP-18 | current/previous/recovery identity and selection |
| APP-103 | Installation/Deployment UI | MP-18 | target identity, partition preview, copy, verify and recovery |
| APP-104 | Public Demo Portal | MP-18 | authenticated lease, visible expiry, reconnect and cleanup |
| APP-105 | Support Bundle | MP-18 | consent, redaction, preview, export and reproduction ID |
| APP-106 | Hardware Test Center | MP-19 | exact device qualification, failures and receipts |
| APP-107 | Accessibility Tour | MP-10 | learn/test screen reader, magnifier, captions and input aids |

## Third-party port/product policy

The research preserves exact external breadth: 307 Astral recipes, 128 banan
ports, 223 Serenity utilities and all other local/external package names. The
destination does not reimplement each third-party program from scratch. MP-15
creates a reproducible port catalogue with, for each selected package:

- immutable source/license/patch/dependency record;
- target ABI and required compatibility features;
- successful build, package, install and real launch/workload evidence;
- sandbox/permission profile, update/remove and known-defect record;
- explicit label as third-party, imported compatibility app or first-party zlOS
  application.

Initial compatibility workload families include a POSIX shell/core utilities,
C/C++ build tools, archive/compression, text editor, scripting runtimes, SDL
games/demos, terminal programs, image/audio/video libraries, X11/Wayland bridge
experiments and selected browser/toolchain dependencies. Recipe existence alone
never satisfies availability.

## Registry completion gate

For every current and destination entry, the generated application registry
must agree on stable ID, exact name, icon, manifest, architecture/ABI,
constructor/entry point, launch route, shipped package/image membership,
readiness nonce, close/teardown and retirement alias. One planted defect in each
field must independently force the aggregate gate nonzero.
