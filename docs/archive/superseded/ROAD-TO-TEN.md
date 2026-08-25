# Road to ten — what the project actually scores, and what moves it

**Written 2026-08-19. Every number below was measured this session, or carries the
file and line that establishes it. Where I could not measure something, it says so.**

> **Evening re-check, same day (HEAD `3f00366`).** The two delivery gaps still
> hold the score. What changed: `main`'s browser home page no longer lies about
> the NIC (`browser.c:158-165` now says HTTP works and HTTPS is refused because
> no TLS is *linked*). The TLS 1.3 / X.509 work is still the 21 unmerged
> commits on `desktop/browser-next`. `key()` still kfatals the panel path
> (`kernel.zl:1517`). Pointer double-drain is gone (`input.c:707` calls one
> `xhci_poll(32)`). The land gate now executes hosttests. The self-hosting
> fixpoint is in `run_tests.sh` and `.github/workflows/gates.yml`.
>
> **Second pass, same evening.** Three corrections that change the ranking, not
> the score:
> 1. `desktop/browser-next` is **83 commits behind `main`**, not a fast-forward.
>    "Zero new engineering" is false. Treat it as a real merge, after the
>    hardware proof, not as this week's cheap win.
> 2. The first ThinkPad boot does **not** need `P`. The runbook is `h z k * u ?
>    x`. `o` writes LBA 1000 of whichever NVMe it finds — on the laptop that is
>    Kali's disk. `P` is a landmine (`key()` kfatals) but it is not on the
>    recorded-boot path. Cold-start modeset is explicitly out of scope.
> 3. The product gap the score ranking underweights: a named file cannot
>    survive a reboot from the GUI. zlfs exists; the editor saves RAM slots;
>    the Files app was built in a Codex `/tmp` clone and destroyed. Spec:
>    `kernel/docs/archive/handoffs/files-app-handoff.md`.

This exists because an external assessment scored the project "7 out of 10 if it
is the common shape — real kernel, real compiler, **ported libraries for the hard
parts**, heavy AI assistance, runs in QEMU" against "9.5 if it is original work
on bare metal, documented and published."

**The "ported libraries" half of that 7 is false here, and it is the half that
was worth the most.** Nothing in the hard parts is ported. That is measured
below. What is true is the QEMU half, and the published half.

So the project does not sit at 7. It sits at roughly **8.5**, held there by two
things that are both *work in hand* rather than work unbuilt:

1. **zlOS has never booted on real hardware.**
2. **The strongest evidence of originality in the whole repo is on an unmerged
   branch, and `main` actively says it does not exist.**

Neither is a capability gap. Both are delivery gaps, which is why they are cheap
and why they are ranked first.

---

## 1. The calibration, discriminator by discriminator

The assessment's own list of what separates 7 from 9.5, scored against the tree.

| Discriminator | State | Evidence |
|---|---|---|
| Own bootloader | yes | `kernel/raw_boot.asm`, `raw_entry.S` |
| Own UEFI application (not GRUB) | **yes** | `kernel/efi.c`, own `BOOTX64.EFI`, gated by `verify-efi.sh` |
| Real→protected→long mode, own page tables | yes | `boot64.S:40-92` — PML4 at `0x1000`, `cr3` loaded at `:92` |
| Self-hosting fixpoint | **holds — ran it** | see §4 |
| Backend emits | LLVM IR text + hand-written x86-64 ELF | `compilel.c`, `nativegen.c` |
| Own IR / SSA / register allocator | **no** | `nativegen.c:10` "Strategy: STACK MACHINE"; `compilel` hands SSA to LLVM |
| **Ring 3 / user mode** | **none at all** | no TSS, no `syscall`/`sysret`, no `MSR_LSTAR`, no per-process PML4 anywhere in `kernel/` |
| Syscall pointer validation | n/a — no syscall boundary exists | — |
| Page fault used for anything | **no — always a crash** | `idt.c:458` routes all 32 CPU exceptions to one `fault_isr` that halts |
| SMP | APs start; band rendering **switched off** | `smp.c`, `smp_trampoline64.S`; 1.78× measured, `smp_go()` reachable only from the old text shell |
| Preemptive scheduler on the boot path | **no** | `sched.c` 305 lines, in SOURCES, reached only from the `+` demo key (`kernel.zl:1771`) |
| Own TCP | yes, shallow | `tcp.c` 812 lines. Slow start only — no congestion avoidance, no fast retransmit, no SACK (`tcp.c:20-22`, `:710`) |
| IP fragment reassembly | no, and says so | `net.c:357-359` counts fragments and drops them |
| **TLS** | **own TLS 1.3, not ported** | `tls.c` 649 lines, `TLS_AES_128_GCM_SHA256`, X25519 |
| **Certificate chain validation** | **full, and correct** | see §2 |
| Crypto primitives written | SHA-1/256, HMAC, PBKDF2, AES-128/CTR/CMAC, RSA, ECDSA, 802.11i PRF | `crypto.c` 983, `rsa.c` 231, `ecdsa.c`, `entropy.c` |
| JavaScript | own bounded tree-walking interpreter | `js.c` 1,146 lines, no GC, bounds documented in `js.h` rather than discovered |
| HTML / CSS / PNG | own | `html.c`, `css.c`, `layout.c` (2,244 added), `png.c` 944 |
| **Runs on real hardware** | **never** | `.ultra/STATE.md` open item 1 |
| Test suite | 2,133 zl assertions + ~26 host harnesses, **and the gate runs them** | `run_tests.sh`; `gates/land-gate.sh:74-90` executes every harness. `STATE-OF-THE-PROJECT.md` §2.1 is closed and stale |
| CI | 6 GitHub Actions workflows | `.github/workflows/` — `STATE-OF-THE-PROJECT.md` §2.5 is stale on this |
| Git history | 311 commits, incremental, honest subjects | avg subject 67 chars; messages like "the guards that did not guard" |
| External review | **zero** | no OSDev post, no upstream patch, no outside reader |

**The one axis I cannot score:** whether the hard parts can be explained without
looking. That is the assessment's real test of depth under AI assistance and no
command settles it. It is not on this plan because it is not a code task.

---

## 2. What is already at 9.5 and is invisible from outside

This is the finding that changes the ranking.

`desktop/browser-next` is **21 commits and 20,272 insertions unmerged**, and it
contains the strongest originality evidence in the project:

- **TLS 1.3 client handshake, written.** `tls.c:181-217` — legacy version 0x0303,
  `supported_versions` pinned to 0x0304, one ciphersuite.
- **X.509 chain validation that is textbook-correct**, which is rare even in
  shipped code:
  - signature verified at *each* link, not just the leaf — and `x509.c:534-539`
    records that comparing subject DNs alone *was* a complete authentication
    bypass, with the exploit written down, and fixed
  - `notBefore` / `notAfter` both checked (`:492-497`)
  - hostname matched against **SAN, not CN** (`:439-440`, with the reason)
  - intermediates must carry CA (`:520`)
  - unknown root ⇒ **refusal, not a warning** (`roots.c:8-10`)
- **`entropy.c` reports its own quality tier** — `RND_HW` / `RND_WEAK` /
  `RND_NONE` — and the caller must refuse to connect on `RND_NONE`. The comment
  explains that a weak RNG produces a handshake that "looks perfect from both
  ends and is transparent to a third party." That is a security-engineering
  instinct, not a feature.

**Correction, made the same day against the tree.** An earlier draft of this
section said `main`'s browser home page lies to the user about the network and
about hashes, and quoted `browser.c:141-144`. **That was stale — both clauses
were already fixed.** `main` now reads (`browser.c:158-168`):

```
"<li><strong>the network</strong> - a real URL over <code>http://</code> is
 resolved by name, fetched and drawn</li>
...
"<li><strong>HTTPS.</strong> Refused, deliberately. There is no TLS in this
 kernel - no ciphersuite, no certificate chain validation, and no cipher or
 hash primitive linked into it at all ..."
```

That is **accurate for `main`**, precisely worded, and it earns the project
credit rather than costing it. So the argument for landing the branch is not
"stop lying" — it is narrower and still holds:

**The best work in the repository is invisible.** `main` correctly reports that
it has no TLS. The TLS exists, it is 21 commits away, and anyone scoring this
project against the certificate-validation questions is scoring `main`. Merging
is zero new engineering and it is the difference between "no TLS in this kernel"
and a verified TLS 1.3 handshake with a correct chain.

---

## 3. The plan, ranked by score-movement ÷ work

### Tier 1 — pure proof. Nothing new gets built; the ceiling moves anyway.

**1. Boot the ThinkPad.**
The one item that converts "runs in QEMU" into "runs on bare metal", which is the
literal wording of the gap between the two scores. Everything needed is already
built: `verify-efi.sh` gates the exact path a real machine takes,
`kernel/docs/guides/thinkpad-first-boot.md` is the runbook, the USB image has been
rebuilt (it was two builds stale). `.ultra/STATE.md` open item 1: flash
`/dev/sda`, boot, run `blit`, steps 1–7 with 7 as the win.
**Record it on video.** An unrecorded bare-metal boot is worth about as much as
an unmerged branch.

**2. Land `desktop/browser-next`.** §2. Twenty-one commits. The `browser.c:141`
string is already fixed there (`acec0f5`).

**3. Get an artifact for the Wikipedia claim.**
`kernel/docs/browser-status.md:37` asserts it "fetches `https://en.wikipedia.org/`
by name over verified TLS 1.3, and renders it with images, flexbox and grid."
**There is no screenshot on that branch to support it** — `docs/evidence/visual-diffs/` holds two
merge-help PNGs and nothing else. This is precisely the assessment's Q41
("screenshot it"), and it is the project's own honesty rule applied to its best
result. `browsershot` exists; run it and commit the render.

### Tier 2 — the one architectural gap that is genuinely a gap

**4. Ring 3 and a syscall boundary.**
The assessment says this "moves the score more than any other" and zlOS has
none of it. The *bounded* version — not "port Linux":

- a TSS with an IST stack, and a ring-3 code/data pair in `gdt64.c`
- `MSR_STAR` / `MSR_LSTAR` / `MSR_SFMASK`, a `syscall` entry stub, `sysret` back
- one user PML4 built from `boot64.S`'s existing tables, so the kernel is mapped
  but not user-accessible
- **pointer validation on the syscall boundary** — the question the assessment
  singles out. A user pointer must be range-checked against the user half before
  a single byte is copied.
- a `#PF` handler that is not `fault_isr` — even if all it does at first is
  report `cr2` and kill the process rather than halt the machine

Call it 800–1,200 lines across `gdt64.c`, `idt.c`, a new `syscall.S`, and a new
`proc.c`. Four unknowns to resolve: IST layout, the `sysret` `RCX`/`R11`
contract, where the user stack lives in the high-RAM map, and whether
`-mgeneral-regs-only` covers the entry stub (see `kernel/docs/reference/system/input-stack.md`).

**The first user process is already written and already stranded.**
`interp_kernel.c` is 721 complete lines, in no build, whose `k_malloc` already
routes to `arena_alloc` (`:191`) — and `arena_alloc` currently has no kernel
caller at all (`STATE-OF-THE-PROJECT.md` §6.2). Running the zl interpreter as a
ring-3 process closes §6.1, §6.3, §6.4 and half of §6.2 in one increment, and
turns "a language and an OS in the same repo" into "the OS runs the language."

That is also the thing no other hobby OS in this class has: **zl programs as user
processes on zlOS.** It is the originality axis the assessment says is the only
thing that counts above 8.

### Tier 3 — proof machinery. Cheap, and disproportionately credible.

**5. ~~Make the land gate run what it builds.~~ DONE — verified against the tree.**
`STATE-OF-THE-PROJECT.md` §2.1 (~26 host harnesses compiled and thrown away) is
closed: `gates/land-gate.sh:74-90` now has a `=== hosttest run ===` block that
executes every binary with a 180 s timeout and fails the gate on any non-zero
exit. That entry in `STATE-OF-THE-PROJECT.md` is stale — as is §2.5, since `main`
has six workflows in `.github/workflows/`.

What replaces it here: **the fixpoint gate, which was genuinely missing and is
now wired** (§4). Beyond that, the remaining test-surface item is the one the
assessment would ask about next — *what does the suite not cover?* Today the
honest answer is the display driver and anything needing real hardware.

**6. A README that states the split plainly** — own / ported / AI-assisted, per
subsystem. The honest answer here is unusually good (**nothing in the hard parts
is ported**; the two borrowings are reference material, not code: an 80-byte Gen9
pixel shader lifted from Mesa and a captured 77-packet blend pipeline, both
documented as such). Overclaiming is the only thing that can hurt; there is
nothing here to overclaim about.

**7. External review.** OSDev forum, r/osdev, and one patch through review on a
real project. Unlike everything above, this cannot be practised alone, and the
assessment is right that it is the part almost nobody does.

### Tier 4 — real depth, but not the ceiling

**8. SMP band rendering on.** 1.78× on the desktop redraw, written, switched off.
Turns "SMP exists" into "SMP is load-bearing." Best win-to-risk on the board per
`.ultra/STATE.md`.

**9. TCP Reno.** Congestion avoidance + fast retransmit + fast recovery on top of
`tcp.c`'s existing slow start. The file already names its own absence at `:20-22`.
~150 lines and it makes "I wrote a TCP stack" survive follow-up questions.

**10. An IR and a register allocator for `nativegen`.** Today the honest answer
to "SSA? register allocation?" is *stack machine, and LLVM does the real one*.
This is the largest compiler-depth item and the least urgent, because the
language is not what is being scored short of a 10.

---

## 4. What I measured this session

**The self-hosting fixpoint holds.** It had never been run on Linux — the check
lived in `verify.ps1`, a PowerShell script that did not survive the port, and
nothing in `run_tests.sh`, `gates/land-gate.sh` or `.github/workflows/` runs it.

```
$ cd /tmp/zlfix && cp compiler.zl input.zl
$ interp compiler.zl                 # gen1: interpreter compiles the compiler
$ gcc -O2 -w -D_strdup=strdup -o zlc1 gen1.c runtime.c os_linux.c -lm
$ ./zlc1                             # gen2: that binary compiles the same source
$ md5sum gen1.c gen2.c
27e04c382694276d3001b57df144f001  gen1.c
27e04c382694276d3001b57df144f001  gen2.c
```

`f(f(x)) == f(x)`. Byte-identical, 936 lines.

**The precise claim, because this is exactly where projects overclaim.**
`compiler.zl` is 716 lines and compiles a *subset* — `fn`, `if`/`else`, `while`,
`for`, `and`/`or`/`not`, `return`, lists, strings — to C. It is a real
self-hosting bootstrap and the fixpoint is real. It is **not** the production
toolchain: `interp.c`, `compile.c` and `compilel.c` are C. So the honest sentence
is *"zl has a self-hosting compiler for a subset, and the fixpoint is checked"*,
not *"the zl toolchain is self-hosting."*

**Action: this belongs in a gate.** It takes under two seconds, needs no QEMU and
no hardware, and it is the one check that proves the language did not silently
break. Add it to `run_tests.sh` and to `.github/workflows/gates.yml`.

---

## 5. The weakest link in this document

The `desktop/browser-next` assessment in §2 is based on **reading that branch's
source, not on running it.** I verified the code does what the file names claim —
the ciphersuite bytes, the SAN-over-CN decision, the per-link signature check,
the entropy tiers — and I did not build it, boot it, or watch a handshake
complete. The Wikipedia claim is the branch's own assertion with no artifact,
which is why getting one is ranked at Tier 1 item 3 rather than assumed.

Second weakest: the ring-3 line count in Tier 2 is an estimate from the shape of
the work (four files, four named unknowns), not from a written spike. Treat it as
a scope sketch, not a measurement.
