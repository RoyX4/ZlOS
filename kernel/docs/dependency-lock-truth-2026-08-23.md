# zlOS host dependency lock

[`../metadata/dependency-lock.json`](../metadata/dependency-lock.json) is the exact local lock for
the tools and firmware used by the current build, packaging and six-route QEMU
proof. [`../tools/generators/gen-dependency-lock.py`](../tools/generators/gen-dependency-lock.py) regenerates and
checks it.

The lock records absolute invoked and resolved paths, byte count, SHA-256,
version command
and measured version for 15 commands: GCC, Clang, GNU ld/objcopy, LLD linker,
NASM, GRUB rescue, xorriso, GPT/FAT/mtools utilities and both QEMU
architectures. Multi-call
tools retain both the invoked alias and resolved binary path, so `lld-link`
cannot silently become the generic `lld` driver during identification.
It separately hashes both OVMF code and variable-store images, the
Python executable and the current zlOS build identity.

The landing gate checks but never rewrites this file. A package upgrade,
alternative binary on `PATH`, changed OVMF blob or new build identity therefore
makes the gate red until the dependency change is deliberately inspected and
the lock regenerated. The self-test plants missing-command, binary-drift,
wrong-invocation-alias, missing-firmware-identity and missing-build-identity
mutations.

This is not a signed supply-chain attestation. It proves exactly which local
bytes produced and booted the current artifacts; it does not prove who
published those bytes. Publisher signatures, immutable package sources and a
release SBOM remain later MP-00 work.
