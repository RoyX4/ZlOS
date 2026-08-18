# The exec track — running code the kernel was not built with

The sixth brief, and the most architecturally consequential. The other five
make zlOS **better**. This one decides what it **is**.

---

## How to start it

Open a session in `~/Documents/repos/zl-exec` and paste only this:

```
Read kernel/docs/EXEC-PROMPT.md in full and execute it exactly as written.
You are in a git WORKTREE on branch desktop/exec-track - confirm with
`git branch --show-current` before your first commit. Other sessions run in
sibling worktrees; §4's ownership split keeps you apart. §2 is a hard
prerequisite. Read §1 before choosing anything - the level choice is the whole
decision and it is not reversible cheaply. Gate every item, commit on green.
Agents are authorised; §9 says how. Do not stop until I tell you to.
```

---

## 0. What is true today

Established by grep, not memory:

- **Everything runs in ring 0.** No user mode, no syscalls, no memory
  protection. One incidental hit for "syscall" in the whole tree, in a comment.
- **A program becomes code by being compiled INTO the kernel.** `kernel.zl` →
  `compile` → `out.c` → linked into `kernel.elf`. To run a program you rebuild
  the OS and reboot.
- **There is a scheduler and it is deliberately not wired in.** `sched.c`, 292
  lines, real cooperative context switching — a task is a stack plus a saved
  stack pointer, and the trick is that `ret` returns to wherever the *new*
  stack says to go. `kernel.zl` touches it twice, both in a diagnostic.

`DECISIONS.md` #5 says why it is not connected:

> preemptive tasks sharing one framebuffer with no memory protection and no
> locks is a data race with extra steps

**That call was correct and it is still correct.** Do not "just turn the
scheduler on". Everything below is about earning the right to.

---

## 1. THE LEVEL CHOICE — read all three before writing anything

These are not stages of one road. They are three different operating systems,
and picking one closes doors on the others.

### Level 1 — zlOS runs **zl**

Load a `.zl` source file and interpret it.

**The case for it:** the OS is written in zl. An OS that runs its own language
is a coherent identity, and it is roughly what TempleOS did with HolyC. It
needs no rings, no MMU work, no syscall boundary, and it cannot be "half done"
— either a script runs or it does not.

**The honest cost.** `interp.c` is **1,900 lines** and `runtime.c` is **1,355**,
and the interpreter depends on the *full* runtime: a heap, list values, string
values. The kernel deliberately uses `runtime_kernel.c` instead, which rejects
all three (`zl_list_n` is a hard fault). So this is **not** "link the
interpreter in":

| | |
|---|---|
| port `interp.c` to the kernel | mostly mechanical, but every allocation site must change |
| an **arena allocator** | ~300 lines. There is no heap, and this is where one arrives |
| boxed values with lists and strings | the thing `runtime_kernel.c` exists to refuse |
| a per-script memory budget and a hard kill | ~200 lines |

**~2,400 lines, and it introduces a heap to a kernel whose boot log currently
boasts "no heap".** That is a real identity change and it should be a decision,
not a side effect.

### Level 2 — flat binaries, still ring 0

Load a position-independent blob at a fixed address and call it. **~200 lines.**

**Do not do this as an end state.** With no protection, a bad program does not
crash — it corrupts the compositor's back buffer, or the DMA arena, and the
symptom appears somewhere else entirely. This project has already spent five
separate bugs on fixed addresses colliding; handing that gun to arbitrary code
is worse than not running code at all.

It is legitimate **only** as a stepping stone to Level 3, and only if you go
there immediately.

### Level 3 — real processes, ring 3

TSS, a syscall gate, per-process page tables, an ELF loader. **~3,000 lines,
and it reshapes everything above it.**

The part people underestimate: **every driver becomes a syscall.** User code
cannot touch the framebuffer, so `fb_*`, `wm_*`, `ui_*` and the whole app
contract in `ui.h` need a boundary. `wm.c` currently calls `app_draw` as a
direct C call; across a ring transition that is a different design.

**This is where "an operating system" begins in the textbook sense**, and it is
a project comparable in size to everything built in this repo so far.

### The recommendation, and it is a recommendation not an instruction

**Level 1**, for three reasons: it is the only one that does not require
rewriting the app contract, it is the most in character for a project whose
whole thesis is its own language, and its failure mode is "the script does not
run" rather than "the machine is subtly corrupted".

**But Roy chooses.** Put the choice at the top of `.ultra/TENSIONS.md` as an
`ASSUMED:` line if you have to proceed without an answer, and say which level
you assumed.

---

## 2. HARD PREREQUISITE

**You cannot load a program you cannot name.** There is no filesystem — ten
numbered slots in raw memory, gone on reboot (`SYSTEM-PROMPT.md` finding 0.1).

The system track's item 2 is building one. **Check before you start:**

```bash
git log --all --oneline | grep -i "filesystem\|fs.c"
ls kernel/fs.c 2>/dev/null
```

If it is not there yet, do **Item 0** below, which needs no filesystem, and
come back.

---

## 3. HARD RULES

- **Stay in your worktree.** One `.git` shared between sessions cost a
  corrupted object store and three commits on the wrong branch (T-10, T-12).
  `git status` before every commit; stage by name; never `git add -A`.
- **4 cores, 15 GB, OOM-killed twice. ONE QEMU across the whole box.** Six
  sessions may exist now — `pgrep -f qemu-system` before booting.
- **Gates:** `verify.sh` is 32-bit BIOS only; also `verify-efi.sh` and
  `verify-raw.sh`. Never a fixed wall-clock wait. Never build on a red gate.
- **Four build scripts, four source lists.** This track adds several files.
- **Do not touch `intel.c`.** Its write paths can damage hardware.

---

## 4. OWNERSHIP

| yours | others |
|---|---|
| `kernel/exec.c`, `arena.c` *(new)* | `ui.c`, `gen_*` → look |
| `kernel/sched.c` | `term.c`, `smp.c` → platform |
| `kernel/interp_kernel.c` *(new)* | `cursor.c`, `settings.c` → feel |
| `kernel/proc.c`, `syscall.c` *(new, L3)* | `fs.c`, `rtc.c` → system |
| | `net*.c`, `html.c` → browser |

**Shared:** `kernel.zl` — you own **only** the `run` command and its app branch.
`runtime_kernel.c` — builtins, appended. **`fb.c` you do not touch at all**;
three tracks are already in it.

---

## 5. THE WORK QUEUE

### Item 0 — a memory budget, before anything can run

Whatever level you pick, foreign code needs somewhere to live that is **not**
the kernel's memory. Do this first; it needs no filesystem and no decision.

- carve a **program arena** from the high-RAM map, checked against its
  neighbours: bg 128 MiB, sp 160, sched 176, back 192, nvme 208, xhci 224,
  virtio-gpu 240. **Compute the base; do not guess it.** A fixed address
  colliding with a neighbour is this project's recurring bug, five times over.
- a bump allocator with a **reset**, not a free list. Programs are short-lived;
  reset between runs and the whole class of use-after-free disappears.
- a **hard ceiling**, and exceeding it is a refusal that **prints**. Silent
  fallbacks have been found twice in this project already.

**Gate:** `hosttest/arenatest.c` — allocate to the ceiling, confirm the refusal
prints, reset, confirm the space comes back. No QEMU needed.

### Item 1 — `run` with nothing to run

The command, the app window, the error path. **Before** any execution exists.

`run hello.zl` with no filesystem should say **"no filesystem"**, not crash and
not silently do nothing. Get the failure modes right while they are the only
modes.

**Gate:** typing `run` and `run nothing.zl` both produce a clear line.

### Item 2 — the level you chose

Follow §1. Whichever it is, three properties are not negotiable:

1. **A runaway program must be killable.** An infinite loop in a script cannot
   wedge the machine. For Level 1 that is an instruction budget checked in the
   interpreter loop; for Level 3 it is the timer interrupt. **Decide this
   before you write the loop, not after.**
2. **A crashing program must not take the kernel with it.** Level 1: catch the
   fault at the interpreter boundary. Level 3: a fault handler that kills the
   process.
3. **Memory it touches must be its own.** Level 1: only through the arena.
   Level 3: page tables.

**Gate:** three scripts — one that works, one that loops forever, one that
does something illegal. All three must end with the desktop still responsive
and a line explaining what happened. **The last two matter more than the
first.**

### Item 3 — a real `ps`, and the scheduler earns its keep

Only after Item 2. `sched.c` has worked for weeks with nothing using it.

Wire it so a running program is a **task**, and the compositor's frame loop is
another. `DECISIONS.md` #5's objection was the shared framebuffer — that
objection weakens once programs draw through a boundary rather than into the
buffer directly, and it must be **answered explicitly in writing** before you
turn it on, not assumed away.

**Gate:** run a long script, and the desktop still drags windows while it runs.
That single sentence is the whole point of an operating system.

### Item 4 — ship a program that is not built in

The proof. A `.zl` file on disk, not in `kernel.zl`, that draws something in a
window.

**Gate:** edit the file, `run` it again, see the change — **without rebuilding
the kernel.** The first time this project does something it was not compiled to
do. Screenshot it.

---

## 6. What this changes about the boot log

The boot log currently says:

```
[ INFO ] no heap, no filesystem, no scheduler
```

That line has been honest and it is about to stop being. **Update it as part of
the work**, not afterwards. This project holds `HANDOFF.md` to recording what
is *verified* rather than intended; the screen is held to the same standard —
which is exactly the argument being made about the decorative `net up` label in
`SYSTEM-PROMPT.md`.

---

## 7. What "done" means

- a gate command **and its output**
- for Item 4, a **screenshot** and the fact that no rebuild happened
- for the failure modes, the **actual line** the machine printed
- a number you **measured**

---

## 8. Hazards

- **`runtime_kernel.c` refuses lists and strings on purpose.** `zl_list_n` is a
  hard fault. Level 1 has to change that, and changing it is a design decision
  with a boot-log consequence — see §6.
- **`-mgeneral-regs-only` on `idt.c`** — no SSE in an ISR, and no calling out to
  code that uses it. A preemptive timer that switches tasks touches this
  directly, and getting it wrong killed the 64-bit boot once already.
- **Multi-MB buffers never in BSS**; check the high-RAM map.
- **The zl parser takes no multi-line call arguments.**
- **Local timestamps are unreliable** — RTC runs slow, NTP corrects mid-session.

---

## 9. Agents

Authorised, and this track has the best case for them of any:

- **An adversarial reviewer on the kill path.** "Can you write a script that
  wedges the machine?" is exactly the question the author is worst placed to
  answer, and Item 2's whole value is that the answer is no.
- **Fan out readers** across `interp.c`, `runtime.c` and `runtime_kernel.c` to
  produce an exact list of every allocation site the port must change, before
  any porting starts. That inventory is the difference between a week and an
  afternoon.

**Not** for parallel QEMU boots.

**If you delegate, you own collection.** Never end a turn waiting on an agent.

---

## 10. The honest summary, for whoever reads this cold

zlOS today is a very good single program with a GUI. It has a compositor, a
toolkit, a terminal, a renderer that does gamma-correct subpixel text, and
fifteen drivers — and **every line of it was compiled in before boot**.

The gap between that and an operating system is this document. It is not a
large gap in lines. It is a large gap in **decisions**, and §1 is the one that
matters.

---

System track (filesystem): [`SYSTEM-PROMPT.md`](SYSTEM-PROMPT.md) · Contract:
[`../ui.h`](../ui.h) · Decisions: [`DECISIONS.md`](DECISIONS.md) ·
Scheduler: `../sched.c`
