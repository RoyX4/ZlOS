# First roadmap implementation: userspace spawn and wait

Status: local implementation with host proof and four native-UEFI parent/child
QEMU passes repeated at `52dc8b9c…` after the shared allocator rollback repair.
The full host run, native boot gate and existing fault/exit/sleep probes pass
for that image. The BIOS32 Run check and all 47 app lifecycle cycles also pass.
The full boot matrix, later-main integration and hosted closure remain pending.
No feature maturity promotion.

The user accepted the full roadmap and authorized implementation on 2026-09-08.
This pass starts `M-03.03` with the audited parent/child process contract.
The original checkpoint remains in `/home/roy/Documents/repos/zl-linux-integration-2026-09-06`,
branch `codex/integrate-sweep-process`. Current work is in the persistent
`/home/roy/Documents/repos/zl-linux-spawn-wait-2026-09-08` checkout on
`codex/spawn-wait-reconcile-2026-09-08`, based on `5a33421`, with a prepared,
uncommitted merge of main `02180af`. All local changes remain uncommitted.
The user explicitly authorized commit/push of this batch on 2026-09-08.
The approval-pending statements below describe earlier checkpoints. Publication
will fast-forward the existing draft PR #15 after reconciliation and checks.
The separate `zl` language checkout and other worktrees were left untouched.

The live check at entry found draft [PR #15](https://github.com/RoyX4/ZlOS/pull/15)
open and mergeable at that same head, with base `9212979`, and nine retained
worktrees. No other active zlOS Codex task was found in the task listing.
That is a dated coordination snapshot, not a claim about every editor/process.

The earlier [integration receipt](integration-hosted-closure-2026-09-06.md)
remains evidence for the prior implementation. Its 126 passing hosted steps
do not verify this changed implementation. The old 8/55/843 feature ledger is
not promoted or rebound to untested source by this pass.

## What is implemented

- Inactive child construction from a supervisor-only kernel template, with
  disjoint frame ownership and no selection of the child during parent spawn.
- Generated SPAWN/WAIT syscall numbers, exact eight-byte child handles,
  parent-derived authority and a fixed 32-byte typed termination result.
- Empty-slot admission, allocation rollback, stale-generation refusal,
  nonblocking wait, signed exit/fault distinction and retry after cleanup refusal.
- Privileged adoption of live or terminal orphans into kernel custody before
  a parent identity is released.
- Disk-loaded parent/child fixtures for signed exit and attempted access to a
  parent-private anonymous page. Both scenarios run two spawn/wait cycles.
- New host targets and mandatory native-UEFI QEMU steps in the landing gate,
  including checks that deleting those entries, substituting fixture-only
  execution or running the wrong child mode is rejected.

The [implemented ABI and ownership decision](../design/userspace-spawn-wait-abi.md)
records exact fields, errors, publication, state owners and cleanup boundaries.
It resolves only the initial two-slot/raw-image portions of `D-02`, `D-05`,
`D-06` and `D-07`, not full credentials, SMP, threads or virtual memory.

## Evidence completed

Retained local artifacts:
`/home/roy/Documents/artifacts/zl-linux/process-spawn-2026-09-08/`.
`verification.json` binds the checked source and outputs; `host-checks.log`,
`negative-controls.json`, `compile-checks.json`, `fixture-checks.log`,
`fault-fixtures.json` and `exit-fixtures.json` preserve the respective lanes.

| Lane | Verified result and limits |
|---|---|
| Actual architecture syscall host harness | `userspawnwaittest`: 625 checks, zero failures; actual constructor, dispatcher, scheduler and reap code, with privileged CR3 reads and device entry replaced by explicit host adapters |
| Inactive image host harness | `userimage64test`: 5,248 checks, zero failures; includes per-byte/PTE checks, disjoint frames, guards, permissions, parent preservation and all eight short-pool allocation failures |
| Existing lifecycle/service/scheduler/frame/anonymous checks | 109 / 144 / 126 / 191 / 243 checks respectively, zero failures |
| Negative controls | Four deliberate broken implementations rejected: contaminated kernel template, child selection during spawn, four-byte handle truncation, adoption from a live parent |
| Target object compilation | Changed architecture/lifecycle sources compile for BIOS32, Multiboot64 and native UEFI64; object compilation is not full linking or boot proof |
| Real zlfs fixture preparation | Both fault and signed-exit binaries assemble and survive fresh read-only exact file readback after seeding both files into a disposable disk |
| Seeder negative controls | Append refuses an unformatted disk without changing it; read-only verification refuses wrong bytes and a matching but shorter prefix without changing the disk |
| ABI and gate structure | Generated syscall table and host inventory checked; landing gate deletion controls checked; exact logs retained locally |

The wrapper inventory refresh also incorporates previously merged tools that
were absent from the retained inventory. It is a static inventory with declared
legacy policy gaps, not fresh build or runtime evidence for those tools.

The syscall harness checks pending and foreign-child refusal, high-bit and
maximum generations, stale reuse, full-table exhaustion, invalid complete
output ranges, zero/oversized/missing programs, corrupt scheduler admission,
nonempty anonymous custody, every image-allocation failure and parent saved/
active state preservation. It injects foreign physical ownership during WAIT
cleanup, verifies that the terminal record/output survive refusal, then repairs
the fixture and successfully retries. Parent death before and after child
termination is checked through the real architecture release and administrative
reap path. It does not execute hardware interrupt entry/return or the real
device reader inside Ring 3.

The target fixture uses IPC rendezvous to keep the child alive for the pending
WAIT assertion. It obtains the sender's actual PID, so repeated cycles do not
assume the child always has PID 1001. The fault child accesses its own virtual
offset corresponding to the parent's private page; the parent expects exact
page-fault vector/error/address and checks its own page remains unchanged.
Fixture-only receipts explicitly say `PASS_FIXTURES_ONLY`.

## Current place in the roadmap

| Sequence | Current state |
|---|---|
| N-PROCESS.01–.10 | Baseline and owners inspected; initial ABI, template, inactive construction and admission rules implemented |
| N-PROCESS.11–.24 | Host implementation/negative evidence present; hardware execution and the full repeated lifecycle matrix remain open |
| N-PROCESS.25 | Both disk fixture scenarios assembled and exact bytes read back |
| N-PROCESS.26–.27 | Passed both scenarios again on repaired native-UEFI image `52dc8b9c…`; exact receipts below |
| N-PROCESS.28–.29 | Terminal retry, exhaustion, repeated cycles, both orphan orderings and final physical totals passed on target; allocation failure injection remains a host lane |
| N-PROCESS.30–.32 | Full regression/hosted closure, evidence rebinding and integration-ready review pending |

## Earlier resource stop and remaining limits

The local machine repeatedly exceeded the root working agreement's roughly
four-core heavy-work load threshold. The complete contained gate also requires
load at most 1.50 and at least 8,192 MiB available memory before starting.
Build/QEMU work was deferred while other tasks occupied the machine. No old
kernel image was booted as evidence for the changed sources.
The final contained-launcher preflight exited 1 with
`load 4.25 exceeds the start ceiling 1.50`; the exact output is retained in
`contained-preflight.log` in the artifact directory above.

The continuation below subsequently built a fresh image and passed both
parent/child probes after repairing a real boot-test regression. The existing
external fault, normal-exit, sleep,
boot and desktop regressions still need current execution through the contained/hosted landing route, which
now includes both new scenarios. A full run must regenerate and validate all
derived build/evidence registries against that image; retained old receipts
must not be treated as current proof.

The subsequent four-scenario continuation below closes the bounded target
orphan-ordering and physical-baseline observations. Allocation failure injection
is host-tested; it is not represented as a QEMU fault-injection sweep. General process scaling, SMP
locking, capability transfer, executable formats, an automatic orphan reaper,
physical qualification and the broader 906-feature contracts remain open.
This pass is not integration-ready and has not been committed or pushed.

## Ownership follow-up, 2026-09-08

After the user challenged the vague wording about concurrent work, a live
process-parent trace identified actual separate zlOS compilation:
`clang` in `/home/roy/Documents/repos/zl-linux/kernel` was launched by
`tools/hazard-scan.sh`, through the main checkout's `tools/preflight.sh`, a Git
operation, and Claude process 313062 under Claude Desktop. This was not this
Codex task's build. The observed compiler PID was 607098; process IDs are
ephemeral and this paragraph is a dated observation.

Main had advanced to `02180af`, after the `b24d111` sweep follow-up, and had
10 changed paths. This task's isolated branch and draft PR #15 remained at
`5a33421`, with the local roadmap/process implementation uncommitted. The nine
registered worktrees are retained checkouts, not evidence of nine active
workers. Main's new changes must be reviewed before any later integration.

This task had completed its focused tests and ended its prior turn; it had no
build, QEMU run, subagent or automatic retry running in the background. A fresh
source-hash comparison still matched all 29 recorded implementation/check
inputs. Rerunning `userspawnwaittest` during this follow-up again produced
625 checks and zero failures. “Machine busy” should not be used as a substitute
for identifying the actual owner of an observed workload.

## Main reconciliation and fresh target proof, 2026-09-08

The original 56 pending paths were copied byte-for-byte into a persistent
snapshot before creating a tenth registered worktree. Main's `b24d111` and
`02180af` changes were combined with the local implementation in the new
checkout; only generated inventories and the host README conflicted. The final
pending merge has no unresolved conflicts. Main's per-core TSS/IST changes
retain the BSP `rsp0` setter used by this single-CPU process implementation.
Claude's live checkout and the original local checkpoint were preserved.
This is repository reconciliation, not direct communication with Claude.

Artifacts: `/home/roy/Documents/artifacts/zl-linux/process-reconcile-2026-09-08/`.
`input-snapshot.json` binds the saved pending paths and selected main commit.
`before-accounting-build-native-result.json` binds the initial source identity
`fe4be2792ea51df9a9946b12bae67a1ceddf467fd1baeae681ac329989f82ed0`
and exact USB image SHA-256
`f509d106ac9cf1d22269b445e0c43dd1f887dcfe7ba0f099a77feeb86770d588`.
The image build completed in 96.18 seconds under a one-core CPU limit and 2 GiB
memory ceiling; this targeted run is separate from the complete landing gate.

The first real boot failed: the built-in unknown-syscall fixture still used 26,
which now names SPAWN. `native-before-fix.json` records exit 1 and
`native-before-fix.log` retains the actual target failure. The added host
regression also failed in three assertions before repair
(`unknown-syscall-red-host.log`). The actual target bytecode now derives the
first gap from the generated syscall header, while the receipt writer reuses
the canonical generator's validated range. The host architecture harness then
passed 632 checks; the five other focused targets also passed.

`initial-signed-exit.json` and `initial-private-page-fault.json` record
`PASS_NATIVE_UEFI64_QEMU` for that first image, with exact fixture/source hashes,
serial transcripts and QEMU diagnostics retained beside them. Both runs require
the existing process boot assertions before starting the external parent.
The parent performs two spawn/wait cycles, checks exhaustion and stale identity,
receives the exact signed exit or private-page fault, then exits with status 37.
Administrative reap leaves the process table empty.

The invalid-output case now proves ordering without a guessed delay: IPC SEND
rejects the child's PID only once it is no longer runnable; the child never
sleeps and no other actor can reap it. Only then does the parent attempt the
invalid WAIT output and retry with a valid buffer. Both target scenarios passed
this terminal-custody retry. Empty process slots alone do not prove exact final
physical-frame totals; the next continuation adds an explicit measurement.


## Four-scenario target closure, 2026-09-08

The latest native image is bound by `build-native-result.json`:
identity `078f1f2f38679e92cbe0915558ea76a179ab997c2f53810b0543ef288209ab23`;
USB image SHA-256 `e398fefeaa1ede099e12bd930b27c55b665bf834a04a9f74e6e4ad022ac8d222`. Adding the read-only `userps` counters required
refreshing the existing application source hash and embedded manifest; the
generator diff changed only those two hashes. The new image built successfully.

All four exact receipts now record `PASS_NATIVE_UEFI64_QEMU`, one virtual CPU,
and physical frames `before=0`, `after=0`, `allocator_faults=0`:

The same generated JSON and byte-identical serial transcripts are retained in
`kernel/docs/receipts/` under the landing gate's declared paths:
[exit](../../kernel/docs/receipts/user-spawn-wait-exit-native-uefi64-qemu-2026-09-08.json),
[fault](../../kernel/docs/receipts/user-spawn-wait-fault-native-uefi64-qemu-2026-09-08.json),
[live orphan](../../kernel/docs/receipts/user-orphan-parent-first-native-uefi64-qemu-2026-09-08.json),
[terminal orphan](../../kernel/docs/receipts/user-orphan-child-first-native-uefi64-qemu-2026-09-08.json).

| Receipt | Actual target observation |
|---|---|
| `signed-exit.json` | Two child generations exit with signed status -37; parent retries terminal WAIT after invalid output and exits 37 |
| `private-page-fault.json` | Two child generations fault on the parent's private-page address with exact vector 14/error 4; parent retains its page and retries terminal WAIT |
| `orphan-parent-first.json` | Parent exits -19 while child is live; child continues and exits -37; kernel reaps child before releasing parent identity |
| `orphan-child-first.json` | Child faults first; parent then exits -19; kernel retains and reaps child's vector-14 record before releasing parent identity |

The child-first administrative reap is intentional: it proves adoption occurred
at the parent's terminating service step. The fallback in later parent cleanup
cannot make this ordering pass. Exact fault error/address custody for orphan
cleanup is additionally checked on host; the existing `userps` command exposes
only the orphan's vector on target.

The target harness reads the existing PMM used-frame count and complete
allocator consistency check after mount, before process admission, and after
final reap. This proves the measured physical-frame baseline is restored,
alongside the empty process table and absence of a scheduler fail-stop. It does
not measure every cache, DMA allocation, graphics surface or kernel resource.

The new scenarios explicitly use one virtual CPU, matching the documented
single-CPU process service. Existing multi-core boot gates are unchanged.
Laptop load checks remain enabled; build and scenario starts
were deferred when load crossed 4. The scenarios subsequently passed under the
one-core/2-GiB execution limit. No physical-hardware claim follows from them.

The full host build completed. Its first execution found the missing interpreter
prerequisite in this new checkout; `full-host-initial-failure.json` retains that
failure. Building `interp` with the repository's existing recipe and rerunning
produced 77 passing targets, three hardware-unavailable outcomes, 14 explicit
non-runs and zero failures. The final host execution also passed with those same counts and binds the
latest `078f1f2f…` image identity. The complete log is
`final-host-execution.log`; `full-host-receipt.json` records every result.


## Existing regressions and current stopping boundary

The final source identity also passed the existing native-UEFI boot gate,
external-file fault/reap, external-file signed exit/reap, and real-tick sleep
probes. `native-efi.log` ends with `EFI gate green`; the current application,
scheduler, process, physical allocator and page-table receipt writers and their
negative controls all passed. `existing-regressions.json` records these exits.

The Run desktop gate first could not start because this isolated checkout had
no GRUB/BIOS ISO. Building the 32-bit kernel and ISO with the existing
`mkiso.sh` recipe succeeded and retained the same route-neutral source identity.
The Run gate then passed its keyboard error ladder and open/ready/close
observations. These are BIOS32 desktop observations, separate from native-UEFI64
process evidence; they do not prove successful loading through the Run app.

The final 47-application lifecycle sweep did not start: its guarded wait expired
after ten minutes with load still above the root threshold of 4. This is an
unrun regression, not a passed or failed app workflow. A separate QEMU process
was observed in `/home/roy/Documents/repos/zl-linux/kernel` during that wait;
it was left untouched. No direct communication with Claude occurred.

The complete landing gate, fresh hosted closure, full BIOS/raw/multiboot64 boot
matrix and physical qualification remain open. `N-PROCESS.30` is the current
sequence; the broader 906-feature maturity ledger is unchanged. The prepared
merge and local feature/roadmap changes remain uncommitted and unpushed.
`verification.json` in the current artifact directory validates all four
source-bound target receipts, the host receipt, image hashes and the completed
regression results, and names the pending desktop sweep explicitly.

## Ownership review and rollback repair

The follow-up code review traced syscall input validation, inactive construction,
publication, scheduler admission, WAIT cleanup and orphan adoption through their
shared owners. It confirmed a failure-accounting defect in
`kernel/src/core/process_memory.c`: allocation rollback decremented `acquired`
before `pmm_release` accepted the frame. A refusal on the final remaining frame
left a retained page with `acquired == 0`. The architecture constructor uses
that count to retain corrupt candidate custody and stop, so the inconsistent
count could defeat that failure path.

`processmemorytest.c` now injects release refusals at the allocator boundary
with the linker's existing wrapping facility. Normal calls still reach the
shipping PMM; there is no production fault switch. The 28 cases cover every
release position after acquiring one through seven frames. Each checks the
retained count, exact page references, real allocator totals and the refusal
to treat partial custody as a ready image. The new test failed before the fix:
555 checks, 28 failures. The fixed test passes all 555 checks. The shared fix
changes the count only after a successful release.

Fresh host consumer builds also passed `userimage64test` (5,248 checks) and
`userspawnwaittest` (632 checks). The changed allocator compiles for BIOS32,
Multiboot64 and native UEFI64; these are object checks, not image/boot results.
The host inventory generator and its negative controls pass with the wrapped
test recipe. Artifacts are `rollback-refusal-red.{json,log}`,
`rollback-refusal-green.{json,log}`, `rollback-consumer-checks.json` and
`rollback-route-compile.json` in the current artifact directory.

The prior verified image checkpoint and pending patch are retained in
`before-rollback-review/`. Its four QEMU passes, full host execution and five
existing regression passes still describe that earlier source. The current
source correctly fails the old build-identity check with “build-identity.json
is stale”; `rollback-prior-image-drift.log` retains that rejection. No receipt
was relabeled to make the new fix appear boot-tested. Rebuild and rerun the
affected host/native/desktop lanes before claiming current target closure.
The complete local gate remains resource-blocked, and the requested commit/push
approval is still pending. No new GitHub publication or direct Claude message
has occurred.

## Rebuilt rollback checkpoint and live coordination

The repaired native image was subsequently built in 95.28 seconds under the
same one-core/2-GiB ceiling. Its source identity is
`52dc8b9c468cc8b2945f0b447f78480c5b94df3613ca4494ac16bc65ca27ec0e`;
the USB image SHA-256 is
`ccdb86b0c2895a39bafc45b4aa4d06191ca09b135c4d80391f98e363ce47739b`.
New attempts and results are retained in `after-rollback-review/` under the
current artifact directory, keeping the preceding runs intact.

The first native gate booted this image through its TCG fallback after a KVM
emulator crash, but then correctly rejected the preceding build's scheduler
host receipt. That gate exited 1 and is not called a pass. The host execution
was then refreshed for `52dc8b9c…`: 94 targets, 82 commands, 77 passed, three
hardware skips, 14 explicit non-runs, zero failures and zero unavailable.
`before-host-refresh-native-efi.log`, `full-host.log` and
`full-host-receipt.json` retain the order and results. The native rerun must
finish before claiming complete current target verification.

The new scenario receipt now includes its imported exercise and external-exit
helpers in source provenance. The older external probes' scope text now says
that those probes do not exercise the separate spawn/wait ABI. It no longer
claims the entire kernel lacks that API. These probe changes are outside the
image's source-identity scope and will be bound by their fresh runtime receipts.

The reconciled roadmap also passed a read-only structural recheck: 7,151 checks,
exact catalogue/contract/target coverage, all 174 original contract bodies,
acyclic explicit dependencies and local links. Its receipt is
`after-rollback-review/check_current_roadmap.json`. These remain planning checks.

At 02:53:47 UTC, 11 registered worktrees were present. An actual compiler-parent
trace identified Claude process 313062 building native EFI in
`/home/roy/Documents/repos/zl-linux/.claude/worktrees/fable-next` on
`fable/usb-replug`, based on `02180af`. A separate compiler was active in main.
This task neither messaged Claude nor changed either checkout.
`after-rollback-review/concurrent-builds.json` retains the dated observation.

Local main had advanced to `bd7555246c0952850d90033803f14662c1e3270e`
(`fix: 32-bit double-fault task gate, bios32 double-fault crash route`). Review
of its implementation diff found the new BIOS32 TSS/task-gate route, shared
crash-record validation and an added landing-gate case; it does not edit
`process_memory.c`. Its 32 changed paths include generated evidence. That
commit is not incorporated into this pending `02180af` merge. Later integration
must retain the new crash gate and regenerate evidence for the combined source;
this branch's results do not verify that combined tree. GitHub PR #15 was still
the earlier draft at `5a33421`, with base `9212979`, when checked in this pass.

An alternate-index merge preview against `bd75552` found ten conflict paths:
nine generated identity/receipt files and `gates/land-gate.sh`. No active index,
working file or branch was changed by that preview. The prepared gate text in
`after-rollback-review/integration-preview-gate.sh` retains all 124 currently
mandatory seams, adds the BIOS32 double-fault case, and preserves bounded
resource waiting. The new case also needs a deletion-control assertion when
integrated. This is a reviewed future resolution, not a tested merged tree.

After the host refresh, the native gate rerun passed in 40.79 seconds. All
four repaired-image parent/child runs passed, followed by the existing external
fault, signed-exit and real-tick sleep probes. `target-checks.json` records all
eight successful command exits. The four scenarios each measured physical
frames `0 -> 0`, no allocator invariant failures, and an empty final process
table. Their exact JSON and serial transcripts are also retained in the project:

- [Signed exit](../../kernel/docs/receipts/user-spawn-wait-exit-native-uefi64-qemu-rollback-2026-09-08.json).
- [Private-page fault](../../kernel/docs/receipts/user-spawn-wait-fault-native-uefi64-qemu-rollback-2026-09-08.json).
- [Live orphan](../../kernel/docs/receipts/user-orphan-parent-first-native-uefi64-qemu-rollback-2026-09-08.json).
- [Terminal orphan](../../kernel/docs/receipts/user-orphan-child-first-native-uefi64-qemu-rollback-2026-09-08.json).

The source-bound verifier in `after-rollback-review/verify_current_checkpoint.py`
checks these receipts, imported helper hashes, exact image bytes and host
executables. It separately records which bounded regression steps have run.
Allocation and release-refusal injection are still host evidence; these QEMU
runs do not add a target allocator-failure injector or a physical qualification.

The repaired BIOS32 ISO then built successfully with the same `52dc8b9c…`
source identity. Its SHA-256 is
`57e57209e9758baffce3a0cd91e5e1e710d6fa060d81b8f06086a0db4574850e`.
The [Run regression](../../kernel/docs/receipts/run-route-qemu-spawn-wait-2026-09-08.json)
passed in 22.51 seconds. The
[47-app lifecycle sweep](../../kernel/docs/receipts/app-lifecycle-qemu-spawn-wait-2026-09-08.json)
passed all 47 open-ready-close cycles in 69.96 seconds, with zero failures.
These are BIOS32 observations of the named routes; app readiness is not a
successful full user workflow, and this Run test does not prove its successful
executable-loading route. Other menu/boot-open/register routes and physical
input/display remain outside these receipts.

`after-rollback-review/verification.json` now verifies the repaired image,
four parent/child receipts, fresh host results, all eight target command exits
and all four post-target command exits. No named step in that bounded local
set remains unrun. The full BIOS/raw/multiboot matrix and complete hosted
landing gate are separate open checks. The final contained-gate doctor still
refused with `a compiler is active; retry after the other build finishes`.
All test services started by this continuation have finished. Commit/push
approval remains pending; the latest main commit and Claude's USB work remain
separate, and this checkpoint is not claimed as full integration or publication.

## Authorized publication checkpoint

The 2026-09-08 publication pass rechecked the saved `52dc8b9c…` image,
receipts, source hashes and all 121 pending paths with the checkpoint verifier.
Its bounded checks pass with zero failed host targets. Commit/push is now
authorized. This first commit preserves that tested source and the prepared
`02180af` merge before incorporating main `bd75552`. The four local process
receipts remain evidence for their recorded source identity, not for a later
combined tree.

The remote integration branch was still `5a33421`, and remote main was
`bd75552`, at the publication check. Claude's separate `fable/usb-replug` branch
now contains `465e075` and journal commit `a3fdb03`; its USB/display changes
remain separate from this process publication. The original dirty integration
checkout is preserved. No direct message was sent to Claude.
