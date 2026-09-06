# Process service, whole-tree sweep and cleanup integration

This is the current integration candidate for the complete 906-feature program.
It combines existing work in an isolated checkout; it does not mark the program
or any physical platform complete. Main and the original worktrees are unchanged.

## Inputs and custody

The checkout is `/home/roy/Documents/repos/zl-linux-integration-2026-09-06`,
branch `codex/integrate-sweep-process`, based on main `8fb1425`.

| Input | Exact committed source | Integration |
|---|---|---|
| Scheduler and bounded sleep, PRs #12/#13 | `ddb79d14731d48b9e169a4d6afa6b1baf53769d0` | Starting tree; includes the repaired scheduler base. |
| Claude whole-tree sweep | `921297911b340790cd8fd11635f28760cd89f67e` | Ancestry preserved by merge `6052418`. |
| Atomic TODO and hook protection, PR #14 | `5cd58d7888187d1f88a371c3a097c8d24b40cb20` | Ancestry preserved by merge `c554481`. |

The sleep and sweep branches change 66 and 120 paths respectively relative to
main, with 13 paths in common. Review covered those integration points and
selected changed storage, USB and TLS paths. This is not a claim of independent
exhaustive verification of all 120 sweep paths. Its original dated receipt is
retained as the author's evidence, with its unrun/physical limits intact.

Five first-merge conflicts were resolved: contained-gate checker, host-test
README/policy and the two generated app-manifest files. Hosted runner controls
and comment rejection both survive; all test targets survive. App manifests,
host inventories and the directory README were regenerated with their owning
tools. The cleanup merge reconciles the tools README and TODO generator.

The TODO generator retains atomic rename, checked generation/publication,
permission preservation and signal cleanup from #14, plus the sweep's closing
marker check. A missing handwritten-block closing marker reproduces failure
against #14 alone; the combined implementation refuses publication and preserves
the exact old backlog. All 10 recovery tests and seven installed-hook fixture
tests pass. Shared installed hooks have not been replaced.

## FP state boundary found during review

The sweep saves Ring-3 x87/SSE state before syscall/timer dispatch, but originally
called C handlers without restoring kernel FP controls. A program could leave
MXCSR zero (unmasked exceptions) and a different x87 rounding mode active in the
handler. Saving the user image alone does not change the active CPU state.

`python3 tools/test_user_fpu_boundary.py` executes the shipping assembly with a
synthetic CPL-3 interrupt-return frame and observation-only dispatchers. Before
the fix, both syscall and timer tests report kernel entry failure:
`MXCSR=0 FCW=0x77f DF=0`; user return still passes. The fix restores the saved
kernel image immediately after saving user state. Both tests then pass, checking
kernel FP controls/XMM state, clear handler DF, and exact user control/XMM/DF
restoration. This is native host execution of the real stubs, not a privileged
entry, CR3, preemption or physical-hardware test. QEMU gates remain required.

The interrupt checker also skipped explicit C roots and silently stopped after
four worklist levels. Planted root and six-level SSE calls both escaped. It now
checks roots and drains the complete discovered call worklist. The stronger
check then caught GCC vectorizing `user64_timer_dispatch`. Its C body and shared
FX-copy helper now require general registers, matching the unsaved Ring-0 IRQ
contract. All four planted cases are rejected and the current direct-call
closure passes. Indirect callbacks and physical execution are separate proof.

## Verification and limits

Local logs and exact inputs are retained in
`/home/roy/Documents/artifacts/zl-linux/integration-2026-09-06/`:

- Two FP boundary tests, 10 TODO tests, seven hook tests pass.
- Six allocator-join, five sleep-join and six toolchain-join tests pass.
- Contained-gate mutation tests retain all 64 controls, including commented-out
  local/hosted controls. Landing-gate mutation checks retain 118 required seams.
- Program validation retains 906 feature atoms, 21 phases, 174 research
  contracts, 63 named implementations and 24 games.
- Directory documentation checks all 118 directories and eight surfaces each.
- Both warning-strict BIOS64 and UEFI64 compilation of changed usermode code
  pass, separately from complete artifact builds and boot execution.

Fresh hosted ordinary and complete gates must verify this combined source.
The earlier scheduler/sleep green runs do not prove this integration. Their
latest downloaded sleep ledger remains 8 proved, 55 partial, 843 planned;
no new maturity count is claimed until a fresh full-run evidence join succeeds.
Those earlier performance registries retain an over-budget host build category.

No physical ThinkPad, panel-power, firmware or installed-hook changes were made.
The source sweep's hardware limitations remain open. Its X.509 parser recognises
several critical-extension OIDs without enforcing their full semantics; the
comment calling them safe to ignore is not security proof. Key usage, extended
key usage and the other certificate-policy limits require a separate tested
contract before claiming complete certificate validation.

After combined-source verification and integration review, the next product
slice is the existing [userspace spawn/wait contract](../design/userspace-process-management-next.md).
Image preparation must first preserve the active caller and construct a child
from a supervisor-only mapping template. No spawn/wait syscall is implemented
by this integration.
