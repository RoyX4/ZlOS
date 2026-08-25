# MP-00 benchmark receipt

[`receipts/benchmark-host-2026-08-23.json`](../../receipts/benchmark-host-2026-08-23.json)
records the exact `wmbench` and `fbbench` executables, sources, raw output,
machine, load, sampling method, frame metrics and 60 Hz budget result.

The current result is `PASS_WITH_OPEN_REGRESSIONS`. Five of seven host metrics
fit 16.667 ms. The shipping serial full-desktop redraw exceeds that budget at
2560x1440 and 3840x2160. The four-band host experiment fits at all three modes,
but it is not the native zlOS route and is not promoted as one.

The receipt also says what it does not measure: percentiles, peaks and native
target frame time. Its self-test catches hidden regressions, target overclaim,
invented percentiles, missing executable identity and raw-output drift.

Run a fresh measurement only on a reasonably quiet machine:

```sh
cd kernel
python3 run-benchmarks.py --run --selftest
python3 run-benchmarks.py --check --selftest
```
