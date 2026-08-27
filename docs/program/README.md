# zlOS complete implementation program

Date: 2026-08-22

This directory is the execution authority for turning the starred-repository
research into zlOS. It preserves the full destination rather than redefining it
as a small MVP. Delivery is staged only because later capabilities depend on
earlier ones.

It is a specification and status system, not 906 implemented features. The
current checked-in status snapshot records 906 rows: 877 planned and unproved,
22 partial, and 7 proved for their explicitly bounded contracts. Those labels
must be refreshed from evidence before they are treated as current.

The program covers:

- all 906 stable product feature atoms in the canonical research catalogue;
- every normalized firmware, platform, driver, filesystem and device target;
- every system service and cross-process protocol;
- all 61 current named zlOS implementations, the All Applications surface and
  all 24 current games;
- the selected browser, productivity, creative, communication, administration,
  development, compatibility and learning application families;
- zl language/compiler/runtime/self-hosting work;
- agent, automation, public-demo, release, recovery and support work;
- host, image, QEMU, physical-hardware, security, accessibility, visual and
  performance proof.

This is a clean-room behavior and architecture plan. External source is not
copied. A donor implementation can suggest a contract, failure case or test,
but zlOS owns its design and code.

## Read this suite

1. [`PHASES.md`](PHASES.md) — the dependency graph, delivery waves and exit
   gates.
2. [`FEATURE-MAP.md`](FEATURE-MAP.md) — generated row-for-row mapping of all 906
   feature IDs to workstream and delivery phase.
3. [`RESEARCH-CONTRACT-CROSSWALK.md`](RESEARCH-CONTRACT-CROSSWALK.md) — all
   174 implementation contracts from the three earlier backlogs mapped into the
   master phase DAG.
4. [`DRIVERS.md`](DRIVERS.md) — every normalized platform and device provider.
5. [`SERVICES.md`](SERVICES.md) — every system service and protocol boundary.
6. [`APPLICATIONS.md`](APPLICATIONS.md) — every current and destination app,
   game and user-facing workflow.
7. [`LANGUAGE-AGENTS-OPERATIONS.md`](LANGUAGE-AGENTS-OPERATIONS.md) — zl,
   developer platform, compatibility, agents, deployment and self-hosting.
8. [`PROOF-GATES.md`](PROOF-GATES.md) — the evidence contract that prevents
   source-only, false-green, QEMU-only or host-harness claims from becoming
   product completion.
9. [`VALIDATION-RECEIPT.md`](VALIDATION-RECEIPT.md) — exact coverage output,
   refutations incorporated, evidence ceiling and weakest links.
10. [`research/README.md`](research/README.md) — the retained 33-repository
    research shelf, canonical 906-row catalogue, matrices, deep dives,
    refutations, the three 174-contract source backlogs, and read-only snapshots
    of the zl language plans used by the clean-room program.

The local `AGENTS.md`, `CLAUDE.md`, `RULES.md`, `SOP.md`, `STATUS.md`, `TODO.md`
and `VALUES.md` files are the standard per-directory contributor surface. They
orient work in this folder; they do not add features or change program status.

## What each file proves

| File | What it proves | What it does not prove |
|---|---|---|
| `FEATURE-MAP.md` | Every canonical feature ID has one phase and workstream. | Implementation or runtime behavior. |
| `FEATURE-STATUS.json` | A dated evidence join for all 906 IDs. | That the join is fresh after later source or layout changes. |
| `PHASES.md` | MP-00 through MP-20 exist with dependencies and exit contracts. | That any phase has exited. |
| `DRIVERS.md`, `SERVICES.md`, `APPLICATIONS.md`, `LANGUAGE-AGENTS-OPERATIONS.md` | The normalized target registries are documented. | That listed targets are shipped. |
| `RESEARCH-CONTRACT-CROSSWALK.md` | All 174 source contracts have destinations. | That their acceptance contracts pass. |
| `PROOF-GATES.md` | The evidence vocabulary and promotion rules. | Fresh host, QEMU or physical evidence. |
| `PARTIAL-CLOSURE.md` | The dated closure plan for the then-partial batch. | A live queue or completed work. |
| `research/` | The complete source research and rejected-claim trail. | Permission to copy donor code or claim donor behavior. |

Run `python3 tools/validate_master_program.py` from the zl-linux root after any
program change. It must report exactly 906 unique feature atoms, no missing
phase/workstream, no unknown dependencies, all named current apps and games,
and all required registry sections.

## Authority order

When documents disagree, use this order:

1. current source, generated artifacts and fresh reproducible receipts;
2. [`kernel/docs/evidence/performance-architecture-implementation-2026-08-22.md`](../../kernel/docs/evidence/performance-architecture-implementation-2026-08-22.md)
   for the live locally verified implementation batch;
3. this program for destination and ordering;
4. [`docs/EXECUTION-ROADMAP.md`](../EXECUTION-ROADMAP.md) for the current
   near-term physical/performance queue;
5. the research suite and its adversarial corrections;
6. older handoffs, plans, screenshots and prose claims.

The program was first frozen against committed HEAD
`b8a00ec45ac1f9f955ba79ed63f0072540067d4e` plus a large shared uncommitted
implementation batch. Revalidate the live tree before every implementation
slice. Do not assume this date-stamped baseline remains current.

## Current baseline to preserve

The locally verified batch already includes retained client and shell surfaces,
bounded damage/occlusion, deadline-aware pacing, a bounded block cache, named
crash-consistent zlfs files, two isolated cooperative Ring-3 processes, bounded
PID IPC, virtio-net and e1000 with DHCP/DNS/TCP/TLS, persistent browser state,
and opt-in Intel blitter fallback. BIOS and native UEFI application exercises
were 32/32 in the cited receipt.

Those are migration assets, not excuses to mark the complete contracts done.
Notably still open are timer preemption, a general user window/input ABI,
capability handles and typed IPC, service/process migration, physical I219 and
Intel promotion, broad driver coverage, browser process isolation, audio,
package/app process boundaries and the in-zlOS toolchain.

## End-state architecture

```text
firmware / boot acquisition providers
                 |
                 v
bounded zlOS boot manager -> typed immutable handover -> mechanism kernel
                                                          |
                                      +-------------------+------------------+
                                      |                   |                  |
                                memory/process       capability/IPC     interrupts/time
                                      |                   |                  |
                                      +--------- supervised services --------+
                                                        |
          +--------------------+------------------------+---------------------+
          |                    |                        |                     |
     driver providers     storage/network         display/input/audio   identity/policy
          |                    |                        |                     |
          +--------------------+---------- typed portals/services -----------+
                                               |
                         +---------------------+----------------------+
                         |                     |                      |
                    shell/session       isolated applications   split browser/web
                         |                     |                      |
                         +---------- packages, SDK and zl -----------+
                                               |
                                  agents / operations / self-hosting
```

The kernel keeps mechanisms that require privilege: address spaces, scheduling,
interrupts, low-level time, handle tables, IPC transport, fault containment and
minimal device mediation. Policy, parsing, files, networking, display, input,
audio, packages, sessions and applications move to supervised processes as the
required process/IPC foundations become ready. Proven in-kernel fallbacks are
kept until replacement providers pass equivalent target gates.

## One implementation per real contract

“Every driver they have” does not mean cloning 33 versions of an AHCI driver.
It means:

- every distinct hardware or protocol family stays in the driver registry;
- one common block, network, display, input, audio, camera and sensor contract
  serves many providers;
- PCI IDs and revisions are data entries when behavior is genuinely shared;
- materially different controller generations remain separate providers;
- third-party breadth is retained as compatibility/port evidence, not falsely
  relabelled first-party zlOS support;
- source-only, stub, disabled, unreachable and mock features become negative
  tests or deferred targets, never completion evidence.

## Completion semantics

A feature is complete only when the outcome, owner, protocol, authority,
resource limits, lifecycle, failure behavior, persistence, accessibility,
performance and evidence agree. Each implementation slice follows:

```text
intent -> versioned contract -> bounded implementation -> deterministic checks
       -> image/QEMU gate -> required physical gate -> independent refutation
       -> receipt and rollback path
```

No phase may delete a proven fallback before its replacement passes the same or
stronger evidence lane. No later phase can be called complete merely because a
source file, package recipe, menu entry, screenshot or build target exists.

## Relationship to the research shelf

The complete evidence shelf is retained locally under [`research/`](research/),
so a contributor can audit the program without another checkout. The original
working copy may also exist in `zl-merge-all`, but it is no longer required by
the validator. The front doors are:

- [`CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md`](research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md) — 906 product
  atoms and exact current-app/game crosswalk;
- [`CLEAN_ROOM_ZL_ZLOS_INTEGRATION_PLAN_2026-08-21.md`](research/CLEAN_ROOM_ZL_ZLOS_INTEGRATION_PLAN_2026-08-21.md) — architecture;
- [`IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md`](research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md) — 64 core contracts;
- [`DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md`](research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md) — 56
  driver/service/app contracts;
- [`VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md`](research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md) — 54
  visual/browser/app contracts;
- the three driver/app and three visual/browser deep dives plus all refutation
  reports — exact source evidence and rejected claims;
- [`RESPONSIVENESS_RENDERING_AND_VISUAL_POLISH_DEEP_DIVE_2026-08-22.md`](research/RESPONSIVENESS_RENDERING_AND_VISUAL_POLISH_DEEP_DIVE_2026-08-22.md) —
  fast-path and visual-coherence mechanics.

This directory is the implementation projection of that shelf. It does not
erase the source trails or their evidence ceilings.
