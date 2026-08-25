> **AUDITED 2026-08-19 · REFERENCE, STILL VALID.** Checked item by item against `main` at `06ced13`,
> after the eleven-track merge. Its method — separate "no because it is enormous" from "no because a vendor holds a key" — survives the merge intact and is worth keeping as written. Two lines must be corrected before acting on it: its closing paragraph says "about six" walls where its own body says four, and its most actionable claim (blocked on one missing caller, nothing arms `lt_armed`) is false — `kernel.zl:1395` calls `panel_up()`, which arms it.
>
> **What is still open from this document is in
> [`docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md`](../../../docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md) — read that first, and do not
> work from the task list below.**


# What is actually impossible — the wall map

## The thesis this project is testing

> zlOS exists to show that one person can do what is assumed to need a team of
> thousands.

That is not a side note, it is the success criterion, and it changes how every
"is this feasible" question has to be graded. **"Too big for one person" is not
a valid objection in this project — it is the hypothesis under test.** So this
document re-grades the whole board against that bar instead of the generic one.

The useful finding: **almost nothing here is impossible because of size.**
The real walls are a different shape, there are fewer of them than expected,
and they have one thing in common.

---

## There are two kinds of "no", and only one is real here

**Kind 1 — "no" because it is enormous.** *Not a real no.* This has been refuted
too many times to keep repeating:

| Who | Alone | What |
|---|---|---|
| Terry Davis | ~10 years | TempleOS: **own language, own compiler, own kernel, own graphics, own 3D** |
| nakst (Essence OS) | since 2017, **nine years** | kernel, EssenceFS, software **vector** renderer with animation, POSIX subsystem running GCC |
| Andreas Kling | years before it grew a team | SerenityOS: kernel to browser, one repo, no third-party code |
| Fabrice Bellard | — | QEMU, FFmpeg, TCC, QuickJS |
| **Roy** | months | zl (5 backends), zlOS (15 drivers), **a cold-start Intel Gen9 modeset** |

Size costs *time*, not possibility. Every one of these is a grind that ended.

**Kind 2 — "no" because someone else holds a key you cannot get, or the target
moves faster than one person can chase.** *This is the real wall*, and it has
nothing to do with how good you are.

The distinction matters because the two need opposite responses. Kind 1 you
schedule. Kind 2 you route around — and routing around is not defeat, it is the
only correct move.

---

## The real NOs — someone else holds the key

These fail for the same reason every time: **a vendor cryptographically or
legally controls the thing you would have to write.**

> **Corrected the same day — see [`wireless-plan.md`](../plans/wireless-plan.md).** This
> table originally listed **Bluetooth** as a wall and put the iwlwifi blob at
> 2–3 MB. Both wrong. BT on this laptop is a **standard USB device speaking a
> published open standard** (`8087:0026`, class 224/1/1, and `xhci.c` already
> has all three transfer types it needs) — it is one of the *more* tractable
> subsystems here, not a wall. The iwlwifi blob is **1.3 MB**. And WiFi stops
> being gated the moment you pick the radio: the **AR9271's firmware is 50 KB
> and open source.** Bluetooth is struck from this table; WiFi is downgraded.

| | Why it is a wall | Route around it |
|---|---|---|
| ~~**Bluetooth**~~ | **STRUCK — not a wall.** HCI is a published Bluetooth SIG standard; the device is ordinary USB | Build it. ~6,800 lines to a working BT keyboard — `plans/wireless-plan.md` |
| **WiFi — AX201 only** | *Downgraded to Kind-1.* The 1.3 MB blob is redistributable, so it is not *gated* — it is ~40k lines of undocumented host↔firmware protocol | **AR9271 USB dongle: 50 KB open-source firmware**, ~8,000 lines. Or an ESP32 over UART, ~400 |
| **Widevine / DRM video** | Cryptographically gated by design, forever. Not hard — *forbidden* | Don't. Nothing to build |
| **Cellular modem** | Signed firmware + carrier certification | USB tether |
| **Shipping a Secure-Boot-signed image** | Needs Microsoft's signature to boot on other people's machines | **Soft no** — you can enrol your own key (MOK/setup mode) and boot your own hardware. Only *distribution* is gated |
| **GPU 3D acceleration** | i915 is **11.2 MB**, Mesa's Vulkan **24.3 MB**; newer parts need signed GuC/HuC | **Split the row — 2026-08-19.** For a COMPOSITOR: the ring is proven on this silicon (`kernel/docs/drivers/display/gpu-driver.md`, 16384/16384 px) and a fixed shader is 80 bytes lifted from Mesa, so it is weeks, not megabytes. For ARBITRARY programs: the compiler is the cost, and that is the 24 MB. Also still true: a software rasterizer needs no GPU at all — SerenityOS runs Quake III on one, `fb3d.c` is step one |

**The genuine list is four, not six.** And the lesson generalises: *"a vendor
holds a key" is a claim about a specific part, not about a capability.* Check
the part before calling the capability impossible.

**Note what is *not* on this list: the Intel display driver.** Modesetting needs
no firmware blob and no signed anything, which is exactly why it was reachable
and why it is 90% done. That was a good instinct.

---

## The real NOs — the target moves

Not gated, just infinite. No state exists in which you are finished.

| | Why | The bounded version to build instead |
|---|---|---|
| **Chrome-compatible browser** | The web is defined by whatever Chrome shipped last Tuesday, not by a spec | **A document browser** — ~13,200 lines, has a finish line. See `kernel/docs/concepts/beyond-the-kernel.md` §2b |
| **"Support all USB devices"** | Every device is a new quirk | **USB *classes*** — HID, mass storage, CDC-ECM. Bounded, and two of three already work |
| **Run unmodified Linux/Windows binaries** | The compatibility tail never ends (ReactOS, 27 years) | A WASM interpreter — every language, one bounded port |
| **Full ACPI/AML** | ACPICA is ~50k lines of interpreter for a bytecode vendors ship buggy | Parse the **static tables** only (MADT, MCFG) — already done for APIC |

---

## Things that look like a NO and are actually a YES

The encouraging list, and the one to plan from:

| | Size | Note |
|---|---|---|
| **A heap** | ~300 | The single highest-leverage missing piece. Gates the browser, other languages, and filesystems |
| **Clip rectangle** | ~2 functions | Already #1 on the catalogue's ranked list. Three customers waiting |
| **FAT32, read-write** | ~2,000 | A real filesystem. Read-only ext2 is similar |
| **Network to HTTPS** | ~10,000 | Every layer bounded and specified. TLS is 5–8k of it |
| **Document browser** | ~13,200 | Dillo/NetSurf/w3m proved the category |
| **Sound (AC'97 or HDA)** | ~1,500 | Whole new subsystem, but small and documented |
| **Software 3D rasterizer** | ~4,000 | SerenityOS runs *Half-Life* on one |
| **Hosting zl on zlOS** | ~1,200 | `src/runtime/interp.c` is only 1,900 lines. Self-hosting |
| **Drivers written in zl** | ~1,500 compiler + grind | See `kernel/docs/concepts/beyond-the-kernel.md` §3 |

**Every row is a Kind-1 problem.** Schedule them; none is a wall.

---

## What actually kills solo OS projects — and it is not difficulty

This is the part worth internalising, because the failure mode is never the one
people brace for.

**1. The breadth tax.** Every feature must keep working while you add the next.
15 drivers is 15 things that can silently regress. This is the real cost of
scope, and it compounds — it is why the second half of an OS is slower than the
first even though the parts are easier.

*You are already defended here*, and unusually well: `verify.sh`, `verify-raw.sh`
and `verify-iso.sh` are golden-transcript gates, and `HANDOFF.md` records what is
*verified* rather than what is intended. That discipline is worth more than any
feature on this page. Do not let it slip when the work gets exciting.

**2. Hardware diversity.** "Works on my ThinkPad" is not "works". Every machine
has a different panel, a different NIC, a different quirk. **The correct answer
is to not chase it** — say "zlOS targets this laptop", make that a stated scope
rather than an unadmitted gap, and it stops being a debt.

**3. Stopping.** The actual killer. Essence is nine years and still going.
TempleOS was about ten. The projects that failed, failed by ending — not by
hitting a wall.

---

## What would prove the thesis hardest

Blunt answer: **the most impressive thing in this project is the one you are
already 90% through, and it is not the browser.**

A **cold-start Intel Gen9 modeset written from scratch with no Linux code** is a
thing very few individuals have ever done. Your own research already contains the
comparison that makes the point: **Haiku is 25 years old, has a team, and its
Intel driver is modesetting-only** — i.e. exactly where `intel.c` is aiming.
And it is proven: `sudo ./modeset-run.sh --modeset` lit the ThinkPad's panel
from cold with 34/34 steps green and **zero FIFO underruns over ten seconds**.

The single most thesis-proving demo available:

> **zlOS boots the ThinkPad from its own 512-byte bootloader, brings up its own
> panel at 2560×1440 through its own modeset, and draws its own desktop — with
> no GRUB, no Linux, no firmware blob, and no borrowed code anywhere in the
> chain.**

Every layer of that is yours. Nothing in it is gated by a vendor. And it is
blocked on **one missing caller** — nothing in the kernel arms `lt_armed`
(`HANDOFF.md`), so the driver that is proven to work has nobody to call it.

That is the highest ratio of "impressive" to "remaining work" on the whole board,
by a wide margin. The browser is real and worth building — it is just worth
building *after* the thing that is one function away from done.

---

## The one-paragraph version

Nothing on this board is impossible because it is big; that objection has been
refuted by Terry Davis, nakst, Kling and Bellard, and partly by this repo
already. The genuine walls are all the same shape — **a vendor holds a signing
key, or the target is a competitor's shipping product** — and there are only
about six of them, each with a legitimate route around it. Everything else is
schedulable. The thing that kills projects like this is breadth without
verification, and the discipline that stops that is already in place here.
