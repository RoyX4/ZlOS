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
