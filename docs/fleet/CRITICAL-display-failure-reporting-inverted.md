# The display driver's two failure modes are inverted

**2026-08-19 · worktree `zl-linux-fleet` · tree `3f00366` · both verified by hand**

From the fleet's `silent-failure` bug-class sweep. Two defects in `kernel/intel.c`, found
together, and each is the mirror of the other:

- **A real failure reports success** — link training returns 1 on a panel the sink was
  never told to leave training pattern on. Black screen, reported green.
- **A cosmetic failure reports total failure** — a backlight or telltale step marks the
  entire modeset failed, so `panel_up` returns 0 on a panel that is lit and working.

On a machine whose only diagnostic is the screen
(`kernel/docs/thinkpad-first-boot.md`), both directions are expensive.

---

## 1. `intel.c:2555` — link training reports success on a sink still in training

```c
/* kernel/intel.c:2548-2560 */
if (!intel_dpcd_write(port, DPCD_LANE_COUNT_SET, &lc, 1)) return 0;   /* checked */

if (!train_clock_recovery(port, lanes)) return 0;                     /* checked */
if (!train_channel_eq(port, lanes, tps3)) return 0;                   /* checked */

/* done: stop the pattern at both ends and let real pixels flow */
u8 none = DP_TRAIN_PAT_NONE;
intel_dpcd_write(port, DPCD_TRAINING_PATTERN, &none, 1);              /* NOT checked */
tp_ctl_pattern(port, DP_TP_CTL_LINK_TRAIN_IDLE, enhanced);
cpu_delay_us(500);
tp_ctl_pattern(port, DP_TP_CTL_LINK_TRAIN_NORM, enhanced);
return 1;                                                             /* unconditional */
```

**Every other `intel_dpcd_write` in this function is checked.** The one that is not is
the final one — the write that tells the *sink* to leave the training pattern.

If that AUX transaction fails or is NAK'd, the **source** proceeds to
`DP_TP_CTL_LINK_TRAIN_NORM` and starts sending real pixels, while the **sink** is still
in training pattern and displays nothing. `intel_link_train()` returns 1, and every
layer above it reports a successful modeset over a black panel.

This is precisely the class `kernel/net.c:58-64` was written about, quoted here because
the reasoning is the repo's own:

> **EVERY DROP PATH GETS A COUNTER.** This is not bookkeeping: `handle_ip` used to drop
> a short or malformed header with a bare `return`, and one such frame per bring-up cost
> **six wrong hypotheses** to find […] A drop with no counter is a bug that cannot be
> located from the outside.

**Fix:** `if (!intel_dpcd_write(port, DPCD_TRAINING_PATTERN, &none, 1)) return 0;` — the
same shape as the three checks above it. If failing the whole train is judged too harsh,
record it into the `ms_log` so it is *visible*; silently returning 1 is the one option
that should be off the table.

## 2. `intel.c:3882` — `MS_STEP_SOFT` is soft about continuing and not about the verdict

The macros do differ, and the comment at `intel.c:3891` describes the intent correctly:

```c
/* MS_STEP aborts the sequence on failure. MS_STEP_SOFT records and carries on. */
#define MS_STEP(n, name, call) \
    do { if (ms_dry) ms_do((n), name, 1); \
         else if (!ms_do((n), name, (call))) return 0; } while (0)      /* aborts */

#define MS_STEP_SOFT(n, name, call) \
    do { if (ms_dry) ms_do((n), name, 1); else ms_do((n), name, (call)); } while (0)
```

`MS_STEP_SOFT` does carry on executing. But **both macros go through `ms_do`**, and
`ms_do` does not know which one called it:

```c
/* kernel/intel.c:3882 */
if (!result && !ms_failed_at) ms_failed_at = plan_step;
```

and the sequence's verdict is:

```c
/* kernel/intel.c:4089 */
return ms_failed_at ? 0 : 1;
```

So a soft step's failure poisons `ms_failed_at`, and `intel_modeset_run` returns **0**.
The soft/hard distinction is expressed in the macros and erased one function later.

### Which steps this covers

```
intel.c:3991  MS_STEP_SOFT(8,  "DPLL0 locked at wanted rate", …)
intel.c:4079  MS_STEP_SOFT(56, "plane SKIPPED (no framebuffer)", 1)   ← literal 1, always passes
intel.c:4081  MS_STEP_SOFT(57, "underrun telltale cleared", intel_pipe_underrun_clear())
intel.c:4084  MS_STEP_SOFT(58, "backlight PWM",             …)
intel.c:4087  MS_STEP_SOFT(58, "backlight enabled",         intel_panel_backlight_enable(1))
```

**A backlight that does not come up, or an underrun telltale that does not clear, makes
the entire modeset report failure** — on a panel that is otherwise lit, trained and
scanning out.

That propagates: `intel_bringup_panel` (`intel.c:4324`) does
`int ok = intel_modeset_run(0); … if (!ok) return 0;`, so the zl builtin `panel_up` — the
shipping `P` command — returns 0 for a working display.

**Fix:** give `ms_do` a `soft` parameter, or set `ms_failed_at` from the `MS_STEP` macro
rather than inside `ms_do`. Keep the soft result in `ms_log` so
`intel_modeset_step_result()` still reports it — the log is the right place for
"something non-fatal went wrong", and it is already plumbed out through
`intel_modeset_steps()` / `intel_modeset_step_name()`.

---

## The structural finding underneath both

The sweep's own framing is worth keeping:

> The tree has **two populations**. `net.c`, `arena.c`, `wm.c`, `html.c`, `layout.c`,
> `notify.c`, `virtio_net.c` and `tcp.c` follow the doctrine written at
> `kernel/net.c:58-64` and are clean. **The display driver, the PCI power path, the xHCI
> firmware handoff and the whole input event path do not.**

The doctrine exists, is written down, is well-argued, and is followed by half the
codebase. The other half predates it or was never brought in line. That is a much more
tractable problem than "add error handling everywhere" — it is a list of four subsystems
and a rule they already have.

Other instances the same sweep reported, **not hand-verified**:

| file:line | claim |
|---|---|
| `intel.c:4343` | `panel_step()` reports 0 or a stale step number for eight of nine bring-up failure paths |
| `pci.c:315` | `pci_enable()` discards `pci_power_on()`'s result — reintroducing the exact failure `pci.c` was rewritten to prevent |
| `xhci.c:262` | the xHCI firmware-ownership handshake measures whether SMM released the controller and throws the answer away |
| `input.c:118` | four input-path ring drops with no counter, one of which loses data permanently |

`pci.c:315` is the sharpest of the four: the capability walk and `pci_power_on` were
added in `06b1a0f` specifically because *"MMIO on a device in D3 returns `0xFFFFFFFF`,
which every driver will read as 'every status bit set' or 'device absent'"* — and if its
result is discarded, that failure comes straight back.
