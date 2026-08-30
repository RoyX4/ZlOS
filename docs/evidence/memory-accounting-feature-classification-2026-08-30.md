# Memory accounting feature classification

Date: 2026-08-30

Feature: `KR-015` memory accounting

## Red observation

The canonical row requires process, service, cache, pinned, DMA, surface and
kernel totals with quotas and provenance. The bounded physical allocator had a
global used/high-water/refusal total and byte owner tags, but it did not expose
exact per-owner totals or enforce owner limits. `KR-015` therefore remained
`PLANNED_UNPROVED` even after process-frame reclamation existed.

## Implemented slice

`kernel/src/core/pmm.c` now keeps one exact account for each admitted owner:

- live pages;
- high-water pages;
- configured page limit;
- currently available pages, bounded by both global free pages and owner quota;
- allocation refusals; and
- the typed PMM owner identifier as the current bounded provenance key; and
- an invariant walk that recomputes every owner total from page metadata and
  requires their sum to equal the global used total.

Allocation at the owner limit is refused. Lowering a limit below current live
ownership returns `PMM_E_LIMIT` without changing the account. Release checks the
page owner before decrementing its exact total. Reinitialization clears quota
and history state.

The native UEFI64 process path binds this policy to four production owners:

- fixed process-image owners 32 and 33 each have a 16-page limit, allowing the
  complete eight-page successor image to be acquired before predecessor release;
- anonymous-memory owners 64 and 65 each have a 32-page limit, equal to the
  complete typed anonymous window.

## Proof contract

The shipping allocator host test covers empty accounts, independent owners,
exact usage/high-water/availability/refusal totals, one-over-quota refusal,
failed shrink non-mutation, grow-and-reuse, metadata-to-owner invariants,
release history and reset behavior. The process and anonymous host tests retain
their exhaustive short-pool rollback and reclamation checks.

The native UEFI64 boot gate requires this exact marker from the current image:

```text
<- process memory accounting: fixed/anonymous quotas and owner totals passed
```

That marker is emitted only after both fixed and both anonymous owner totals are
zero, their limits are exactly 16/16 and 32/32, high-water remains bounded,
refusal totals remain zero, the global PMM baseline is restored and the complete
metadata invariant walk passes. The user-process and PMM receipts bind the
marker to the exact artifact, sources, host receipt and receipt generators.

## Classification

`KR-015` is `PARTIAL_CURRENT`, not complete. This tranche proves exact bounded
physical-page accounting and quota enforcement for two fixed process-image
owners and two anonymous-memory owners.

Open work remains:

- no unified service accounting;
- no page/file-cache accounting;
- no pinned-memory accounting;
- no DMA accounting;
- no surface accounting;
- no complete attribution of every kernel allocation;
- no allocation-origin provenance beyond the typed PMM owner identifier;
- no memory-pressure or reclaim policy driven by these accounts;
- no concurrent, interrupt-context or SMP ownership contract; and
- no current physical-hardware accounting receipt.
