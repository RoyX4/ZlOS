# `lt_armed` is armed by a keypress in the shipping tree — four docs say it cannot be

**2026-08-19 · worktree `zl-linux-fleet` · tree `3f00366` · verified by hand**

This is the safety-relevant finding of the fleet run. A driver-audit agent (lens
`intel-ungated`) raised it; every step below was then re-derived directly from the
tree, because the claim contradicts a documented hardware-safety boundary.

**The code is careful. The documentation is wrong, in the dangerous direction.**

---

## The chain, verified end to end

```
kernel/kernel.zl:1489        if cmd == 80 {          # 'P' - light the panel with OUR driver
        ↓
freestanding/runtime_kernel.c:1484
        if (streq(name, "panel_up")) return zl_num((double)intel_bringup_panel());
        ↓
kernel/intel.c:4288          u32 intel_bringup_panel(void)
        ↓
kernel/intel.c:4323              intel_link_train_arm(1);
kernel/intel.c:4324              int ok = intel_modeset_run(0);
kernel/intel.c:4325              intel_link_train_arm(0);
```

`intel_link_train_arm` is the setter for `lt_armed` (`intel.c:2186`). So pressing `P`
in the shipping desktop runs the full 30-step modeset on the real panel, including the
PPS panel-down / panel-up sequence.

There is a second identical arming pair at `intel.c:4336`/`4338`, in the shutdown path
(`intel_shutdown_panel`, bound to the zl builtin `panel_down` at
`runtime_kernel.c:1485`).

```
$ grep -rn "intel_link_train_arm" kernel/*.c freestanding/*.c | grep -v 'out.c\|_gen'
kernel/intel.c:2186:void intel_link_train_arm(int on) { lt_armed = on ? 1 : 0; }
kernel/intel.c:4323:    intel_link_train_arm(1);
kernel/intel.c:4325:    intel_link_train_arm(0);
kernel/intel.c:4336:    intel_link_train_arm(1);
kernel/intel.c:4338:    intel_link_train_arm(0);
```

Two of those four are in the shipping kernel and reachable from a zl builtin.

---

## What the documentation says

Four separate assertions to the contrary, in three files:

| file:line | text |
|---|---|
| `kernel/docs/display-roadmap.md:21` | *"Does zlOS itself call it? **no** — nothing arms `lt_armed` outside `hosttest/`"* |
| `kernel/docs/display-roadmap.md:37` | *"Nothing in the kernel arms `lt_armed`, so zlOS still cannot light its own panel"* |
| `kernel/HANDOFF.md:209` | *"**Nothing in the kernel ever arms `lt_armed`.** Only `hosttest/dpll_test.c` and …"* |
| `kernel/HANDOFF.md:313` | *"**Nothing in the kernel arms `lt_armed`.** This runs from the host harness only."* |
| `kernel/HANDOFF.md:365` | *"…on **one missing caller**: nothing in the kernel arms `lt_armed`."* |

And `CLAUDE.md:234` instructs every agent and every human to *"check whether anything
arms `lt_armed`, before believing a function runs"* — advice that is correct in method
and that the four statements above cause to be answered wrongly.

### The repo already half-knew

`kernel/HANDOFF.md:10` carries a correction banner:

> says nothing in the kernel arms `lt_armed` (`intel.c:4232` does, reachable …

So one line of `HANDOFF.md` records the truth while three later lines of the *same file*
still assert the falsehood, and `display-roadmap.md` was never updated at all. (The
banner's `intel.c:4232` is itself stale line-numbering; the arming sites are 4323 and
4336.)

This is the pattern the repo has been burned by before: a correction landed in one
place and the four copies of the original claim stayed.

---

## Why this is not an emergency, stated precisely

The audit agent checked the hazard discipline on this path and it holds. Re-read and
confirmed here:

- **T12 (500 ms power-cycle delay) is honoured.** `pp_wait_power_cycle`
  (`intel.c:2789-2808`) pays the remaining delay, floors at 500 ms (`intel.c:2785`),
  and — the part that matters — treats *unknown* power history as owing the **full**
  cycle (`intel.c:2796-2797`). That is the correct conservative default.
- **AUX is guarded against an unpowered panel** at `intel.c:1897-1898`.

So the shipping `P` command does not violate the two rules that can destroy the panel.
The code is written by someone who understood the hazard.

**The danger is entirely in the documentation.** The stated safety boundary is
"this can only happen from the host harness." That boundary does not exist. Anyone
reasoning from the docs — a human, or any of the agents in this fleet, all of which
were briefed with the hazard rules — will conclude the shipping kernel cannot touch
panel power, and will make a change on that assumption.

---

## What to do

1. **Correct all five doc sites**, and prefer deleting the claim to weakening it. The
   accurate sentence is: *"`lt_armed` is armed in two places in the shipping kernel,
   `intel.c:4323` and `intel.c:4336`, both reachable from zl builtins `panel_up` /
   `panel_down`, which `kernel.zl:1489` binds to the `P` command."*
2. **Do not add a gate to `intel_read_edid` / `gmbus_read_edid` as a reflex.** The audit
   agent flagged this specifically: `kernel.zl:1573` calls EDID reading in the shipping
   `k` command with `lt_armed` at 0, so gating it would silently turn EDID reporting
   into a permanent "no EDID" (`kernel.zl:1585`) — a wrong "not supported" of exactly
   the kind nobody retests.
3. **Re-check `CLAUDE.md`'s ungated-write table while you are there.** The same agent
   reports it lists four ungated writers where a full function-by-function scan finds at
   least eleven, naming `intel_backlight_set` (`intel.c:995`, writes `BLC_PWM_DUTY` at
   1006), `intel_flip` (`intel.c:1029`, writes `PLANE_SURF_1_A` at 1033), `intel_ggtt_map`
   (`intel.c:564`) and `intel_ggtt_map_range` among the omissions. **That count is the
   agent's and has not been re-derived by hand here** — treat it as a lead, not a fact,
   and settle it with a scan before editing the table.

---

## Method note

The fleet's hazard brief told all 25 driver agents that panel power is untouchable and
harness-only, because that is what the repo's own documentation says. One agent read the
code instead of the brief and contradicted it with a call chain. That is the behaviour
the brief asked for — *"where this brief states a claim as fact, VERIFY it"* — and it is
the reason the instruction is worth the tokens it costs.
