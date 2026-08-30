# The system track — what was built, and what it cost

The substrate brief: storage, time, clipboard, and the things that make a
collection of programs behave like one machine. This is the account of what
landed, what it refuses to do, and the six ways the filesystem was broken
before it was any good.

Companion to [`SYSTEM-PROMPT.md`](../archive/prompts/SYSTEM-PROMPT.md), which is the brief. This
is the result.

---

## The brief's finding 0.2 was wrong, and it changed the work

> "`nvme_read_block`, `nvme_write_block` and `nvme_ready` … **Zero of them are
> exposed as builtins**, and `kernel.zl` mentions nvme exactly once — in a
> diagnostic."

Fifteen of them were already exposed. The grep that produced that finding
searched for `nvme`, and the builtins are named `nv_*` — `nv_read`, `nv_write`,
`nv_get`, `nv_set` and eleven more, all in `runtime_kernel.c` since before this
track started. `kernel.zl`'s `o` command already wrote a pattern to LBA 1000
and read it back.

So Item 1 was not "five lines of binding". The real gaps were two, and both
mattered more than the one described:

1. **There was no way to move a block to or from an arbitrary address.** Every
   transfer landed in one fixed page and came out through `nv_get`, one byte
   per call — 512 crossings of the zl/C boundary per block. That is not a
   performance question. It is the reason nobody was ever going to build a
   filesystem on it.
2. **Persistence had never been proven.** Writing a block and reading it back
   in the same boot proves a DMA round trip and nothing else; a filesystem
   backed entirely by RAM passes that test perfectly.

Both are closed. `nvme_read_to` / `nvme_write_from` take an address, and
`verify-disk.sh` power-cycles the machine three times.

---

## zlfs

A superblock at a fixed LBA, a flat directory of 32 fixed-size entries, files
as contiguous runs. No heap, no free list, no extents, no subdirectories.

### The decisions worth recording

**The volume starts 1 MiB in, expressed in BYTES not as LBA 2048.** 2048
blocks is 1 MiB only on a 512-byte device; on a 4096-byte one it is 8 MiB in,
which silently refused to format anything smaller. The host harness found this
the first time it tried a second geometry.

**Directory entries are raw bytes, not a struct.** Three different compilers
build this file — `gcc -m32`, `gcc -m64`, and `clang` targeting PE32+/LLP64. A
struct that pads differently under one of them is a disk that mounts on two
machines out of three.

**The superblock checksum covers the whole block, not the header.** A
header-only checksum passes on a block whose tail is garbage, and the tail is
exactly where a half-completed write leaves garbage.

**Checks run in an order that produces the right message.** Magic, then
version, then checksum, then geometry. Corrupt the geometry field and the
checksum catches it first — which is correct defence in depth, and means the
only way to reach the geometry check is a superblock that is internally
consistent and still describes a disk this is not. That is what an image copied
from a larger disk looks like, so it is worth checking for.

**A file that outgrows its run is copied, and the new run may not overlap the
old one.** `alloc_run` has no `skip` argument. It used to, so a growing file
could reuse its own blocks — which is exactly what made a failed copy
unrecoverable, because it had eaten the bytes it was copying from. The cost is
that a file cannot grow past half the free space. That is a stated limit rather
than a window in which a power cut loses the file.

**A run never shrinks in place.** Writing 2000 bytes into a file that owns
eight blocks leaves it owning eight. Giving them back means the next write past
2000 bytes has to relocate, and written-short-then-long is the common case.

### What it refuses to do, out loud

Every refusal prints the value that caused it. A filesystem that silently
treats garbage as data is worse than one that will not mount.

- no volume here — prints the magic it found and the one it wanted
- a future on-disk version — prints both numbers
- a bad checksum — says the volume is *damaged*, not empty
- geometry that does not fit this disk
- a directory entry whose run leaves the volume
- a block size over 4096 — see below
- no free directory slot, no contiguous run long enough
- a name longer than 23 characters — refused, never truncated

---

## The six defects, and how they were found

`fs.c` was handed to a fresh agent with one instruction — **lose a file** — and
told to prove anything it claimed by running it. It broke the filesystem six
ways in eleven minutes. The author had already written 63 passing assertions
against the same code.

| | defect | what it cost |
|---|---|---|
| D1 | `blocks_for()` overflowed: `(bytes + bsize - 1) / bsize` wraps for the top 511 u32 values and returns **zero** | a zero-length run on top of a live file; the volume then unmountable *forever*, losing every other file; and `alloc_run`'s cursor stopped advancing, so the next create **span in an infinite loop** |
| D2 | `fs_write` published the new run *before* copying | a failed relocation left the file pointing at a deleted file's bytes while its own sat orphaned. It read back as someone else's deleted contents, and the next unrelated `dir_flush` made it permanent |
| D3 | mount's `st + nb > vol_end` wrapped | an entry with start 3000 and 4294964396 blocks summed to 100 and passed; `alloc_run` then handed out blocks *below* the data area, so an ordinary create+write **overwrote the superblock** |
| D4 | names over 23 chars were truncated into the entry while `fs_find` compared the full string | the file could never be found, so the duplicate check never fired, so creating it twice made two entries with identical names and one unreachable |
| D5 | a failed **reformat** left the old superblock valid | the volume mounted with a half-erased directory — four files out of twenty |
| D6 | the same wrap on the superblock's own geometry | a volume claiming 4.29 billion data blocks mounted |

A seventh was found afterwards, from the reviewer's own note about what it had
*not* tested:

| D7 | `dir_flush` writes four blocks; a failure part-way left the on-disk directory a mixture | a create that reported **failure** could still leave a phantom file on the platter, appearing at the next mount, pointing at blocks nobody wrote |

D7's fix is `dir_commit()`: on a failed flush, put the entry back and flush
again. If *that* fails the volume is not under our control, so it unmounts
rather than continuing to write to a directory it cannot predict. Those are two
different outcomes and the harness tests both — a transient glitch the retry
recovers from, and a disk that takes no writes at all.

### The lesson, stated plainly

**D2's comment was worse than D2.** The code published the start and blocks
early and the comment above it asserted, in so many words, that "a failure
anywhere leaves the file exactly as it was". A reader checking that invariant
would have believed the comment. The author wrote both.

Every expected value in `rtctest.c` came from `date -u -d ... +%s` for the same
reason. The first draft of one of them was wrong by 7200 because the arithmetic
was done in my head.

---

## Two guards `nvme.c` never had

**Block size.** Every command carries PRP1 and no PRP2, so one transfer is one
4 KiB page. The driver accepted LBADS up to 16 — a 64 KiB logical block — and
would have had the *controller* DMA past that page into the xHCI arena, with a
successful completion status. Nothing common reports over 4096, which is
exactly why it would have sat there. It refuses now.

**`nvme_ram_ok()`.** Every other driver here ships one; this one did not. It
probes the lowest *and highest* address of the arena, because the failure mode
is a boundary and a base-only probe passes on a machine whose RAM ends in the
middle.

Both are instances of the project's documented recurring bug — a DMA buffer
outside guest RAM, or an address truncated to 32 bits, five times so far, and
it reads as a protocol bug every time.

---

## The clock

`rtc.c`, the MC146818 through ports 0x70/0x71. Three things make it a driver
rather than two `inb` instructions:

**The torn read.** Waiting for the UIP flag is necessary and not sufficient:
the spec grants 244 µs afterwards, and reading seven registers can exceed that
on an emulated machine under load — which is the machine this runs on. Read at
the wrong moment across 10:59:59 → 11:00:00 and you get **10:00:00**: minutes
and seconds already reset, hour not yet carried. An hour wrong, occasionally,
and unreproducible when you go looking for it. So the whole set is read twice
and accepted only when two sweeps agree, and after eight tries it *refuses*
rather than guessing.

**Three encodings, all optional**, announced in status register B: BCD or
binary, 24-hour or 12-hour. In 12-hour mode the PM flag sits on top of the hour
digits, so it must come off *before* the BCD decode or 1 PM reads as 81. And 12
AM is midnight while 12 PM is noon — the one case where `hour + 12` is wrong in
both directions.

**NMI.** Bit 7 of port 0x70 is the NMI disable line, not part of the register
address. Writing a register number without masking leaves non-maskable
interrupts off for the rest of the machine's life, and the symptom shows up in
an unrelated subsystem months later.

That torn read cannot be reproduced on a running machine, so `rtctest.c` fakes
the chip: it can hold UIP high, hand out a different time on the second sweep,
claim any encoding, or not be there at all.

---

## The header stopped lying

It drew `net`, a green dot, and `up`. There is no network driver anywhere in
this tree — no e1000, no virtio-net, no rtl8139, no network code at all. That
dot was a claim about hardware that does not exist, made to the user on every
frame.

`HANDOFF.md` is held to the standard that it records what is *verified* rather
than what is intended. The screen is now held to the same one.

**Changed the label rather than writing the driver, deliberately.** Nothing in
this worktree could boot at the time, so a virtio-net driver would have
replaced a decorative lie with an unproven claim, which is not an improvement.
What is there instead is the one thing that corner can truthfully say: the real
time. When the clock cannot be read it spells `--:--`.

---

## Testing without a machine

Every module here talks to hardware through a seam of two to four functions,
and every seam has a fake:

| module | seam | the fake can |
|---|---|---|
| `fs.c` | `fsdev_read/write/bsize/blocks` | fail a chosen write, once or for ever |
| `rtc.c` | `rtc_port_in/out` | hold UIP high, tear a read, change encoding, vanish |
| `clip.c` `snap.c` `notify.c` | none needed — integer logic | — |

This was not a preference. **The kernel did not link for the entire first half
of this track** (`.ultra/TENSIONS.md` T-SYS-1: `b19207d` committed the call
sites of the USB pointer with no definitions on any branch), so every QEMU gate
in the project was unrunnable. `kernel/tests/host/` never links
`runtime_kernel.c`, so it was unaffected, and all of this was built and gated
there while the tree was red.

The regression check that works while the tree does not link: **diff the set of
`undefined reference` symbols against a baseline.** Unchanged means your new
file added none. Four new `.c` files went in that way.

---

## The gates

```
kernel/tests/host/fstest      117 assertions  zlfs, incl. a cold start in a SEPARATE PROCESS
kernel/tests/host/rtctest      50 assertions  the clock, against a misbehaving CMOS
kernel/tests/host/systest      96 assertions  clipboard, snapping, notifications
kernel/tests/host/toasttest    17 assertions  the toast INSIDE the compositor, by pixel
kernel/verify.sh                            BIOS boot vs golden.txt
kernel/tools/checks/verify-disk.sh                       THREE boots against one disk image
kernel/tools/checks/verify-clock.sh                      three chosen -rtc base= values, exact output
```

`verify-disk.sh` is the only gate in the tree that power-cycles the machine. It
asserts `BOOTCOUNT` goes 1 → 2 → 3, and the third boot is not padding: 1,1,1
means the volume is reformatted on every mount and 1,2,2 means the second write
never reached the platter. Both look like success at two boots.

It uses `-m 512` because the NVMe arena is at 208 MiB and QEMU's default is
128. It polls for its marker rather than sleeping, because a fixed timeout
turns a busy box into a failed gate on unchanged code. And it refuses to start
if another QEMU is running or the load average is above 4 — four sessions share
this machine and §1.2 allows one VM.

---

## The toast, which is the one thing this track put in `wm.c`

§2 permits adding a notification surface and forbids changing routing, damage
or z-order. The surface is 40 lines and touches none of those three: a
`toast_draw()` called last inside each damage rectangle in `wm_repaint`, and a
`notify_tick()` in `wm_frame` that damages **only the toast's own rectangle**
and only when what is on screen actually changed.

**A toast is not a window.** It is not in `wins`, not in `zorder`, and has no
window id. That is not a shortcut — it is the feature. A notification that
takes focus eats the next keystroke: you are typing, something completes, and
the character you were mid-way through goes to something about to close itself.
There is nothing here that *could* take focus, which is a stronger guarantee
than remembering not to.

`hosttest/toasttest.c` asserts it against the real compositor, with a window
deliberately placed **underneath** the toast so "it appears" means "on top of a
window" and not "on empty wallpaper":

```
after a frame the toast is ON SCREEN                    ok
    62685 pixels of toast inside its rectangle
...and it is ON TOP of the window, not behind it        ok
focus is STILL the window, not the toast                ok
...and the window count is unchanged                    ok
...it has no place in the z-order at all                ok
...and the pixels are GONE, repainted by what was under ok
    0 pixels left behind
...leaving no ghost - the window under it is intact     ok
```

That last pair is the classic compositor bug and it is only visible in the
frame *after* the one you would screenshot: a surface that retires without
damaging its own rectangle leaves a ghost on the wallpaper for ever.

---

## Snapping, and `wm_resize`'s first caller in its life

`wm_resize` has existed since the compositor was written and **nothing had ever
called it**, which is why every window was whatever size `layout()` decided at
boot, for ever.

Two triggers, both thin — all the arithmetic is in `snap.c` and is asserted on
the host with no compositor at all:

- **drag to an edge.** The drop point decides the zone; corners beat edges,
  because a pointer in the top-left is inside both strips and "quarter" is the
  more specific intent.
- **Super+arrow.** `MOD_SUPER` has been tracked by `input.c` since it was
  written and read by nothing at all; this is its first reader. Left and right
  walk half → quarter → other quarter, so a four-up layout is four keystrokes.

**The bug the harness caught.** A drag has *already moved* the window by the
time it is dropped, so capturing the restore rectangle at the drop stores the
**dragged** position — the window comes back the right size in the wrong place.
The restore rectangle is now taken when the drag *starts*. That is the second
version of "the part people forget": the first is not storing it at all, the
second is storing it on every snap, and this is the third.

`toasttest.c` drives it through the real compositor, as a sequence of pointer
frames rather than direct calls, because a drag is only a drag if the frame
loop sees it:

```
dropping it at the left edge SNAPS it                    ok
...to exactly the left half of the work area             ok
    640x608 at 0,64   (work area starts at y=64, dock at y=672)
...which is below the header                             ok
...and stops above the dock                              ok
dragging it off the edge clears the snap                 ok
...and leaves it the size it was, not teleported         ok
un-snapping restores the ORIGINAL size, not the left
    half it passed through                               ok
...and its original position                             ok
```

`wm_close` now calls `snap_note_closed`, or a window opening into a reused slot
inherits a restore rectangle that belonged to something else.

**This changed `route_key` and `route_mouse`, which §2 fences off.** The rule's
stated purpose is keeping concurrent sessions apart, and by the time this
landed all three sibling worktrees had committed and were clean — `wm.c` was
nobody's open file. The diff is a `snap_to_rect()` helper, four lines in the
drop path, and eight in `route_key`. Called out here rather than buried.

---

## What is NOT done, and why

**`term.c` is untouched** — platform track owns it. That is why the disk
commands are single characters, `.` `,` `;` `'`: the typed-word table lives
there, and giving these real names is a two-line change for whoever owns it.

---

## The clipboard, and why no routing changed for it

Ctrl+C is character 3 and Ctrl+V is 22 — `input.c` has always produced them and
`route_key` has always delivered them to the focused app as an ordinary
`EV_CHAR`. So the clipboard needed **no change to the compositor at all**, only
apps that act on those two codes.

There are two, and they share nothing else:

- the **editor** copies its whole buffer on Ctrl+C and appends the clipboard on
  Ctrl+V, and has no idea what else exists
- the **`'` command** writes the clipboard to `clip.txt` on the disk, and has
  no idea where the bytes came from

That is the whole feature. Copy in the editor, press `'`, and the text is a
file on a disk that survives the power going off — through one buffer that
neither end knows anything about.

One thing had to be added to make Ctrl+C reachable at all: the PS/2 path in
`edit_key()` tracked **shift and nothing else**, so `Ctrl+C` arrived as plain
`c`. It now tracks ctrl (scancodes 0x1D/0x9D) and returns `band(ch, 31)` for a
ctrl+letter — which is not a convention invented here, it is exactly what
`input.c` already produces on the USB path, so the editor sees the same codes
whichever keyboard is being typed on.
