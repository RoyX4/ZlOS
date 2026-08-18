/* interp.c - stage 3: the tree-walking interpreter.
 *
 * The parser gave us a TREE. This walks the tree and DOES what it
 * says. eval() handles expressions (things that produce a value);
 * exec() handles statements (things that make something happen).
 *
 * This is the moment the language becomes ALIVE: after this runs,
 * `print("hello")` actually prints hello.
 *
 * SAFETY NOTE: the dangerous PC-control built-ins (kill, poke, rm,
 * ...) are SIMULATED for now - they print what they *would* do
 * instead of doing it. Real, dangerous behaviour is a later floor,
 * and only after VM testing. `print` and `input` are real.
 */

#ifdef ZL_FREESTANDING
/* ---- the kernel build ----------------------------------------------------
 * There is no libc here, so the names this file uses are redirected to
 * interp_kernel.c's, which are checked against the real libc's over tens of
 * thousands of inputs in hosttest/libctest.c.
 *
 * MACROS RATHER THAN #ifdefs THROUGHOUT THE BODY, on purpose. The alternative
 * is a second copy of the interpreter that the hosted test suite never runs,
 * and the two drift the first time either is fixed. This way there is one
 * interpreter, the tests exercise it, and the only difference between the two
 * builds is which strlen it links against.
 *
 * What is NOT redirected is the OS surface - files, processes, the clock.
 * Those have no kernel equivalent at all, so the builtins that use them are
 * compiled out below and refused by name at run time. Both, deliberately: the
 * compile-out is what makes the kernel link, and the run-time refusal is what
 * makes the message honest on the host too. */
#include "freestanding/zl_freestanding.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <stdint.h>
#include <sys/wait.h>
#endif

#include "lexer.h"
#include "parser.h"
#ifndef ZL_FREESTANDING
#include "os.h"          /* the OS layer: real machine access, kept
                            in its own file so <windows.h> can't
                            clash with our TokenType/Node names */
#endif


/* =============================================================
 * THE KILL PATH - how a program that misbehaves is stopped
 *
 * zlOS is about to run scripts it was not compiled with, in ring 0, with no
 * memory protection (kernel/docs/EXEC-PROMPT.md, Item 2). Two of that item's
 * three non-negotiables live here, and the brief is emphatic that they are
 * decided BEFORE the loop is written rather than bolted on after:
 *
 *   1. a runaway program must be killable
 *   2. a crashing program must not take the kernel with it
 *
 * A STEP BUDGET, NOT A TIMER. The obvious reflex is to preempt from the timer
 * interrupt, and it is wrong twice over. DECISIONS.md #5 refused preemptive
 * tasks sharing one framebuffer with no memory protection and no locks, and
 * that call still stands. More basic than that: a timer does not DECIDE
 * anything. Being interrupted 100 times a second does not make an infinite
 * loop finite - you still need a policy that says "this has had enough", and
 * the budget IS that policy. It is also reproducible, which a wall clock is
 * not: the same script dies at the same step every time, on a loaded box or an
 * idle one, so a gate written against it can never be timing-sensitive - the
 * rule this project adopted after verify-raw.sh failed on an unchanged kernel.
 *
 * A DEPTH CAP AS WELL, because the budget alone does not save the stack. A
 * script that recurses is consuming C stack frames per level, and the kernel
 * has 256 KiB of stack in total. A budget of ten million steps is reached long
 * after a recursion of a hundred thousand has walked off the bottom of it -
 * and this project has already had a stack overflow write into console statics
 * (16 KiB was not enough for the compositor). So depth is counted too, and it
 * is the cheaper of the two checks.
 *
 * BOTH CHECKS ARE OFF BY DEFAULT. zi_limit(0, 0) means unlimited, which is
 * exactly what the hosted `zl` interpreter has always done, so this changes
 * nothing for it. The kernel arms them. That way there is ONE code path
 * instead of an #ifdef pair, the hosted test suite exercises the same lines
 * the kernel runs, and neither can drift from the other.
 *
 * THE TRAP. runtime_error() has always called exit(1), which is correct for a
 * program and fatal for an operating system. When a trap is armed it longjmps
 * back to whoever armed it instead, carrying the message. Nothing else about
 * the error path changes, and with no trap armed the behaviour is identical to
 * what it was.
 * ============================================================= */

#ifndef ZL_FREESTANDING
#include <setjmp.h>
#define zi_jmp_buf  jmp_buf
#define zi_setjmp   setjmp
#define zi_longjmp  longjmp
#endif

#define ZI_ERRMAX 192

static long long zi_steps_left;      /* 0 = unlimited                        */
static long long zi_steps_used;
static int       zi_depth;
static int       zi_depth_max;       /* 0 = unlimited                        */
static int       zi_depth_peak;
static zi_jmp_buf zi_trap;
static int       zi_trap_armed;
static int       zi_killed;          /* 1 = stopped by budget or depth       */
static char      zi_errmsg[ZI_ERRMAX];

long long zi_used(void)      { return zi_steps_used; }
int       zi_peak_depth(void){ return zi_depth_peak; }
int       zi_was_killed(void){ return zi_killed; }
const char *zi_error(void)   { return zi_errmsg; }

void zi_limit(long long steps, int max_depth)
{
    zi_steps_left = steps;
    zi_steps_used = 0;
    zi_depth      = 0;
    zi_depth_max  = max_depth;
    zi_depth_peak = 0;
    zi_killed     = 0;
    zi_errmsg[0]  = 0;
}

static void zi_seterr(const char *msg)
{
    int i = 0;
    while (msg && msg[i] && i < ZI_ERRMAX - 1) { zi_errmsg[i] = msg[i]; i++; }
    zi_errmsg[i] = 0;
}


/* ---- the allocation seam, and the second half of the budget --------------
 * THE HOLE THIS CLOSES. The step budget counts visits to eval() and exec(),
 * which bounds how many NODES a program executes and says nothing about how
 * much WORK each one does. Found by an adversarial reader, and confirmed:
 *
 *     $ echo 'xs = range(50000000)
 *             print(len(xs))' > onestep.zl
 *     $ ./interp --steps 100 --depth 50 onestep.zl
 *     50000000
 *     exit=0
 *
 * A hundred steps of budget, fifty million allocations, and the program won.
 * One eval() of one builtin call. Every container-building builtin has this
 * shape - range, repeat, concat, join, split, sort - and capping them one at a
 * time is whack-a-mole that the next builtin somebody adds walks straight past.
 *
 * So the budget is charged where the work actually is: PER BYTE ALLOCATED.
 * That is one chokepoint, it cannot be forgotten by a new builtin because a new
 * builtin cannot allocate without coming through here, and it makes the budget
 * mean "work" rather than "statements" - which is what a caller wanting to
 * bound a program actually means.
 *
 * It is ALSO the seam the kernel port needs for Item 2's third non-negotiable,
 * that a program's memory must be its own. In the kernel zi_alloc becomes
 * arena_alloc; here it stays malloc. One function to change instead of 68.
 *
 * THE DIVISOR. Charging one step per byte would make the budget's units
 * incomparable between a loop that allocates and one that does not. A step is
 * roughly "one node visited", and 64 bytes is roughly the cost of a Value, so
 * a step per 64 bytes puts allocation and execution on the same scale: the
 * fifty-million-element list above costs about 50 million steps rather than 1.
 */
#define ZI_BYTES_PER_STEP 64

static void zi_charge(long long units);      /* defined with zi_step below */

static void *zi_alloc(unsigned long bytes)
{
    zi_charge((long long)(bytes / ZI_BYTES_PER_STEP) + 1);
    return malloc(bytes);
}

static char *zi_strdup(const char *s)
{
    zi_charge((long long)(s ? (strlen(s) / ZI_BYTES_PER_STEP) : 0) + 1);
    return _strdup(s ? s : "");
}

static void *zi_realloc(void *p, unsigned long bytes)
{
    zi_charge((long long)(bytes / ZI_BYTES_PER_STEP) + 1);
    return realloc(p, bytes);
}


/* ---- the memory window: which addresses a program may touch ---------------
 * FOUND BY AN ADVERSARIAL READER, AND IT IS THE WHOLE OF NON-NEGOTIABLE 3.
 *
 *     poke32(0, 1)                    -> SIGSEGV
 *     x = peek8(0)                    -> SIGSEGV
 *     fill_mem(0, 0, 1000000000000)   -> SIGSEGV
 *
 * Three characters of zl, and on a hosted Linux that is a dead process. In the
 * kernel - ring 0, no memory protection, which is exactly the situation
 * EXEC-PROMPT.md describes - it is a dead machine, and none of the step budget
 * or the depth cap or the longjmp trap comes anywhere near it. A signal is not
 * a longjmp and there is no handler; a page fault in ring 0 is not a signal at
 * all.
 *
 * The budget bounds TIME and the arena bounds HOW MUCH memory. Neither bounds
 * WHICH memory, and the raw-memory builtins - peek*, poke*, alloc, copy_mem,
 * fill_mem - exist precisely to hand out arbitrary addresses. They are not a
 * mistake: design_memory_structs.md §3.1 fixes their names because a page
 * allocator is written against them. They are how zl drives hardware. But a
 * program the kernel was not built with is not the kernel.
 *
 * So there is a WINDOW. zi_confine(lo, hi) says "this program may touch
 * [lo, hi) and nothing else"; every raw access is checked against it and a
 * violation is an ordinary runtime_error, which the trap already catches and
 * reports. Set to (0, 0) - the default - there is no window and nothing is
 * checked, which is what the hosted interpreter and kernel.zl's own compiled
 * code have always had. Only a foreign program gets confined, and the kernel
 * confines it to exactly the arena.
 *
 * The check is a subtraction, not an addition: `len > hi - addr` cannot wrap,
 * where `addr + len > hi` wraps for a length a script chose. That is the same
 * form arena.c uses for its ceiling and for the same reason.
 */
static void runtime_error(const char *msg);   /* the trap - see below */
static void zi_charge(long long units);

static unsigned long long zi_win_lo, zi_win_hi;

void zi_confine(unsigned long long lo, unsigned long long hi)
{
    zi_win_lo = lo;
    zi_win_hi = hi;
}

static void zi_check(unsigned long long addr, unsigned long long len,
                     const char *what)
{
    if (zi_win_hi == 0) return;                 /* no window: unconfined */
    if (addr < zi_win_lo || addr >= zi_win_hi) {
        zi_killed = 1;
        runtime_error(what);
    }
    if (len > zi_win_hi - addr) {               /* subtraction: cannot wrap */
        zi_killed = 1;
        runtime_error(what);
    }
}

/* ---- what a confined program may not do at all ---------------------------
 * The window above says which ADDRESSES a program may touch. This says which
 * BUILTINS it may call, and it exists because an adversarial reader found two
 * more ways past everything else:
 *
 *     run("sleep 3600")     one eval() node, one step, blocks forever. The
 *                           budget counts statements, not seconds, so a
 *                           builtin that waits is unbounded no matter how
 *                           small the budget is.
 *     exit(0)               calls exit() directly. Not an error, so the trap
 *                           never sees it; in a kernel it is the machine.
 *
 * Neither is a bug in those builtins - they are exactly what a shell language
 * should have, and kernel.zl's own compiled code may want them. They are
 * simply not things a program the kernel was NOT built with gets to do, and
 * the honest way to say that is to refuse by name rather than to quietly
 * return nil, which would look like the call had worked.
 *
 * The list is of everything that reaches OUTSIDE the interpreter: the host
 * process, the filesystem, the clock, other programs. A confined program is a
 * pure computation over its own arena, and that is the whole of what Level 1
 * promised.
 */
static const char *const ZI_FORBIDDEN[] = {
    "run", "start", "kill", "procs", "exit", "input",
    "read", "write", "write_bytes", "dir", "rm", "move", "copy",
    "env", "now",
    0
};
/* NOT on the list, deliberately, and each for a reason:
 *   seed    is srand() of a number the program supplies. It reaches nothing
 *           outside the interpreter, and a confined program that wants
 *           REPRODUCIBLE randomness needs it. Refusing it would be strictness
 *           that costs something and buys nothing.
 *   random,
 *   randint are pure reads of that state.
 *   print   is the whole point - a program that cannot say anything is not
 *           worth running. It goes to the terminal, which is where the person
 *           who typed `run` is looking.
 * `now` IS on the list only because it is clock(), which has no freestanding
 * implementation yet - an honest "not available" rather than a wrong number. */

static void zi_forbid(const char *name)
{
    if (zi_win_hi == 0) return;                 /* unconfined: everything allowed */
    for (int i = 0; ZI_FORBIDDEN[i]; i++) {
        if (strcmp(name, ZI_FORBIDDEN[i]) == 0) {
            char buf[96];
            snprintf(buf, sizeof buf,
                     "'%s' is not available to a program run this way", name);
            zi_killed = 1;
            runtime_error(buf);
        }
    }
}


/* =============================================================
 * VALUES - what an expression evaluates to
 * ============================================================= */

typedef enum { V_NIL, V_NUM, V_STR, V_BOOL, V_LIST, V_FN } ValueType;

typedef struct Value {
    ValueType type;
    double    num;                 /* V_NUM, and V_BOOL (0/1)     */
    char     *str;                 /* V_STR (malloc'd)            */
    struct Value **items;          /* V_LIST                      */
    int            nitems;
    int            cap;            /* V_LIST spare capacity (amortized push) */
    int           *tip;            /* V_LIST slots handed out - see push()  */
    Node          *fn;             /* V_FN - points at the N_FN   */
} Value;

/* Value is a STRUCT, not a union, so building one by setting only the
 * fields that type uses leaves the others INDETERMINATE. make_str left
 * .num unwritten and make_nil left everything but .type unwritten, so
 * any read of .num on a string or nil picked up whatever was on the
 * stack. runtime.c has always zeroed the whole struct in zl_nil(), so
 * the C backend saw 0 where the interpreter saw garbage - the two
 * engines disagreed on the same program. Every constructor here now
 * starts from a fully zeroed struct, exactly like zl_nil(). */
static Value make_nil(void)          { Value v; memset(&v, 0, sizeof(v)); v.type=V_NIL; return v; }
static Value make_num(double n)      { Value v = make_nil(); v.type=V_NUM;  v.num=n; return v; }
static Value make_bool(int b)        { Value v = make_nil(); v.type=V_BOOL; v.num=b?1:0; return v; }

static Value make_str(const char *s)
{
    Value v = make_nil(); v.type = V_STR;
    v.str = zi_alloc(strlen(s) + 1);
    strcpy(v.str, s);
    return v;
}

/* truthiness: what counts as "yes" in an if/while */
static int is_truthy(Value v)
{
    switch (v.type) {
        case V_NIL:  return 0;
        case V_BOOL: return v.num != 0;
        case V_NUM:  return v.num != 0;
        case V_STR:  return v.str && v.str[0] != '\0';
        case V_LIST: return v.nitems > 0;
        case V_FN:   return 1;
    }
    return 0;
}

/* How deep a Value may nest before an engine refuses to walk it.
 *
 * A zl list can CONTAIN ITSELF - `xs = [1, 2]` then `xs[0] = xs`, which
 * index-assign allows because it stores a Value whose .items is the very
 * array being written into. Every recursive walk over a Value therefore
 * needs a floor under it, or a cyclic list is an uncatchable
 * STATUS_STACK_OVERFLOW: no message, and every buffered line of output
 * lost with the process.
 *
 * The cap is far above any real nesting (a list 256 deep is not a data
 * structure anyone typed) and small enough that the frames fit the 1 MB
 * default stack a COMPILED zl program gets, not just interp.exe's 64 MB.
 * Shared by value_to_string here, values_equal and value_compare below,
 * and mirrored in runtime.c. */
#define MAX_VALUE_DEPTH 256

static void runtime_error(const char *msg);   /* defined with the rest
                                                 of the machinery below */

/* turn any value into text (for print and for string joining) */
static char *value_to_string_depth(Value v, int depth)
{
    char buf[64];
    switch (v.type) {
        case V_NIL:  return zi_strdup("nil");
        case V_BOOL: return zi_strdup(v.num ? "true" : "false");
        case V_STR:  return zi_strdup(v.str ? v.str : "");
        case V_FN:   return zi_strdup("<function>");
        case V_NUM:
            /* whole numbers print without a trailing ".000000" */
            if (v.num == (long long)v.num)
                snprintf(buf, sizeof(buf), "%lld", (long long)v.num);
            else
                snprintf(buf, sizeof(buf), "%g", v.num);
            return zi_strdup(buf);
        case V_LIST: {
            /* [a, b, c] */
            size_t cap = 3;
            char  *out;
            if (depth >= MAX_VALUE_DEPTH)
                runtime_error("str nested too deep to print "
                              "(does a list contain itself?)");
            out = zi_alloc(cap);
            strcpy(out, "[");
            for (int i = 0; i < v.nitems; i++) {
                char *part = value_to_string_depth(*v.items[i], depth + 1);
                cap += strlen(part) + 2;
                out = zi_realloc(out, cap);
                if (i) strcat(out, ", ");
                strcat(out, part);
                free(part);
            }
            strcat(out, "]");
            return out;
        }
    }
    return zi_strdup("?");
}

static char *value_to_string(Value v) { return value_to_string_depth(v, 0); }

/* =============================================================
 * ENVIRONMENT - where variables live
 * ============================================================= */

typedef struct Var {
    char        name[MAX_TEXT];
    Value       val;
    struct Var *next;
} Var;

typedef struct Env {
    struct Env *parent;
    Var        *vars;
} Env;

static Env *env_new(Env *parent)
{
    Env *e = zi_alloc(sizeof(Env));
    e->parent = parent;
    e->vars   = NULL;
    return e;
}

/* find a variable's slot, searching this scope then outer scopes */
static Var *env_find(Env *e, const char *name)
{
    for (Env *s = e; s != NULL; s = s->parent)
        for (Var *v = s->vars; v != NULL; v = v->next)
            if (strcmp(v->name, name) == 0) return v;
    return NULL;
}

/* find a variable's slot in THIS scope only - used to tell "already
 * bound here" from "visible from here". */
static Var *env_find_local(Env *e, const char *name)
{
    for (Var *v = e->vars; v != NULL; v = v->next)
        if (strcmp(v->name, name) == 0) return v;
    return NULL;
}

/* define: create a NEW slot in this scope, whatever exists outside it.
 * This is what BINDING a name does (a function's parameters), as opposed
 * to assigning to one. Binding used to go through env_assign, which
 * walks the parent chain, so calling f(1) where a global `g` existed and
 * the parameter was also called `g` OVERWROTE the global - no assignment
 * needed, the call alone did it. The C backend never had this, because
 * a parameter there is a C function argument and shadows naturally. */
static void env_define(Env *e, const char *name, Value val)
{
    Var *v = zi_alloc(sizeof(Var));
    strncpy(v->name, name, MAX_TEXT - 1);
    v->name[MAX_TEXT - 1] = '\0';
    v->val  = val;
    v->next = e->vars;
    e->vars = v;
}

/* assign: update an existing variable anywhere up the chain,
 * otherwise create it in the current (innermost) scope.
 *
 * NOTE, deliberate and shared with the C backend: assigning a name
 * inside a function writes the GLOBAL of that name when one exists.
 * That is what lets the self-hosted compiler share a cursor across
 * functions. Only parameter binding is scoped; see env_define. */
static void env_assign(Env *e, const char *name, Value val)
{
    Var *found = env_find(e, name);
    if (found) { found->val = val; return; }
    env_define(e, name, val);
}

/* =============================================================
 * FORWARD DECLARATIONS
 * ============================================================= */

static Value eval(Node *n, Env *env);
static Value eval_inner(Node *n, Env *env);

/* statements can trigger a `return`. We carry that back up with a
 * flag + the returned value, checked after every statement. */
static int   g_returning = 0;
static int   g_breaking = 0;      /* set by `break`    - exits the loop  */
static int   g_continuing = 0;    /* set by `continue` - next iteration  */
static Value g_return_value;

/* eval/exec recurse on the C stack, so zl recursion used to be bounded
 * only by the OS thread stack - about 165 zl frames, at which point the
 * process was KILLED (STATUS_STACK_OVERFLOW) with no message and with
 * every buffered line of output lost. A recursive walk over a 200-item
 * list was enough to reach it. Counting the depth turns that into the
 * ordinary runtime_error path; build.bat also gives interp.exe a bigger
 * stack so the limit, not the stack, is what you hit. */
#define MAX_CALL_DEPTH 2000
static int   g_depth = 0;

/* the global (top-level) scope. Function calls hang their local
 * scope off THIS, not off the caller - so a function sees globals
 * and its own locals, but not the caller's locals. Without this the
 * language has dynamic scoping and nested calls that reuse a
 * variable name clobber each other. (Bug found by self-hosting.) */
static Env *g_global = NULL;

static void exec(Node *n, Env *env);
static void exec_inner(Node *n, Env *env);

static void runtime_error(const char *msg)
{
    /* THE BOUNDARY. With a trap armed this unwinds to whoever armed it and the
     * caller decides what to do; with none it exits, exactly as it always has.
     * A kernel cannot afford the second: exit(1) there is the machine.
     *
     * The message is COPIED before unwinding. Several callers pass a pointer
     * into a local buffer (see N_IDENT's "doesn't exist yet"), and that buffer
     * is gone the moment the stack unwinds past it - so handing the caller the
     * pointer would be a read of dead stack, which is the shape of bug that
     * reproduces on one build and not the other. */
    if (zi_trap_armed) {
        zi_seterr(msg);
        zi_trap_armed = 0;
        zi_longjmp(zi_trap, 1);
    }
    fflush(stdout);            /* stdout is block-buffered when redirected;
                                  without this the output that explains HOW
                                  we got here dies with the process. */
    fprintf(stderr, "runtime error: %s\n", msg);
    exit(1);
}

/* =============================================================
 * NARROWING A NUMBER TO A C INTEGER
 *
 * zl's only number type is a double. Converting a double to a C
 * integer type is UNDEFINED BEHAVIOUR in C whenever the truncated
 * value does not fit, and whenever the value is NaN. On x86-64 the
 * cast quietly produces the "integer indefinite" pattern - INT32_MIN
 * or INT64_MIN - so an enormous count used to read back as a NEGATIVE
 * number and then clamp to zero (take(xs, 2^31) returned nothing
 * instead of everything), and NaN used to read back as a valid index.
 *
 * Every builtin that needs a C integer now goes through one of these
 * two helpers instead of casting directly.
 * ============================================================= */

/* a list/string count or index: clamped into [lo, hi] while it is still
 * a double, so nothing can overflow on the way in. NaN clamps to lo -
 * every comparison against NaN is false, so it must be handled first. */
static int clamp_index(double d, int lo, int hi)
{
    if (d != d)          return lo;             /* NaN */
    if (d <= (double)lo) return lo;
    if (d >= (double)hi) return hi;
    return (int)d;
}

/* =============================================================
 * THE INTEGER-WIDTH POLICY  (one rule, applied everywhere below)
 *
 * A zl number IS a double. A double represents every integer up to
 * 2^53 exactly, some integers above it, and nothing at all outside
 * +/-2^63 that a C `long long` can hold. So:
 *
 *   1. A builtin whose meaning is BITWISE (band bor bxor bnot shl shr)
 *      only has an answer when both the operands and the result are
 *      integers the double represents EXACTLY. When they are not, it
 *      raises a runtime error. It never rounds and hands the rounded
 *      value back as if it were the answer, and it never casts an
 *      out-of-range double to long long (undefined in C; on x86-64 it
 *      silently yields INT64_MIN). exact_i64 enforces this on the way
 *      in, bit_result on the way out. Either you get the right bits or
 *      you get told you cannot.
 *
 *   2. A builtin whose meaning is a CONVERSION (int, hex) never has to
 *      refuse, because neither one loses information: trunc() is exact
 *      for every double including the infinities, and hex() falls back
 *      to printing the double's own exact digits past 2^63 rather than
 *      saturating. So these stay total - no error, no narrowing cast.
 *
 * The rule is deliberately NOT "round quietly": a silent wrong answer
 * in the top 11 bits of a word is the worst outcome of the three. The
 * way to lift the 53-bit ceiling is to give zl an exact integer type,
 * not to patch these builtins one at a time - the same ceiling already
 * applies to a LITERAL (4611686018427387903 parses as ...904).
 *
 * runtime.c carries a byte-for-byte copy of all of this. The two must
 * agree or the interpreter and a compiled program disagree, and
 * verify.ps1 cannot see it because both engines would be wrong the
 * same way.
 * ============================================================= */

/* an exact 64-bit integer operand (the bitwise builtins). A value that
 * is not a 64-bit integer has no bitwise meaning, so we refuse it. These
 * builtins already refuse a non-number argument; quietly turning NaN or
 * 2^70 into INT64_MIN is strictly worse than saying so out loud. */
static long long exact_i64(double d, const char *who)
{
    if (d != d || d < -9223372036854775808.0 || d >= 9223372036854775808.0) {
        char buf[96];
        snprintf(buf, sizeof(buf),
                 "%s needs 64-bit integers (got NaN or a value out of range)", who);
        runtime_error(buf);
    }
    return (long long)d;
}

/* the RESULT of a bitwise builtin, on its way back out through a double.
 *
 * The builtins compute an exact int64, but zl's only number type is a
 * double, which carries 53 significant bits. An int64 result needing more
 * than that used to be ROUNDED on the way out and handed back as if it
 * were the answer: bor(2^53, 1) returned 2^53, so the bit that had just
 * been set was gone, bxor(x, 1) could be a no-op, and bnot stopped being
 * an involution. Setting, clearing or toggling a bit in the top 11 bits
 * of a word was silently nothing.
 *
 * We refuse instead. The test is exact - round-trip the value and see
 * whether it survived - so it fires only when precision is really lost,
 * and never for a result that a double holds exactly (shl(1,62) is
 * fine). It is the same policy exact_i64 applies to the operands: a
 * bitwise builtin either gives you the right bits or says it cannot.
 *
 * The way to lift this is to give zl an exact integer representation,
 * not to patch these six functions - the same 53-bit limit already
 * applies to a literal, and 4611686018427387903 parses as ...904. See
 * docs/REFERENCE.md. */
static Value bit_result(long long r, const char *who)
{
    double d = (double)r;
    /* d can round UP to exactly 2^63, which is outside long long, so the
     * range has to be tested before casting back - that cast would be
     * undefined otherwise. (It cannot round below -2^63: that value is
     * exact.) */
    if (d >= 9223372036854775808.0 || (long long)d != r) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "%s result needs more than 53 bits of precision - "
                 "a zl number is a double and cannot hold it exactly", who);
        runtime_error(buf);
    }
    return make_num(d);
}

/* hex(x): x truncated toward zero, written in hexadecimal.
 *
 * A value that fits in a signed 64-bit integer keeps the two's-complement
 * view the language already had (hex(-1) == "ffffffffffffffff"). Past
 * that a double is still an exact integer, just a bigger one, so we print
 * its exact digits rather than saturating - hex(2^63), hex(2^64) and
 * hex(2^65) all used to come back as the same "8000000000000000".
 *
 * A double is mantissa * 2^exp with a 53-bit integer mantissa, so the
 * exact digits are the mantissa's digits followed by exp/4 zeros, once
 * the mantissa has absorbed the leftover exp%4 bits. */
#define HEX_BUF 288        /* 1024/4 digits + mantissa + sign + NUL */

static void hex_of_double(char *out, double v)
{
    if (v != v || v == HUGE_VAL || v == -HUGE_VAL)
        runtime_error("hex needs a finite number");
    v = trunc(v);

    if (v >= -9223372036854775808.0 && v < 9223372036854775808.0) {
        snprintf(out, HEX_BUF, "%llx", (unsigned long long)(long long)v);
        return;
    }

    int    neg = v < 0;
    double a   = neg ? -v : v;
    int    e2;
    double f    = frexp(a, &e2);                    /* a = f * 2^e2  */
    unsigned long long mant = (unsigned long long)ldexp(f, 53);
    int    exp  = e2 - 53;                          /* a = mant * 2^exp */
    while (mant != 0 && (mant & 1) == 0) { mant >>= 1; exp++; }

    char *p = out;
    if (neg) *p++ = '-';
    p += snprintf(p, 24, "%llx", mant << (exp & 3));
    for (int i = 0, z = exp >> 2; i < z; i++) *p++ = '0';
    *p = '\0';
}

/* =============================================================
 * BUILT-IN FUNCTIONS
 * ============================================================= */

static int   values_equal(Value l, Value r);
static int   value_compare(const Value *a, const Value *b);

/* poke/peek/window stay simulated (see runtime.c for why). kill/start/rm/
 * copy/move/run got a REAL Linux implementation - see call_builtin. */
static const char *SIMULATED[] = {
    "poke", "peek", "window", NULL
};

static int is_simulated(const char *name)
{
    for (int i = 0; SIMULATED[i]; i++)
        if (strcmp(name, SIMULATED[i]) == 0) return 1;
    return 0;
}

/* find a running process's pid by exact command name (as seen in `procs()`) */
static long find_pid_by_name(const char *want)
{
    DIR *d = opendir("/proc");
    if (!d) return -1;
    long pid = -1;
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        int is_pid = 1;
        for (char *p = ent->d_name; *p; p++) if (!isdigit((unsigned char)*p)) { is_pid = 0; break; }
        if (!is_pid || ent->d_name[0] == '\0') continue;

        char path[300], comm[256];
        snprintf(path, sizeof(path), "/proc/%s/comm", ent->d_name);
        FILE *f = fopen(path, "r");
        if (!f) continue;
        int matched = 0;
        if (fgets(comm, sizeof(comm), f)) {
            comm[strcspn(comm, "\n")] = 0;
            matched = (strcmp(comm, want) == 0);
        }
        fclose(f);
        if (matched) { pid = atol(ent->d_name); break; }
    }
    closedir(d);
    return pid;
}

/* helper: append a length-limited C string to a V_LIST as a V_STR */
static void list_push_str(Value *list, int *cap, const char *p, int len)
{
    if (list->nitems == *cap) {
        *cap *= 2;
        list->items = zi_realloc(list->items, sizeof(Value*) * (size_t)(*cap));
    }
    char *buf = zi_alloc((size_t)len + 1);
    memcpy(buf, p, (size_t)len);
    buf[len] = '\0';
    list->items[list->nitems] = zi_alloc(sizeof(Value));
    Value v = make_nil(); v.type = V_STR; v.str = buf;
    *list->items[list->nitems] = v;
    list->nitems++;
}

/* run a built-in by name, given already-evaluated argument values */
static Value call_builtin(const char *name, Value *args, int nargs)
{
    /* One gate, at the one door. Putting this at each dangerous builtin
     * instead would be a list that the next dangerous builtin is not on. */
    zi_forbid(name);
    /* print(...) - the real one */
    if (strcmp(name, "print") == 0) {
        for (int i = 0; i < nargs; i++) {
            char *s = value_to_string(args[i]);
            if (i) printf(" ");
            printf("%s", s);
            free(s);
        }
        printf("\n");
        return make_nil();
    }

    /* input("prompt?") - reads a real line from the keyboard */
    if (strcmp(name, "input") == 0) {
        if (nargs > 0) { char *p = value_to_string(args[0]); printf("%s", p); free(p); }
        char line[512];
        if (fgets(line, sizeof(line), stdin)) {
            line[strcspn(line, "\r\n")] = '\0';
            return make_str(line);
        }
        return make_str("");
    }

    /* ---- text tools: needed to do real work with strings ---- */

    /* len(x) -> number of characters in a string, or items in a list */
    if (strcmp(name, "len") == 0) {
        if (nargs < 1) runtime_error("len needs an argument");
        if (args[0].type == V_STR)  return make_num((double)strlen(args[0].str));
        if (args[0].type == V_LIST) return make_num((double)args[0].nitems);
        runtime_error("len needs a string or a list");
    }

    /* lines(text) -> a list of the lines in the text */
    if (strcmp(name, "lines") == 0) {
        if (nargs < 1 || args[0].type != V_STR) runtime_error("lines needs a string");
        Value list = make_nil(); list.type = V_LIST; list.nitems = 0; list.cap = 0;
        int cap = 8;
        list.items = zi_alloc(sizeof(Value*) * (size_t)cap);

        const char *s = args[0].str, *start = s;
        while (*s) {
            if (*s == '\n') {
                int L = (int)(s - start);
                if (L > 0 && start[L-1] == '\r') L--;      /* strip CR */
                list_push_str(&list, &cap, start, L);
                start = s + 1;
            }
            s++;
        }
        if (s > start) {                                    /* last partial line */
            int L = (int)(s - start);
            if (L > 0 && start[L-1] == '\r') L--;
            list_push_str(&list, &cap, start, L);
        }
        return list;
    }

    /* ends(text, suffix) -> true if text ends with suffix */
    if (strcmp(name, "ends") == 0) {
        if (nargs < 2 || args[0].type != V_STR || args[1].type != V_STR)
            runtime_error("ends needs two strings");
        size_t tl = strlen(args[0].str), sl = strlen(args[1].str);
        return make_bool(tl >= sl && strcmp(args[0].str + tl - sl, args[1].str) == 0);
    }

    /* has(text, part) -> true if part appears anywhere in text */
    if (strcmp(name, "has") == 0) {
        if (nargs < 2 || args[0].type != V_STR || args[1].type != V_STR)
            runtime_error("has needs two strings");
        return make_bool(strstr(args[0].str, args[1].str) != NULL);
    }

    /* at(text, i) -> the single character at position i as a string
     * ("" if out of range). Lets zl code walk a string char by char. */
    if (strcmp(name, "at") == 0) {
        if (nargs < 2 || args[0].type != V_STR || args[1].type != V_NUM)
            runtime_error("at needs a string and a number");
        size_t L = strlen(args[0].str);
        /* range-test the DOUBLE: the cast is only defined once we know the
         * value fits. NaN fails the test (every NaN comparison is false)
         * and so reads as out of range, which is the answer we want. */
        if (!(args[1].num >= 0 && args[1].num < (double)L)) return make_str("");
        int i = (int)args[1].num;
        char b[2]; b[0] = args[0].str[i]; b[1] = '\0';
        return make_str(b);
    }

    /* push(list, item) -> a NEW list with item added on the end.
     * Lets zl code grow a list (e.g. a list of tokens).
     *
     * The new list gets its OWN items array. It used to reuse the source
     * list's array and grow it in place ("amortized O(1)"), which meant
     * push WROTE INTO ITS INPUT: two pushes branched off one base shared
     * the new slot, so the second silently overwrote the first result,
     * and the realloc on the growth path freed an array the caller still
     * pointed at (heap corruption / access violation). A builtin
     * documented to return a NEW list may not touch its argument.
     *
     * Only the pointer array is copied, so the elements stay shared -
     * exactly as they were before, since the old code handed back the
     * very same array. */
    /* push(list, item) -> a NEW list, by TIP TRACKING.
     *
     * Each items array carries a shared counter, .tip: how many of its
     * slots have ever been handed out. A push may append IN PLACE only
     * when the source is the newest tip of its array (nitems == *tip)
     * and there is spare capacity. Branch a second push off the same
     * base and it sees nitems != *tip, so it copies instead - the two
     * results can never share slot m.
     *
     * This is what the .cap field was always for. Two earlier versions
     * both got it wrong:
     *   - appending in place UNCONDITIONALLY (the original) let a
     *     branched push overwrite the first result, and because every
     *     constructor set cap == nitems the growth path reallocated an
     *     array the source variable still pointed at: a use-after-free.
     *   - copying on EVERY call (the fix for that) was correct but
     *     O(n) per push, so building a list was O(n^2). It took the
     *     examples from 430ms / 14MB to 61s / 2.6GB and made four of
     *     the six die on an unchecked malloc.
     * Tip tracking keeps the branched case copying - so the aliasing
     * assertions in tests/test_lists.zl still hold - while making the
     * `xs = push(xs, v)` loop amortized O(1) again.
     *
     * A list from any other constructor has tip == NULL (every Value
     * starts fully zeroed), so it takes the copy path and the result
     * begins a fresh tip of its own. */
    if (strcmp(name, "push") == 0) {
        if (nargs < 2 || args[0].type != V_LIST)
            runtime_error("push needs a list and an item");
        Value src = args[0];
        int m = src.nitems;

        if (src.items && src.tip && *src.tip == m && m < src.cap) {
            src.items[m] = zi_alloc(sizeof(Value));
            *src.items[m] = args[1];
            *src.tip = m + 1;
            src.nitems = m + 1;      /* same array, same tip, same cap */
            return src;
        }

        int want = m + 1;
        int newcap = (want > 1073741823) ? want : want * 2;
        if (newcap < 8) newcap = 8;
        Value v = make_nil(); v.type = V_LIST; v.nitems = want; v.cap = newcap;
        v.items = zi_alloc(sizeof(Value*) * (size_t)newcap);
        if (m > 0) memcpy(v.items, src.items, sizeof(Value*) * (size_t)m);
        v.items[m] = zi_alloc(sizeof(Value));
        *v.items[m] = args[1];
        v.tip = zi_alloc(sizeof(int)); *v.tip = want;
        return v;
    }

    /* num(x) -> x as a number (parses a string) */
    if (strcmp(name, "num") == 0) {
        if (nargs < 1) runtime_error("num needs an argument");
        if (args[0].type == V_NUM) return args[0];
        if (args[0].type == V_STR) return make_num(atof(args[0].str));
        runtime_error("num needs a string or number");
    }

    /* int(x) -> x truncated toward zero (integer part).
     * trunc() already IS "truncated toward zero" for every double, and
     * unlike the long long round-trip it stays defined outside 64-bit
     * range: int(+inf) used to come back NEGATIVE (LLONG_MIN). */
    if (strcmp(name, "int") == 0) {
        if (nargs < 1) runtime_error("int needs an argument");
        if (args[0].type == V_NUM) return make_num(trunc(args[0].num));
        if (args[0].type == V_STR) return make_num(trunc(atof(args[0].str)));
        runtime_error("int needs a number or string");
    }

    /* abs / min / max on numbers */
    if (strcmp(name, "abs") == 0) {
        if (nargs < 1 || args[0].type != V_NUM) runtime_error("abs needs a number");
        double v = args[0].num; return make_num(v < 0 ? -v : v);
    }
    /* min/max follow IEEE-754 minNum/maxNum (C's fmin/fmax): a NaN on
     * either side is IGNORED, so the answer does not depend on which
     * operand it was. Without the NaN test every comparison is false and
     * the ternary always fell through to the SECOND argument, making
     * min(1,nan) and min(nan,1) disagree. */
    if (strcmp(name, "min") == 0) {
        if (nargs < 2 || args[0].type != V_NUM || args[1].type != V_NUM) runtime_error("min needs two numbers");
        if (args[0].num != args[0].num) return args[1];
        if (args[1].num != args[1].num) return args[0];
        return args[0].num <= args[1].num ? args[0] : args[1];
    }
    if (strcmp(name, "max") == 0) {
        if (nargs < 2 || args[0].type != V_NUM || args[1].type != V_NUM) runtime_error("max needs two numbers");
        if (args[0].num != args[0].num) return args[1];
        if (args[1].num != args[1].num) return args[0];
        return args[0].num >= args[1].num ? args[0] : args[1];
    }

    /* contains(list, x) / index_of(list, x) */
    if (strcmp(name, "contains") == 0 || strcmp(name, "index_of") == 0) {
        if (nargs < 2 || args[0].type != V_LIST) runtime_error("contains/index_of need a list and a value");
        int want_index = (strcmp(name, "index_of") == 0);
        for (int i = 0; i < args[0].nitems; i++)
            if (values_equal(*args[0].items[i], args[1]))
                return want_index ? make_num(i) : make_bool(1);
        return want_index ? make_num(-1) : make_bool(0);
    }

    /* sort(list) -> a NEW list, sorted by value_compare's total order. */
    if (strcmp(name, "sort") == 0) {
        if (nargs < 1 || args[0].type != V_LIST) runtime_error("sort needs a list");
        int m = args[0].nitems;
        Value v = make_nil(); v.type = V_LIST; v.nitems = m; v.cap = m;
        v.items = zi_alloc(sizeof(Value*) * (m > 0 ? m : 1));
        for (int i = 0; i < m; i++) { v.items[i] = zi_alloc(sizeof(Value)); *v.items[i] = *args[0].items[i]; }
        for (int i = 1; i < m; i++) {           /* insertion sort */
            Value *key = v.items[i]; int j = i - 1;
            while (j >= 0 && value_compare(v.items[j], key) > 0) { v.items[j+1] = v.items[j]; j--; }
            v.items[j+1] = key;
        }
        return v;
    }

    /* slice(text, start, end) -> the substring [start, end).
     * Both ends clamp into 0..len. The clamp has to happen BEFORE the
     * narrowing to int, or an end index past INT_MAX lands on INT_MIN and
     * the clamp can no longer rescue it - slice(s, 0, 1e20) used to
     * return "" where slice(s, 0, 99) correctly returns the whole
     * string. */
    if (strcmp(name, "slice") == 0) {
        if (nargs < 3 || args[0].type != V_STR || args[1].type != V_NUM || args[2].type != V_NUM)
            runtime_error("slice needs a string and two numbers");
        int L = (int)strlen(args[0].str);
        int a = clamp_index(args[1].num, 0, L), b = clamp_index(args[2].num, 0, L);
        if (a > b) a = b;
        int m = b - a;
        char *buf = zi_alloc((size_t)m + 1);
        memcpy(buf, args[0].str + a, (size_t)m);
        buf[m] = '\0';
        Value v = make_nil(); v.type = V_STR; v.str = buf; return v;
    }

    /* find(text, part) -> index of part in text, or -1 */
    if (strcmp(name, "find") == 0) {
        if (nargs < 2 || args[0].type != V_STR || args[1].type != V_STR)
            runtime_error("find needs two strings");
        char *p = strstr(args[0].str, args[1].str);
        if (!p) return make_num(-1);
        return make_num((double)(p - args[0].str));
    }

    /* upper(text) / lower(text) -> case-converted copy */
    if (strcmp(name, "upper") == 0 || strcmp(name, "lower") == 0) {
        if (nargs < 1 || args[0].type != V_STR) runtime_error("upper/lower needs a string");
        int up = (strcmp(name, "upper") == 0);
        size_t L = strlen(args[0].str);
        char *buf = zi_alloc(L + 1);
        for (size_t i = 0; i < L; i++) {
            char ch = args[0].str[i];
            if (up  && ch >= 'a' && ch <= 'z') ch = (char)(ch - 32);
            if (!up && ch >= 'A' && ch <= 'Z') ch = (char)(ch + 32);
            buf[i] = ch;
        }
        buf[L] = '\0';
        Value v = make_nil(); v.type = V_STR; v.str = buf; return v;
    }

    /* join(list, sep) -> the items joined into one string */
    if (strcmp(name, "join") == 0) {
        if (nargs < 2 || args[0].type != V_LIST || args[1].type != V_STR)
            runtime_error("join needs a list and a separator string");
        size_t cap = 1;
        char *out = zi_alloc(cap); out[0] = '\0';
        for (int i = 0; i < args[0].nitems; i++) {
            char *part = value_to_string(*args[0].items[i]);
            cap += strlen(part) + strlen(args[1].str);
            out = zi_realloc(out, cap);
            if (i) strcat(out, args[1].str);
            strcat(out, part);
            free(part);
        }
        Value v = make_nil(); v.type = V_STR; v.str = out; return v;
    }

    /* split(text, sep) -> a list of the pieces between separators */
    if (strcmp(name, "split") == 0) {
        if (nargs < 2 || args[0].type != V_STR || args[1].type != V_STR)
            runtime_error("split needs two strings");
        Value list = make_nil(); list.type = V_LIST; list.nitems = 0; list.cap = 0;
        int cap = 8;
        list.items = zi_alloc(sizeof(Value*) * (size_t)cap);
        const char *sep = args[1].str;
        size_t seplen = strlen(sep);
        const char *start = args[0].str;
        if (seplen > 0) {
            const char *p;
            while ((p = strstr(start, sep)) != NULL) {
                list_push_str(&list, &cap, start, (int)(p - start));
                start = p + seplen;
            }
        }
        list_push_str(&list, &cap, start, (int)strlen(start));
        return list;
    }

    /* ---- REAL PC-control: reading is safe, so these are real ---- */

    /* read("path") -> the whole file as a string */
    if (strcmp(name, "read") == 0) {
        if (nargs < 1 || args[0].type != V_STR) runtime_error("read needs a filename");
        FILE *f = fopen(args[0].str, "rb");
        if (!f) { runtime_error("read: can't open that file"); }
        fseek(f, 0, SEEK_END); long sz = ftell(f); rewind(f);
        char *buf = zi_alloc((size_t)sz + 1);
        size_t got = fread(buf, 1, (size_t)sz, f);
        buf[got] = '\0';
        fclose(f);
        Value v = make_nil(); v.type = V_STR; v.str = buf;
        return v;
    }

    /* write("path", "text") -> creates/overwrites a real file */
    if (strcmp(name, "write") == 0) {
        if (nargs < 2 || args[0].type != V_STR) runtime_error("write needs a filename and text");
        FILE *f = fopen(args[0].str, "wb");
        if (!f) { runtime_error("write: can't create that file"); }
        char *text = value_to_string(args[1]);
        fputs(text, f);
        free(text);
        fclose(f);
        return make_nil();
    }

    /* code("A") -> the byte value of a string's first character (65).
     * The inverse of building bytes; lets zl turn text into raw bytes. */
    if (strcmp(name, "code") == 0) {
        if (nargs < 1 || args[0].type != V_STR) runtime_error("code needs a string");
        return make_num((double)(unsigned char)args[0].str[0]);
    }

    /* write_bytes("path", [n, n, ...]) -> write a list of byte values as
     * a real binary file. zl strings are NUL-terminated and cannot hold
     * binary, so this is how a zl program emits a .exe. */
    if (strcmp(name, "write_bytes") == 0) {
        if (nargs < 2 || args[0].type != V_STR || args[1].type != V_LIST)
            runtime_error("write_bytes needs a filename and a list of byte values");
        FILE *f = fopen(args[0].str, "wb");
        if (!f) { runtime_error("write_bytes: can't create that file"); }
        for (int i = 0; i < args[1].nitems; i++) {
            unsigned char byte = (unsigned char)(long long)args[1].items[i]->num;
            fputc(byte, f);
        }
        fclose(f);
        return make_nil();
    }

    /* dir("path") -> a real LIST of the filenames in that folder.
     * The actual Windows call lives in os_win.c; we just wrap the
     * result up as a list of string values here. */
    if (strcmp(name, "dir") == 0) {
        const char *path = (nargs > 0 && args[0].type == V_STR) ? args[0].str : ".";

        int    count = 0;
        char **names = os_dir(path, &count);

        Value list = make_nil(); list.type = V_LIST; list.nitems = count; list.cap = count;
        list.items = zi_alloc(sizeof(Value*) * (count > 0 ? count : 1));
        for (int i = 0; i < count; i++) {
            list.items[i] = zi_alloc(sizeof(Value));
            *list.items[i] = make_str(names[i]);
            free(names[i]);
        }
        free(names);
        return list;
    }

    if (strcmp(name, "rm") == 0) {
        if (nargs < 1 || args[0].type != V_STR) runtime_error("rm needs a filename");
        if (remove(args[0].str) != 0) runtime_error("rm: couldn't remove that path");
        return make_nil();
    }

    if (strcmp(name, "move") == 0) {
        if (nargs < 2 || args[0].type != V_STR || args[1].type != V_STR)
            runtime_error("move needs a source and destination path");
        if (rename(args[0].str, args[1].str) != 0) runtime_error("move: rename failed");
        return make_nil();
    }

    if (strcmp(name, "copy") == 0) {
        if (nargs < 2 || args[0].type != V_STR || args[1].type != V_STR)
            runtime_error("copy needs a source and destination path");
        FILE *in = fopen(args[0].str, "rb");
        if (!in) runtime_error("copy: can't open source file");
        FILE *outf = fopen(args[1].str, "wb");
        if (!outf) { fclose(in); runtime_error("copy: can't create destination file"); }
        char buf[65536]; size_t n;
        while ((n = fread(buf, 1, sizeof(buf), in)) > 0) fwrite(buf, 1, n, outf);
        fclose(in); fclose(outf);
        return make_nil();
    }

    /* start(path[, args...]) - launch a program, don't wait for it, return its pid */
    if (strcmp(name, "start") == 0) {
        if (nargs < 1 || args[0].type != V_STR) runtime_error("start needs a program path");
        pid_t child = fork();
        if (child < 0) runtime_error("start: fork failed");
        if (child == 0) {
            char *argv[16]; int ac = 0;
            argv[ac++] = args[0].str;
            for (int i = 1; i < nargs && ac < 15; i++) {
                if (args[i].type == V_STR) argv[ac++] = args[i].str;
            }
            argv[ac] = NULL;
            execvp(args[0].str, argv);
            _exit(127); /* execvp only returns on failure */
        }
        return make_num((double)child);
    }

    /* run(command) - run a shell command, wait for it, return its exit code */
    if (strcmp(name, "run") == 0) {
        if (nargs < 1 || args[0].type != V_STR) runtime_error("run needs a command string");
        int status = system(args[0].str);
        return make_num((double)(status == -1 ? -1 : WEXITSTATUS(status)));
    }

    /* kill(name_or_pid) - terminate a process (SIGTERM) by exact `procs()` name or numeric pid */
    if (strcmp(name, "kill") == 0) {
        if (nargs < 1) runtime_error("kill needs a process name or pid");
        long pid;
        if (args[0].type == V_NUM) {
            pid = (long)args[0].num;
        } else if (args[0].type == V_STR) {
            pid = find_pid_by_name(args[0].str);
            if (pid < 0) return make_bool(0);
        } else {
            runtime_error("kill needs a process name (string) or pid (number)");
            return make_nil();
        }
        return make_bool(kill((pid_t)pid, SIGTERM) == 0);
    }

    /* ---- W5 raw memory (docs/design/design_memory_structs.md §3.1) ----
     *
     * Sized, unsigned loads and stores. The NAMES are fixed by that doc: it
     * notes design_kernel.md §8.2's page allocator and design_game_system.md
     * §2.2 are already written against these exact spellings, so they are
     * acceptance tests, not suggestions.
     *
     * The bare `peek`/`poke` deliberately stay in SIMULATED[] - the doc is
     * explicit that repurposing them would make any existing program calling
     * poke(a, v) silently start writing memory. They are deprecated in favour
     * of these, and left exactly as they were.
     *
     * An address rides in a double, so it is exact only below 2^53. That is
     * the one documented hole (§3.2) and it is fine for every address a
     * kernel actually pokes. */
    if (strncmp(name, "peek", 4) == 0 && name[4] != '\0') {
        int w = atoi(name + 4);
        if (w == 8 || w == 16 || w == 32 || w == 64) {
            if (nargs < 1 || args[0].type != V_NUM) runtime_error("peek needs an address");
            unsigned long long p = (unsigned long long)args[0].num;
            unsigned long long v = 0;
            zi_check(p, (unsigned long long)(w / 8),
                     "peek outside the memory this program is allowed to touch");
            if      (w == 8)  v = *(unsigned char *)(uintptr_t)p;
            else if (w == 16) v = *(unsigned short *)(uintptr_t)p;
            else if (w == 32) v = *(unsigned int *)(uintptr_t)p;
            else              v = *(unsigned long long *)(uintptr_t)p;
            /* A zl number is a double, so a bit pattern above 2^53 cannot be
             * returned intact. design_kernel.md §2 names this exactly: a GDT
             * entry like 0x00AF9A000000FFFF would come back as a DIFFERENT
             * number and silently write the wrong descriptor. Refuse instead
             * - "backends may REJECT a program, never answer differently"
             * (MASTER_PLAN §8 risk 9). The documented fix is two halves. */
            if (v > 9007199254740992ULL)
                runtime_error("peek64: value above 2^53 cannot be represented exactly "
                              "(a zl number is a double) - read it as two peek32 halves");
            return make_num((double)v);
        }
    }
    if (strncmp(name, "poke", 4) == 0 && name[4] != '\0') {
        int w = atoi(name + 4);
        if (w == 8 || w == 16 || w == 32 || w == 64) {
            if (nargs < 2 || args[0].type != V_NUM || args[1].type != V_NUM)
                runtime_error("poke needs an address and a value");
            unsigned long long p = (unsigned long long)args[0].num;
            unsigned long long v = (unsigned long long)args[1].num;
            /* same 2^53 rule on the way in - by the time the value reaches
             * here a too-big literal has ALREADY been rounded, so the only
             * honest move is to refuse rather than write the rounded bytes */
            if (w == 64 && v > 9007199254740992ULL)
                runtime_error("poke64: value above 2^53 has already lost precision "
                              "(a zl number is a double) - write it as two poke32 halves");
            zi_check(p, (unsigned long long)(w / 8),
                     "poke outside the memory this program is allowed to touch");
            if      (w == 8)  *(unsigned char *)(uintptr_t)p  = (unsigned char)v;
            else if (w == 16) *(unsigned short *)(uintptr_t)p = (unsigned short)v;
            else if (w == 32) *(unsigned int *)(uintptr_t)p   = (unsigned int)v;
            else              *(unsigned long long *)(uintptr_t)p = v;
            return make_nil();
        }
    }

    /* alloc(n) - n ZEROED bytes, 16-byte aligned, address returned.
     * free(p) is accepted and ignored in v1, exactly as the doc specifies. */
    if (strcmp(name, "alloc") == 0) {
        if (nargs < 1 || args[0].type != V_NUM) runtime_error("alloc needs a byte count");
        size_t n = (size_t)args[0].num;
        void *p = NULL;
        /* Through the SAME seam as every other allocation, so it is charged to
         * the budget and, in the kernel, comes out of the arena - which is what
         * makes the address it returns land inside the window above. An alloc
         * that bypassed this would hand a confined program a legal pointer to
         * memory it is not allowed to touch. */
        p = zi_alloc((unsigned long)(n ? n : 16));
        if (!p) runtime_error("alloc failed");
        memset(p, 0, n ? n : 16);
        return make_num((double)(unsigned long long)(uintptr_t)p);
    }
    if (strcmp(name, "free") == 0) return make_nil();   /* ignored in v1 */

    if (strcmp(name, "copy_mem") == 0) {               /* memmove, overlap-safe */
        if (nargs < 3) runtime_error("copy_mem needs dst, src and a length");
        zi_charge((long long)((unsigned long long)args[2].num / ZI_BYTES_PER_STEP) + 1);
        zi_check((unsigned long long)args[0].num, (unsigned long long)args[2].num,
                 "copy_mem destination is outside this program's memory");
        zi_check((unsigned long long)args[1].num, (unsigned long long)args[2].num,
                 "copy_mem source is outside this program's memory");
        memmove((void *)(uintptr_t)(unsigned long long)args[0].num,
                (void *)(uintptr_t)(unsigned long long)args[1].num,
                (size_t)args[2].num);
        return make_nil();
    }
    if (strcmp(name, "fill_mem") == 0) {               /* memset */
        if (nargs < 3) runtime_error("fill_mem needs an address, a byte and a length");
        zi_charge((long long)((unsigned long long)args[2].num / ZI_BYTES_PER_STEP) + 1);
        zi_check((unsigned long long)args[0].num, (unsigned long long)args[2].num,
                 "fill_mem is outside the memory this program is allowed to touch");
        memset((void *)(uintptr_t)(unsigned long long)args[0].num,
               (int)args[1].num, (size_t)args[2].num);
        return make_nil();
    }
    /* sext(v, bits) - sign-extend the low `bits` of v. Loads are unsigned, so
     * this is how hand-written code reads a signed field (§3.2). */
    if (strcmp(name, "sext") == 0) {
        if (nargs < 2) runtime_error("sext needs a value and a bit width");
        int bits = (int)args[1].num;
        if (bits <= 0 || bits > 64) runtime_error("sext needs 1..64 bits");
        if (bits == 64) return args[0];
        unsigned long long v = (unsigned long long)args[0].num;
        unsigned long long m = 1ULL << (bits - 1);
        v &= (1ULL << bits) - 1;
        return make_num((double)(long long)((v ^ m) - m));
    }

    /* procs() -> a real LIST of the names of every running process */
    if (strcmp(name, "procs") == 0) {
        int    count = 0;
        char **names = os_procs(&count);

        Value list = make_nil(); list.type = V_LIST; list.nitems = count; list.cap = count;
        list.items = zi_alloc(sizeof(Value*) * (count > 0 ? count : 1));
        for (int i = 0; i < count; i++) {
            list.items[i] = zi_alloc(sizeof(Value));
            *list.items[i] = make_str(names[i]);
            free(names[i]);
        }
        free(names);
        return list;
    }

    /* the simulated dangerous ones: show what they WOULD do */
    if (is_simulated(name)) {
        printf("[sim] %s(", name);
        for (int i = 0; i < nargs; i++) {
            char *s = value_to_string(args[i]);
            if (i) printf(", ");
            printf("%s", s);
            free(s);
        }
        printf(")\n");
        return make_nil();
    }

    /* ---- extended builtins (math / string / list / type) ---- */
    if (strcmp(name, "nil") == 0)    { return make_nil(); }
    if (strcmp(name, "pi") == 0)     { return make_num(3.14159265358979323846); }
    if (strcmp(name, "e") == 0)      { return make_num(2.71828182845904523536); }
    if (strcmp(name, "assert") == 0) {
        if (nargs<1 || !is_truthy(args[0])) {
            if (nargs>=2 && args[1].type==V_STR) fprintf(stderr,"assertion failed: %s\n", args[1].str);
            else runtime_error("assertion failed");
            exit(1);
        }
        return make_nil();
    }
    if (strcmp(name, "seed") == 0)   { srand((unsigned)(long long)args[0].num); return make_nil(); }
    if (strcmp(name, "random") == 0) { return make_num((double)rand() / ((double)RAND_MAX + 1.0)); }
    if (strcmp(name, "randint") == 0){
        long long lo=(long long)args[0].num, hi=(long long)args[1].num;
        if (hi<lo){ long long t=lo; lo=hi; hi=t; }
        return make_num((double)(lo + rand() % (hi - lo + 1)));
    }
    if (strcmp(name, "sin") == 0)   { return make_num(sin(args[0].num)); }
    if (strcmp(name, "cos") == 0)   { return make_num(cos(args[0].num)); }
    if (strcmp(name, "tan") == 0)   { return make_num(tan(args[0].num)); }
    if (strcmp(name, "log") == 0)   { return make_num(log(args[0].num)); }
    if (strcmp(name, "exp") == 0)   { return make_num(exp(args[0].num)); }
    if (strcmp(name, "atan") == 0)  { return make_num(atan(args[0].num)); }
    if (strcmp(name, "sqrt") == 0)  { if (nargs<1) runtime_error("sqrt needs a number");  return make_num(sqrt(args[0].num)); }
    if (strcmp(name, "pow") == 0)   { if (nargs<2) runtime_error("pow needs two numbers"); return make_num(pow(args[0].num, args[1].num)); }
    if (strcmp(name, "floor") == 0) { if (nargs<1) runtime_error("floor needs a number"); return make_num(floor(args[0].num)); }
    if (strcmp(name, "ceil") == 0)  { if (nargs<1) runtime_error("ceil needs a number");  return make_num(ceil(args[0].num)); }
    if (strcmp(name, "round") == 0) { if (nargs<1) runtime_error("round needs a number");  return make_num(round(args[0].num)); }
    if (strcmp(name, "sign") == 0)  { double x=args[0].num; return make_num(x>0?1:(x<0?-1:0)); }
    if (strcmp(name, "gcd") == 0)   { long long a=(long long)args[0].num,b=(long long)args[1].num; if(a<0)a=-a; if(b<0)b=-b; while(b){long long t=a%b;a=b;b=t;} return make_num((double)a); }
    if (strcmp(name, "bool") == 0)  { return make_bool(is_truthy(args[0])); }
    if (strcmp(name, "type") == 0)  {
        const char *t="nil";
        switch (args[0].type){case V_NUM:t="num";break;case V_STR:t="str";break;case V_BOOL:t="bool";break;case V_LIST:t="list";break;case V_FN:t="fn";break;default:t="nil";}
        return make_str(t);
    }
    if (strcmp(name, "str") == 0)   { char *s=value_to_string(args[0]); Value v=make_str(s); free(s); return v; }
    /* chr(code) -> the one-character string for that byte.
     * The code is range-checked instead of truncated: chr(321) used to
     * BE chr(65) (321 & 255) and chr(256) an empty string, so distinct
     * codes silently collided and code(chr(n)) only round-tripped for
     * 1..255. Code 0 is refused too - a zl string is a NUL-terminated
     * C string, so a NUL cannot be held in one, and returning "" for it
     * loses the character with no warning. */
    if (strcmp(name, "chr") == 0)   {
        if (nargs<1||args[0].type!=V_NUM) runtime_error("chr needs a number");
        double d = args[0].num;
        if (!(d >= 1 && d <= 255)) {
            if (d == 0) runtime_error("chr(0): a zl string cannot hold a NUL byte");
            runtime_error("chr needs a code in 1..255");
        }
        char b[2]; b[0]=(char)(int)d; b[1]='\0'; return make_str(b);
    }
    /* sum(list) -> the total. Every element must be a number (or a bool,
     * which counts as 0/1). It used to add items[i]->num for ANY element:
     * Value is a struct, not a union, and .num is never written for a
     * string/list/nil, so sum(["a","b"]) returned uninitialised heap
     * bytes - a different garbage double on every run. */
    if (strcmp(name, "sum") == 0)   {
        if (args[0].type!=V_LIST) runtime_error("sum needs a list");
        double s=0;
        for(int i=0;i<args[0].nitems;i++) {
            ValueType t = args[0].items[i]->type;
            if (t != V_NUM && t != V_BOOL) runtime_error("sum needs a list of numbers");
            s += args[0].items[i]->num;
        }
        return make_num(s);
    }
    if (strcmp(name, "first") == 0) { if (args[0].type!=V_LIST||args[0].nitems==0) runtime_error("first needs a non-empty list"); return *args[0].items[0]; }
    if (strcmp(name, "last") == 0)  { if (args[0].type!=V_LIST||args[0].nitems==0) runtime_error("last needs a non-empty list"); return *args[0].items[args[0].nitems-1]; }
    if (strcmp(name, "starts") == 0){ if (args[0].type!=V_STR||args[1].type!=V_STR) runtime_error("starts needs two strings"); return make_bool(strncmp(args[0].str,args[1].str,strlen(args[1].str))==0); }
    /* range(hi) / range(lo, hi) -> [lo, hi). The count is worked out in
     * the double domain and refused if it will not fit in a list: the
     * old (int) narrowing turned range(2^31) into a list whose reported
     * length was INT32_MIN, a live value that str() rendered as empty
     * and len() reported as negative. */
    if (strcmp(name, "range") == 0) {
        if (nargs<1) runtime_error("range needs a count");
        double dlo=0, dhi;
        if (nargs>=2){ dlo=args[0].num; dhi=args[1].num; } else dhi=args[0].num;
        /* A NaN bound makes dhi > dlo false, so the range is empty -
         * which is what it already was, only now without going through
         * an undefined cast to get there. */
        double dcnt = dhi > dlo ? dhi - dlo : 0;
        if (dcnt > 100000000.0) runtime_error("range count is too large to build");
        int cnt = (int)dcnt;
        long long lo = cnt > 0 ? exact_i64(dlo, "range") : 0;
        Value v = make_nil(); v.type=V_LIST; v.nitems=cnt; v.cap=cnt; v.items=zi_alloc(sizeof(Value*)*(size_t)(cnt>0?cnt:1));
        if (!v.items) runtime_error("out of memory building a range");
        for (int i=0;i<cnt;i++){ v.items[i]=zi_alloc(sizeof(Value)); *v.items[i]=make_num((double)(lo+i)); }
        return v;
    }
    if (strcmp(name, "reverse") == 0) {
        if (args[0].type==V_STR){ size_t L=strlen(args[0].str); char*b=zi_alloc(L+1); for(size_t i=0;i<L;i++) b[i]=args[0].str[L-1-i]; b[L]='\0'; Value v = make_nil(); v.type=V_STR; v.str=b; return v; }
        if (args[0].type==V_LIST){ int m=args[0].nitems; Value v = make_nil(); v.type=V_LIST; v.nitems=m; v.cap=m; v.items=zi_alloc(sizeof(Value*)*(m>0?m:1)); for(int i=0;i<m;i++){v.items[i]=zi_alloc(sizeof(Value)); *v.items[i]=*args[0].items[m-1-i];} return v; }
        runtime_error("reverse needs a string or list");
    }
    if (strcmp(name, "repeat") == 0) {
        if (args[0].type!=V_STR||args[1].type!=V_NUM) runtime_error("repeat needs a string and a count");
        int n=clamp_index(args[1].num, 0, 100000000); size_t L=strlen(args[0].str);
        if (L > 0 && (size_t)n > 100000000u / L) runtime_error("repeat result is too large to build");
        char*b=zi_alloc(L*(size_t)n+1);
        if (!b) runtime_error("out of memory in repeat");
        for (int i=0;i<n;i++) memcpy(b+(size_t)i*L, args[0].str, L); b[L*(size_t)n]='\0';
        Value v = make_nil(); v.type=V_STR; v.str=b; return v;
    }
    if (strcmp(name, "trim") == 0) {
        if (args[0].type!=V_STR) runtime_error("trim needs a string");
        const char*s=args[0].str; while(*s==' '||*s=='\t'||*s=='\n'||*s=='\r')s++;
        const char*e=s+strlen(s); while(e>s&&(e[-1]==' '||e[-1]=='\t'||e[-1]=='\n'||e[-1]=='\r'))e--;
        size_t L=(size_t)(e-s); char*b=zi_alloc(L+1); memcpy(b,s,L); b[L]='\0'; Value v = make_nil(); v.type=V_STR; v.str=b; return v;
    }
    /* count(text, part) -> how many non-overlapping times part occurs.
     * The empty needle occurs at every position, so it occurs len+1
     * times. It used to hard-return 0, which contradicted find, has and
     * `in` - all three delegate to strstr, and strstr(hay, "") finds it
     * at index 0 - so has(s,x) == (count(s,x) > 0) was false for x == "".
     * (replace(s, "", new) stays a documented no-op: replace rewrites,
     * it is not one of the search predicates.) */
    if (strcmp(name, "count") == 0) {
        if (args[0].type!=V_STR||args[1].type!=V_STR) runtime_error("count needs two strings");
        const char*o=args[1].str; if(!*o) return make_num((double)strlen(args[0].str)+1);
        int c=0; const char*p=args[0].str; while((p=strstr(p,o))){c++; p+=strlen(o);} return make_num(c);
    }
    if (strcmp(name, "hex") == 0)   {
        if (nargs<1||args[0].type!=V_NUM) runtime_error("hex needs a number");
        char b[HEX_BUF]; hex_of_double(b, args[0].num); return make_str(b);
    }
    if (strcmp(name, "pad") == 0) {
        /* A negative width used to be cast to size_t, becoming SIZE_MAX,
         * so the fill loop walked off the end of the buffer and killed
         * the process. Clamp while it is still signed. */
        if (args[0].type!=V_STR||args[1].type!=V_NUM) runtime_error("pad needs a string and width");
        int w=clamp_index(args[1].num, 0, 100000000); size_t L=strlen(args[0].str); size_t o=((size_t)w>L)?(size_t)w:L;
        char*b=zi_alloc(o+1);
        if (!b) runtime_error("out of memory in pad");
        memcpy(b,args[0].str,L); for(size_t i=L;i<o;i++)b[i]=' '; b[o]='\0'; Value v = make_nil(); v.type=V_STR; v.str=b; return v;
    }
    if (strcmp(name, "replace") == 0) {
        if (args[0].type!=V_STR||args[1].type!=V_STR||args[2].type!=V_STR) runtime_error("replace needs three strings");
        const char*s=args[0].str,*o=args[1].str,*nw=args[2].str; size_t ol=strlen(o),nl=strlen(nw);
        if (ol==0) return make_str(s);
        int c=0; const char*p=s; while((p=strstr(p,o))){c++; p+=ol;}
        size_t out=strlen(s)-(size_t)c*ol+(size_t)c*nl; char*b=zi_alloc(out+1),*w=b; p=s; const char*q;
        while((q=strstr(p,o))){ memcpy(w,p,(size_t)(q-p)); w+=q-p; memcpy(w,nw,nl); w+=nl; p=q+ol; }
        strcpy(w,p); Value v = make_nil(); v.type=V_STR; v.str=b; return v;
    }
    if (strcmp(name, "insert") == 0) {
        if (args[0].type!=V_LIST||args[1].type!=V_NUM) runtime_error("insert needs a list, index, value");
        int m=args[0].nitems, idx=clamp_index(args[1].num, 0, m);
        Value v = make_nil(); v.type=V_LIST; v.nitems=m+1; v.cap=m+1; v.items=zi_alloc(sizeof(Value*)*(size_t)(m+1)); int k=0;
        for(int i=0;i<idx;i++){v.items[k]=zi_alloc(sizeof(Value)); *v.items[k]=*args[0].items[i]; k++;}
        v.items[k]=zi_alloc(sizeof(Value)); *v.items[k]=args[2]; k++;
        for(int i=idx;i<m;i++){v.items[k]=zi_alloc(sizeof(Value)); *v.items[k]=*args[0].items[i]; k++;}
        return v;
    }
    if (strcmp(name, "remove") == 0) {
        if (args[0].type!=V_LIST||args[1].type!=V_NUM) runtime_error("remove needs a list and index");
        int m=args[0].nitems;
        if (!(args[1].num >= 0 && args[1].num < (double)m)) runtime_error("remove index out of range");
        int idx=(int)args[1].num;
        Value v = make_nil(); v.type=V_LIST; v.nitems=m-1; v.cap=(m-1>0)?m-1:1; v.items=zi_alloc(sizeof(Value*)*(size_t)(m>1?m-1:1)); int k=0;
        for(int i=0;i<m;i++){ if(i==idx)continue; v.items[k]=zi_alloc(sizeof(Value)); *v.items[k]=*args[0].items[i]; k++; }
        return v;
    }

    /* ---- bitwise: zl has no bitwise OPERATORS, so these are the way.
     * See THE INTEGER-WIDTH POLICY at the top of this file. In short:
     * every argument goes in through exact_i64 and every answer comes
     * back out through bit_result, so a value the double cannot hold
     * exactly is an error at BOTH ends instead of a silent lie at
     * either. A shift count outside 0..63 gives 0. ---- */
    if (strcmp(name, "band") == 0) {
        if (nargs<2||args[0].type!=V_NUM||args[1].type!=V_NUM) runtime_error("band needs two numbers");
        return bit_result(exact_i64(args[0].num,"band") & exact_i64(args[1].num,"band"), "band");
    }
    if (strcmp(name, "bor") == 0) {
        if (nargs<2||args[0].type!=V_NUM||args[1].type!=V_NUM) runtime_error("bor needs two numbers");
        return bit_result(exact_i64(args[0].num,"bor") | exact_i64(args[1].num,"bor"), "bor");
    }
    if (strcmp(name, "bxor") == 0) {
        if (nargs<2||args[0].type!=V_NUM||args[1].type!=V_NUM) runtime_error("bxor needs two numbers");
        return bit_result(exact_i64(args[0].num,"bxor") ^ exact_i64(args[1].num,"bxor"), "bxor");
    }
    /* bnot must be an INVOLUTION. It stopped being one twice over: once
     * for any x needing 54+ bits (the rounded ~x was not ~x), and once
     * at INT64_MIN, where ~x is INT64_MAX and the nearest double to
     * that is 2^63 - one PAST the top of the range, so feeding it back
     * in hit the undefined cast. bit_result rejects both, which makes
     * bnot(bnot(x)) == x hold for every x it accepts at all. */
    if (strcmp(name, "bnot") == 0) {
        if (nargs<1||args[0].type!=V_NUM) runtime_error("bnot needs a number");
        return bit_result(~exact_i64(args[0].num,"bnot"), "bnot");
    }
    /* The shift COUNT is not an operand, it is a selector, and a count
     * outside 0..63 is documented to give 0 for ANY magnitude - so it is
     * truncated and then range-tested as a double rather than going
     * through exact_i64, which would turn shl(1, 1e20) into an error. */
    if (strcmp(name, "shl") == 0) {
        if (nargs<2||args[0].type!=V_NUM||args[1].type!=V_NUM) runtime_error("shl needs two numbers");
        double dn = trunc(args[1].num);
        if (!(dn >= 0 && dn <= 63)) return make_num(0);
        long long a=exact_i64(args[0].num,"shl"), n=(long long)dn;
        return bit_result((long long)((unsigned long long)a << n), "shl");
    }
    if (strcmp(name, "shr") == 0) {   /* arithmetic (sign-filling) shift */
        if (nargs<2||args[0].type!=V_NUM||args[1].type!=V_NUM) runtime_error("shr needs two numbers");
        double dn = trunc(args[1].num);
        if (!(dn >= 0 && dn <= 63)) return make_num(0);
        long long a=exact_i64(args[0].num,"shr"), n=(long long)dn;
        if (a<0) return bit_result(-1 - (long long)((unsigned long long)(-1 - a) >> n), "shr");
        return bit_result((long long)((unsigned long long)a >> n), "shr");
    }

    /* ---- more math ---- */
    if (strcmp(name, "asin") == 0)  { if (nargs<1||args[0].type!=V_NUM) runtime_error("asin needs a number");  return make_num(asin(args[0].num)); }
    if (strcmp(name, "acos") == 0)  { if (nargs<1||args[0].type!=V_NUM) runtime_error("acos needs a number");  return make_num(acos(args[0].num)); }
    if (strcmp(name, "log2") == 0)  { if (nargs<1||args[0].type!=V_NUM) runtime_error("log2 needs a number");  return make_num(log2(args[0].num)); }
    if (strcmp(name, "log10") == 0) { if (nargs<1||args[0].type!=V_NUM) runtime_error("log10 needs a number"); return make_num(log10(args[0].num)); }
    if (strcmp(name, "trunc") == 0) { if (nargs<1||args[0].type!=V_NUM) runtime_error("trunc needs a number"); return make_num(trunc(args[0].num)); }
    /* C's hypot() scales its operands, so it does not overflow or
     * underflow on the way to a result that IS representable. The
     * hand-rolled sqrt(a*a+b*b) did: hypot(1e200, 0) was inf, and
     * hypot(3e-200, 4e-200) was 0. */
    if (strcmp(name, "hypot") == 0) {
        if (nargs<2||args[0].type!=V_NUM||args[1].type!=V_NUM) runtime_error("hypot needs two numbers");
        return make_num(hypot(args[0].num, args[1].num));
    }
    if (strcmp(name, "fmod") == 0) {  /* b == 0 gives nan, like / gives inf */
        if (nargs<2||args[0].type!=V_NUM||args[1].type!=V_NUM) runtime_error("fmod needs two numbers");
        return make_num(fmod(args[0].num, args[1].num));
    }
    if (strcmp(name, "clamp") == 0) {
        if (nargs<3||args[0].type!=V_NUM||args[1].type!=V_NUM||args[2].type!=V_NUM) runtime_error("clamp needs three numbers");
        double x=args[0].num, lo=args[1].num, hi=args[2].num;
        if (x<lo) return make_num(lo);
        if (x>hi) return make_num(hi);
        return make_num(x);
    }

    /* ---- more strings ---- */
    if (strcmp(name, "ltrim") == 0) {
        if (nargs<1||args[0].type!=V_STR) runtime_error("ltrim needs a string");
        const char*s=args[0].str; while(*s==' '||*s=='\t'||*s=='\n'||*s=='\r')s++;
        return make_str(s);
    }
    if (strcmp(name, "rtrim") == 0) {
        if (nargs<1||args[0].type!=V_STR) runtime_error("rtrim needs a string");
        const char*s=args[0].str; const char*e=s+strlen(s);
        while(e>s&&(e[-1]==' '||e[-1]=='\t'||e[-1]=='\n'||e[-1]=='\r'))e--;
        size_t L=(size_t)(e-s); char*b=zi_alloc(L+1); memcpy(b,s,L); b[L]='\0';
        Value v = make_nil(); v.type=V_STR; v.str=b; return v;
    }
    if (strcmp(name, "title") == 0) {  /* upper-case the first letter of each word */
        if (nargs<1||args[0].type!=V_STR) runtime_error("title needs a string");
        size_t L=strlen(args[0].str); char*b=zi_alloc(L+1); int at_start=1;
        for (size_t i=0;i<L;i++) {
            char ch=args[0].str[i];
            int alnum=(ch>='a'&&ch<='z')||(ch>='A'&&ch<='Z')||(ch>='0'&&ch<='9');
            if (at_start && ch>='a' && ch<='z') ch=(char)(ch-32);
            b[i]=ch; at_start = !alnum;
        }
        b[L]='\0'; Value v = make_nil(); v.type=V_STR; v.str=b; return v;
    }
    if (strcmp(name, "swapcase") == 0) {
        if (nargs<1||args[0].type!=V_STR) runtime_error("swapcase needs a string");
        size_t L=strlen(args[0].str); char*b=zi_alloc(L+1);
        for (size_t i=0;i<L;i++) {
            char ch=args[0].str[i];
            if (ch>='a'&&ch<='z') ch=(char)(ch-32);
            else if (ch>='A'&&ch<='Z') ch=(char)(ch+32);
            b[i]=ch;
        }
        b[L]='\0'; Value v = make_nil(); v.type=V_STR; v.str=b; return v;
    }
    if (strcmp(name, "index_at") == 0) {  /* character code at position i, -1 off the end */
        if (nargs<2||args[0].type!=V_STR||args[1].type!=V_NUM) runtime_error("index_at needs a string and a number");
        size_t L=strlen(args[0].str);
        if (!(args[1].num >= 0 && args[1].num < (double)L)) return make_num(-1);
        return make_num((double)(unsigned char)args[0].str[(int)args[1].num]);
    }

    /* ---- more lists ---- */
    if (strcmp(name, "concat") == 0) {
        if (nargs<2||args[0].type!=V_LIST||args[1].type!=V_LIST) runtime_error("concat needs two lists");
        int m=args[0].nitems+args[1].nitems;
        Value v = make_nil(); v.type=V_LIST; v.nitems=m; v.cap=m; v.items=zi_alloc(sizeof(Value*)*(m>0?m:1)); int k=0;
        for(int i=0;i<args[0].nitems;i++){v.items[k]=zi_alloc(sizeof(Value)); *v.items[k]=*args[0].items[i]; k++;}
        for(int i=0;i<args[1].nitems;i++){v.items[k]=zi_alloc(sizeof(Value)); *v.items[k]=*args[1].items[i]; k++;}
        return v;
    }
    if (strcmp(name, "fill") == 0) {
        if (nargs<2||args[0].type!=V_NUM) runtime_error("fill needs a count and a value");
        if (args[0].num > 100000000.0) runtime_error("fill count is too large to build");
        int m=clamp_index(args[0].num, 0, 100000000);
        Value v = make_nil(); v.type=V_LIST; v.nitems=m; v.cap=m; v.items=zi_alloc(sizeof(Value*)*(size_t)(m>0?m:1));
        if (!v.items) runtime_error("out of memory in fill");
        for(int i=0;i<m;i++){v.items[i]=zi_alloc(sizeof(Value)); *v.items[i]=args[1];}
        return v;
    }
    if (strcmp(name, "flat") == 0) {   /* flattens ONE level */
        if (nargs<1||args[0].type!=V_LIST) runtime_error("flat needs a list");
        int m=0;
        for(int i=0;i<args[0].nitems;i++) m += (args[0].items[i]->type==V_LIST) ? args[0].items[i]->nitems : 1;
        Value v = make_nil(); v.type=V_LIST; v.nitems=m; v.cap=m; v.items=zi_alloc(sizeof(Value*)*(m>0?m:1)); int k=0;
        for(int i=0;i<args[0].nitems;i++){
            Value *it=args[0].items[i];
            if (it->type==V_LIST) { for(int j=0;j<it->nitems;j++){v.items[k]=zi_alloc(sizeof(Value)); *v.items[k]=*it->items[j]; k++;} }
            else { v.items[k]=zi_alloc(sizeof(Value)); *v.items[k]=*it; k++; }
        }
        return v;
    }
    /* take/drop: n clamped to 0..len. The clamp runs in the DOUBLE
     * domain - narrowing first meant any n >= 2^31 landed on INT32_MIN,
     * which the old `if (n<0) n=0` then read as ZERO, so take(xs, 2^31)
     * returned nothing where take(xs, 2^31 - 1) returned everything. */
    if (strcmp(name, "take") == 0) {
        if (nargs<2||args[0].type!=V_LIST||args[1].type!=V_NUM) runtime_error("take needs a list and a number");
        int m=args[0].nitems, n=clamp_index(args[1].num, 0, m);
        Value v = make_nil(); v.type=V_LIST; v.nitems=n; v.cap=n; v.items=zi_alloc(sizeof(Value*)*(n>0?n:1));
        for(int i=0;i<n;i++){v.items[i]=zi_alloc(sizeof(Value)); *v.items[i]=*args[0].items[i];}
        return v;
    }
    if (strcmp(name, "drop") == 0) {
        if (nargs<2||args[0].type!=V_LIST||args[1].type!=V_NUM) runtime_error("drop needs a list and a number");
        int m=args[0].nitems, n=clamp_index(args[1].num, 0, m); int c=m-n;
        Value v = make_nil(); v.type=V_LIST; v.nitems=c; v.cap=c; v.items=zi_alloc(sizeof(Value*)*(c>0?c:1));
        for(int i=0;i<c;i++){v.items[i]=zi_alloc(sizeof(Value)); *v.items[i]=*args[0].items[n+i];}
        return v;
    }

    /* ---- system ---- */
    if (strcmp(name, "now") == 0) {    /* milliseconds since this process started */
        return make_num((double)clock() * 1000.0 / (double)CLOCKS_PER_SEC);
    }
    if (strcmp(name, "exit") == 0) {
        exit((nargs>=1 && args[0].type==V_NUM) ? (int)args[0].num : 0);
    }
    if (strcmp(name, "env") == 0) {
        if (nargs<1||args[0].type!=V_STR) runtime_error("env needs a string");
        const char *val = getenv(args[0].str);
        return make_str(val ? val : "");
    }

    runtime_error("unknown function");
    return make_nil();
}

/* =============================================================
 * EVALUATING EXPRESSIONS
 * ============================================================= */

/* the '+' operator: numbers add, lists concatenate, else join as text */
static Value eval_plus(Value l, Value r)
{
    if (l.type == V_NUM && r.type == V_NUM)
        return make_num(l.num + r.num);

    if (l.type == V_LIST && r.type == V_LIST) {
        Value v = make_nil(); v.type = V_LIST; v.nitems = l.nitems + r.nitems; v.cap = v.nitems;
        v.items = zi_alloc(sizeof(Value*) * (v.nitems > 0 ? v.nitems : 1));
        int k = 0;
        for (int i = 0; i < l.nitems; i++) { v.items[k] = zi_alloc(sizeof(Value)); *v.items[k] = *l.items[i]; k++; }
        for (int i = 0; i < r.nitems; i++) { v.items[k] = zi_alloc(sizeof(Value)); *v.items[k] = *r.items[i]; k++; }
        return v;
    }

    char *ls = value_to_string(l);
    char *rs = value_to_string(r);
    char *out = zi_alloc(strlen(ls) + strlen(rs) + 1);
    strcpy(out, ls); strcat(out, rs);
    Value v = make_nil(); v.type = V_STR; v.str = out;
    free(ls); free(rs);
    return v;
}

/* == and != , and the search built-ins (contains / index_of / in).
 *
 * Lists compare STRUCTURALLY: same length, then element by element.
 * Without the V_LIST arm a list was the only zl value that was not even
 * equal to ITSELF - `a == a` was false - which also made contains() and
 * index_of() permanently blind to a list element.
 *
 * A structural walk has to survive a list that CONTAINS ITSELF, which zl
 * can build in two lines:  xs = [1, 2]  then  xs[0] = xs  - index-assign
 * stores a Value whose .items is the very array being written into. Two
 * guards, in this order:
 *
 *   1. IDENTITY. Two Values that share an .items array ARE the same
 *      list, so they are equal without looking inside. This is what
 *      makes `xs == xs` answer true for a self-referencing list instead
 *      of descending forever, and it is a fast path for every ordinary
 *      list too.
 *   2. A DEPTH CAP. Identity does not save two DIFFERENT cyclic lists
 *      (xs[0]=xs, ys[0]=ys, xs == ys), which have no finite structural
 *      answer here. Deciding that needs a bisimulation over visited
 *      PAIRS, not a deeper stack, so we refuse out loud rather than
 *      pretend. Without the cap the process was killed outright with
 *      STATUS_STACK_OVERFLOW - no message, and every buffered line of
 *      output lost. The cap is far above any real nesting (a list 256
 *      deep is not a data structure anyone typed) and small enough that
 *      the frames fit the 1 MB default stack a COMPILED zl program
 *      gets, not just interp.exe's 64 MB.
 *
 * MAX_VALUE_DEPTH is shared with value_compare below, which recurses
 * over exactly the same shape and can be reached by the same cycle
 * through sort(). */
#define MAX_VALUE_DEPTH 256

static int values_equal_depth(Value l, Value r, int depth)
{
    if (l.type != r.type) return 0;
    switch (l.type) {
        case V_NUM:
        case V_BOOL: return l.num == r.num;
        case V_STR:  return strcmp(l.str, r.str) == 0;
        case V_NIL:  return 1;
        case V_LIST:
            if (l.nitems != r.nitems) return 0;
            if (l.items == r.items) return 1;   /* the same list */
            if (depth >= MAX_VALUE_DEPTH)
                runtime_error("== nested too deep to compare "
                              "(does a list contain itself?)");
            for (int i = 0; i < l.nitems; i++)
                if (!values_equal_depth(*l.items[i], *r.items[i], depth + 1)) return 0;
            return 1;
        default:     return 0;                  /* V_FN: identity-free */
    }
}

static int values_equal(Value l, Value r) { return values_equal_depth(l, r, 0); }

/* sort()'s ordering - a TOTAL order over every value.
 *
 * zl's < is numbers-only, so sort cannot borrow it. It used to compare
 * the raw .num field of whatever it was given: Value is a struct, not a
 * union, so .num is never written for a string, a list or nil, and sort
 * was ordering them by uninitialised heap bytes - the same multiset in a
 * different input order came back differently. A NaN was just as bad,
 * because every comparison against one is false, so the insertion sort
 * stopped dead at it and left the numbers around it unsorted.
 *
 *   nil  <  bool  <  num  <  str  <  list  <  fn        (by type first)
 *   numbers ascending, NaN after every real number
 *   strings by byte value
 *   lists element-wise, then shorter first
 *
 * This is sort's order and only sort's order: the < operator still
 * refuses anything but numbers. */
static int type_rank(ValueType t)
{
    switch (t) {
        case V_NIL:  return 0;
        case V_BOOL: return 1;
        case V_NUM:  return 2;
        case V_STR:  return 3;
        case V_LIST: return 4;
        case V_FN:   return 5;
    }
    return 6;
}

static int value_compare_depth(const Value *a, const Value *b, int depth)
{
    int ra = type_rank(a->type), rb = type_rank(b->type);
    if (ra != rb) return ra < rb ? -1 : 1;

    switch (a->type) {
        case V_BOOL:
        case V_NUM: {
            int na = a->num != a->num, nb = b->num != b->num;   /* NaN */
            if (na || nb) return na == nb ? 0 : (na ? 1 : -1);
            if (a->num < b->num) return -1;
            if (a->num > b->num) return  1;
            return 0;
        }
        case V_STR:  return strcmp(a->str, b->str) < 0 ? -1 : (strcmp(a->str, b->str) > 0 ? 1 : 0);
        case V_LIST: {
            /* same two cycle guards as values_equal - sort() reaches the
             * same self-referencing list, and sort([xs, xs]) used to die
             * with STATUS_STACK_OVERFLOW. */
            if (a->items == b->items && a->nitems == b->nitems) return 0;
            if (depth >= MAX_VALUE_DEPTH)
                runtime_error("sort: a list is nested too deep to order "
                              "(does a list contain itself?)");
            int n = a->nitems < b->nitems ? a->nitems : b->nitems;
            for (int i = 0; i < n; i++) {
                int c = value_compare_depth(a->items[i], b->items[i], depth + 1);
                if (c) return c;
            }
            if (a->nitems != b->nitems) return a->nitems < b->nitems ? -1 : 1;
            return 0;
        }
        default: return 0;                       /* nil, fn: all equal */
    }
}

static int value_compare(const Value *a, const Value *b) { return value_compare_depth(a, b, 0); }

static Value eval_binary(const char *op, Value l, Value r)
{
    if (strcmp(op, "+") == 0) return eval_plus(l, r);

    if (strcmp(op, "==") == 0) return make_bool(values_equal(l, r));
    if (strcmp(op, "!=") == 0) return make_bool(!values_equal(l, r));

    if (strcmp(op, "and") == 0) return make_bool(is_truthy(l) && is_truthy(r));
    if (strcmp(op, "or") == 0)  return make_bool(is_truthy(l) || is_truthy(r));

    /* 'x in xs' IS contains(xs, x), and 'sub in text' IS has(text, sub).
     * No new search logic: the operator just re-orders the operands
     * (the container is on the right) and calls the built-in. Anything
     * that isn't a list goes to has(), which reports the type error. */
    if (strcmp(op, "in") == 0) {
        Value swapped[2]; swapped[0] = r; swapped[1] = l;
        return call_builtin(r.type == V_LIST ? "contains" : "has", swapped, 2);
    }

    /* the rest are numeric */
    if (l.type != V_NUM || r.type != V_NUM)
        runtime_error("this operator needs numbers");

    double a = l.num, b = r.num;
    if (strcmp(op, "-") == 0)  return make_num(a - b);
    if (strcmp(op, "*") == 0)  return make_num(a * b);
    if (strcmp(op, "/") == 0)  return make_num(a / b);
    /* % is an INTEGER modulo, so it must never hand the CPU a divisor it
     * cannot use: the two cases below are hardware TRAPS, not wrong
     * answers. Either one killed the process outright (no message, and
     * every buffered line of output lost with it).
     *   b == 0   -> 0xC0000094 STATUS_INTEGER_DIVIDE_BY_ZERO.  Answered
     *               with nan, which is what the siblings already do:
     *               fmod(a, 0) is nan and a / 0 is inf.
     *   b == -1  -> 0xC0000095 STATUS_INTEGER_OVERFLOW, for the single
     *               value LLONG_MIN % -1.  Every a % -1 is 0
     *               mathematically, so answer 0 without dividing.
     * Mirrored verbatim in runtime.c - see the parity note there. */
    if (strcmp(op, "%") == 0) {
        long long bi = (long long)b;
        if (bi == 0)  return make_num(fmod(a, 0.0));
        if (bi == -1) return make_num(0.0);
        return make_num((double)((long long)a % bi));
    }
    if (strcmp(op, ">")  == 0) return make_bool(a >  b);
    if (strcmp(op, "<")  == 0) return make_bool(a <  b);
    if (strcmp(op, ">=") == 0) return make_bool(a >= b);
    if (strcmp(op, "<=") == 0) return make_bool(a <= b);

    runtime_error("unknown operator");
    return make_nil();
}

/* evaluate a function call node */
/* A for-loop variable BINDS, the way a parameter does - the loop
 * introduces the name, the programmer did not assign it - so it must not
 * write a global that happens to share it. `for i in [7,8] {}` inside a
 * function used to leave a top-level `i` holding 8.
 *
 * They are bound at CALL time rather than when the loop is reached, so
 * the name is local for the whole body and not just from the loop
 * onwards. That is what the C backend does - compile.c declares the loop
 * variable alongside the function's other locals - and the two engines
 * have to agree. (A top-level loop variable is still a global in both,
 * which is the same thing seen from the outermost scope.) */
static void define_loop_vars(Node *n, Env *env)
{
    if (!n) return;
    if (n->type == N_FOR && !env_find_local(env, n->text))
        env_define(env, n->text, make_nil());
    for (int i = 0; i < n->nkids; i++) define_loop_vars(n->kids[i], env);
    define_loop_vars(n->a, env);
    define_loop_vars(n->b, env);
    define_loop_vars(n->c, env);
}

static Value eval_call(Node *n, Env *env)
{
    /* evaluate the arguments first */
    int nargs = n->nkids;
    Value *args = zi_alloc(sizeof(Value) * (nargs > 0 ? nargs : 1));
    for (int i = 0; i < nargs; i++) args[i] = eval(n->kids[i], env);

    /* case 1: callee is a plain name */
    if (n->a->type == N_IDENT) {
        const char *name = n->a->text;
        Var *slot = env_find(env, name);

        /* a user-defined function shadows nothing here - check it first */
        if (slot && slot->val.type == V_FN) {
            Node *fn = slot->val.fn;

            /* parent is the GLOBAL scope, not the caller - real
             * function-local scoping (see g_global note above). */
            Env *call_env = env_new(g_global);
            for (int i = 0; i < fn->nkids && i < nargs; i++)
                env_define(call_env, fn->kids[i]->text, args[i]);
            define_loop_vars(fn->a, call_env);

            if (++g_depth > MAX_CALL_DEPTH) {
                g_depth = 0;
                runtime_error("recursion too deep");
            }
            g_returning = 0;
            g_return_value = make_nil();
            exec(fn->a, call_env);                /* run the body block */
            g_depth--;

            Value result = g_returning ? g_return_value : make_nil();
            g_returning = 0;
            free(args);
            return result;
        }

        /* otherwise it's a built-in (print, kill, ...) */
        Value r = call_builtin(name, args, nargs);
        free(args);
        return r;
    }

    runtime_error("that is not something you can call");
    free(args);
    return make_nil();
}

/* ---- the two places every step of a program passes through ---------------
 * eval() is every expression, exec() is every statement, and a zl program
 * cannot do ANYTHING without going through one of them - a loop body, a
 * function call, an operand, all of it. So the budget needs exactly two check
 * sites, not a sprinkling of them, and there is no path around it.
 *
 * The wrapper pair exists so that depth is decremented on the way out along
 * EVERY return path. eval_inner has a dozen returns in a switch; putting a
 * decrement before each was the alternative and it is one edit away from a
 * permanent leak that only shows up as a spurious "too deep" ten thousand
 * calls later.
 *
 * The unwind is the exception, and it is deliberate: runtime_error longjmps
 * straight past these frames, so zi_depth is left high. It does not matter and
 * must not be "fixed" with a cleanup - the trap catcher is the only code that
 * runs afterwards and zi_limit() resets the counters before anything runs
 * again. A cleanup here would be code that only ever executes while unwinding,
 * i.e. code no test can reach. */
/* Debit the budget by more than one step. Same kill as zi_step's, so a program
 * cannot escape by doing its work in one enormous allocation instead of many
 * small statements. Saturating rather than wrapping: a request big enough to
 * overflow the counter must exhaust the budget, never lap it. */
static void zi_charge(long long units)
{
    if (units < 1) units = 1;
    zi_steps_used += units;
    if (!zi_steps_left) return;
    if (units >= zi_steps_left) {
        zi_steps_left = 0;
        zi_killed = 1;
        runtime_error("step budget exhausted - the program was stopped");
    }
    zi_steps_left -= units;
}

static void zi_step(void)
{
    if (zi_steps_left) {
        if (--zi_steps_left <= 0) {
            zi_killed = 1;
            runtime_error("step budget exhausted - the program was stopped");
        }
    }
    zi_steps_used++;
    if (zi_depth > zi_depth_peak) zi_depth_peak = zi_depth;
    if (zi_depth_max && zi_depth >= zi_depth_max) {
        zi_killed = 1;
        runtime_error("too deeply nested - the program was stopped");
    }
}

static Value eval(Node *n, Env *env)
{
    Value v;
    zi_step();
    zi_depth++;
    v = eval_inner(n, env);
    zi_depth--;
    return v;
}

static void exec(Node *n, Env *env)
{
    zi_step();
    zi_depth++;
    exec_inner(n, env);
    zi_depth--;
}

static Value eval_inner(Node *n, Env *env)
{
    switch (n->type) {
        case N_NUMBER: return make_num(atof(n->text));
        case N_STRING: return make_str(n->text);
        case N_BOOL:   return make_bool(strcmp(n->text, "true") == 0);

        case N_IDENT: {
            Var *v = env_find(env, n->text);
            if (!v) {
                char buf[MAX_TEXT + 32];
                snprintf(buf, sizeof(buf), "'%s' doesn't exist yet", n->text);
                runtime_error(buf);
            }
            return v->val;
        }

        case N_LIST: {
            Value v = make_nil(); v.type = V_LIST; v.nitems = n->nkids; v.cap = n->nkids;
            v.items = zi_alloc(sizeof(Value*) * (n->nkids > 0 ? n->nkids : 1));
            for (int i = 0; i < n->nkids; i++) {
                v.items[i] = zi_alloc(sizeof(Value));
                *v.items[i] = eval(n->kids[i], env);
            }
            return v;
        }

        case N_BINARY: {
            /* `and` and `or` SHORT-CIRCUIT: the right operand is only
             * evaluated when the left has not already decided the answer.
             * That has to happen here, before n->b is touched - down in
             * eval_binary both operands have already been evaluated, so
             * the right side's side effects (and its errors) had already
             * happened, and the standard guard
             *     if len(xs) > 0 and xs[0] == 1 { ... }
             * aborted on an empty list instead of being false. The
             * ternary below has always short-circuited; now these match. */
            if (strcmp(n->text, "and") == 0) {
                Value l = eval(n->a, env);
                if (!is_truthy(l)) return make_bool(0);
                return make_bool(is_truthy(eval(n->b, env)));
            }
            if (strcmp(n->text, "or") == 0) {
                Value l = eval(n->a, env);
                if (is_truthy(l)) return make_bool(1);
                return make_bool(is_truthy(eval(n->b, env)));
            }
            Value l = eval(n->a, env);
            Value r = eval(n->b, env);
            return eval_binary(n->text, l, r);
        }

        case N_UNARY: {
            Value x = eval(n->a, env);
            if (strcmp(n->text, "-") == 0) {
                if (x.type != V_NUM) runtime_error("cannot negate a non-number");
                return make_num(-x.num);
            }
            if (strcmp(n->text, "not") == 0) return make_bool(!is_truthy(x));
            runtime_error("unknown unary operator");
        }

        case N_TERNARY:
            /* Only the taken branch is evaluated. That is the whole
             * point of a ternary: the untaken side may be something
             * you must NOT run (a divide by zero, a side effect). */
            return is_truthy(eval(n->a, env)) ? eval(n->b, env)
                                              : eval(n->c, env);

        case N_DANGER:
            /* the '!' marker has no runtime effect yet - it's a
             * signal of intent. Just evaluate what it wraps. */
            return eval(n->a, env);

        case N_CALL: return eval_call(n, env);

        case N_INDEX: {
            Value obj = eval(n->a, env);
            Value idx = eval(n->b, env);
            if (obj.type != V_LIST) runtime_error("only lists can be indexed");
            if (idx.type != V_NUM)  runtime_error("list index must be a number");
            int i = (int)idx.num;
            if (i < 0 || i >= obj.nitems) runtime_error("list index out of range");
            return *obj.items[i];
        }

        case N_MEMBER:
            runtime_error("member access (.) isn't supported yet");

        default:
            runtime_error("cannot evaluate this");
    }
    return make_nil();
}

/* =============================================================
 * EXECUTING STATEMENTS
 * ============================================================= */

static void exec_block(Node *block, Env *env)
{
    for (int i = 0; i < block->nkids; i++) {
        exec(block->kids[i], env);
        /* return / break / continue all stop the rest of the block */
        if (g_returning || g_breaking || g_continuing) return;
    }
}

static void exec_inner(Node *n, Env *env)
{
    switch (n->type) {
        case N_EXPRSTMT:
            eval(n->a, env);
            break;

        /* n->text is empty for a plain `=` and holds the operator for a
         * compound `op=`. The compound form reads the target ONCE: for
         * an index target the container and the subscript are evaluated
         * a single time and reused for both the read and the write. */
        case N_ASSIGN:
            if (n->a->type == N_IDENT) {
                if (n->text[0]) {
                    Value cur = eval(n->a, env);          /* reports "doesn't exist yet" */
                    Value val = eval(n->b, env);
                    env_assign(env, n->a->text, eval_binary(n->text, cur, val));
                } else {
                    env_assign(env, n->a->text, eval(n->b, env));
                }
            } else if (n->a->type == N_INDEX) {
                /* x[i] = v : mutate the list element in place */
                Value list = eval(n->a->a, env);
                Value idx  = eval(n->a->b, env);
                if (list.type != V_LIST) runtime_error("can only index-assign a list");
                if (idx.type != V_NUM)   runtime_error("list index must be a number");
                if (!(idx.num >= 0 && idx.num < (double)list.nitems))
                    runtime_error("index-assign out of range");
                int i = (int)idx.num;
                Value val;
                if (n->text[0]) {
                    Value cur = *list.items[i];           /* read before the
                                                             right side runs */
                    val = eval_binary(n->text, cur, eval(n->b, env));
                } else {
                    val = eval(n->b, env);
                }
                *list.items[i] = val;
            } else {
                runtime_error("can only assign to a name or a list index");
            }
            break;

        case N_IF: {
            Value cond = eval(n->a, env);
            if (is_truthy(cond))       exec_block(n->b, env);
            else if (n->c != NULL)     exec_block(n->c, env);
            break;
        }

        case N_BREAK:    g_breaking = 1;    break;
        case N_CONTINUE: g_continuing = 1;  break;

        case N_FOR: {
            Value seq = eval(n->a, env);
            if (seq.type != V_LIST)
                runtime_error("'for' can only loop over a list");
            for (int i = 0; i < seq.nitems; i++) {
                env_assign(env, n->text, *seq.items[i]);
                exec_block(n->b, env);
                if (g_returning) return;
                if (g_breaking)   { g_breaking = 0;   break; }
                if (g_continuing) { g_continuing = 0; }
            }
            break;
        }

        case N_WHILE:
            while (is_truthy(eval(n->a, env))) {
                exec_block(n->b, env);
                if (g_returning) return;
                if (g_breaking)   { g_breaking = 0;   break; }
                if (g_continuing) { g_continuing = 0; }
            }
            break;

        case N_FN: {
            /* defining a function stores it as a variable */
            Value v = make_nil(); v.type = V_FN; v.fn = n;
            env_assign(env, n->text, v);
            break;
        }

        case N_RETURN:
            g_return_value = (n->a != NULL) ? eval(n->a, env) : make_nil();
            g_returning = 1;
            break;

        case N_BLOCK:
            exec_block(n, env);
            break;

        default:
            /* a bare expression statement fell through: evaluate it */
            eval(n, env);
            break;
    }
}

/* =============================================================
 * MAIN
 * ============================================================= */

/* ---- running a program, with a way out -----------------------------------
 * The entry point the kernel needs and the one main() now uses, so the path a
 * gate exercises is the path that ships rather than a parallel one written for
 * testing.
 *
 * Everything that makes this safe is here rather than at the call sites:
 *
 *   - THE CONTROL-FLOW FLAGS ARE RESET FIRST. g_returning / g_breaking /
 *     g_continuing are globals, and a program killed mid-loop leaves whichever
 *     one it was carrying set. The next program then returns from its first
 *     statement, for no visible reason, and the bug looks like it is in the
 *     second program. Nothing resets these today because nothing has ever run
 *     two programs in one process - the kernel will run one per `run`.
 *
 *   - THE TRAP IS ARMED AFTER setjmp AND DISARMED ON EVERY EXIT. Armed before,
 *     and a longjmp taken during setup would jump into a frame that is not yet
 *     valid. Left armed on the way out, and the NEXT runtime_error - possibly
 *     from a completely different subsystem - would unwind into a dead frame.
 *
 *   - NOTHING LOCAL IS READ AFTER THE UNWIND. Locals modified between setjmp
 *     and longjmp are indeterminate afterwards unless volatile, so the return
 *     path reads only the zi_* globals. That is not pedantry: it is UB that
 *     works at -O0 and breaks at -O2, which is the optimisation level the
 *     kernel builds at.
 */

/* ---- the nesting guard, and why it runs BEFORE the parser ----------------
 * FOUND BY AN ADVERSARIAL READER. A file containing nothing but open brackets:
 *
 *     python3 -c "open('x.zl','w').write('['*8000)"
 *     ./interp --steps 1 --depth 1 x.zl      ->  SIGSEGV
 *
 * `--steps 1 --depth 1` and it still crashes, because NONE of the kill path is
 * involved. parse() runs before zl_run_program() arms the trap or sets a
 * limit, and the parser is uncapped recursive descent: every `[` re-enters
 * parse_expr through a dozen frames, and the overflow happens on the DESCENT,
 * so no closing bracket is ever needed. Measured on this host's 8 MiB stack:
 * 6000 brackets survives, 6500 crashes. THE KERNEL HAS 256 KiB, about 32x
 * less - so roughly 200 brackets, a source file under a quarter of a kilobyte,
 * and no memory protection to contain where it lands.
 *
 * Two ways to fix it and only one of them is small. Capping recursion inside
 * parser.c means finding every recursive path through eleven precedence levels
 * and threading a counter through all of them - in a file this track does not
 * own. But parser recursion is bounded by BRACKET NESTING, and bracket nesting
 * is visible in the token stream before a single frame is pushed. One linear
 * scan, no parser change, and it cannot be wrong about a construct it has not
 * been taught, because it counts the only thing that makes the parser recurse.
 *
 * The limit is deliberately generous. Real zl nests a handful deep; 64 is far
 * past anything anyone writes and far below what either stack can take.
 */

#define ZI_MAX_NESTING 64

static int zi_nesting_ok(const Token *tokens, int count, int *deepest, int *line)
{
    int depth = 0, worst = 0, worst_line = 0;
    for (int i = 0; i < count; i++) {
        if (tokens[i].type != T_SYMBOL) continue;
        char c = tokens[i].text[0];
        if (tokens[i].text[1] != '\0') continue;      /* ==, >=, ... are not brackets */
        if (c == '(' || c == '[' || c == '{') {
            depth++;
            if (depth > worst) { worst = depth; worst_line = tokens[i].line; }
        } else if (c == ')' || c == ']' || c == '}') {
            if (depth > 0) depth--;
        }
    }
    if (deepest) *deepest = worst;
    if (line) *line = worst_line;
    return worst <= ZI_MAX_NESTING;
}

#define ZI_OK      0
#define ZI_ERROR   1     /* the program did something illegal - trap caught it */
#define ZI_KILLED  2     /* the budget or the depth cap stopped it             */

/* Parse, but refuse a program whose nesting would overflow the stack getting
 * there. Returns NULL and reports; the caller must not parse it otherwise. */
Node *zl_parse_guarded(Token *tokens, int count)
{
    int deepest = 0, line = 0;
    if (!zi_nesting_ok(tokens, count, &deepest, &line)) {
        zi_killed = 1;
        zi_seterr("nested too deeply to parse - the program was refused");
        fflush(stdout);
        fprintf(stderr, "refused: nesting %d deep at line %d, limit %d "
                        "(it would overflow the stack before it ran)\n",
                deepest, line, ZI_MAX_NESTING);
        return NULL;
    }
    return parse(tokens, count);
}

int zl_run_program(Node *program, long long steps, int max_depth)
{
    zi_limit(steps, max_depth);

    g_returning = 0;
    g_breaking  = 0;
    g_continuing = 0;

    Env *global = env_new(NULL);
    g_global = global;                 /* functions scope off this */

    if (zi_setjmp(zi_trap) != 0)
        return zi_killed ? ZI_KILLED : ZI_ERROR;

    zi_trap_armed = 1;
    for (int i = 0; i < program->nkids; i++) {
        exec(program->kids[i], global);
        if (g_returning) break;
    }
    zi_trap_armed = 0;
    return ZI_OK;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: interp [--steps N] [--depth N] [--confine LO HI] <file>\n");
        return 1;
    }

    /* Unlimited unless asked otherwise, which is what this interpreter has
     * always been. The flags exist so the kill path can be driven from a gate
     * without a kernel - see kernel/hosttest/killtest.sh. */
    long long steps = 0;
    int depth = 0, argi = 1;
    unsigned long long confine_lo = 0, confine_hi = 0;
    while (argi < argc - 1) {
        if (strcmp(argv[argi], "--steps") == 0 && argi + 1 < argc) {
            steps = atoll(argv[++argi]); argi++;
        } else if (strcmp(argv[argi], "--depth") == 0 && argi + 1 < argc) {
            depth = atoi(argv[++argi]); argi++;
        } else if (strcmp(argv[argi], "--confine") == 0 && argi + 2 < argc) {
            /* --confine LO HI: the only addresses raw memory may touch. This
             * is what the kernel sets to the arena's bounds. */
            confine_lo = strtoull(argv[++argi], NULL, 0);
            confine_hi = strtoull(argv[++argi], NULL, 0); argi++;
        } else break;
    }

    int    count;
    Token *tokens  = lex_file(argv[argi], &count);
    Node  *program = zl_parse_guarded(tokens, count);
    if (!program) return 2;              /* refused - stopped, not a crash */

    if (confine_hi) zi_confine(confine_lo, confine_hi);
    int r = zl_run_program(program, steps, depth);
    if (r != ZI_OK) {
        fflush(stdout);
        fprintf(stderr, "%s: %s\n",
                r == ZI_KILLED ? "stopped" : "runtime error", zi_error());
        fprintf(stderr, "  steps used %lld, peak depth %d\n",
                zi_used(), zi_peak_depth());
        return r == ZI_KILLED ? 2 : 1;
    }
    return 0;
}
