# MP-00 structured event trace core — 2026-08-24

## Result

EV-018 now has a real transport-independent core, not only a planned schema.
The implementation is intentionally bounded and pointer-free:

- [`../hosttest/trace_event.h`](../hosttest/trace_event.h) defines the logical envelope and API;
- [`../hosttest/trace_event.c`](../hosttest/trace_event.c) implements admission, sequencing,
  redaction, checksum, queue pressure evidence and wire import/export;
- [`../event-schema.json`](../event-schema.json) is the generated 28-field wire
  contract;
- [`../hosttest/eventtracetest.c`](../hosttest/eventtracetest.c) executes the
  real module under ASan/UBSan;
- [`../docs/receipts/event-trace-host-2026-08-24.json`](receipts/event-trace-host-2026-08-24.json)
  binds 37 passing assertions and three compile lanes to exact source/tool
  hashes.

This is `PARTIAL_CURRENT`. It is not in `kernel/SOURCES`, is not compiled into a
shipped kernel, has no booted emitter, is not SMP/IRQ safe, and has no durable
or tamper-evident service. Those omissions are recorded rather than inferred
away.

## Requirements fixed by the master program

The complete system needs typed monotonic events carrying feature, process,
authority, correlation, drop and redaction identity. The core also has to
survive every ABI zlOS currently builds, avoid allocation at failure time and
never accept a raw user/kernel pointer as evidence.

The resulting non-functional requirements are:

1. fixed memory and execution bounds;
2. no hidden overwrite when a queue is full;
3. stable encoding across ILP32, LP64 and UEFI LLP64;
4. failure before mutation for invalid input;
5. a generation beside every reusable process/authority ID;
6. explicit privacy handling before commit;
7. corruption detection before consumption;
8. honest separation between host proof and target reachability.

## Boundary and data flow

```text
driver / kernel / future service
        |
        | zlos_trace_input (fixed values; no pointers)
        v
  admission and identity checks
        |
        +-- invalid -----------------> caller gets INVALID; no mutation
        |
        +-- queue full --------------> exact pending/total drop counters
        |
        v
 redact classified payload words
        |
        v
 assign sequence + checksum + commit
        |
        v
 fixed caller-owned FIFO
        |
        +-- corrupt head ------------> CORRUPT; record retained
        |
        v
 152-byte little-endian export
        |
        v
 future Audit/Event Service -> durable anchor/export/viewer
```

The core reads no clock. A future owner supplies `monotonic_ns`, boot identity,
process generation, authority generation and correlation identity from the
authoritative mechanisms that own them. This prevents a low-level library from
silently becoming the policy source for identity or time.

## Envelope contract

Every record is exactly 152 bytes. The generated registry is the complete
offset authority; the main semantic groups are:

| Group | Fields | Rule |
|---|---|---|
| Schema | magic, version, bytes | `ZLEV`, version 1, 152 bytes |
| Order | sequence, monotonic time | sequence strictly increases; time never decreases |
| Run | boot ID | nonzero and fixed for one trace |
| Causality | correlation ID, parent sequence | correlation is mandatory; parent is zero or earlier |
| Subject | process ID + generation | both absent or both nonzero |
| Authority | authority ID + generation | both absent or both nonzero |
| Meaning | feature ID, component ID, event code, outcome, kind, severity | bounded tokens and enums only |
| Privacy | privacy class, redaction mask | redacted words become zero before commit |
| Payload | four 64-bit words | no pointers, strings or variable-length bytes |
| Integrity | checksum | covers every preceding logical field |

`SECRET` requires all four payload words to be redacted. The fixed four-word
payload is deliberately small: larger data belongs in a separately bounded
artifact named by a future content identity, not inside an unbounded log line.

## Queue and failure contract

Production capacity defaults to 64 records in caller-owned storage. The module
does not allocate.

When full, the new event is dropped and existing committed evidence remains
byte-for-byte unchanged. The exact count accumulates. Once there is room, the
next successful emit first commits one synthetic `DROP` event containing the
pending count, then commits the caller's event if a second slot remains. If
only one slot is free, the drop marker consumes it and the caller's event is
truthfully counted as another drop.

The queue never:

- overwrites the oldest event;
- calls a logger while handling logger pressure;
- blocks waiting for a consumer;
- allocates emergency memory;
- treats a corrupt checksum as consumable evidence.

Invalid feature/component tokens, unknown enum values, unknown flags,
noncanonical reserved bytes, half-present identity pairs, future parents,
missing correlation, time regression and incomplete secret redaction all fail
before queue mutation.

## Concurrency and authority ceiling

The current queue has one serialized owner. It does not contain a spinlock,
disable interrupts or claim lock-free atomic publication. Adding a casual lock
inside the core would be the wrong abstraction: a fault/NMI path, interrupt
producer and process service have different blocking rules.

The target design will use separate bounded producer lanes:

```text
per-CPU kernel lane ----+
IRQ-safe driver lane ---+--> Audit/Event Service --> ordered merge + anchor
process IPC lane -------+
crash-reserved lane ----+
```

Each lane retains local sequence/drop truth. The service assigns a merge order
without rewriting the original producer order. Crash evidence remains on a
reserved path so ordinary event pressure cannot consume its storage.

## Privacy model

The core enforces mechanics, not policy. It understands four classes:

- `PUBLIC` — safe for ordinary diagnostics;
- `INTERNAL` — system detail not intended for public export;
- `SENSITIVE` — identity/content-derived values requiring named redaction;
- `SECRET` — payload is always all-zero in the committed record.

The future service must own field policy, export authorization, retention,
rotation, user disclosure and erasure behavior. A redaction bit proves only
that a numeric payload word was zeroed. It does not prove that the feature or
component names, event timing or correlation graph are privacy-safe.

## Wire compatibility

Import/export uses explicit little-endian offsets rather than copying compiler
struct padding. A compile-time assertion still pins the in-memory record to 152
bytes. The receipt compiles the same source in three lanes:

1. host ASan/UBSan execution;
2. 32-bit freestanding ILP32 object;
3. x86-64 Windows COFF/LLP64 UEFI object.

Unknown versions fail closed. Version 1 has no optional field bitmap; extending
it requires a new version and an explicit compatibility/migration test rather
than silently reusing reserved bytes.

## Executed proof

The dedicated verifier currently proves 37 assertions, including:

- exact size and schema identity;
- empty initialization and nonzero boot identity;
- exact sequence/time/process/authority/correlation preservation;
- checksum validation and byte-exact wire round trip;
- corrupted wire rejection and short-buffer refusal;
- sensitive and secret redaction behavior;
- time regression, future parent, half identity, missing correlation and bad
  feature rejection without state mutation;
- full-queue no-overwrite behavior;
- FIFO order across wrap/pressure;
- exact synthetic drop count and ordering;
- corrupt-head fail-closed retention.

The receipt self-test independently rejects a hidden host failure, missing
source identity, missing ABI lane, invented target integration and hidden
single-owner gap. The schema self-test rejects a missing/overlapping field,
missing secret class, overwrite policy, invented concurrency and invented
target emitter.

## Integration order

The next target work is dependency ordered:

1. enroll `eventtracetest` in the global host inventory and refresh the entire
   host receipt during an idle contained gate;
2. promote `hosttest/trace_event.c`/`.h` into the kernel runtime, add the C file
   to `kernel/SOURCES`, rebuild all four compiler routes and refresh
   build/source/artifact identities;
3. create one boot emitter for build/boot identity and one drop-pressure probe;
4. capture exact QEMU records from all required routes;
5. introduce per-CPU/IRQ-safe lanes only after scheduler/interrupt ownership is
   explicit;
6. move merge, retention, redaction policy and export into SVC-009;
7. add a reserved durable anchor and recovery reader;
8. connect the provenance viewer without granting it mutation authority;
9. perform native-hardware pressure, reboot and recovery proofs.

No later subsystem may invent a different event envelope. New event kinds and
payload meanings must be registered, versioned and mutation-tested against this
contract.

## Trade-offs and revisit points

- Four payload words keep the failure path bounded but require external
  artifacts for rich diagnostics. Revisit only with measured event families.
- Single-owner semantics are safe and testable now but cannot serve SMP. Split
  producer lanes rather than turning one global ring into a contended lock.
- FNV-1a detects accidental corruption; it is not authenticity. A future anchor
  needs a keyed/signature chain and independent custody.
- Fixed 16-byte identifiers keep records small. If canonical feature/component
  IDs exceed 15 characters, add a versioned numeric registry mapping instead of
  truncating them.
- Little-endian matches every current zlOS target. A future big-endian port must
  keep the wire canonical and prove explicit conversion.

The weakest current link is target absence: every behavior above is real code
with host/ABI proof, but zero booted zlOS component emits it today.
