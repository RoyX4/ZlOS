# Measured toolchain closure join repair

The full hosted scheduler run
[33958363486](https://github.com/RoyX4/ZlOS/actions/runs/33958363486), code
`fc038d4`, completed with four failing steps: joined-evidence write/check and
feature-status write/check. Host, build and QEMU steps passed; physical
qualification was not run. Its build benchmark retained an over-budget
measurement, so a passed evidence step does not mean every product budget met.

The runner measured seven tools, four target lanes and **83** external headers.
The toolchain generator accepted that exact closure. The joined registry had
hardcoded **82** headers twice, in its input check and aggregate count check.
Its rejection left the historical joined registry untouched. Feature status
then compared the fresh dependency archive to that old joined build identity
and correctly refused to publish. The dependency archive itself was current.

The join now reuses the trusted local `gen-toolchain-manifest.py` validator and
derives the aggregate counts from the validated manifest. Tool and lane sets,
header identities, count consistency, environment restrictions and open
portability gaps remain required. Earlier generation/check steps still measure
the actual host bytes; the join does not invent missing toolchain evidence.

## Reproduction and verification

- Replaying the retained hosted inputs with the old join failed with
  `toolchain manifest is missing or overpromoted`.
- Six focused tests cover changing measured header counts, falsified counts,
  missing header identity, empty closure, absent tools/lanes and hidden limits.
  Before the repair, three valid-count cases failed and four invalid-manifest
  cases escaped. After repair, all six tests pass.
- The repaired full registry joined the retained inputs at identity
  `cdae9885ee8dd1becd17284ee090fae3318dfce4a302076acbb8e9956ed17dd2`
  with 83 headers. All 29 registry mutation checks were caught, including a
  newly added forged aggregate header count.
- The unchanged dependency-archive validator accepts its retained receipt
  against that repaired joined identity. This confirms the downstream failure
  was caused by the stale join; no archive gate was removed or relaxed.

Run `python3 tools/test_evidence_toolchain.py`. The full hosted workflow runs
this check before the expensive landing gate. The earlier PMM regression also
remains green. Logs and original/replayed artifacts are retained under
`/home/roy/Documents/artifacts/zl-linux/scheduler-continuation-2026-09-05/`.
Replay proves the join repair on retained evidence; the next complete hosted
run must still verify the newly published source. No generated runtime receipt
from the old worktree was staged with this repair.
