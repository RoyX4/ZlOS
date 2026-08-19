# refrender — making the desktop reference actually render

`ds.html` is the north star for the zlOS desktop: a 4338-line, 300 KB mockup of
the whole environment — top bar, wallpaper, desktop icons, windows, dock, and 53
apps. Until now it could only be **read**, never **run**, because line 6 says:

```html
<script src="./support.js"></script>
```

and `support.js` was never delivered. It is the loader for a proprietary
React-based template runtime the mockup is written against — `<x-dc>` templates,
`{{ }}` bindings, `<sc-for>`, `<sc-if>`, and a `DCLogic` base class. Without it
the page loads to a blank body.

`support.js` in this directory is a reimplementation of that runtime. It is not
the original and makes no claim to be; it implements the subset of the runtime
that `ds.html` actually exercises, which was enumerated from the file rather
than guessed.

## Run it

```bash
cd kernel/refrender
npm install                  # once: pulls playwright-core

node render-ref.mjs                             # out/reference-1280x800.png
node render-ref.mjs --app term                  # out/app-term.png
node render-ref.mjs --state '{"locked":true}'   # any state patch you like
node render-ref.mjs --shot lockscreen --state '{"locked":true}'
node render-ref.mjs --list-apps                 # the 53 app ids
./shots.sh                                      # the whole standard set
./shots.sh term files mon                       # just those
```

`out/` is gitignored. Nothing here builds or boots the kernel.

### Requirements

- `node` (uses `playwright-core` from npm, installed locally into `node_modules/`)
- `/usr/bin/chromium` — override with `CHROMIUM_PATH`
- network, for the Google Fonts stylesheet (see **Fonts** below)

**Not python-playwright.** `python3-playwright` is installed on this box but is
unusable: it hardcodes its node driver to `/usr/share/nodejs/playwright/cli.js`,
which no installed package provides, and the packaged `node-playwright` is 1.38
against a 1.55 python binding. `playwright-core` from npm driving the system
chromium is the combination that works.

React 18.3.1 UMD is **vendored** into `vendor/`. Nothing is fetched from a CDN at
render time; the only network request is the font stylesheet, which is the
reference's own.

## What the shots are for

Each PNG is a ground-truth frame of the reference at exactly the size the
reference declares for itself (`$preview: 1280x800`, `ds.html:974`). They exist
to be diffed against zlOS's own framebuffer output as the clone progresses.

Shots are **reproducible**: the clock is frozen to a fixed instant, `Math.random`
is replaced with a seeded xorshift, and infinite CSS animations (the `zpulse`
dock indicator) are pinned to their first keyframe rather than sampled at
whatever phase the screenshot happened to land on. Two runs of the same shot are
byte-identical — verified, not assumed:

```
$ node render-ref.mjs --shot rep1 && node render-ref.mjs --shot rep2
$ sha256sum out/rep1.png out/rep2.png
dba231aef503d35d5cfbedc140588b828c515d3d6d3ecc57a5734650e52ae1dc  out/rep1.png
dba231aef503d35d5cfbedc140588b828c515d3d6d3ecc57a5734650e52ae1dc  out/rep2.png
```

Pass `--live` to get the real clock and unseeded randomness back.

**No shot waits on a wall clock for correctness.** Entry animations are waited
out by polling `document.getAnimations()`, per this repo's rule that a gate must
never be timing-sensitive. `--settle` is a small extra margin, not the
mechanism. (Note the poll counts rounds rather than checking `Date.now()` — the
clock is frozen, so a deadline check there would never fire.)

## What the shim implements

Everything below was enumerated from `ds.html` lines 9–973 by counting actual
occurrences, not by reading the mockup's prose.

| Feature | Occurrences | Status |
|---|---|---|
| `{{ path }}` in text nodes | — | yes |
| `{{ path }}` as a whole attribute value | — | yes |
| `<sc-for list="{{ xs }}" as="w">` | 84 | yes, nested and shadowing |
| `<sc-if value="{{ cond }}">` | 68 | yes |
| `style="{{ obj }}"` — camelCase JS object | 194 dynamic | yes, handed straight to React |
| `style="literal:css"` | 365 literal | yes, parsed to a React style object |
| `style-hover="..."` | 3 | yes, real generated `.dchN:hover` rule |
| `ref="{{ r }}"` | 21 | yes — ref objects *and* callback refs |
| `onClick` 81, `onPointerDown` 85, `onChange` 11, `onDoubleClick` 7, `onContextMenu` 7, `onKeyDown` 7, `onKeyUp` 1, `onScroll` 1, `onWheel` 1 | 201 | yes |
| `value="{{ x }}"` on `<input>` / `<textarea>` | 11 | yes |
| `<helmet>` with `<link>` and `<style>` | 1 | yes, hoisted into `<head>` |
| `<canvas>`, `<svg>`, `<polyline>`, `<line>` | 10 | yes, as ordinary elements |
| `hint-placeholder-count` / `hint-placeholder-val` | 152 | ignored, as intended |
| `data-role`, `data-app`, `title`, `placeholder`, `spellcheck`, `type`, `min`/`max`/`step`, `width`/`height`, `viewBox`, `points`, `stroke`… | — | yes |

`onInput` never appears as a template attribute — it is the *name* of a handler
in `renderVals` bound to an `onChange` attribute (`ds.html:3483`). The shim maps
it anyway, along with every other DOM event React knows, so an unanticipated
`onFoo` binds as a handler instead of becoming a string attribute.

`DCLogic` extends `React.Component`; its `render()` instantiates the template
with the object returned by the subclass's `renderVals()` (`ds.html:3037`).
`React.createRef()`, `this.state` as a class field, `setState` in both object and
updater-function forms with the optional callback, `forceUpdate()`,
`componentDidMount`, `componentDidUpdate(prev)` and `componentWillUnmount` all
work because they *are* React's — the shim adds nothing there.

`accent` (`#b8e838`) and `shadow` (`6`) are read from the `data-props`
declaration on `ds.html:974` and passed as props. The component happens to
duplicate both as state defaults, so nothing currently reads the props.

### Details that were not obvious

- **Attribute case.** The HTML parser lowercases attributes on HTML elements, so
  the template's `onPointerDown` arrives as `onpointerdown`. There is an explicit
  map back to React prop names. (Inside `<svg>`, foreign-content parsing already
  preserves `viewBox`, so both spellings are handled.)
- **Whitespace.** Template whitespace is collapsed with Babel's exact
  `cleanJSXElementLiteralChild` algorithm. The original runtime was a React
  template compiler, and if whitespace does not collapse the way JSX collapses
  it, every flex row in the mockup gains stray text nodes and the layout drifts.
- **Literal styles stay inline** rather than becoming generated classes. The
  helmet stylesheet contains `input[type=range]{…}` at specificity (0,1,1),
  which would beat a single generated class and change the rendering.
- **`style-hover` rules use `!important`.** Three of the four sites sit on
  elements whose `style` is a dynamic object; an inline style would otherwise
  beat the `:hover` class rule. Nothing is hovered in a screenshot, so this only
  matters for interactive use.
- **Boot is skipped by state, not by clicking.** The `skipBoot` handler
  (`ds.html:4316`) also opens a "Read-only file system" dialog, which would sit
  in the middle of every shot. `render-ref.mjs` sets `booting:false` directly.

## What the shim deliberately does NOT implement

- **Arbitrary expressions in `{{ }}`.** Only identifiers and dotted paths
  (`w.title`, `i.onClick`, `r.numStyle`) resolve. This is not a limitation in
  practice: all **594** distinct expressions in the template were checked and
  **every one** is a plain dotted path. Anything else is dropped and logged as a
  shim warning rather than failing silently.
- **Partial interpolation in an attribute** (`style="color:{{ c }}"`). Checked:
  zero occurrences. Logged as a warning if one ever appears.
- **The `hint-placeholder-*` design-time preview counts.** Intentionally ignored;
  they exist to give the mockup's own editor something to draw before data
  exists.
- **The editor/`$preview` side of the runtime** — prop editors, the colour and
  range widgets described in `data-props`. Irrelevant to rendering.
- **Comments in the template.** Dropped, like the zl lexer drops them.

## The weakest link, stated up front

This is a reimplementation. The original `support.js` does not exist, so
**nothing here can be proven identical to it** — only self-consistent. The
evidence that it is right is: every one of the 594 distinct template expressions
is a plain dotted path, so nothing is being approximated; zero uninterpolated
`{{` survive into the DOM in any of the 54 shots; and all 54 render as coherent,
correctly-fonted, correctly-laid-out frames.

The single most likely place for a **silent** divergence is **whitespace
collapsing**. The rule implemented is Babel's JSX rule, chosen because the
original was evidently a React template compiler — but that is an inference, not
a measurement, and it is unfalsifiable without the original. If a later pixel
diff shows text sitting a few pixels off inside a flex row and nothing else
explains it, suspect this first: `cleanText()` in `support.js`.

Second most likely: `style-hover` using `!important`. It is correct for the
screenshots (nothing is hovered) but is a stronger rule than the original
probably used.

## What is missing from ds.html itself, not from the shim

This is the important caveat and it is not a shim defect. `ds.html` dynamically
imports **seven sibling ES modules that were never delivered either**:

| module | `ds.html` | what is lost |
|---|---|---|
| `zl-fs.js` | 1399 | OPFS-backed `/tmp`; `fsMode` reports `unavailable` |
| `zl-tar.js` | 1416 | Archive Manager has no tar backend |
| `zl-games.js` | 1420 | **all 24 games render an empty canvas** |
| `zl-wasm.js` | 1426 | the zl compiler backend; `make` produces no diagnostics |
| `zl-raster.js` | 1430 | the software rasteriser |
| `zl-vt.js` | 1439 | **Console (tty1) renders an empty canvas** |
| `zl-gl.js` | 2017 | **Renderer (3D) renders an empty canvas** |

plus a worker, `new Worker('zl-kernel.js')` (`ds.html:1452`), which supplies the
System Monitor's real scheduler data. Every one of these is inside a
`try`/`catch`, so the desktop boots normally and the *windows*, *title bars*,
*toolbars*, *sidebars* and *chrome* of all these apps render correctly — only
their canvas content is blank, and the kernel log says so (`gl: no webgl
backend`, `tty1: emulator failed to load`, and so on).

Concretely, the apps whose **canvas** is blank in every shot are: `render`
(Renderer), `fb` (Framebuffer), `tty` (Console), `font` (Font Atlas), `img`
(Image Viewer), and all 24 `g_*` games. Everything else — Terminal, Files,
System Monitor, Text Editor, Kernel Log, Hex Viewer, Calculator, Disk Usage,
Services, Archive Manager, Network, Clocks, System Info, Settings, and all 10
`u_*` utilities — is DOM and renders in full.

Writing those seven modules is a separate job from writing the shim. Nothing in
this directory can recover them.

## Fonts

The reference asks for **Public Sans** and **Roboto Mono** from Google Fonts, via
the `<link>` inside its `<helmet>`. Both currently load, and `render-ref.mjs`
checks it explicitly with `document.fonts.check()` after `document.fonts.ready`:

```
fonts ok: Public Sans, Roboto Mono
```

**If they ever fail to load, the script prints `*** WEB FONTS DID NOT LOAD ***`
to stderr and exits 3.** Do not ignore that and do not diff against a PNG
produced by a failed run — a fallback face changes every text metric in the
image, and every later comparison against zlOS would be measuring the wrong
thing. The fonts are fetched from the network on every run and are deliberately
*not* vendored: pinning a stale copy of a Google font would be a quieter version
of the same failure.

## Console noise that is expected

Two categories of console error appear on every run and neither indicates a
problem:

1. **Seven `404`s** for the `zl-*.js` modules above, plus a failed `zl-kernel.js`
   worker. Expected; those files do not exist.
2. **Four SVG attribute errors** —
   `<line> attribute y1: Expected length, "{{ g }}"` and similar. These come from
   the browser parsing the *raw template* in `<x-dc>` before any script runs: the
   parser sees the literal `{{ g }}` in an SVG attribute and complains. The shim
   compiles that same markup correctly and removes the raw `<x-dc>` subtree
   before mounting, so nothing is wrong with the output. They cannot be
   suppressed without editing `ds.html`.

`render-ref.mjs` also asserts that **zero** uninterpolated `{{` survive into the
live DOM, which is the check that would actually catch a template feature being
silently skipped.

## Files

| file | what |
|---|---|
| `ds.html` | the reference, byte-identical copy of the original |
| `support.js` | the shim — the `x-dc` runtime reimplementation |
| `render-ref.mjs` | Playwright driver: one shot, or `--batch` for all of them |
| `shots.sh` | the standard set into `out/` |
| `vendor/react.js`, `vendor/react-dom.js` | React 18.3.1 UMD, production |
| `out/` | generated PNGs — gitignored |

`ds.html` is vendored here so the render is self-contained and versioned:
`support.js` must sit next to the document it is loaded from, and the original
lives outside the repo. Note there is a **second** copy of the same file at
`docs/design/ds-reference.html`, committed by a different track; they are
identical today, and if one is ever edited the other should be re-synced or
dropped.
