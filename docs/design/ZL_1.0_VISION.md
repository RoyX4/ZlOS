# What zl looks like finished — proposed syntax for the spine

Drafted 2026-08-03. This is the payoff of the planning docs: concrete syntax for the seven
spine features, shown as real code, so the decisions become visible. **Everything here is a
PROPOSAL to ratify, not settled.** Where a choice is genuinely open it says so.

The rule from MASTER_PLAN holds: match the existing identity — no keyword for variables, `{}`
blocks, terse, readable, `!` marks danger.

---

## 1. Imports (item 3)

Today: copy-paste. Proposed:

    import sortx                      # whole module, names prefixed: sortx.quick(xs)
    import quick, merge from sortx    # specific names, unprefixed
    import graph as g                 # aliased

Open question: search path. A `zl.mod` file at project root listing where modules live, or a
convention (`./stdlib/`, `./`, then a global cache). Recommend: local dir → `./lib/` → global,
first match wins, and a `zl.mod` only when you need to override.

---

## 2. Records / structs (item 4)

`.` is ALREADY parsed (src/frontend/parser.c builds N_MEMBER), so this is close.

    rec Point { x: int, y: int }
    p = Point { x: 10, y: 20 }
    print(p.x)                       # 10
    p.y = 30                         # field assignment

    rec Image { w: int, h: int, px: list[int] }

Migration is gentle: `[10, 20]` is still a valid `Point` (records are a naming layer over lists,
per design_records.md), so old code keeps working while new code gets names.

Tuples are the anonymous version, and zl already has them — `[a, b]` IS the tuple. `rec` just
names the slots.

---

## 3. Maps (item 4)

    ages = { "zac": 17, "sam": 22 }
    print(ages["zac"])               # 17
    ages["max"] = 30

**THE ONE REAL CONFLICT:** `{}` already means a block. `{ "zac": 17 }` has to be distinguishable
from a block by the `key: value` inside. design_maps.md proposes `[k: v]` instead to avoid it:

    ages = ["zac": 17, "sam": 22]    # unambiguous - a list literal with pairs

Recommend `[k: v]` — it never collides, it reads as "a list of pairs" which is what it is, and it
composes with the existing list syntax. Decide before maps are built.

---

## 4. Closures (item 11) — the biggest ergonomic unlock

    add = fn(a, b) { return a + b }          # a function stored in a variable
    print(add(2, 3))                          # 5

    # passed inline - this is what unlocks the whole map/filter/reduce family:
    people = sort(people, fn(a, b) { return a.age < b.age })
    doubled = map(xs, fn(x) { return x * 2 })
    adults  = filter(people, fn(p) { return p.age >= 18 })
    total   = reduce(xs, 0, fn(acc, x) { return acc + x })

Optional shorthand for one-liners (decide later): `fn(x) => x * 2`.

The hard part is NOT syntax — it is capture. zl's scoping rule (assignment inside a function
writes the global) means a naive closure captures globals, not locals. design_scoping_decision.md
must resolve this FIRST. That is why the scoping decision blocks the type system AND closures.

---

## 5. Errors (item 5)

Three shapes were on the table. Recommendation: **errors as values**, because it composes with
nullables and needs no exception machinery in five backends.

    fn parse(s) -> int | error {
        if not is_number(s) { return error("not a number: " + s) }
        return num(s)
    }

    n = parse(input())
    if n is error { print(n.message)  return }
    print(n + 1)

And the shorthand that makes it pleasant, borrowed from Rust's `?`:

    n = parse(input())?          # if error, return it up immediately

This also gives the test suites back the ~20 assertions deleted this session:

    assert(parse("abc") is error)

---

## 6. Sized integers (item 15) — the gate in front of FFI

    b: u8 = 255
    count: u32 = 0
    offset: i64 = -40

Needed because you cannot describe a Windows function's arguments without them. Also enables
binary formats — stdlib/bmp.zl hand-assembles bytes today and would use these.

---

## 7. FFI (item 16) — the unlock for controlling the PC

    extern "user32.dll" fn MessageBoxW(hwnd: ptr, text: ptr, caption: ptr, kind: u32) -> i32

    MessageBoxW(nil, "hello from zl", "zl", 0)

Once this works, everything from PC control through kernel talk is a zl LIBRARY:

    import msgbox, run, registry from windows

    out = run("git", "status")           # launch a program, capture output
    registry.set("HKCU\\...\\zl", "1")   # write a setting
    packet = windivert.recv()            # intercept a packet  (via WinDivert)

---

## Two complete programs in "finished zl"

### A real command-line tool (needs: imports, args, errors, closures)

    import args, exit from sys
    import filter, map from list

    fn main() {
        files = args()?                              # item 8: read argv
        if len(files) == 0 { print("usage: count <files>")  exit(1) }

        for f in files {
            text = read(f)?                          # item 5: error if missing
            lines = split(text, "\n")
            code  = filter(lines, fn(l) { return trim(l) != "" })   # item 11
            print(f + ": " + str(len(code)) + " non-blank lines")
        }
    }

### A tiny Windows tool (needs: FFI, sized ints)

    import beep, message from windows

    fn main() {
        beep(750, 300)                               # FFI to kernel32 Beep
        message("Backup complete", "zl")             # FFI to user32 MessageBoxW
    }

Neither of these is writable in zl today. Both are writable once items 3, 5, 8, 11, 15, 16 land.

---

## The critical path — what blocks what

Read top to bottom; each depends on the ones above it.

    scoping decision (design_scoping_decision.md)
       |  blocks BOTH the type system and closures - a function's variables
       |  have no stable meaning until this is settled
       v
    +--> closures (11) --> map/filter/reduce + the whole iterator family
    |                      + comparators for sort + callbacks
    |
    +--> type system (Phase 7)
            |
            +--> sized ints (15) --> FFI (16) --> ALL PC control, networking,
            |                                     kernel-talk (items 50-65)
            |
            +--> records (4) --> maps (4) --> every keyed collection
            |
            +--> unboxing --> the 4-6x speed win

    Independent of all the above (can happen any time):
       memory ceiling in interp (stops crashes)   <- do this NEXT, it is ~20 lines
       imports (3)                                 <- highest value, low risk
       stack traces (7), argv (8), REPL (9)        <- cheap usability
       float support in fast backends              <- running now

## The single most important sentence in all the planning

**The scoping decision is the true root of the dependency tree.** It is a DECISION, not code -
it costs an afternoon of thought and zero implementation - and until it is made, neither closures
nor the type system can be built correctly. It should be resolved before any Phase 2+ code is
written. Everything else has a clear path; this one thing gates the two biggest unlocks at once.
