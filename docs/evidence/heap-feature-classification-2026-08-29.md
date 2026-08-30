# Kernel heap feature classification

Date: 2026-08-29

Feature: `KR-002` kernel heap

## Red observation

The host probe was extended before the allocator. Linking the shipping
`kernel/src/core/heap.c` then failed on the absent interfaces
`heap_alloc_tagged`, `heap_tag`, `heap_fail_after`, `heap_fail_disable`, and
`heap_injected_failures`. This established that allocation tags and controlled
failure were not merely undocumented existing behavior. Freed-block poison was
also absent.

## Implementation

- The existing 16-byte header remains identical across ILP32, LP64, and LLP64.
  Its upper flag bits now carry a 24-bit diagnostic tag.
- `heap_realloc` preserves the tag whether growth is in place or moves the
  payload.
- `heap_fail_after(n)` injects one counted allocation refusal after exactly `n`
  successful calls, then disables itself.
- Every free-list block receives two poison words after its two link words.
  Removal validates the guard before the block can be returned to a caller.
- Poisoning is intentionally constant-size. It does not turn frame-path frees
  into work proportional to allocation size.

## Current proof

The exact current subject is the `build_identity` field in generated
`docs/program/FEATURE-STATUS.json` and the bound host receipt; this page does
not duplicate that volatile value.

The current host receipt records `HT-030 heaptest` passed. Its exact output
includes:

- 2,162 checks and zero failures;
- 4,000 mixed allocation/free operations with payload verification and an
  invariant walk after every mutation;
- measured worst cases of three allocation steps and four free steps;
- tag readback and preservation across `realloc`;
- a 32-position allocation-refusal sweep proving the selected refusal changes
  neither used-byte nor live-block accounting, later calls recover and all
  successful allocations are released; and
- detection of a freed-pointer write before reuse, followed by deliberate heap
  shutdown and successful test-only reinitialization.

The changed allocator also compiled across the complete 66-target host build.
A native UEFI64 QEMU boot reached the real desktop, Ring 3, scheduler, IPC, and
post-ExitBootServices path with the heap online.

## Classification

`KR-002` is `PARTIAL_CURRENT`, not complete. The feature-specific tag,
injection, poison, stress, and bounded-step observations are host execution.
QEMU proves the changed allocator remains usable on one target route, but it
does not exercise those diagnostic controls directly.

Open work remains:

- poison covers an 8-byte guard, not every freed payload byte;
- tags are diagnostic values, not an ownership or quota policy;
- failure injection covers allocation, not every dependent I/O/service family;
- no concurrent allocator or interrupt-context contract is proved; and
- no current physical-hardware receipt exists.
