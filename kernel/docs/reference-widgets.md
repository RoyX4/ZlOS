# Reference widget specification

Extracted from `docs/design/ds-reference.html` (300 KB, 4338 lines, vendored in
this repo; byte-identical to `~/Downloads/ds.html`). This is the specification
the shared zlOS widget toolkit is built from, and the document the ~53 app
agents build against.

**Every number here came out of the file.** Line numbers are cited on every
record. Where a frequency is quoted, the command that produced it is shown.
Where two instances of the same shape disagree, both are given and the
disagreement is called out — §20 is the index of every one of them.

## 0. How the reference is structured, and what that means for you

| Region | Lines | What lives there |
|---|---|---|
| `<style>` in `<helmet>` | 12–32 | The only real stylesheet: scrollbar, `input[type=range]`, `::selection`, keyframes |
| `<x-dc>` template | 35–972 | Structure + **literal inline styles** for anything that never changes |
| `renderVals()` | 3037–4320 | Returns the props object holding **every computed style**. Repeated style objects here are the widgets. |
| `utilVals()` | 1773–~1950 | The 10 utility apps' fields, buttons and rows |
| `icon()` | 2050–2059 | SVG icon factory: `viewBox 0 0 20 20`, `strokeWidth` default `1.7`, `strokeLinecap:'square'`, `strokeLinejoin:'miter'` |

Two consequences a toolkit author must plan for:

1. **A widget's geometry is usually split across two places.** The container is
   an inline style in the template; the items inside it are a computed style in
   `renderVals()`. Neither half is complete on its own. Every record below gives
   both line numbers.
2. **There are only five hover rules in the entire 4338-line file.**
   ```
   $ grep -n 'style-hover\|:hover' docs/design/ds-reference.html
   22:  a:hover{color:#d8f588}
   28:  *::-webkit-scrollbar-thumb:hover{background:#474b50;background-clip:padding-box}
   84:  ...style-hover="background:#ff6a50;color:#0c0f05"        (window close button)
   766: ...style-hover="background:rgba(255,255,255,.09)"        (overview app tile)
   944: ...style-hover="background:#22262b"                      (dialog OK button)
   ```
   Everything else has **no hover state at all** — the reference distinguishes
   states by *selection*, not by pointer. `cursor:'default'` appears on almost
   every clickable thing. Do not invent hover states and call them "from the
   reference"; if the toolkit adds them, that is a deliberate extension.

---

## 1. Global tokens

### 1.1 The eight font sizes

```
$ grep -o "fontSize:'[0-9.]*px'\|font-size:[0-9.]*px" docs/design/ds-reference.html \
  | sed "s/.*[:']\([0-9.]*\)px.*/\1/" | sort -n | uniq -c | sort -k2 -n
```

| px | uses | Who uses it | Family |
|---|---|---|---|
| **9** | 9 | stat-card **keys** (uppercase, `.07em`), window **subtitle**, palette `kind` tag, overview thumbnail icon, boot tagline | mixed |
| **9.5** | 24 | badge pills, service unit `state`, `heap` indicator, `camText`, `fbNote`, `fontNote`, `ttyGeom`, `svcSummary`, note timestamps, mount flags, boot subtitle | mostly Roboto Mono |
| **10** | 26 | hex status bar, file-row `monoStyle`, icon-button glyph sizing, `imgZoom`, `fontZooms`, `hexPages`, service action buttons, `noteClearStyle`, `timerRows.xStyle`, calendar head, palette hint | mostly Roboto Mono |
| **10.5** | 35 | file-row `numStyle`, menu-item shortcut key, editor + log status bars, `laps`, `imgTabs`, `meshBtns`, calc tape, utility buttons, desktop icon labels, sysinfo tag, sidebar headings (Files), overview thumbnail titles, syscall trace, archive `monoStyle` | mixed |
| **11** | 42 | column headers (`pcolGrid`/`fcolGrid`), kernel-log body, status bars (Files/Monitor), most pill buttons, `dimStyle`, `netToggle`/`netPing`, `timerBtns`, sysinfo k/v grid, props modal grid, overview labels, calendar day | mixed |
| **11.5** | 29 | segmented-control items (Monitor, Log), `killStyle`, `spinBtn`, `clkBtn`/`swBtn`, `ovCats`, crumbs, stat-card **values**, service unit name, hex body, `netRows`, sysinfo group name, util input + row values, `svcTabs` | mixed |
| **12** | 28 | **window title**, terminal body, editor body, menu items, context-menu items, list-row text, sidebar tree rows, toggle-row label region, dialog body, tray toggles, settings slider label | mixed |
| **12.5** | 12 | **root default** (line 35), palette items, calc keys, settings nav rows, toggle-switch row label, modal headers, dialog OK, disk mount name, lock password, agg panel title | mixed |

Outliers, each used exactly as many times as listed — these are *not* part of
the scale and each belongs to one specific widget:

| px | uses | Widget | Line |
|---|---|---|---|
| 13 | 5 | overview search input, calendar month, overview empty text, lock date, palette input | 750, 842, 785, 951, 879 |
| 14 | 4 | dock/apps-grid glyph size, dialog title, icon-button glyph | 3411, 3424, 941 |
| 15 | 2 | settings page title, boot brand wordmark | 706, 964 |
| 16 | 1 | System Info product name (`zl 0.1`) | 675 |
| 19 | 1 | lock-screen brand tile glyph | 952 |
| 21 | 1 | clock zone time | 4072 |
| 25 | 1 | calculator result readout | 3735 |
| 26 | 1 | stopwatch readout | 4075 |
| 62 | 1 | lock-screen clock | 950 |

### 1.2 Font families

Exactly two, both loaded from Google Fonts at line 11:
`Public Sans` weights 400/500/600/700, `Roboto Mono` weights 400/500.

- **`'Public Sans', system-ui, sans-serif`** — the root font (line 35). All prose,
  titles, labels, list names, settings, dialogs.
- **`'Roboto Mono', monospace`** — everything tabular or technical: terminal,
  editor buffer + gutter, kernel log, hex dump, syscall trace, all numeric
  columns (`numStyle`, `monoStyle`, `stStyle`, `cpuStyle`, `cmdStyle`), stat-card
  values, badges, flags, clock/stopwatch/timer readouts, `camText`, `fbNote`,
  window **subtitle**, calculator input + keypad + tape, service unit names,
  utility inputs and values, drag ghost, boot screen, palette `kind` tag and
  `hint`, `hexPages`, `imgTabs`, `imgZoom`, `fontZooms`, `timerBtns`,
  `netToggle`/`netPing`, dock item (`fontFamily` set but glyphs are SVG).

Rule of thumb that holds throughout: **if the value is a number, a path, a
register, a hash, a state word or a time, it is Roboto Mono.**

### 1.3 Font weights

```
$ grep -o "fontWeight:[^,}]*" ... ; grep -o "font-weight:[^;\"]*" ...
```
`700` × 25 · `500` × 5 · `600` × 2 · two conditional `700 : 400`.

- `600` — **window title** (3111) and the top-bar "Activities" label (3375). Only these two.
- `500` — clock button (3438), clock zone time (4072), stopwatch (4075), timer row (4101), calculator result (3735).
- `700` — everything else that is emphasised.
- `400` — implicit default (never written).

### 1.4 Letter-spacing

| Value | Uses | Where |
|---|---|---|
| `.07em` | 5 | stat-card keys (uppercase 9px), palette `kind` tag |
| `.08em` | 4 | section headings (Clock, Network "ping output") |
| `.05em` | 3 | Files sidebar headings, Disk "Largest files" |
| `.02em` | 3 | window subtitle, clock button, boot wordmark |
| `.04em` | 2 | `heapStyle`, service unit `state` |
| `-.02em` | 2 | clock zone time, stopwatch |
| `.17em` `.16em` | 1 each | boot subtitle, "click to skip" |
| `.09em` `.06em` `.03em` | 1 each | Network "Interfaces" heading, palette kind, hex bytes |
| `.01em` `.005em` | 1 each | Activities label, window title |
| `-.04em` `-.01em` | 1 each | lock clock, System Info product name |

Pattern: **positive tracking scales with smallness and uppercase-ness; negative
tracking only on display-size numerals.**

### 1.5 Border radius

```
$ grep -o "borderRadius:'[0-9.]*px'\|border-radius:[0-9.]*px" docs/design/ds-reference.html \
  | sed "s/.*[:']\([0-9.]*\)px.*/\1/" | sort -n | uniq -c | sort -rn
```

| px | uses | Primary role |
|---|---|---|
| **11** | 26 | the standard pill button; also popover list rows, tray toggles, segmented-control **container** |
| **12** | 18 | top-bar items, desktop-icon tiles, `ovCats`, `clkBtn`/`swBtn`, slider thumb, toast/notify icon circles, knob |
| **9** | 15 | list-row corner, menu items, segmented-control **item**, calc keys, small badges, mount flag, util input |
| **14** | 14 | modals, toast, icon buttons (22×22), toggle-switch track, search field, overview app tile, snap preview |
| **13** | 13 | cards, window menu / context menu, dock item, overview app icon, ham/close button family is 16 (see below) |
| **10** | 12 | sidebar tree rows, crumbs, editor find input, rename input, util button, drag ghost, net ping panel |
| **16** | 10 | window frame, window control buttons, top bar, dock, notification panel, sysinfo mark |
| **7** | 7 | meter-bar track and fill, scrollbar thumb, slider track, terminal jump line, tab top corners |
| **8** | 5 | badge pills, service action buttons, overview thumbnail head, sysinfo group icon |
| **5** | 5 | status dots, workspace pips, disk bar segments |
| **4** | 5 | timer/disk bar fills, notification badge |
| **6** | 2 | spark fill, workspace chips |
| **17** | 2 | overview search input, `markStyle` |
| **20** | 1 | lock-screen brand tile |
| **15** | 1 | command palette |

**11px is the most common radius, 26 uses.** But note the corner-radius scale is
*not* a clean geometric ladder — 9/10/11/12/13/14 all appear in double digits and
often on shapes of the same size. See §20.3.

### 1.6 Spacing

Every `padding` and `gap` component token, by frequency:

```
$ { grep -o "padding:'[^']*'" f | sed "s/padding:'//;s/'//"
    grep -o "padding:[0-9][^;\"]*" f | sed "s/padding://"
    grep -o "gap:'[^']*'" f | sed "s/gap:'//;s/'//"
    grep -o "[^-a-zA-Z]gap:[0-9][^;\"]*" f | sed "s/.*gap://"; } \
  | tr ' ' '\n' | grep -E '^[0-9.]+px$|^0$' | sort | uniq -c | sort -rn
```

| Token | Count |
|---|---|
| `0` | 64 |
| `9px` | 50 |
| `12px` | 38 |
| `8px` | 35 |
| `6px` | 34 |
| `7px` | 31 |
| `10px` | 30 |
| `5px` | 27 |
| `13px` | 27 |
| `11px` | 26 |
| `14px` | 24 |
| `4px` | 23 |
| `3px` | 19 |
| `2px` | 18 |
| `1px` | 10 |
| `16px` | 8 |
| `15px` | 7 |
| `18px` | 4 |
| `26px` | 3 |
| 17/20/22/30/34/52/82 | 1 each |

Gaps alone (`gap` only, both syntaxes): `8px` 15 · `7px` 15 · `9px` 14 · `5px` 11
· `2px` 11 · `1px` 9 · `10px` 9 · `6px` 8 · `14px` 8 · `3px` 7 · `12px` 7 ·
`13px` 3 · `16px` 2.

**There is no 4/8 grid.** The reference uses every integer from 1 to 16. The
densest band is 5–14 px. A toolkit that snaps to a 4-px grid will not reproduce
this file; a toolkit that keeps arbitrary integers will. Recommend keeping the
literals.

### 1.7 Colours (raw literals — mapping to `kernel/src/graphics/ui/design.h` tokens is a later step)

Hex, by frequency:

```
$ grep -oiE '#[0-9a-f]{6}\b' docs/design/ds-reference.html | tr 'A-F' 'a-f' | sort | uniq -c | sort -rn
```

| Hex | n | Role |
|---|---|---|
| `#dfe2e5` | 60 | default body text |
| `#9ba0a6` | 54 | secondary / dimmed text |
| `#0b0d0f` | 47 | hairline border, terminal + editor background, stat-strip gutter |
| `#eef0f2` | 43 | emphasised / selected text |
| `#74797f` | 40 | tertiary text, shortcut keys, mono labels |
| `#5c6167` | 37 | quaternary text, section headings, comments |
| `#14171a` | 32 | card / sidebar / status-bar surface |
| `#1c2024` | 30 | card border, input border, divider |
| `#22262b` | 26 | popover border, scrollbar thumb, neutral button fill |
| `#101215` | 18 | window body, toolbar surface, thumbnail body |
| `#474b50` | 16 | disabled / very dim, gutter numbers |
| `#b8e838` | 15 | **default accent** (`ACC`) |
| `#07080a` | 12 | deepest surface (canvas backgrounds, calc input) |
| `#090a0c` | 11 | inset surface (segmented control, search field, meter track) |
| `#cdf25a` | 10 | accent text / links / sort arrows |
| `#b9bec4` | 10 | icon-button glyph, calendar day |
| `#f5b93c` | 6 | **`WARN`** |
| `#c4c9cf` | 5 | desktop-icon label, sidebar-nav text |
| `#171a1e` | 5 | focused title bar, tab strip, find bar |
| `#0d0f12` | 5 | calculator body, util input, syscall panel, timer track |
| `#ff6a50` | 4 | **`BAD`** |
| `#a9e34b` | 4 | **`OK`** |
| `#dff29a` | 3 | selected sidebar row text |
| `#d8f588` | 3 | hex bytes, near edges, link hover |
| `#c7ce9a` | 3 | directory-row text |
| `#8b939e` | 3 | inactive underline tab, archive column head |
| `#2a2f35` | 3 | notification panel border, archive hero glyph |
| `#1a1d21` | 3 | System Info card border, net ping panel border |
| `#ff9c88` | 2 | destructive button text |
| `#a693ff` | 2 | syntax keyword, `.zl` file icon |
| `#8f7bff` | 2 | accent option |
| `#8b9096` | 2 | unfocused window title, dir icon |
| `#7d848c` | 2 | inactive tree icon |
| `#57b6ff` `#4ce0b3` | 2 | accent options |
| `#161a1e` | 2 | disk bar free segment |
| `#0f1114` | 2 | clock / sysinfo header band |
| `#0d1005` | 2 | dark ink (utility button, drag ghost) |
| `#0c1005` | 2 | dark ink (`INK`, font atlas) |
| `#0b0d10` | 2 | game canvas background |
| `#070809` | 2 | page + desktop background |
| `#ffffff` | 1 | light ink (`INK` alternate) |
| `#ffd166` | 1 | restart-count badge text |
| `#ffb3a4` | 1 | `killStyle` text |
| `#e86ec4` | 1 | accent option |
| `#0c0f05` | 1 | close-button hover ink (line 84) |
| `#131518` | 1 | **unfocused title bar** (3110) |
| `#060708` | 1 | boot screen background (960) |
| `#232830` | 1 | **utility-input border** (1776) — appears nowhere else |
| `#87b0f0` | 1 | syscall name |
| `#68762f` `#3a4218` `#59662a` `#4e555e` | 1 each | wireframe depth ramp, thumbnail glyph |

Named semantic constants, declared at line 3046 and re-declared at 1774:
```js
const OK = '#a9e34b', BAD = '#ff6a50', WARN = '#f5b93c';
```
`TC` (terminal colour map, 3050): `{txt:'#dfe2e5', dim:'#9ba0a6', b:'#cdf25a', g:OK, y:WARN, red:BAD, warn:WARN}`.

`ACC` is `state.accent`, default `#b8e838` (line 974 props default).
`INK` (3039–3045) is **computed**: the WCAG relative luminance of `ACC` is
compared against black-ish `0.0034` and white `1`, and `INK` becomes `#0c1005`
or `#ffffff` — whichever has the better contrast ratio against the accent. Any
widget painting text *on* the accent should use `INK`, not a literal.

rgba, by frequency (top of the list):

| rgba | n | Role |
|---|---|---|
| `rgba(255,255,255,.07)` | 14 | neutral button fill |
| `rgba(255,255,255,.08)` | 8 | icon-button fill |
| `rgba(255,255,255,.06)` | 8 | secondary chip fill, tray-toggle active |
| `rgba(255,255,255,.09)` | 6 | overview input, lock input, app-tile hover |
| `rgba(255,255,255,.05)` | 6 | calc key fill, lap chip, dock border |
| `rgba(184,232,56,.15)` | 5 | **selection tint** (list row, tree row, settings nav) |
| `rgba(184,232,56,.1)` | 4 | accent-tinted badge background |
| `rgba(255,255,255,.1)` | 3 | crumb active, overview app icon |
| `rgba(184,232,56,.16)` | 3 | desktop-icon selected tile, CPU sparkline area fill |
| `rgba(255,255,255,.085)` | 2 | dock item idle |
| `rgba(212,105,90,.16)` | 2 | destructive fill (calc `C`, stopwatch stop) |
| `rgba(169,227,75,.14)` | 2 | `OK`-tinted fill (mount flag rw, `start` action) |
| `rgba(245,185,60,.1)` | 2 | `WARN`-tinted fill (sysinfo badge) |
| `rgba(18,21,24,.72)` | 2 | desktop-icon tile idle |

### 1.8 Icon sizes

`icon(name, size, color, weight)` at 2050; `viewBox 0 0 20 20`, default size 18
(never used — every call passes a size), default `strokeWidth` 1.7.

```
$ grep -noE "this\.icon\([^)]*\)" docs/design/ds-reference.html | grep -v '^205'
```

| px | Where | Line |
|---|---|---|
| **8** | sort arrows in both column headers | 3228, 3272 |
| **9** | tab close ✕ (Terminal + Editor), tray caret | 3494, 3620, 3667 |
| **11** | find-bar close | 3666 |
| **12** | find-bar up/down carets | 3666 |
| **13** | window close ✕, overview thumbnail icon, sysinfo group icon, tray net + speaker, notification bell, Files device icon | 3130, 3310, 4142, 3451, 3453, 3952, 3561 |
| **14** | hamburger menu, editor save/dot, Files tree icon, notification-row icon | 3118, 3129, 3550, 3966 |
| **15** | Files back/forward chevrons, Terminal new-tab `+`, **file-row icon (list view)**, tray toggle icons | 3125–3128, 3184, 3460–3461 |
| **16** | dock apps-grid button, toast icon | 3667, 4228 |
| **17** | quick-settings brand mark | 3456 |
| **20** | **dock item**, trash desktop icon | 3421, 3999 |
| **21** | overview file-hit icon | 3306 |
| **22** | desktop icon | 3978 |
| **23** | overview app icon | 3302 |
| **27** | file icon, **icon view** (`weight 1.5`) | 3185 |
| **30** | overview thumbnail big glyph (`weight 1.4`), boot mark (`weight 1.5`) | 3310, 4312 |
| **34** | System Info brand mark (`weight 1.5`) | 4110 |
| **40** | Archive Manager empty-state hero (`weight 1.4`) | 4158 |

**Stroke weight drops as the icon grows**: 1.7 default up to 27 px, then 1.5 at
27/30/34 and 1.4 at 30/40. `markStyle`'s 17 px icon goes the other way — weight
`2` (3456). Only `brand`, `archive` and `mark` ever pass a weight.

---

## 2. Window frame

**Lines** template 72–74, 744–745; computed 3101–3108.
**Used by** every one of the 53 apps.

| Property | Value |
|---|---|
| background | `#101215` |
| border-radius | `16px` |
| overflow | `hidden` |
| layout | `display:flex; flexDirection:column` |
| position | `absolute`, `left/top/width/height` from `state.pos[appId]` |
| z-index | `state.zOf[appId] \|\| 5` |
| transition | `box-shadow .18s ease` |
| open animation | `zwin .2s cubic-bezier(.2,.85,.3,1)` — only while `state.fresh[appId]` |

Shadow is parameterised on the `shadow` prop (`sh`, default 6, range 0–10, from
the props declaration at line 974):

```
focused:   0 {sh*2+4}px {sh*9+12}px -6px rgba(0,0,0,.9),
           0 0 0 1px rgba(184,232,56,.16),
           0 0 34px -12px rgba(184,232,56,.18)
unfocused: 0 {sh+2}px {sh*4}px -8px rgba(0,0,0,.75),
           0 0 0 1px rgba(255,255,255,.035)
```

At the default `sh = 6`: focused `0 16px 66px -6px …`, unfocused `0 8px 24px -8px …`.
**The focused window gets an accent-tinted 1 px ring and an accent glow; the
unfocused one gets a neutral white ring.** That ring, not the title-bar colour,
is the primary focus signal.

`@keyframes zwin` (line 15): `from{opacity:0;transform:scale(.965) translateY(10px)} to{opacity:1;transform:none}`.

### 2.1 Title bar

**Lines** template 74–85; computed 3109–3110.

| Property | Value |
|---|---|
| height | `36px`, `flex:'0 0 36px'` |
| layout | `display:flex; alignItems:center` |
| gap | `5px` |
| padding | `0 8px` |
| background focused | `#171a1e` |
| background unfocused | `#131518` — the only use of this hex in the file |
| cursor | `default` |

Child order (74–84): hamburger → left buttons → **flex:1 centred title column
(`pointer-events:none`)** → right buttons → close.

### 2.2 Title and subtitle

**Lines** 3111–3116, container 79.

| | Title | Subtitle |
|---|---|---|
| font-family | Public Sans (inherited) | **`'Roboto Mono', monospace`** |
| font-size | `12px` | `9px` |
| font-weight | `600` | 400 |
| letter-spacing | `.005em` | `.02em` |
| line-height | `1.15` | `1.2` |
| colour focused | `#eef0f2` | `#74797f` |
| colour unfocused | `#8b9096` | `#5c6167` |
| overflow | `nowrap` / `hidden` / `ellipsis`, `maxWidth:100%` | same |

Subtitle only renders when `w.hasSub` (3100). Only three apps set one (3093):
Text Editor → full document path, Files → cwd, Terminal → session cwd.

### 2.3 Window control button — 26×26

**Lines** `btn` factory 3094–3096, hamburger 3119–3121, close 3130–3132.

All three share: `width:26px; height:26px; borderRadius:16px; display:grid;
placeItems:center; cursor:default; flex:'0 0 26px'`.
Default fill `rgba(255,255,255,.08)`, glyph `#b9bec4`.

**They disagree on `fontSize`** — a leftover from a text-glyph era, since the
content is SVG:

| Button | `fontSize` | Icon px | Extra |
|---|---|---|---|
| generic `btn` (back/forward/+/save) | `11px` | 15 / 14 | — |
| hamburger | `12px` | 14 | active: `background:ACC; color:'#fff'` |
| close | `10px` | 13 | **hover** `background:#ff6a50; color:#0c0f05` (line 84) |

Note the hamburger's active text is a literal `#fff`, **not `INK`** — on the
light `#b8e838` accent this is a contrast failure. See §20.1.

Smaller icon-button variant, `findBtnStyle` (3664): `22×22`, `borderRadius:14px`,
`fontSize:9px`, `background:rgba(255,255,255,.08)`, `color:#b9bec4`. Used only in
the editor find bar, ×3.

### 2.4 Resize grips

**Lines** 3136–3145. Eight absolutely-positioned children, `zIndex:3`,
no background (invisible).

| Edge | Geometry | cursor |
|---|---|---|
| `n` | `top:0; left:11px; right:11px; height:5px` | `ns-resize` |
| `s` | `bottom:0; left:11px; right:11px; height:5px` | `ns-resize` |
| `w` | `left:0; top:11px; bottom:11px; width:5px` | `ew-resize` |
| `e` | `right:0; top:11px; bottom:11px; width:5px` | `ew-resize` |
| `nw` | `top:0; left:0; 13×13` | `nwse-resize` |
| `ne` | `top:0; right:0; 13×13` | `nesw-resize` |
| `sw` | `bottom:0; left:0; 13×13` | `nesw-resize` |
| **`se`** | `bottom:0; right:0; **16×16**` | `nwse-resize` |

The `se` corner is 16×16 while the other three corners are 13×13 (3144). Given
`se` is the conventional grab corner this reads deliberate, but it is the only
asymmetry.

Minimum window size is enforced in `resizeDown` (3340): `MW = 320, MH = 220`.

---

## 3. Segmented control (pill tab group)

**Container** (template) lines 160, 258, 303, 393.
**Items** (computed) `monTabs` 3574–3577, `logFilters` 3679–3682, `meshBtns`
3849–3852, `fontZooms` 3757–3760.
**Used by** System Monitor, Kernel Log, Renderer, Font Atlas.

Container, three of four identical:
```
display:flex; background:#090a0c; border:1px solid #1c2024;
border-radius:11px; padding:2px; gap:2px
```
**Font Atlas (line 393) uses `background:#07080a` instead of `#090a0c`.** One
step darker. Nothing else differs.

Items — all four share `borderRadius:9px`, `cursor:default`,
`whiteSpace:nowrap`, active `background:ACC / color:INK`, inactive
`background:transparent / color:#9ba0a6`. **Padding and type disagree four ways:**

| Instance | padding | font-size | font-family | Line |
|---|---|---|---|---|
| System Monitor `monTabs` | `4px 13px` | `11.5px` | Public Sans | 3576 |
| Kernel Log `logFilters` | `4px 13px` | `11.5px` | Public Sans | 3681 |
| Renderer `meshBtns` | `4px 9px` | `10.5px` | Public Sans | 3851 |
| Font Atlas `fontZooms` | `3px 10px` | `10px` | **Roboto Mono** | 3759 |

Monitor and Log agree exactly; Renderer and Font Atlas each go their own way.
The toolkit needs a size parameter here, not one fixed pill.

The bar the control sits in: `height:36px; flex:0 0 36px; display:flex;
align-items:center; justify-content:center; background:#101215;
border-bottom:1px solid #0b0d0f` (159, 257). Renderer and Font Atlas embed
theirs inside a general toolbar row instead (301, 391).

---

## 4. Tab strips

Three unrelated tab idioms exist. They are not variants of one widget.

### 4.1 Closeable tab strip — Terminal, Text Editor

**Container** lines 99, 223 (byte-identical):
```
display:flex; align-items:stretch; height:30px; flex:0 0 30px;
background:#171a1e; border-bottom:1px solid #0b0d0f;
padding:4px 5px 0; gap:3px; overflow:hidden
```
**Item** `termTabs` 3491–3495, `editTabs` 3617–3621. Verified identical apart
from the state key (`sesI` vs `docI`):
```
$ diff <(sed -n '3491,3495p' f) <(sed -n '3617,3621p' f)
# only differences are st.sesI ↔ st.docI
```

| Property | Value |
|---|---|
| layout | `display:flex; alignItems:center; gap:7px` |
| padding | `0 9px 0 12px` — asymmetric, extra room on the left |
| width | `minWidth:0; maxWidth:170px` |
| border-radius | `7px 7px 0 0` |
| font-size | `11px` |
| active | `background:#0b0d0f; color:#dfe2e5` (matches the body below it — a true "connected" tab) |
| inactive | `background:rgba(255,255,255,.04); color:#74797f` |
| label | `nowrap / hidden / ellipsis` (template 100, 224) |
| close ✕ | `display:grid; placeItems:center; flex:'0 0 9px'; cursor:'pointer'`; icon 9 px; colour active `#9ba0a6`, inactive `#474b50` |

The ✕ is the **only element in the file with `cursor:'pointer'`** other than
column headers — everything else is `cursor:'default'`.

Tab labels: Terminal `'zl:' + cwd` truncated to `…` + last 11 chars past 12
(3487); Editor `basename + (saved ? '' : ' •')` (3614).

### 4.2 Underline tab strip — Services

**Container** line 431: `height:32px; flex:0 0 32px; display:flex;
align-items:stretch; background:#14171a; border-bottom:1px solid #0b0d0f`.
**Item** 3787–3791:

| Property | Value |
|---|---|
| layout | `display:flex; alignItems:center` |
| padding | `0 15px` |
| font-size | `11.5px` |
| active | `color:#eef0f2; boxShadow:'inset 0 -2px 0 ' + ACC` |
| inactive | `color:#8b939e; boxShadow:'none'` |

No background change at all — the 2 px inset underline is the whole affordance.

### 4.3 Pill tab strip — Image Viewer

`imgTabs` 4203–4207 (file selection) and `imgZoom` 4208–4212 (zoom), in the same
34 px toolbar (514). Unlike §3 these are **not** grouped inside a bordered
container; they are loose pills with an idle fill.

| | `imgTabs` | `imgZoom` |
|---|---|---|
| padding | `5px 11px` | `5px 9px` |
| radius | `10px` | `9px` |
| font | Roboto Mono `10.5px` | Roboto Mono `10px` |
| active | `ACC` / `INK` | `ACC` / `INK` |
| idle | `rgba(255,255,255,.06)` / `#9ba0a6` | `rgba(255,255,255,.06)` / `#74797f` |

---

## 5. Toolbar row

There is no single toolbar. Fourteen instances, at four heights and three
backgrounds:

| App | Height | Padding | Background | Border | Line |
|---|---|---|---|---|---|
| System Monitor (segmented bar) | 36 | — (centred) | `#101215` | bottom `1px #0b0d0f` | 159 |
| Kernel Log (segmented bar) | 36 | — (centred) | `#101215` | bottom | 257 |
| Files (breadcrumbs + search) | 34 | `0 8px`, gap 4 | `#101215` | bottom | 119 |
| Text Editor (find bar) | 34 | `0 9px`, gap 6 | `#171a1e` | bottom | 228 |
| Hex Viewer | 32 | `0 8px`, gap 6 | `#101215` | bottom | 269 |
| Services (tab bar) | 32 | — | `#14171a` | bottom | 431 |
| Archive Manager | 36 | `0 10px`, gap 7 | `#14171a` | bottom | 471 |
| Network | 36 | `0 10px`, gap 7 | `#14171a` | bottom | 598 |
| Image Viewer | 34 | `0 9px`, gap 5 | `#14171a` | bottom | 514 |
| Renderer (bottom) | 38 | `0 10px`, gap 6 | `#14171a` | **top** | 301 |
| Framebuffer (bottom) | 34 | `0 10px`, gap 8 | `#14171a` | top | 328 |
| Console tty (bottom) | 34 | `0 10px`, gap 7 | `#14171a` | top | 350 |
| Font Atlas (bottom) | 36 | `0 11px`, gap 7 | `#14171a` | top | 391 |
| Snake/games (bottom) | 30 | `0 11px`, gap 8 | `#14171a` | top | 576 |

Consistent within the file: **top toolbars sit on `#101215`/`#171a1e` with a
bottom border; bottom toolbars sit on `#14171a` with a top border.** The
exceptions are Services, Archive Manager, Network and Image Viewer, which put a
`#14171a` bar at the *top*.

Border colour is always `1px solid #0b0d0f`. All are `flex:'0 0 <h>px'` with
`overflow:hidden` where they can wrap.

---

## 6. Status bar row

| App | Height | Padding | Gap | Background | Font | Colour | Line |
|---|---|---|---|---|---|---|---|
| Files | 26 | `0 11px` | 12 | `#14171a` | 11px Public Sans | `#9ba0a6` | 151 |
| System Monitor | 38 | `0 10px` | 8 | `#14171a` | 11px | `#9ba0a6` | 178 |
| Text Editor | 25 | `0 11px` | 13 | `#14171a` | 10.5px | `#9ba0a6` | 251 |
| Kernel Log | 25 | `0 11px` | — | `#14171a` | 10.5px | `#9ba0a6` | 265 |
| Hex Viewer | 26 | `0 9px` | 12 | **`#101215`** | **Roboto Mono 10px** | **`#74797f`** | 286 |

All carry `border-top:1px solid #0b0d0f`, `white-space:nowrap`,
`flex:'0 0 <h>px'`, and use `<span style="flex:1"></span>` as the spacer between
left and right groups.

**System Monitor's is 38 px because it holds a button** (`killStyle`, 3584); the
others hold only text. Hex Viewer is the outlier on all three of background,
font family and colour.

---

## 7. Column header grid + list row

Three apps use a CSS-grid table. All three declare the same track string in the
header and in the row, so header and body stay aligned.

### 7.1 Track definitions — verbatim

```js
// line 3055 — System Monitor process table
const PGRID = 'minmax(0,48px) minmax(0,52px) minmax(0,28px) minmax(0,48px) minmax(0,52px) minmax(0,1fr)';
// line 3056
const pcolDefs = [['pid','PID','r'],['user','USER','l'],['stat','S','c'],
                  ['cpu','%CPU','r'],['mem','RSS','r'],['cmd','COMMAND','l']];

// line 3057 — Files list view
const FGRID = 'minmax(0,1fr) minmax(0,58px) minmax(0,84px) minmax(0,78px) minmax(0,62px)';
// line 3058
const colDefs = [['name','Name','left'],['size','Size','right'],['type','Type','left'],
                 ['perm','Permissions','left'],['mtime','Modified','left']];

// line 4160 / 4175 — Archive Manager (declared inline, twice, not hoisted to a const)
'minmax(0,1fr) minmax(0,66px) minmax(0,62px) minmax(0,62px) minmax(0,54px) minmax(0,58px)'
// line 4163
['Member','Size','Mode','Owner','Magic','Checksum']
```

Note `FGRID` puts the `1fr` **first** (name grows) while `PGRID` puts it **last**
(command grows). The archive grid follows `FGRID`'s shape.

### 7.2 Header row

| | `pcolGrid` (3580) | `fcolGrid` (3582) | `archGrid` (4160) |
|---|---|---|---|
| height | `23px`, `flex:0 0 23px` | `24px`, `flex:0 0 24px` | `24px`, `flex:0 0 24px` |
| align-items | `stretch` | `stretch` | `stretch` |
| background | `#101215` | `#101215` | **`#14171a`** |
| border-bottom | `1px solid #0b0d0f` | `1px solid #0b0d0f` | `1px solid #0b0d0f` |
| font-size | `11px` | `11px` | **`10.5px`** |
| padding | *(none)* | `0 15px 0 4px` | `0 15px 0 4px` |
| box-sizing | *(none)* | `border-box` | `border-box` |

The `0 15px` right padding on `fcolGrid`/`archGrid` reserves room for the
scrollbar (`9px` wide, line 25) so the header stays aligned with the scrolled
body. **`pcolGrid` omits it, so the Monitor's header is 15 px out of alignment
with its rows whenever the process list scrolls.** See §20.5.

### 7.3 Header cell

`cols` 3227–3234 (Files), `pcols` 3271–3278 (Monitor), `archCols` 4163–4166.

| Property | Value |
|---|---|
| layout | `display:flex; alignItems:center` |
| justify | `flex-end` when the column is right-aligned, `center` for `'c'`, else `flex-start` |
| padding | right-aligned `0 10px 0 0`; centred `0`; left `0 0 0 8px` |
| cursor | **`pointer`** (Files, Monitor) / not set (Archive — headers are inert there) |
| overflow | `hidden / ellipsis / nowrap`, `minWidth:0`, `boxSizing:'border-box'` |
| colour active-sort | `#eef0f2` |
| colour inactive | `#9ba0a6` (Files, Monitor) / **`#8b939e`** (Archive) |
| sort arrow | `icon('sortU'\|'sortD', 8)`, `display:grid; placeItems:center; paddingLeft:4px; color:#cdf25a` |

Files toggles direction on repeat click (3230); Monitor only sets the key and
always draws `sortD` (3272) — its arrow never flips.

### 7.4 List row

| | Files `rows` (3209) | Monitor `procs` (3255) | Archive `archRows` (4175) |
|---|---|---|---|
| height | `26px` | `23px` | `25px` |
| border-radius | `9px` | *(none)* | *(none)* |
| font-size | `12px` | *(inherited 12.5px)* | *(inherited)* |
| padding | *(none)* | *(none)* | `0 4px`, `boxSizing:border-box` |
| selected bg | `rgba(184,232,56,.15)` | `rgba(184,232,56,.15)` | **solid `ACC`** |
| selected marker | `inset 2px 0 0 ACC` | `inset 2px 0 0 ACC` | *(none)* |
| selected text | `#eef0f2` | `#eef0f2` | `INK` |
| zebra | *(none)* | odd `rgba(255,255,255,.014)` | odd `rgba(255,255,255,.012)` |
| unselected text | dir `#c7ce9a`, dotfile `#74797f`, else `#dfe2e5` | `#dfe2e5` | `#eef0f2` |

Three different selection treatments, two different zebra alphas one thousandth
apart. See §20.2.

Shared cell styles (Files 3223–3225, Monitor 3258–3266, Archive 4178–4185):

| Cell kind | Style |
|---|---|
| `numStyle` | Roboto Mono `10.5px`, `textAlign:right`, `paddingRight:10px`, colour `#9ba0a6` (Files/Archive) or `#dfe2e5` (Monitor) |
| `dimStyle` | `11px` Public Sans, `paddingLeft:4px` (Files) / `7px` (Monitor), `#9ba0a6`, ellipsis |
| `monoStyle` | Roboto Mono `10px`, `paddingLeft:4px` (Files) / `8px` (Archive), `#74797f` |
| `cmdStyle` | Roboto Mono `10.5px`, `paddingLeft:8px; paddingRight:8px`, `#9ba0a6`, ellipsis |
| `stStyle` | Roboto Mono `10.5px`, `textAlign:center`, colour by state: `R`→`OK`, `D`→`BAD`, `T`→`WARN`, else `#9ba0a6` |
| `nameStyle` (Archive) | Roboto Mono `11px`, `paddingLeft:8px`, ellipsis |
| icon cell (Files) | template 132: `display:flex; alignItems:center; gap:8px; paddingLeft:8px`; icon `flex:'0 0 15px'` |

Row icon colour by file kind (3219–3222): selected `#eef0f2` · directory
`#cdf25a` · `.zl` `#a693ff` · `.c`/`.S` `#f5b93c` · perm starts `-r-x` → `OK` ·
perm starts `c`/`b` → `WARN` · else `#5c6167`.

### 7.5 Inline sparkline inside a row cell

`cpuStyle` + `sparkFillStyle`, 3262–3269. A 2 px bar drawn behind the %CPU number:
```
position:absolute; right:6px; bottom:3px; height:2px; borderRadius:6px;
width: max(3, min(38, cpu * 1.6)) px
background: selected ? rgba(255,255,255,.85) : cpu>15 ? WARN : cpu>1 ? ACC : #474b50
```
The containing cell is `position:relative` with `height:100%` and the number is
wrapped in its own `position:relative` span (template 173) so it paints above.

### 7.6 Icon-view grid (Files alternate)

Container line 142: `display:grid; grid-template-columns:repeat(auto-fill,minmax(92px,1fr));
gap:5px; padding:11px; align-content:start; background:#101215`.
Tile `gridStyle` 3213–3216: `flex column; alignItems:center; gap:6px;
padding:9px 4px; borderRadius:12px`; selected `rgba(184,232,56,.15)` +
`inset 0 0 0 1px ACC` (a full ring, not the 2 px left bar the list row uses).
Icon 27 px weight 1.5; label `11px`, `textAlign:center`, `lineHeight:1.3`,
`wordBreak:break-word` (template 146). Name is truncated to 14 chars + `…` past
15 (3186).

---

## 8. Stat card strip

The `repeat(auto-fit, minmax(N, 1fr))` counter strip.
**Used by** Framebuffer, Console (tty1), Archive Manager, Image Viewer.

Container, all four (320, 342, 494, 524):
```
flex:0 0 auto; display:grid;
grid-template-columns:repeat(auto-fit,minmax(Npx,1fr));
gap:1px; background:#0b0d0f; border-top:1px solid #0b0d0f
```
The `gap:1px` over a `#0b0d0f` background is what draws the hairlines between
cells — the cells themselves have no borders.

| App | `minmax` | Cell padding | Line |
|---|---|---|---|
| Framebuffer | `88px` | `7px 9px` | 320 |
| Console (tty1) | **`84px`** | **`6px 9px`** | 342 |
| Archive Manager | **`96px`** | `7px 10px` | 494 |
| Image Viewer | `88px` | `7px 10px` | 524 |

Four instances, three `minmax` values, three paddings. Nothing in the content
justifies the difference. See §20.4.

Cell interior, identical in all four:
```
background:#14171a; display:flex; flexDirection:column; gap:2px; minWidth:0
```
**Key** (323, 345, 497, 527): `font-size:9px; letter-spacing:.07em;
text-transform:uppercase; color:#5c6167; white-space:nowrap`.
**Value** (`vStyle`, 3724 / 3835 / 4197 / 4223): `fontFamily:'Roboto Mono';
fontSize:11.5px; whiteSpace:nowrap; overflow:hidden; textOverflow:ellipsis`,
colour per datum — `#dfe2e5` normal, `#9ba0a6` de-emphasised, `WARN`/`OK` for
threshold values (e.g. `f.ms > 16 ? WARN : OK` at 3722).

---

## 9. Sidebar

### 9.1 Sidebar panel

| App | Width | Padding | Line |
|---|---|---|---|
| Files | `150px` | `7px 6px` | 110–111 |
| Network | `158px` | `8px 7px` | 584–585 |
| Settings | `158px` | `7px 6px` | 701–702 |

All: `background:#14171a; border-right:1px solid #0b0d0f; overflow-y:auto;
overflow-x:hidden; min-height:0`. Files and Settings add `box-sizing:border-box`
(Files only).

Outer grid: Files `grid-template-columns:150px minmax(0,1fr)`; Network
`158px minmax(0,1fr)`; Settings `158px 1fr` — Settings omits the `minmax(0,…)`
that stops a grid child overflowing. Rows are `minmax(0,1fr)` in all three.

### 9.2 Section heading — five different treatments

| Heading | Size | Weight | Tracking | Case | Colour | Padding | Line |
|---|---|---|---|---|---|---|---|
| Files "Places" | `10.5px` | **700** | `.05em` | as-written | `#74797f` | `4px 8px 6px` | 112 |
| Files "Devices" | `10.5px` | **700** | `.05em` | as-written | `#74797f` | `11px 8px 6px` | 114 |
| Network "Interfaces" | **`9px`** | 400 | `.09em` | **uppercase** | `#5c6167` | `3px 8px 6px` | 586 |
| Clock "Stopwatch"/"Countdown" | `10px` | 400 | `.08em` | **uppercase** | `#5c6167` | — | 636, 650 |
| Network "ping output" | `10px` | 400 | `.08em` | **uppercase** | `#5c6167` | — | 612 |
| Disk "Largest files on rd0" | **`11px`** | 400 | `.05em` | **uppercase** | `#5c6167` | — | 417 |

Only the two Files headings agree with each other. The dominant shape is
`10px / 400 / .08em / uppercase / #5c6167`; Files is the odd one out with
`10.5px / 700 / .05em / mixed case / #74797f`. See §20.6.

### 9.3 Sidebar nav row

| | Files tree (3554) | Files mounts (3563) | Settings nav (3856) | Network ifaces (4034) |
|---|---|---|---|---|
| padding | `6px 9px` | `6px 9px` | `8px 11px` | `8px 9px` |
| radius | `10px` | `10px` | `11px` | `11px` |
| font-size | `12px` | `12px` | `12.5px` | inherited |
| gap | `8px` | `8px` | — | `9px` |
| indent | `marginLeft:12px` when depth > 2 | — | — | `marginBottom:2px` |
| selected bg | `rgba(184,232,56,.15)` | *(never selected)* | `rgba(184,232,56,.15)` | **solid `ACC`** |
| selected marker | `inset 2px 0 0 ACC` | — | `inset 2px 0 0 ACC` | *(none)* |
| selected text | `#dff29a` | — | `#dff29a` | `INK` |
| idle text | `#b9bec4` | `#9ba0a6` | `#b9bec4` | `#c4c9cf` |

Same disagreement as §7.4: tint + 2 px inset bar (Files, Settings) versus solid
accent fill (Network).

Files tree icon: 14 px, `flex:'0 0 14px'`, colour selected `#dff29a`, `/tmp`
`OK`, else `#7d848c` (3551). Mount icon 13 px, colour `OK` when writable else
`#7d848c` (3562), with a Roboto Mono `9.5px` `ro`/`rw` flag coloured `OK`/`BAD`
(3564).

Files sidebar footer (116): `margin:9px 8px 0; padding-top:9px;
border-top:1px solid #1c2024; font-family:'Roboto Mono'; font-size:9.5px;
line-height:1.65; color:#5c6167; white-space:pre-wrap`.

Network interface row body (590–592): two-line stack, `gap:1px`; name Roboto
Mono `11.5px`; state `9.5px`, `#74797f`, ellipsis.

---

## 10. Bars — meter, progress, segment, inline

Four distinct bar widgets. They do not share a radius or a track colour.

### 10.1 Meter bar — System Monitor Resources

**Track** template 200: `height:5px; border-radius:7px; background:#090a0c; overflow:hidden`.
**Fill** `barStyle` 3601: `width:<pct>%; height:100%; borderRadius:7px; background:<col>`.
**Row** `rowStyle` 3600: `padding:11px 13px; borderTop: i ? '1px solid #1c2024' : 'none'`.
**Label line** template 199: `display:flex; justify-content:space-between; gap:12px;
font-size:11.5px; white-space:nowrap; margin-bottom:6px`; value `valStyle` 3599:
Roboto Mono `10.5px`, coloured by the meter's own `col`.

The four meters and their colours (3594–3598): Memory `OK`, Frame time `ACC`,
Heap `BAD` (pinned at 100 %), tmpfs `WARN` (floored at 2 %).

### 10.2 Progress bar — Clock countdown

**Track** template 659: `flex:1; height:5px; background:#0d0f12; border-radius:4px; overflow:hidden`.
**Fill** `barStyle` 4103: `width:<pct>%; height:100%; borderRadius:4px;
background: left<5000 ? WARN : ACC; transition:'width .1s linear'`.

Same 5 px height as the meter, **different radius (4 vs 7) and different track
colour (`#0d0f12` vs `#090a0c`)**.

### 10.3 Disk segment bar — Disk Usage

**Track** template 410: `height:12px; background:#07080a; border:1px solid #1c2024;
border-radius:7px; padding:1px; display:flex; gap:1px; overflow:hidden`.
**Segment** `seg()` 3767: `flex:<fraction>; background:<col>; border-radius:5px`,
fraction floored at `0.001`. Used segment `ACC` (writable) or `BAD` (read-only);
free segment `#161a1e`.

The only bar that is bordered and padded — it reads as a container of segments,
not a track with a fill.

### 10.4 Inline mini-bar — Disk Usage largest-files list

**Track** template 421: `flex:0 0 84px; height:6px; background:#07080a;
border-radius:4px; overflow:hidden`.
**Fill** `barStyle` 3785: `width:max(2, size/max*100)%; height:100%;
background:ACC; borderRadius:4px`.

Row (419): `display:flex; align-items:center; gap:9px; padding:4px 0;
border-bottom:1px solid #101215`; path Roboto Mono `11px` `#dfe2e5` ellipsis;
size `flex:0 0 54px; text-align:right`, Roboto Mono `10.5px` `#9ba0a6`.

---

## 11. Toggle switch

**Lines** 3903–3908 (row + track + knob), template 725–728.
**Used by** Settings only (Appearance, Windows, Sound, Kernel, Network, Games panes).

| Part | Value |
|---|---|
| row | `display:flex; alignItems:center; gap:12px; padding:12px 13px; cursor:default; borderTop: i ? '1px solid #1c2024' : 'none'` |
| row label | `font-size:12.5px; flex:1` (template 726) |
| track | `width:40px; height:22px; borderRadius:14px; flex:'0 0 40px'; position:relative` |
| track on | `background:ACC` |
| track off | `background:#22262b` |
| track transition | `background .15s` |
| knob | `position:absolute; top:3px; width:16px; height:16px; borderRadius:12px; background:#fff; boxShadow:'0 1px 3px rgba(0,0,0,.4)'` |
| knob on | `left:21px` |
| knob off | `left:3px` |
| knob transition | `left .15s` |

Geometry check: `3 + 16 + 21 = 40` — the knob's right edge lands flush on the
track's right edge when on, with no right margin. Off state has 3 px on the left
and 21 px of empty track on the right. **The travel is asymmetric by 3 px.**
The knob is also 16 px in a 22 px track, i.e. 3 px top and bottom, which matches.

The card the toggles live in (723): `background:#14171a; border:1px solid #1c2024;
border-radius:13px; overflow:hidden; margin-bottom:14px`. There is no gap
between rows — the `borderTop` on rows 2..n is the separator.

---

## 12. Slider

The only widget defined purely in CSS, lines 29–31:

```css
input[type=range]{-webkit-appearance:none;appearance:none;height:4px;
  border-radius:7px;background:#22262b;outline:none}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:15px;
  height:15px;border-radius:12px;background:#b8e838;
  box-shadow:0 1px 4px rgba(0,0,0,.6),0 0 10px rgba(184,232,56,.35);cursor:pointer}
input[type=range]::-moz-range-thumb{width:15px;height:15px;border:0;
  border-radius:12px;background:#b8e838;cursor:pointer}
```

| Part | Value |
|---|---|
| track height | `4px` |
| track radius | `7px` |
| track colour | `#22262b` |
| thumb | `15×15`, radius `12px` |
| thumb colour | **`#b8e838` hard-coded** — not `ACC`. Changing the accent does not move the slider. |
| thumb shadow | `0 1px 4px rgba(0,0,0,.6), 0 0 10px rgba(184,232,56,.35)` — a drop shadow plus an accent glow |

The Firefox thumb rule omits the box-shadow entirely, so the glow is
WebKit-only. Both hard-code the accent.

Slider card (template 707–711): `background:#14171a; border:1px solid #1c2024;
border-radius:13px; padding:11px 13px; gap:8px; margin-bottom:14px`; label row
`font-size:12px` with the value in Roboto Mono `#9ba0a6`; the input is
`width:100%`.

Four sliders exist (3863–3877): Window shadow 0–10, Beep pitch 220–1400 Hz, Beep
length 20–400 ms, Monitor sample interval 1–5 s, Scheduler quantum 200–4000.
All `step="1"`.

---

## 13. Buttons

### 13.1 The pill button — 26 near-identical variants

This is the single biggest inconsistency in the reference. Every one of these is
the same shape (rounded rect, one line of text, `cursor:default`,
`whiteSpace:nowrap`) and no two agree on all three of padding, radius and size.

| Name | Padding | Radius | Size | Family | Idle fill / text | Line |
|---|---|---|---|---|---|---|
| `killStyle` | `6px 13px` | 11 | 11.5 | sans | `rgba(255,106,80,.16)` + `1px rgba(255,106,80,.4)` / `#ffb3a4` | 3584 |
| `spinBtnStyle` | `6px 14px` | 11 | 11.5 | sans | `ACC` when off / `#fff` | 3706 |
| `wireBtnStyle` | `6px 12px` | 11 | 11 | sans | `rgba(255,255,255,.07)` / `#dfe2e5` | 3712 |
| `fbBtnStyle` | `5px 12px` | 11 | 11 | sans | `rgba(255,255,255,.07)` / `#dfe2e5` | 3728 |
| `ttyRunStyle` | `5px 12px` | 11 | 11 | sans | `rgba(255,255,255,.07)` / `#dfe2e5` | 3839 |
| `ttyScanStyle` | `5px 12px` | 11 | 11 | sans | `rgba(255,255,255,.07)` / `#dfe2e5` | 3842 |
| `ttyResetStyle` | `5px 12px` | 11 | 11 | sans | `rgba(255,255,255,.07)` / `#dfe2e5` | 3845 |
| `netToggleStyle` | `6px 13px` | 11 | 11 | **mono** | `rgba(255,255,255,.07)` / `#dfe2e5` | 4041 |
| `netPingStyle` | `6px 13px` | 11 | 11 | **mono**, w700 | `ACC` / `INK` | 4044 |
| `archPackStyle` | `6px 13px` | 11 | 11 | sans, w700 | `ACC` / `INK` | 4154 |
| `archBtnStyle` | `6px 12px` | 11 | 11 | sans | `rgba(255,255,255,.07)` / `#dfe2e5` | 4156 |
| `timerBtns` | `6px 13px` | 11 | 11 | **mono** | `rgba(255,255,255,.07)` / `#dfe2e5` | 4092 |
| `clkBtnStyle` | `7px 13px` | **12** | 11.5 | sans | `rgba(255,255,255,.07)` / `#dfe2e5` | 4084 |
| `swBtnStyle` | `7px 17px` | **12** | 11.5 | sans, w700 | `ACC` / `INK`, running → `rgba(212,105,90,.16)` / `BAD` | 4078 |
| `renameOkStyle` | `6px 15px` | 11 | 12 | sans | `ACC` / **`#fff`** | 4302 |
| props "Close" | `6px 18px` | 11 | 12 | sans | `#22262b` | 920 |
| rename "Cancel" | `6px 14px` | 11 | 12 | sans | `#22262b` | 932 |
| `ovCats` | `6px 15px` | **12** | 11.5 | sans | `rgba(255,255,255,.07)` / `#c4c9cf`, active `ACC`/`INK` | 3396 |
| `crumbs` | `4px 10px` | **10** | 11.5 | sans | transparent / `#9ba0a6`, active `rgba(255,255,255,.1)`/`#eef0f2` | 3238 |
| `intervals` | `3px 11px` | **10** | 11 | sans | `rgba(255,255,255,.06)` / `#9ba0a6` | 3592 |
| util `btn` | `5px 11px` | **10** | 10.5 | **mono** | `rgba(255,255,255,.07)` / `#dfe2e5`, on `ACC`/**`#0d1005`** | 1778 |
| `imgTabs` | `5px 11px` | **10** | 10.5 | **mono** | `rgba(255,255,255,.06)` / `#9ba0a6` | 4205 |
| `imgZoom` | `5px 9px` | **9** | 10 | **mono** | `rgba(255,255,255,.06)` / `#74797f` | 4208 |
| `noteClearStyle` | `3px 9px` | **9** | 10 | sans | `rgba(255,255,255,.07)` / `#9ba0a6` | 3961 |
| svc `act()` | `3px 9px` | **8** | 10 | sans | start `rgba(169,227,75,.14)`/`#c7f36a`, stop `rgba(212,105,90,.14)`/`#ff9c88`, else `rgba(255,255,255,.06)`/`#c9ced4` | 3802 |
| svc restart badge | `3px 7px` | **8** | 10 | sans | `rgba(245,185,60,.13)` / `#ffd166` | 3818 |
| `hexPages` | `3px 9px` | **12** | 10 | **mono** | `#090a0c` + `1px #1c2024` / `#9ba0a6`, active `ACC`/`INK` + `1px rgba(255,255,255,.2)` | 3522 |
| `timerRows.xStyle` (text-only) | — | — | 10 | sans | `#74797f` | 4106 |

**Recommendation for the toolkit:** three sizes covers all of it —
`sm` = `3px 9px` / r9 / 10px, `md` = `5–6px 12–13px` / r11 / 11px,
`lg` = `7px 13–17px` / r12 / 11.5px — with `mono` and `weight700` as flags. But
record that the reference does **not** collapse this cleanly, so any port will
differ from it by 1–4 px in places.

### 13.2 Primary button

`background: ACC`, `color: INK`, usually `fontWeight:700`.
Canonical: `archPackStyle` (4154), `netPingStyle` (4044), `swBtnStyle` (4078).

**Three variants get the ink wrong** and hard-code white instead of computing it:

| Widget | Line | Writes |
|---|---|---|
| `spinBtnStyle` | 3707 | `background: st.spin ? 'rgba(255,255,255,.09)' : ACC, color:'#fff'` |
| `renameOkStyle` | 4302 | `background:ACC, color:'#fff'` |
| window hamburger active | 3121 | `color: st.winMenu === a.id ? '#fff' : '#b9bec4'` |

On the default `#b8e838` accent, `INK` resolves to `#0c1005` (dark), so these
three render white-on-light-green. See §20.1.

### 13.3 Destructive button

Two unrelated red tints exist:

| Widget | Fill | Text | Border | Line |
|---|---|---|---|---|
| `killStyle` (End Process) | `rgba(255,106,80,.16)` = `BAD` at 16 % | `#ffb3a4` | `1px rgba(255,106,80,.4)` | 3584 |
| calculator `C` key | `rgba(212,105,90,.16)` | `#ff9c88` | — | 3750 |
| stopwatch running | `rgba(212,105,90,.16)` | `BAD` | — | 4079 |
| svc `stop` action | `rgba(212,105,90,.14)` | `#ff9c88` | — | 3803 |
| notification error row | `rgba(212,105,90,.07)` | — | — | 3968 |

`rgb(212,105,90)` is `#d4695a` — **it is not `BAD` (`#ff6a50`)**, and it appears
only as an rgba, never as a hex. Four uses of the phantom red versus one use of
the real one. See §20.7.

### 13.4 Icon button

See §2.3. Two sizes: `26×26 / r16` (title bar) and `22×22 / r14` (find bar).
Both `display:grid; placeItems:center`, fill `rgba(255,255,255,.08)`, glyph
`#b9bec4`.

Circular icon variants elsewhere:
- notification-row icon `20×20 / r12`, `rgba(255,255,255,.06)` (3969)
- toast icon `22×22 / r14`, fill = kind colour, glyph `#fff` (4233)
- tray-toggle icon `22×22 / r14`, on `ACC`/`INK`, off `rgba(255,255,255,.09)`/`#fff` (3465)
- sysinfo group icon `22×22 / **r8**`, accent- or warn-tinted (4143)
- quick-settings mark `30×30 / r17`, `ACC`/`INK` (3457)
- System Info mark `52×52 / r16`, `rgba(184,232,56,.1)` + `1px rgba(184,232,56,.2)` (4111)

---

## 14. Monospace panel

Five instances. **No two share a background, size and line-height.**

| Panel | Background | Padding | Size | line-height | Extra | Line |
|---|---|---|---|---|---|---|
| Terminal | `#0b0d0f` | `8px 11px 9px` | `12px` | `1.5` | `cursor:text; user-select:text` | 103 |
| Text Editor | `#0b0d0f` | `9px 12px` (inner) | `12px` | `1.6` | gutter `flex:0 0 42px` | 236, 3638 |
| Kernel Log | `#0b0d0f` | `6px 10px` | `11px` | `1.55` | `user-select:text` | 262 |
| Hex Viewer | `#090a0c` | `7px 9px` | `11.5px` | `1.5` | `user-select:text` | 277 |
| Syscall trace | `#0d0f12` | `5px 0` (rows `2px 12px`) | `10.5px` | — | `white-space:nowrap` | 454, 456 |

### 14.1 Terminal line

`termLines` 3475–3481. Plain: `whiteSpace:'pre-wrap'; wordBreak:'break-word'`.
Jump-target line (a diagnostic you can click): adds
`cursor:'pointer'; borderRadius:'7px'; margin:'0 -4px'; padding:'0 4px';
background:'rgba(184,232,56,.07)'` — the negative margin lets the highlight bleed
past the panel padding.

Prompt (template 105): `zl` `#a9e34b` · `@` `#5c6167` · `surface` `#b8e838` ·
`:` `#5c6167` · cwd `#f5b93c` · `$&nbsp;` `#dfe2e5`; input is transparent,
`border:0`, Roboto Mono `12px`, `color:#eef0f2`, `flex:1; min-width:60px`.

### 14.2 Kernel log line

Template 263: `display:flex; gap:9px`; timestamp `color:#5c6167; flex:0 0 56px;
text-align:right; white-space:nowrap`; message colour (3685) `e`→`BAD`,
`w`→`WARN`, else `#b9bec4`.

### 14.3 Hex row

Template 279–283: `display:flex; gap:13px; white-space:pre`.
Offset `flex:0 0 62px; color:#474b50` · bytes `flex:1; color:#d8f588;
letter-spacing:.03em` · ASCII `flex:0 0 132px; color:#9ba0a6`.
16 bytes per row, 16 rows per page, 256-byte pages (3527–3537).

### 14.4 Syscall trace row

Template 456–461: `display:flex; gap:9px; padding:2px 12px; font-size:10.5px`.
time `flex:0 0 52px; text-align:right; #474b50` · pid `flex:0 0 34px;
text-align:right; #74797f` · call `flex:0 0 92px`, colour `#87b0f0` ok /
`BAD` fail (3824) · arg `flex:1; #9ba0a6; ellipsis` · result `flex:0 0 42px;
text-align:right`, `OK`/`BAD` (3825).

### 14.5 Editor gutter, current line, diagnostics

| Part | Value | Line |
|---|---|---|
| gutter | `flex:'0 0 42px'; padding:'9px 0'; text-align:right; color:#474b50; background:#0b0d0f; user-select:none; overflow:hidden` | 238 |
| gutter number | `padding:'0 9px 0 0'`; colour: diagnostic → `WARN` (warning) or `ACC` (note); caret line → `#b9bec4`; else `#474b50`. `fontWeight:700` on a diagnostic line | 3625–3627 |
| current-line band | `position:absolute; left:0; right:0; top:(9 + (line-1)*19.2)px; height:19.2px; background:rgba(255,255,255,.035)` | 3630 |
| diagnostic strip | same geometry; warning `rgba(245,185,60,.09)` + `inset 0 -1px 0 rgba(245,185,60,.5)`; note `rgba(184,232,56,.08)` + `inset 0 -1px 0 rgba(184,232,56,.45)` | 3632–3635 |
| highlight layer | `position:absolute; inset:0; padding:'9px 12px'; Roboto Mono 12px; lineHeight:1.6; pointerEvents:none; color:#dfe2e5` | 3638 |
| textarea overlay | same box; `background:transparent; border:0; color:transparent; caretColor:#eef0f2; userSelect:text; resize:none` | 3643 |
| code line | `minHeight:'19.2px'` | 3641 |

**`19.2px` is `12px × 1.6`** and is hard-coded in three places (3630, 3633,
3641). Any change to the editor font size or line-height must change all three
or the highlight bands drift off the text.

### 14.6 Syntax token colours

`SC`, line 3244–3245:

| Token | Style |
|---|---|
| `kw` keyword | `#a693ff` |
| `num` number | `#f5b93c` |
| `cm` comment | `#5c6167`, `fontStyle:'italic'` |
| `op` operator | `#9ba0a6` |
| `id` identifier | `#dfe2e5` (also the fallback) |
| `ws` whitespace | `{}` |
| `head` markdown heading | `#cdf25a`, `fontWeight:700` |

---

## 15. Cards and key/value grids

### 15.1 Card surface

| Instance | Background | Border | Radius | Padding | Line |
|---|---|---|---|---|---|
| Monitor CPU card | `#14171a` | `1px #1c2024` | `13px` | `11px 12px`, gap 7 | 187 |
| Monitor meters card | `#14171a` | `1px #1c2024` | `13px` | — (`overflow:hidden`) | 196 |
| Monitor overview card | `#14171a` | `1px #1c2024` | `13px` | — | 212 |
| Settings slider card | `#14171a` | `1px #1c2024` | `13px` | `11px 13px`, gap 8 | 708 |
| Settings accent card | `#14171a` | `1px #1c2024` | `13px` | `12px 13px`, gap 9 | 714 |
| Settings toggles card | `#14171a` | `1px #1c2024` | `13px` | — | 723 |
| Settings about card | `#14171a` | `1px #1c2024` | `13px` | — | 733 |
| **System Info group card** | `#14171a` | **`1px #1a1d21`** | **`12px`** | — | 681 |
| Network ping panel | **`#0d0f12`** | **`1px #1a1d21`** | **`10px`** | `9px 11px` | 613 |
| Clock timers card | `#0b0d0f` | `1px #0b0d0f` | `10px` | — (`gap:1px`) | 655 |

The dominant card is `#14171a / 1px #1c2024 / r13`, eight uses. System Info uses
a different border colour and radius; Network's panel is a third thing. All
cards that hold rows set `overflow:hidden` and use `borderTop` on rows 2..n
rather than a gap.

Card margin-bottom in Settings is `14px` on every card (708, 714, 723, 733).

### 15.2 Card header + key/value grid — System Info

**Header** template 682–687: `display:flex; align-items:center; gap:8px;
padding:9px 12px; border-bottom:1px solid #1a1d21`; icon 22×22 r8 tinted
(`rgba(184,232,56,.1)` ok / `rgba(245,185,60,.1)` warn, 4143); name `11.5px`
weight 700 `#eef0f2`; `flex:1` spacer; badge pill.

**Badge** `badgeStyle` 4145–4147: Roboto Mono `9.5px`, `padding:3px 8px`,
`borderRadius:8px`, `whiteSpace:nowrap`, background/colour accent-tinted when ok
else warn-tinted.

**Grid** template 688: `display:grid; grid-template-columns:auto minmax(0,1fr);
gap:5px 16px; padding:10px 12px; font-family:'Roboto Mono'; font-size:11px`.
Key `color:#74797f; white-space:nowrap`; value `vStyle` 4148: `minWidth:0;
whiteSpace:nowrap; overflow:hidden; textOverflow:ellipsis`, colour `WARN` when
the value matches `/MISSING|not responding|not run|—/`, else `#dfe2e5`.

Other key/value grids, all `auto minmax(0,1fr)` but with different gaps:

| Where | Gap | Font | Line |
|---|---|---|---|
| System Info group | `5px 16px` | mono 11px | 688 |
| Network detail | `6px 16px` | mono 11.5px | 605 |
| Properties modal | `7px 14px` | mono 11px | 917 |

### 15.3 Key/value **row** (flex, not grid)

Monitor overview (3610) and Settings about (3941):

| | Monitor overview | Settings about |
|---|---|---|
| padding | `9px 13px` | `11px 13px` |
| font | Roboto Mono `11px` | Public Sans `12px` |
| separator | `borderTop: i ? '1px solid #1c2024' : 'none'` | same |
| layout | `display:flex; justifyContent:space-between; gap:14px` | same |
| key colour | `#9ba0a6` (template 214) | `#9ba0a6` (template 734) |
| value | `textAlign:right; wordBreak:break-all`, `BAD` when flagged | `textAlign:right`, `#dfe2e5` |

Utility rows (`row()`, 1782–1784) are a third shape: `display:flex; gap:10px;
alignItems:baseline`; key `flex:0 0 96px`, Roboto Mono `10.5px`, `#74797f`
(template 558); value `flex:1; minWidth:0`, Roboto Mono `11.5px`,
`whiteSpace:pre-wrap; wordBreak:break-all`.

---

## 16. Overlays

### 16.1 Popover / menu surface

| Surface | Width | Background | Border | Radius | Shadow | Padding | Animation | Line |
|---|---|---|---|---|---|---|---|---|
| Window menu | `224px` | `#1c2024` | `1px #22262b` | `13px` | `0 16px 40px rgba(0,0,0,.66)` | `5px` | `zpop .1s ease-out` | 88 |
| Context menu | `218px` | `#1c2024` | `1px #22262b` | `13px` | `0 16px 38px rgba(0,0,0,.68)` | `5px` | `zpop .08s ease-out` | 4238 |
| Calendar | `288px` | `rgba(22,25,29,.97)` | `1px #22262b` | `14px` | `0 20px 46px rgba(0,0,0,.7)` | `12px` | `zpop .1s` | 841 |
| Quick settings | `240px` | `rgba(22,25,29,.97)` | `1px #22262b` | `14px` | `0 20px 46px rgba(0,0,0,.7)` | `10px` | `zpop .1s` | 854 |
| Notifications | `312px` | `rgba(26,29,34,.98)` | **`1px #2a2f35`** | **`16px`** | `0 22px 52px rgba(0,0,0,.7)` | `6px` (list) | `zpop .1s` | 812 |
| Command palette | `min(520px,74%)` | `#1c2024` | `1px #22262b` | `15px` | `0 26px 62px rgba(0,0,0,.74)` | `6px` (list) | `zpop .11s` | 878 |

The three floating popovers (calendar, quick settings, notifications) add
`backdrop-filter:blur(18px)`. Window menu and context menu are opaque.

`@keyframes zpop` (line 14):
`from{opacity:0;transform:translateY(-6px) scale(.98)} to{opacity:1;transform:translateY(0) scale(1)}`.

Window-menu position (88): `left:8px; top:44px; z-index:40` — i.e. anchored under
the hamburger, 8 px in from the frame, 8 px below the 36 px bar.
Context menu position (4238): `left/top` from the pointer, `z-index:78`.

### 16.2 Menu item row

`mi()` 3061–3062 (window menu) and `ctxItems` 4280–4281 (context menu):

| Property | Window menu | Context menu |
|---|---|---|
| layout | `display:flex; alignItems:center` | same |
| **gap** | **`16px`** | **`14px`** |
| padding | `7px 10px` | `7px 10px` |
| radius | `9px` | `9px` |
| font-size | `12px` | `12px` |
| colour | `#dfe2e5` | `#dfe2e5` |
| shortcut | `color:#74797f; fontSize:10.5px; whiteSpace:nowrap` | identical |

Same widget in every respect but the gap. See §20.8.
The label is wrapped in `<span style="flex:1">` (template 90, 901) so the
shortcut hugs the right edge.

### 16.3 Modal dialog

| Modal | Width | Radius | Shadow | Top | Line |
|---|---|---|---|---|---|
| Properties | `334px` | `14px` | `0 20px 50px rgba(0,0,0,.72)` | `44%` | 915 |
| Rename | `314px` | `14px` | `0 20px 50px rgba(0,0,0,.72)` | `42%` | 925 |
| Dialog (alert) | `370px` | `14px` | `0 22px 54px rgba(0,0,0,.74)` | `40%` | 939 |

All: `position:absolute; left:50%; transform:translate(-50%,-50%);
background:#1c2024; border:1px solid #22262b; overflow:hidden;
animation:zpop .1s ease-out`. z-index 84 / 84 / 90.

**Header** (Properties, Rename): `height:42px; display:flex; align-items:center;
justify-content:center; font-size:12.5px; font-weight:700;
border-bottom:1px solid #22262b`.
**Dialog has no header** — a centred `14px/700` title with `margin-bottom:7px`
inside a `18px 18px 14px` block, body `12px`, `#9ba0a6`, `line-height:1.6`,
`white-space:pre-wrap` (940–942).

**Footers**:
- Properties (920): `display:flex; justify-content:flex-end; padding:0 13px 13px`; one `#22262b` button.
- Rename (931): same, `gap:8px`, Cancel `#22262b` + Rename `ACC`.
- Dialog (944): full-width `height:40px; display:grid; place-items:center;
  border-top:1px solid #22262b; font-size:12.5px; font-weight:700;
  color:#cdf25a`, **hover `background:#22262b`** — one of the file's three hovers.

There is **no backdrop scrim on any of the three modals.** Only the command
palette (877, `rgba(5,6,7,.5)`) and the overview (748, `rgba(7,8,10,.86)` +
`blur(22px)`) dim what is behind them.

### 16.4 Toast

**Container** (905): `position:absolute; z-index:86; left:50%; bottom:80px;
transform:translateX(-50%); display:flex; flexDirection:column; gap:7px;
width:300px`.
**Toast** `style` 4230–4232:
```
display:flex; gap:11px; padding:12px 13px;
background:rgba(26,29,34,.97); backdropFilter:blur(16px);
border:1px solid #22262b; borderRadius:14px;
boxShadow:0 14px 34px rgba(0,0,0,.62); cursor:default;
animation:ztoast .16s ease-out
```
**Icon** 4233: `22×22; borderRadius:14px; display:grid; placeItems:center;
flex:'0 0 22px'; fontFamily:'Roboto Mono'; fontSize:10px; color:#fff`,
background = kind colour (`WARN` / `BAD` / `ACC`); glyph icon at 16 px.
**Text** (909): title `12px/700 #eef0f2`; body `11px #9ba0a6; line-height:1.4;
word-break:break-word`.

`@keyframes ztoast` (line 20): `from{opacity:0;transform:translateY(10px)} to{…}`.
Toasts auto-dismiss after **4200 ms** (2362) and are capped at **3** (`slice(-3)`, 2358).

### 16.5 Notification row

`noteList` 3965–3970: `display:flex; gap:10px; padding:9px 10px;
borderRadius:11px`; error kind gets `background:rgba(212,105,90,.07)`.
Icon `20×20 / r12 / rgba(255,255,255,.06)`, glyph 14 px tinted by kind.
Title `11.5px/700 #eef0f2` ellipsis; timestamp Roboto Mono `9.5px #5c6167`;
body `11px #9ba0a6; line-height:1.4; word-break:break-word` (825–829).

Panel header (813): `display:flex; align-items:center; gap:9px;
padding:11px 13px 9px; border-bottom:1px solid #22262b`; title `12px/700`;
count Roboto Mono `9.5px #5c6167`; Clear button (3961).
List body (819): `max-height:330px; overflow-y:auto; padding:6px`.
Empty state (834): `padding:26px 12px; text-align:center; font-size:11.5px;
color:#5c6167`.

Notification history is capped at **60** (2361); toasts at 3.

### 16.6 Command palette

Overlay (877): `z-index:92; inset:0; background:rgba(5,6,7,.5);
justify-content:center; align-items:flex-start; padding-top:78px`.
Panel (878): `width:min(520px,74%)`, `#1c2024`, `1px #22262b`, `r15`,
`0 26px 62px rgba(0,0,0,.74)`, `zpop .11s`.
Input (879): `width:100%; box-sizing:border-box; height:44px; padding:0 15px;
background:transparent; border:0; border-bottom:1px solid #22262b;
font-size:13px; color:#eef0f2; outline:none`.
List (880): `max-height:322px; overflow-y:auto; padding:6px`.
Item (3513–3516): `display:flex; alignItems:center; gap:10px; padding:8px 9px;
borderRadius:11px; fontSize:12.5px`; selected `background:ACC; color:INK`,
else `transparent`/`#dfe2e5`.
`kindStyle`: `flex:'0 0 52px'`, Roboto Mono `9px`, `letterSpacing:.06em`,
`textTransform:uppercase`, colour selected `rgba(255,255,255,.8)` else `#74797f`.
Hint (885): Roboto Mono `10px #74797f`.
Footer (892): `border-top:1px solid #22262b; padding:7px 13px; display:flex;
gap:14px; font-size:10.5px; color:#74797f`.
Empty (889): `padding:20px; text-align:center; font-size:12px; color:#74797f`.

Results are capped at **40**, and with no query the list shows the first **14**
(3340).

---

## 17. Chart

### 17.1 CPU sparkline — System Monitor Resources

**Lines** template 189–194; data 3248–3249, 3588.

```html
<svg viewBox="0 0 300 66" preserveAspectRatio="none"
     style="width:100%;height:66px;display:block">
  <line x1="0" y1="{16.5|33|49.5}" x2="300" y2="…" stroke="#1c2024" stroke-width="1">
  <polyline fill="rgba(184,232,56,.16)" stroke="none" points="{cpuArea}">
  <polyline fill="none" stroke="#b8e838" stroke-width="1.4" points="{cpuPoints}">
</svg>
```

| Part | Value |
|---|---|
| viewBox | `0 0 300 66`, `preserveAspectRatio="none"` (stretches horizontally) |
| rendered height | `66px` |
| grid lines | y = `16.5`, `33`, `49.5` (quarters), stroke `#1c2024` width `1` |
| area fill | `rgba(184,232,56,.16)` |
| line | stroke `#b8e838` **hard-coded, not `ACC`**, width `1.4` |
| area closure | `'0,66 ' + pts + ' 300,66'` (3588) |
| header row | template 188: label `12px/700`; value Roboto Mono `11px` `#cdf25a` |
| footer row | template 194: Roboto Mono `9.5px` `#5c6167`, left `histSpan`, right `now` |
| history | 40 samples; `histSpan = '-' + 40*interval + ' s'` (3589) |

**Point mapping (3249) has a 1.9× gain and clips:**
```js
(66 - v / 100 * 66 * 1.9).toFixed(1)
```
```
$ node -e "const y=v=>66-v/100*66*1.9; …"
cpu=50%   -> y=3.3
cpu=52.6% -> y=0.0
cpu=53%   -> y=-0.5   CLIPPED (above viewBox top)
cpu=100%  -> y=-59.4  CLIPPED
threshold v = 52.63%
```
The SVG root clips by default, so **any CPU reading above 52.63 % renders as a
flat line pinned to the top of the graph.** This is a bug in the reference, not
a style. A port should either drop the 1.9 or clamp. Recorded here so a toolkit
author does not faithfully reproduce it.

---

## 18. Indicators and inputs

### 18.1 Status dot

| Widget | Size | Radius | Colour | Glow | Line |
|---|---|---|---|---|---|
| Activities dot | `5×5` | **`5px`** | `ACC`, or `INK` when overview is open | `0 0 7px ACC` when closed | 3377 |
| Notification badge | `5×5` | **`4px`** | `BAD` if any error else `ACC` | — | 3958 |
| Dock running dot | `3.5×3.5` | `7px` | focused `#fff`, else `rgba(255,255,255,.7)` | — | 3432 |
| Service unit dot | `7×7` | `5px` | active `OK`, failed `BAD`, activating `WARN`, else `#474b50` | `0 0 7px <col>` when active | 3812 |
| Network iface dot | `7×7` | `5px` | up `OK`, down `#474b50` | `0 0 7px OK` when up | 4037 |

Two 5 px dots with radii 4 and 5 — both fully round in practice, but the
literals disagree. The dock dot's radius (7) exceeds its size (3.5) entirely.

Service dot also animates: `zpulse 1s ease-in-out infinite` while `activating`
(3814). `@keyframes zpulse` (line 17): `0%,100%{opacity:.55} 50%{opacity:1}`.
Dock idle dots use `zpulse 2.6s` (3434).

### 18.2 Badge pill

| Widget | Padding | Radius | Font | Line |
|---|---|---|---|---|
| System Info group badge | `3px 8px` | `8px` | mono `9.5px` | 4145 |
| Game pid badge | `3px 8px` | `8px` | mono `9.5px` | 3156 |
| Files mount flag | `3px 8px` | **`9px`** | sans **`10.5px`** | 3546 |
| Service unit state | *(none)* | — | mono `9.5px`, `.04em`, uppercase | 3815 |
| `heapStyle` (top bar) | *(none)* | — | mono `9.5px`, `.04em`, uppercase | 3448 |

Mount-flag colours: writable `rgba(169,227,75,.14)` / `OK`; read-only
`rgba(255,106,80,.14)` / `BAD`.

### 18.3 Text inputs — eight variants

| Input | Height | Padding | Background | Border | Radius | Font | Line |
|---|---|---|---|---|---|---|---|
| Files search | `21px` (`width:128px`) | `0 9px` | `#090a0c` | `1px #1c2024` | `14px` | sans `11px` | 122 |
| Editor find | `23px` (`flex:1`) | `0 9px` | `#090a0c` | `1px #1c2024` | `10px` | sans `11.5px` | 229 |
| Utility field | `26px` (`flex:1`) | `0 10px` | **`#0d0f12`** | **`1px #232830`** | `9px` | **mono `11.5px`** | 1775 |
| Rename | `28px` | `0 9px` | `#090a0c` | `1px #1c2024` | `10px` | **mono `12px`** | 929 |
| Calculator expr | `30px` (`width:100%`) | `0 10px` | **`#07080a`** | `1px #1c2024` | `9px` | **mono `12.5px`** | 363 |
| Lock password | `30px` (`width:206px`) | `0 12px` | `rgba(255,255,255,.09)` | `1px rgba(255,255,255,.14)` | `16px` | sans `12.5px`, `text-align:center` | 953 |
| Overview search | `34px` (`width:min(400px,52%)`) | `0 15px` | `rgba(255,255,255,.09)` | `1px rgba(255,255,255,.13)` | `17px` | sans `13px`, `text-align:center`, `line-height:32px` | 750 |
| Palette | `44px` (`width:100%`) | `0 15px` | `transparent` | `0` + `border-bottom:1px #22262b` | — | sans `13px` | 879 |

All set `outline:none`, `spellcheck="false"`, and `color:#dfe2e5` or `#eef0f2`.
Global rule at line 23: `input,textarea{font:inherit;color:inherit}` — every
input inherits `12.5px Public Sans` unless it overrides.

**There is no focus ring anywhere.** No input declares `:focus`, no
`box-shadow` on focus, no border change. The only focus signal in the whole
reference is the window frame's accent ring.

Eight heights (21/23/26/28/30/30/34/44), five radii (9/10/14/16/17), three
background families. The `#232830` border on the utility field appears exactly
once in the file.

### 18.4 Placeholder text

Set via the `placeholder` attribute only; no `::placeholder` rule exists, so it
renders at the browser default. Placeholders in use: `"Search rd0…"` (122),
`"Search…"` (229), `"12 * (34 + 5) >> 2"` (363), `"Type to search…"` (750),
`"Run a command, open a file, jump to a diagnostic…"` (879), `"Password"` (953),
plus per-utility hints from `F(label, key, ph)` (1785).

---

## 19. Shell chrome (desktop, top bar, dock, overview)

### 19.1 Top bar ("island")

**Line 791.** `position:absolute; z-index:62; top:8px; left:50%;
transform:translateX(-50%); display:flex; align-items:center; gap:3px;
height:32px; padding:0 4px; border-radius:16px;
background:rgba(12,14,16,.82); backdrop-filter:blur(20px) saturate(1.3);
box-shadow:0 2px 20px rgba(0,0,0,.6), 0 0 0 1px rgba(184,232,56,.09),
           0 1px 0 rgba(255,255,255,.05) inset`.

Divider (793, 795, 797, 804): `width:1px; height:15px;
background:rgba(255,255,255,.08)`.

Items — all `height:24px; borderRadius:12px`, active `ACC`/`INK`:

| Item | Padding | Gap | Font | Line |
|---|---|---|---|---|
| Activities | `0 12px` | 7 | sans `11.5px` **w600** `.01em` | 3374 |
| Clock | `0 12px` | — | **mono** `11px` **w500** `.02em` | 3437 |
| Tray aggregate | **`0 11px`** | 7 | sans `11px` | 3445 |
| Bell | `20×17px`, radius `7px` | — | icon 13 px | 3955 |

Workspace pips (3383): `height:7px; borderRadius:5px`,
width `20px` when current else `7px`, `transition:'width .16s ease, background .16s ease'`;
background current `ACC`, occupied `rgba(255,255,255,.34)`, empty `rgba(255,255,255,.13)`.

`TOP = 48` and `DOCK = 62` (line 1213) are the reserved strips used by snapping
and window placement — note these are **not** the bar's own 32 px height; they
include the 8 px offset and margin.

### 19.2 Dock

**Line 865.** `position:absolute; z-index:63; left:50%; bottom:9px;
transform:translateX(-50%); max-width:calc(100vw - 24px);
overflow-x:auto; display:flex; align-items:center; gap:5px; padding:5px 7px;
background:rgba(18,21,24,.82); backdrop-filter:blur(22px);
border:1px solid rgba(255,255,255,.11); border-radius:16px;
box-shadow:0 12px 34px rgba(0,0,0,.5), 0 0 0 1px rgba(0,0,0,.3)`.

Divider (872): `width:1px; height:22px; background:rgba(255,255,255,.14); margin:0 2px`.

**Dock item** 3424–3431: `width:33px; height:33px; borderRadius:13px;
display:grid; placeItems:center; fontSize:14px; flex:'0 0 33px';
fontFamily:'Roboto Mono'; position:relative; boxSizing:'border-box'`.
Focused+open `background:ACC` + `border:1px solid rgba(255,255,255,.22)`;
otherwise `rgba(255,255,255,.085)` + `1px rgba(255,255,255,.05)`.
Press animation `zpress .25s ease` for 250 ms (3421, 3426).
`@keyframes zpress` (16): `0%{scale(1)} 45%{scale(.88)} 100%{scale(1)}`.

**Bug:** the object sets `color` twice (3429 `? INK : '#fff'`, then 3431
`color:'#fff'`). In a JS object literal the last key wins:
```
$ node -e "console.log(JSON.stringify({color:'INK', boxSizing:'border-box', color:'#fff'}))"
{"color":"#fff","boxSizing":"border-box"}
```
So a focused dock icon is always `#fff` on the accent. See §20.1.

Apps-grid button `gridBtnStyle` (3411) is the same `33×33 / r13 / 14px` shape,
also with the border pair.

Dock pin order (3416): `term files edit mon log hex render tty fb arch img net
clock sysinfo set` — 15 pinned, then any other running app appended.

### 19.3 Desktop icon

`deskIcons` 3986–3997, trash 4001–4011.

| Part | Value |
|---|---|
| tile | `position:absolute; zIndex:3; width:68px; display:flex column; alignItems:center; gap:5px; padding:8px 4px; borderRadius:12px` |
| tile selected | `background:rgba(184,232,56,.14)`, `border:1px solid rgba(184,232,56,.3)` |
| tile idle | `background:transparent`, `border:1px solid transparent` |
| icon plate | `38×38; borderRadius:12px; display:grid; placeItems:center; boxShadow:'0 3px 10px rgba(0,0,0,.45)'` |
| plate selected | `rgba(184,232,56,.16)` + `1px rgba(184,232,56,.28)` |
| plate idle | `rgba(18,21,24,.72)` + `1px rgba(255,255,255,.06)` |
| glyph | 22 px, colour selected `INK` else `#c4c9cf` |
| label | `10.5px; textAlign:center; lineHeight:1.3; maxWidth:66px; nowrap/hidden/ellipsis; textShadow:'0 1px 3px rgba(0,0,0,.85)'`; colour selected `#eef0f2` else `#c4c9cf` |

Trash is the same shape with `right:22px; bottom:(62+22)=84px`, plate tinted
`rgba(245,185,60,.12)` + `1px rgba(245,185,60,.26)` when non-empty, glyph 20 px
`WARN`. Tidy-icons layout is `x:22, y:16 + i*88` (4248).

### 19.4 Drag ghost

`dropHintStyle` 4025–4029: `position:absolute; zIndex:96; left:x+12; top:y+10;
padding:6px 11px; borderRadius:10px; pointerEvents:none; fontFamily:'Roboto Mono';
fontSize:11px; whiteSpace:nowrap; background:rgba(184,232,56,.92);
color:'#0d1005'; fontWeight:700; boxShadow:'0 8px 22px rgba(0,0,0,.6)'`.

### 19.5 Snap preview

`snapStyle` 3366–3370: `position:absolute; zIndex:50;
background:rgba(184,232,56,.2); border:2px solid rgba(184,232,56,.6);
borderRadius:14px; pointerEvents:none`. Zones: `top` = full work area,
`left`/`right` = half width. Work area is `TOP` (48) to `H - TOP - DOCK` (62).

### 19.6 Activities overview

Backdrop (748): `z-index:58; inset:0; background:rgba(7,8,10,.86);
backdrop-filter:blur(22px); padding:52px 0 82px; animation:zov .16s ease-out`.
`@keyframes zov` (19): `from{opacity:0;transform:scale(1.03)} to{…}`.

**Workspace thumbnail** 3404–3408: `96×56; borderRadius:10px; display:flex;
flexWrap:wrap; alignContent:flex-start; gap:3px; padding:5px`;
current `rgba(255,255,255,.16)` + `1px rgba(255,255,255,.35)`, else
`rgba(255,255,255,.05)` + `1px rgba(255,255,255,.09)`.
Chips inside: `26×16; borderRadius:6px; background:rgba(255,255,255,.3)`, max 6.

**Window thumbnail** 3312–3318: `width:168px; display:flex column`.
Head `height:22px; gap:6px; padding:0 8px; background:#22262b;
borderRadius:'8px 8px 0 0'; color:#dfe2e5`; head icon mono `9px` `opacity:.8`,
head title `10.5px/700` ellipsis.
Body `height:84px; background:#101215; borderRadius:'0 0 8px 8px';
display:grid; placeItems:center; boxShadow:'0 10px 26px rgba(0,0,0,.5)'`;
glyph 30 px weight 1.4, colour `#4e555e`.
Label `textAlign:center; fontSize:11px; color:#b9bec4; paddingTop:7px`.
Row container (774): `flex-wrap; justify-content:center; gap:14px`.

**App grid** (764): `repeat(auto-fill, minmax(92px,1fr)); gap:10px;
max-width:640px; margin:0 auto` — same `minmax(92px,1fr)` as the Files icon view
(142) but `gap:10px` instead of `5px`.
Tile (766): `flex column; alignItems:center; gap:6px; padding:7px 5px;
borderRadius:14px`, **hover `rgba(255,255,255,.09)`**.
Icon plate (3304): `42×42; borderRadius:13px; background:rgba(255,255,255,.1);
color:#eef0f2`; file hits get `rgba(255,255,255,.07)` / `#9ba0a6` (3308).
Label (768): `height:26px; display:flex; align-items:flex-start;
justify-content:center; font-size:10.5px; line-height:1.32; text-align:center;
color:#eef0f2; text-wrap:pretty`. The fixed 26 px label box is what keeps the
grid rows aligned when a name wraps to two lines.

Category chips `ovCats` (3396) — see §13.1.
Empty state (785): `text-align:center; color:#74797f; font-size:13px; padding-top:40px`.

### 19.7 Calendar

Panel §16.1. Month title (842): `13px/700; margin-bottom:9px`.
Grid (843): `repeat(7,1fr); gap:2px`.
Head cell (844): `text-align:center; font-size:10px; color:#74797f; padding:3px 0`.
Day cell (3349–3350): `textAlign:center; fontSize:11px; padding:4px 0;
borderRadius:9px`; today `background:ACC; color:INK; fontWeight:700`,
else `transparent` / `#b9bec4` / 400.
Footer (847): `margin-top:10px; padding-top:10px; border-top:1px solid #22262b;
font-family:'Roboto Mono'; font-size:10.5px; color:#9ba0a6; line-height:1.7`.

### 19.8 Quick-settings panel

Panel §16.1. Header (855): `gap:9px; padding:4px 4px 10px`; mark 30×30 r17
`ACC`/`INK`; name `12.5px/700`; sub `10.5px #9ba0a6`.
Toggle row (3463–3467): `display:flex; alignItems:center; gap:9px;
padding:8px 9px; borderRadius:11px; fontSize:12px`; on
`background:rgba(255,255,255,.06)`; icon 22×22 r14, on `ACC`/`INK`, off
`rgba(255,255,255,.09)`/`#fff`; state text `10.5px #9ba0a6`.
Divider (860): `height:1px; background:#22262b; margin:7px 2px`.
Item row (3473): `padding:8px 9px; borderRadius:11px; fontSize:12px; color:#dfe2e5`.

### 19.9 Lock screen and boot screen

**Lock** (948–955): `z-index:94; inset:0; background:rgba(7,8,10,.97);
backdrop-filter:blur(10px); gap:14px`.
Clock `62px/700; letter-spacing:-.04em; color:#eef0f2; line-height:1`.
Date `13px #9ba0a6`. Brand tile `46×46; borderRadius:20px; background:#b8e838;
display:grid; place-items:center; font-size:19px; font-weight:700; color:#fff;
margin-top:18px`. Unlock link `11.5px #cdf25a`. Hint Roboto Mono `10.5px #474b50`.

**Boot** (959–969): `z-index:95; inset:0; background:#060708; padding:26px 30px;
font-family:'Roboto Mono'; font-size:12px; line-height:1.55`.
Header `gap:13px; margin-bottom:18px`; mark `flex:0 0 30px; color:#b8e838`
(icon 30 px weight 1.5); wordmark `'Public Sans'` `15px/700 .02em #eef0f2
line-height:1`; subtitle `9.5px .17em uppercase #474b50 line-height:1`.
Line (968, 4313–4315): `display:flex; gap:8px`; tag `[ ok/fail/warn ] <name>`
padded to 9 chars, `white-space:pre`, colour `OK`/`BAD`/`WARN`; text `#9ba0a6`
`white-space:pre-wrap`.
"click to skip" (969): `position:absolute; left:30px; bottom:24px;
font-size:9.5px; letter-spacing:.16em; text-transform:uppercase; color:#22262b`.

### 19.10 Scrollbar

Lines 25–28: `width:9px; height:9px`; track `transparent`;
thumb `background:#22262b; border:2px solid transparent;
background-clip:padding-box; border-radius:11px`; thumb hover `#474b50`.
The 2 px transparent border with `padding-box` clipping makes the visible thumb
5 px wide inside a 9 px gutter.

### 19.11 Accent swatch

`accents` 3909–3911: `26×26; borderRadius:16px; background:<colour>;
cursor:default`; selected ring `0 0 0 2px #14171a, 0 0 0 4px <colour>` — a
2 px gap in the card colour, then a 2 px ring in the swatch's own colour.

**The accent list is declared twice and the two disagree:**
```
line 1212: const ACCENTS = ['#b8e838','#4ce0b3','#57b6ff','#8f7bff','#e86ec4'];
line  974: props options   ["#b8e838","#a9e34b","#8f7bff","#f5b93c","#c2402c"]
```
Only `#b8e838` and `#8f7bff` appear in both. The swatch row renders `ACCENTS`;
the props panel offers the other list. See §20.9.

### 19.12 Calculator keypad

Grid (369): `repeat(4,1fr); gap:5px; padding:10px`.
Key (3748–3751): `height:34px; display:grid; placeItems:center;
borderRadius:9px; fontFamily:'Roboto Mono'; fontSize:12.5px`.
`=` → `ACC`/`INK`; `C` → `rgba(212,105,90,.16)`/`#ff9c88`; rest →
`rgba(255,255,255,.05)`/`#dfe2e5`. Layout is 6 rows × 4 (3741).
Result readout (3735): Roboto Mono `25px` w500 `line-height:1.1`, `#eef0f2` or
`BAD` on error.
Tape row (374–377): `gap:8px; padding:3px 0; font-family:'Roboto Mono';
font-size:10.5px; border-bottom:1px solid #101215`; expr `#74797f`,
out `#dfe2e5`, bytes `#474b50`.

### 19.13 Clock readouts and lap chips

Zone time (4072): Roboto Mono `21px` w500 `-.02em`; local `#eef0f2`, others `#9ba0a6`.
Zone block (627): `flex column; align-items:center; gap:3px; min-width:88px`;
row gap `26px`, `padding:18px 14px 14px`, `background:#0f1114`.
Stopwatch (4075): Roboto Mono `26px` w500 `-.02em`; running `ACC` else `#eef0f2`;
`minWidth:124px`.
Lap chip (4090): Roboto Mono `10.5px; padding:4px 9px; borderRadius:9px;
background:rgba(255,255,255,.05); color:#9ba0a6`. Capped at 8 (`slice(-8)`, 4086).
Timer row (657): `gap:10px; padding:9px 11px; background:#14171a`;
time Roboto Mono `14px` w500 `minWidth:72px`, `WARN` under 5 s.

### 19.14 Empty states

| Where | Style | Line |
|---|---|---|
| Archive Manager | hero icon 40 px `#2a2f35` w1.4 `opacity:.9`; text `12px #9ba0a6; text-align:center; line-height:1.6; max-width:340px; text-wrap:pretty`; container `gap:12px; padding:22px` | 504–507, 4158 |
| Notifications | `padding:26px 12px; text-align:center; font-size:11.5px; color:#5c6167` | 834 |
| Command palette | `padding:20px; text-align:center; font-size:12px; color:#74797f` | 889 |
| Overview | `text-align:center; color:#74797f; font-size:13px; padding-top:40px` | 785 |
| Network ping | `color:#474b50` inline text | 615 |

---

## 20. Inconsistencies index

Every place the reference defines the same shape two different ways. A toolkit
author has to pick one; these are the picks that need a decision.

**20.1 — Ink on accent is written four different ways.**
The correct value is `INK` (3039–3045), computed for contrast. But:
`spinBtnStyle` (3707) and `renameOkStyle` (4302) hard-code `'#fff'` on an `ACC`
background; the window hamburger active state (3121) does the same; the dock
item (3429/3431) computes `INK` and then **overwrites it with `'#fff'` via a
duplicate object key**, so the computation is dead code. Separately, three
near-identical dark inks exist: `#0c1005` (`INK`, 3044 and 1559), `#0d1005`
(utility button 1781, drag ghost 4028), `#0c0f05` (close-button hover, line 84).
**Pick one dark ink, and always route accent-background text through it.**

**20.2 — List-row selection has three treatments.**
Files and Monitor: `rgba(184,232,56,.15)` tint + `inset 2px 0 0 ACC` left bar +
`#eef0f2` text. Archive Manager: **solid `ACC` fill** + `INK` text, no bar.
Files icon view: tint + `inset 0 0 0 1px ACC` full ring. Zebra striping is
`rgba(255,255,255,.014)` in Monitor and `rgba(255,255,255,.012)` in Archive —
one thousandth apart, visually identical, almost certainly a typo.

**20.3 — Sidebar-row selection repeats the same split.**
Files tree and Settings nav use tint + inset bar; Network interfaces use solid
`ACC` + `INK`. Same widget, two idioms, in the same app family.

**20.4 — The stat card strip disagrees on both track and padding.**
`minmax(88px)` Framebuffer, `minmax(84px)` Console, `minmax(96px)` Archive,
`minmax(88px)` Image Viewer; padding `7px 9px` / `6px 9px` / `7px 10px` /
`7px 10px`. Nothing about the content explains the spread.

**20.5 — `pcolGrid` is missing the scrollbar gutter its siblings have.**
`fcolGrid` (3583) and `archGrid` (4162) both set `padding:'0 15px 0 4px'` so the
header aligns with a body that has a 9 px scrollbar. `pcolGrid` (3580) sets no
padding at all, so System Monitor's column headers sit 15 px off from its rows.

**20.6 — Section headings: six instances, five treatments.**
The Files pair (`10.5px / 700 / .05em / mixed case / #74797f`) versus the
majority shape (`9–11px / 400 / .05–.09em / uppercase / #5c6167`), which itself
comes in three sizes.

**20.7 — Two reds for "destructive".**
`BAD` is `#ff6a50`, used as `rgba(255,106,80,…)` in `killStyle` and the mount
flag. But `rgba(212,105,90,…)` (`#d4695a`) is used in four other destructive
contexts (calc `C`, stopwatch stop, service stop, notification error row) and
never appears as a hex constant.

**20.8 — The window menu and the context menu are the same widget with different numbers.**
Width 224 vs 218. Shadow `0 16px 40px rgba(0,0,0,.66)` vs
`0 16px 38px rgba(0,0,0,.68)`. Animation `.1s` vs `.08s`. Item gap 16 vs 14.
Everything else is byte-identical.

**20.9 — Two accent palettes.**
`ACCENTS` (1212) `#b8e838 #4ce0b3 #57b6ff #8f7bff #e86ec4` versus the props
declaration (974) `#b8e838 #a9e34b #8f7bff #f5b93c #c2402c`. Note the second
list includes `#a9e34b` (= `OK`) and `#f5b93c` (= `WARN`) as accent options,
which would collide with the semantic colours.

**20.10 — Segmented control has four sizes for four apps.**
See §3. Also Font Atlas alone uses `#07080a` for the container instead of
`#090a0c`.

**20.11 — Meter/progress/segment bars share no radius or track colour.**
Radius 7 (meter) vs 4 (timer, disk mini-bar) vs 7 (disk segment track) with 5 px
segments. Track colour `#090a0c` vs `#0d0f12` vs `#07080a`.

**20.12 — Eight text-input geometries, no focus state on any of them.**
See §18.3. Heights 21/23/26/28/30/34/44, radii 9/10/14/16/17. `#232830` is used
as a border once and nowhere else.

**20.13 — Window control buttons share geometry but not glyph size.**
`26×26 / r16` on all three, `fontSize` 10 / 11 / 12 and icon 13 / 14–15 / 14.

**20.14 — Card border colour and radius.**
Eight cards use `1px #1c2024 / r13`; System Info uses `1px #1a1d21 / r12`;
Network's ping panel uses `1px #1a1d21 / r10` on `#0d0f12`.

**20.15 — Toast and notification panel surfaces nearly match but not quite.**
Toast `rgba(26,29,34,.97)` + `1px #22262b` + `r14`; notification panel
`rgba(26,29,34,.98)` + `1px #2a2f35` + `r16`.

**20.16 — Toolbar heights: 30 / 32 / 34 / 36 / 38.**
Five values across fourteen bars (§5). Status bars add a sixth and seventh at
25 and 26 (§6).

**20.17 — Two hard-coded accents that ignore the accent setting.**
The slider thumb (`#b8e838`, lines 30–31) and the CPU sparkline stroke
(`#b8e838`, line 192). Changing the accent leaves both green.

**20.18 — The CPU sparkline clips above 52.63 %.**
See §17.1. A 1.9× gain into a viewBox that does not accommodate it.

---

## 21. Things a toolkit author will trip over

1. **There are no hover states.** Five rules total in 4338 lines (§0). If the
   toolkit adds hover feedback, that is new design, not a port.
2. **There is no focus ring on any input, button or list.** The window frame's
   accent ring is the only focus affordance in the file. Anything
   keyboard-navigable in zlOS needs a focus treatment invented from scratch.
3. **`cursor` is `default` almost everywhere**, including on things that are
   clickable — 75 occurrences.
   ```
   $ grep -c "cursor:'default'\|cursor:default" docs/design/ds-reference.html
   75
   $ grep -n "cursor:'pointer'\|cursor:pointer" docs/design/ds-reference.html
   30,31   input[type=range] thumb (webkit + moz)
   3232    Files column header
   3276    Monitor column header
   3479    terminal line that jumps to a diagnostic
   3495    Terminal tab close ✕
   3621    Editor tab close ✕
   ```
   Plus `cursor:text` on the terminal body (103) and `cursor:grab` on the
   renderer viewport (293). Everything else — every button, list row, tab,
   toggle, nav item — is `default`.
4. **Half of every widget is inline in the template and half is computed.**
   Reading only `renderVals()` gives you items without containers; reading only
   the template gives you containers without states.
5. **Geometry constants are duplicated rather than shared.** `19.2px` (editor
   line height) appears in three places; `26px` (window button) in three;
   the archive grid track string in two; `#0b0d0f` borders in dozens. Any
   toolkit that parameterises these will diverge from the file the first time
   one of the copies would have been changed alone.
6. **`TOP = 48` and `DOCK = 62`** (line 1213) are the layout reserves, and they
   do **not** equal the visual heights (32 px bar at `top:8px`, 43 px dock at
   `bottom:9px`). Snapping, window placement and the trash position all use the
   reserves.
7. **Selection, not hover, is the interaction language.** Every list, tree, tab
   and nav distinguishes state by a tint (`rgba(184,232,56,.15)`) plus an inset
   marker, or by a solid accent fill. Build both; §20.2 says the reference never
   settled on which.
8. **`INK` must be computed, not stored.** Four widgets already got this wrong
   in the reference (§20.1). If the toolkit exposes an accent setting, the ink
   has to be derived at paint time from the WCAG luminance formula at 3039–3045.
9. **Nothing is `!important`, nothing is a class, nothing cascades.** Every
   style is an inline object. Porting to a C toolkit means the style struct is
   the unit, not a stylesheet — which is closer to what zlOS wants anyway.
10. **Window minimum is 320×220** (3340) and windows are absolutely positioned
    with per-app defaults in `APPS` (995–1015); the 53 apps ship with hand-picked
    `w/h/x/y/ws`, not a layout algorithm.
