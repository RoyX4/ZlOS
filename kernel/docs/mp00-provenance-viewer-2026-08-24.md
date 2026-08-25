# MP-00 provenance viewer — 2026-08-24

## Result

[`../provenance-viewer.json`](../provenance-viewer.json) and the self-contained
[`provenance-viewer.html`](provenance-viewer.html) implement the first EV-028
read-only provenance viewer. The static projection includes the exact build and
source origin, licensing and signature state, 9 artifacts, 62 applications, 17
security/permission claims, host/QEMU test counts, 11 health areas and 17
unreleased changes.

This is `PARTIAL_CURRENT`. It is a host-viewable static artifact, not a booted
zlOS application, live monitor, authenticated remote service or signed release
portal. All cryptographic-signature counts remain zero and per-application
permission grants remain absent.

## One-way evidence architecture

The viewer reads lower-level manifests and the EV-027 unreleased changelog. It
does not read `evidence-registry.json`. The final evidence registry reads the
viewer model after generation, preserving this direction:

```text
source/build/license/test/security manifests
                     |
             decision ledger
                     |
            unreleased changelog
                     |
             provenance viewer
                     |
         joined evidence registry
```

No displayed summary can therefore become its own evidence source.

## Visual and interaction system

The design-system pass chose a **flight-recorder evidence room** rather than a
marketing dashboard. It uses a fixed token set: charcoal canvas/panels, warm
paper text, amber warnings, red blockers, green passes and cyan links. Headings
use a book-like system serif; identifiers and statuses use the system monospace
stack. There are no gradients, remote fonts, images or network requests.

The layout uses a persistent evidence rail and asymmetric identity/status area,
then dense tables for facts that need exact comparison. It avoids card-grid
decoration and keeps the evidence ceiling beside the data.

The HTML includes:

- a skip link, one `main` landmark and labelled navigation;
- visible status words so color is never the only signal;
- keyboard-focus outlines and native searchable/filter buttons;
- an `aria-live` result count and an explicit empty result state;
- captions and row headers on all five data tables;
- reduced-motion handling and a responsive single-column layout;
- a restrictive no-network CSP and no external assets, `fetch`, `innerHTML` or
  document-writing path.

## What “permissions” means here

The current application manifest has zero admitted per-app permission grants.
The viewer does not invent them. Its Permissions & Security table exposes the
17 current security claims, their enforcement points and residual risks; every
application row says `MISSING` for permission grants. When capability-backed
app manifests exist, this field can become a real grant projection.

## Rejection tests

The generator rejects:

- a missing artifact;
- an invented cryptographic signature;
- a hidden licensing block;
- an invented application permission;
- an invented runtime zlOS viewer route;
- missing focus styling;
- any injected external request;
- an artifact omitted from the HTML.

It also checks exact component/landmark counts, every artifact/application/
security/change row, all source hashes, design tokens, open gaps and the absence
of unsafe/external HTML patterns. The 906-row join separately rejects unknown
canonical feature IDs.

## Commands

```sh
python3 kernel/tools/generators/gen-provenance-viewer.py --write --selftest
python3 kernel/tools/generators/gen-provenance-viewer.py --check --selftest
```

## Open completion gates

EV-028 still requires a process-isolated zlOS app or authenticated read-only
portal, admitted per-app permission data, signed provenance, live bounded health
updates, a current screenshot/visual receipt, a complete assistive-provider
target workflow and proof against the exact released artifact. Those gaps are
part of the viewer model and cannot be hidden by its static host rendering.
