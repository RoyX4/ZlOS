# Performance architecture exercise evidence — 2026-08-24

The raw QEMU exercise directories from the performance-architecture worktree
are preserved in `performance-architecture-evidence-2026-08-24.tar.zst`.
They are archived instead of committed as hundreds of individual 6.6 MiB PPM
frames.

Archive contents:

- `bios-performance-architecture/`
- `bios-performance-architecture-final/`
- `repro-editor-file-final/`
- `repro-smp-input-gpu/`
- `repro-smp-input-gpu-final/`
- `repro-smp-input-gpu-fixed/`
- `repro-smp-input-gpu-fixed2/`
- `repro-smp-input-gpu-mwait/`
- `repro-storage-editor-final/`
- `repro-visual-storage-fixed/`

SHA-256:

`652707a96fb3f8f85ac1ab150eceaec870bf8b212a31437c0a622e37e6df020b`

Restore with:

```sh
tar --zstd -xf performance-architecture-evidence-2026-08-24.tar.zst
```
