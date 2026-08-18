# Typing into the compositor

How a harness gets a command into zlOS now that the desktop is the boot state,
and the false conclusion that is very easy to reach on the way there.

Written 2026-08-19, from measurement. Everything with a number in it below was
run on this branch; nothing here is reasoned from reading the source.

---

## The short version

**Serial reaches the compositor. It always has, since the day the desktop
became the boot state.** `input.c` drains COM1 into the one event queue
alongside PS/2 and USB — the block commented `SERIAL, the third source`. That
was done deliberately and for exactly this reason: every gate and probe in this
repo drives zlOS over that wire, and without it all of them would have gone
blind the moment `wm_frame()` took the screen (`.ultra/TENSIONS.md` T‑13).

What does **not** work is sending a bare character and expecting a command to
run. Under the compositor the shell is a window whose input is a **line**.
`term.c`'s `term_key()` buffers printable characters and does nothing else
until Enter, at which point it echoes the line and looks the first **word** up
in its table. So:

| you send | what happens |
|---|---|
| `w` | lands in the line buffer, is drawn at the prompt, **runs nothing** |
| `windows\r` | submitted, matched against `term.c`'s table → code 119 → runs |

`w` was a command in the old text shell, where `run_command` dispatched on
single characters. `term.c`'s table is the only place the two vocabularies
meet, and `{ "windows", 119 }, { "wm", 119 }` is the entry that used to be `w`.

## The trap: no output is not no delivery

Send a single `h` over serial, wait as long as you like, and the serial log
says nothing at all. It is extremely tempting — and wrong — to conclude the
byte never arrived.

It arrived. `term_key()` echoes **only in its Enter branch**, and deliberately:
the prefix goes to the scrollback and the typed characters go to the scrollback
*and* COM1, so that the serial transcript reads as one session rather than
`zl> zl> help`. Before Enter there is nothing to echo, so a correctly delivered
keystroke and a dropped one produce identical silence.

Proof, three boots of `probe-shot.py` on this branch:

| shots | pixels differing | where |
|---|---|---|
| no `-k` vs `-k w` | 1 225 (0.05 %) | y 950–1129 only — the shell's prompt line |
| no `-k` vs `-k $'windows\r'` | 113 015 (4.9 %) | y 177–1129 — the whole desktop |

Cropping the first pair settles it: the `w` **is on screen**, sitting in the
line buffer with the cursor behind it. The key was delivered; nothing submitted
it.

Independently, `./exercise.py --only help,fib,windows` types all three as
**words over serial** and scores 4/4, with `windows` changing 17 % of the
screen. Twenty-odd steps of that sweep have been driving the compositor over
COM1 this whole time.

## Which wire to use

Both work. They assert different things, and that is the only reason to choose.

- **Serial** (`probe-shot.py` default, `exercise.py` throughout) — faster, no
  per-key sleep, no character it cannot carry. Use it when the *shell* is what
  is under test.
- **The emulated keyboard** (`qtype()` → QMP `input-send-event`) — exercises the
  PS/2 and USB HID decoders. Use it when the *input stack* is under test, which
  is the only input a laptop has and the one path with no wire to fall back on.

`qtype()` and its `QCODE` table live in **`exercise.py`**, which is the one
module every probe already imports. `probe-term.py` and `probe-net.py` each
carry a local copy because a module name with a hyphen in it is not importable
and neither could import the other; both import `exercise.py` already, so when
either lands on this line its copy should be deleted in favour of the shared
one.

Note that `input-send-event` is required and `send-key` is not a substitute:
under `-display none` there is no active console handler and `send-key` is
silently dropped.

## How to prove the line was taken

Do not photograph, then hope. Wait for **term.c's echo of the submitted line**:
it is emitted inside the Enter branch and at no other time, it arrives the
instant Enter is handled, it is identical whichever wire the keys came in on,
and it still arrives when the command then blocks in a demo. That last property
is why it beats waiting for `zl> ` to come back.

`probe-shot.py` does this now, and exits non-zero saying so when the echo never
comes. It is the difference between a probe that fails and a probe that quietly
returns a picture of a desktop no command ran in.

## What the courtesy prompt is and is not

`ser_out("zl> ")` at the end of `kernel.zl`'s boot is written to the serial line
only, once, because every probe in this repo waits for that string before it
types. In the compositor the real prompt is pixels at the bottom of a window.
Later prompts come from `term.c` with the echo.

So the prompt on the serial log means "it booted and is listening". It is not
evidence of a text shell awaiting serial input — that loop is genuinely never
entered when there is a framebuffer (`kernel.zl`, `if wm_boot == 1 { while
wm_run() == 1 { wm_frame() } ... running = 0 }`). Both of those facts are true
at once, and reading the second as "so serial is dead" is the mistake this page
exists for.

## Stale claims still in the tree

`probe-term.py` (on `desktop/browser-next` and elsewhere) and `probe-net.py`
(added on `desktop/browser-next`, df77bcd) both state in their docstrings that
serial keystrokes cannot reach the compositor and that `wm_frame()` reads PS/2
and USB HID only. **`desktop/browser-next` has the `SERIAL, the third source`
block in its own `input.c`.** The claim is stale on the very branch that makes
it; the measurement it cites (`'N'` produced no output) is the buffered-line
silence described above, not a dropped key.

Their *choice* of QMP is still fine — it is the right wire for a probe whose
subject is the keyboard. Only the stated reason is wrong, and a wrong "this is
not supported" is the expensive kind: nobody re-tests what they have been told
is absent.
