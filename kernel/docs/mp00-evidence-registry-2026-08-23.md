# MP-00 joined evidence registry

[`../evidence-registry.json`](../evidence-registry.json) is the current front
door for implementation evidence. It joins 25 inputs by exact SHA-256 and build
identity: the build identity and reconstructable source snapshot, app
manifest/evidence, artifact/boot registry,
initialization registry, host dependency lock, license registry, adversarial
registry, test inventory and execution receipt, and paired reproducibility
receipt, and host benchmark receipt.
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
- 62/62 shipped app identities with open-ready-close evidence;
- 16 ordered initialization stages;
- 123 exact build inputs in a deterministic local archive, with 0 off-host
  copies and no signature;
- 15 locked host commands and 2 OVMF blobs;
- 7 build tools, 4 exact target/ABI lanes and 82 external header identities,
  with 0 hermetic builds;
- 123 declared build inputs joined through 404 graph nodes and 663 edges to 268
  logical objects and all 9 current artifacts, with 0 orphans and 1 explicitly
  scope-only conservative header;
- 48 passing host targets, 0 failures;
- 3 explicit hardware skips and 11 explicit non-runs.
- 18/18 verifier canaries caught, 0 exhaustive failure-injection families and
  5/9 hostile-input families with executed host proof.
- 3/7 host frame metrics within 16.667 ms, with 4 open regressions and no
  native-target performance distribution.
- 46 visual assets inventoried, but 0 bound to the current build and all 6
  required visual-variant dimensions still open.
- 4 accessibility primitives host-passed, 2 partial, 9 missing and 0 complete
  target workflows.
- 17 security claims recorded: 1 static, 4 host-limited, 1 QEMU-partial, 11
  missing and 0 production-complete.
- 19 normalized decision/reversal/deprecation records, all 47 legacy labels
  indexed, 5 legacy meanings normalized, 42 still open, and 2 superseded
  records retained with their replacements.
- 17 unreleased change candidates, 6 potentially user-visible candidates, 3
  migrations, 12 known issues and 3 recovery paths; published entries remain 0.
- a self-contained read-only provenance projection of 9 artifacts, 62 apps, 17
  security claims, 17 changes and 11 health areas, with 0 signatures and 0
  declared per-app permission grants.
- 14 observability capabilities: 1 QEMU hash-only, 1 QEMU-proved control frame,
  1 QEMU-proved-partial capability, 1 host-limited, 4 host-proved event-core,
  1 host-proved partial export/import and 5 missing. EV-018 has 28 typed wire
  fields, 37 passing host assertions and 3 compile lanes, but 0 target emitters;
  durable crash receipts remain 0.

The registry preserves every current blocker: the source snapshot has no
off-host copy, signature or whole-repository closure; all 9 artifact hashes lack
physical proof; the 3 hardware skips and 11 non-runs are not passes; all 123
build inputs lack an established redistribution grant; public release remains
blocked; all 7 failure families remain open; and 4 hostile-input families remain
open. Four host frame metrics remain over budget; target percentiles remain absent.
Old/unbound screenshots are not current visual-regression proof.
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
overclaim, hidden hardware skip, hidden release block, hidden failure-injection,
hostile-corpus, performance, accessibility, security and observability gaps,
the hidden decision-history, release-note and provenance-runtime blockers,
invented native/visual proof and a missing build identity. The landing gate
writes and checks this join last, after every producing receipt has refreshed.
