# Code map — where everything actually lives

Written because the top-level listing is misleading. There is no `apps/`
directory, no `browser/` directory, and no `desktop/` directory. The
desktop, the eight apps, and the web browser are all real and all
checked in — they are just not where the directory names suggest.

Read this before asking "where is X".

## The one-line version

`kernel/kernel.zl` **is** the desktop. Every app is a function in it,
written in zl. The C files in `kernel/` are the platform underneath it —
window manager, UI toolkit, drivers, network stack — that `kernel.zl`
calls into.

## Nine repos, one codebase

`~/Documents/repos/` has nine `zl*` directories. Eight of them are the
same repository:

| Directory | Remote | Branch |
|---|---|---|
| `zl-main` | `RoyX4/zl-linux` | `main` — integration branch |
| `zl-linux` | `RoyX4/zl-linux` | `desktop/overnight-compositor` |
| `zl-apps` | `RoyX4/zl-linux` | `desktop/apps-in-windows` |
| `zl-browser` | `RoyX4/zl-linux` | `desktop/browser` |
| `zl-exec` | `RoyX4/zl-linux` | `desktop/exec-track` |
| `zl-feel` | `RoyX4/zl-linux` | `desktop/feel-and-control` |
| `zl-system` | `RoyX4/zl-linux` | `desktop/system-track` |
| `zl-value16` | `RoyX4/zl-linux` | `lang/value-16` |
| `zl` | `RoyX4/zl` | the original Windows repo |

They are **parallel track checkouts**, not separate projects. Summing
their line counts inflates the total roughly 8×.

The old Windows `zl` repo is effectively absorbed: 139 of its 147 shared
paths are byte-identical to the Linux port. Only ~863 lines are genuinely
Windows-only (`os_win.c` plus top-level scratch files like `t1.zl`).

## Where the desktop is

### `kernel/kernel.zl` — the desktop and all its apps

One zl file, 2,121 nonblank lines on `main` and up to 2,823 on
`desktop/overnight-compositor`. It declares and drives every app:

| Constant | Window title |
|---|---|
| `APP_SHELL` | zl shell |
| `APP_MONITOR` | System Monitor |
| `APP_ABOUT` | About |
| `APP_SNAKE` | Snake |
| `APP_MENU` | start menu (modal) |
| `APP_BROWSER` | Browser |
| `APP_SETTINGS` | Settings |
| `APP_RUN` | run |
| `APP_PAINT` | Paint |
| `APP_CUBE` | 3D |
| `APP_ANIM` | zlOS |
| `APP_MOUSE` | Pointer |
| `APP_EDIT` | Editor |

Each app is a branch in the dispatch on window id, plus a `wm_open()`
call at startup. There is no per-app file and no per-app directory. If
you are looking for Paint, it is a function in `kernel.zl`.

### `kernel/out.c` — generated, ignore it

`out.c` is the C that `compilel`/`compile` emits from `kernel.zl`. It is
**not tracked by git** and must never be counted as source or edited by
hand. Searching the tree for app symbols hits `out.c` first and makes the
apps look like C. They are not.

### The C platform under the desktop

These are hand-written and tracked. Sizes are nonblank lines, taken from
the largest version across the tracks:

| File | Lines | What |
|---|---:|---|
| `kernel/wm.c` | 1,017 | window manager — stacking, focus, drag, resize, snap |
| `kernel/ui.c` | 457 | immediate-mode UI toolkit: buttons, sliders, toggles, lists, scroll |
| `kernel/term.c` | 259 | terminal widget |
| `kernel/wmglue.c` | 117 | the bridge exposing `wm_*` to zl code |

`ui.c` is a real immediate-mode toolkit — `ui_begin`/`ui_button`/
`ui_slider`/`ui_toggle`/`ui_list_row`/`ui_scroll_begin`. That is the
whole reason `kernel.zl` can stay small.

### The browser — `desktop/browser` branch only

Not on `main`. Roughly 3,700 nonblank lines across:

| File | Lines |
|---|---:|
| `kernel/tcp.c` | 740 |
| `kernel/virtio_net.c` | 690 |
| `kernel/browser.c` | 612 |
| `kernel/html.c` | 518 |
| `kernel/layout.c` | 443 |
| `kernel/net.c` | 443 |
| `kernel/http.c` | 260 |

A TCP/IP stack, an HTML parser, a layout engine and a renderer. Checking
out `main` and grepping for it finds nothing.

## Why every track adds the same files

`git merge-base` says `main` is the common ancestor of all seven tracks.
Yet every track shows `kernel/wm.c`, `ui.c`, `ui.h`, `term.c`,
`wmglue.c`, `font_prop.h` and the `hosttest/*` harnesses as **added**
files.

That means the desktop foundation exists on all seven tracks and on none
of them is it merged back to `main`. Each track then evolved those shared
files independently — `wm.c` is 741 lines on three tracks, 918 on
system, 996 on feel, 1,017 on compositor. They will conflict at merge.
That is expected; it is what `docs/INTEGRATION-PLAN.md` is for.

Practical consequence: **`main` has no windowed desktop at all.** Its
`kernel.zl` contains zero `wm_*` calls and zero `APP_` constants, and its
`kernel/build.sh` never compiles `wm.c`, `ui.c` or `term.c`. `main`
builds the pre-windowing kernel — framebuffer, drivers, console. Windows
and apps exist only on the tracks. To see the desktop run, check out a
track.

## Size

Union of all branches, cloc `code` (blanks and comments excluded):

| | files | code |
|---|---:|---:|
| `.zl` — written in zl itself | 138 | 34,522 |
| `.c` / `.h` / `.S` — bootstrap compiler, runtime, kernel | 104 | 53,005 |
| **total** | **242** | **87,527** |

By area:

| | code |
|---|---:|
| `kernel/` — drivers, compositor, browser, fs, exec | 46,405 |
| `stdlib/` | 17,770 |
| root — lexer, parser, four backends | 10,421 |
| `tests/` | 6,481 |
| `examples/` | 5,326 |
| `freestanding/` | 1,086 |

`main` alone is 71,569. Add ~863 for the Windows-only remnant in `zl` and
the honest grand total is **~88,400**.

Two caveats on that number. It is a *projection*: for each path it takes
the largest version across the branches, so where two tracks edited
different regions of the same file a real merge lands higher. And it
excludes `kernel/font_prop.inc`, ~14k lines of generated font bitmap
data — that is data, not code.

## How to count it yourself

`cloc --vcs=git` alone is wrong here in two ways: it does not know the
`.zl` extension, and run per-directory it counts the eight checkouts
eight times. Count one checkout, force the language, and take the union
across branches by path:

```bash
cd ~/Documents/repos/zl-main
git ls-files '*.zl' | xargs cloc --quiet --force-lang=C
git ls-files '*.c' '*.h' '*.S' | xargs cloc --quiet
```

Generated output (`out.c`, `outf.c`, `out.ll`, `examples_out/`,
`native_out/`) is untracked, so `git ls-files` already excludes it. Never
count it.
