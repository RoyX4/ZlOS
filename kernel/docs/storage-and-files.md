# Persistent storage and the Files app

zlOS has one real filesystem: `zlfs` in `fs.c`. New volumes use format v2: a
checksummed superblock, two checksummed directory generations, a flat 32-entry
namespace and contiguous copy-on-write file runs on NVMe. The Files app in
`kernel.zl` is now the normal way into it. Version-1 volumes still mount; they
retain their legacy single-directory update rules until explicitly reformatted.

## What works

- Open **Files** from the dock, start menu, or `files` shell command.
- An existing zlfs volume mounts when the app opens. A blank disk is not
  formatted silently: press `Shift+F` twice in the app to confirm formatting,
  or use the explicit `format`/`mkfs` shell command.
- Press `N`, type a name, then Enter to create a named file.
- Select with Up/Down or the pointer. Enter or double-click opens the file.
- The editor reads the file from zlfs. `Ctrl+S` saves; `Esc` saves and closes.
- `Ctrl+C` copies the whole document and `Ctrl+V` appends the clipboard.
- Delete is deliberately two-step: press Delete twice on the same selection.
- Names and bytes survive a power cycle. `probe-files.py` boots two separate
  QEMU processes against the same NVMe image to prove the full app path.
- File replacement writes a separate run, flushes it, writes the inactive
  directory generation, flushes that data, then publishes its header. Mount
  chooses the newest complete generation and falls back to the older one.

## Boundary

zl's kernel subset has string literals but no runtime string values. Filenames
therefore stay inside `fs.c`. zl pushes a name byte-by-byte through `fs_npush`
and paints it byte-by-byte through `fs_nch`/`fs_ch`. This is a narrow bridge,
not a second filesystem or a filename copy in application memory.

The editor buffer is capped at 8,000 bytes. Opening a larger file refuses with
a visible notification; it never truncates the file. zlfs still has a flat
directory, 23-byte ASCII names and 32 files maximum. Directories and growth
beyond those limits are separate filesystem work.

The old ten numbered RAM slots and their `FS_META`/`FS_DATA`/`fs_init`/
`fs_save`/`fs_load` implementation are gone. `edit` opens or creates
`/user/notes.txt`; `ls` lists zlfs names and sizes; `files` opens the graphical
manager. Settings, browser history/bookmarks/page saves and the editor use the
same named-file service and explicit sync boundary.

## Checks

```bash
cd kernel/hosttest
./build.sh
./fstest

cd ..
./probe-files.py
./verify-disk.sh
```

`fstest` covers the byte-wise name bridge and filesystem failure handling. It
also cuts a replacement after every one of its block writes and cold-mounts
each result; every cutoff returns exactly the old or new file, never mixed
bytes. The newest-generation corruption case must recover the older copy.
`probe-files.py` covers create → edit → save → power off → mount → reopen.
`verify-disk.sh` independently proves named-file persistence over three boots.
