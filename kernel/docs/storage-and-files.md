# Persistent storage and the Files app

zlOS has one real filesystem: `zlfs` in `fs.c`. It stores a checksummed
superblock, a flat 32-entry directory and contiguous file runs on NVMe. The
Files app in `kernel.zl` is now the normal way into it.

## What works

- Open **Files** from the dock, start menu, or `files` shell command.
- An existing zlfs volume mounts when the app opens. A blank disk is not
  formatted silently: press `Shift+F` twice in the app to confirm formatting.
- Press `N`, type a name, then Enter to create a named file.
- Select with Up/Down or the pointer. Enter or double-click opens the file.
- The editor reads the file from zlfs. `Ctrl+S` saves; `Esc` saves and closes.
- `Ctrl+C` copies the whole document and `Ctrl+V` appends the clipboard.
- Delete is deliberately two-step: press Delete twice on the same selection.
- Names and bytes survive a power cycle. `probe-files.py` boots two separate
  QEMU processes against the same NVMe image to prove the full app path.

## Boundary

zl's kernel subset has string literals but no runtime string values. Filenames
therefore stay inside `fs.c`. zl pushes a name byte-by-byte through `fs_npush`
and paints it byte-by-byte through `fs_nch`/`fs_ch`. This is a narrow bridge,
not a second filesystem or a filename copy in application memory.

The editor buffer is capped at 8,000 bytes. Opening a larger file refuses with
a visible notification; it never truncates the file. zlfs still has a flat
directory, 23-byte ASCII names and 32 files maximum. Directories and growth
beyond those limits are separate filesystem work.

The old ten numbered RAM slots remain for `edit <n>` and text-only boot
compatibility. Do not build new features on them. Once the text-mode shell has
a named-file flow, delete `FS_META`, `FS_DATA`, `fs_init`, `fs_save`, and
`fs_load` together.

## Checks

```bash
cd kernel/hosttest
./build.sh
./fstest

cd ..
./probe-files.py
./verify-disk.sh
```

`fstest` covers the byte-wise name bridge and filesystem failure handling.
`probe-files.py` covers create → edit → save → power off → mount → reopen.
`verify-disk.sh` independently proves named-file persistence over three boots.
