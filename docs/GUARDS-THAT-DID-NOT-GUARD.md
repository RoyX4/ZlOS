# Guards that did not guard

Written 2026-08-19. Five checks in this tree that reported green, or read as
coverage, while checking nothing — each with the command that establishes it.

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
(`kernel/wguard.sh` automates this):

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

`kernel/wguard.sh` runs all three directions — guard bites, `-w` still
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
at all. Running the checker today prints a clean six-region map and says `OK`.

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

**Still open:** the checker's blindness itself. Nothing scans C for a hardcoded
literal that lands inside a declared `HI_*` region without being its base. That
is the rule that would have caught `edid_buf` (and the `i2c_hid` collision
before it) with no allowlist to rot — a literal *equal* to a region base is a
deliberate cross-check, a literal *strictly inside* one is the bug. It must
strip comments first: `memmap.h` and `i2c_hid.c` both quote the old addresses
in prose describing the fix.

---

## 3. The SMP wiring is complete. The ignition is missing.

`SESSION-CONTINUE.md` and `LOOK-AND-SPEED-PROMPT.md` both say to *"check whether
anything actually calls `fb_par_hook` at boot"*, implying the gap is in
`fb.c`/`smp.c`. **Something does call it, and that reading sends you to the
wrong file.**

```
$ grep -rn 'fb_par_hook' --include=*.c kernel/ | grep -v hosttest
fb.c:259    void fb_par_hook(...)                    <- the definition
smp.c:265   if (bands > 1) fb_par_hook(smp_band_dispatch, bands);   <- a real caller
```

`smp.c:265` is inside `smp_start()`, and the wiring below it is finished and
correct. The gap is one rung up — **nothing calls `smp_start()` at boot:**

```
$ grep -n 'smp_go' kernel/kernel.zl
1886:        smn = smp_go()          <- the ONLY occurrence

$ grep -n 'if cmd == 42' kernel/kernel.zl
1874:    if cmd == 42 {              # * - wake the other CPU cores
```

One call site, inside `run_command()`, under the `*` key of the old text shell.
Press `*` and three cores wake and start rendering bands. Boot the desktop and
they stay parked forever.

So the conclusion in those documents is right — three of four cores are idle —
but anyone who follows the stated method finds `fb_par_hook` properly called,
concludes the path is live, and stops. **Verify the ignition, not the wiring.**

Two things to know before switching it on, neither of them blocking:

- `kernel.zl:1874`'s own text says *"they park immediately: nothing is
  lock-protected yet"*. The APs' only job would be `smp_band_dispatch`; that
  needs to be true, not assumed.
- **No gate covers more than one core.** `verify.sh` boots `-smp 1`, so a
  regression here is invisible to every boot gate in the repo.

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

## What these have in common

Four of the five are **not** wrong code. They are correct code with a false
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
