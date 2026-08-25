# Remaining-work map — zlOS after the merge

The sequence. Evidence stays in
[`STATE-OF-THE-PROJECT-2026-08-19.md`](evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md).
Do not re-audit the
twenty-one planning documents from this file.

**Identity.** zlOS is the OS written in its own language that lights a real
laptop panel with no vendor blob, and is then a desktop you write zl on.
BoredOS ([boredos.dev](https://boredos.dev)) is the peer for “usable desktop +
a compiler that runs on the OS”. Essence is the peer for “one person, own
stack”. Serenity is the thing we refuse to chase.

Steal the product bar. Do not steal POSIX, mlibc, lwIP, a package manager, or
10 TTYs. The in-OS language stays **zl**.

```
H0 stop lying → H1 desktop you can use → H2 ThinkPad is ours → H3 write zl on zlOS
```

H2 is the public finish line. H3 is the language thesis. Later apps are
content, not architecture.

## Anti-goals

- Do not become BoredOS-in-zl.
- Do not restore `bg_snapshot` / 128 MiB drag buffers.
- Do not take `i915.ko`.
- Do not write WiFi/BT before H2.
- Do not migrate 82 `fb_*` sites as a “quick toolkit win”.
- Do not treat [`archive/superseded/INTEGRATION-PLAN.md`](archive/superseded/INTEGRATION-PLAN.md) as live.
- Do not count “exists” as “done”. Done = reachable **and** a gate covers the call.

## Horizons

**H0 — Stop lying.** The land gate runs the harnesses it builds. The three
static checkers run. `key()` does not halt the panel-handover path.
`check-memmap.sh` sees `DISK_SCRATCH` and `PAINT_BUF`. Pointer-drain is on
`main` (`7ddedb0`). Confirmed on a live ISO VM with `usb-mouse` (relative,
the `./try.sh` device): `probe-pointer-usb.py` saw the compositor pointer
move. gtk `./try.sh` *feel* is still **[HUMAN]** — this box has no display.

**H1 — Desktop you can actually use.** Settings opens from the start menu and
`settings_load` runs at boot when NVMe is up. Editor Ctrl+C/V is the clipboard
write path. Start menu is `WF_MODAL`. `verify-net.sh` fetches
`http://example.com/`. `ui_*` widgets are zl builtins; About uses one.

**H2 — The laptop is ours.** `edid_buf` is `HI_EDID` (top of the HID window,
outside `HI_BLUR`). `intel_bringup_panel` calls `intel_vbt_find()`; PPS delays
and I_boost take VBT values when present. Ordered modeset is already the
`P` path. **[HUMAN]** ThinkPad boot: Secure Boot off, USB image, screen-only
diagnostics. Success is the panel at 2560×1440 with the H1 desktop on it.
This environment cannot do that step.

**H3 — Write zl on zlOS.** Interpreter linked (`interp_kernel.c` + repo-root
lexer/parser/interp + `ksetjmp.S`). `fs_try` mounts zlfs at boot when a
volume is there, silently if not. `hello.zl` is seeded on a live volume;
`run hello.zl` executes it.
