# MP-00 security claim registry

`kernel/security-registry.json` records 17 claims with their protected asset,
threat, enforcement point, current evidence, adversarial proof and residual risk.
It deliberately contains zero production-complete and zero native-hardware-
complete security claims.

Current evidence is limited to one static claim (non-RWX ELF LOAD segments),
four host-limited areas (zlfs integrity, network parsers, TLS/X.509 and
interpreter budgets), and one QEMU-boot-reachable partial Ring-3 mechanism.

Eleven foundations are missing, including exhaustive syscall/usercopy proof,
capability handles/revocation, credentials, typed authorized IPC, IOMMU/DMA
isolation, secret storage, boot authenticity, app sandboxing, package signatures,
durable audit integrity and complete privileged-operation mediation.

The self-test rejects a missing claim, production overclaim, hidden DMA risk,
invented DMA proof and a claim without a protected asset.
