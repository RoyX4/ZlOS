# MP-08: Make networking reliable and explicitly authorized

Status: full-scope planning proposal, 2026-09-08. Nothing on this page is an implementation-completion claim.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Decisions](FULL-ROADMAP-DECISIONS.md) · [Machine-readable steps](FULL-SYSTEM-ROADMAP.json)

Owner scope to inspect: `kernel/src/net/; kernel/src/drivers/network/; kernel/src/web/`. Paths identify current areas, not invented future files.

## Bounded handoff and full completion

`H-08` exports: Socket/resolver/network policy and a tested virtual provider; physical provider claims remain separate.

The handoff enables only its named subset. `CLOSE-08` requires every complete feature, target and source contract below. Prose dependencies must be bound to concrete providers before implementation. Shared work is implemented once and checked against each consuming contract.

## Ordered subsystem milestones

### M-08.01 — Preserve the current virtual-network/browser path and freeze packet, socket, resolver and provider contracts

Preserve the current virtual-network/browser path and freeze packet, socket, resolver and provider contracts.

**Requires:** `D-01`, `D-02`, `D-10`, `D-15`, `H-00`, `H-01`, `H-04`, `H-05`, `H-06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-08.02 — Implement NIC ownership, RX/TX ring bounds, buffers, interrupts, reset and removal for each selected provider

Implement NIC ownership, RX/TX ring bounds, buffers, interrupts, reset and removal for each selected provider.

**Requires:** `M-08.01`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-08.03 — Complete Ethernet/ARP/NDP, IPv4/IPv6, addressing, routes, ICMP, UDP and TCP state/reliability behavior

Complete Ethernet/ARP/NDP, IPv4/IPv6, addressing, routes, ICMP, UDP and TCP state/reliability behavior.

**Requires:** `M-08.02`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-08.04 — Add deterministic packet loss, duplication, reordering, fragmentation, zero-window and reconnect fixtures

Add deterministic packet loss, duplication, reordering, fragmentation, zero-window and reconnect fixtures.

**Requires:** `M-08.03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-08.05 — Resolve TLS/X.509 chain, name, time, key usage, critical-extension and trust-store policy

Resolve TLS/X.509 chain, name, time, key usage, critical-extension and trust-store policy; do not treat recognized OIDs as enforced semantics.

**Requires:** `M-08.04`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### H-08 — Bounded development handoff: Make networking reliable and explicitly authorized

Socket/resolver/network policy and a tested virtual provider; physical provider claims remain separate.

**Requires:** `M-08.05`.

**Acceptance:** Name the exact exported subset, version, producer/consumer, bounds, failure/cleanup and passing proof. This does not mark the phase or its feature list complete.

### M-08.06 — Join socket, resolver, firewall, VPN, downloads and remote endpoints to explicit process/session authority

Join socket, resolver, firewall, VPN, downloads and remote endpoints to explicit process/session authority.

**Requires:** `M-08.05`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-08.07 — Implement offline, captive, link loss, roaming and radio lifecycle states without invented connectivity indicators

Implement offline, captive, link loss, roaming and radio lifecycle states without invented connectivity indicators.

**Requires:** `M-08.06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-08.08 — Reconcile wireless-plan.md with exact available hardware and firmware licenses

Reconcile wireless-plan.md with exact available hardware and firmware licenses; qualify Wi-Fi and Bluetooth profiles independently.

**Requires:** `M-08.07`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-08.09 — Prove physical carrier/reset/traffic/teardown on each claimed device and retain unsupported/degraded states for the rest

Prove physical carrier/reset/traffic/teardown on each claimed device and retain unsupported/degraded states for the rest.

**Requires:** `M-08.08`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

## Package index

| ID | Kind | Required outcome |
|---|---|---|
| [F-NW-001](#f-nw-001) | feature | NIC service contract |
| [F-NW-002](#f-nw-002) | feature | Ethernet |
| [F-NW-003](#f-nw-003) | feature | ARP |
| [F-NW-004](#f-nw-004) | feature | IPv4 |
| [F-NW-005](#f-nw-005) | feature | IPv6 |
| [F-NW-006](#f-nw-006) | feature | ICMP/ICMPv6 |
| [F-NW-007](#f-nw-007) | feature | UDP |
| [F-NW-008](#f-nw-008) | feature | TCP |
| [F-NW-009](#f-nw-009) | feature | DHCPv4 |
| [F-NW-010](#f-nw-010) | feature | DHCPv6/SLAAC |
| [F-NW-011](#f-nw-011) | feature | DNS resolver |
| [F-NW-012](#f-nw-012) | feature | socket API |
| [F-NW-013](#f-nw-013) | feature | network namespaces |
| [F-NW-014](#f-nw-014) | feature | route table |
| [F-NW-015](#f-nw-015) | feature | loopback |
| [F-NW-016](#f-nw-016) | feature | TLS |
| [F-NW-017](#f-nw-017) | feature | HTTP/1.1 |
| [F-NW-018](#f-nw-018) | feature | HTTP/2 |
| [F-NW-019](#f-nw-019) | feature | HTTP/3/QUIC |
| [F-NW-020](#f-nw-020) | feature | WebSocket |
| [F-NW-021](#f-nw-021) | feature | proxy support |
| [F-NW-022](#f-nw-022) | feature | firewall |
| [F-NW-023](#f-nw-023) | feature | network time |
| [F-NW-024](#f-nw-024) | feature | mDNS/service discovery |
| [F-NW-025](#f-nw-025) | feature | SSH client/server |
| [F-NW-026](#f-nw-026) | feature | remote desktop |
| [F-NW-027](#f-nw-027) | feature | captive-portal flow |
| [F-NW-028](#f-nw-028) | feature | Wi-Fi management |
| [F-NW-029](#f-nw-029) | feature | VPN management |
| [F-NW-030](#f-nw-030) | feature | connectivity diagnostics |
| [F-NW-031](#f-nw-031) | feature | per-app network permission |
| [F-NW-032](#f-nw-032) | feature | traffic accounting |
| [F-NW-033](#f-nw-033) | feature | egress policy for demos/agents |
| [F-NW-034](#f-nw-034) | feature | offline mode |
| [F-NW-035](#f-nw-035) | feature | network recovery |
| [C-P5.5](#c-p5-5) | contract | Network/Resolver/TLS service APIs |
| [C-P6.1](#c-p6-1) | contract | NIC provider selection and resource contract |
| [C-P6.2](#c-p6-2) | contract | DHCP/DNS/TCP reliability on physical link |
| [C-P6.3](#c-p6-3) | contract | TLS/request boundary |
| [C-DA-13](#c-da-13) | contract | NIC provider and packet simulator |
| [C-DA-22](#c-da-22) | contract | socket, resolver and network policy services |
| [T-USB-010](#t-usb-010) | target | CDC-ECM network |
| [T-USB-011](#t-usb-011) | target | RNDIS network |
| [T-NIC-001](#t-nic-001) | target | loopback |
| [T-NIC-002](#t-nic-002) | target | virtio-net |
| [T-NIC-003](#t-nic-003) | target | Intel 8254x e1000 |
| [T-NIC-004](#t-nic-004) | target | Intel e1000e/I219 |
| [T-NIC-005](#t-nic-005) | target | Intel igb |
| [T-NIC-006](#t-nic-006) | target | Intel igc/I225/I226 |
| [T-NIC-007](#t-nic-007) | target | Realtek RTL8139 |
| [T-NIC-008](#t-nic-008) | target | Realtek RTL8168/8169 |
| [T-NIC-011](#t-nic-011) | target | USB CDC-ECM adapter |
| [T-NIC-012](#t-nic-012) | target | USB RNDIS adapter |
| [T-NIC-013](#t-nic-013) | target | USB ASIX Ethernet |
| [T-NIC-014](#t-nic-014) | target | USB Realtek Ethernet |
| [T-SVC-050](#t-svc-050) | target | Network Device Broker |
| [T-SVC-051](#t-svc-051) | target | Network Stack Service |
| [T-SVC-052](#t-svc-052) | target | Address Configuration |
| [T-SVC-053](#t-svc-053) | target | Resolver Service |
| [T-SVC-054](#t-svc-054) | target | TLS/Certificate Service |
| [T-SVC-055](#t-svc-055) | target | HTTP/Transfer Service |
| [T-SVC-056](#t-svc-056) | target | Routing/Firewall Service |
| [T-SVC-059](#t-svc-059) | target | Connectivity/Captive Portal Service |
| [T-SVC-062](#t-svc-062) | target | Time Synchronization Service |
| [T-SVC-063](#t-svc-063) | target | Network Diagnostics Service |

<a id="f-nw-001"></a>
## F-NW-001 — NIC service contract

**Original requirement:** provider-neutral bounded packet buffers, link/MTU/features/reset/counters

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-001.01 — Reconcile existing NIC service contract**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for NIC service contract. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: provider-neutral bounded packet buffers, link/MTU/features/reset/counters
- [ ] **F-NW-001.02 — Freeze the exact contract for NIC service contract**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: provider-neutral bounded packet buffers, link/MTU/features/reset/counters. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-001.03 — Implement/prove: provider-neutral bounded packet buffers**
  - Action: For NIC service contract, implement or reuse and verify this exact obligation: provider-neutral bounded packet buffers. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for provider-neutral bounded packet buffers; retain observable state/resource expectations.
- [ ] **F-NW-001.04 — Implement/prove: link/MTU/features/reset/counters**
  - Action: For NIC service contract, implement or reuse and verify this exact obligation: link/MTU/features/reset/counters. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-001.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for link/MTU/features/reset/counters; retain observable state/resource expectations.
- [ ] **F-NW-001.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to NIC service contract: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-001.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-001.06 — Integrate into the real consumer and runtime route**
  - Action: Wire NIC service contract into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-001.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-001.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for NIC service contract as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-001.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-002"></a>
## F-NW-002 — Ethernet

**Original requirement:** checked frame lengths/types, immutable packet ownership and bounded demux

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-002.01 — Reconcile existing Ethernet**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for Ethernet. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: checked frame lengths/types, immutable packet ownership and bounded demux
- [ ] **F-NW-002.02 — Freeze the exact contract for Ethernet**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: checked frame lengths/types, immutable packet ownership and bounded demux. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-002.03 — Implement/prove: checked frame lengths/types**
  - Action: For Ethernet, implement or reuse and verify this exact obligation: checked frame lengths/types. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for checked frame lengths/types; retain observable state/resource expectations.
- [ ] **F-NW-002.04 — Implement/prove: immutable packet ownership and bounded demux**
  - Action: For Ethernet, implement or reuse and verify this exact obligation: immutable packet ownership and bounded demux. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-002.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for immutable packet ownership and bounded demux; retain observable state/resource expectations.
- [ ] **F-NW-002.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to Ethernet: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-002.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-002.06 — Integrate into the real consumer and runtime route**
  - Action: Wire Ethernet into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-002.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-002.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for Ethernet as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-002.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-003"></a>
## F-NW-003 — ARP

**Original requirement:** bounded neighbor states, retry/age/spoof policy and conflict handling

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-003.01 — Reconcile existing ARP**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for ARP. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bounded neighbor states, retry/age/spoof policy and conflict handling
- [ ] **F-NW-003.02 — Freeze the exact contract for ARP**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bounded neighbor states, retry/age/spoof policy and conflict handling. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-003.03 — Implement/prove: bounded neighbor states**
  - Action: For ARP, implement or reuse and verify this exact obligation: bounded neighbor states. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded neighbor states; retain observable state/resource expectations.
- [ ] **F-NW-003.04 — Implement/prove: retry/age/spoof policy and conflict handling**
  - Action: For ARP, implement or reuse and verify this exact obligation: retry/age/spoof policy and conflict handling. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-003.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for retry/age/spoof policy and conflict handling; retain observable state/resource expectations.
- [ ] **F-NW-003.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to ARP: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-003.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-003.06 — Integrate into the real consumer and runtime route**
  - Action: Wire ARP into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-003.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-003.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for ARP as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-003.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-004"></a>
## F-NW-004 — IPv4

**Original requirement:** header/options/checksum/fragment/reassembly/routing and hostile corpus

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-004.01 — Reconcile existing IPv4**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for IPv4. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: header/options/checksum/fragment/reassembly/routing and hostile corpus
- [ ] **F-NW-004.02 — Freeze the exact contract for IPv4**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: header/options/checksum/fragment/reassembly/routing and hostile corpus. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-004.03 — Implement/prove: header/options/checksum/fragment/reassembly/routing and hostile corpus**
  - Action: For IPv4, implement or reuse and verify this exact obligation: header/options/checksum/fragment/reassembly/routing and hostile corpus. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for header/options/checksum/fragment/reassembly/routing and hostile corpus; retain observable state/resource expectations.
- [ ] **F-NW-004.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to IPv4: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-004.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-004.05 — Integrate into the real consumer and runtime route**
  - Action: Wire IPv4 into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-004.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-004.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for IPv4 as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-004.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-005"></a>
## F-NW-005 — IPv6

**Original requirement:** NDP, SLAAC/DHCPv6, routes, extension headers, reassembly and dual-stack policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-005.01 — Reconcile existing IPv6**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for IPv6. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: NDP, SLAAC/DHCPv6, routes, extension headers, reassembly and dual-stack policy
- [ ] **F-NW-005.02 — Freeze the exact contract for IPv6**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: NDP, SLAAC/DHCPv6, routes, extension headers, reassembly and dual-stack policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-005.03 — Implement/prove: NDP**
  - Action: For IPv6, implement or reuse and verify this exact obligation: NDP. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for NDP; retain observable state/resource expectations.
- [ ] **F-NW-005.04 — Implement/prove: SLAAC/DHCPv6**
  - Action: For IPv6, implement or reuse and verify this exact obligation: SLAAC/DHCPv6. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-005.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for SLAAC/DHCPv6; retain observable state/resource expectations.
- [ ] **F-NW-005.05 — Implement/prove: routes**
  - Action: For IPv6, implement or reuse and verify this exact obligation: routes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-005.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for routes; retain observable state/resource expectations.
- [ ] **F-NW-005.06 — Implement/prove: extension headers**
  - Action: For IPv6, implement or reuse and verify this exact obligation: extension headers. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-005.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for extension headers; retain observable state/resource expectations.
- [ ] **F-NW-005.07 — Implement/prove: reassembly and dual-stack policy**
  - Action: For IPv6, implement or reuse and verify this exact obligation: reassembly and dual-stack policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-005.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reassembly and dual-stack policy; retain observable state/resource expectations.
- [ ] **F-NW-005.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to IPv6: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-005.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-005.09 — Integrate into the real consumer and runtime route**
  - Action: Wire IPv6 into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-005.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-005.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for IPv6 as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-005.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-006"></a>
## F-NW-006 — ICMP/ICMPv6

**Original requirement:** real echo/error/PMTU path with checksums, IDs, timeouts and socket feedback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-006.01 — Reconcile existing ICMP/ICMPv6**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for ICMP/ICMPv6. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: real echo/error/PMTU path with checksums, IDs, timeouts and socket feedback
- [ ] **F-NW-006.02 — Freeze the exact contract for ICMP/ICMPv6**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: real echo/error/PMTU path with checksums, IDs, timeouts and socket feedback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-006.03 — Implement/prove: real echo/error/PMTU path with checksums**
  - Action: For ICMP/ICMPv6, implement or reuse and verify this exact obligation: real echo/error/PMTU path with checksums. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for real echo/error/PMTU path with checksums; retain observable state/resource expectations.
- [ ] **F-NW-006.04 — Implement/prove: IDs**
  - Action: For ICMP/ICMPv6, implement or reuse and verify this exact obligation: IDs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-006.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for IDs; retain observable state/resource expectations.
- [ ] **F-NW-006.05 — Implement/prove: timeouts and socket feedback**
  - Action: For ICMP/ICMPv6, implement or reuse and verify this exact obligation: timeouts and socket feedback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-006.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for timeouts and socket feedback; retain observable state/resource expectations.
- [ ] **F-NW-006.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to ICMP/ICMPv6: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-006.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-006.07 — Integrate into the real consumer and runtime route**
  - Action: Wire ICMP/ICMPv6 into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-006.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-006.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for ICMP/ICMPv6 as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-006.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-007"></a>
## F-NW-007 — UDP

**Original requirement:** bounded datagrams, bind/demux/source metadata/truncation/checksum/exhaustion

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-007.01 — Reconcile existing UDP**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for UDP. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bounded datagrams, bind/demux/source metadata/truncation/checksum/exhaustion
- [ ] **F-NW-007.02 — Freeze the exact contract for UDP**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bounded datagrams, bind/demux/source metadata/truncation/checksum/exhaustion. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-007.03 — Implement/prove: bounded datagrams**
  - Action: For UDP, implement or reuse and verify this exact obligation: bounded datagrams. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded datagrams; retain observable state/resource expectations.
- [ ] **F-NW-007.04 — Implement/prove: bind/demux/source metadata/truncation/checksum/exhaustion**
  - Action: For UDP, implement or reuse and verify this exact obligation: bind/demux/source metadata/truncation/checksum/exhaustion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-007.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bind/demux/source metadata/truncation/checksum/exhaustion; retain observable state/resource expectations.
- [ ] **F-NW-007.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to UDP: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-007.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-007.06 — Integrate into the real consumer and runtime route**
  - Action: Wire UDP into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-007.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-007.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for UDP as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-007.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-008"></a>
## F-NW-008 — TCP

**Original requirement:** complete state machine, windows, retransmit/congestion, loss/reorder/wrap/reset tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-008.01 — Reconcile existing TCP**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for TCP. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: complete state machine, windows, retransmit/congestion, loss/reorder/wrap/reset tests
- [ ] **F-NW-008.02 — Freeze the exact contract for TCP**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: complete state machine, windows, retransmit/congestion, loss/reorder/wrap/reset tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-008.03 — Implement/prove: complete state machine**
  - Action: For TCP, implement or reuse and verify this exact obligation: complete state machine. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for complete state machine; retain observable state/resource expectations.
- [ ] **F-NW-008.04 — Implement/prove: windows**
  - Action: For TCP, implement or reuse and verify this exact obligation: windows. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-008.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for windows; retain observable state/resource expectations.
- [ ] **F-NW-008.05 — Implement/prove: retransmit/congestion**
  - Action: For TCP, implement or reuse and verify this exact obligation: retransmit/congestion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-008.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for retransmit/congestion; retain observable state/resource expectations.
- [ ] **F-NW-008.06 — Implement/prove: loss/reorder/wrap/reset tests**
  - Action: For TCP, implement or reuse and verify this exact obligation: loss/reorder/wrap/reset tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-008.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for loss/reorder/wrap/reset tests; retain observable state/resource expectations.
- [ ] **F-NW-008.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to TCP: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-008.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-008.08 — Integrate into the real consumer and runtime route**
  - Action: Wire TCP into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-008.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-008.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for TCP as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-008.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-009"></a>
## F-NW-009 — DHCPv4

**Original requirement:** async discover/offer/request/lease/renew/rebind/expiry and hostile server handling

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-009.01 — Reconcile existing DHCPv4**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for DHCPv4. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: async discover/offer/request/lease/renew/rebind/expiry and hostile server handling
- [ ] **F-NW-009.02 — Freeze the exact contract for DHCPv4**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: async discover/offer/request/lease/renew/rebind/expiry and hostile server handling. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-009.03 — Implement/prove: async discover/offer/request/lease/renew/rebind/expiry and hostile server handling**
  - Action: For DHCPv4, implement or reuse and verify this exact obligation: async discover/offer/request/lease/renew/rebind/expiry and hostile server handling. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for async discover/offer/request/lease/renew/rebind/expiry and hostile server handling; retain observable state/resource expectations.
- [ ] **F-NW-009.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to DHCPv4: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-009.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-009.05 — Integrate into the real consumer and runtime route**
  - Action: Wire DHCPv4 into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-009.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-009.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for DHCPv4 as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-009.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-010"></a>
## F-NW-010 — DHCPv6/SLAAC

**Original requirement:** address/prefix/router/DNS lifecycles and privacy policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-010.01 — Reconcile existing DHCPv6/SLAAC**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for DHCPv6/SLAAC. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: address/prefix/router/DNS lifecycles and privacy policy
- [ ] **F-NW-010.02 — Freeze the exact contract for DHCPv6/SLAAC**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: address/prefix/router/DNS lifecycles and privacy policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-010.03 — Implement/prove: address/prefix/router/DNS lifecycles and privacy policy**
  - Action: For DHCPv6/SLAAC, implement or reuse and verify this exact obligation: address/prefix/router/DNS lifecycles and privacy policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for address/prefix/router/DNS lifecycles and privacy policy; retain observable state/resource expectations.
- [ ] **F-NW-010.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to DHCPv6/SLAAC: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-010.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-010.05 — Integrate into the real consumer and runtime route**
  - Action: Wire DHCPv6/SLAAC into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-010.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-010.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for DHCPv6/SLAAC as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-010.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-011"></a>
## F-NW-011 — DNS resolver

**Original requirement:** restricted service, cache/negative TTL, compression bounds, UDP/TCP fallback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-011.01 — Reconcile existing DNS resolver**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for DNS resolver. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: restricted service, cache/negative TTL, compression bounds, UDP/TCP fallback
- [ ] **F-NW-011.02 — Freeze the exact contract for DNS resolver**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: restricted service, cache/negative TTL, compression bounds, UDP/TCP fallback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-011.03 — Implement/prove: restricted service**
  - Action: For DNS resolver, implement or reuse and verify this exact obligation: restricted service. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for restricted service; retain observable state/resource expectations.
- [ ] **F-NW-011.04 — Implement/prove: cache/negative TTL**
  - Action: For DNS resolver, implement or reuse and verify this exact obligation: cache/negative TTL. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-011.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cache/negative TTL; retain observable state/resource expectations.
- [ ] **F-NW-011.05 — Implement/prove: compression bounds**
  - Action: For DNS resolver, implement or reuse and verify this exact obligation: compression bounds. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-011.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for compression bounds; retain observable state/resource expectations.
- [ ] **F-NW-011.06 — Implement/prove: UDP/TCP fallback**
  - Action: For DNS resolver, implement or reuse and verify this exact obligation: UDP/TCP fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-011.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for UDP/TCP fallback; retain observable state/resource expectations.
- [ ] **F-NW-011.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to DNS resolver: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-011.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-011.08 — Integrate into the real consumer and runtime route**
  - Action: Wire DNS resolver into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-011.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-011.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for DNS resolver as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-011.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-012"></a>
## F-NW-012 — socket API

**Original requirement:** handle-based bind/connect/listen/accept/send/recv/poll with deadlines/cancel

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-012.01 — Reconcile existing socket API**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for socket API. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: handle-based bind/connect/listen/accept/send/recv/poll with deadlines/cancel
- [ ] **F-NW-012.02 — Freeze the exact contract for socket API**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: handle-based bind/connect/listen/accept/send/recv/poll with deadlines/cancel. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-012.03 — Implement/prove: handle-based bind/connect/listen/accept/send/recv/poll with deadlines/cancel**
  - Action: For socket API, implement or reuse and verify this exact obligation: handle-based bind/connect/listen/accept/send/recv/poll with deadlines/cancel. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for handle-based bind/connect/listen/accept/send/recv/poll with deadlines/cancel; retain observable state/resource expectations.
- [ ] **F-NW-012.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to socket API: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-012.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-012.05 — Integrate into the real consumer and runtime route**
  - Action: Wire socket API into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-012.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-012.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for socket API as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-012.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-013"></a>
## F-NW-013 — network namespaces

**Original requirement:** interface/route/socket/firewall/DNS isolation and explicit joining

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-013.01 — Reconcile existing network namespaces**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for network namespaces. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: interface/route/socket/firewall/DNS isolation and explicit joining
- [ ] **F-NW-013.02 — Freeze the exact contract for network namespaces**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: interface/route/socket/firewall/DNS isolation and explicit joining. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-013.03 — Implement/prove: interface/route/socket/firewall/DNS isolation and explicit joining**
  - Action: For network namespaces, implement or reuse and verify this exact obligation: interface/route/socket/firewall/DNS isolation and explicit joining. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for interface/route/socket/firewall/DNS isolation and explicit joining; retain observable state/resource expectations.
- [ ] **F-NW-013.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to network namespaces: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-013.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-013.05 — Integrate into the real consumer and runtime route**
  - Action: Wire network namespaces into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-013.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-013.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for network namespaces as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-013.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-014"></a>
## F-NW-014 — route table

**Original requirement:** longest-prefix routing, metrics, source selection, changes and diagnostics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-014.01 — Reconcile existing route table**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for route table. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: longest-prefix routing, metrics, source selection, changes and diagnostics
- [ ] **F-NW-014.02 — Freeze the exact contract for route table**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: longest-prefix routing, metrics, source selection, changes and diagnostics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-014.03 — Implement/prove: longest-prefix routing**
  - Action: For route table, implement or reuse and verify this exact obligation: longest-prefix routing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for longest-prefix routing; retain observable state/resource expectations.
- [ ] **F-NW-014.04 — Implement/prove: metrics**
  - Action: For route table, implement or reuse and verify this exact obligation: metrics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-014.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for metrics; retain observable state/resource expectations.
- [ ] **F-NW-014.05 — Implement/prove: source selection**
  - Action: For route table, implement or reuse and verify this exact obligation: source selection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-014.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for source selection; retain observable state/resource expectations.
- [ ] **F-NW-014.06 — Implement/prove: changes and diagnostics**
  - Action: For route table, implement or reuse and verify this exact obligation: changes and diagnostics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-014.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for changes and diagnostics; retain observable state/resource expectations.
- [ ] **F-NW-014.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to route table: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-014.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-014.08 — Integrate into the real consumer and runtime route**
  - Action: Wire route table into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-014.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-014.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for route table as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-014.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-015"></a>
## F-NW-015 — loopback

**Original requirement:** deterministic local service/testing path with same socket semantics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-015.01 — Reconcile existing loopback**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for loopback. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: deterministic local service/testing path with same socket semantics
- [ ] **F-NW-015.02 — Freeze the exact contract for loopback**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: deterministic local service/testing path with same socket semantics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-015.03 — Implement/prove: deterministic local service/testing path with same socket semantics**
  - Action: For loopback, implement or reuse and verify this exact obligation: deterministic local service/testing path with same socket semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deterministic local service/testing path with same socket semantics; retain observable state/resource expectations.
- [ ] **F-NW-015.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to loopback: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-015.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-015.05 — Integrate into the real consumer and runtime route**
  - Action: Wire loopback into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-015.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-015.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for loopback as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-015.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-016"></a>
## F-NW-016 — TLS

**Original requirement:** restricted user-space implementation, trust/time/hostname/revocation and hostile corpus

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-016.01 — Reconcile existing TLS**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for TLS. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: restricted user-space implementation, trust/time/hostname/revocation and hostile corpus
- [ ] **F-NW-016.02 — Freeze the exact contract for TLS**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: restricted user-space implementation, trust/time/hostname/revocation and hostile corpus. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-016.03 — Implement/prove: restricted user-space implementation**
  - Action: For TLS, implement or reuse and verify this exact obligation: restricted user-space implementation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for restricted user-space implementation; retain observable state/resource expectations.
- [ ] **F-NW-016.04 — Implement/prove: trust/time/hostname/revocation and hostile corpus**
  - Action: For TLS, implement or reuse and verify this exact obligation: trust/time/hostname/revocation and hostile corpus. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-016.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for trust/time/hostname/revocation and hostile corpus; retain observable state/resource expectations.
- [ ] **F-NW-016.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to TLS: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-016.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-016.06 — Integrate into the real consumer and runtime route**
  - Action: Wire TLS into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-016.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-016.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for TLS as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-016.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-017"></a>
## F-NW-017 — HTTP/1.1

**Original requirement:** bounded parser/body/redirect/deadline/cancel/connection reuse and truthful errors

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-017.01 — Reconcile existing HTTP/1.1**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for HTTP/1.1. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bounded parser/body/redirect/deadline/cancel/connection reuse and truthful errors
- [ ] **F-NW-017.02 — Freeze the exact contract for HTTP/1.1**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bounded parser/body/redirect/deadline/cancel/connection reuse and truthful errors. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-017.03 — Implement/prove: bounded parser/body/redirect/deadline/cancel/connection reuse and truthful errors**
  - Action: For HTTP/1.1, implement or reuse and verify this exact obligation: bounded parser/body/redirect/deadline/cancel/connection reuse and truthful errors. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded parser/body/redirect/deadline/cancel/connection reuse and truthful errors; retain observable state/resource expectations.
- [ ] **F-NW-017.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to HTTP/1.1: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-017.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-017.05 — Integrate into the real consumer and runtime route**
  - Action: Wire HTTP/1.1 into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-017.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-017.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for HTTP/1.1 as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-017.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-018"></a>
## F-NW-018 — HTTP/2

**Original requirement:** framed multiplexing, flow control, HPACK bounds, stream cancellation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-018.01 — Reconcile existing HTTP/2**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for HTTP/2. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: framed multiplexing, flow control, HPACK bounds, stream cancellation
- [ ] **F-NW-018.02 — Freeze the exact contract for HTTP/2**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: framed multiplexing, flow control, HPACK bounds, stream cancellation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-018.03 — Implement/prove: framed multiplexing**
  - Action: For HTTP/2, implement or reuse and verify this exact obligation: framed multiplexing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for framed multiplexing; retain observable state/resource expectations.
- [ ] **F-NW-018.04 — Implement/prove: flow control**
  - Action: For HTTP/2, implement or reuse and verify this exact obligation: flow control. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-018.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for flow control; retain observable state/resource expectations.
- [ ] **F-NW-018.05 — Implement/prove: HPACK bounds**
  - Action: For HTTP/2, implement or reuse and verify this exact obligation: HPACK bounds. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-018.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for HPACK bounds; retain observable state/resource expectations.
- [ ] **F-NW-018.06 — Implement/prove: stream cancellation**
  - Action: For HTTP/2, implement or reuse and verify this exact obligation: stream cancellation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-018.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stream cancellation; retain observable state/resource expectations.
- [ ] **F-NW-018.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to HTTP/2: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-018.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-018.08 — Integrate into the real consumer and runtime route**
  - Action: Wire HTTP/2 into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-018.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-018.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for HTTP/2 as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-018.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-019"></a>
## F-NW-019 — HTTP/3/QUIC

**Original requirement:** authenticated transport, congestion, migration, stream budgets and fallback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-019.01 — Reconcile existing HTTP/3/QUIC**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for HTTP/3/QUIC. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: authenticated transport, congestion, migration, stream budgets and fallback
- [ ] **F-NW-019.02 — Freeze the exact contract for HTTP/3/QUIC**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: authenticated transport, congestion, migration, stream budgets and fallback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-019.03 — Implement/prove: authenticated transport**
  - Action: For HTTP/3/QUIC, implement or reuse and verify this exact obligation: authenticated transport. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for authenticated transport; retain observable state/resource expectations.
- [ ] **F-NW-019.04 — Implement/prove: congestion**
  - Action: For HTTP/3/QUIC, implement or reuse and verify this exact obligation: congestion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-019.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for congestion; retain observable state/resource expectations.
- [ ] **F-NW-019.05 — Implement/prove: migration**
  - Action: For HTTP/3/QUIC, implement or reuse and verify this exact obligation: migration. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-019.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for migration; retain observable state/resource expectations.
- [ ] **F-NW-019.06 — Implement/prove: stream budgets and fallback**
  - Action: For HTTP/3/QUIC, implement or reuse and verify this exact obligation: stream budgets and fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-019.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stream budgets and fallback; retain observable state/resource expectations.
- [ ] **F-NW-019.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to HTTP/3/QUIC: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-019.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-019.08 — Integrate into the real consumer and runtime route**
  - Action: Wire HTTP/3/QUIC into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-019.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-019.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for HTTP/3/QUIC as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-019.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-020"></a>
## F-NW-020 — WebSocket

**Original requirement:** authenticated upgrade, bounded frames/messages, ping/close and backpressure

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-020.01 — Reconcile existing WebSocket**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for WebSocket. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: authenticated upgrade, bounded frames/messages, ping/close and backpressure
- [ ] **F-NW-020.02 — Freeze the exact contract for WebSocket**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: authenticated upgrade, bounded frames/messages, ping/close and backpressure. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-020.03 — Implement/prove: authenticated upgrade**
  - Action: For WebSocket, implement or reuse and verify this exact obligation: authenticated upgrade. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for authenticated upgrade; retain observable state/resource expectations.
- [ ] **F-NW-020.04 — Implement/prove: bounded frames/messages**
  - Action: For WebSocket, implement or reuse and verify this exact obligation: bounded frames/messages. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-020.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded frames/messages; retain observable state/resource expectations.
- [ ] **F-NW-020.05 — Implement/prove: ping/close and backpressure**
  - Action: For WebSocket, implement or reuse and verify this exact obligation: ping/close and backpressure. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-020.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ping/close and backpressure; retain observable state/resource expectations.
- [ ] **F-NW-020.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to WebSocket: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-020.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-020.07 — Integrate into the real consumer and runtime route**
  - Action: Wire WebSocket into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-020.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-020.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for WebSocket as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-020.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-021"></a>
## F-NW-021 — proxy support

**Original requirement:** explicit HTTP/SOCKS/PAC policy, credentials, bypass and privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-021.01 — Reconcile existing proxy support**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for proxy support. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit HTTP/SOCKS/PAC policy, credentials, bypass and privacy
- [ ] **F-NW-021.02 — Freeze the exact contract for proxy support**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit HTTP/SOCKS/PAC policy, credentials, bypass and privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-021.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-021.03 — Implement/prove: explicit HTTP/SOCKS/PAC policy**
  - Action: For proxy support, implement or reuse and verify this exact obligation: explicit HTTP/SOCKS/PAC policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-021.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit HTTP/SOCKS/PAC policy; retain observable state/resource expectations.
- [ ] **F-NW-021.04 — Implement/prove: credentials**
  - Action: For proxy support, implement or reuse and verify this exact obligation: credentials. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-021.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for credentials; retain observable state/resource expectations.
- [ ] **F-NW-021.05 — Implement/prove: bypass and privacy**
  - Action: For proxy support, implement or reuse and verify this exact obligation: bypass and privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-021.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bypass and privacy; retain observable state/resource expectations.
- [ ] **F-NW-021.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to proxy support: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-021.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-021.07 — Integrate into the real consumer and runtime route**
  - Action: Wire proxy support into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-021.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-021.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for proxy support as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-021.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-022"></a>
## F-NW-022 — firewall

**Original requirement:** default policy, per-app/service rules, state, logs and user/admin UI

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-022.01 — Reconcile existing firewall**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for firewall. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: default policy, per-app/service rules, state, logs and user/admin UI
- [ ] **F-NW-022.02 — Freeze the exact contract for firewall**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: default policy, per-app/service rules, state, logs and user/admin UI. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-022.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-022.03 — Implement/prove: default policy**
  - Action: For firewall, implement or reuse and verify this exact obligation: default policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-022.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for default policy; retain observable state/resource expectations.
- [ ] **F-NW-022.04 — Implement/prove: per-app/service rules**
  - Action: For firewall, implement or reuse and verify this exact obligation: per-app/service rules. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-022.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-app/service rules; retain observable state/resource expectations.
- [ ] **F-NW-022.05 — Implement/prove: state**
  - Action: For firewall, implement or reuse and verify this exact obligation: state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-022.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for state; retain observable state/resource expectations.
- [ ] **F-NW-022.06 — Implement/prove: logs and user/admin UI**
  - Action: For firewall, implement or reuse and verify this exact obligation: logs and user/admin UI. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-022.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for logs and user/admin UI; retain observable state/resource expectations.
- [ ] **F-NW-022.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to firewall: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-022.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-022.08 — Integrate into the real consumer and runtime route**
  - Action: Wire firewall into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-022.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-022.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for firewall as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-022.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-023"></a>
## F-NW-023 — network time

**Original requirement:** authenticated/bounded synchronization, drift/step policy and wall-clock separation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-023.01 — Reconcile existing network time**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for network time. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: authenticated/bounded synchronization, drift/step policy and wall-clock separation
- [ ] **F-NW-023.02 — Freeze the exact contract for network time**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: authenticated/bounded synchronization, drift/step policy and wall-clock separation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-023.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-023.03 — Implement/prove: authenticated/bounded synchronization**
  - Action: For network time, implement or reuse and verify this exact obligation: authenticated/bounded synchronization. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-023.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for authenticated/bounded synchronization; retain observable state/resource expectations.
- [ ] **F-NW-023.04 — Implement/prove: drift/step policy and wall-clock separation**
  - Action: For network time, implement or reuse and verify this exact obligation: drift/step policy and wall-clock separation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-023.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for drift/step policy and wall-clock separation; retain observable state/resource expectations.
- [ ] **F-NW-023.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to network time: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-023.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-023.06 — Integrate into the real consumer and runtime route**
  - Action: Wire network time into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-023.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-023.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for network time as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-023.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-024"></a>
## F-NW-024 — mDNS/service discovery

**Original requirement:** scoped discovery, names/conflicts, privacy and untrusted-record bounds

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-024.01 — Reconcile existing mDNS/service discovery**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for mDNS/service discovery. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: scoped discovery, names/conflicts, privacy and untrusted-record bounds
- [ ] **F-NW-024.02 — Freeze the exact contract for mDNS/service discovery**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: scoped discovery, names/conflicts, privacy and untrusted-record bounds. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-024.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-024.03 — Implement/prove: scoped discovery**
  - Action: For mDNS/service discovery, implement or reuse and verify this exact obligation: scoped discovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-024.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for scoped discovery; retain observable state/resource expectations.
- [ ] **F-NW-024.04 — Implement/prove: names/conflicts**
  - Action: For mDNS/service discovery, implement or reuse and verify this exact obligation: names/conflicts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-024.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for names/conflicts; retain observable state/resource expectations.
- [ ] **F-NW-024.05 — Implement/prove: privacy and untrusted-record bounds**
  - Action: For mDNS/service discovery, implement or reuse and verify this exact obligation: privacy and untrusted-record bounds. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-024.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy and untrusted-record bounds; retain observable state/resource expectations.
- [ ] **F-NW-024.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to mDNS/service discovery: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-024.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-024.07 — Integrate into the real consumer and runtime route**
  - Action: Wire mDNS/service discovery into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-024.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-024.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for mDNS/service discovery as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-024.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-025"></a>
## F-NW-025 — SSH client/server

**Original requirement:** real key exchange/auth/channel/terminal/file transfer; version banner alone is not SSH

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-025.01 — Reconcile existing SSH client/server**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for SSH client/server. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: real key exchange/auth/channel/terminal/file transfer; version banner alone is not SSH
- [ ] **F-NW-025.02 — Freeze the exact contract for SSH client/server**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: real key exchange/auth/channel/terminal/file transfer; version banner alone is not SSH. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-025.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-025.03 — Implement/prove: real key exchange/auth/channel/terminal/file transfer**
  - Action: For SSH client/server, implement or reuse and verify this exact obligation: real key exchange/auth/channel/terminal/file transfer. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-025.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for real key exchange/auth/channel/terminal/file transfer; retain observable state/resource expectations.
- [ ] **F-NW-025.04 — Implement/prove: version banner alone is not SSH**
  - Action: For SSH client/server, implement or reuse and verify this exact obligation: version banner alone is not SSH. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-025.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for version banner alone is not SSH; retain observable state/resource expectations.
- [ ] **F-NW-025.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to SSH client/server: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-025.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-025.06 — Integrate into the real consumer and runtime route**
  - Action: Wire SSH client/server into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-025.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-025.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for SSH client/server as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-025.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-026"></a>
## F-NW-026 — remote desktop

**Original requirement:** authenticated encrypted session, resize/input/clipboard, reconnect and privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-026.01 — Reconcile existing remote desktop**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for remote desktop. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: authenticated encrypted session, resize/input/clipboard, reconnect and privacy
- [ ] **F-NW-026.02 — Freeze the exact contract for remote desktop**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: authenticated encrypted session, resize/input/clipboard, reconnect and privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-026.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-026.03 — Implement/prove: authenticated encrypted session**
  - Action: For remote desktop, implement or reuse and verify this exact obligation: authenticated encrypted session. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-026.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for authenticated encrypted session; retain observable state/resource expectations.
- [ ] **F-NW-026.04 — Implement/prove: resize/input/clipboard**
  - Action: For remote desktop, implement or reuse and verify this exact obligation: resize/input/clipboard. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-026.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for resize/input/clipboard; retain observable state/resource expectations.
- [ ] **F-NW-026.05 — Implement/prove: reconnect and privacy**
  - Action: For remote desktop, implement or reuse and verify this exact obligation: reconnect and privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-026.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reconnect and privacy; retain observable state/resource expectations.
- [ ] **F-NW-026.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to remote desktop: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-026.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-026.07 — Integrate into the real consumer and runtime route**
  - Action: Wire remote desktop into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-026.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-026.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for remote desktop as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-026.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-027"></a>
## F-NW-027 — captive-portal flow

**Original requirement:** detected restricted state, sandboxed login, privacy and explicit completion

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-027.01 — Reconcile existing captive-portal flow**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for captive-portal flow. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: detected restricted state, sandboxed login, privacy and explicit completion
- [ ] **F-NW-027.02 — Freeze the exact contract for captive-portal flow**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: detected restricted state, sandboxed login, privacy and explicit completion. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-027.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-027.03 — Implement/prove: detected restricted state**
  - Action: For captive-portal flow, implement or reuse and verify this exact obligation: detected restricted state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-027.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for detected restricted state; retain observable state/resource expectations.
- [ ] **F-NW-027.04 — Implement/prove: sandboxed login**
  - Action: For captive-portal flow, implement or reuse and verify this exact obligation: sandboxed login. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-027.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sandboxed login; retain observable state/resource expectations.
- [ ] **F-NW-027.05 — Implement/prove: privacy and explicit completion**
  - Action: For captive-portal flow, implement or reuse and verify this exact obligation: privacy and explicit completion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-027.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy and explicit completion; retain observable state/resource expectations.
- [ ] **F-NW-027.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to captive-portal flow: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-027.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-027.07 — Integrate into the real consumer and runtime route**
  - Action: Wire captive-portal flow into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-027.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-027.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for captive-portal flow as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-027.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-028"></a>
## F-NW-028 — Wi-Fi management

**Original requirement:** scan, profiles, secrets, association, roam, signal, failures and UI

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-028.01 — Reconcile existing Wi-Fi management**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for Wi-Fi management. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: scan, profiles, secrets, association, roam, signal, failures and UI
- [ ] **F-NW-028.02 — Freeze the exact contract for Wi-Fi management**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: scan, profiles, secrets, association, roam, signal, failures and UI. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-028.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-028.03 — Implement/prove: scan**
  - Action: For Wi-Fi management, implement or reuse and verify this exact obligation: scan. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-028.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for scan; retain observable state/resource expectations.
- [ ] **F-NW-028.04 — Implement/prove: profiles**
  - Action: For Wi-Fi management, implement or reuse and verify this exact obligation: profiles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-028.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for profiles; retain observable state/resource expectations.
- [ ] **F-NW-028.05 — Implement/prove: secrets**
  - Action: For Wi-Fi management, implement or reuse and verify this exact obligation: secrets. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-028.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for secrets; retain observable state/resource expectations.
- [ ] **F-NW-028.06 — Implement/prove: association**
  - Action: For Wi-Fi management, implement or reuse and verify this exact obligation: association. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-028.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for association; retain observable state/resource expectations.
- [ ] **F-NW-028.07 — Implement/prove: roam**
  - Action: For Wi-Fi management, implement or reuse and verify this exact obligation: roam. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-028.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for roam; retain observable state/resource expectations.
- [ ] **F-NW-028.08 — Implement/prove: signal**
  - Action: For Wi-Fi management, implement or reuse and verify this exact obligation: signal. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-028.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for signal; retain observable state/resource expectations.
- [ ] **F-NW-028.09 — Implement/prove: failures and UI**
  - Action: For Wi-Fi management, implement or reuse and verify this exact obligation: failures and UI. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-028.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for failures and UI; retain observable state/resource expectations.
- [ ] **F-NW-028.10 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to Wi-Fi management: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-028.09.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-028.11 — Integrate into the real consumer and runtime route**
  - Action: Wire Wi-Fi management into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-028.10.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-028.12 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for Wi-Fi management as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-028.11.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-029"></a>
## F-NW-029 — VPN management

**Original requirement:** tunnel identity, routes/DNS, secrets, kill-switch and recovery

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-029.01 — Reconcile existing VPN management**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for VPN management. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: tunnel identity, routes/DNS, secrets, kill-switch and recovery
- [ ] **F-NW-029.02 — Freeze the exact contract for VPN management**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: tunnel identity, routes/DNS, secrets, kill-switch and recovery. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-029.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-029.03 — Implement/prove: tunnel identity**
  - Action: For VPN management, implement or reuse and verify this exact obligation: tunnel identity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-029.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for tunnel identity; retain observable state/resource expectations.
- [ ] **F-NW-029.04 — Implement/prove: routes/DNS**
  - Action: For VPN management, implement or reuse and verify this exact obligation: routes/DNS. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-029.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for routes/DNS; retain observable state/resource expectations.
- [ ] **F-NW-029.05 — Implement/prove: secrets**
  - Action: For VPN management, implement or reuse and verify this exact obligation: secrets. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-029.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for secrets; retain observable state/resource expectations.
- [ ] **F-NW-029.06 — Implement/prove: kill-switch and recovery**
  - Action: For VPN management, implement or reuse and verify this exact obligation: kill-switch and recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-029.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for kill-switch and recovery; retain observable state/resource expectations.
- [ ] **F-NW-029.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to VPN management: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-029.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-029.08 — Integrate into the real consumer and runtime route**
  - Action: Wire VPN management into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-029.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-029.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for VPN management as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-029.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-030"></a>
## F-NW-030 — connectivity diagnostics

**Original requirement:** truthful link/address/route/DNS/TCP/TLS probes with timing and export

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-030.01 — Reconcile existing connectivity diagnostics**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for connectivity diagnostics. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: truthful link/address/route/DNS/TCP/TLS probes with timing and export
- [ ] **F-NW-030.02 — Freeze the exact contract for connectivity diagnostics**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: truthful link/address/route/DNS/TCP/TLS probes with timing and export. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-030.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-030.03 — Implement/prove: truthful link/address/route/DNS/TCP/TLS probes with timing and export**
  - Action: For connectivity diagnostics, implement or reuse and verify this exact obligation: truthful link/address/route/DNS/TCP/TLS probes with timing and export. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-030.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for truthful link/address/route/DNS/TCP/TLS probes with timing and export; retain observable state/resource expectations.
- [ ] **F-NW-030.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to connectivity diagnostics: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-030.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-030.05 — Integrate into the real consumer and runtime route**
  - Action: Wire connectivity diagnostics into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-030.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-030.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for connectivity diagnostics as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-030.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-031"></a>
## F-NW-031 — per-app network permission

**Original requirement:** destination/protocol/listen/background policy with live revoke

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-031.01 — Reconcile existing per-app network permission**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for per-app network permission. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: destination/protocol/listen/background policy with live revoke
- [ ] **F-NW-031.02 — Freeze the exact contract for per-app network permission**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: destination/protocol/listen/background policy with live revoke. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-031.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-031.03 — Implement/prove: destination/protocol/listen/background policy with live revoke**
  - Action: For per-app network permission, implement or reuse and verify this exact obligation: destination/protocol/listen/background policy with live revoke. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-031.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for destination/protocol/listen/background policy with live revoke; retain observable state/resource expectations.
- [ ] **F-NW-031.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to per-app network permission: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-031.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-031.05 — Integrate into the real consumer and runtime route**
  - Action: Wire per-app network permission into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-031.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-031.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for per-app network permission as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-031.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-032"></a>
## F-NW-032 — traffic accounting

**Original requirement:** per-interface/app/service bytes, drops, latency and privacy-aware history

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-032.01 — Reconcile existing traffic accounting**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for traffic accounting. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: per-interface/app/service bytes, drops, latency and privacy-aware history
- [ ] **F-NW-032.02 — Freeze the exact contract for traffic accounting**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: per-interface/app/service bytes, drops, latency and privacy-aware history. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-032.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-032.03 — Implement/prove: per-interface/app/service bytes**
  - Action: For traffic accounting, implement or reuse and verify this exact obligation: per-interface/app/service bytes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-032.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-interface/app/service bytes; retain observable state/resource expectations.
- [ ] **F-NW-032.04 — Implement/prove: drops**
  - Action: For traffic accounting, implement or reuse and verify this exact obligation: drops. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-032.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for drops; retain observable state/resource expectations.
- [ ] **F-NW-032.05 — Implement/prove: latency and privacy-aware history**
  - Action: For traffic accounting, implement or reuse and verify this exact obligation: latency and privacy-aware history. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-032.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for latency and privacy-aware history; retain observable state/resource expectations.
- [ ] **F-NW-032.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to traffic accounting: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-032.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-032.07 — Integrate into the real consumer and runtime route**
  - Action: Wire traffic accounting into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-032.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-032.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for traffic accounting as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-032.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-033"></a>
## F-NW-033 — egress policy for demos/agents

**Original requirement:** allowlisted destinations, quotas, DNS consistency, audit and termination

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-033.01 — Reconcile existing egress policy for demos/agents**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for egress policy for demos/agents. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: allowlisted destinations, quotas, DNS consistency, audit and termination
- [ ] **F-NW-033.02 — Freeze the exact contract for egress policy for demos/agents**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: allowlisted destinations, quotas, DNS consistency, audit and termination. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-033.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-033.03 — Implement/prove: allowlisted destinations**
  - Action: For egress policy for demos/agents, implement or reuse and verify this exact obligation: allowlisted destinations. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-033.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for allowlisted destinations; retain observable state/resource expectations.
- [ ] **F-NW-033.04 — Implement/prove: quotas**
  - Action: For egress policy for demos/agents, implement or reuse and verify this exact obligation: quotas. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-033.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for quotas; retain observable state/resource expectations.
- [ ] **F-NW-033.05 — Implement/prove: DNS consistency**
  - Action: For egress policy for demos/agents, implement or reuse and verify this exact obligation: DNS consistency. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-033.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for DNS consistency; retain observable state/resource expectations.
- [ ] **F-NW-033.06 — Implement/prove: audit and termination**
  - Action: For egress policy for demos/agents, implement or reuse and verify this exact obligation: audit and termination. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-033.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for audit and termination; retain observable state/resource expectations.
- [ ] **F-NW-033.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to egress policy for demos/agents: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-033.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-033.08 — Integrate into the real consumer and runtime route**
  - Action: Wire egress policy for demos/agents into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-033.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-033.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for egress policy for demos/agents as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-033.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-034"></a>
## F-NW-034 — offline mode

**Original requirement:** deterministic network refusal, cached-data behavior and visible state

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-034.01 — Reconcile existing offline mode**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for offline mode. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: deterministic network refusal, cached-data behavior and visible state
- [ ] **F-NW-034.02 — Freeze the exact contract for offline mode**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: deterministic network refusal, cached-data behavior and visible state. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-034.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-034.03 — Implement/prove: deterministic network refusal**
  - Action: For offline mode, implement or reuse and verify this exact obligation: deterministic network refusal. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-034.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deterministic network refusal; retain observable state/resource expectations.
- [ ] **F-NW-034.04 — Implement/prove: cached-data behavior and visible state**
  - Action: For offline mode, implement or reuse and verify this exact obligation: cached-data behavior and visible state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-034.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cached-data behavior and visible state; retain observable state/resource expectations.
- [ ] **F-NW-034.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to offline mode: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-034.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-034.06 — Integrate into the real consumer and runtime route**
  - Action: Wire offline mode into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-034.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-034.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for offline mode as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-034.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-nw-035"></a>
## F-NW-035 — network recovery

**Original requirement:** provider/service restart, connection invalidation, retry/backoff and user state

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-NW-035.01 — Reconcile existing network recovery**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for network recovery. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: provider/service restart, connection invalidation, retry/backoff and user state
- [ ] **F-NW-035.02 — Freeze the exact contract for network recovery**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: provider/service restart, connection invalidation, retry/backoff and user state. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-NW-035.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-NW-035.03 — Implement/prove: provider/service restart**
  - Action: For network recovery, implement or reuse and verify this exact obligation: provider/service restart. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-035.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for provider/service restart; retain observable state/resource expectations.
- [ ] **F-NW-035.04 — Implement/prove: connection invalidation**
  - Action: For network recovery, implement or reuse and verify this exact obligation: connection invalidation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-035.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for connection invalidation; retain observable state/resource expectations.
- [ ] **F-NW-035.05 — Implement/prove: retry/backoff and user state**
  - Action: For network recovery, implement or reuse and verify this exact obligation: retry/backoff and user state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-NW-035.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for retry/backoff and user state; retain observable state/resource expectations.
- [ ] **F-NW-035.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to network recovery: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-NW-035.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-NW-035.07 — Integrate into the real consumer and runtime route**
  - Action: Wire network recovery into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-NW-035.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-NW-035.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for network recovery as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-NW-035.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="c-p5-5"></a>
## C-P5.5 — Network/Resolver/TLS service APIs

**Original requirement:** Network/Resolver/TLS service APIs

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 309.

### Preserved original contract

- **Dependencies/current/provenance:** P4 and current protocol code; Brook host TCP, Serenity services, snarkOS role budgets; reject NIC-global API and one-fetch completion.
- **I/O and state:** network capability/socket/query plus bounds/deadline in; socket/data/DNS/TLS result out; standard socket states and service lifecycle.
- **Invariants/failure:** per-process quotas; no cross-handle access; deterministic timers; resolver/TLS policy explicit; driver restart terminates/rebinds sockets visibly.
- **Deterministic proof:** protocol simulator loss/reorder/duplicate/delay/wrap/congestion, malformed packet fuzz, DNS/TLS failures, quota, service crash.
- **Target proof:** QEMU existing virtio flow via service; physical driver proof in Phase 6.
- **Receipt/removal:** packet/timer/state/cert policy trace; legacy `net_link` adapter; remove direct app stack calls after browser migration.

### Execution steps

- [ ] **C-P5.5.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P5.5.02 — Resolve this contract's exact dependencies**
  - Action: P4 and current protocol code; Brook host TCP, Serenity services, snarkOS role budgets; reject NIC-global API and one-fetch completion. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P5.5.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P5.5.03 — I/O and state — Network/Resolver/TLS service APIs**
  - Action: network capability/socket/query plus bounds/deadline in; socket/data/DNS/TLS result out; standard socket states and service lifecycle.
  - Requires: C-P5.5.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P5.5.
- [ ] **C-P5.5.04 — Invariants/failure — Network/Resolver/TLS service APIs**
  - Action: per-process quotas; no cross-handle access; deterministic timers; resolver/TLS policy explicit; driver restart terminates/rebinds sockets visibly.
  - Requires: C-P5.5.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P5.5.
- [ ] **C-P5.5.05 — Deterministic proof — Network/Resolver/TLS service APIs**
  - Action: protocol simulator loss/reorder/duplicate/delay/wrap/congestion, malformed packet fuzz, DNS/TLS failures, quota, service crash.
  - Requires: C-P5.5.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P5.5.
- [ ] **C-P5.5.06 — Target proof — Network/Resolver/TLS service APIs**
  - Action: QEMU existing virtio flow via service; physical driver proof in Phase 6.
  - Requires: C-P5.5.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P5.5.
- [ ] **C-P5.5.07 — Receipt/removal — Network/Resolver/TLS service APIs**
  - Action: packet/timer/state/cert policy trace; legacy `net_link` adapter; remove direct app stack calls after browser migration.
  - Requires: C-P5.5.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P5.5.
- [ ] **C-P5.5.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P5.5. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P5.5.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p6-1"></a>
## C-P6.1 — NIC provider selection and resource contract

**Original requirement:** NIC provider selection and resource contract

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 340.

### Preserved original contract

- **Dependencies/current/provenance:** P5.5, P4 caps and driver descriptor; Zinnia modules, Astral NIC boundary; reject virtio as network API.
- **I/O and state:** PCI/USB resource handles and device descriptor in; link provider handle/mac/carrier/counters out; driver lifecycle plus link `Down -> Negotiating -> Up -> Lost`.
- **Invariants/failure:** explicit DMA/IRQ/BAR ownership; bounded reset/rings; carrier truthful; one selected route policy; device removal completes requests.
- **Deterministic proof:** fake descriptor rings, wrap/full/reset, missing carrier, removal, DMA bounds, provider competition.
- **Target proof:** QEMU virtio; supported USB Ethernet/tether; I219 only with physical carrier/dongle.
- **Receipt/removal:** device IDs/resources/rings/carrier/packet counters; virtio stays fallback; no provider removed merely for adding hardware.

### Execution steps

- [ ] **C-P6.1.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P6.1.02 — Resolve this contract's exact dependencies**
  - Action: P5.5, P4 caps and driver descriptor; Zinnia modules, Astral NIC boundary; reject virtio as network API. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P6.1.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P6.1.03 — I/O and state — NIC provider selection and resource contract**
  - Action: PCI/USB resource handles and device descriptor in; link provider handle/mac/carrier/counters out; driver lifecycle plus link `Down -> Negotiating -> Up -> Lost`.
  - Requires: C-P6.1.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P6.1.
- [ ] **C-P6.1.04 — Invariants/failure — NIC provider selection and resource contract**
  - Action: explicit DMA/IRQ/BAR ownership; bounded reset/rings; carrier truthful; one selected route policy; device removal completes requests.
  - Requires: C-P6.1.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P6.1.
- [ ] **C-P6.1.05 — Deterministic proof — NIC provider selection and resource contract**
  - Action: fake descriptor rings, wrap/full/reset, missing carrier, removal, DMA bounds, provider competition.
  - Requires: C-P6.1.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P6.1.
- [ ] **C-P6.1.06 — Target proof — NIC provider selection and resource contract**
  - Action: QEMU virtio; supported USB Ethernet/tether; I219 only with physical carrier/dongle.
  - Requires: C-P6.1.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P6.1.
- [ ] **C-P6.1.07 — Receipt/removal — NIC provider selection and resource contract**
  - Action: device IDs/resources/rings/carrier/packet counters; virtio stays fallback; no provider removed merely for adding hardware.
  - Requires: C-P6.1.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P6.1.
- [ ] **C-P6.1.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P6.1. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P6.1.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p6-2"></a>
## C-P6.2 — DHCP/DNS/TCP reliability on physical link

**Original requirement:** DHCP/DNS/TCP reliability on physical link

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 349.

### Preserved original contract

- **Dependencies/current/provenance:** P6.1 and P5.5 simulator; Brook/Astral/Serenity; reject hard-coded QEMU configuration.
- **I/O and state:** carrier and configured policy in; lease/routes/resolvers/reliable byte stream out; protocol states with controlled deadlines.
- **Invariants/failure:** checksums/IDs/sequence windows validated; retransmit and reorder bounded; lease expiry/rebind explicit; no busy wait.
- **Deterministic proof:** packet fault matrix, long response, connection reuse, simultaneous close/reset, lease renew/loss, DNS truncation/failure.
- **Target proof:** same corpus over QEMU and real wired link with packet/state ZLLOG summaries.
- **Receipt/removal:** separate carrier/DHCP/DNS/TCP receipts; static config remains diagnostic fallback; remove default hard-coded path after DHCP success/failure gates.

### Execution steps

- [ ] **C-P6.2.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P6.2.02 — Resolve this contract's exact dependencies**
  - Action: P6.1 and P5.5 simulator; Brook/Astral/Serenity; reject hard-coded QEMU configuration. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P6.2.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P6.2.03 — I/O and state — DHCP/DNS/TCP reliability on physical link**
  - Action: carrier and configured policy in; lease/routes/resolvers/reliable byte stream out; protocol states with controlled deadlines.
  - Requires: C-P6.2.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P6.2.
- [ ] **C-P6.2.04 — Invariants/failure — DHCP/DNS/TCP reliability on physical link**
  - Action: checksums/IDs/sequence windows validated; retransmit and reorder bounded; lease expiry/rebind explicit; no busy wait.
  - Requires: C-P6.2.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P6.2.
- [ ] **C-P6.2.05 — Deterministic proof — DHCP/DNS/TCP reliability on physical link**
  - Action: packet fault matrix, long response, connection reuse, simultaneous close/reset, lease renew/loss, DNS truncation/failure.
  - Requires: C-P6.2.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P6.2.
- [ ] **C-P6.2.06 — Target proof — DHCP/DNS/TCP reliability on physical link**
  - Action: same corpus over QEMU and real wired link with packet/state ZLLOG summaries.
  - Requires: C-P6.2.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P6.2.
- [ ] **C-P6.2.07 — Receipt/removal — DHCP/DNS/TCP reliability on physical link**
  - Action: separate carrier/DHCP/DNS/TCP receipts; static config remains diagnostic fallback; remove default hard-coded path after DHCP success/failure gates.
  - Requires: C-P6.2.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P6.2.
- [ ] **C-P6.2.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P6.2. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P6.2.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p6-3"></a>
## C-P6.3 — TLS/request boundary

**Original requirement:** TLS/request boundary

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 358.

### Preserved original contract

- **Dependencies/current/provenance:** P6.2, clocks/cert store and restricted process; Serenity RequestServer; reject silent certificate downgrade.
- **I/O and state:** URL/request handle, policy, deadline in; response stream/metadata or typed TLS/network error out.
- **Invariants/failure:** hostname/time/chain validation; bounded headers/body; redirects/protocols explicit; cancellation closes resources; unsupported feature refuses.
- **Deterministic proof:** known test certificates, wrong host/expired/untrusted/truncated records, large/slow responses, cancellation, parser separation.
- **Target proof:** QEMU and physical HTTPS fetch with exact policy receipt, not body logging.
- **Receipt/removal:** endpoint/cert-policy/result digests; current direct TLS path can be selected for diagnosis; remove only after browser parity.

### Execution steps

- [ ] **C-P6.3.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P6.3.02 — Resolve this contract's exact dependencies**
  - Action: P6.2, clocks/cert store and restricted process; Serenity RequestServer; reject silent certificate downgrade. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P6.3.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P6.3.03 — I/O and state — TLS/request boundary**
  - Action: URL/request handle, policy, deadline in; response stream/metadata or typed TLS/network error out.
  - Requires: C-P6.3.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P6.3.
- [ ] **C-P6.3.04 — Invariants/failure — TLS/request boundary**
  - Action: hostname/time/chain validation; bounded headers/body; redirects/protocols explicit; cancellation closes resources; unsupported feature refuses.
  - Requires: C-P6.3.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P6.3.
- [ ] **C-P6.3.05 — Deterministic proof — TLS/request boundary**
  - Action: known test certificates, wrong host/expired/untrusted/truncated records, large/slow responses, cancellation, parser separation.
  - Requires: C-P6.3.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P6.3.
- [ ] **C-P6.3.06 — Target proof — TLS/request boundary**
  - Action: QEMU and physical HTTPS fetch with exact policy receipt, not body logging.
  - Requires: C-P6.3.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P6.3.
- [ ] **C-P6.3.07 — Receipt/removal — TLS/request boundary**
  - Action: endpoint/cert-policy/result digests; current direct TLS path can be selected for diagnosis; remove only after browser parity.
  - Requires: C-P6.3.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P6.3.
- [ ] **C-P6.3.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P6.3. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P6.3.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-13"></a>
## C-DA-13 — NIC provider and packet simulator

**Original requirement:** NIC provider and packet simulator

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 338.

### Preserved original contract

**Depends on:** DA-04 through DA-08.

**Deliver:** identity/MAC/MTU/features/carrier, RX-buffer supply, TX packet, cancel,
counters and reset. Wrap virtio-net, add deterministic simulation, then physical
wired hardware; protocol/socket policy remains separate.

**Invariants:** packets are checked slices; RX buffers have single ownership;
backpressure and drops are counted; device reset cannot duplicate completion;
ordinary apps cannot configure raw NIC state.

**Proof:** runt/oversized frames, bad checksum/offload metadata, loss/duplicate/
reorder/delay, ring exhaustion, link flap, reset during TX/RX, DHCP/DNS/TCP/TLS
milestones kept separate, QEMU virtio and named physical NIC receipt.

### Execution steps

- [ ] **C-DA-13.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-13.02 — Resolve this contract's exact dependencies**
  - Action: DA-04 through DA-08. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-13.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-13.03 — Deliver — NIC provider and packet simulator**
  - Action: identity/MAC/MTU/features/carrier, RX-buffer supply, TX packet, cancel, counters and reset. Wrap virtio-net, add deterministic simulation, then physical wired hardware; protocol/socket policy remains separate.
  - Requires: C-DA-13.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-13.
- [ ] **C-DA-13.04 — Invariants — NIC provider and packet simulator**
  - Action: packets are checked slices; RX buffers have single ownership; backpressure and drops are counted; device reset cannot duplicate completion; ordinary apps cannot configure raw NIC state.
  - Requires: C-DA-13.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-13.
- [ ] **C-DA-13.05 — Proof — NIC provider and packet simulator**
  - Action: runt/oversized frames, bad checksum/offload metadata, loss/duplicate/ reorder/delay, ring exhaustion, link flap, reset during TX/RX, DHCP/DNS/TCP/TLS milestones kept separate, QEMU virtio and named physical NIC receipt.
  - Requires: C-DA-13.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-13.
- [ ] **C-DA-13.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-13. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-13.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-22"></a>
## C-DA-22 — socket, resolver and network policy services

**Original requirement:** socket, resolver and network policy services

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 567.

### Preserved original contract

**Depends on:** DA-13, DA-18 and DA-19.

**Deliver:** capability-scoped socket handles, DHCP/address service, resolver, TCP/
UDP and TLS policy separated from NIC provider; explicit raw-packet authority.

**Invariants:** network access follows app manifest and session policy; DNS/TCP/TLS
completion are distinct; secret material is never logged; service restart fails or
reconnects requests explicitly.

**Proof:** malformed packets/certificates, loss/reorder/timeouts, DNS poisoning
fixture, revoked network right, raw-socket denial, resolver/TLS crash and browser
survival.

### Execution steps

- [ ] **C-DA-22.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-22.02 — Resolve this contract's exact dependencies**
  - Action: DA-13, DA-18 and DA-19. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-22.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-22.03 — Deliver — socket, resolver and network policy services**
  - Action: capability-scoped socket handles, DHCP/address service, resolver, TCP/ UDP and TLS policy separated from NIC provider; explicit raw-packet authority.
  - Requires: C-DA-22.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-22.
- [ ] **C-DA-22.04 — Invariants — socket, resolver and network policy services**
  - Action: network access follows app manifest and session policy; DNS/TCP/TLS completion are distinct; secret material is never logged; service restart fails or reconnects requests explicitly.
  - Requires: C-DA-22.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-22.
- [ ] **C-DA-22.05 — Proof — socket, resolver and network policy services**
  - Action: malformed packets/certificates, loss/reorder/timeouts, DNS poisoning fixture, revoked network right, raw-socket denial, resolver/TLS crash and browser survival.
  - Requires: C-DA-22.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-22.
- [ ] **C-DA-22.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-22. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-22.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="t-usb-010"></a>
## T-USB-010 — CDC-ECM network

**Original requirement:** descriptors, link, frames and hotplug

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 145.

### Execution steps

- [ ] **T-USB-010.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve CDC-ECM network to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-USB-010.02 — Specify the complete target boundary**
  - Action: CDC-ECM network must supply: descriptors, link, frames and hotplug. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-USB-010.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-USB-010.03 — Implement the exact target behavior**
  - Action: Implement or reuse CDC-ECM network through the shared platform contract, delivering every part of: descriptors, link, frames and hotplug. Do not fork a duplicate subsystem for this row.
  - Requires: T-USB-010.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-USB-010.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-USB-010.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for CDC-ECM network.
- [ ] **T-USB-010.05 — Qualify and retain this target's own result**
  - Action: Bind CDC-ECM network to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-USB-010.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-usb-011"></a>
## T-USB-011 — RNDIS network

**Original requirement:** bounded messages, compatibility and teardown

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 146.

### Execution steps

- [ ] **T-USB-011.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve RNDIS network to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-USB-011.02 — Specify the complete target boundary**
  - Action: RNDIS network must supply: bounded messages, compatibility and teardown. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-USB-011.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-USB-011.03 — Implement the exact target behavior**
  - Action: Implement or reuse RNDIS network through the shared platform contract, delivering every part of: bounded messages, compatibility and teardown. Do not fork a duplicate subsystem for this row.
  - Requires: T-USB-011.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-USB-011.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-USB-011.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for RNDIS network.
- [ ] **T-USB-011.05 — Qualify and retain this target's own result**
  - Action: Bind RNDIS network to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-USB-011.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-nic-001"></a>
## T-NIC-001 — loopback

**Original requirement:** full socket semantics and fault injection

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 204.

### Execution steps

- [ ] **T-NIC-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve loopback to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-NIC-001.02 — Specify the complete target boundary**
  - Action: loopback must supply: full socket semantics and fault injection. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-NIC-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-NIC-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse loopback through the shared platform contract, delivering every part of: full socket semantics and fault injection. Do not fork a duplicate subsystem for this row.
  - Requires: T-NIC-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-NIC-001.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-NIC-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for loopback.
- [ ] **T-NIC-001.05 — Qualify and retain this target's own result**
  - Action: Bind loopback to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-NIC-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-nic-002"></a>
## T-NIC-002 — virtio-net

**Original requirement:** queues/features/offloads/reset/multiqueue

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 205.

### Execution steps

- [ ] **T-NIC-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve virtio-net to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-NIC-002.02 — Specify the complete target boundary**
  - Action: virtio-net must supply: queues/features/offloads/reset/multiqueue. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-NIC-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-NIC-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse virtio-net through the shared platform contract, delivering every part of: queues/features/offloads/reset/multiqueue. Do not fork a duplicate subsystem for this row.
  - Requires: T-NIC-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-NIC-002.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-NIC-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for virtio-net.
- [ ] **T-NIC-002.05 — Qualify and retain this target's own result**
  - Action: Bind virtio-net to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-NIC-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-nic-003"></a>
## T-NIC-003 — Intel 8254x e1000

**Original requirement:** RX/TX rings, link, reset and physical variants

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 206.

### Execution steps

- [ ] **T-NIC-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Intel 8254x e1000 to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-NIC-003.02 — Specify the complete target boundary**
  - Action: Intel 8254x e1000 must supply: RX/TX rings, link, reset and physical variants. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-NIC-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-NIC-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse Intel 8254x e1000 through the shared platform contract, delivering every part of: RX/TX rings, link, reset and physical variants. Do not fork a duplicate subsystem for this row.
  - Requires: T-NIC-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-NIC-003.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-NIC-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Intel 8254x e1000.
- [ ] **T-NIC-003.05 — Qualify and retain this target's own result**
  - Action: Bind Intel 8254x e1000 to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-NIC-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-nic-004"></a>
## T-NIC-004 — Intel e1000e/I219

**Original requirement:** NVM/PHY/carrier/reset and ThinkPad receipt

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 207.

### Execution steps

- [ ] **T-NIC-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Intel e1000e/I219 to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-NIC-004.02 — Specify the complete target boundary**
  - Action: Intel e1000e/I219 must supply: NVM/PHY/carrier/reset and ThinkPad receipt. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-NIC-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-NIC-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse Intel e1000e/I219 through the shared platform contract, delivering every part of: NVM/PHY/carrier/reset and ThinkPad receipt. Do not fork a duplicate subsystem for this row.
  - Requires: T-NIC-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-NIC-004.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-NIC-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Intel e1000e/I219.
- [ ] **T-NIC-004.05 — Qualify and retain this target's own result**
  - Action: Bind Intel e1000e/I219 to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-NIC-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-nic-005"></a>
## T-NIC-005 — Intel igb

**Original requirement:** multiqueue/MSI-X/offloads and reset

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 208.

### Execution steps

- [ ] **T-NIC-005.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Intel igb to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-NIC-005.02 — Specify the complete target boundary**
  - Action: Intel igb must supply: multiqueue/MSI-X/offloads and reset. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-NIC-005.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-NIC-005.03 — Implement the exact target behavior**
  - Action: Implement or reuse Intel igb through the shared platform contract, delivering every part of: multiqueue/MSI-X/offloads and reset. Do not fork a duplicate subsystem for this row.
  - Requires: T-NIC-005.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-NIC-005.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-NIC-005.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Intel igb.
- [ ] **T-NIC-005.05 — Qualify and retain this target's own result**
  - Action: Bind Intel igb to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-NIC-005.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-nic-006"></a>
## T-NIC-006 — Intel igc/I225/I226

**Original requirement:** generation link/queue/timing behavior

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 209.

### Execution steps

- [ ] **T-NIC-006.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Intel igc/I225/I226 to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-NIC-006.02 — Specify the complete target boundary**
  - Action: Intel igc/I225/I226 must supply: generation link/queue/timing behavior. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-NIC-006.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-NIC-006.03 — Implement the exact target behavior**
  - Action: Implement or reuse Intel igc/I225/I226 through the shared platform contract, delivering every part of: generation link/queue/timing behavior. Do not fork a duplicate subsystem for this row.
  - Requires: T-NIC-006.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-NIC-006.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-NIC-006.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Intel igc/I225/I226.
- [ ] **T-NIC-006.05 — Qualify and retain this target's own result**
  - Action: Bind Intel igc/I225/I226 to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-NIC-006.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-nic-007"></a>
## T-NIC-007 — Realtek RTL8139

**Original requirement:** legacy rings, overflow and reset

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 210.

### Execution steps

- [ ] **T-NIC-007.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Realtek RTL8139 to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-NIC-007.02 — Specify the complete target boundary**
  - Action: Realtek RTL8139 must supply: legacy rings, overflow and reset. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-NIC-007.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-NIC-007.03 — Implement the exact target behavior**
  - Action: Implement or reuse Realtek RTL8139 through the shared platform contract, delivering every part of: legacy rings, overflow and reset. Do not fork a duplicate subsystem for this row.
  - Requires: T-NIC-007.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-NIC-007.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-NIC-007.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Realtek RTL8139.
- [ ] **T-NIC-007.05 — Qualify and retain this target's own result**
  - Action: Bind Realtek RTL8139 to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-NIC-007.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-nic-008"></a>
## T-NIC-008 — Realtek RTL8168/8169

**Original requirement:** descriptor revisions/PHY/offloads/reset

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 211.

### Execution steps

- [ ] **T-NIC-008.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Realtek RTL8168/8169 to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-NIC-008.02 — Specify the complete target boundary**
  - Action: Realtek RTL8168/8169 must supply: descriptor revisions/PHY/offloads/reset. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-NIC-008.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-NIC-008.03 — Implement the exact target behavior**
  - Action: Implement or reuse Realtek RTL8168/8169 through the shared platform contract, delivering every part of: descriptor revisions/PHY/offloads/reset. Do not fork a duplicate subsystem for this row.
  - Requires: T-NIC-008.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-NIC-008.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-NIC-008.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Realtek RTL8168/8169.
- [ ] **T-NIC-008.05 — Qualify and retain this target's own result**
  - Action: Bind Realtek RTL8168/8169 to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-NIC-008.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-nic-011"></a>
## T-NIC-011 — USB CDC-ECM adapter

**Original requirement:** class lifecycle and physical dongle matrix

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 214.

### Execution steps

- [ ] **T-NIC-011.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve USB CDC-ECM adapter to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-NIC-011.02 — Specify the complete target boundary**
  - Action: USB CDC-ECM adapter must supply: class lifecycle and physical dongle matrix. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-NIC-011.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-NIC-011.03 — Implement the exact target behavior**
  - Action: Implement or reuse USB CDC-ECM adapter through the shared platform contract, delivering every part of: class lifecycle and physical dongle matrix. Do not fork a duplicate subsystem for this row.
  - Requires: T-NIC-011.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-NIC-011.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-NIC-011.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for USB CDC-ECM adapter.
- [ ] **T-NIC-011.05 — Qualify and retain this target's own result**
  - Action: Bind USB CDC-ECM adapter to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-NIC-011.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-nic-012"></a>
## T-NIC-012 — USB RNDIS adapter

**Original requirement:** compatibility protocol and containment

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 215.

### Execution steps

- [ ] **T-NIC-012.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve USB RNDIS adapter to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-NIC-012.02 — Specify the complete target boundary**
  - Action: USB RNDIS adapter must supply: compatibility protocol and containment. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-NIC-012.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-NIC-012.03 — Implement the exact target behavior**
  - Action: Implement or reuse USB RNDIS adapter through the shared platform contract, delivering every part of: compatibility protocol and containment. Do not fork a duplicate subsystem for this row.
  - Requires: T-NIC-012.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-NIC-012.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-NIC-012.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for USB RNDIS adapter.
- [ ] **T-NIC-012.05 — Qualify and retain this target's own result**
  - Action: Bind USB RNDIS adapter to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-NIC-012.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-nic-013"></a>
## T-NIC-013 — USB ASIX Ethernet

**Original requirement:** real driver beyond identify-only

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 216.

### Execution steps

- [ ] **T-NIC-013.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve USB ASIX Ethernet to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-NIC-013.02 — Specify the complete target boundary**
  - Action: USB ASIX Ethernet must supply: real driver beyond identify-only. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-NIC-013.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-NIC-013.03 — Implement the exact target behavior**
  - Action: Implement or reuse USB ASIX Ethernet through the shared platform contract, delivering every part of: real driver beyond identify-only. Do not fork a duplicate subsystem for this row.
  - Requires: T-NIC-013.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-NIC-013.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-NIC-013.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for USB ASIX Ethernet.
- [ ] **T-NIC-013.05 — Qualify and retain this target's own result**
  - Action: Bind USB ASIX Ethernet to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-NIC-013.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-nic-014"></a>
## T-NIC-014 — USB Realtek Ethernet

**Original requirement:** real driver beyond identify-only

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 217.

### Execution steps

- [ ] **T-NIC-014.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve USB Realtek Ethernet to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-NIC-014.02 — Specify the complete target boundary**
  - Action: USB Realtek Ethernet must supply: real driver beyond identify-only. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-NIC-014.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-NIC-014.03 — Implement the exact target behavior**
  - Action: Implement or reuse USB Realtek Ethernet through the shared platform contract, delivering every part of: real driver beyond identify-only. Do not fork a duplicate subsystem for this row.
  - Requires: T-NIC-014.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-NIC-014.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-NIC-014.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for USB Realtek Ethernet.
- [ ] **T-NIC-014.05 — Qualify and retain this target's own result**
  - Action: Bind USB Realtek Ethernet to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-NIC-014.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-050"></a>
## T-SVC-050 — Network Device Broker

**Original requirement:** current netdev -> multiple providers/hotplug/failover

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 70.

### Execution steps

- [ ] **T-SVC-050.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Network Device Broker to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-050.02 — Specify the complete target boundary**
  - Action: Network Device Broker must supply: current netdev -> multiple providers/hotplug/failover. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-050.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-050.03 — Implement the exact target behavior**
  - Action: Implement or reuse Network Device Broker through the shared platform contract, delivering every part of: current netdev -> multiple providers/hotplug/failover. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-050.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-050.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-SVC-050.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Network Device Broker.
- [ ] **T-SVC-050.05 — Qualify and retain this target's own result**
  - Action: Bind Network Device Broker to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-050.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-051"></a>
## T-SVC-051 — Network Stack Service

**Original requirement:** Ethernet/ARP/NDP/IP/ICMP/UDP/TCP/local sockets

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 71.

### Execution steps

- [ ] **T-SVC-051.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Network Stack Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-051.02 — Specify the complete target boundary**
  - Action: Network Stack Service must supply: Ethernet/ARP/NDP/IP/ICMP/UDP/TCP/local sockets. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-051.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-051.03 — Implement the exact target behavior**
  - Action: Implement or reuse Network Stack Service through the shared platform contract, delivering every part of: Ethernet/ARP/NDP/IP/ICMP/UDP/TCP/local sockets. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-051.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-051.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-SVC-051.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Network Stack Service.
- [ ] **T-SVC-051.05 — Qualify and retain this target's own result**
  - Action: Bind Network Stack Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-051.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-052"></a>
## T-SVC-052 — Address Configuration

**Original requirement:** DHCPv4/v6, SLAAC, static and lease persistence

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 72.

### Execution steps

- [ ] **T-SVC-052.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Address Configuration to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-052.02 — Specify the complete target boundary**
  - Action: Address Configuration must supply: DHCPv4/v6, SLAAC, static and lease persistence. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-052.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-052.03 — Implement the exact target behavior**
  - Action: Implement or reuse Address Configuration through the shared platform contract, delivering every part of: DHCPv4/v6, SLAAC, static and lease persistence. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-052.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-052.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-SVC-052.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Address Configuration.
- [ ] **T-SVC-052.05 — Qualify and retain this target's own result**
  - Action: Bind Address Configuration to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-052.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-053"></a>
## T-SVC-053 — Resolver Service

**Original requirement:** DNS cache/search/DNSSEC policy/offline errors

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 73.

### Execution steps

- [ ] **T-SVC-053.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Resolver Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-053.02 — Specify the complete target boundary**
  - Action: Resolver Service must supply: DNS cache/search/DNSSEC policy/offline errors. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-053.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-053.03 — Implement the exact target behavior**
  - Action: Implement or reuse Resolver Service through the shared platform contract, delivering every part of: DNS cache/search/DNSSEC policy/offline errors. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-053.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-053.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-SVC-053.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Resolver Service.
- [ ] **T-SVC-053.05 — Qualify and retain this target's own result**
  - Action: Bind Resolver Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-053.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-054"></a>
## T-SVC-054 — TLS/Certificate Service

**Original requirement:** roots, validation, clocks, key isolation and revocation

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 74.

### Execution steps

- [ ] **T-SVC-054.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve TLS/Certificate Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-054.02 — Specify the complete target boundary**
  - Action: TLS/Certificate Service must supply: roots, validation, clocks, key isolation and revocation. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-054.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-054.03 — Implement the exact target behavior**
  - Action: Implement or reuse TLS/Certificate Service through the shared platform contract, delivering every part of: roots, validation, clocks, key isolation and revocation. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-054.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-054.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-SVC-054.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for TLS/Certificate Service.
- [ ] **T-SVC-054.05 — Qualify and retain this target's own result**
  - Action: Bind TLS/Certificate Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-054.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-055"></a>
## T-SVC-055 — HTTP/Transfer Service

**Original requirement:** bounded requests, redirects, caching, resume and download

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 75.

### Execution steps

- [ ] **T-SVC-055.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve HTTP/Transfer Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-055.02 — Specify the complete target boundary**
  - Action: HTTP/Transfer Service must supply: bounded requests, redirects, caching, resume and download. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-055.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-055.03 — Implement the exact target behavior**
  - Action: Implement or reuse HTTP/Transfer Service through the shared platform contract, delivering every part of: bounded requests, redirects, caching, resume and download. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-055.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-055.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-SVC-055.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for HTTP/Transfer Service.
- [ ] **T-SVC-055.05 — Qualify and retain this target's own result**
  - Action: Bind HTTP/Transfer Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-055.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-056"></a>
## T-SVC-056 — Routing/Firewall Service

**Original requirement:** routes, interfaces, zones, filtering and audit

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 76.

### Execution steps

- [ ] **T-SVC-056.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Routing/Firewall Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-056.02 — Specify the complete target boundary**
  - Action: Routing/Firewall Service must supply: routes, interfaces, zones, filtering and audit. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-056.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-056.03 — Implement the exact target behavior**
  - Action: Implement or reuse Routing/Firewall Service through the shared platform contract, delivering every part of: routes, interfaces, zones, filtering and audit. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-056.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-056.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-SVC-056.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Routing/Firewall Service.
- [ ] **T-SVC-056.05 — Qualify and retain this target's own result**
  - Action: Bind Routing/Firewall Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-056.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-059"></a>
## T-SVC-059 — Connectivity/Captive Portal Service

**Original requirement:** online state, portal detection and user flow

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 79.

### Execution steps

- [ ] **T-SVC-059.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Connectivity/Captive Portal Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-059.02 — Specify the complete target boundary**
  - Action: Connectivity/Captive Portal Service must supply: online state, portal detection and user flow. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-059.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-059.03 — Implement the exact target behavior**
  - Action: Implement or reuse Connectivity/Captive Portal Service through the shared platform contract, delivering every part of: online state, portal detection and user flow. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-059.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-059.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-SVC-059.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Connectivity/Captive Portal Service.
- [ ] **T-SVC-059.05 — Qualify and retain this target's own result**
  - Action: Bind Connectivity/Captive Portal Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-059.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-062"></a>
## T-SVC-062 — Time Synchronization Service

**Original requirement:** NTP/SNTP validation, monotonic-safe clock adjustment

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 82.

### Execution steps

- [ ] **T-SVC-062.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Time Synchronization Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-062.02 — Specify the complete target boundary**
  - Action: Time Synchronization Service must supply: NTP/SNTP validation, monotonic-safe clock adjustment. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-062.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-062.03 — Implement the exact target behavior**
  - Action: Implement or reuse Time Synchronization Service through the shared platform contract, delivering every part of: NTP/SNTP validation, monotonic-safe clock adjustment. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-062.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-062.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-SVC-062.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Time Synchronization Service.
- [ ] **T-SVC-062.05 — Qualify and retain this target's own result**
  - Action: Bind Time Synchronization Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-062.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-063"></a>
## T-SVC-063 — Network Diagnostics Service

**Original requirement:** ping/trace/DNS/socket/capture with scoped authority

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 83.

### Execution steps

- [ ] **T-SVC-063.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Network Diagnostics Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-10, D-15, H-08.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-063.02 — Specify the complete target boundary**
  - Action: Network Diagnostics Service must supply: ping/trace/DNS/socket/capture with scoped authority. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-063.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-063.03 — Implement the exact target behavior**
  - Action: Implement or reuse Network Diagnostics Service through the shared platform contract, delivering every part of: ping/trace/DNS/socket/capture with scoped authority. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-063.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-063.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-SVC-063.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Network Diagnostics Service.
- [ ] **T-SVC-063.05 — Qualify and retain this target's own result**
  - Action: Bind Network Diagnostics Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-063.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.
