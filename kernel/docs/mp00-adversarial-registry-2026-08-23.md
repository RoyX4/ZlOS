# MP-00 verifier canary, failure-injection and hostile-corpus registry

`kernel/metadata/adversarial-registry.json` is the current machine-readable boundary for
EV-013, EV-019 and EV-020. It runs 21 verifier self-tests and refuses to call a
verifier trusted unless its planted mutation is caught.

The two crash canaries independently mutate-check the BIOS32 and native-UEFI64
full-register receipts; one route cannot silently stand in for the other.

The result is deliberately `PASS_WITH_OPEN_GAPS`, not complete adversarial
coverage. Current host evidence covers seven of nine hostile-input families:
ELF, dependency archives, filesystem, network, image, web and certificates.
The image corpus is the registered sanitizer-backed `pngtest` gate. Font and
typed-IPC product parsers and corpora are absent.

Failure injection is still partial, but it is now current and exact for two
bounded transactions: all 32 positions in a heap-allocation sequence and all
512 writes in a page-table transaction, including flush failure, post-flush
corruption and exact rollback. Queue, I/O and lifecycle paths retain narrower
partial tests; provider, service and package rollback injection is missing.
None of the seven broad failure families is globally exhaustive yet.

Run:

```sh
python3 kernel/tools/generators/gen-adversarial-registry.py --write --selftest
python3 kernel/tools/generators/gen-adversarial-registry.py --check --selftest
```

The generator also plants missing-canary, lost-allocation-sweep,
page-table-failure-drift, hidden-failure-gap, lost-ELF-proof,
unearned-font-promotion, archive-evidence-drift, hidden-hostile-gap and
missing-build-identity defects. Every one must make validation fail.
