# Physical touchpad ZLLOG archive — 2026-08-24

Source device: Imation `ImationFlashDriv`, serial `07B70D07914C6D7E`, stable
by-id path ending `usb-Imation_ImationFlashDriv_07B70D07914C6D7E-0:0`.

This directory preserves the recorder evidence before the next corrected image
reinitializes the USB stick:

- `zllog-all.json`: decoded records from every valid boot slot;
- `zllog-partition.raw.zst`: lossless compressed copy of the complete 512 MiB
  ZLLOG partition, including both superblocks and all boot slots;
- `SHA256SUMS`: hashes of both archived artifacts.

The live `/dev/sda2` partition and the decompressed archive both hashed to
`4583cf257b32fb0e9ac8afad7dda45d1ffe0d6087b93bcf126823906ac2430c8`,
so the compressed copy was independently read back before the stick was
reflashed.

The evidence interpretation and exact limits are in
[`physical-input-and-recorder-diagnosis-2026-08-23.md`](../../../../physical-input-and-recorder-diagnosis-2026-08-23.md).
