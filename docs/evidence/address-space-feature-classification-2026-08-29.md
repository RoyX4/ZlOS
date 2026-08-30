# Address-space feature classification - 2026-08-30

This receipt classifies KR-003 against the canonical 906-feature ledger. It
adds a generated joined address-space contract and does not claim that the
kernel now has a general transactional page-table mapper.

## Result

KR-003 is `PARTIAL_CURRENT`. The exact current subject is the `build_identity`
field in generated `docs/program/FEATURE-STATUS.json` and the address-space
registry; this page does not duplicate that volatile value.

`kernel/metadata/address-space-registry.json` joins and validates:

- 18 fixed physical reservations plus the PMM-owned dynamic frame region from
  320 MiB to the 1 GiB managed-memory ceiling;
- six per-process user-slot regions: RX code, an absent lower user-stack
  guard, an RW/NX user stack, an absent lower kernel-stack guard, and a
  two-page supervisor RW/NX kernel stack, followed by a 32-page anonymous
  window whose reserved state is absent and committed state is user RW/NX;
- all eight required ownership categories: kernel, user, device, stack, heap,
  shared, guard, and dynamic physical frames;
- 24 exact source assertions against the linker, memory-map header, heap and
  physical allocators, typed process-frame contract, scheduler, user-mode
  setup, anonymous-window contract, and paging code.

The generator sorts each map and rejects empty, inverted, duplicate, or
overlapping regions. It also recomputes byte lengths and rendered ranges,
checks the exact category/count summary, binds the registry to the current
build identity, and verifies every source assertion hash.

## Red/green evidence

Before this tranche, the fixed physical map, zl low-buffer checker, and dynamic
Ring 3 layout were separate authorities. There was no generated manifest that
could answer whether kernel, user, device, stack, heap, shared, and guard
regions were jointly represented.

The current self-test plants and catches:

- a physical overlap;
- an inverted user stack;
- a missing required category;
- a foreign build identity;
- stale derived size/range fields;
- drift in an asserted source address.

The mandatory landing gate now runs both write and check modes. The gate's own
mutation checker requires both invocations, so deleting either makes the gate
red. The existing low-memory overlap and mirror checks remain separate and
mandatory because they validate the internal zl buffer layout inside the
joined map's conservative shared-memory envelope.

## Evidence ceiling

This is source/build proof. It does not prove:

- an internal generated size manifest for every zl low shared buffer;
- which free PML4 slot a process receives at runtime;
- physical memory above the current 1 GiB managed ceiling;
- full-range reserve, validate, apply, flush, commit, or exact rollback of page
  table mutations;
- QEMU execution of every mapped region;
- physical-hardware execution.

Those limits keep KR-004 partial and the physical proof lanes open. Exact
current maturity counts remain in generated `program/FEATURE-STATUS.json`.
