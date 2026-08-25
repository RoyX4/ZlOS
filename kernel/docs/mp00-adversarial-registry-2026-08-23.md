# MP-00 verifier canary, failure-injection and hostile-corpus registry

`kernel/metadata/adversarial-registry.json` is the current machine-readable boundary for
EV-013, EV-019 and EV-020. It runs 18 verifier self-tests and refuses to call a
verifier trusted unless its planted mutation is caught.

The result is deliberately `PASS_WITH_OPEN_GAPS`, not complete adversarial
coverage. Current host evidence covers five of nine hostile-input families:
filesystem, network, image, web and certificates. The image corpus is now the
registered sanitizer-backed `pngtest` gate. ELF, archive, font and typed-IPC
corpora are absent.

Failure injection is even earlier: allocation, queue, I/O and lifecycle paths
have partial tests, while provider, service and package rollback injection is
missing. None of the seven required failure families is exhaustive yet.

Run:

```sh
cd kernel
python3 gen-adversarial-registry.py --write --selftest
python3 gen-adversarial-registry.py --check --selftest
```

The generator also plants missing-canary, hidden-failure-gap,
unearned-ELF-promotion, hidden-hostile-gap and missing-build-identity defects.
Every one must make validation fail.
