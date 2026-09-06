# Next userspace process-management slice

Source audited at `7e395d5` on 2026-09-05. This is the next implementation
contract for Wave 3 / KR-027 and KR-028 after persistent scheduling and sleep.
It does not implement or advertise userspace spawn/wait.

## Existing pieces to reuse

`process_lifecycle` already provides generation-safe identities, parent-only
terminal observation, exact exit/fault records and refusal to reap a parent
while children still reference it. `process_memory` acquires and releases the
eight typed process frames. The persistent service binds those identities to
the existing scheduler policy. `user64_name`, the shipping zlfs reader and
complete-range usercopy already exist. No second process registry, file loader,
timer queue or generic agent framework is needed.

## First dependency: separate image preparation from active execution

The current architecture loader has a kernel-caller contract:

- `process64_prepare` reads the active CR3 at lines 688-689 of
  `kernel/src/arch/x86/usermode.c` and copies all 512 PML4 entries at line 725.
- Its final publication changes `proc64_index`, `proc64`,
  `user64_kernel_cr3` and `user64_process_cr3`; the latter two are consumed by
  assembly enter/resume/abort.
- A syscall runs with the calling process's address space active. Reusing
  this helper directly would therefore copy the parent's user mappings and
  replace the caller's active context while its syscall frame is still live.

These are extension hazards found by source inspection, not evidence of an
existing userspace spawn exploit: that syscall does not exist.

Before adding spawn, construct a candidate image from an explicitly captured
supervisor-only kernel template, keeping all active caller state unchanged.
Separate image construction/publication from selection and actual entry.
Prove that a child has only its own user mappings, and that both successful and
failed preparation preserve the parent's CR3, register frame, TSS stack and
return path. Do not simply change the syscall table to admit another number.

## Bounded spawn and wait contract

The first spawn call should accept a validated name in the existing flat zlfs
namespace and the existing 1..4096-byte raw executable format. Its parent is
always the current lifecycle handle derived by the kernel. Userspace never
supplies a privileged parent token. It publishes the new generation-tagged child
handle only after image, identity and scheduler admission all succeed.

Use an eight-byte output buffer for that handle and return zero or a negative
error from the syscall. The existing handle is `(generation << 32) | (slot+1)`
and permits generation `0xffffffff`; its high bit is not an error bit.
Returning the handle through the signed-error register would collide with
valid generations. Validate the entire writable output range before acquiring
resources, preserve all 64 bits, and leave it untouched on failure. A truncated
or negative-looking handle is not permission to reinterpret it as a PID.

Use one empty slot, never replace a live process. Prepare all fallible resources
before publication. On any failure, retain the caller and restore exact PMM,
identity and scheduler ownership. Exhaustion returns an explicit error. Do not
invent ELF, argument inheritance or shared-library support in this change.

Wait should accept an exact child handle and a versioned fixed-size result
buffer. A live child returns an explicit pending result; a foreign child is
denied and a stale generation cannot name a replacement. The termination record
must distinguish signed normal exit from fault vector, error and address.
Validate the complete output range before consuming terminal custody. Retain
the result and identity if cleanup cannot complete, so a failed operation does
not lose the child's only exit record.

Parent death needs an explicit orphan owner and cleanup order before spawn is
exposed. The current lifecycle contract deliberately retains the parent until
its child references close. Automatically reaping that parent, or passing zero
from a userspace request as kernel authority, would violate the existing model.
For this bounded service, use kernel custody for the direct children of a
terminal parent. A privileged lifecycle operation validates the exact terminal
parent and the complete table before atomically transferring those parent
references to kernel ownership. It preserves child handles, states and
termination records. Runnable children continue; exited/faulted children stay
observable through the existing administrative path until explicitly reaped.
Only then can the old parent identity be released. This does not add an
automatic orphan reaper or allow a userspace caller to assert kernel authority.
Prove that every adopted descendant remains reachable and reclaimable; two-slot
exhaustion must remain an explicit error rather than silently losing records.

## Required proof

Host tests must cover generation reuse, foreign-parent rejection, pending wait,
signed exit and fault records, parent death before and after child termination,
and rollback at every allocation/admission/publication boundary. Include handle
generations `0x80000000` and `0xffffffff`, complete output-range rejection and
unchanged output buffers after failures. Preserve exact outputs and ownership
on failed calls.

The target route must run a disk-loaded parent which spawns a disk-loaded child,
survives child failure, receives its exact status and reaps it. Include a child
attempt to access the parent's user page, successful independent continuation
of the parent, invalid output-pointer retry without losing exit custody, slot
reuse with stale-handle refusal, and final allocator/scheduler baselines.

Existing fault, normal-exit, sleep, boot and desktop gates remain mandatory.
The two-slot bound, absent SMP locking, absent general capability transfer,
general executable formats and physical qualification remain open parts of the
full 906-feature program.
