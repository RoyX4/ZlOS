# Physical page allocator feature classification

Date: 2026-08-30

Feature: `KR-001` physical page allocator

## Red observation

The canonical feature row required typed ownership, reserved-map proof,
deterministic exhaustion, zero/reuse and double-free detection. Before this
tranche, no feature-specific implementation or receipt was joined. The kernel
used fixed physical regions and UEFI page allocation before
`ExitBootServices`; there was no post-handover frame allocation service.

## Implementation

`kernel/src/core/pmm.c` consumes the sealed boot memory map and manages only
complete 4 KiB pages which firmware labels usable. Its first bounded policy is
deliberately conservative:

- addresses below 320 MiB are always reserved because they contain the kernel,
  fixed zlOS regions and DMA arenas;
- addresses at or above 1 GiB are outside this version's metadata contract;
- UEFI admits only `EfiConventionalMemory`; Multiboot admits only type 1;
- every allocated page carries one explicit owner tag;
- each owner has exact live, high-water, available and refusal counters;
- allocation stops at the configured owner quota and a shrink below live usage
  is refused without mutation;
- reserved, unaligned, wrong-owner and double releases are refused;
- every successful allocation is zeroed before publication; and
- release moves the scan cursor back so reuse is deterministic and testable.

The allocator is initialized before the program arena and kernel heap on boot
routes that carry a memory map. Raw BIOS remains unchanged because its typed
handover has no memory map. Native UEFI64 process slots now consume eight typed
frames each for PML4, PDPT, PD, PT, code, user stack and a two-page kernel
stack. Successor allocation completes before predecessor release under a
16-frame owner quota; each anonymous owner is bounded to its 32-frame window.
Release preflights every owner before mutating the allocator.

## Current proof

The current host inventory contains 70 compiled targets and 9 scripts. It ran
69 automatic commands: 64 targets passed, none failed, 12 manual/instrument
targets were not run and 3 hardware-only targets were explicitly skipped.

`pmmtest` passed 177 checks over the shipping allocator source. Coverage
includes malformed and overflowing ranges, overlapping reserved descriptors,
partial-page admission, the 320 MiB fixed floor, the 1 GiB ceiling, sealed UEFI
and variable-entry Multiboot parsing, unretired and overflowing map pointers,
typed ownership, wrong-owner non-mutation, reserved/unaligned/double-free
refusal, complete exhaustion, failed-zeroing rollback, measured high-water
accounting, exact per-owner live/high-water/available/refusal totals, quota
refusal, non-mutating failed quota shrink, deterministic reuse,
zero-on-allocation and a full metadata-to-owner invariant walk.

`processmemorytest` passed 191 checks over the shipping process-frame
owner. It forces every short-pool acquisition failure, proves exact rollback,
keeps two process allocations disjoint, refuses a foreign-owner teardown before
any release, deterministically replaces a process and restores the PMM baseline.

`anonmemorytest` passed 243 checks over the shipping anonymous-frame consumer.
It proves no-frame reservation, exact zero-filled commit and reuse, every
short-pool OOM and PTE rollback point, collision and foreign-owner refusal,
hardware accessed/dirty-bit admission, mixed release and holey teardown.

The exact native-UEFI64 QEMU artifact reported:

```text
pmm: 168259/168259 pages free in [320, 1024) MiB
<- physical allocator reserved floor, owner quota/mismatch, double-free and zero/reuse passed; baseline restored
<- process memory accounting: fixed/anonymous quotas and owner totals passed
<- process-owned page tables/code/stacks reclaimed; PMM baseline restored
```

The exact Multiboot golden route independently reported `180192/180192` pages
and the same restored-baseline marker. It is live route coverage, but it does
not yet have its own dedicated allocator receipt.

The receipt binds that log to the current USB image, verifier, implementation,
host receipt, user-process consumer receipt and generator. Sixteen receipt
mutations prove that a foreign build, artifact or consumer receipt, missing
assertion or ownership claim, invented restored baseline, invented reserved
floor, lost host exhaustion or process-baseline proof, missing log identity,
foreign generator, wrong host target, invented owner-account invariant,
substituted gap or hidden gaps are rejected.

## Classification

`KR-001` is `PARTIAL_CURRENT`, not complete. Current evidence proves a bounded
allocator core, one exact Multiboot golden route and one dedicated
native-UEFI64 QEMU receipt. The native process diagnostic is now a real
allocator consumer, including bounded anonymous pages, but this does not prove
a general physical-memory subsystem.

Open work remains:

- production accounting covers only the fixed two-slot native-UEFI64 process
  path and its bounded anonymous window;
- typed owner IDs are not per-allocation origin or call-site provenance;
- general page-table services, drivers and persistent process creation do not consume it;
- the ownership table is bounded to 320 MiB through the first 1 GiB;
- no segmented metadata supports memory above 1 GiB;
- no concurrent, interrupt-context or SMP locking contract exists;
- Multiboot has an exact live golden route but no dedicated allocator receipt; and
- no current physical-hardware allocator receipt exists.
