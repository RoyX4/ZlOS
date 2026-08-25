# Stopping a program that will not stop

zlOS is about to run scripts it was not compiled with, in ring 0, with no memory
protection. `EXEC-PROMPT.md` Item 2 names three properties as non-negotiable and
says the second and third matter more than the first:

1. a runaway program must be killable
2. a crashing program must not take the kernel with it
3. memory it touches must be its own

It also says to decide the mechanism **before** writing the loop. This is that
decision, and the evidence for it.

## A step budget, not a timer

The reflex is to preempt from the timer interrupt, which already fires 100 times
a second. It is wrong twice over.

**`DECISIONS.md` #5 refused preemption and that call still stands** — preemptive
tasks sharing one framebuffer with no memory protection and no locks is a data
race with extra steps. Nothing about running a script changes that.

**More basic: a timer does not decide anything.** Being interrupted a hundred
times a second does not make an infinite loop finite. You still need a policy
that says *this has had enough*, and the budget **is** that policy. The timer
would only be the thing that lets you notice.

And the budget is **reproducible**, which a wall clock is not. The same script
dies at the same step on a loaded box and an idle one:

```
$ ./interp --steps 5000 fib.zl 2>&1 | grep -o "steps used [0-9]*"
steps used 4999
$ ./interp --steps 5000 fib.zl 2>&1 | grep -o "steps used [0-9]*"
steps used 4999
```

That matters more here than it looks. This project banned timing-sensitive gates
after `verify-raw.sh` failed on an *unchanged* kernel because the host was busy —
an A/B against a clean worktree showed baseline and modified passing and failing
together, tracking load. A kill mechanism measured in wall-clock time would put
that same non-determinism inside the kernel.

## A depth cap as well, because the budget does not save the stack

A budget of ten million steps is reached long *after* a recursion of a hundred
thousand frames has walked off the bottom of the stack. The kernel has 256 KiB
in total, and this project has already had a stack overflow write through into
console statics — 16 KiB was not enough for the compositor, which is why it now
has 256.

So depth is counted too, and it is the cheaper of the two checks. Infinite
recursion is caught by the cap, not the budget:

```
  ok    infinite recursion hits the DEPTH cap, not the budget
  ok    mutual recursion is stopped too
```

Mutual recursion is in there deliberately: a naive "is this function calling
itself" check passes `ping`/`pong` straight through.

## Two check sites, and no way around them

`eval()` is every expression and `exec()` is every statement. A zl program cannot
do anything without going through one of them — a loop body, a function call, an
operand, all of it. So the budget needs exactly **two** check sites rather than a
sprinkling, and there is no path that avoids it.

They are implemented as a wrapper pair (`eval` → `eval_inner`, `exec` →
`exec_inner`) so that depth is decremented on the way out along *every* return
path. `eval_inner` has a dozen returns inside a switch; a decrement before each
was the alternative, and it is one edit away from a permanent leak that shows up
as a spurious "too deep" ten thousand calls later.

The unwind deliberately skips the decrements — `runtime_error` longjmps straight
past those frames, leaving `zi_depth` high. That does not matter and must not be
"fixed": the trap catcher is the only thing that runs afterwards, and
`zi_limit()` resets the counters before anything runs again. A cleanup there
would be code that only executes while unwinding, which no test can reach.

## The boundary: a trap, not an exit

`runtime_error()` has always called `exit(1)`. Correct for a program, fatal for
an operating system. It now longjmps to a trap armed by `zl_run_program()` when
one is armed, and behaves exactly as before when none is.

Three things make that safe, and all of them are at the one call site rather
than scattered:

- **The message is copied before unwinding.** Several callers pass a pointer into
  a local buffer — `N_IDENT`'s "'%s' doesn't exist yet" builds one on the stack —
  and that buffer is gone the moment the stack unwinds past it. Handing the
  caller the pointer would be a read of dead stack: the shape of bug that
  reproduces on one build and not the other.
- **The control-flow flags are reset first.** `g_returning` / `g_breaking` /
  `g_continuing` are globals, and a program killed mid-loop leaves whichever one
  it was carrying set. The next program then returns from its first statement for
  no visible reason, and the bug looks like it is in the second program. Nothing
  reset these before because nothing had ever run two programs in one process.
  The kernel runs one per `run`.
- **Nothing local is read after the unwind.** Locals modified between `setjmp`
  and `longjmp` are indeterminate unless `volatile`. The return path reads only
  globals. That is not pedantry — it is undefined behaviour that works at `-O0`
  and breaks at `-O2`, which is what the kernel builds at.

## The hole: counting nodes is not counting work

The version above was wrong, and an adversarial reader found it within minutes
of being asked "can you write a script that wedges the machine?".

The budget counted **visits to `eval()` and `exec()`**. That bounds how many
nodes a program executes and says nothing about how much work each one does. A
single builtin call is one step no matter how large:

```
$ cat onestep.zl
xs = range(50000000)
print(len(xs))
$ ./interp --steps 100 --depth 50 onestep.zl
50000000
$ echo $?
0
```

A hundred steps of budget, fifty million allocations, and the program won.

Capping `range()` would have been the wrong fix. Every container-building
builtin has the same shape — `repeat`, `concat`, `join`, `split`, `sort`,
`reverse` — and the next builtin somebody adds walks straight past a list of
special cases.

**So the budget is charged per byte allocated, at a single allocation seam.**
`zi_alloc` / `zi_strdup` / `zi_realloc` replaced all 60 `malloc`/`_strdup`/
`realloc` sites in `src/runtime/interp.c`. A new builtin cannot escape the budget without
avoiding memory altogether, and nobody has to remember anything.

The divisor is 64 bytes to a step, which is roughly the cost of a `Value`, so
allocation and execution are on the same scale — the list above costs about six
million steps rather than one. Charging saturates rather than wraps: a request
big enough to overflow the counter must exhaust the budget, never lap it.

```
$ ./interp --steps 100 --depth 50 onestep.zl
stopped: step budget exhausted - the program was stopped
  steps used 6250006, peak depth 2
```

It fires on the 400 MB pointer array, before any of the fifty million values
are built.

**That seam is also the answer to non-negotiable 3.** In the kernel `zi_alloc`
becomes `arena_alloc`, so a program's memory is its own by construction — one
function to change instead of sixty-eight call sites.

## Off by default, so there is one code path and not two

`zi_limit(0, 0)` means unlimited, which is exactly what the hosted `zl`
interpreter has always done. The kernel arms the limits; nothing else does.

That is why there is no `#ifdef` pair: the hosted test suite exercises **the same
lines** the kernel runs, and neither copy can drift from the other. `run_tests.sh`
is `ALL GREEN` with the machinery compiled in and disarmed.

## The gate is adversarial, and it runs without a kernel

`hosttest/killtest.sh`. Every script in it is written to win — to run forever, to
exhaust the stack, to allocate without bound — and the assertion is that each one
is stopped, says which limit stopped it, and leaves the process alive to be
asked.

```
14 cases, 0 failures
ok    every program written to wedge the machine was stopped, and said why
```

It runs on the host because the mechanism is arithmetic and a longjmp, neither of
which is kernel-specific — a second per case instead of a QEMU boot, so the
adversarial scripts can be plentiful. Every one of them exercises the shipping
code.

Two cases earn their place beyond the obvious:

- **A `while` loop with an empty body.** No allocation, no builtin, no output —
  so anything that detects a runaway by watching for side effects sees a program
  indistinguishable from a halted machine.
- **The control case.** `fib(20)` must still return 6765 under the limits. A
  budget that stops everything is not a budget, it is a broken interpreter, and
  without this the gate passes on one.

What it does **not** prove, and the kernel gate must: that the desktop stays
responsive while a program is being stopped. That needs a machine, and it is
Item 2's own gate.

## What three attackers found that I did not

I wrote fourteen adversarial cases and every one of them passed. `EXEC-PROMPT.md`
§9 says to put an adversarial reader on this exact question — *"can you write a
script that wedges the machine?" is exactly the question the author is worst
placed to answer"* — so three agents were asked it, with different lenses. They
produced **twelve verified payloads in three families**, and the budget could
not see any of them.

### Raw memory: three characters of zl, a dead machine

```
poke32(0, 1)                    -> SIGSEGV
x = peek8(0)                    -> SIGSEGV
fill_mem(0, 0, 1000000000000)   -> SIGSEGV
copy_mem(0, 0, 4096)            -> SIGSEGV
```

The budget bounds *time*. The arena bounds *how much* memory. **Neither bounds
which memory**, and `peek`/`poke`/`fill_mem`/`copy_mem` exist precisely to hand
out arbitrary addresses — `design_memory_structs.md` §3.1 fixes their names
because a page allocator is written against them. They are how zl drives
hardware. But a program the kernel was not built with is not the kernel.

A signal is not a `longjmp` and there is no handler; in ring 0 a page fault is
not a signal at all.

**The fix is a window.** `zi_confine(lo, hi)` says which addresses a program may
touch; every raw access is checked and a violation is an ordinary
`runtime_error` the trap already catches and reports. Unset by default — the
hosted interpreter and `kernel.zl`'s own compiled code are untouched. The kernel
confines a program to exactly the arena, which is non-negotiable 3 implemented
rather than asserted. The bound is a subtraction (`len > hi - addr`), never an
addition, for the same reason `arena.c`'s ceiling is.

### The parser, upstream of every check

```
$ python3 -c "open('x.zl','w').write('['*8000)"
$ ./interp --steps 1 --depth 1 x.zl
Segmentation fault
```

`--steps 1 --depth 1` and it still crashes, because none of the kill path is
involved: `parse()` runs before the trap is armed or any limit is set. The
parser is uncapped recursive descent — every `[` re-enters `parse_expr` through
about a dozen frames — and **the overflow happens on the descent**, so no
closing bracket is ever needed.

Measured on this host's 8 MiB stack: 6000 brackets survives, 6500 crashes. **The
kernel has 256 KiB, roughly 32× less — about 200 brackets, a source file under a
quarter of a kilobyte, with no memory protection to contain where it lands.**

Capping recursion inside `src/frontend/parser.c` would mean threading a counter through
eleven precedence levels in a file this track does not own. But parser recursion
is bounded by bracket nesting, and bracket nesting is visible in the **token
stream** before a single frame is pushed. One linear scan, no parser change, and
it cannot be wrong about a construct it has not been taught because it counts
the only thing that makes the parser recurse.

### Builtins that leave the interpreter

```
run("sleep 3600")   one step, blocks forever
exit(0)             not an error, so the trap never sees it
```

The budget counts statements, not seconds. A builtin that waits is unbounded no
matter how small the budget is, and `exit()` is a machine in a kernel.

Refused by name, at the single door into `call_builtin`, whenever a window is
armed. The list is everything reaching outside the interpreter: the host, the
filesystem, the clock, other programs. **A confined program is a pure
computation over its own arena** — which is the whole of what Level 1 promised.

`fill_mem` and `copy_mem` also now charge their length to the budget. They call
`memset`/`memmove` directly, so `zi_alloc` never saw them and a terabyte fill was
one step.

### The one overclaim, for the record

`sort(reverse(range(2000000)))` was reported as running forever. It does not —
the per-byte allocation charge, added from the *first* attacker's finding before
that agent ran, stops it at 250,010 steps. An independent verify pass confirmed
it. **Attackers overclaim; the verify stage earned its place.**

### Where it stands

```
25 cases, 0 failures
ok    every program written to wedge the machine was stopped, and said why
```

and `run_tests.sh` is still ALL GREEN, which is the check that matters for "off
by default" — none of this changed the hosted interpreter by a single behaviour.

## What this found on the way past

`1 / 0` is **not** an error in the interpreter — it produces `inf` and exits 0.
`freestanding/runtime_kernel.c:579` disagrees and calls `kfatal("divide by
zero")`, which in a kernel is the end of the machine. Same three characters,
silent `inf` on one path and a dead machine on the other, depending on whether
the program was interpreted or compiled in.

It is now pinned by a gate (case 7b) so it cannot change unnoticed, and tracked
as T-EXEC-8. The interpreter's behaviour is the one that should change: "the
script did something silly" must never be able to mean "the machine stopped".
That is a language-semantics decision, not an exec-track one.
