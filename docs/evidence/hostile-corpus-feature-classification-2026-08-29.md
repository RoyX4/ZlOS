# Hostile-corpus feature classification - 2026-08-29

This receipt classifies EV-020 against the canonical 906-feature ledger. It
records current host rejection evidence without claiming target-hardware or
parser coverage for product surfaces that do not exist.

## Result

EV-020 is `PARTIAL_CURRENT`. The exact current subject is the `build_identity`
field in generated `docs/program/FEATURE-STATUS.json` and the bound adversarial
registry; this page does not duplicate that volatile value.

The adversarial registry catches all 21 planted verifier mutations and records
executed host evidence for 7 of 9 hostile-input families: ELF, dependency
archives, filesystem, network, image, web and certificates.

## Current rejection proof

`check-elf-permissions.py --selftest` constructs ten minimized malformed ELF
cases covering empty and invalid identification data, unsupported class and
endianness, undersized and missing program headers, out-of-bounds and truncated
tables, missing load segments and an executable-writable load segment. It also
checks every current kernel image and proves that a planted RWX mutation is
rejected.

`verify-dependency-archives.py --receipt-check --selftest` rejects missing
binary and source archives, manifest and relationship drift, invented offline
resolution, missing trust roots, byte changes, archive path escapes, malformed
`.dsc` control data and truncated `.deb` payloads. The full local-cache mode
additionally opens and hashes all 160 binary archives and 104 source-package
sets without network access.

The existing filesystem, network, PNG/DEFLATE, HTML/CSS/JS/HTTP and X.509 host
targets remain joined through the build-bound host-test receipt.

## Evidence ceiling

No font parser or hostile font corpus exists. zlOS also has no typed IPC
envelope parser or hostile IPC corpus. Those two absent product surfaces remain
explicitly missing rather than being represented by synthetic tests detached
from shipped code.

The ELF and archive corpora have not been replayed by a target-side parser, and
no physical machine receipt exists. EV-020 therefore remains
`PARTIAL_CURRENT`; the current host proof does not imply complete hostile-input
coverage, target execution or physical validation.
