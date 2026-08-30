# Benchmarks

This directory contains repeatable host-language performance programs and the
scripts that compare their outputs and timings.

- `run_bench.sh` measures the current boxed-C and interpreter paths.
- `ab.sh` compares two runtime implementations against identical generated C.
- `abtree.sh` compares complete checkouts when runtime layout also changes.
- `b*.zl` files are benchmark inputs, not standard-library modules or tests.

Performance results are evidence only when the harness records the machine,
revision, command, output agreement, and run conditions.
