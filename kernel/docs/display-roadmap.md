# Finishing the display driver

The decision: **complete the display subsystem entirely before any GPU work.**
No ring buffers, no blitter, no execution engine until everything below is done.

This is the plan for that. It is ordered by dependency, not by appeal — several
of the interesting items are blocked behind two boring ones.

## Where the code actually is (measured 2026-08-17, not remembered)

| Question | Answer | How it was checked |
|---|---|---|
| eDP cold-start modeset | **works on hardware** | gradient displayed, 10 s, underrun clear |
| Does zlOS itself call it? | **no** | nothing arms `lt_armed` outside `hosttest/` |
| Pipe/port generality | **none** — 42 `*_A` registers | `grep -c '_1_A\|_A(' intel.c` |
| VBT / opregion parsing | **none** | 2 incidental mentions, no parser |
| HDMI | clock math only, **no port bring-up** | `intel_dpll_compute_hdmi` exists; nothing drives a TMDS port |
| Hotplug / interrupts | **zero lines** | `grep -cin 'hotplug\|IIR\|irq' intel.c` = 0 |

So the honest summary is: **one port, one pipe, one panel, one mode, polled.**
Everything below is about removing one of those words.

---

## Phase 0 — close what is already open

Small, and all of it is finishing rather than starting.

### 0.1 Wire the modeset into zlOS *(the actual finish line)*
Nothing in the kernel arms `lt_armed`, so zlOS still cannot light its own panel —
only the Linux harness can. Needs: a caller in `kernel.zl`, a framebuffer in
stolen memory, GGTT setup, and a decision about what to do when it fails (fall
back to the loader's framebuffer, do not leave a dark screen).

**Done when:** zlOS boots from its own bootloader on the ThinkPad, with no GRUB
and no UEFI framebuffer handover, and draws its shell on the panel.

### 0.2 The second modeset
Every run so far has been cold → on → teardown → hand back. Nobody has done
**off → on → off → on**. The plan (§5 stage 10) says this is where latent bugs
surface: the second time round T12 is a real debt, `pp_last_off_ms` matters, and
nothing is in its power-on-reset state. Cheap now that the harness works.

### 0.3 Link-training retry
One attempt, then fail. The plan wants a bounded retry — and a correct retry must
cycle the port first, because `DDI_BUF_CTL[3:1]` and `DP_TP_CTL[18]` cannot change
while the port is enabled (4.3 #20).

### 0.4 EDID over I2C-over-AUX
`intel_read_edid()` is GMBUS, and **GMBUS does not serve eDP on DDI A**. The panel's
EDID lives at I2C address 0x50 reached over AUX. Today the mode is read back from
registers firmware programmed, which works only because firmware ran first — a
genuine cold boot has nobody to read back from.

### 0.5 X-tiled scanout
The panel is natively X-tiled with stride 10240; we scan out linear. Works, uses
more memory bandwidth. `PLANE_STRIDE` unit changes 64 B → 512 B, and
`PLANE_CTL[12:10]` selects tiling. Small, and it removes a standing difference
from what firmware does.

---

## Phase 1 — VBT parsing *(unblocks most of what follows)*

The Video BIOS Table is the OEM's description of **this board**: which ports are
populated, panel power delays, the low-vswing flag, backlight controller and
frequency, DDI buffer settings, LVDS/eDP panel index.

Right now every one of those is either hardcoded for this ThinkPad or recovered by
reading back what firmware left. Both stop working on a cold boot or a different
machine.

Reached through the Intel opregion: PCI config `ASLS` (0xFC) → opregion base →
`RVDA`/`VBT` blob → BDB blocks. It is a parser, not a hardware sequence, so it is
testable without touching the display at all.

**Do this before external ports.** "Which ports exist" is a VBT question, and
guessing means driving a port that is not wired to anything.

---

## Phase 2 — generalise off pipe A and port A

42 registers are named `*_A`. Every one needs a pipe or port index. This is
mechanical, unglamorous, and blocks Phases 3–5 completely.

- Register macros take `(pipe)` / `(port)`
- `trans_base()` becomes a function of the transcoder, not a global guess
- Power wells per port (`DDI_B` = index 2, `DDI_C` = 3, `DDI_D` = 4 — already
  reachable since the well guard was widened to 15)
- Watermarks and the DDB split become per-pipe: **the DDB is shared**, so a second
  pipe means re-dividing 892 blocks between them, not adding more

**Hazard:** the DDB split is the one place where a plausible-looking pair of
numbers corrupts scanout on *both* pipes (4.3 #14).

---

## Phase 3 — external ports

> **Corrected by Phase 1, before a line of Phase 3 was written.**
>
> The VBT says this board has **no HDMI port at all**. It declares eight child
> device slots and populates three: `dvo_port 10` = DP-A (the eDP panel),
> `dvo_port 7` = DP-B, `dvo_port 8` = DP-C. `intel_vbt_port_present(1)` — HDMI-B
> — returns no.
>
> So "HDMI first because it is easier" is not available on this machine. The two
> external ports are DisplayPort behind Type-C/Thunderbolt, which §3.3 called
> "deliberately last, and possibly not worth it".
>
> This is precisely what putting VBT before external ports was for: the
> alternative was writing an HDMI port bring-up and then discovering there is
> nothing to plug into. Linux does create an `HDMI-A-1` connector, which is
> speculative and not backed by a VBT child device — believing *it* would have
> cost the same mistake.
>
> **Revised order for Phase 3:** external DP (§3.2) is the only reachable target,
> and it arrives coupled to the Type-C work rather than after it. HDMI stays
> written up because the code is generic and a future machine may have one, but
> it cannot be tested here.

### 3.1 HDMI *(no port on this board — untestable here)*
The DPLL side already exists and is verified — `intel_dpll_compute_hdmi()` finds
P/Q/K dividers and was checked against known clocks. What does not exist is the
port: TMDS instead of DP, **no link training at all**, different buf-trans table,
`TRANS_DDI_FUNC_CTL` mode field 000b not 010b, and an infoframe/AVI path.

Notably easier than eDP was — no training, no panel power sequencer, no T12.

### 3.2 DisplayPort (external)
Reuses the entire link-training path. Differences: `DP_TP_STATUS` **does exist**
on ports B/C/D (it does not on DDI A), so the idle polls that had to be skipped
become real; hotplug matters; and the sink can be anything, so the rate/lane
ladder that this panel did not need becomes load-bearing.

### 3.3 Type-C / Thunderbolt
The two DP ports on this laptop are behind Thunderbolt. Materially harder —
involves the TCSS, port ownership negotiation with the firmware, and DP-alt-mode.
**Deliberately last**, and possibly not worth it.

---

## Phase 4 — hotplug

Zero lines today. Needs a real interrupt path: `GEN8_DE_PORT_IIR` / `SDEIIR`,
short-pulse versus long-pulse decoding, debounce, then a re-probe of the port.
Everything today is polled and started by a human.

Depends on Phase 2 (per-port) and pairs with Phase 3 — hotplug of nothing is
pointless.

---

## Phase 5 — planes and composition

- **Sprite / overlay planes** — planes 2 and 3 per pipe. Same programming shape as
  plane 1, plus alpha blending and Z order.
- **Rotation** — `PLANE_CTL[1:0]`. Free in hardware, needs the surface laid out to
  suit.
- **Hardware scaling** — the two scalers per pipe that we currently switch OFF
  (`PS_CTRL_1_A`/`PS_CTRL_2_A`). Enables non-native resolutions without blur.
- **Y / Yf tiling** — better locality than X for rotated and 3D surfaces.
  **Trap:** tiling value 5 is Yf on gen9 and Tile4 on gen12+; cargo-culting a
  newer driver programs a layout the hardware reads differently (4.3 #15).

---

## Phase 6 — colour

Gamma LUT, the pipe CSC matrix, and degamma. Pure register programming, no
sequencing hazards, immediately visible. HDR needs the extended LUT modes and is
only meaningful on a panel that supports it — this one does not.

---

## Phase 7 — power and latency features

- **PSR** — currently we *disable* it as step 4 of every modeset. Enabling it
  properly means the panel refreshes itself and the pipe idles. It also fights
  every plane update and issues its own AUX traffic, which is why it was disabled
  rather than kept.
- **DRRS** — the M2/N2 register pair we deliberately write as zero. Switching
  between two refresh rates to save power.
- **RC6 / frequency scaling** — arguably GPU rather than display, but it is the
  same power-well machinery.

---

## Phase 8 — audio over HDMI/DP

The GPU carries audio to the monitor. Needs the display side (ELD, the audio
`TRANS_DDI_FUNC_CTL` bits) **and** an HDA controller driver, which zlOS does not
have. Largest single item here and the least related to everything else.

---

## Order, and why

```
0  close what is open        ← start here, it is finishing not starting
1  VBT                       ← unblocks 2 and 3; a parser, not a sequence
2  de-hardcode pipe/port     ← boring, mechanical, blocks 3/4/5
3  HDMI, then external DP    ← HDMI first: no training, no panel power
4  hotplug                   ← needs 2 and 3 to mean anything
5  planes, rotation, scaling, tiling
6  colour
7  PSR / DRRS
8  audio                     ← last, needs a whole new driver alongside
```

Phases 0 and 1 are days. Phase 2 is a grind with no discoveries in it. Phase 3
onward is where it starts looking like a real driver.

## The rule that got us here

Every number this driver programs was checked against what firmware left in the
same register, on the same panel, before any of it ran. That is why 34 untested
steps worked on the fourth attempt rather than the fortieth — and every one of
those first three failures was a bug in a *check*, not in the driver.

Keep doing that. Firmware has already solved every problem in this document at
least once, on this exact machine, and it leaves its answers lying around.

---

# STATUS (2026-08-18)

| Phase | | Verified how |
|---|---|---|
| 0.1 zlOS calls its own driver | **done** | builds 4 ways, 3 gates; **`P` never pressed on hardware** |
| 0.2 second modeset | **not done** | needs a hardware run |
| 0.3 link-training retry | **done** | gates; never triggered — no link has failed |
| 0.4 EDID over I2C-over-AUX | **done** | **128/128 bytes off the real panel**, checksum OK, 241690 kHz |
| 0.5 tiling support | **done** | stride units checked; painter still linear |
| 1 VBT parsing | **done** | **every value matches an independently known one** |
| 2 pipe-indexed registers | **done** | pipe A reads byte-identical after the refactor |
| 3 external DisplayPort | **written** | untestable — both ports behind Thunderbolt |
| 3.3 Type-C / Thunderbolt | **not done** | needs TCSS port-ownership negotiation |
| 4 hotplug | **done** | registers decoded and cross-checked against VBT |
| 5 planes, rotation, scalers | **done** | gates; no second plane has been enabled |
| 6 colour | **done** | palette base + format proved by firmware's identity ramp |
| 7 DRRS / PSR | **done** | gates; neither has been switched on |
| 8 audio | **deliberately not written** | see below |

## Why phase 8 is not written

The whole audio block, `0x65000`–`0x650D0`, reads **zero on this machine**. There
is an HDMI codec at `/proc/asound/card0/codec#2`, but no display audio is active
and the eDP panel has no audio path at all.

So there is nothing to check against. Every other register in this driver was
settled by comparing our value to what firmware programmed for the same
hardware — that is the entire method, and it is why 34 untested steps worked on
the fourth attempt. For audio that answer key does not exist here.

Phase 8 also needs an **HDA controller driver**, which zlOS does not have. The
display half alone produces no sound; it is one end of a link whose other end is
absent.

Writing speculative register pokes for a subsystem with no consumer and no way
to verify them is precisely the failure this project's method is built to avoid.
It stays unwritten until there is a machine that can prove it — which means
external DP working first, since that is the only port here that carries audio.

## What has NEVER executed on hardware

Worth keeping separate from "done", because the two are not the same:

- the `P` command — zlOS has never booted on the ThinkPad
- the link-training retry — no link has failed
- any second plane, rotation, or scaler
- DRRS or PSR enable
- any external port — nothing can be plugged in without Thunderbolt work
- a second modeset (off → on → off → on)

Everything above compiles four ways and passes three boot gates. That is a real
bar and it is not the same bar as "runs".
