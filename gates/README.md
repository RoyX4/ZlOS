# Landing Gates

This directory owns orchestration for checks required before integration.
Individual language, documentation, host, QEMU, and hardware checks stay with
the subsystem they validate; `land-gate.sh` coordinates them and reports what
was passed, skipped, failed, or left unverified.
