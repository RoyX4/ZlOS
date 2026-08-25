> **AUDITED 2026-08-19 · PARTLY OPEN.** Checked item by item against `main` at `06ced13`,
> after the eleven-track merge. The substrate genuinely exists: zlfs survives three power cycles under `verify-disk.sh`, `rtc.c` decodes under `verify-clock.sh`, snapping gave `wm_resize` its first caller, and all seven data-loss defects have named regressions. Read it now for its hazard list and its "what done means" standard — four of its user-facing outcomes were silently reverted by the merge.
>
> **What is still open from this document is in
> [`docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md`](../../../../docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md) — read that first, and do not
> work from the task list below.**

# The system track — storage, time, clipboard, and the things an OS just has

The fourth brief. The other three are the **look**, **apps and speed**, and
**feel and control**. This one is the substrate: the things that make a
collection of programs behave like one machine.

---

## How to start it

Open a session in `~/Documents/repos/zl-system` and paste only this:

```
Read kernel/docs/archive/prompts/SYSTEM-PROMPT.md in full and execute it exactly as written.
You are in a git WORKTREE on branch desktop/system-track - confirm with
`git branch --show-current` before your first commit. Other sessions are
running in sibling worktrees; §2's ownership split keeps you apart. Work the
queue in order, gate every item, commit on green. Agents are authorised - §7
says how without OOM-killing the box. Do not stop until I tell you to.
```

---

## 0. Five findings. All established by reading the source.

### 0.1 Files have no names, and nothing survives a reboot

`fs_list()` prints ten numbered slots:

```
RAM files:
  file 0:  empty
  file 1:  empty
```

That is the whole filesystem. **No names, no directories, no persistence.** It
is raw memory with an index, which is honest about what it is — and it is the
loudest "this is not an OS yet" signal in the product, louder than any pixel.

### 0.2 The NVMe driver works and zl cannot reach it

`nvme.c` has `nvme_read_block`, `nvme_write_block` and `nvme_ready`. All three
work. **Zero of them are exposed as builtins**, and `kernel.zl` mentions nvme
exactly once — in a diagnostic.

So there is a working disk that nothing can use. That is not a missing driver,
it is a missing *five lines of binding*, and it is the single cheapest large
win in this document.

### 0.3 The clock is not a clock

The header draws a time from `ticks()` — the 100 Hz PIT counter, i.e. **uptime
since boot**. There is **no RTC driver**: nothing reads CMOS at port 0x70.

So the desktop displays a number that looks like a time and is not one. An OS
knows what time it is.

### 0.4 The header says "net up" and there is no network driver

`kernel.zl:198` draws `net`, `kernel.zl:200` draws `up`. There is no `e1000`,
no `virtio-net`, no `rtl8139` — no network code anywhere in the tree.

**That is a decorative lie on screen.** This project's standing rule is that
`HANDOFF.md` records what is *verified*, not what is intended; the same rule
should apply to what the desktop tells the user. Either build the driver or
change the label. **Do not leave it claiming a link that does not exist.**

### 0.5 No clipboard, no notifications, no window snapping

Grepped: zero hits for any of the three. Clipboard in particular is what makes
several apps feel like *one system* rather than several programs that happen to
share a screen.

---

## 1. HARD RULES

### 1.1 Stay in your worktree

One `.git` shared between sessions cost a corrupted object store and three
commits on the wrong branch on 2026-08-18 (T-10, T-12). Never `git checkout`
another branch here; never touch a sibling worktree. `git status` before every
commit, stage **by name**, never `git add -A`.

### 1.2 Resources — and there are now four sessions

4 cores, 15 GB, OOM-killed twice. **ONE QEMU across the whole box, ever.**
Check `pgrep -f qemu-system` before booting; if another session is mid-gate,
wait. `cut -d' ' -f1-3 /proc/loadavg` first; above ~4, wait. Gates in the
background, collected.

Prefer the host harnesses — `hosttest/*` needs no QEMU and runs in seconds.
Most of this track can be tested there.

### 1.3 Gates

`verify.sh` is 32-bit BIOS only; also `verify-efi.sh` and `verify-raw.sh`.
Never a fixed wall-clock wait — poll for the expected output. Never build on a
red gate; log it to `.ultra/TENSIONS.md` and take the next independent item.

### 1.4 Four build scripts, four source lists

`build.sh`, `build64.sh`, `buildefi.sh`, `mkdisk.sh`. `verify.sh` sees only the
first, and adding a `.c` broke two builds twice in one day. **This track adds
several new files** — add each to all four, and verify by building all four.

### 1.5 Do not touch `intel.c`. Its write paths can damage hardware.

---

## 2. OWNERSHIP

| yours | others |
|---|---|
| `kernel/fs.c` *(new)* | `ui.c`, `gen_*` → look track |
| `kernel/rtc.c` *(new)* | `term.c`, `smp.c` → platform track |
| `kernel/clip.c` *(new)* | `cursor.c`, `settings.c` → feel track |
| `kernel/nvme.c` | |
| `kernel/notify.c` *(new)* | |

**Shared:**
- **`kernel.zl`** — you own the **storage/time/clipboard commands and their
  app_draw branches**. The platform track owns the boot sequence and the demos;
  the look track owns the drawing. Keep edits contiguous and commit often.
- **`wm.c`** — you may **add** a notification surface. Do not change routing,
  damage or z-order; the platform track is in there.
- **`runtime_kernel.c`** — builtins only, appended.

---

## 3. THE WORK QUEUE

### Item 1 — expose NVMe to zl (finding 0.2)

The cheapest large win here. `nvme_read_block`, `nvme_write_block`,
`nvme_ready` become builtins, plus a way to move bytes between a disk block and
a raw memory address.

**Gate:** write a known pattern to a block, read it back into a different
address, compare. Then reboot in QEMU and read it again — **the same bytes, on
a machine that has been power-cycled.** That is the first time anything in this
project has survived a reboot, and it deserves to be proven rather than assumed.

### Item 2 — a filesystem with names (finding 0.1)

Not ext4. The smallest thing that is honestly a filesystem:

- a **superblock** at a fixed LBA: magic, version, block count, checksum
- a flat **directory** of fixed-size entries — name[24], start LBA, length,
  flags. One block holds ~40 of them, which is plenty
- files as **contiguous runs** of blocks. No fragmentation, no free-list
  walking, no extents. When a file grows past its run, copy it. That is
  "wrong" for a real disk and exactly right for this one.

**No heap.** Fixed arrays, a fixed maximum file count, and **"no free slot" is
a refusal that prints** — never a silent drop. That rule has been broken twice
in this project already.

**Refuse to mount a superblock with a bad magic or checksum, and say so.** A
filesystem that silently treats garbage as data is worse than one that will not
mount.

`fs_list()` becomes a real listing. `ls` in the terminal shows names.

**Gate:** create, write, list, read back, delete, reboot, list again. Then
**deliberately corrupt the superblock** and confirm it refuses and prints. Write
this as a host harness first (`hosttest/fstest.c`) against a fake block device —
it will find more bugs in an afternoon than QEMU will in a week.

### Item 3 — an RTC, so the clock is real (finding 0.3)

CMOS at ports 0x70/0x71. ~80 lines including BCD conversion and the
update-in-progress flag.

**The trap everyone hits:** reading while bit 7 of status register A is set
gives a torn value. Poll until it clears, then read — and read **twice**,
accepting only when two consecutive reads agree. Otherwise the clock
occasionally shows a second from the middle of a carry.

Then the header shows the real time and date.

**Gate:** boot, read the time, compare against the host's clock in the same
minute. *(Note: `CLAUDE.md` warns this box's RTC runs slow and NTP corrects it
mid-session — so compare against the guest's own CMOS, which is what QEMU
seeds, not against wall-clock expectations.)*

### Item 4 — the clipboard (finding 0.5)

`clip.c`: one fixed buffer, a length, and a type tag. `clip_put(ptr, len)`,
`clip_get()`, `clip_len()`.

Then wire **Ctrl+C / Ctrl+V** into `wm.c`'s key routing and the terminal.
`input.c` already produces control codes — Ctrl+C is 3, Ctrl+V is 22 — so the
plumbing exists.

**This is the item that makes it feel like one system.** Copy a line in the
terminal, paste it into the editor, and it stops being several programs sharing
a screen.

**Gate:** a `wmtest` assertion — put, get, and a paste that lands in a different
app than the copy.

### Item 5 — notifications

`wm.c` already has `WF_MODAL` and a 4-frame open animation. A toast is a
borderless window that opens, sits, and closes itself on a tick count.

Keep it to: a queue of 4, one visible at a time, auto-dismiss, click to dismiss
early. **Do not build a notification centre.**

**Gate:** `wmtest` — it appears, it expires on its own, it does not steal focus.
That last one matters: a toast that takes focus eats the next keystroke.

### Item 6 — window snapping

`wm_resize` exists and **nothing calls it**. Drag a window to an edge, it takes
that half; to a corner, that quarter. Super+arrow does the same from the
keyboard — `MOD_SUPER` is tracked and used for nothing.

**Gate:** `wmtest` — geometry after each snap is exactly the expected rectangle,
and un-snapping restores the pre-snap size. Storing that "restore" rectangle is
the part people forget.

### Item 7 — the "net up" label (finding 0.4)

Decide honestly, and do one of:

- **change the label** to say what is true, or
- **write a driver.** `virtio-net` is the realistic one — QEMU has it, the
  project already drives `virtio-gpu`, so the queue/descriptor pattern is
  already in the tree and proven

If you pick the driver, scope it to **link up + send + receive one frame**. Do
not build a TCP stack.

**Whichever you pick, the screen must stop claiming something untrue.** That is
the same standard `HANDOFF.md` is held to.

---

## 4. What "done" means

Never "it looks right":

- a gate command **and its output**
- for storage, a **reboot** — persistence claimed without a power cycle is not
  persistence
- for anything invisible, an **assertion in a harness**
- a number you **measured**

---

## 5. The order matters

1 → 2 → 3 → 4 are roughly independent after 1, and **1 unblocks 2**.

**5, 6, 7 can be done any time** and are good when a QEMU gate is occupied by
another session, because 5 and 6 are pure `wmtest` work with no boot needed.

---

## 6. Hazards, each already paid for

- **No heap, no lists, no runtime strings** in the zl kernel subset. Filenames
  are compared in **C** — that is why `term.c` holds the command table.
- **The zl parser takes no multi-line call arguments.**
- **Multi-MB buffers never in BSS** — fixed high RAM, and check the map at the
  top of `fb.c`. A few KB in BSS is fine; that rule is about megabytes.
- **The DMA arena and its neighbours**: bg 128 MiB, sp 160, sched 176, back 192,
  nvme 208, xhci 224, virtio-gpu 240. **Any new buffer must be checked against
  this map** — a fixed address colliding with a neighbour is this project's
  recurring bug, five times so far.
- **`-mgeneral-regs-only` on `idt.c`** — no SSE in an ISR, and no calling out to
  code that uses it.
- **Local timestamps are unreliable** — the RTC runs slow and NTP corrects
  mid-session. Relevant to Item 3 specifically.

---

## 7. Agents

Authorised. Best used here for:

- **an adversarial reviewer on the filesystem write path.** Item 2 can destroy
  data, and the author is the worst person to review it. Have a fresh reader
  try to construct a sequence that loses a file.
- **fanning out readers** to inventory what a real OS has that this does not —
  cheap, parallel, and this document was written from exactly that exercise.

**Not** for parallel QEMU boots. Four sessions now share one box.

**If you delegate, you own collection.** Never end a turn waiting on an agent.

---

Look: [`desktop-v10-plan.md`](../../evidence/desktop-v10-plan.md) · Platform:
[`PLATFORM-PROMPT.md`](PLATFORM-PROMPT.md) · Feel:
[`FEEL-PROMPT.md`](FEEL-PROMPT.md) · Contract: [`../ui.h`](../../../src/graphics/ui/ui.h) · History:
[`desktop-overnight-run.md`](../../evidence/desktop-overnight-run.md)
