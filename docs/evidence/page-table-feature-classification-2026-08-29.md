# Page-table feature classification - 2026-08-29

This receipt classifies KR-004 against the canonical 906-feature ledger. It
adds a reusable page-table transaction core and routes the 64 MiB heap window
and live framebuffer cache retyping through it. It does not claim that every
page-table user is transactional.

## Result

KR-004 is `PARTIAL_CURRENT`. The exact current subject is the `build_identity`
field in generated `docs/program/FEATURE-STATUS.json` and the page-table
receipt; this page does not duplicate that volatile value.

The transaction contract is:

```text
reserve without writing -> validate the complete set -> apply -> flush
-> caller validation -> commit, or restore every applied entry -> flush -> verify
```

Reservations capture exact prior values and reject duplicates and journal
overflow. Apply revalidates the entire set before its first write. A failed
write, flush, or post-flush readback restores all applied entries. A failed
rollback flush is reported as a hard rollback failure, never as success.

## Current proof

The current 76-target host receipt records 61 passes, 3 hardware skips, 12
explicit non-runs, and zero failures. `pagetxntest` passes 26 assertions across:

- a complete 512-entry transaction;
- all 512 possible nth-write failures;
- stale-entry refusal before mutation;
- duplicate and capacity refusal;
- apply-flush failure and a successful rollback re-flush;
- post-flush corruption detection;
- caller-requested rollback;
- rollback-flush failure classification; and
- commit finality.

The native UEFI64 QEMU receipt binds the exact `zlOS-usb.img`, verifier, host
receipt, transaction implementation, and serial log. The booted kernel mapped
64 MiB at a dynamically selected virtual PML4 slot to the 256 MiB physical heap
and passed the two-way alias probe before commit. The same route also validates
the complete live framebuffer range before applying cache-type changes, flushes
global translations, commits through the transaction core, and records
`cache=write-combining` in the selected boot journal.

Two complete independent build recipes produced byte-identical hashes for all
nine retained artifacts. All six retained QEMU boot routes then passed against
that build identity: GRUB BIOS32, GRUB UEFI32, GRUB BIOS64, GRUB UEFI64, the raw
BIOS loader, and the native UEFI64 loader. The persistent-disk, RTC, network,
crash, graphical app-route, app-lifecycle, and Run probes also passed against
the same current build.

## Evidence ceiling

KR-004 remains partial because:

- callers still provide exclusive page-table ownership rather than a shared
  SMP locking contract;
- there is no general page-table allocator or teardown service;
- fixed Ring 3 private tables are built before CR3 publication but do not use
  the transaction journal;
- future live page-table writers must adopt the transaction core before
  publication;
- live transactional mapping is x86-64 only; the 32-bit build keeps paging
  disabled; and
- no current physical-hardware page-table receipt exists.

Exact current maturity counts remain in generated `program/FEATURE-STATUS.json`.
