# Physical-debug QEMU exercise evidence — 2026-08-24

The raw QEMU captures produced while diagnosing the ThinkPad boot, terminal,
I219 quarantine, and network paths are preserved in
`physical-debug-qemu-evidence-2026-08-24.tar.zst`. The archive keeps every
original file without adding hundreds of individual 6.6 MiB PPM frames to Git.

Archive contents:

- `uefi-boot-order-fix/`
- `uefi-i219-quarantine/`
- `uefi-network-boot-fix/`
- `uefi-terminal-input-double-check/`
- `uefi-terminal-input-final/`
- `uefi-terminal-input-fix/`

SHA-256:

`b532db579734aeede053120a2031d06bc62c3b6700b5e6419fad5475676336cb`

Restore with:

```sh
tar --zstd -xf physical-debug-qemu-evidence-2026-08-24.tar.zst
```
