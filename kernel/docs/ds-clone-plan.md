# Cloning ds.html into zlOS — how this is actually done

The ask: reproduce `/home/roy/Downloads/ds.html` inside zlOS **exactly** — every
app, every colour, every animation, every layout — keeping only zlOS's own
identity underneath. This is the plan for doing that without it drifting,
stalling, or quietly becoming "close enough".

Written 2026-08-19, after three previous attempts at pieces of it lost their
work entirely.

## 1. The one idea the whole project hangs on: make "the same" a number

"Exact same" is unfalsifiable as an instruction. Fifty agents each asked to
"match the reference" will produce fifty readings of it, and nobody will be able
to say which ones are wrong. **Before any app is written, build the oracle:**

1. Render `ds.html` headless at a fixed resolution (the same one zlOS boots at)
   and save it as the reference frame.
2. Boot zlOS at that resolution and `screendump` over QMP — this already works,
   `probe-shot.py` and `probe-catalog.py` do it today.
3. Diff the two **per region**: one box per app window, one per chrome element.
   Emit a percentage per region.

Then every agent's definition of done is *its own region's number*, not its
taste. A wave either moved the number or it did not.

This is the difference between a project that converges and one that produces
53 plausible windows nobody can compare to anything. **Wave 0 builds this and
nothing else.**

Second use, just as important: the same harness run twice a few frames apart
proves an *animation* actually animates. A green build says nothing about
motion — a previous run had 9 of 14 event handlers updating state and never
calling `wm_dmg()`, compiling clean and repainting nothing.

## 2. What cannot be exact — decided up front, not discovered later

Naming these now stops 53 agents from each independently burning a day on them.

**Typography — the real blocker.** The reference uses eight sizes, including
half-pixel steps: 9, 9.5, 10, 10.5, 11, 11.5, 12, 12.5 px. zlOS draws from
*bitmap atlases*, and `font8x16.c` + `font_aa.c` + `font_big.c` + `font_sub.c`
already occupy **~1.6 MB of a 2.6 MB kernel budget**. Eight more atlases cannot
fit. Three options, and one must be chosen before app work starts:

- (a) collapse the eight sizes onto the 3-4 that exist — cheapest, and every
  layout inherits slightly wrong metrics
- (b) scale glyphs at runtime from one high-resolution atlas — one code change,
  quality cost, size cost near zero
- (c) ship more atlases — arithmetically impossible inside the current ceiling

Recommendation: **(b)**, with (a) as the fallback for body text. Whatever is
chosen, record it — this is the single largest source of "why does it not look
identical" and it is a *platform* limit, not an app bug.

**`backdrop-filter: blur(18px)` / `blur(22px)`, and glow layers at 30/34 px.**
No GPU, no gaussian blur. `fb.c` already has a cached blur arena; that is the
approximation. It will not be pixel-identical and should not be chased.

**`cubic-bezier(.2,.85,.3,1)` over 0.2 s.** The compositor is released by a
100 Hz PIT against a 59.998 Hz panel, so easing quantises to ~12 frames. Close,
not exact, and made worse by TCG under load.

Everything else — the palette, radii, shadows, gradients, alpha, layout metrics,
per-app structure — **can** be exact, and should be held to it.

## 3. The size wall, in arithmetic

| | bytes |
|---|---|
| kernel.elf today, **20** apps | 2,209,940 |
| raw-boot ceiling, `CHUNKS = 80` | 2,621,440 |
| headroom | 411,500 |
| 33 more apps at the measured ~20 KB | ~660,000 |

**It does not fit.** `raw_boot.asm`'s `CHUNKS` must rise 80 -> ~112 (3.5 MiB)
with `mkdisk.sh`'s image raised alongside it. That puts the kernel top at
1 MiB + 3.5 MiB = 4.5 MiB, which still clears `raw_entry.S`'s 6 MiB stack and
`arena.c`'s 8 MiB `ARENA_BASE` — but it is the **last raise available** before
the high-RAM map itself has to move. Plan the font decision (§2) with that in
mind: option (c) is not merely expensive, it is impossible.

## 4. Decomposition — by dependency, not by app

The instinct is to fan 53 agents at 53 apps. That produces 53 different button
implementations and destroys the "exact same" goal on day one. The apps are the
*last* phase, not the first.

```
Wave 0   1 agent    the fidelity oracle (§1). Blocks everything.
Wave 1   2-3        foundation: ONE palette source, the radius scale,
                    the easing/animation engine, the blur approximation,
                    the font decision from §2 implemented
Wave 2   3-4        chrome: header bar, dock, workspaces, window frame,
                    the - [] x controls
Wave 3   2-3        THE WIDGET TOOLKIT - tab strips, list rows, stat cards,
                    toolbars, sidebars, status bars, monospace panels
Waves 4-12  6 each  the 53 apps, one app (or tight group) per agent
parallel track      the icon set (independent of app logic)
Final    2-3        integration, full-screen diff, the size/CHUNKS raise
```

**Wave 3 is the bottleneck and it must be serial.** Every app in the reference
is built from the same handful of components. Build those once, prove them
against the oracle once, and the 53 app agents become assembly rather than
invention — which is also what makes them *consistent*, which is what "exact
same" actually requires.

Skipping wave 3 to start apps sooner is the single most likely way this fails.

## 5. One file per agent — the merge rule

Fifty-three agents editing `kernel/src/kernel.zl` is a guaranteed conflict storm.
The rule:

- each app is **its own `kernel/apps_<name>.zl`**, owned by exactly one agent
- the only shared edit is **appending one line** to the registry table
- `kernel.zl` itself is touched by wave 1-3 agents and then frozen for app work

`desktop/app-suite` already established this shape (`apps_registry.zl` +
`apps_common.zl` + category modules) and it is why 20 apps landed without the
tree tearing itself apart. Keep it.

Watch the app-id rule while appending: `APP_CATALOG` is a **dispatch floor** —
`kernel.zl` routes `if id >= APP_CATALOG` into the registry — so two apps
sharing an id compiles clean and misroutes at runtime. It has already happened
once, between two branches that were each individually green.

## 6. Concurrency — what the box actually allows

The user asked for up to ~100 agents. **In total, across waves — yes.
Concurrently — no.** This machine is 8 logical / 4 physical cores and the cap is
`min(16, cores-2)` = **6**. Passing 100 at once does not create 100 lanes; it
creates a queue and thrashes the host. Measured on this box: load 13.2 at 18
Claude processes, and `CLAUDE.md` records the OOM killer terminating an agent at
7.9 GB resident.

Waves of <=6, `cut -d' ' -f1-3 /proc/loadavg` checked before each, and never a
QEMU gate running alongside a full fan-out — a boot that normally takes 12 s
took over 30 s at load 6 and made a gate report a regression that did not exist.

Roughly 15-20 waves at 5-6 agents is the ~100 the user wants, structured so they
finish.

## 7. The rules that exist because they were paid for

- **Commit from the first file.** Three sessions did parts of this task and lost
  *everything* — 90 patches, 3,000+ insertions — by working in worktrees they
  never committed to. Two of the three were recoverable only because a copy
  happened to survive outside `/tmp`.
- **Never `/tmp` as a working copy.** It is wiped on this box. It is what
  destroyed those attempts.
- **A zl call cannot span a newline.** `f(a,\n b)` fails with
  `expected a value (got '\n')`. One such line left an entire finished visual
  pass unbuildable.
- **Compile after every small piece**, never at the end.
- **New fixed-address buffers must pass `check-memmap.sh`.** Overlapping a live
  buffer is this repo's worst defect class.
- **Never pipe a build through `tail` and read `$?`** — that reports tail's
  status, and it is how a tree that did not link once gated green.

## 8. Definition of done

Not "the registry exists". Not "40 of 53". Every app present, internally laid
out like the reference, with the palette, the seven animations, the depth
language and the radius scale in; Settings real; the VFS and three workspaces
in; icons complete — and the **oracle's per-region numbers** low enough across
the board to back the claim, with the §2 exceptions named as platform limits
rather than quietly counted as passes.
