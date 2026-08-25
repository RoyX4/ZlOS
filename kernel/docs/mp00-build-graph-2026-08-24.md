# MP-00 source, object and artifact build graph

## Outcome

[`../metadata/build-graph.json`](../metadata/build-graph.json) closes the previous gap between an
input hash list and final artifact list. It currently records:

- all 123 build-identity inputs;
- all 62 C compilation units declared by `SOURCES`;
- 4 target lanes;
- 268 lane-qualified logical objects;
- all 9 artifact-registry outputs;
- 404 total nodes and 663 typed edges;
- 0 orphan source inputs;
- 1 explicitly scope-only conservative input.

The graph is `PASS_WITH_DECLARED_SUPERSET`, not per-object binary provenance.
Its object edges are derived from the active build scripts and source/include
graphs. Final artifact bytes remain proven by the separate two-build
reproducibility receipt.

## Why objects are lane-qualified

The 32-bit Multiboot and raw-BIOS scripts reuse physical filenames such as
`_gen.o` and `_rt.o`. A path-only graph would merge two different build actions
and make one lane appear to prove another. Logical object IDs therefore include
the lane even when their temporary on-disk paths match.

Each lane has 67 logical objects: generated zl C, 62 shared `SOURCES` units and
4 lane-specific runtime/entry/GDT/trampoline inputs. Those objects link into:

- `kernel.elf` for BIOS/UEFI GRUB Multiboot32;
- `kernel64.elf` for GRUB Multiboot64;
- `BOOTX64.EFI` for native UEFI64;
- `kernel_raw.elf` for the owned raw-BIOS loader.

Packaging edges then connect the kernels to `zlOS.iso`, `zlOS64.iso`,
`zlOS.img` and `zlOS-usb.img`, including boot-sector, deterministic media-ID
and reproducible-time helper inputs.

## Honest conservative superset

`freestanding/zl_freestanding.h` is in the build identity's deliberate header
superset but has no active include edge. The graph marks it
`declared-scope-only`; it does not call it compiled or silently discard it.

External angle-bracket headers such as GCC/Clang SIMD headers and the host
media helper's libc headers are owned by `toolchain-manifest.json`, where their
actual paths and hashes are measured. They are not relabelled repository source.

## Rejection proof

```sh
python3 kernel/tools/generators/gen-build-graph.py --check --selftest
```

The self-test removes, independently:

1. a declared source input;
2. a target lane;
3. every outgoing edge from one source, creating an orphan;
4. a shipped artifact node.

Each mutation fails. The complete landing gate regenerates and immediately
checks the graph before compilation, then joins it into the evidence registry.

## Remaining evidence ceiling

Open work remains explicit:

- no hash/receipt exists for each intermediate object;
- the one conservative header is scope-only;
- future packages, services, app bundles and self-hosted outputs are not current
  graph nodes.

EV-007 therefore remains partial even though every current declared input and
artifact has a graph position.
