# Novel language ideas — things few or no languages ship

Brainstormed 2026-08-03. These are NOT on the committed roadmap — they are the experimental
edge, kept so they are not lost. Each is tagged for how novel it actually is. The throughline:
zl's real niche is not "fast like C" (crowded, and already matched on integer benchmarks) but
"a systems language that understands SECURITY at the language level" — which nobody is building.

zl has four assets most languages lack, and every idea here leans on one of them:
 - AI in the compiler (Floor 2)
 - an interpreter living in the same binary as the compiler
 - a security / kernel purpose (Aegis, RedOps)
 - tiny, no-runtime output (kernel32-only PE, ~1.5 KB)

---

## 1. `ai` as a compile-time keyword  [RARE — this is the flagship]

    rule = ai "block any process that injects into lsass"

The LLM runs AT COMPILE TIME, emits readable zl, and FREEZES it into the source where a human
reads and edits it. At runtime there is no AI — only the frozen code. The self-hosting fixpoint
forces the hard part to be solved: output must be cached and committed, or `verify.ps1` (compile
twice, demand byte-identical) fails. So reproducibility is enforced by a mechanism that already
exists. This is MASTER_PLAN Floor 2 and cites "Compiled AI" (arXiv 2604.05150). Almost no shipping
language does this; it is the single strongest differentiator in the whole project.

## 2. Trust levels in the type system (taint / information-flow)  [RESEARCH-ONLY elsewhere]

    data: tainted = read(socket)
    sql(data)        # compile error: tainted value reached a trusted sink

Every value carries provenance. Data from an untrusted source cannot reach a dangerous sink
unless it passes through a validator the programmer wrote. Makes whole bug classes (injection,
SSRF, unvalidated-input-to-syscall) impossible at COMPILE time. Information-flow control is a
decades-old research area that never reached a mainstream systems language. For a security
language with Aegis as a testbed, it is a natural fit and a genuine first.

## 3. Two views of one file  [NOVEL]

zl already has the `!` danger marker. Extend it: the POWER view is the raw dangerous source; the
SAFE view is an auto-generated, read-only rendering where every `!` operation is annotated with
what it does and why it is unsafe. One file, two lenses. A reviewer reads the safe view; the
author writes the power view. No language ships this.

## 4. `explain` — code and its description stay verified-in-sync  [NOVEL as enforced]

    fn parse(s) explain "turns source text into a token list" { ... }

The compiler treats the explanation as a checkable artifact — it can be regenerated and compared,
so it cannot silently rot the way comments do, and docs generate from it. Docs-from-code exists;
docs the compiler KEEPS TRUE do not.

## 5. Capabilities as types  [RARE — Zig does part]

    fn log(msg, f: FileAccess) { ... }     # cannot touch files without the capability

A function physically cannot do I/O, networking, or spawn a process unless it was HANDED that
capability as a value. THIS IS ALSO THE MEMORY ANSWER: nothing allocates unless given an
allocator, so the 51.8 GB runaway of 2026-08-03 becomes structurally impossible. One mechanism
solves memory safety AND PC-control security. Zig does allocators-as-arguments; extending it to
all OS effects (files, net, process, registry, kernel) as a unified capability type is largely
unexplored in a shipping language, and lands exactly on zl's "controls the PC" identity.

## 6. Reversible execution / step-backward debugging  [RARE — built-in is rarer]

The interpreter already observes every value mutation. Log them and a program can run in REVERSE:
step back to the state before a bug. A handful of tools bolt this on; almost no language ships it
as a first-class mode. zl gets it nearly free because the interpreter is the reference engine and
already sees everything.

## 7. Compile-time units  [UNCOMMON]

    timeout = 30s      size = 4kb      addr = 0x40      mask = 0b1010

Units and typed literals the compiler understands and checks (adding `30s + 4kb` is an error).
MASTER_PLAN already lists `30d` durations and `0x4000` hex as open items — this generalises them.

## 8. The fixpoint as a user-facing guarantee  [zl-specific]

zl can prove `f(f(x)) == f(x)` for its own compiler. Expose that as a language-level property any
program can assert about its own transforms — a formatter that must be idempotent, a serializer
that must round-trip. "This function is a fixpoint" becomes a checkable claim, not a hope.

## 9. Security-graded output  [zl-specific, ties to Aegis]

A function marked with a trust/clearance level compiles to code that literally cannot call
lower-trust code, enforced in the emitted binary — a language-level version of the ring model. The
kernel-driver target (Floor 6) is where this stops being theoretical.

---

## Why these matter together

Ideas 1, 2, 3, 5 and 9 are one identity from five angles: **a language where security is
expressed and enforced in the syntax and the type system, not bolted on afterward.** That is a
niche no mainstream systems language occupies, it is downstream of features already on the roadmap
(the type system, FFI, comptime), and it is exactly where zl's sister project Aegis already lives.

The pragmatic reading: finish the ordinary language first (the ~66-item roadmap), because every
idea here needs the type system, comptime, or FFI underneath it. But when choosing WHICH ordinary
features to sharpen, bias toward the ones these novel ideas build on — the type system most of all.
