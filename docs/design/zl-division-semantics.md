# `/` in zl: two engines, two answers, and how that cost a day

**Short version:** in the **compiled** kernel, `/` is integer division that
truncates. In the **hosted interpreter** (`./interp`), `/` is float division.
The desktop is compiled. A comment in `kernel/src/kernel.zl` asserted the
interpreter's answer about the kernel, cited a real experiment run on the wrong
engine, and I believed it and acted on it across 26 sites.

## The derivation

Every binary operator in zl source compiles to a `zl_binop` call:

```
src/backends/c/compile.c:346     emits zl_binop("<op>", a, b)
```

`kernel/_gen.c` — the generated C for `kernel.zl` and the `apps_*.zl` it
imports — contains 8,146 such calls. So the desktop's arithmetic is whatever
`zl_binop` does.

Each kernel build links exactly one runtime:

```
kernel/build.sh:34      gcc ... -c ../freestanding/runtime_kernel.c -o _rt.o
kernel/build64.sh:25    (same)
kernel/buildefi.sh:62   (same)
```

and that file's `zl_binop` opens:

```c
/* Integer semantics on purpose. design_kernel.md §2 is built on
 * nativegen's exact-i64 arithmetic; a kernel doing floating-point div on a
 * descriptor is how you get a wrong GDT entry. Numbers here are whole. */
Value zl_binop(const char *op, Value a, Value b)
{
    ...
    long long x = (long long)a.num, y = (long long)b.num;
    ...
    if (op[0] == '/' && !op[1]) { if (!y) kfatal("divide by zero"); return zl_num((double)(x / y)); }
```

Both operands are truncated to `long long` **before** the divide, and the
quotient is integer. `src/runtime/runtime.c`, which does implement float `/`, is
not linked into any kernel image — `grep -rn "runtime/runtime.c" kernel/*.sh`
returns nothing.

## Where the float answer is real

`kernel/build.sh:63` also compiles `src/runtime/interp.c` (with
`-DZL_FREESTANDING`), so the kernel embeds an **interpreter** for running `.zl`
files at runtime. There:

```
src/runtime/interp.c:1948    if (strcmp(op, "/") == 0)  return make_num(a / b);
```

is genuine float division. So a zl program *interpreted* on this machine and the
same program *compiled into* it disagree about `7 / 2`. That is a real
inconsistency in the language implementation, worth knowing, and it is not what
the desktop's code path does.

## What idiv actually differs by

`idiv` is still worth having, for reasons that are **not** truncation:

| | `a / b` | `idiv(a, b)` |
|---|---|---|
| `b == 0` | `kfatal("divide by zero")` — halts the machine | returns `0` |
| result | as-is | masked to `0x7FFFFFFF`, negatives negated around the mask |
| truncation | yes | yes |

The `b == 0` row is the one that matters, and it goes the **wrong** way for
finding bugs: a zero divisor becomes a plausible index instead of a loud stop.
Choose deliberately.

## How the wrong claim got in, and why it was believed

`kernel/src/kernel.zl` carried this, above `idiv`:

> Every zl number is a double, so `/` is FLOAT division: `7 / 2` is 3.5 and
> `4093 / 256` is 15.9883. **Verified with ./interp, not assumed.**

Every word of that is true *about `./interp`*. `./interp` is built by
`kernel/build.sh:14` from the hosted runtime. The experiment was performed, the
numbers are right, and the conclusion was applied to a different engine.

It was believed **because** it carried evidence. A bare assertion would have
been checked; "verified, not assumed" reads as already checked. That is the
failure mode worth taking from this: a citation raises confidence without
raising correctness, and the thing to verify is not whether the experiment
happened but whether it was run on the system under discussion.

## What was done on the strength of it

Three commits (`a4f8844`, `d85afc8`, `0c131ee`) converted 26 sites to `idiv`,
added ~9 comments asserting the false mechanism, and installed a hazard-scan
guard (`check-intdiv.py`) enforcing it. The commit messages described failures —
"the Files list ignored nineteen clicks in twenty", "the calculator pressed '+'
for every key", "no row ever highlighted" — **as measurements. They were
inferences from the operator. None was observed.**

The guard is deleted. The comments are corrected in place rather than removed,
so the next reader finds the correction where the claim was. The `idiv` calls
are kept: they are not worse, and rewriting 26 sites again would be churn on top
of churn.

## What is still open

If any of those controls **is** actually dead, its cause has not been found, and
the reports were closed against a mechanism that was not it. That question is
reopened, not answered. The one place there is evidence: `probe-calckeys.py`
passed both before and after the calculator's conversion, which is consistent
with the keypad never having been broken this way.

## What survives from that work

The `ac_rand` / `sn_rand` fix (`d5f5106`) is **unaffected and correct**. That bug
is in the MULTIPLY, not the divide: `zl_binop` computes `x * y` exactly in
`long long` and then `zl_num((double)(x * y))` rounds away everything below
2^53. The measurements in that commit were taken by replaying `zl_binop`'s own
long-long semantics in C — the kernel's semantics, not the interpreter's — and
they stand:

```
ac_rand(4)  before:  0 = 95.3%   1 = 2.9%   2 = 0.4%   3 = 1.4%
ac_rand(4)  after:   24.9 / 25.1 / 25.1 / 24.9
```

It is what made the `/` claim plausible by association: one real
double-rounding bug in the same file made a second one easy to believe.
