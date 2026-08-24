# Host freeze and landing-gate containment — 2026-08-24

## Result

The exact freeze trigger is not recoverable. The forced power-off left no log
for roughly the final four and a half minutes. The strongest supported
explanation is **whole-machine scheduling and graphics pressure from overlapping
development workloads**, including an unrestricted zlOS landing gate, another
zlOS task, editor activity and two Electron applications. This is not a
proved out-of-memory event and not evidence of hardware damage.

The complete gate is no longer allowed to run unrestricted. Its only supported
entry point is:

```sh
gates/run-land-gate-contained.sh start
```

Calling `gates/land-gate.sh` directly now fails closed.

## Evidence that survived

Previous-boot journal evidence establishes this sequence on 2026-08-24:

- Intel i915 reported atomic-update deadline failures on pipe A at 09:01:10,
  09:01:28, 09:01:44 and 09:09:50. These prove display scheduling trouble;
  they do not alone prove the final freeze cause.
- The landing-gate journal completed the 64-bit kernel at 09:11:39 and entered
  the EFI build. The final desktop log was at 09:11:49.
- The landing gate had started at 09:04:13. Just before it, the scheduled
  `chkrootkit` service had consumed 3 minutes 34 seconds of CPU over 3 minutes
  39 seconds and peaked at 869.5 MiB. This increased host pressure, but it had
  already exited before the landing gate began.
- The next recorded event was reboot filesystem recovery at 09:16:19. The
  missing interval contains no surviving causal record.
- Cursor's stopped GNOME scope reported a 1.2 GiB memory peak. Its overlapping
  Chromium application scope reported 2.4 GiB; those two figures may include
  the same process tree and must not be added together. They prove a large
  editor workload, not memory exhaustion.
- Searches of the surviving previous-boot journal found no kernel OOM kill,
  kernel panic, hung-task report, NVMe/I/O error or thermal shutdown.
- The reboot performed EXT4 orphan cleanup and replaced unclean system and user
  journals. That is a consequence of forced power loss, not evidence that the
  NVMe caused the freeze.
- Immediately after reboot the host had about 11 GiB available memory, zero
  swap use, normal CPU/NVMe temperatures and no storage-capacity pressure.

The absence claims above are bounded by the missing final interval. They must
not be rewritten as proof that an OOM, graphics hang or hardware fault was
impossible.

## Separate 09:48 reboot

The reboot at 09:48 was not another freeze or forced power loss. The journal
records `gnome-session-shell` PID 1742 asking `systemd-logind` to reboot at
09:48:26, followed by an orderly service stop, filesystem sync and reboot. The
log identifies the GNOME session as the requester but does not prove whether
that request came from a person clicking the desktop reboot action or another
desktop client using the same session API. It is separate from the 09:11
incident.

After the second reboot, the host again reached a load average above 7 while
Cursor, ChatGPT, GNOME Shell and their renderer/language-server processes were
simultaneously CPU-active. No gate, QEMU or compiler was active at that check.
That observation reinforces the concurrency-pressure explanation and is why
the contained launcher refuses to begin while load exceeds 1.50.

## What was recovered

The interrupted verifier had previously been hardened with a persistent
transaction record. On the first post-reboot check:

- `verify-sources.sh --recover-only` reported a clean transaction;
- the source manifest returned to 123 inputs;
- build identity matched
  `85027b159c9a594045c2f900e5971bb3408dd418dd61a373625425fba9030d13`;
- no landing-gate service or landing-gate process remained.

Two zero-byte compiler temporary objects survived the interrupted EFI build.
They are not evidence artifacts and must be removed only after confirming no
compiler owns them. A contained run refuses to start while any `*.o.tmp` file
remains.

## Mandatory host-safety contract

`run-land-gate-contained.sh` enforces all of the following before launch:

1. no other landing gate, QEMU, GCC, Clang or linker process is active;
2. one-minute load average is at most 1.50;
3. at least 8 GiB memory is available;
4. no interrupted compiler temporary remains;
5. the SOURCES transaction is clean or recovered;
6. the generated build identity and its mutation self-test pass.

The gate then runs as one sequential systemd user service with:

- at most one CPU core (`CPUQuota=100%`), low CPU and I/O weights, and nice 10;
- a 4 GiB soft memory boundary, 6 GiB hard boundary and 2 GiB swap boundary;
- at most 128 tasks;
- whole-cgroup termination, a 30-second stop deadline and stop-on-OOM policy;
- all output in the user journal, so status and the exact last completed step
  remain inspectable outside the initiating terminal.

These limits preserve desktop headroom; they are not performance evidence.
Benchmark receipts produced under contention remain noisy host measurements and
cannot be promoted to native-target performance.

A harmless transient-service probe on the rebooted host verified that systemd
accepted the intended controls: one-second CPU quota per one-second period,
CPU/IO weights 10, nice 10, memory boundaries 4/6/2 GiB, 128 tasks,
`OOMPolicy=stop`, whole-control-group kill and a 30-second stop deadline. That
proves configuration admission, not the behavior of a full gate under load.

## Operator commands

```sh
gates/run-land-gate-contained.sh doctor  # readiness and stale-state recovery
gates/run-land-gate-contained.sh start   # start only after doctor passes
gates/run-land-gate-contained.sh status
gates/run-land-gate-contained.sh limits
gates/run-land-gate-contained.sh logs
gates/run-land-gate-contained.sh stop
```

Never kill an unrelated QEMU or compiler to make `doctor` green. Wait for its
owning task. Never bypass the raw-gate environment guard to save time.

## Open evidence gap

There is no completed full landing-gate receipt after this incident. The last
attempt stopped during the EFI build and must not be described as green. The
next full proof is allowed only through the contained launcher when the host is
idle; its final status, resource limits and journal tail must be captured
together.
