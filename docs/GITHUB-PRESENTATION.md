# GitHub presentation contract

The public repository page is a product front door, not an internal operator
handoff. It must let a new visitor answer, in order:

1. What is zlOS?
2. What does it look like?
3. What works now?
4. How can I try or inspect it?
5. What is not proved or releasable yet?

## Public surfaces

| Surface | Authority |
|---|---|
| GitHub landing README | [`.github/README.md`](../.github/README.md) |
| Deep technical README | [`README.md`](../README.md) |
| Repository description and topics | GitHub repository metadata |
| Current implementation truth | [`PROJECT-STATUS.md`](PROJECT-STATUS.md) |
| 906-feature counts | [`program/FEATURE-STATUS.json`](program/FEATURE-STATUS.json) |
| Contribution entry point | [`.github/CONTRIBUTING.md`](../.github/CONTRIBUTING.md) |
| Security reporting | [`.github/SECURITY.md`](../.github/SECURITY.md) |
| Licensing boundary | [`LICENSE-STATUS.md`](../LICENSE-STATUS.md) |

GitHub gives `.github/README.md` precedence over the repository-root README.
That file therefore owns the concise visitor presentation. The root README
retains the deeper language/toolchain and kernel explanation instead of being
duplicated wholesale.

## Claim rules

- Use actual zlOS output for hero imagery; do not substitute a concept mockup.
- Keep `PROVED_CURRENT`, `PARTIAL_CURRENT`, and `PLANNED_UNPROVED` separate.
- Never turn a coherent 906-row inventory into “906 implemented features.”
- Distinguish source, host, build, QEMU, physical, and release evidence.
- Do not advertise a public download while the release/license gate is red.
- Keep exact hashes and detailed receipts in evidence documents rather than
  crowding the first screen.

## Visual direction

Use the Presswork desktop's graphite, paper, amber, and blue visual identity.
The real 1920x1200 boot capture is the hero. Avoid generic generated gradients,
decorative badge walls, and feature-card layouts that make the repository look
like an unrelated web template.
