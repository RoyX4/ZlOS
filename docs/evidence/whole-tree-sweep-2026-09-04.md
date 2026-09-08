# Whole-tree adversarial sweep - 2026-09-04

Branch `fable/whole-tree-sweep`, worktree `.claude/worktrees/fable-sweep`, base
`8fb1425`. Zac asked for the whole of zlOS to be gone over: bugs, hazards,
guards that do not guard. This is the receipt: what was measured, what changed,
what a gate now pins, and what stayed open.

## Method

Ten read-only reviewers, one per subsystem lane (arch/x86 + boot, core memory
+ scheduler + fs + NVMe, net + TLS, zl toolchain, input/xHCI, Intel display,
web engine, framebuffer + compositor, UI toolkit + zl builtin boundary,
kernel.zl). Each brief stated the tree's own claims (CLAUDE.md, HANDOFF.md,
the 2026-08-26 Codex audit, PROJECT-STATUS) as fact and ordered them refuted;
every finding had to carry a file:line, a concrete failure, and a VERIFIED or
SUSPECTED label with the command behind it; known items had to be checked
against `.ultra/TENSIONS.md` and `docs/evidence` first. Every VERIFIED finding
below was reproduced in this session before its file was touched - by reading
the lines, by running the reviewer's probe, or by an A/B against the pre-fix
source. The eighteen language-lane findings were handed to a builder agent
with the same rules; its results are in its own section.

Environment facts that affected the run: `/tmp` is a 7.7 GB tmpfs shared by
every agent and it filled to zero twice from one reviewer's runaway
`./compile` output (see the C-backend finding), which blinded every Bash call
until the file was removed; load average reached 21 on this 4-core box while
the toolchain and the host suite compiled at once. The QEMU gates were run
sequentially after everything else had finished.

## Verified defects fixed on this branch

Severity is the reviewer's; "pin" is the check that now goes red if it
returns.

### Network and TLS

| # | defect | file | pin |
|---|---|---|---|
| N1 CRITICAL | `tls_start` reset the whole connection with `tmemset` four lines above the comment saying the caller supplies the private key; only `verify/roots/nroots/nowZ` survived. Every HTTPS session used clamp(0), a public constant - `http.c`'s `rnd_bytes` quality gate guarded a value that was discarded. `tlstest` could not see it because a handshake succeeds with any scalar. | `net/tls.c` | `tlsstatetest` 1, `tlstest` "kept the caller's private key" |
| N2 MEDIUM | Encrypted records accepted before ServerHello under key 0 / nonce 0; a forged Finished under that key verifies (every input is public). Now refused in `WAIT_SH`; Finished refused outside `WAIT_FIN`. | `net/tls.c` | `tlsstatetest` 2, 3 |
| N3 MEDIUM | TCP held ANY future sequence number in its one out-of-order slot (2^31 values); a blind off-path segment parked there for the life of the connection and every legitimate out-of-order segment after it was dropped. RFC 793 window test added on the data path (the RST path already had it). | `net/tcp.c` | `tcptest` `t_blind_future_segment` |
| N4 MEDIUM | Handshake messages split across records were dropped (`TLS_HS_MAX ... we will reassemble` was false). A carry buffer now reassembles across records. | `net/tls.c`, `tls.h` | `tlsstatetest` 4 |
| N5 LOW | Data segments without ACK accepted in ESTABLISHED / FIN_WAIT. | `net/tcp.c` | `tcptest` `t_data_without_ack` |
| N6 LOW | DNS replies never UDP-checksummed on receive; the file said they were. | `net/dns.c` | `dnstest` `t_bad_checksum` |
| N7 LOW | Critical X.509 extensions ignored whether known or not; a critical `nameConstraints` on a sub-CA was silently dropped. Unknown-critical now fails the certificate; the routinely-critical set (KU, EKU, policies, AKI, SKI, CRL DP, AIA) is recognised. | `net/x509.c` | `x509test` real chain still validates (35/35) |
| N8 LOW | CertificateVerify scheme not bound to the leaf's curve (P-384 scheme over a P-256 key truncated the digest). | `net/tls.c` | - |
| N9 LOW | `status_code * 10` overflowed on a hostile status line (UB). | `net/http.c` | - |
| N10 LOW | Decrypted application data silently dropped when the buffer was full; now `TLS_E_OVERFLOW`. | `net/tls.c` | `tlsstatetest` 5 |
| N11 LOW | e1000 descriptor rings were non-volatile statics. | `drivers/network/e1000.c` | `e1000test` |
| N12 LOW | All-zero X25519 shared secret accepted. | `net/tls.c` | - |

### Architecture, boot, scheduler

| # | defect | file | pin |
|---|---|---|---|
| A1 HIGH | EFI (Win64 ABI) `switch_to` saved only the SysV six; `rsi`, `rdi`, `xmm6-15` are callee-saved there and clang keeps `zl_builtin`'s struct-return pointer in `rsi` across `call yield`. Measured on the old code: `rdi=0xcccc xmm6=0xb6` after one round trip. | `core/sched.c` | `schedtest_ms` (compiled `-mabi=ms`; red on `main`'s sched.c, green now) |
| A2 HIGH | Native UEFI entry never set EFER.NXE while every process PTE sets NX (a reserved bit while NXE is clear); the "structural verifier" was a text grep over two `.S` files that cannot see `efi.c`. OVMF sets NXE itself, which is why QEMU passed. `gdt_init` now sets it on every 64-bit route. | `boot/gdt64.c` | QEMU cannot show this one; physical boot remains the gate |
| A3 HIGH | No FPU/SSE state across the ring-3 boundary: user `ldmxcsr 0; exit` halted the kernel on its first inexact SSE op, a preempted process resumed with its sibling's xmm, and on Win64 user code clobbered the kernel's callee-saved xmm6-15. `fxsave`/`fxrstor` images for the kernel and per process. | `arch/x86/usermode.c` | not host-testable; `verify-efi.sh` / `verify-64.sh` exercise the entry/exit path |
| A4 MEDIUM | `zllog_event_irq`, reached from `keyboard_isr`'s drop branch, used xmm0-3 in the gcc 64-bit build (the file flag guards a file, not its callees). | `core/zllog.c` | `check-isr-sse.sh`, new, in the landing gate and CI |
| A5 MEDIUM | Syscall/timer stubs never `cld`; a user `std` ran the handler with DF set. | `arch/x86/usermode.c` | - |
| A6 MEDIUM | Raw-boot link script asserted an address, not a stack: an image ending at `0xBFF000` linked with 4 KiB of stack under a 256 KiB compositor. | `boot/link-raw.ld` | the link itself |
| A7 MEDIUM | Three copies of the zl-low extent disagreed; kernel.zl had 22 buffers above the end memmap.h and efi.c asserted against, invisible to the EFI fixed-memory witness. `ZL_LOW_END` = `HI_IMG`; the witness covers `HI_TAR`. | `memmap.h`, `boot/efi.c`, `kernel.zl` | `check-memmap-mirror.py`, `check-header-mirror.py` |
| A8 MEDIUM | Timer preemption armed only inside the selftest; a real `user.bin` of `jmp $` wedged the box. | `arch/x86/usermode.c` | - |
| A9 MEDIUM | EFI `ksetjmp`/`klongjmp` dropped xmm6-15 and MXCSR (callee-saved on Win64); `zl_run_program`'s longjmp path skipped every epilogue that would have restored them. | `arch/x86/ksetjmp.S`, `zl_freestanding.h` | - |
| A10 MEDIUM | `task_sleep` returned at once when nothing else was runnable (52 tick reads for `task_sleep(50)` is the measured result now; 2 before). | `core/sched.c` | `schedtest` |
| A11 LOW | APIC base masked to 32 bits; AP arrival `|=`/`++` unsynchronised; page-table verify compared A/D bits; `cpuid` asm cleared `rbx[63:32]` undeclared; `AP_STACK_SPAN` sized for 17 stacks while the trampoline indexes by APIC id up to 255. | `apic.c`, `smp.c`, `page_table_txn.c`, `support.c`, `memmap.h` | `pagetxntest`, `check-himap.sh` |
| A12 LOW | `pmm_report` re-ran `pmm_init_boot` unconditionally, orphaning every live page (zl builtin `pmm_up`). | `core/pmm.c` | - |
| A13 LOW | NVMe block count truncated to 32 bits in `block.c`'s LBA check. | `drivers/storage/block.c` | `blocktest` |

### Storage, display, input, web, UI

| # | defect | file | pin |
|---|---|---|---|
| S1 MEDIUM | `fs_create(n)` + `fs_write(n)` needed two contiguous `n`-block runs (the first write into an empty file was copy-on-write); a volume with room for one archive refused it. | `fs/fs.c` | `fstest` "less than two runs free" |
| D1 HIGH | A bring-up that failed before plan step 27 ran the full teardown and disabled the FIRMWARE's plane, transcoder, port and panel - the only console on the laptop. Now restores PSR/backlight and leaves the firmware display running. | `drivers/display/intel.c` | fake-BAR harness (reviewer's), not yet in tree |
| D2 HIGH | GGTT PTEs 0x100.. rewritten before arming with no check against `PLANE_SURFLIVE`; the window is now placed past the live scanout. | `intel.c` | - |
| D3 MEDIUM | Level-0 watermark computed for 24 bpp on a 32-bpp plane (43 blocks vs firmware's 41; the source comment's figure was a 32-bpp number). | `intel.c` | - |
| D4 MEDIUM | Link training wrote per-lane drive levels to the sink but drove all lanes from lane 0, never clamped swing+pre-emphasis <= 3, and forced enhanced framing on while the port was enabled (GEN9-S40b/c/g, open since 2026-08-19). | `intel.c` | - |
| D5 MEDIUM | GGTT PTE written low-with-PRESENT first; `virtio_gpu_init` spun forever on device reset. | `intel.c`, `virtio_gpu.c` | - |
| I1 MEDIUM | Configuration descriptors clamped to 256 bytes with no diagnostic: a HID interface past byte 256 of a composite device was "not a keyboard". 4 KiB buffer now. | `drivers/input/xhci.c` | reviewer's `cfgtest` case B (scratch) |
| I2 MEDIUM | Device-supplied `wMaxPacketSize` (up to 1024) was the DMA permit length for 256/64-byte buffers. | `xhci.c` | - |
| I3 MEDIUM | `xhci_port_reset`'s "pure delay" was `wait_bit` on an impossible mask and logged an ERROR xHCI timeout on every successful reset. | `xhci.c` | - |
| W1 HIGH | JS parser depth guard covered `(` and `{` only; ~1,500 `[` or a run of `!` in a `<script>` overflowed the 256 KiB stack at parse time. Old parser: SIGSEGV under `ulimit -s 256`; new: 58/58. | `web/js.c` | `jstest` six `deep()` cases |
| U1 MEDIUM | Six `uitest` checks passed the VALUE to `oknum` instead of a condition (any nonzero answer was green; a knob not drawn at all passed "sits at inset 1"). | `kernel/tests/host/uitest.c` | the checks themselves |
| U2 MEDIUM | Snake body length unbounded; the 513th segment's shift wrote `SNAKE_X[512]` = `SNAKE_Y[0]`. | `kernel.zl` | - |
| U3 MEDIUM | `fill_mem`/`copy_mem` took any count; -1 was 2^64 bytes. Refused past 4 MiB. | `freestanding/runtime_kernel.c` | - |
| U4 LOW | `SNAP_WINDOWS` was a second copy of `WM_MAX`. | `graphics/ui/snap.c` | compile |
| K1 HIGH | Typing `snake` seeded the board with the client size in PIXELS as cells (head at cell ~209 of a ~52-cell client; food visible ~1.5 % of the time), and `sn_draw` never re-seeded because `sn_ready` was already 1. The rail path was correct, so the rail probe passed. | `kernel.zl` `open_app` | - |
| K2 HIGH | Every per-app window id was a slot number tested with `wm_open_p`, and `wm_open` reuses the lowest free slot; closebox/Ctrl+W never told zl. Open Snake, close it, open Paint, type `snake`: Paint focused, Snake never reopened. Sixteen sites now go through `win_is(w, APP_X)`, which also checks `wm_app`. | `kernel.zl` | `check-zlcalls.py` (call sites resolve) |
| K3 HIGH | `fib <n>` was naive recursion on a 256 KiB stack with no depth guard and a typed argument saturating at 2^31-1. Refused past 40. | `kernel.zl` | - |
| G1 HIGH | `fb_gradient` (and `fb_rrect_grad_top` through it) computed `x + w` and `clip_x0 - x` in `int`; for x or w near 2^31 the back-buffer row pointer went gigabytes outside the framebuffer, and `fb_clip` let a scissor START past the screen. The zl bridge casts every argument with `(int)double`, which is INT_MIN for a NaN, an inf or any out-of-range value on x86 - so a zl layout that divided by zero was that call. Every rectangle primitive now saturates its coordinates to +-2^28 on entry. | `graphics/framebuffer/fb.c` | guard-page harness (reviewer's), to become a host test |
| G2 HIGH | `slot_capture` (`fb_stash`, `fb_blur_cache`) computed `w * h` in 32 bits; 2^31 x 2^31 wrapped to 4 pixels, the arena take succeeded and the copy ran off HI_BLUR into HI_NVME. 64-bit product, capped at the arena. | `fb.c` | - |
| G3 MEDIUM | `fb_blur_paint`, `fb_stash_blend`, `fb_wall_paint` tested `back_on` without `!surface_on`; under a client surface of a window hanging off the screen edge (a normal state) they indexed `back` and `wall_buf` outside them (ASan on the real wm.c). | `fb.c` | - |
| G4 MEDIUM | `wm_close` from inside a draw hook freed the client surface the compositor was drawing into (ASan use-after-free). Deferred to the next frame. | `graphics/windowing/wm.c` | - |
| G5 MEDIUM | `wm_focus` installed a closed or out-of-range id as `focus_win`; every key then went to a dead slot (850 stale-focus states in a 20,000-op storm). | `wm.c` | - |
| G6 MEDIUM | `fb_line` trapped (#DE) for an endpoint at INT_MIN and never terminated for one at INT_MAX; `y0 << 16` overflowed. Endpoints saturate at +-64K, accumulator is 64-bit. | `fb.c` | - |
| G7 LOW | fb3d's scanline loop skipped clipped rows INSIDE the loop, so a vertex at INT_MAX never ended it; the default clip was a million pixels; `edge()` overflowed. Vertices saturate at +-8192, default clip likewise, edge in 64-bit. | `fb3d.c` | `tritest` |
| G8 LOW | `fb_shade` / `fb_mix` clipped to the screen instead of the scissor, leaving a retained off-screen client surface unpainted. | `fb.c` | - |
| G9 LOW | `fb_rrect` / `fb_rrect_blend` tested `2 * r > w`, which overflowed for a huge r and skipped the clamp before an r^2 corner loop. | `fb.c` | - |
| K4 MEDIUM | The text shell's top-level `r = run_command(...)` / `ch = key_get()` made `r` and `ch` globals, so 27 and 11 functions' "local" writes went through to them; measured on the host: a row loop over `r` calling `ac_rand()` ran 51 times instead of 6 once a global `r` existed. Renamed `sh_r`/`sh_ch`. | `kernel.zl` | - |

### Second wave: unread regions, app modules, the test layer

| # | defect | file | pin |
|---|---|---|---|
| R1 HIGH | `zl_calln`'s argument table has eight slots and the loop filling it stops there, but the print/put arms iterated to `n`: a nine-argument `print` read stack garbage and, if it said V_STR, dereferenced it in ring 0 (ASan-confirmed). Refused past eight. | `freestanding/runtime_kernel.c` | - |
| R2 MEDIUM | `ui_scroll_begin` with a negative height made `content > h` true for an empty viewport; `ui_scroll_end` then divided by zero (#DE from the zl `ui_scroll` builtin). | `graphics/ui/ui.c` | - |
| R3 MEDIUM | A table cell's natural width was never clamped; `colw * room` overflowed on a 400 KB cell and the column collapsed to one glyph per line, filling the run arena. `lay_flex`'s shrink product overflowed the same way. | `web/layout.c` | `browsertest_san`/UBSan |
| R4 LOW | `zl_binop` guarded `/ 0` but not `LLONG_MIN / -1` (the same #DE); `ui_num`/`zl_itoa`/`zl_ltoa` negated INT_MIN (UB); `ui_segmented` measured its label regular and drew it bold; `ui_tabstrip` drew a label that did not fit under the close glyph. | `runtime_kernel.c`, `ui.c`, `uikit.c` | - |
| P1 HIGH | The Clocks app's "RTC SECONDS" card showed `rtc_up()`, which is `rtc_read()`'s success flag: 00 h 00 m 01 s forever. Composed from the hour/minute/second accessors. | `apps/apps_system.zl` | - |
| P2 MEDIUM | Calculator: a second `=` zeroed the result; HEX/OCTAL/BINARY rows drew garbage glyphs for a negative value (`-5 % 16` is -5 in kernel arithmetic). | `apps_system.zl` | - |
| P3 MEDIUM | Sokoban recorded moves only while the undo record had room, so after 512 moves an undo walked back the wrong move (crate left in place, shown on the host harness). Oldest entry now dropped. | `apps/apps_games4.zl` | `games4_rules` |
| P4 MEDIUM | The keyboard tester logged a wheel-down notch as type "-" code 65535 (signed notch packed into an unsigned field). | `apps/apps_utils.zl` | - |
| P5 MEDIUM | The two zl rule harnesses (244 checks over the games) were documented with a cwd that cannot resolve their imports and were wired into nothing. `run-all.sh` runs them from `kernel/apps`. | `kernel/tests/host/run-all.sh` | themselves |
| P6 LOW | Connect Four dropped a piece on a click anywhere in the column, caption and status bar included; `cat_draw` and `s3du_mount` wrote through to the text shell's `crow`/`ccol`/`tk` globals; Settings opened 660 wide from one route and 720 from the other. | `apps_games1.zl`, `apps_registry.zl`, `apps_sys3.zl`, `kernel.zl` | - |
| T1 HIGH | The three tests this sweep added were not in `test-policy.json`, so `run-host-tests.py` (the landing gate's runner) never executed them and the inventory check was red. | `kernel/tests/host/test-policy.json` | `gen-test-inventory.py --check` |
| T2 HIGH | `check-isr-sse.sh` was blind past the first call level: under `-mcmodel=large` a call is `movabs; call *%rax` and the callee's name is only a relocation, which a plain `objdump -d` body never shows. Rewritten to read relocations, root at every `__attribute__((interrupt))` function plus the ring-0 timer dispatcher, walk three levels, skip data symbols; a two-level cross-TU plant now runs first. | `kernel/tools/checks/check-isr-sse.sh` | its own plants |
| T3 MEDIUM | `verify-iso.sh` and `verify-64.sh` printed `skip` (no colon, which no skip detector matches) for the UEFI leg and ended "gate green" after booting half their routes. Now `skip:` and red. | `verify-iso.sh`, `verify-64.sh` | - |
| T4 MEDIUM | Three contract checkers were substring tests: the authority commented out still passed. Comments are stripped first. `check-ram.sh` was wired to nothing and red on four false positives (a `--version` probe on the next physical line, a route tuple, a dict key); fixed and in the landing gate with `check-dma.sh` and `wguard.sh`, which nothing ran either. | `gates/check-contained-gate.py`, `check-land-gate.py`, `check-build-contract.py`, `check-ram.sh`, `land-gate.sh` | their selftests |
| T5 MEDIUM | `tlstest` printed a skip and returned 0 when openssl was missing; the inventory classes it a gate, so a box with no openssl recorded PASS. Red now. | `kernel/tests/host/tlstest.c` | - |
| T6 LOW | `run-all.sh` never ran `jmptest32` (announced on a shared line); the reverse-SOURCES sweep matched bare basenames. | `kernel/tests/host/build.sh`, `land-gate.sh` | - |

Still open from that wave (**corrected 2026-09-06:** `zllog_e2e_test.py` is now `zllog-e2e.sh` in the gate list and passed 5/5 on first run; `dpll_test` has a `build.sh` line after two missing stubs were added; the `--write`/`--check` limit was measured - seven of eight registries fail `--check` on `main` as pushed - and recorded in `GUARDS-THAT-DID-NOT-GUARD.md` §6 rather than changed, because every one chains on the per-build identity): `zllogtest.c`, `zllog_e2e_test.py` and `dpll_test.c` exist and nothing builds or runs them (`gen-test-inventory.py` enumerates `.sh` only); the `--write` then `--check` pairs in the landing gate prove determinism, not that the committed registry was current; hardware skips roll up to a green label; CI boots four of nine routes; the docs-versus-tree corrections are in the same commit (see `GUARDS-THAT-DID-NOT-GUARD.md` §6 and the per-file dated corrections).

### Language toolchain (builder agent)

| # | defect | file | pin |
|---|---|---|---|
| L0 CRITICAL | The C backend emitted an `elif` chain as nested `else { if ... }` blocks one indent deeper per branch, so the output was quadratic in the chain length: 3,000 branches were 72.6 MB of C, and a reviewer's 30,000-branch probe wrote 7.5 GB and filled the shared tmpfs twice in one day (every Bash call in the session then failed with ENOSPC). Emitted flat as `else if` at the same indent: 566 KB for the same chain, byte-identical program output. | `src/backends/c/compile.c` | `run_tests.sh` "elif chain emits linear C" (2,000 branches under `timeout 20`, output under 2 MB) |

The builder agent handled the seventeen findings below (its report is
summarised here; each has its pin in `tests/*.zl` or `run_tests.sh`):
a lexer/parser `exit(1)` that halted the kernel on a script syntax error is
now a trap that returns NULL to `exec.c` (`zl_lex_guarded`/`zl_parse_guarded`;
pinned by a run_tests harness that links the unmodified exec.c); number
literals go through one canonical parser in every backend (octal `010` and
`09`, and integers past 2^63 no longer differ per engine - the integer
backends refuse instead of saturating); a call with the wrong argument count
is refused in every engine instead of reading the same-named global; every
builtin has a minimum-arity check (ASan-confirmed reads past `args[]`);
nativegen prints negatives with a sign and names its fractional refusal;
`break`/`continue` outside a loop is a parse error in every engine and a
function body cannot leak them into its caller's loop; an imported module's
for-range temporaries no longer collide with the caller's (the hidden-name
counter is monotonic, and `parse_import_into` stopped restoring it); parser
recursion is bounded at 64 in the parser itself, so `compile`/`compilel`/
`nativegen` refuse what used to segfault them; the LLVM backend's NaN index,
`-true` and `%` overflow now match the interpreter (its local-scoping
divergence is documented, not changed); `compilel`'s silent 512-entry caps are
loud and a failed run removes `out.ll`; `zl_set`/`zl_index` type- and
range-check the index; three `is_text`/`is_sym` swaps let `"}"` and `"]"`
literals parse; `zlfmt` refuses a file with a NUL instead of truncating it;
`read()` of a directory and `kill()` of pid <= 1 are refused; the three
`-Wstringop-truncation` sites are bounded copies with explicit terminators.
The builder could not reproduce the runaway-output report by hand (it tried
nested if/else forms and ~700 fuzzed programs); the chain shape above was
found afterwards by measuring output size against branch count.

The original list, for the record: lexer/parser `exit(1)` halting the kernel on a script syntax
error (CRITICAL), octal literals in the C backend, big-literal saturation in
three backends, missing arguments reading the same-named global, builtins
indexing past `args[]` (ASan-confirmed), nativegen printing negatives as
unsigned, `break` inside a function exiting the caller's loop, module
for-range temporaries colliding with the caller's, unbounded parser recursion
in three binaries, five LLVM-backend divergences, compilel's silent 512 cap,
`zl_set` never type-checking its index, three `is_text`/`is_sym` swaps,
zlfmt truncating at NUL, `read()` of a directory, `kill(-1)`, and the C
backend emitting whitespace forever on a nested `if` after `} else {`.

## Claims that survived refutation

Worth keeping because the next session will be told to re-check them: the
crypto primitives (SHA-256, HMAC, AES-GCM, X25519, HKDF, ECDSA, RSA PKCS#1
v1.5, DER walker) pass every published vector and their bounds checks hold;
GCM checks the tag before decrypting; the heap survived a 400,000-op
randomised fuzz with per-block fill checks; PMM/anon/process accounting cannot
go negative; zlfs's dual-generation directory and every on-disk range check;
the syscall admission table (all 24 entries) validates every pointer and
length and copies before use; page-table transactions roll back exactly; the
IST1/TSS arithmetic on the BSP; xHCI ring/cycle handling, firmware handoff,
scratchpads, pointer drain; the PNG decoder (146/146 under ASan); html/css/
layout through 16.4 M fuzz checks and 50k nested divs; the settings parser
against a hostile file; toast queue, ease, widget arrays; every game board
writer except Snake; the T12 timer, AUX-into-unpowered-panel refusal, PPS
from VBT, the DPLL/M/N arithmetic reproducing the measured 241,690 kHz.

## Gates run

Final sequential chain on the finished branch (logs under
`~/.cache/fable-sweep/` on the dev box; every line below is pasted from them):

| gate | result |
|---|---|
| `kernel/build.sh`, `build64.sh`, `buildefi.sh`, `kernel/tools/images/mkdisk.sh` | all exit 0 (the app manifest regenerated first) |
| `kernel/tools/run/run-host-tests.py --run --selftest` (the landing gate's runner) | `PASS: commands_executed 74, failed 0, not-run 12, passed 69, skipped-hardware 3, targets 84` |
| `kernel/tests/host/run-all.sh` | `64 passed, 0 failed, 0 NOT BUILT, 13 skipped` (was 58/0/0/13 at the start of the day: three new C tests, the two zl rule harnesses, jmptest32) |
| `./run_tests.sh` (language, self-host, freestanding, and its own BIOS / raw-bootloader / native-EFI QEMU gates) | exit 0, 78 ok/match lines, 0 FAIL |
| `./verify_selfhost.sh`, `./verify_fmt.sh` | PASS (fixpoint gen1 == gen2; 175 files, 0 fail, NUL case included) |
| every static checker under `kernel/tools/checks/` and `gates/` (24 of them, listed in the chain log), each with its selftest where it has one | all exit 0, including the three new ones in the landing gate (`check-isr-sse.sh`, `check-dma.sh`, `wguard.sh`, `check-ram.sh`: `OK: 16 files, 23 launches, every one at or above 1024 MiB`) |
| `check-reproducible-build.py --check --selftest` | `PASS: two independent recipe runs match` |
| `verify-efi.sh` (native UEFI, the ThinkPad's path) | `EFI gate green` - ring 3 entered, made syscalls and exited; the earlier red run was a host receipt from before the rebuild |
| `verify-64.sh` (64-bit multiboot, BIOS + UEFI through GRUB) | `64-bit multiboot gate green`, `fib(20)=6765` on both legs |
| `verify-iso.sh` (BIOS + UEFI through GRUB, 32-bit) | `ISO gate green` |
| `verify-disk.sh`, `verify-clock.sh` | exit 0 in the chain before the toolchain rebuild (disk skips itself above load 4; it ran at 3.0) |
| `verify-net.sh` | not run - this box is on a network that drops outbound UDP and SNI-filters TCP |

No physical ThinkPad boot was performed.

## Still open

- Physical ThinkPad boot. Nothing here proves A2, A3, D1-D5 or I1-I3 on the
  laptop; the fake-BAR harness the display reviewer built is in a scratch
  directory and should become a host test under `kernel/tests/host/` (a fake-BAR modeset harness).
- Intel: PTE save/restore in teardown (only relocation landed); PSR restore
  order in teardown (suspected); `intel_dpll_program_*` and the AUX stack
  are still ungated - the CLAUDE.md table now says so.
- xHCI: no disconnect handling (modifiers stick on unplug) - **closed 2026-09-06:** `xhci_poll` now handles Port Status Change Events: a port whose CCS cleared detaches the keyboard (every held key released into the event queue, then `KEV_MOD(0)`, `kbd_mods` = 0, `kbd_ready` = 0) or the pointer (button released, `ptr_ready` = 0) and acknowledges the RW1C bits through `portsc_keep`; `xhcitest` "unplug releases held keys and modifiers" is red on the old driver (8 of 13 assertions) and green now. Re-plug is not automatic: `xhci_bringup` re-enumerates a not-ready device only when `usb_up()` runs again, and `kernel.zl` calls that at boot (`usb_boot`) and from the Terminal `usb` and `/` commands, never periodically - so a plugged-back keyboard needs the `usb` command typed (from the PS/2 or a second keyboard). Still open: the 64-bit
  multiboot build takes a BAR above 4 GiB it has not mapped; ECM TX buffer
  reuse after a timeout.
- 32-bit lane has no double-fault stack; 64-bit APs have no TSS. **APs closed 2026-09-06:** `gdt64.c` builds one TSS per slot (8, held equal to smp.c's `SMP_SLOTS` by `gdt64test`, which reads the number out of `smp.c`), each with its own 16 KiB IST1 stack, and `smp_ap_main` loads OUR GDT and its slot's TSS before going live - an AP whose `str` does not read back its selector parks instead of joining the band count, so `verify-64`/`verify-efi` (`-smp 2`) witness the ltr. The #DF stack classifier in `idt.c` now resolves the faulting core's stack through `str`. APs keep RSP0 = 0: only the BSP runs ring 3. **32-bit closed 2026-09-08:** vector 8 is a task gate (`idt[8]` type 0x85 → GDT 0x30) into a second TSS with its own 16 KiB stack in `gdt.c`; `fault_df_task32` reads the interrupted context back out of the main TSS and records it. The recorder had a "32-bit has no IST" shortcut that let a 32-bit #DF commit with no emergency stack - now both lanes must present one (`crashtest`, 4 new pins). `crashdftest` on that lane loads a one-byte data segment (GDT 0x38) into SS so its first push is #SS and the delivery faults again - paging is off there, so "RSP on an unmapped page" cannot be the trigger. `verify-crash.py --route bios32 --fault double-fault` is the QEMU witness and is in the landing gate.
- `settingstest` never runs the zlfs branch the kernel uses. **Closed 2026-09-06:** `settingstest_zlfs` links `fs.c`, mounts a zlfs on the same fake disk and runs 30 cases on that branch (file wins over a valid sector, corruption named as the file not "LBA 64" - a message fix in `settings.c` - short file, write and final-sync failures, never-writes-on-load). 0 failures.
- compilel/nativegen locals do not write through to globals (a design
  decision, now documented, not a fix).
- `check-memmap.sh` still only "notes" zl constants above `ZL_LOW_END`. **Closed 2026-09-06:** a kernel.zl fixed address at or above `ZL_LOW_END` (parsed from `memmap.h`) now fails unless the line declares `# memmap.h NAME`; the selftest plants `CODEX_HIGH = 0x03400000` and requires the red. Today's tree has one such constant, `RULER_DMA`, and it is declared.
