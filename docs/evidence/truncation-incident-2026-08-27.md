# ~125 tracked files truncated to zero bytes, and committed

**2026-08-27, branch `design/presswork`.** Recorded because the recovery was easy
and the detection was not, and the detection is the reusable part.

## What happened

Between a QEMU boot probe finishing and the next commit, about 125 tracked files
became zero bytes. Not edited - emptied. Among them:

```
kernel/src/kernel.zl                       268 KB -> 0
kernel/src/drivers/display/intel.c         233 KB -> 0
kernel/src/graphics/framebuffer/fb.c       170 KB -> 0
kernel/src/graphics/windowing/wm.c         147 KB -> 0
freestanding/runtime_kernel.c              138 KB -> 0
src/runtime/interp.c                       114 KB -> 0
...every metadata registry, every receipt, every generator
```

They were then committed as `f2778e6`, because the commit used `git add -A`.

## Cause: unknown, and that is the honest answer

It was **not** disk space - 246 GB free, 11 % inodes, checked because that was the
obvious candidate and would have meant more damage was coming. `dmesg` shows no
I/O or filesystem errors in the window. No agent was running at the time. The
mechanism is not established, so nothing here claims one.

## IT ALSO HIT `.git`, and that was not discovered for five hours

This document originally said the event zeroed "about 125 tracked files". That
was incomplete. In the **same minute**, 15:47, it also zeroed:

```
.git/refs/stash                 0 bytes
.git/logs/refs/stash            0 bytes
.git/objects/2d/c42231…         0 bytes   } seven loose
.git/objects/47/f2c673…         0 bytes   } objects, all
.git/objects/54/c293f8…         0 bytes   } orphaned, all
.git/objects/78/0350a6…         0 bytes   } at 15:47
.git/objects/9c/d512ae…         0 bytes   }
.git/objects/cc/d3ca4b…         0 bytes   }
.git/objects/eb/472218…         0 bytes   }
```

Note that this is the SHARED repository - `zl-linux-presswork` is a linked
worktree, so `.git` here is a file pointing at
`/home/roy/Documents/repos/zl-linux/.git`. The corruption therefore affected
every worktree on that repo, including other sessions'.

**How it surfaced: a `git push` that produced no output at all.** Not an error,
not a rejection - a zero-byte log file. `git fetch` was more forthcoming:

```
fatal: bad object refs/stash
error: … did not send all necessary objects
```

`git fsck` found nine problems and, importantly, **zero broken links** - no
reachable commit, tree or blob depended on any of the seven objects. They were
orphans.

**An empty object file is worse than a missing one.** Git finds it, fails to
mmap it, and errors - instead of falling through to a pack or refetching. So
seven files containing no bytes at all were enough to break push and fetch for
the whole repository.

The fix was to move all nine into `.git/quarantine-truncation-2026-08-27/`
rather than delete them - reversible, though since every one is zero bytes there
is no data in them to lose. `git fsck` then exits 0 and push and fetch work.

**The lesson is about the shape of the evidence, not the bytes.** The working-tree
damage was loud: a hundred source files at zero bytes, and a doc-check that went
red. The `.git` damage was silent for five hours and then presented as a command
that printed nothing. When a git operation produces NO output where it should
produce some, suspect the repository, not the network - and run `git fsck`
before assuming the tree is the only casualty.

## Recovery

Everything was in git. `6763c0d` - the commit before the damage - was whole, and
carried all the parity and depth work plus the NAMESET_MAX fix.

```
git tag -f damaged/truncation-f2778e6 f2778e6   # keep it; destroy nothing
git reset --hard 6763c0d
```

Verified after: 0 empty tracked files, `check-zlcalls` 1083 functions / 759
builtins all resolving, `check-shell-layout` PASS at four panel sizes, toolchain
exit 0, `kernel.elf` exit 0 with 0 undefined symbols at 5,589,472 bytes -
byte-identical in size to before the damage.

One commit's work was lost and redone: the `files_draw` fix.

## What detected it, and what did not

**The build did not.** The kernel still compiled, because the compiler reads
`kernel/out.c`, which was generated before the truncation and was not itself
emptied. A green build over an empty source tree.

**`doc-check.sh`'s registered claims did.** Two assertions failed at once:

```
AGENTS.md  "wmshot renders the compositor with no boot and no GPU"
           check: grep -q 'P6' kernel/tests/host/wmshot.c
CLAUDE.md  "verify-efi.sh no longer hard-requires KVM"
           check: grep -q 'accel tcg' kernel/tools/checks/verify-efi.sh
```

Both were collateral - those two files were among the emptied - and both were
read at first as separate documentation problems. **Two unrelated registered
claims failing in the same run is not a documentation problem. It is a signal
that the tree moved underneath the docs**, and it is worth treating as one.

## The habit that made it worse

`git add -A` is what turned a filesystem event into a history event. It had been
used all session because the working tree is genuinely noisy - regenerated
manifests, build identities, receipts - and staging explicitly felt like friction.
That friction is the feature. Stage paths.
