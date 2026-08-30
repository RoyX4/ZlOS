# MP-00 joined evidence registry

[`../metadata/evidence-registry.json`](../metadata/evidence-registry.json) is the current front
door for implementation evidence. It joins 37 inputs by exact SHA-256 and build
identity: the build identity and reconstructable source snapshot, app
manifest/evidence, artifact/boot registry, six-route hardware receipt plan,
initialization registry, host dependency lock and offline archive receipt,
license registry, adversarial registry, test inventory and execution receipt,
paired reproducibility receipt, host frame/build benchmark receipts, the
performance registry and the current native-UEFI64 page-table transaction
receipt.
It also joins the visual asset registry.
The accessibility proof registry is joined separately from ordinary input.
The security claim registry is joined without promoting limited host checks.
The EV-026 decision ledger is joined without treating a dated decision as
runtime proof or hiding its incomplete historical coverage.
The EV-027 unreleased changelog is joined without inventing a version, channel,
signature, publication or completed migration/rollback story.
The EV-028 provenance viewer is joined without turning a host-static HTML model
into a booted app, live portal, permission system or signed attestation.
Crash/log/event evidence and the separate EV-018 schema/host receipt are joined
without treating a host core or log hash as a shipped audit service.

The result is deliberately `PASS_WITH_OPEN_GAPS`, not “zlOS complete.” Current
joined proof is:

- 9 byte-reproducible artifacts;
- 6 QEMU boot routes;
- 64/64 shipped application identities with open-ready-close evidence;
- 18 ordered initialization stages;
- 154 exact build inputs in a deterministic local archive, with 0 off-host
  copies and no signature;
- 20 locked host commands, 2 OVMF blobs, 101 runtime files, 160 binary package
  archives and 104 source-package sets covering every dependency edge offline;
- 7 build tools, 4 exact target/ABI lanes and 82 external header identities,
  with 0 hermetic builds;
- 154 declared build inputs joined through 483 graph nodes and 759 edges to all
  9 current artifacts, with 0 orphans and 11 explicitly scope-only inputs;
- 61 passing host targets, 0 failures;
- 3 explicit hardware skips and 12 explicit non-runs.
- 6 exact-hash physical route cases with 7 lifecycle stages and 10 required
  raw-evidence classes each; validated physical receipts remain 0.
- 23/23 verifier canaries caught, 0 globally exhaustive failure-injection
  families and 7/9 hostile-input families with executed host proof. Current
  bounded failure proof covers all 32 heap-allocation positions and all 512
  page-table writes; the current ELF and dependency-archive rejection suites
  are included in the seven hostile families.
- 1 current page-table transaction receipt proving all 512 injected host write
  failures plus native-UEFI64 heap-window and framebuffer-cache commits; full
  caller adoption, shared SMP ownership and physical execution remain open.
- 7 host frame metrics measured against the 60 Hz budget, with exact current
  pass/fail state retained only in the hash-bound benchmark receipt and no
  native-target performance distribution.
- all 7 performance categories measured on the current host; exact current
  budget counts and regressions remain in the hash-bound performance registry;
  five subsystem values are gate runtimes rather than product latency
  distributions.
- 45 visual assets inventoried: 4 exact current-build QEMU screenshots with
  strict stable-region goldens across 2 routes and 2 states, plus 41 unbound
  historical images. The native-UEFI capture uses a temporary write snapshot
  and revalidates the boot artifact after QEMU exits. All 6 required
  visual-variant dimensions remain open.
- 4 accessibility primitives host-passed, 2 partial, 9 missing and 0 complete
  target workflows.
- 17 security claims recorded: 1 static, 4 host-limited, 1 QEMU-partial, 11
  missing and 0 production-complete.
- 20 normalized decision/reversal/deprecation records, all 47 legacy labels
  indexed and normalized, and 3 superseded records retained with replacements.
- 17 unreleased change candidates, 6 potentially user-visible candidates, 3
  migrations, 12 known issues and 3 recovery paths; published entries remain 0.
- a self-contained read-only provenance projection of 9 artifacts, 64 apps, 17
  security claims, 17 changes and 13 health areas, with 0 signatures and 0
  declared per-app permission grants.
- 15 observability capabilities: 1 QEMU hash-only, 2 QEMU-proved capabilities,
  1 QEMU-proved-partial capability backed by BIOS32 and native-UEFI64 full
  register receipts, 1 host-limited, 4 host-proved event-core,
  1 host-proved partial export/import and 5 missing. EV-018 has 28 typed wire
  fields, 37 passing host assertions and 3 compile lanes, but 0 target emitters;
  durable crash receipts remain 0.

The registry preserves every current blocker: the source snapshot has no
off-host copy, signature or whole-repository closure; all 9 artifact hashes lack
physical proof; the 3 hardware skips and 12 non-runs are not passes; all 154
build inputs lack an established redistribution grant; public release remains
blocked; all 7 failure families remain open; and 2 hostile-input families remain
open: font and typed IPC, whose product parsers do not exist yet. Offline
dependency resolution is complete locally, but the archive has no
off-host custody and the toolchain is not hermetically rebuilt. Native-target
performance distributions remain absent.
The performance registry has no timed product-artifact build distribution; any
current host regression remains explicit in its generated `open_regressions`.
The four current goldens cover only their named default routes/states;
old/unbound screenshots and missing variants are not current regression proof.
Keyboard/focus tests do not imply semantic or assistive-provider completion.
Security source or host proof does not imply complete mediation or production safety.
A recorded decision does not imply runtime effect, complete repository history
or independent approval.
An unreleased change candidate is not a signed, published or release-complete
user promise.
A static provenance viewer is not a live authenticated zlOS service, admitted
permission grant or cryptographic attestation.
A hashed boot transcript is not a raw crash bundle or durable audit trail.

Its self-test plants a missing registry, invented source custody, physical
overclaim, hidden hardware matrix gap, hidden hardware skip, hidden release block, hidden failure-injection,
hostile-corpus, performance, accessibility, security and observability gaps,
the hidden decision-history, release-note and provenance-runtime blockers,
lost offline dependency proof, invented native/visual proof and a missing build
identity. The landing gate
writes and checks this join last, after every producing receipt has refreshed.
