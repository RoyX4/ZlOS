# Architecture OS audit: adversarial refutation and omissions

Date: 2026-08-21
Companion report: `ARCHITECTURE_OS_AUDIT_2026-08-21.md`
Source shelf: `/home/roy/Documents/repos/zl-starred-sources`

## Result

The first audit's overall synthesis survives, but several of its strongest mechanism claims need narrower wording. The most important corrections are:

1. NexiOS does not currently enforce `CAP_RIGHT_COPY` on the source capability, and its claimed bounded cascade revoke can skip grandchildren when a node has more than eight child CNodes. It is not yet a derivation-tree reference.
2. Chitti does not uniformly use asymmetric signatures for installable skills. Bundled/local packages use a kernel-baked symmetric MAC mislabeled `Ed25519`; foreign downloaded package ingestion is explicitly unfinished. The installer also ignores placement failure.
3. Zinnia's uACPI submodule is present, contrary to one sentence in the first audit, but critical OS-layer hooks are stubs or `todo!()` and cannot support a maturity claim.
4. Skift's receive-side capability transfer is not a usable transactional-transfer reference at this commit because its destination-capacity predicate counts occupied slots as available slots.
5. Fudge has real ring-3, per-task page tables and CR3 switching. Calling its address-space story absent was incorrect. Its authorization model remains weak, and its ELF tooling remains unsafe.
6. TacOS has two additional high-impact loader/syscall defects: negative syscall indexes bypass the upper-bound check, and ELF `p_filesz` can overflow an allocation sized from `p_memsz`.
7. NyauxKC's USTAR importer does not merely lack validation: it writes bytes starting at the tar header into every imported regular file. Its four-architecture build surface is only two-architecture source support.
8. Cyjon's syscall table bounds check is sound, but raw user output pointers can page-fault in kernel mode and the common exception path halts the entire machine.

RustOS's evidence-discipline conclusion survived the refutation pass. The needed correction is evidence granularity: its normal PR formal job does not require a fresh KVM boot trace, and the nightly-only tools must not be attributed to the PR gate.

## Evidence contract and method

- `[C]` claimed in repository prose, status, workflow, or checked-in evidence.
- `[S]` implementation source exists at the cited immutable snapshot.
- `[R]` statically connected to a normal build, boot path, syscall table, or service graph.
- `[T]` directly named by a test, test runner, CI job, model, or scenario. This does not mean it passed here.
- `[V-host]` a small host-side probe was run in this pass. It is not an OS build or boot.
- `[I]` inference from source control/data flow which was not reproduced in a booted target.

This was a read-only source, configuration, workflow, test, and license pass. No repository was built or booted, no QEMU or hardware test was run, and none of the findings below is runtime-verified. The only execution was a tiny NexiOS `expect` harness probe using `/bin/true`; it returned exit 1 and a `QEMU_EXIT` verdict, confirming that the active first block fails closed on EOF. Repositories were not modified.

## Immutable snapshots and licenses

| Repository | Exact `HEAD` | License found in checkout | Adversarial note |
|---|---|---|---|
| ChittiOS | `a282d7907c93f39fa1e2ce42741132276e02bb4f` | Apache-2.0, `LICENSE` | local signature label is misleading |
| NexiOS | `624b9e5f5209e51088754a5f3672c34b1c5df880` | GPL-3.0-or-later, `LICENSE.txt` | active x86 CI is stronger than first report stated; cap semantics are weaker |
| RustOS | `73db2d5fec0e4dfa72c4c8ccf1cffc17e932861c` | MIT, `LICENSE` | formal breadth survives; PR is not a fresh KVM product run |
| Zinnia | `1c10cf45381092aa3bb0388b6b63af10859a0e22` | GPL-2.0-only, `LICENSE` | uACPI gitlink is initialized at `022bc38ee675da1c63e6a6567e510cf58b3d2f7e` |
| Skift | `e3822171165cef5535e25602794750c4df1e3530` | LGPL-3.0-or-later, `license.txt` | capability receive defect remains source-level, not reproduced |
| Fudge | `5ffb39c7b608a3b117a724d11dc6725857b6c6de` | MIT, `LICENCE:1-22` | per-task isolation exists; Travis only builds |
| TacOS | `c10e450debf628233f5b345e2232dc1fc5d28619` | MPL-2.0, `LICENSE:1` | no CI or real test suite found |
| NyauxKC | `94879b5b2c5cb1f0ee7c9a52cc73273c574ed4c0` | ISC-style text, `LICENSE:1-12` | copyright names mintsuki; reuse provenance needs review |
| Cyjon | `b164a3a8d7b1cff0e853944a11078dd749104019` | custom attribution/non-commercial/no-derivatives, `LICENSE:3-15` | Fern-Night pinned at `00298942749a1d6a913f8b5dbc3d6483a4c57b96`; study only |

License conclusions in the companion report stand. No implementation code from these repositories should be copied into zl/zlOS without a separate license and provenance decision.

## 1. ChittiOS

### Claims that survived

- `[S][R][T]` The tool path is genuinely grammar/registry/capability/taint/audit structured, and CI boots in-kernel tests on x86_64 and AArch64. `.github/workflows/ci.yml:39-121` connects both targets. End-to-end scenarios remain intentionally outside CI (`:3-7`).
- `[S][R][T]` Subagent dispatch attenuates requested grants against the passed parent grant and reaps its task (`kernel/src/agent/subagent.rs:66-165`); widening refusal is tested at `:259-273`.
- `[S][R]` The bounded, process-local audit characterization remains correct. It should not be upgraded to durable external attestation.

### Refutations and omissions

1. **“Skill installation verifies ECDSA P-256” is too broad.** `[S][R]` The local package signer writes `SigAlgo::Ed25519`, but verification dispatches that value to a custom symmetric MAC (`kernel/src/skills/package.rs:102-128`). The MAC key is baked into the kernel and the source explicitly says this is not off-device authenticity (`kernel/src/skills/crypto.rs:1-16,20-24,82-117`). Real P-256 verification exists (`crypto.rs:119-167`), but the only baked key is a test publisher (`:136-145`). The correct statement is: *built-in packages use a self-contained integrity MAC; the public index can use P-256; asymmetric foreign-package delivery is incomplete*.
2. **The public registry is discovery/index verification, not a complete downloaded-package pipeline.** `[S]` `kernel/src/skills/registry_client.rs:17-29` says the foreign postcard download/deserialization/P-256 verification path is the next increment and resolves current entries from a built-in signed catalog.
3. **Install can report success after placement failure.** `[S][R][I]` `install()` discards `pkg.place_trusted()`'s `Result` (`kernel/src/skills/install.rs:104-106`) and then records `verified: true`, persists the record, logs “installed,” and returns `Ok` (`:124-147`). `place_trusted()` can fail while registering L0 metadata after already writing bodies/assets and registering tools (`kernel/src/skills/package.rs:131-155`). This is a non-atomic partial-install/false-success path.
4. **Uninstall is incomplete.** `[S][R][I]` `uninstall()` removes the install record, agent role, and skill index (`kernel/src/skills/install.rs:150-157`) but does not delete placed body/assets/agent-home data or deregister bundled tools registered globally by `kernel/src/skills/package.rs:159-169`. Revoked grants reduce authority, but stale code/data and registry entries remain.
5. **`Orchestrator::kill()` does not implement its own teardown comment.** `[S][R][I]` It only writes a trace (`kernel/src/agent/orchestrator.rs:206-210`); it does not mark `self.caller` dead or revoke its cap table. Unless another path owns that transition, the parked root-capability task remains live.
6. **Subagent parent authority is captured from the manifest, not re-read from live task capabilities.** `[S][R][I]` `router()` clones `self.manifest.capabilities` and passes that list to dispatch (`kernel/src/agent/orchestrator.rs:217-237`). This is safe only while the root's live cap table exactly matches the manifest. Future runtime revocation or attenuation could be re-granted to a child unless dispatch is changed to derive from live handles.

### Missing test obligations

- Placement failure must leave no record, body, asset, tool, role, or index entry.
- Uninstall must prove all installed artifacts and tool registrations are unreachable.
- Killing an orchestrator must prove its task is dead and every granted handle is stale.
- Runtime parent revocation followed by subagent spawn must prove no authority resurrection.
- Signature tests must distinguish local-MAC integrity from publisher-authenticated P-256 packages.

### Clean-room lesson

Keep Chitti's typed-plan and policy-boundary shape, but make installation a staged transaction: verify package, stage immutable objects, validate all registrations, atomically publish, and roll every artifact back on failure. Derive child authority from current live handles, never a manifest snapshot.

## 2. NexiOS

### Claims that survived or strengthened

- `[S][R][T]` The x86 test suite is an active CI gate, not merely a checked-in pass claim. `.github/workflows/ci.yml:64-83` builds x86 debug and runs `selftest`, `all-1`, and `all-2` under timeouts.
- `[S][T][V-host]` The main `expect` block requires `PLANNED == EXECUTED` and `FAILED == 0`, and timeout/EOF fail (`tools/run-test.exp:63-89`). A local `/bin/true` probe produced `RESULT: QEMU_EXIT` with exit 1. Lines `91-109` are stray duplicate dead text, but they did not prevent the active block from failing closed in this probe.
- `[S][R][T]` Fixed pools, ready-bitmaps, deadline bookkeeping, and explicit bounded structures remain valuable patterns. This pass did not establish end-to-end hard real-time behavior.

### Refutations and defects

1. **`CAP_RIGHT_COPY` is checked on the destination CNode, not the source cap.** `[S][R][I]` The right is documented as “may copy” (`src/kernel/cap/cap_types.hpp:41-47`). `do_copy_pinned()` looks up the source with required rights `0`, masks requested rights, and installs the same target (`src/kernel/cap/cap.cpp:237-279`). `sys_cap_copy()` and `sys_cap_mint()` instead require COPY on `dst_handle` (`src/kernel/syscall/syscall_handlers_cap.cpp:62-76,89-106`). A caller with a COPY-authorized destination CNode can therefore duplicate or mint from a source slot lacking COPY.
2. **The apparent denial test does not test the source-right boundary.** `[T]` `sys_cap_rights_denied_returns_minus1` installs a READ-only *Task* cap and aliases it as `dst_handle`; failure follows because the destination lookup requires a CNode, exactly as its own comment says (`src/kernel/test/test_cap_syscall.cpp:383-420`). It would still pass if source COPY were never enforced—which is the current behavior.
3. **Cascade revoke's worklist bound is also an undocumented width bound.** `[S][R][I]` A CNode has 64 slots but the traversal stack holds eight CNodes (`src/kernel/nexios_config.h:497-508`; `src/kernel/cap/cap.cpp:66-91`). Direct child objects are marked revoked, but after eight child CNodes are queued, remaining child subtrees are not traversed. Their grandchildren can stay live. The test covers only parent -> one child -> one leaf (`src/kernel/test/test_cap_core.cpp:171-195`).
4. **There is no implemented ordinary-cap derivation tree.** `[S]` `CNode::parent_` and `depth_` exist (`src/kernel/cap/cap.hpp:47-52`), but source search finds no assignment to `parent_`; copy/mint installs another reference to the same object. Revocation is object-global plus nested-CNode traversal, not derivation-descendant revocation. The companion report's “CSpace derivation/revocation” wording and recommendation to use NexiOS as the derivation-tree exemplar must be removed.
5. **Multiarchitecture validation is non-blocking.** `[T]` Renode loads x86_64/AArch64/RISC-V platform descriptions under `|| true` (`.github/workflows/ci.yml:55-62`). Only x86 is compiled and boot-tested in the shown CI job.

### Missing test obligations

- Copy and mint from a source without COPY must fail even when the destination CNode has COPY.
- Test a root with 9 and 64 child CNodes, each with a leaf; every leaf must become stale or the syscall must return an explicit exhaustion error before mutation.
- Test depth 8 and 9 independently from breadth; never silently truncate revoke.
- Test a real derivation chain where revoking one ancestor does not globally revoke unrelated aliases.

### Clean-room lesson

Do not copy NexiOS's current CSpace mechanics. Copy its boundedness discipline only after specifying two separate bounds—depth and total visited nodes—and returning a deterministic error rather than silently skipping descendants. Put source-right checks beside source lookup, then independently check destination insertion authority.

## 3. RustOS

### Strongest claims that survived

- `[S][R][T]` The repository genuinely has a strict, broad evidence gate. PR CI runs repository checks, `cargo xtask check`, selected host tests, pinned Kani/Verus setup, impact mapping, and `formal/verify-all.sh --profile pr` (`.github/workflows/rust.yml:51-127`).
- `[S][T]` The PR formal profile runs TLC, source conformance, specification and implementation mutations, fault and recovery scenarios, ABI differential checks, Kani, Verus, concurrency checks, and runtime-control traces (`formal/verify-all.sh:83-113`). It collects child exit status and fails if any lane fails (`:65-80`).
- `[S][R]` The service graph and separate loader/process/network/storage authorities are real source and staging structure. This pass found no new critical authority-boundary defect comparable to NexiOS or TacOS.

### Corrections and omissions

1. **PR formal coverage is not the same as the nightly set.** `[S][T]` Sanitizers, Miri, Apalache, TLAPS, and fuzz smoke only run for `--profile nightly` (`formal/verify-all.sh:114-120`). Do not list them as ordinary PR gates.
2. **The PR “runtime trace” lane does not require a fresh KVM boot.** `[S][T]` It always generates a host `runtime-control` test trace (`formal/run-runtime-traces.sh:11-14`). A KVM P0 trace is checked only if already present, and absence fails only with `FORMAL_REQUIRE_KVM_TRACE=1` (`:15-43`); the PR workflow does not set that variable. Thus PR formal green is strong model/source/host evidence, not a product boot receipt.
3. **Windows compatibility was materially undercounted.** `[S][R]` `services/loaderd/src/pe_loader.rs:3-51` loads a PE main image, preloads a system-DLL registry, resolves the import closure, builds a Windows runtime blob, patches CRT exports, and maps main/DLL pages. Header admission bounds sections, alignment, image size, address range, section table, and checked arithmetic (`:67-218`). This is more than “separate ELF/PE rules.”
4. **POSIX breadth is still incomplete.** `[S][R]` `procd` returns `ENOSYS` for unsupported `execveat` flags and any non-`AT_FDCWD` dirfd (`services/procd/src/main.rs:425-433`). This is an honest boundary, not a defect, but prevents broad compatibility wording.

### Clean-room lesson

RustOS remains the best evidence-process reference here. For zlOS, separate model/source/host evidence from current boot evidence in the receipt schema, and require KVM/hardware evidence explicitly rather than allowing an optional retained trace to inherit the word “runtime.”

## 4. Zinnia

### Corrections to the audit boundary

- `[S][R]` The uACPI submodule is present in the supplied checkout at gitlink `022bc38ee675da1c63e6a6567e510cf58b3d2f7e`. The companion report's snapshot ledger is correct; its later statement that the submodule is absent must be deleted.
- `[T]` CI recursively checks out submodules, formats/lints, and builds only x86_64 (`.github/workflows/ci.yml:10-46`). No boot or test execution is shown.
- `[S][R]` The workspace glob and default build connect the kernel and all driver crates; drivers are not merely loose source files. Runtime success remains unverified.

### Defects and maturity gaps

1. **The executable-loader trust boundary can panic on a malformed ELF.** `[S][R][I]` `ElfFormat::load_file` checks only `EI_VERSION` and machine, with a TODO for the remaining identity checks (`kernel/src/vfs/exec/elf.rs:359-378`). It allocates the program-header buffer from untrusted `e_phentsize`, then unconditionally reads an `ElfPhdr` from it (`:387-395`) without requiring `e_phentsize == size_of::<ElfPhdr>()`. A short entry can trigger a kernel panic; magic, class, data encoding, header size, and several checked-offset invariants are also absent.
2. **uACPI source presence is not mature ACPI support.** `[S][R]` Mutex and event constructors allocate dummy integers; mutex free/release are no-ops; thread ID is always zero; acquire always succeeds; event operations and firmware requests are `todo!()`; interrupt installation returns success without installing anything (`kernel/src/device/acpi/uacpi.rs:223-303`). These fake-success hooks are worse than an explicit unsupported error because they let uACPI proceed under false synchronization/IRQ assumptions.
3. **Range page-table operations are deliberately non-transactional.** `[S][R]` `map_range`, `remap_range`, and `unmap_range` state this in TODOs and stop after the first per-page error, leaving earlier pages mutated (`kernel/src/memory/virt/mmu.rs:323-372,424-448`). Any higher-level atomicity claim must exclude these primitives.
4. **Dynamic modules are build-reachable but incomplete.** `[S][R]` The loader checks that dependencies are already present rather than loading them (`kernel/src/module.rs:355-377`), does not publish new symbols (`:446-449`), does not invoke init arrays (`:459`), and the module-remove syscall is `ENOSYS` (`kernel/src/syscall/mod.rs:300`). Numerous untrusted offset/index slices in `module.rs:336-434` also need admission hardening.

### Clean-room lesson

Keep Zinnia's narrow driver-crate seams, not its current loader/ACPI readiness assumptions. Any foreign parser or firmware library must enter through a complete OS adaptation contract whose unimplemented operations return explicit failure. Host-fuzz ELF/module admission before moving it into ring 0.

## 5. Skift

### Confirmed original finding

- `[S][R][I]` The capability receive defect is real in source. `Domain::_availableUnlocked()` increments for occupied slots (`src/kernel/hjert/core/domain.cpp:98-106`). `Channel::recv()` interprets that result as free capacity, then unwraps `_addUnlock()` (`src/kernel/hjert/core/channel.cpp:87-119`). An empty destination rejects any cap transfer; a sufficiently occupied destination can pass the check and then panic when actually full.
- `[S][R]` Send-side acquisition rollback is deliberate (`channel.cpp:52-84`), but it does not rescue the receive side. The report must not call current transfer transactional as an implemented property.

### Additional defect

- **Address-space mapping is not failure-atomic.** `[S][R][I]` `Space::map()` removes/reserves the virtual interval before calling hardware `mapRange` and flush (`src/kernel/hjert/core/space.cpp:81-109`). Either failing return exits through `try$` without restoring `_ranges`; a partially completed hardware map is also not rolled back. The logical map is appended only after both calls succeed. This can leak virtual ranges and leave logical/hardware state divergent.

### Scope correction

- `[S]` Much of the Karm/Hideo/Vaev product layer is fetched from lockfile-pinned external projects, not present in this source snapshot. The checked-in Skift system adapter itself still returns unsupported for major pipes, directory, spawn, PTY, networking, mapping flush, sandbox, DNS, and bundle operations (`src/libs/karm-sys/skift/sys.cpp:68-282`). Keep product claims outside the audited boundary.

### Clean-room lesson

Skift remains useful for typed object APIs and pledge-style narrowing, not as a proven transfer implementation. zlOS transfer needs destination preallocation, a journaled commit point, and invariant tests comparing logical handles, object references, and both endpoint queues after every injected failure.

## 6. Fudge

### Refuted characterization

- **Fudge does have real process address-space isolation machinery.** `[S][R]` Each task receives its own mapping storage, code, stack, and mmap area (`src/kernel/x86/arch.c:697-715`). Task creation copies the kernel mapping base, adds code/user stack/mmap entries, and loads a ring-3 task (`:126-203`). The scheduler restores ring-3 selectors and switches to the selected task's page directory (`:219-264`); GDT and syscall interrupt setup explicitly define user code/data and a ring-3 `int 0x80` gate (`:652-683`). The companion table's “weak address-space story” must become “real per-task ring-3 isolation, but ambient resource-ID authorization and globally shared kernel regions.”

### Defects and omissions

1. **ELF validation is only four magic bytes.** `[S][R]` `lib/fudge/elf.c:4-10` validates nothing else. `src/utils/elfload.c:155-224` trusts section `info`, `link`, symbol indexes, sizes, offsets, and entry sizes; `relocationheader->size / relocationheader->esize` can divide by zero (`:178`). A malformed module/executable can cause host-tool or target failure and unchecked reads/writes.
2. **Oversized receive silently destroys the message.** `[S][R]` Kernel mailbox `pick()` consumes the header and skips the body when the caller's buffer is too small (`src/kernel/mailbox.c:18-39`). The user library converts `MESSAGE_TOOBIG` into `continue` rather than returning the condition (`lib/abi/channel.c:48-79`), so the caller loses the message and blocks waiting for a later one.
3. **Mailbox backpressure is coarse and lossy.** `[S][R]` `place()` reports `MESSAGE_TOOBIG` both for a permanently oversized message and for a temporarily full ring (`src/kernel/mailbox.c:49-84`); `channel_place()` returns zero for either (`lib/abi/channel.c:82-113`). The protocol cannot distinguish retryable pressure from invalid size.
4. `[T]` `.travis.yml:1-5` only runs `make` under GCC and Clang. No unit, malformed-input, isolation, or boot-verdict suite was found.

### Clean-room lesson

The event/callback surface remains worth studying, now paired with its actual isolation layer. zlOS messages should preserve an oversized head until explicitly dropped, expose required length, distinguish `WOULD_BLOCK` from `MESSAGE_TOO_LARGE`, and test that a malicious task cannot access another task's mappings or ambient resource IDs.

## 7. TacOS

### Confirmed and expanded defects

1. **ELF writable permissions are inverted.** `[S][R][I]` `execve` sets the page-table write bit only when the ELF segment does *not* carry `ELF_FLAG_WRITABLE` (`kernel/src/tasks/exec.c:109-120`). `KERNEL_PFLAG_WRITE` is x86 bit 1 (`kernel/include/mem/paging.h:10`) and `map_pages()` writes supplied flags into the leaf PTE (`kernel/src/mem/paging.c:109-160`). This is not naming ambiguity.
2. **Negative syscall numbers bypass bounds checking.** `[S][R][I]` The `int 0x80` handler compares user-controlled `rax` to `num_syscalls` with signed `jge`, then indexes and indirectly calls `[syscalls + rax*8]` (`kernel/src/tasks/syscall_handler.asm:18-31`). Any value with the sign bit set passes the upper-bound test and addresses memory before the table. This is a ring-3-triggerable kernel crash and potentially an indirect-control-flow vulnerability.
3. **ELF `p_filesz > p_memsz` overflows the allocated segment buffer.** `[S][R][I]` Allocation is based on `size_in_memory`, while `vfs_read` copies `size_in_file` with no comparison (`kernel/src/tasks/exec.c:98-108`). The loader also trusts program-header count/size/offset arithmetic and uses global `file_header`/`program_header` objects (`:11-12`), making concurrent SMP exec calls race.
4. **ELF write inversion is not the only loader validation gap.** `[S]` `verify_elf()` checks magic, endian, class, and machine only (`kernel/src/tasks/exec.c:14-27`); it does not validate file type, header sizes, segment bounds, overflow, canonical user ranges, overlap, alignment, or entry-point membership.
5. `[R]` The default Makefile does connect kernel, libc, every userspace directory, libraries, initrd, and ISO (`Makefile:1-51`), so the small end-to-end architecture description stands statically. The bootloader is fetched from the moving `v9.x-binary` branch (`:14-17`), weakening reproducibility.
6. `[T absent]` No CI workflow or kernel test suite was found. `ipctest` and `kbqueuetest` are user programs, not an automated verdict gate. `lint-signatures` is also written as a grep followed by `... || true` (`Makefile:66-73`), so it never fails the build.

### Clean-room lesson

TacOS remains useful only as a compact connected product graph. For zlOS, syscall dispatch must use an unsigned range check and generated table length, then validate every complete user span before dereference. ELF admission must validate all sizes with checked arithmetic, require `filesz <= memsz`, enforce user-range/W^X invariants, and construct process state privately before one atomic publish.

## 8. NyauxKC

### Confirmed original findings

1. **PCI BAR coordinates are reversed.** `[S][R][I]` Enumeration stores `(bus, slot, function)` (`kernel/src/pci/mod.rs:383-409`), and ordinary virtio config accesses use `.0,.1,.2` in that order (`kernel/src/virtio/mod.rs:393-445`). `pci_map_bar()` alone calls `pci_read_dword(location.2, location.1, location.0, ...)` (`kernel/src/pci/mod.rs:65-75`). Nonzero bus/function devices therefore read the wrong configuration function.
2. **Four-architecture recipes outrun source.** `[C][S][R partial]` The root Makefile offers x86_64, AArch64, RISC-V, and LoongArch QEMU/image paths (`GNUmakefile:34-143`), but Rust's arch module only exposes x86_64 and RISC-V (`kernel/src/arch/mod.rs:16-19`), and the toolchain file comments out AArch64 and LoongArch targets (`kernel/rust-toolchain.toml:1-8`). Linker scripts and QEMU recipes are not ports.

### Stronger parser and feature corrections

1. **USTAR regular-file contents are copied from the header, not the payload.** `[S][R][I]` `transmute_to_ramfs()` receives a slice beginning at the 512-byte header, parses `fz`, and writes `&ptr[..fz]` into the new file (`kernel/src/vfs/ustar.rs:89-127`). Correct payload starts at least 512 bytes later. Imported file data is therefore corrupt even for a well-formed non-empty archive.
2. **USTAR is panic-prone on routine/malformed cases.** `[S][R]` It indexes the first module, trusts path text containing “tar,” slices without admitted lengths, unwraps C strings/octal fields, reads `ptr[..1024]`, never checks checksum, and explicitly panics on empty regular files (`kernel/src/vfs/ustar.rs:41-78,89-147`). The companion report's “fragile” label should be upgraded to a concrete broken importer.
3. **Intel IOMMU source is active but not DMA isolation.** `[S][R partial]` Boot calls `iommu_init()` on Intel (`kernel/src/main.rs:180-186`). It builds root/context tables in passthrough domain 1 and enables translation, then ends with `TODO: actual page tables for the context entries` (`kernel/src/arch/x86_64/intel/iommu/mod.rs:55-160`). Credit the experimental bring-up, but do not call it isolation.
4. **Virtio-GPU is exactly bring-up/query scope.** `[S][R]` It finds the GPU, negotiates version 1, constructs queue 0, and sends only `GET_DISPLAY_INFO` (`kernel/src/virtio/mod.rs:393-625`). No resource creation, backing attachment, scanout, transfer, flush, cursor, or interrupt path appears. The companion report's “incomplete query” wording is accurate.
5. `[T weak]` The Cargo test target exists (`kernel/GNUmakefile:37-39`), but no `#[test]` or test modules were found. Boot-time slab/VMM assertions (`kernel/src/main.rs:146-179`) are embedded smoke checks, not a test suite. No CI workflow was found.

### Clean-room lesson

Use named `PciLocation` fields and parser-returned bounded views. Archive admission must validate header availability, checksum, type, octal fields, path policy, `header + payload + padding <= archive_len`, and empty files; copy from the admitted payload slice only. IOMMU evidence must prove denied DMA, not merely enabled translation in passthrough.

## 9. Cyjon

### Claims that survived

- `[S][R]` The parent kernel is genuinely one readable assembly translation unit: `kernel/init.asm:9-98` includes the modules, and `_entry` explicitly walks Limine, memory, ACPI, paging, GDT/IDT, streams, tasks, IPC, devices, storage, libraries, daemons, init, and SMP (`:107-184`).
- `[S][R]` Fern-Night is not decorative. Root `make:20-24` builds the pinned submodule and root `make:50-54` places its `system.vfs` in the Cyjon ISO. The omission boundary in the companion report is correct.
- `[S][R]` The syscall index check uses unsigned `jnb` (`kernel/syscall.asm:15-17`), so unlike TacOS it rejects both high and sign-bit-set values.

### Defects and omissions

1. **Build success is untrustworthy at two layers.** `[S][R]` Top-level `Makefile:5-6` runs `./make || true`. Inside that script, submodule update, Limine compilation, the entire Fern-Night build, `strip`, copies, ISO creation, and bootloader installation are not consistently checked (`make:20-24,44,50-57`). A missing/stale Fern image or failed ISO command can be followed by later commands, and the outer make will still exit zero.
2. **Raw user pointers can halt the whole OS.** `[S][R][I]` Syscall services directly write through caller-provided `rdi`; for example memory information writes two qwords with no user-span validation (`kernel/service.asm:43-69`), framebuffer and mouse do the same (`:128-151,189-267`). A bad pointer page-faults in ring 0. The common exception handler immediately loops forever (`kernel/idt.asm:5-18`), so one unprivileged task can halt a CPU—and likely the usable system—rather than being terminated.
3. **ELF validation is only magic.** `[S][R]` `lib_elf_identify()` checks four bytes (`library/elf.asm:21-39`). Execution trusts header counts, offsets, segment sizes, virtual addresses, and file boundaries; segment copy uses unchecked source/destination/length (`kernel/exec.asm:320-367`). Bundled VFS input reduces exposure but does not make this a safe loader.
4. **The exception model is not process isolation.** `[S][R]` Every registered CPU exception converges on the global infinite loop (`kernel/idt.asm:10-18`; handlers `:22-230`). A user divide-by-zero, invalid opcode, or page fault therefore cannot be described as isolated task failure.
5. `[T absent]` No CI or tests exist. QEMU/Bochs launchers and serial logging provide manual observability only.
6. `[C][S]` Version drift remains: README says 0.2165 while `kernel/config.inc` declares 2167. The “fully compatible” Fern-Night claim is only a pinned-submodule integration, not a cross-repository compatibility test.

### Clean-room lesson

Retain the explicit boot order and compact layouts. Reject its fault and ABI boundary: every user pointer needs canonical-range and full-span validation, copyin/copyout must return errors, user exceptions must terminate only the current process, and a serial boot verdict must be propagated through every build wrapper.

## Cross-repository correction matrix

| Original reusable strategy | Adversarial correction before zlOS adoption |
|---|---|
| Chitti signed skills | Distinguish local symmetric integrity from publisher authenticity; make package publish/unpublish atomic |
| Chitti subagent attenuation | Derive from live handles, not manifest grants; prove kill/revoke closes the task |
| NexiOS CSpace derivation/revoke | Not a current derivation-tree exemplar; fix source COPY check and separate traversal depth from breadth |
| RustOS runtime/formal evidence | Preserve exact lane/profile labels; require fresh KVM/hardware receipt separately |
| Zinnia modular drivers | Keep crate seam; reject stubbed OS hooks and kernel-hostile parser panics |
| Skift transactional transfer | Design intent only at this commit; implement and inject-fault-test receive commit/rollback |
| Fudge weak/no address spaces | Correct to real ring-3 per-task mappings; remaining weakness is authorization/protocol robustness |
| TacOS compact product smoke | Keep connectivity only; replace syscall/ELF boundary entirely |
| Nyaux typed-hardware warning | Also require archive payload correctness and deny-DMA IOMMU proof |
| Cyjon readable instruction-level state | Add per-process fault recovery, validated copyin/out, and strict gate propagation |

## New highest-priority adversarial tests for zlOS

1. **Capability matrix:** for every operation, vary source right, destination right, object type, stale generation, and foreign table. Specifically deny copy/mint when the source lacks transfer authority.
2. **Revoke shape:** chains at max depth and one beyond; 0, 1, max, and max+1 siblings; mixed shared aliases; injected worklist exhaustion. Never return success with reachable descendants.
3. **Transactional package install:** fail each write/registry/persist step and compare pre/post bodies, assets, tools, roles, records, grants, and handles.
4. **ELF/PE/module admission:** fuzz undersized headers, zero entry size, `filesz > memsz`, overflowed counts/offsets, overlapping mappings, noncanonical addresses, W+X, bad entry points, and concurrent exec.
5. **User ABI:** negative/high syscall numbers; first/last byte unmapped; wraparound pointers; output faults; cross-page buffers; exception while inside copyin/out. Only the current process may die.
6. **IPC transfer:** empty/full destination tables, bad nth handle, receiver buffer too small, closed peer, rollback failure, and retry after pressure. Message and handle queues must be byte-for-byte unchanged on failure.
7. **Archive import:** empty file, checksum failure, truncated header/payload/padding, huge octal value, `..`, absolute path, duplicate, hard/symlink, and exact payload hash.
8. **VM atomicity:** fail allocation/map/shootdown/flush on every page of a range and compare logical allocation, PTEs, TLB receipt, and object references.
9. **Evidence gate:** deliberately fail compiler, dependency build, image creation, bootloader install, boot milestone, and test summary; every outer command must return nonzero and must not accept a stale artifact.

## Exact corrections to propagate into the companion audit

1. At executive result line 11 and Chitti line 85, replace “signed skills / verifies an ECDSA P-256 signature” with: **“built-in packages use a baked-key symmetric integrity MAC despite an `Ed25519` schema label; signed public indexes and P-256 package verification exist, but foreign downloaded package ingestion is unfinished.”** Add ignored placement error, incomplete uninstall, no-op kill, and manifest-vs-live subagent authority as gaps.
2. At executive result line 12, subsystem table line 60/61, repository line 108, best-strategies line 314, and disposition line 407, remove NexiOS as evidence of a working derivation tree. Add: **source COPY/MINT right is not checked; revoke silently truncates broad nested-CNode traversal at eight queued nodes; the current denial test misses the source-right case.** Credit active CI execution of `selftest`, `all-1`, and `all-2`.
3. In RustOS evidence wording, state that **PR formal runs TLC/conformance/mutations/fault/recovery/ABI/Kani/Verus/concurrency/host traces, while sanitizer/Miri/Apalache/TLAPS/fuzz are nightly and KVM trace is optional unless explicitly required.** Add the substantive PE DLL/import/CRT runtime loader as an implemented omission.
4. Delete any Zinnia sentence saying the supplied checkout lacks uACPI; its gitlink is initialized. Replace with: **submodule and adapter are build-reachable, but synchronization, events, firmware requests, and IRQ hooks are stubbed or panic.** Add malformed ELF `e_phentsize` panic and nontransactional range mappings.
5. At Skift best/disposition lines 320 and 410, replace “transactional handle transfer” as an implemented strength with **“typed transfer design with send-side rollback; receive is currently broken by inverted capacity accounting.”** Add the address-space reservation rollback defect.
6. At Fudge executive/table/disposition lines 16, 61, and 411, replace “weak address-space story” with **“real ring-3 per-task page tables and CR3 switching, but ambient resource IDs, globally shared kernel regions, weak message error semantics, and no verification.”** Add ELF divide-by-zero/index trust and oversized-message destruction.
7. Keep TacOS's write-bit inversion, and add two higher-severity items: **signed negative syscall index reaches memory before the service table; `p_filesz` can overflow a `p_memsz`-sized allocation.** Note global ELF header state races under SMP and moving Limine branch weakens reproducibility.
8. Keep NyauxKC's PCI tuple reversal and two-source-architecture conclusion. Upgrade USTAR from “fragile” to **“functionally corrupt: regular-file writes start at the tar header, plus unchecked slices/unwraps/panics.”** Credit IOMMU bring-up only as active passthrough, not DMA isolation.
9. Keep Cyjon's false-green and license conclusions. Add: **syscall index bounds are correctly unsigned, but raw user output pointers plus a global infinite-loop exception handler let a user fault halt the system; ELF validation is magic-only.** Record exact Fern-Night gitlink `00298942749a1d6a913f8b5dbc3d6483a4c57b96`.
10. Update the integrated zlOS plan so capability/revoke tests include rights matrices and breadth exhaustion; install/uninstall is transactional; every parser is length-admitted before allocation/mapping; all user faults are process-scoped; and “runtime/formal” receipts name whether they came from host simulation, fresh QEMU/KVM, or hardware.

The companion report's final architecture direction remains sound after these corrections: small typed kernel mechanisms, isolated services, least-authority handles, explicit bounds, and evidence receipts. The adversarial pass makes one change to emphasis: **transactionality and trust-boundary admission must be treated as kernel architecture, not implementation cleanup.**
