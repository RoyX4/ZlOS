# Language Tests

These `.zl` programs exercise syntax, semantics, runtime behavior, and backend
agreement through the root test harness. Keep reusable code in `stdlib/` and
human-oriented demonstrations in `examples/`.

`run_tests.sh` is the authoritative entry point. A file being present here does
not prove every backend supports it; the harness output establishes that.
