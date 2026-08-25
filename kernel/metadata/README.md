# zlOS Metadata

This directory contains tracked, machine-readable project truth generated from
source, build recipes, receipts, and policy:

- build identity, graph, toolchain, dependency, wrapper, and license inventories
- application, artifact, initialization, event, and evidence registries
- accessibility, security, visual, observability, and adversarial proof ledgers
- decision, provenance, and release metadata

The generators live under `../tools/generators/`. These files are not handwritten
implementation and must be regenerated and checked together after source, path,
recipe, or evidence changes. A generated registry records what its evidence
supports; its presence alone does not promote a feature to complete.

`build-identity.json` is the deliberate exception to pre-build freshness
checking. It contains the current Git HEAD, branch, dirty state, compiler binary
and local tool versions, so a tracked snapshot cannot match every clean checkout
or CI runner. Each kernel build route regenerates it immediately before
compilation and embeds that exact identity. The committed file is a dated
snapshot, not permission to skip build-time generation.
