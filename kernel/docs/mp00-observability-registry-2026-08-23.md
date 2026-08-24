# MP-00 crash and event-trace truth

`kernel/observability-registry.json` records 14 crash/log/event capabilities.
Six ordinary QEMU boot routes preserve only a SHA-256 of their boot transcript;
the raw logs are not stored. Heap refusal/diagnostics have limited host proof.
The dedicated crash route now executes a real `UD2` in the exact current ISO,
captures vector 6 plus the architectural control frame, independently verifies
its checksum and exact linked instruction address, emits one bounded `ZLCRASH`
line, and proves the guest remains halted.

This is deliberately partial fault evidence. General registers are not captured
because the compiler interrupt prologue has already changed them before C sees
the frame. There is still no symbolized stack, guest-persisted crash dump, crash
service or recovery policy.

EV-018 now has a separate host-proved core described in
[`mp00-structured-event-trace-2026-08-24.md`](mp00-structured-event-trace-2026-08-24.md).
It defines a versioned 152-byte envelope, monotonic order, process/authority
generations, correlation, redaction, explicit drop markers, checksum and stable
little-endian import/export. That core is not compiled into the shipped kernel,
has no target emitter and is externally serialized. It therefore does not prove
a system audit service, durable/tamper-evident anchoring or target policy.

The host gate runs 27 admission/commit/checksum/bounds checks under ASan/UBSan.
The QEMU receipt self-test rejects a wrong vector, wrong checksum, wrong symbol,
stale artifact and a guest that did not halt. The registry self-test also
rejects fault-frame overpromotion, invented general-register coverage and
structured-event field drift.
