# Whole-Tree Codex Audit - 2026-08-26

## Claim

An independent Codex pass audited the reviewable zl/zlOS source from commit
`528a21f` in the isolated worktree `/home/roy/Documents/repos/zl-linux-audit-20260826`.
The audit covered the lexer, parser, interpreter, C/LLVM/native backends,
self-host compiler, standard library, kernel core, boot paths, storage, display,
interrupt, scheduler and terminal code. Its raw report is retained locally at
`/home/roy/zl-audit-report-20260826.md`; this receipt is the repository-safe
triage and disposition.

This is a source and host/QEMU audit. It is not proof that every planned feature
is implemented, and it is not physical ThinkPad proof.

## Accepted And Repaired

| Area | Finding | Disposition |
|---|---|---|
| Lexer | Long identifiers and strings silently truncated | Reject oversize input with a location-bearing error; regression added. |
| C backend | Binary, index and indexed-assignment evaluation order was left to C | Emit explicit left-to-right temporary sequencing; differential regression added. |
| Runtime | Non-list iteration could become a silent no-op; number formatting could cast out of range | Enforce list input and guard integer conversion. |
| Native backend | Fixed arrays were unchecked and 16+ argument cleanup used an 8-bit immediate | Add capacity failures and 32-bit stack cleanup; 16-argument stress probe added. |
| Self-host compiler | Parse failures could still emit output and `and`/`or` were eager | Refuse output after parse errors, validate `for`, advance error recovery and emit short-circuit C; fixpoint probe extended. |
| Standard library | JSON list detection, repeated-space word counts, zero-sized chunks and empty statistics were unsafe | Correct type checks and add precondition guards/selftests. |
| EFI boot | Fixed-memory witness sampled only 32 descriptors and did not enforce safety | Scan the complete memory map, cover the fixed arenas and refuse an unsafe map before `ExitBootServices`. |
| GRUB64 boot | BIOS entered long mode without enabling execute-disable while process stacks were mapped NX | Enable EFER.NXE with long mode in both 64-bit entry paths and make the structural verifier enforce it. |
| Display | VirtIO/BGA/framebuffer sizes could overflow; Intel aperture and VBT arithmetic truncated or wrapped | Use checked wide arithmetic, retain pointer width and tear down failed modesets. |
| NVMe | Namespace block size ignored the active LBA format and partial setup could report success | Decode active FLBAS/LBAF, validate size and reset partial state. |
| APIC/scheduler/terminal | Diagnostic addresses narrowed; deadline and numeric parsing could overflow | Preserve 64-bit addresses and use bounded modular arithmetic. |
| Editor route | The Editor app could not open on diskless boot because it required a zlfs directory entry | Open an honest unsaved buffer when storage is absent and refuse save with a clear storage error. |
| Evidence tooling | Moved source/test paths and absolute receipt keys made probes stale and non-reproducible | Migrate probes to the current layout, add selftests and emit guarded repository-relative evidence paths. |

## Focused Evidence

- Root language build passed after the repairs.
- Interpreter and generated-C operand-order probes agree.
- Interpreter and generated C both reject iteration over a non-list.
- A 40,000-call native stress program with 16 arguments completed successfully.
- JSON parser/pretty-printer selftests passed.
- The self-host compiler reached a stable fixpoint and passed its short-circuit probe.
- Engine parity passed with two known output-divergence pins and two explicit
  native subset-rejection pins; no unpinned divergence was accepted.
- The terminal host harness passed, including oversized numeric input.
- All six retained boot routes passed: raw BIOS, GRUB32 BIOS/UEFI, GRUB64
  BIOS/UEFI and native UEFI64.
- The complete app route sweep passed, including the diskless Editor path; all
  47 lifecycle cycles and all 62 fresh QEMU open-ready-close app checks passed.
- Two independent builds agreed byte-for-byte across all nine release artifacts.
- The generated status join accounts for all 906 feature atoms without treating
  historical receipts as current proof.

The final repository, build, image and QEMU gate results belong in the current
build receipts and CI run, not in this source-audit snapshot.

## Remaining Boundaries

- No physical ThinkPad boot was performed in this audit. Intel panel takeover,
  NVMe active-LBA behavior and real input/storage paths remain hardware gates.
- The self-host compiler remains an intentionally bounded subset; this pass did
  not claim full escape or backend semantic parity.
- The standard-library findings were targeted, not a line-by-line proof of every
  library routine.
- Existing documented xHCI transfer-size limitations were outside this repair pass.
- Current QEMU and host receipts do not replace physical evidence for Intel
  display takeover, NVMe behavior, USB input/power, networking or suspend.
- Native capacity guards were source-reviewed and exercised by focused probes,
  not exhaustively load-tested at every configured maximum.

## Rule

Do not turn this receipt into a whole-product completion claim. Promote a feature
only through the repository evidence ladder: source exists, build reachable,
test covered, built, booted, and hardware proven where hardware is required.
