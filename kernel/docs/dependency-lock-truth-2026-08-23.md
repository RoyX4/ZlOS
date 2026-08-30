# zlOS host dependency lock

[`../metadata/dependency-lock.json`](../metadata/dependency-lock.json) is the exact local lock for
the tools and firmware used by the current build, packaging and six-route QEMU
proof. [`../tools/generators/gen-dependency-lock.py`](../tools/generators/gen-dependency-lock.py) regenerates and
checks it.

The lock records absolute invoked and resolved paths, byte count, SHA-256,
version command and measured version for 20 commands. This covers GCC, Clang,
GNU ld/objcopy, LLD linker, NASM, GRUB rescue, xorriso, GPT/FAT/mtools
utilities, both QEMU architectures, and the APT/dpkg/loader commands used to
recover and verify the package closure. Multi-call tools retain both the
invoked alias and resolved binary path, so `lld-link` cannot silently become
the generic `lld` driver during identification. It separately hashes both OVMF
code and variable-store images, the Python executable and the current zlOS
build identity.

The version-3 lock resolves 101 runtime files to 92 seed packages, follows every
installed dependency edge to 160 binary packages, and groups those binaries by
104 exact source-package/version identities. Reverse edges and binary-to-source
relationships are generated and mutation-checked; an unreachable package,
unknown owner, missing source relationship or invented archive custody is a
hard failure.

[`receipts/dependency-archives-host-2026-08-29.json`](receipts/dependency-archives-host-2026-08-29.json)
binds that graph to a content-addressed local cache. The receipt covers all 160
binary archives, all 104 source-package sets, 335 source files and 16 retained
APT metadata/trust files. Its normal check uses no network: it re-hashes every byte,
opens every `.deb`, validates every `.dsc` member and resolves the complete
dependency graph from the lock. Missing bytes, altered bytes, missing source
coverage, relationship drift and a false offline claim are rejected.

The landing gate checks but never rewrites the lock or receipt. A package
upgrade, alternative binary on `PATH`, changed OVMF blob or new build identity
therefore makes the gate red until the dependency change is deliberately
inspected. The landing gate validates the durable archive receipt without
requiring the 1.6 GB local cache in every checkout. The stronger explicit
archive check reads the cache and revalidates every retained byte.

This is not off-host custody, a signed supply-chain attestation, a license
grant, or a hermetic rebuild. One older `linux-libc-dev` binary and its matching
Linux source set remain in Kali's pool but no longer appear in the current
signed rolling index; the receipt labels both as unindexed pool recovery.
EV-002 owns independent off-host custody and EV-005 owns the hermetic rebuild
and signed toolchain boundary.
