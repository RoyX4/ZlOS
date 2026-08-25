# zlOS vs BoredOS — what the numbers actually say

Written 2026-08-18 after someone asked "how does zl compare to
[boredos.dev](https://boredos.dev)". Every number here was measured, not
recalled. The commands are at the bottom so you can re-run them.

BoredOS is the closest thing to a peer project this repo has: same target
(x86_64, from scratch, bare metal), same era, same "one person plus friends
in their spare time" shape. It is the right thing to be measured against.
It is also, on almost every axis, doing the **opposite** thing — and that
contrast is more useful than any score.

## The one-line version

BoredOS is an **operating system project that ports its userland**.
zlOS is a **language project that grew an operating system and ports nothing**.

BoredOS ships a C compiler (TCC), a libc (mlibc), a TCP/IP stack (lwIP), a
scripting language (Lua), an editor (kilo), a TLS library (BearSSL), an
OpenGL subset (TinyGL), a bootloader (Limine) and DOOM. All of those are
existing projects, ported. That is a legitimate and efficient strategy: it
buys a usable system fast, and porting to a non-POSIX kernel is real work.

zlOS ships none of those because it has no way to accept them. There is no
libc to port against, because the system's language is not C. `kernel.zl`
is compiled by a compiler written in `zl`, which is bootstrapped by a
compiler written in C in this repo. The TCP stack, the HTML parser, the
layout engine, the font rasteriser, the display driver and the filesystem
are all first-party because there was never an option to make them
otherwise.

Neither approach is "better". They answer different questions.
BoredOS answers *"can I build a system people can use?"*
zlOS answers *"can one person build the whole stack, language included?"*

## Measured size

Nonblank lines. First-party means written by the project, not ported.

| | zlOS | BoredOS |
|---|---:|---:|
| Kernel, first-party | 57,666 | 27,346 |
| Userland / desktop, first-party | *(in the kernel figure)* | ~32,400 |
| Language + compilers + stdlib | 44,119 | **0** — TCC is a port |
| **First-party total** | **~101,800** | **~59,700** |
| Vendored / ported, in-tree | **0** | 93,833 (lwIP alone) |
| Vendored / ported, submodules | **0** | ~11 MB of source across 10 upstream projects |

zlOS's 101,785 = 34,602 lines of `.zl` + 67,183 of C/H/S, union across the
eight branch checkouts taking the largest version per path. `main` alone is
78,352. `docs/CODE-MAP.md` quotes 87,527 for the same union because it uses
cloc's `code` count, which strips comments; the two are consistent.

BoredOS's ~59,700 = 27,346 (kernel tree) + 20,700 (Nova compositor, after
subtracting the vendored `stb_image.h` and `stb_truetype.h`) + 2,743 (`bsh`)
+ 4,600 (`coreutils`, after `minimp3.h`) + 3,165 (`netutils`) + 1,184 (`bpm`).

The interesting cell is the zero. 76% of the C sitting in BoredOS's own
tree is lwIP.

## Measured project health

| | zlOS | BoredOS |
|---|---|---|
| Contributors | 1 | 9 (656 / 77 / 35 / 18 / 18 / 14 / 4 / 2 / 1 commits) |
| Commits | — | 826 |
| Stars / forks | — | 252 / 26 |
| Tagged releases | none | 10+, roughly monthly, `26.4` … `26.6.0-RC2` |
| Website / Discord / blog | none | all three |
| Predecessor | — | BrewKernel, started 2023 |
| Repo created | — | 2026-02-04 |

BoredOS has a distribution story and zlOS does not. That is the largest
real gap and it is not a technical one.

## Feature-by-feature, verified

Present in both:

| | zlOS | BoredOS |
|---|---|---|
| SMP / multicore | `smp.c`, `smp_trampoline.S` | LAPIC, per-CPU GS, XSAVE |
| Preemptive scheduler | `sched.c` | round-robin, sleep/wake, IPI |
| PCI enumeration | `pci.c` | yes |
| Framebuffer + compositor | `fb.c`, `wm.c` (1,017), `ui.c` (457) | Nova, win9x-inspired, LibWidget |
| TCP/IP + DNS + HTTP | hand-written, `desktop/browser` branch | lwIP |
| Web browsing | `html.c`, `layout.c`, `browser.c` | `netutils` over lwIP |
| Filesystem | zlfs (`fs.c`), survives power cycles | VFS over FAT32, ext4, ProcFS, SysFS |
| Storage driver | NVMe | AHCI (SATA) |
| Boots real hardware | yes, Comet Lake laptop | yes, per README |
| Boots without GRUB | 3 ways: BIOS multiboot, own 512-byte `raw_boot.asm`, native UEFI `efi.c` | Limine |

BoredOS has and zlOS does not: ELF64 loader, package manager (`bpm`) with a
community repo, AC97 audio, ext4, FAT32, PTYs, Unix domain sockets, shared
memory, 10 virtual terminals, a C SDK for third-party apps, TLS, DOOM.

zlOS has and BoredOS does not:

- **A language.** Five execution engines — tree-walking interpreter (the
  reference), two archived C backends, an LLVM backend, and a hand-written
  x86-64 → ELF assembler with no C compiler anywhere in the output path.
- **A real Intel Gen9 display driver.** Not a firmware-handoff framebuffer:
  DPLL programming proven at 720p/1080p/1440p/4K60 with the hardware
  restored afterwards, AUX/DPCD transactions, link training offsets
  confirmed against a live trained link, panel power delays read from
  firmware. Pixel clock measured at 241,690 kHz / 59.998 Hz off
  `PIPE_LINK_M/N`. BoredOS's graphics layer is `graphics.c` + `vga.c`,
  826 lines — it takes the framebuffer Limine hands it.
- **Its own bootloader**, and a documented answer to where the firmware
  boundary actually sits (`kernel/docs/concepts/what-is-a-bios.md`).
- **A USB stack**: xHCI + HID keyboard, debugged against real hardware
  rather than QEMU.
- **A host-side driver development loop.** `kernel/tests/host/` compiles the
  *same* `intel.c` that ships in the kernel as a Linux userspace program
  against the real GPU's PCI BAR. Seconds per iteration instead of
  write-USB → reboot → squint at a screen. This is the single most
  transferable idea in the repo and BoredOS has no equivalent.

## Where BoredOS is straightforwardly ahead

Say these out loud rather than working around them.

1. **It ships.** Ten tagged releases, ISOs, a nightly. zlOS has zero, and
   `main` does not even contain the windowed desktop — `main`'s `kernel.zl`
   has no `wm_*` calls and its `build.sh` never compiles `wm.c`. The desktop
   exists only on the eight unmerged tracks, which have all edited the same
   files independently and will conflict. See `docs/archive/superseded/INTEGRATION-PLAN.md`.
2. **Ports beat rewrites for reach.** DOOM, Lua and TCC running on your
   kernel is a stronger demo to a stranger than a correct DPLL, even though
   the DPLL is much harder.
3. **Nine contributors.** A codebase that other people can enter is a
   different kind of artifact from one that cannot be entered.
4. **The kernel is broader.** ELF loading, PTYs, IPC, audio, two real
   filesystems, three NIC drivers (e1000, RTL8139/8111, virtio-net).

## The philosophical difference worth noting

BoredOS ships an `AI-POLICY.md` that discourages AI-generated code outright:
*"We prefer that code contributions are primarily created by human
developers"*, with PRs eligible to be labelled `AI Slop`. Their stated
reasons are dysfunctional drivers, crashes and data loss.

zlOS is built the opposite way and has hit exactly those failure modes —
and caught them. The record is in this repo: the modeset audit found 86
verified defects in `intel.c` write paths that had never executed; an
adversarial review agent found 6 data-loss bugs sitting underneath 63
green assertions. The conclusion that survives both projects is the same
one: **the review has to be adversarial and it has to run against real
hardware.** BoredOS gets that from human reviewers and nine pairs of eyes.
zlOS has to buy it with fuzzing, mutation testing, host-side hardware
probes and fresh-context critics. Neither gets it for free.

## What to steal from them

- **A release cadence.** Even one tagged ISO forces the eight tracks to
  merge, which is the thing this project is currently avoiding.
- **A package format.** `bpm` is 1,184 lines. It is not the hard part, and
  it turns "my OS runs my apps" into "my OS runs other people's apps".
- **An ELF loader.** `zl-exec` already has `exec.c` and `arena.c` running
  zl programs under a memory budget. ELF64 is the format the rest of the
  world speaks.

## What not to steal

The porting strategy. The moment zlOS accepts mlibc it stops being the
thing it is. The zero in the vendored row is the entire thesis.

## Re-running the measurements

```bash
# zlOS: union across the eight checkouts, largest version per path
cd ~/Documents/repos
python3 - <<'EOF'
import os,subprocess
dirs=["zl-main","zl-linux","zl-apps","zl-browser","zl-exec","zl-feel","zl-system","zl-value16"]
best={}
for d in dirs:
    for f in subprocess.check_output(["git","-C",d,"ls-files"],text=True).split():
        if not f.endswith(('.zl','.c','.h','.S','.asm')): continue
        try: n=sum(1 for L in open(os.path.join(d,f),'rb') if L.strip())
        except: continue
        if n>best.get(f,0): best[f]=n
zl=sum(v for k,v in best.items() if k.endswith('.zl'))
print("zl",zl,"c",sum(best.values())-zl,"total",sum(best.values()))
EOF

# BoredOS: first-party kernel vs vendored lwIP
git clone --depth 1 --no-recurse-submodules https://github.com/BoredDevNL/BoredOS.git bos
cd bos
find . -path ./.git -prune -o -path ./net/third_party -prune -o -path ./fs/vendor -prune \
     -o -type f \( -name '*.c' -o -name '*.h' -o -name '*.S' \) -print \
  | xargs cat | grep -cve '^\s*$'          # 27,346 first-party
find ./net/third_party -type f \( -name '*.c' -o -name '*.h' \) \
  | xargs cat | grep -cve '^\s*$'          # 93,833 lwIP

# BoredOS project stats
gh api repos/BoredDevNL/BoredOS --jq '{stars:.stargazers_count,forks:.forks_count}'
gh api repos/BoredDevNL/BoredOS/contributors --jq '.[] | "\(.login) \(.contributions)"'
gh api repos/BoredDevNL/BoredOS/releases --jq '.[].tag_name'
```

## Caveats on these numbers

- The zlOS union is a *projection*. Where two tracks edited different
  regions of the same file, a real merge lands higher than the max-per-path
  figure. It also excludes `kernel/src/graphics/fonts/font_prop.inc` (~14k lines of generated
  bitmap data — data, not code).
- BoredOS's userland figure counts only the six first-party submodules
  (`nova`, `bsh`, `coreutils`, `netutils`, `bpm`, and not `bart`, which is
  Makefile-only). Their ported submodules are excluded from *both* the
  first-party and the in-tree vendored rows, so the "~11 MB" is a size of
  upstream source, not a like-for-like line count.
- BoredOS feature claims marked "per README" were read from their README
  and repo tree, not run. The zlOS hardware claims were measured on the
  Comet Lake laptop and are recorded in `kernel/HANDOFF.md`.

## Sources

- https://boredos.dev
- https://github.com/BoredDevNL/BoredOS
- `docs/CODE-MAP.md`, `docs/archive/superseded/INTEGRATION-PLAN.md`, `kernel/HANDOFF.md`
