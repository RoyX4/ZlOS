# Persistent storage and the Files app

zlOS has one real filesystem: `zlfs` in `fs.c`. The desktop Files app is its
primary UI. The ten numbered RAM editor slots remain only so old `edit <n>`
workflows keep working.

## On-disk model

`zlfs` stores a checksummed superblock, a flat directory of 32 entries, and
contiguous file runs on NVMe. Names are at most 23 bytes. A growing file either
extends in place or moves to a new contiguous run; `hosttest/fstest.c` covers
failed relocation, partial writes, directory-write rollback, corrupt metadata
and a separate-process remount.

The Files app calls `nv_setup()` and `fs_mount()` when it opens. `nv_setup()`
is idempotent after a complete setup so a mount probe followed by format or
refresh does not try to recreate live NVMe queue IDs. Files does not
silently format an unknown or damaged volume. On an unmounted volume Shift+F is
the explicit destructive action; after that:

- `N` stages a name and creates an empty file
- Enter opens the selected file in zlEDIT
- `D` or Delete removes the selected file
- Up/Down select; `R` remounts/refreshes
- clicking a row selects it

## Why filenames cross a byte bridge

The zl kernel subset has string literals but no runtime string values. Passing
a filename as a zl value would fault. `fs.c` therefore owns one staged name and
exports `fs_name_clear`, `fs_name_push`, `fs_name_pop`,
`fs_name_len` and `fs_name_stage_byte`. `runtime_kernel.c` exposes those
as `fs_nclear`, `fs_npush`, `fs_npop`, `fs_nlen` and `fs_nch`.

Files uses that bridge both ways: keyboard input pushes bytes into C; drawing
reads them back one byte at a time. No second filesystem or string
representation was added.

## zlEDIT modes and limits

RAM mode still loads and saves `FS_DATA` slots. Disk mode records a `zlfs`
directory slot, reads it into `EDIT_BUF`, and writes the real directory entry
on Ctrl+S or ESC. Save failure is visible and prevents ESC from closing the
window.

`EDIT_BUF` is 8,000 bytes. One byte is reserved for the terminating zero, so
Files refuses to open entries of 8,000 bytes or more. The editor still appends
at the end; it has no caret or selection model. Ctrl+C copies the whole
document and Ctrl+V appends the clipboard. Those are real clipboard operations,
not a claim that selection editing exists. The clipboard is 4,096 bytes; an
oversized copy or a paste that would overflow the editor is refused visibly
instead of publishing partial text.

## Verification

Fast checks:

```bash
cd kernel/hosttest
gcc -O2 -g -Wall -Wextra -Wno-unused-parameter -DFS_HOSTTEST \
  -o fstest fstest.c ../fs.c
./fstest
cd ..
./build.sh
```

The acceptance gate is deliberately a power cycle:

```bash
python3 probe-files.py
```

Boot 1 opens Files on a blank NVMe image, formats it, creates and deletes a
named file, creates `notes.txt`, exercises copy/paste and Ctrl+S, and then
kills QEMU. Boot 2 starts a new QEMU process against the same image, mounts on
Files open, reopens `notes.txt`, and requires the editor client pixels and the
post-save Files row to match exactly. An in-process unmount/remount is not
accepted.

`verify-disk.sh` remains the lower-level three-boot filesystem gate: its
`boots` file must report 1 → 2 → 3. The two gates answer different questions:
`verify-disk.sh` isolates zlfs/NVMe; `probe-files.py` covers the actual Files and
zlEDIT path.
