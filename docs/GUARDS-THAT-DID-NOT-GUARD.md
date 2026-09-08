# Guards that did not guard

Written 2026-08-19; §6 added 2026-09-04. Five checks in this tree (twenty-one with
§6) that reported green, or read as coverage, while checking nothing — each with the command that establishes it.

This is not a list of embarrassments. It is a list of **shapes**, because the
same shape keeps recurring here and it is cheap to recognise once named:

> A check whose failure mode is silence looks exactly like a check that passed.

`WORKING-RULE.md` already says *a thing is done when a command says so and you
read the output.* Every case below is what happens when the first half is true
and the second is not.

---

## 1. The EFI `-Werror=` guard was inert for its whole life

`buildefi.sh` carried, and `CLAUDE.md` defended:

```
-w -Werror=shift-count-overflow -Werror=void-pointer-to-int-cast
   -Werror=pointer-to-int-cast  -Werror=int-to-pointer-cast
```

with the stated reasoning: *"Clang applies flags left to right, so these must
stay after the `-w`."* Plausible, written down twice, **never run.**

### The measurement

One file with one instance of each class, compiled three ways
(`kernel/tools/checks/wguard.sh` automates this):

| flags | result |
|---|---|
| the four `-Werror=`, no `-w` | **5 errors** |
| `-w` then the four (the shipped order) | **exit 0, silent** |
| the four then `-w` (the "wrong" order) | **exit 0, silent** |

`-w` wins regardless of position. clang 21.1.8. The ordering claim is false in
both directions, so the guard never fired once.

### What was behind it

**33 casts of exactly the class the guard names**, across six files:

```
11  freestanding/runtime_kernel.c      11  xhci.c       7  fb.c
 2  smp.c                               1  console.c    1  http.c
```

Three of them mattered beyond tidiness:

- **`smp.c`'s `ENTRY_PTR` store is the documented bug verbatim.**
  `(unsigned long long)(unsigned long)smp_ap_main` — a 64-bit destination handed
  an address already truncated to 32 bits. That word is the entry point every
  application processor jumps to. `CLAUDE.md` describes this exact defect under
  *"As a cast, which the struct fix did NOT cover"*, and it was still in the
  tree, behind the guard written to stop it.
- **`fb_phys()` was the unfixed half of a chain someone else had already
  fixed.** `fb.c`'s own comment traces the framebuffer address being widened to
  64 bits end to end — `efi.c` → `console_init_efi` → `console_init_fb` →
  `fb_setup` → `fb_base` — and warns that changing a subset is worse than
  changing none. The address coming back **out** (`fb_phys` → `console_vram` →
  zl's `vram()`) still went through `unsigned long`. On firmware that places
  the GOP framebuffer above 4 GiB, `vram()` returned a truncated address — and
  `vram()` exists precisely so zl can poke at it.
- **`runtime_kernel.c`'s `peek`/`poke`/`fill_mem`/`copy_mem`** all capped at
  4 GiB on the EFI target, while a zl number carries 53 bits of address.

### The tell nobody read

Two files carried defensive code **citing this guard as the reason for it**.
`virtio_net.c:268` still does, and is still correct:

> "...and buildefi.sh makes exactly this class `-Werror` because clang once
> turned such a shift into a bare `ret` in the boot path."

`fb.c`'s `fb_uptr` typedef carried the other, until this change rewrote it while
moving the typedef above the six casts that were still using `unsigned long`:

> "Casting a 64-bit integer straight to a 32-bit pointer is a warning on the
> 32-bit builds and an **ERROR under buildefi.sh's
> `-Werror=int-to-pointer-cast`**, so the narrowing is spelled out once, here,
> where it is provably safe."

It was not an error. It was silent. Both authors wrote careful, correct code to
satisfy a guard that did nothing — and `fb.c`'s author wrote *"spelled out once,
here"* in a file that had six more of them further up, which the guard would
have named instantly had it been running. **A belief that produces work is not
thereby a true belief.**

### Now

`-w` is gone. The four are genuinely fatal, the source set is clean under them,
and the one warning `-w` was legitimately buying (`-Wexcessive-regsave`, 11 hits
in `idt.c`, inherent to `__attribute__((interrupt))`) is suppressed **by name**
so a new class surfaces instead of being swallowed.

`kernel/tools/checks/wguard.sh` runs all three directions — guard bites, `-w` still
silences, tree is clean — in about two seconds with no QEMU. **Direction B is
the one that matters:** it fails if clang ever changes behaviour, which is the
only world where restoring `-w` would be safe.

---

## 2. `check-memmap.sh` cannot see a single line of C

`274b8f6` replaced its hardcoded name list with a discovery sweep, on the right
principle — *"a detector that cannot see a new constant is not a detector, it
is a green light with a hardcoded allowlist."*

The sweep is:

```sh
SRC=${1:-kernel.zl}
grep -oP '^[A-Z_]+\s*=\s*\K0x0[0-9A-Fa-f]{5,}' "$SRC"
```

It reads **`kernel.zl` and nothing else**, and matches only `UPPERCASE = 0x…`
at the start of a line. So the entire C-side high-RAM map is outside its
vision: `memmap.h`'s `HI_*` constants, and any raw literal in a `.c` file.

Which is why it never saw this, sitting in the tree the whole time:

```c
static uptr edid_buf = 0x0C980000u;      /* intel.c */
#define HI_BLUR   0x0C000000UL           /* memmap.h — 16 MiB arena */
```

`0x0C980000` is 9.5 MiB into `fb.c`'s cached-blur arena. `fb.c` hands out that
space and knows nothing of `intel.c`; `intel.c` does not `#include "memmap.h"`
at all. Running the checker today prints a clean six-region map and says `OK`
(21 `HI_*` bases by 2026-09-04, still `OK`).

**Fixed by removal, not by declaration.** `edid_buf` never needed a physical
address: every byte arrives by CPU store (`gmbus_read_edid` reads the `GMBUS3`
register and writes out one byte at a time; the AUX path does the same). No
engine DMAs into it, and DMA is the only thing that requires a known physical
address. It is now a `static u8 edid_store[128]` behind an `edid_addr()`
accessor that still honours `intel_set_edid_buffer()` for the host harness.

`STATE-OF-THE-PROJECT.md` §5.3 proposed three edits — declare `HI_EDID`, include
`memmap.h`, assert the 128 bytes. All correct, all unnecessary: an object with
no address cannot collide with anything, and there is no map entry to keep in
step later.

**Closed since (checked 2026-09-04):** this used to read "Still open: nothing
scans C for a hardcoded literal that lands inside a declared `HI_*` region
without being its base". `kernel/tools/checks/check-himap.sh` is that scanner —
a literal *equal* to a region base is a deliberate cross-check, a literal
*strictly inside* one is the bug, comments stripped first because `memmap.h`
and `i2c_hid.c` quote the old addresses in prose.
`cd kernel && ./tools/checks/check-himap.sh` →
`OK: 8 in-range literals, every one of them a declared region base`.

---

## 3. The SMP wiring is complete. The ignition is missing.

`SESSION-CONTINUE.md` and `LOOK-AND-SPEED-PROMPT.md` both say to *"check whether
anything actually calls `fb_par_hook` at boot"*, implying the gap is in
`fb.c`/`smp.c`. **Something does call it, and that reading sends you to the
wrong file.**

```
$ grep -rn 'fb_par_hook' --include=*.c kernel/ | grep -v hosttest
fb.c:281    void fb_par_hook(...)                    <- the definition
smp.c:335   if (bands > 1) fb_par_hook(smp_band_dispatch, bands);   <- a real caller
```

`smp.c:335` (lines re-grepped 2026-09-04) is inside `smp_start()`, and the wiring below it is finished and
correct. The gap is one rung up — **nothing calls `smp_start()` at boot:**

```
$ grep -n 'smp_go' kernel/src/kernel.zl
5560:        smn = smp_go()          <- the ONLY occurrence

$ grep -n 'if cmd == 42' kernel/src/kernel.zl
5548:    if cmd == 42 {              # * - wake the other CPU cores
```

One call site, inside `run_command()`, under the `*` key of the old text shell
(since the desktop became the boot state, the Terminal's `smp`/`cores` words
reach the same `run_command(42)` — `term.c:368`; checked 2026-09-04).
Press `*` (or type `smp`) and three cores wake and start rendering bands. Boot
the desktop and they stay parked until you do.

So the conclusion in those documents is right — three of four cores are idle —
but anyone who follows the stated method finds `fb_par_hook` properly called,
concludes the path is live, and stops. **Verify the ignition, not the wiring.**

Two things to know before switching it on, neither of them blocking:

- `kernel.zl:5548`'s own text says *"they park immediately: nothing is
  lock-protected yet"*. The APs' only job would be `smp_band_dispatch`; that
  needs to be true, not assumed.
- **Almost no gate covers more than one core.** Corrected 2026-08-19: this
  first said "no gate", and `verify-efi.sh` boots `-smp 2`. The rest run one
  core, and not by an explicit `-smp 1` either - they pass no `-smp` at all and
  QEMU defaults to one. So SMP band rendering is exercised by exactly one of the
  seven boot gates, the EFI one (**2026-09-04:** two of eight —
  `verify-64.sh:94,115` also boots `-smp 2`), and a
  regression here is invisible to every other boot gate in the repo.

---

## 4. `$?` after a pipe reports the pipe, not the command

`gates/land-gate.sh`'s header documents this precisely:

> *"NEVER masks an exit code — the previous version piped every build through
> `tail`, so every step reported the exit status of `tail` (always 0) and a
> tree that did not link gated green."*

I reproduced it in my own reporting within the hour — `./verify-efi.sh | tail -25;
echo "exit=$?"` reads `tail`'s status — and briefly recorded a gate as "printed
FAIL and exited 0". The gate was fine; the measurement was not.

Worth stating plainly because the fixed script is *right there* and it did not
stop the next person: in bash use `${PIPESTATUS[0]}`, in **zsh** it is
`${pipestatus[1]}` — different name, different base. Better: do not pipe the
thing whose status you need.

**zsh does not word-split unquoted parameters either.** `clang $FLAGS -c x.c`
passes one giant argument and fails with `unknown argument`. Use `${=FLAGS}`, or
an array. Both traps cost a wrong answer here today.

---

## 5. A shared checkout makes gates fail for reasons that are not the code

`verify-efi.sh` reported `FAIL: the UEFI image did not build`. Re-run unchanged,
minutes later: **green, booted as a UEFI application, 120x37 framebuffer.**

Nothing about the tree changed between the two runs. Several sessions work in
this one checkout at a time, and all four build scripts write to **fixed,
shared paths** in `kernel/`:

```
out.c   _gen.c   _gen64.c   _genefi.c   _efi_*.o   zlOS-usb.img   BOOTX64.EFI
```

Two builds overlapping clobber each other's intermediates, and the loser reports
a build failure that describes nothing about the source.

It is worse than clobbered object files. `verify-sources.sh` **edits `SOURCES`
in place** for the length of its run — it drops a throwaway `_srcprobe.c` in and
builds all four targets looking for the marker. Its cleanup is correctly on an
`EXIT` trap, so it always removes it. But any other session that builds inside
that window compiles a source list with a probe file in it. Nothing warns; the
build simply differs from the one anybody reads about afterwards.

`CLAUDE.md` already says gates must never be timing-sensitive, for the same
reason at a different layer — a gate that fails on host load costs a bisect
every time it lies. This is that rule one level out: **a gate is only meaningful
if it owns its build directory.** Before believing a red gate in this checkout,
re-run it once. Before believing a green one, check that nothing else was
building.

The durable fix is a private worktree per session, which `WORKING-RULE.md`
already prescribes for other reasons — *one worktree per session, on its own
short-lived branch.* This is one more reason for it, and it is the reason that
bites even when nobody edits the same file.

---

## 6. Seven more, from the 2026-09-04 whole-tree sweep (two added 2026-09-06)

Same shapes, found by telling reviewers to refute the tree's own claims and
then reproducing each one before touching code
([`evidence/whole-tree-sweep-2026-09-04.md`](evidence/whole-tree-sweep-2026-09-04.md)).

| the guard | what it actually checked | how it was shown |
|---|---|---|
| `kernel/tests/host/tlstest` "real handshake against openssl" | that a handshake completes - which it does with ANY X25519 scalar. `tls_start`'s reset memset wiped the caller's private key and every session ran on clamp(0), a public constant, for the whole life of the check. | `tlsstatetest` case 1: `memcmp(c.priv, supplied, 32)` after `tls_start` |
| `input.c`'s sentence "the gate checks that by disassembly" | nothing - there was no such gate. The first run of the one written that day (`check-isr-sse.sh`) found `zllog_event_irq`, reached from `keyboard_isr`, using xmm0-3 in the gcc 64-bit build. | planted-defect direction A of that script |
| `verify-64.sh`'s EFER.NXE "structural verifier" | a `grep -F` over two `.S` files for the literal `1 << 11`. The third 64-bit entry, `efi.c`, is C and sets no EFER bit; every process PTE sets NX, which is a reserved bit while NXE is clear. | `grep -rn 'NXE\|0xC0000080' kernel/` |
| `dns.c`'s comment "the checksum is computed anyway - a resolver that accepts corrupted answers is worse" | the checksum on the way OUT. `dns_input` never verified one. | `dnstest` `t_bad_checksum` |
| six `oknum(value, ...)` calls in `uitest.c` | that the value was nonzero. `oknum` takes a condition; a two-track grid parser passed "parses to six tracks", and a knob that was not drawn at all (`offx == -1`) passed "sits at inset 1". | planted `G.n < 2` in a scratch `uikit.c`: still green |
| `xhci_port_reset`'s `wait_bit(reg, 0xFFFFFFFF, 1, 20)` "pure delay" | it was a wait on a value PORTSC can never hold, so it exited through the timeout branch and wrote an ERROR-severity xHCI timeout into the flight recorder on every successful reset. | fake-controller harness, `cfgtest` case D |
| TCP's "RFC 793 acceptability test" | the RST path only. The data path held ANY future sequence number in its one out-of-order slot - half of sequence space - so a blind off-path segment parked there for the life of the connection. | `tcptest` `t_blind_future_segment` |

And the same day, a reviewer told to break the TEST LAYER itself found these
- including one in a gate written that morning:

| the guard | what it actually checked | how it was shown |
|---|---|---|
| `check-isr-sse.sh`, first version, "follows those calls two levels deep" | the first level. Its function bodies came from `objdump -d`, and under `-mcmodel=large` a call is `movabs $0,%rax; call *%rax` - the callee's name exists only as a relocation, which `-d` does not print. A handler → clean function → xmm function chain stayed green. | planted `mid.c`/`leaf.c`; now direction A of the script |
| the three new host tests of that morning | nothing ran them. `run-host-tests.py` (the landing gate's runner) takes its list from `test-policy.json`, which nobody had told about them; the inventory check was red and no one had run it. | `gen-test-inventory.py --check` |
| `verify-iso.sh` / `verify-64.sh` "UEFI boot" | printed `  skip  no OVMF` (no colon, which no skip detector greps) and ended "gate green" after booting only the BIOS leg. | `printf '  skip  ...' \| grep -E '^\s*skip:'` → no match |
| `check-contained-gate.py`, `check-land-gate.py`, `check-build-contract.py` | that a snippet of text was in the file. Commenting the authority out kept every one green. | each `failures()` run on a commented-out copy |
| `tlstest` "a real TLS 1.3 handshake" | with no openssl it printed a skip and exited 0; the inventory classes it a gate, so the receipt said PASS. | `run-host-tests.py` policy: exit 0 is pass |
| `check-ram.sh`, `check-dma.sh`, `wguard.sh` | nothing - not one gate, workflow or runner invoked them, and `check-ram.sh` was red on four false positives the day it was first wired. | grep over `gates/`, `.github/`, `tools/` |
| `games4_rules.zl`, `games12_rules.zl` (244 checks) | nothing - documented with a cwd from which their imports do not resolve, invoked by no script. | `cd kernel && ../interp hosttest/games4_rules.zl` → module not found |
| `zllog_e2e_test.py` (5 cases, ASan, torn writes and identity refusal against the shipping `zllog.c`) and `dpll_test.c` (the only DPLL write harness) | nothing - `gen-test-inventory.py` enumerates executable `.sh` files, so a `.py` gate is invisible to it, and a `.c` with no `build.sh` line has no compiled target to classify. Both written 2026-08-30. **2026-09-06:** the Python passed the first time anyone ran it (5/5, 16 s); the C had not linked since `intel.c` grew `pci_bar_hi`/`console_init_fb`. Now `zllog-e2e.sh` (gate, refuses without `sgdisk` instead of unittest's green SkipTest) and a `dpll_test` build line (manual-hardware). | `python3 tests/host/zllog_e2e_test.py`; `gcc ... dpll_test.c intel.c` → undefined reference |
| The landing gate's `--write` then `--check` pairs (wrapper registry, dependency lock, license registry, toolchain manifest, build graph, source snapshot, test inventory, address-space registry) | that the second run reproduces the first. Not that the registry *in git* was current: **2026-09-06, measured on `main` as pushed, seven of the eight fail `--check`** before the gate's own `--write` (all but the test inventory), because each chains on the build identity that every build regenerates. Between landings the committed registries are stale by design and nothing says so. A pre-write `--check` would be red on every tree not landed from this machine, so this row records the limit rather than adding a check that can only fail. | `for g in gen-*; do python3 tools/generators/$g.py --check; done` on a clean checkout |

Two more that are not guards but the same belief-without-execution: the
kernel's `task_sleep` was a hint whenever nothing else was runnable
(`yield()` returned at once; `schedtest` measures the wait), and
`fs_create(n)` + `fs_write(n)` needed two contiguous runs of `n` blocks
because the first write into an empty file was treated as an overwrite of live
data (`fstest` "less than two runs free").

---

## What these have in common

Four of the original five are **not** wrong code. They are correct code with a false
belief attached, and the belief was never executed:

- the flag order that "must" work
- the sweep that "discovers every constant"
- the hook that "is not called at boot"
- the exit code that "is the command's"

The fifth is a gate that is correct in isolation and unreliable in the
environment it actually runs in.

The cheapest defence is the one `WORKING-RULE.md` already names, applied to
**checks** and not only to features: run the check against a defect you planted
on purpose, and watch it go red. `wguard.sh` direction A exists for exactly that,
and `verify-efi.sh` is validated the same way (green on a good build, red with
the right diagnosis when the GOP lookup is forced to fail). Every guard in this
tree should have one.
