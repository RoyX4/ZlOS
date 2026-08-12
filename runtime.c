/* runtime.c - implementation of the compiled-program runtime.
 *
 * This is the interpreter's value logic, lifted out so that
 * COMPILED programs can call it too. The interpreter (interp.c) and
 * the compiler's output (out.c) now share the same idea of what a
 * value is and how operators behave - which is exactly what you want,
 * so `2 + 3 * 4` means the same thing interpreted or compiled.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/wait.h>

#include "runtime.h"
#include "os.h"

/* The zlx_ bridge (bottom of this file) lets compiled LLVM code call the
 * boxed builtins by passing POINTERS to Value slots it stack-allocates as
 * raw bytes. compilel.c sizes those slots at 48 bytes (its VALSZ), so a
 * Value must actually be 48 bytes or args[i] indexing in zlx_call desyncs.
 * This is the portable (C89) static assert - MSVC's default C mode does not
 * enable the _Static_assert keyword, so a size mismatch makes this typedef
 * an array of negative length and the compile fails loudly. */
typedef char zl_value_is_48_bytes[(sizeof(Value) == 48) ? 1 : -1];

static void rt_error(const char *msg)
{
    fflush(stdout);            /* stdout is block-buffered when redirected;
                                  without this the output that explains HOW
                                  we got here dies with the process. */
    fprintf(stderr, "runtime error: %s\n", msg);
    exit(1);
}

/* =============================================================
 * NARROWING A NUMBER TO A C INTEGER
 *
 * Kept identical to interp.c's copy - the interpreter and a compiled
 * program have to agree about what a builtin does. See the long comment
 * there: converting a double to a C integer type is undefined in C
 * whenever the value does not fit or is NaN, and on x86-64 it quietly
 * yields INT32_MIN / INT64_MIN.
 * ============================================================= */

/* a list/string count or index, clamped into [lo, hi] as a double */
static int clamp_index(double d, int lo, int hi)
{
    if (d != d)          return lo;             /* NaN */
    if (d <= (double)lo) return lo;
    if (d >= (double)hi) return hi;
    return (int)d;
}

/* THE INTEGER-WIDTH POLICY. The full statement of it lives in interp.c;
 * this is the byte-for-byte other half, and the two MUST stay in step -
 * if only one is fixed, the interpreter and a compiled program still
 * agree with each other and verify.ps1 sees nothing, because both
 * engines are simply wrong the same way. In short: a bitwise builtin
 * refuses any operand (exact_i64) or any result (bit_result) that a
 * double cannot hold exactly, rather than rounding it or casting it out
 * of range; int() and hex() lose nothing, so they never refuse. */

/* an exact 64-bit integer operand (the bitwise builtins) */
static long long exact_i64(double d, const char *who)
{
    if (d != d || d < -9223372036854775808.0 || d >= 9223372036854775808.0) {
        char buf[96];
        snprintf(buf, sizeof(buf),
                 "%s needs 64-bit integers (got NaN or a value out of range)", who);
        rt_error(buf);
    }
    return (long long)d;
}

/* the RESULT of a bitwise builtin, on its way back out through a double.
 * A double carries 53 significant bits, so an int64 answer needing more
 * than that used to be rounded and handed back as if it were the answer
 * - bor(2^53,1) came back as 2^53, so the bit just set was gone. The
 * test round-trips the value and so fires only when precision is really
 * lost; shl(1,62) and shl(1,63) are exact and pass. */
static Value bit_result(long long r, const char *who)
{
    double d = (double)r;
    /* d can round UP to exactly 2^63, which is outside long long, so the
     * range has to be tested before casting back - that cast would be
     * undefined otherwise. (It cannot round below -2^63: that is exact.) */
    if (d >= 9223372036854775808.0 || (long long)d != r) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "%s result needs more than 53 bits of precision - "
                 "a zl number is a double and cannot hold it exactly", who);
        rt_error(buf);
    }
    return zl_num(d);
}

/* hex(x): x truncated toward zero, in hexadecimal. Values that fit in a
 * signed 64-bit integer keep the two's-complement view; past that the
 * exact digits are printed, so hex(2^63), hex(2^64) and hex(2^65) no
 * longer collide on "8000000000000000". Identical to interp.c's copy. */
#define HEX_BUF 288

static void hex_of_double(char *out, double v)
{
    if (v != v || v == HUGE_VAL || v == -HUGE_VAL)
        rt_error("hex needs a finite number");
    v = trunc(v);

    if (v >= -9223372036854775808.0 && v < 9223372036854775808.0) {
        snprintf(out, HEX_BUF, "%llx", (unsigned long long)(long long)v);
        return;
    }

    int    neg = v < 0;
    double a   = neg ? -v : v;
    int    e2;
    double f    = frexp(a, &e2);
    unsigned long long mant = (unsigned long long)ldexp(f, 53);
    int    exp  = e2 - 53;
    while (mant != 0 && (mant & 1) == 0) { mant >>= 1; exp++; }

    char *p = out;
    if (neg) *p++ = '-';
    p += snprintf(p, 24, "%llx", mant << (exp & 3));
    for (int i = 0, z = exp >> 2; i < z; i++) *p++ = '0';
    *p = '\0';
}

/* ---- making values ---- */

Value zl_nil(void)      { Value v; memset(&v, 0, sizeof(v)); v.type = V_NIL;  return v; }
Value zl_num(double n)  { Value v = zl_nil(); v.type = V_NUM;  v.num = n;      return v; }
Value zl_bool(int b)    { Value v = zl_nil(); v.type = V_BOOL; v.num = b?1:0;  return v; }

Value zl_str(const char *s)
{
    Value v = zl_nil();
    v.type = V_STR;
    v.str  = malloc(strlen(s) + 1);
    strcpy(v.str, s);
    return v;
}

Value zl_list_n(int count, ...)
{
    Value v = zl_nil();
    v.type   = V_LIST;
    v.nitems = count;
    v.items  = malloc(sizeof(Value*) * (count > 0 ? count : 1));

    va_list ap;
    va_start(ap, count);
    for (int i = 0; i < count; i++) {
        v.items[i]  = malloc(sizeof(Value));
        *v.items[i] = va_arg(ap, Value);
    }
    va_end(ap);
    return v;
}

/* ---- using values ---- */

int zl_truthy(Value v)
{
    switch (v.type) {
        case V_NIL:  return 0;
        case V_BOOL: return v.num != 0;
        case V_NUM:  return v.num != 0;
        case V_STR:  return v.str && v.str[0] != '\0';
        case V_LIST: return v.nitems > 0;
    }
    return 0;
}

static char *to_string(Value v)
{
    char buf[64];
    switch (v.type) {
        case V_NIL:  return _strdup("nil");
        case V_BOOL: return _strdup(v.num ? "true" : "false");
        case V_STR:  return _strdup(v.str ? v.str : "");
        case V_NUM:
            if (v.num == (long long)v.num)
                snprintf(buf, sizeof(buf), "%lld", (long long)v.num);
            else
                snprintf(buf, sizeof(buf), "%g", v.num);
            return _strdup(buf);
        case V_LIST: {
            size_t cap = 3;
            char  *out = malloc(cap);
            strcpy(out, "[");
            for (int i = 0; i < v.nitems; i++) {
                char *part = to_string(*v.items[i]);
                cap += strlen(part) + 2;
                out = realloc(out, cap);
                if (i) strcat(out, ", ");
                strcat(out, part);
                free(part);
            }
            strcat(out, "]");
            return out;
        }
    }
    return _strdup("?");
}

/* == and != , and the search built-ins (contains / index_of / in).
 * Lists compare STRUCTURALLY - without the V_LIST arm a list was the
 * only zl value not equal to itself. Identical to interp.c's copy.
 *
 * The two cycle guards are identical too - see the long comment there.
 * A zl list can contain itself (xs = [1,2]; xs[0] = xs), so: two Values
 * sharing an .items array are the same list and equal without a walk,
 * and the walk is depth-capped so two DIFFERENT cyclic lists report a
 * runtime error instead of killing the process with a silent stack
 * overflow. The cap is sized for the 1 MB default stack a compiled zl
 * program gets. */
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
                rt_error("== nested too deep to compare "
                         "(does a list contain itself?)");
            for (int i = 0; i < l.nitems; i++)
                if (!values_equal_depth(*l.items[i], *r.items[i], depth + 1)) return 0;
            return 1;
        default:     return 0;
    }
}

static int values_equal(Value l, Value r) { return values_equal_depth(l, r, 0); }

/* sort()'s total order. See the long comment on interp.c's copy:
 *   nil < bool < num < str < list        (by type first)
 *   numbers ascending, NaN after every real number
 *   strings by byte value; lists element-wise, then shorter first
 * This is sort's order only - < still refuses anything but numbers. */
static int type_rank(ValueType t)
{
    switch (t) {
        case V_NIL:  return 0;
        case V_BOOL: return 1;
        case V_NUM:  return 2;
        case V_STR:  return 3;
        case V_LIST: return 4;
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
            /* same two cycle guards as values_equal - sort([xs, xs]) on a
             * self-referencing xs used to die with a stack overflow. */
            if (a->items == b->items && a->nitems == b->nitems) return 0;
            if (depth >= MAX_VALUE_DEPTH)
                rt_error("sort: a list is nested too deep to order "
                         "(does a list contain itself?)");
            int n = a->nitems < b->nitems ? a->nitems : b->nitems;
            for (int i = 0; i < n; i++) {
                int c = value_compare_depth(a->items[i], b->items[i], depth + 1);
                if (c) return c;
            }
            if (a->nitems != b->nitems) return a->nitems < b->nitems ? -1 : 1;
            return 0;
        }
        default: return 0;
    }
}

static int value_compare(const Value *a, const Value *b) { return value_compare_depth(a, b, 0); }

static Value binop_plus(Value l, Value r)
{
    if (l.type == V_NUM && r.type == V_NUM)
        return zl_num(l.num + r.num);

    if (l.type == V_LIST && r.type == V_LIST) {
        Value v = zl_nil(); v.type = V_LIST; v.nitems = l.nitems + r.nitems;
        v.items = malloc(sizeof(Value*) * (v.nitems > 0 ? v.nitems : 1));
        int k = 0;
        for (int i = 0; i < l.nitems; i++) { v.items[k] = malloc(sizeof(Value)); *v.items[k] = *l.items[i]; k++; }
        for (int i = 0; i < r.nitems; i++) { v.items[k] = malloc(sizeof(Value)); *v.items[k] = *r.items[i]; k++; }
        return v;
    }

    char *ls = to_string(l);
    char *rs = to_string(r);
    char *out = malloc(strlen(ls) + strlen(rs) + 1);
    strcpy(out, ls); strcat(out, rs);
    Value v = zl_nil(); v.type = V_STR; v.str = out;
    free(ls); free(rs);
    return v;
}

Value zl_binop(const char *op, Value l, Value r)
{
    if (strcmp(op, "+") == 0)  return binop_plus(l, r);
    if (strcmp(op, "==") == 0) return zl_bool(values_equal(l, r));
    if (strcmp(op, "!=") == 0) return zl_bool(!values_equal(l, r));
    if (strcmp(op, "and") == 0) return zl_bool(zl_truthy(l) && zl_truthy(r));
    if (strcmp(op, "or") == 0)  return zl_bool(zl_truthy(l) || zl_truthy(r));

    if (l.type != V_NUM || r.type != V_NUM) rt_error("this operator needs numbers");
    double a = l.num, b = r.num;
    if (strcmp(op, "-") == 0)  return zl_num(a - b);
    if (strcmp(op, "*") == 0)  return zl_num(a * b);
    if (strcmp(op, "/") == 0)  return zl_num(a / b);
    /* % traps on two divisors, identical to interp.c's copy - see the
     * long comment there. b == 0 is 0xC0000094 and answers nan (like
     * fmod), b == -1 is 0xC0000095 on LLONG_MIN and answers 0. */
    if (strcmp(op, "%") == 0) {
        long long bi = (long long)b;
        if (bi == 0)  return zl_num(fmod(a, 0.0));
        if (bi == -1) return zl_num(0.0);
        return zl_num((double)((long long)a % bi));
    }
    if (strcmp(op, ">")  == 0) return zl_bool(a >  b);
    if (strcmp(op, "<")  == 0) return zl_bool(a <  b);
    if (strcmp(op, ">=") == 0) return zl_bool(a >= b);
    if (strcmp(op, "<=") == 0) return zl_bool(a <= b);
    rt_error("unknown operator");
    return zl_nil();
}

Value zl_unop(const char *op, Value a)
{
    if (strcmp(op, "-") == 0) {
        if (a.type != V_NUM) rt_error("cannot negate a non-number");
        return zl_num(-a.num);
    }
    if (strcmp(op, "not") == 0) return zl_bool(!zl_truthy(a));
    rt_error("unknown unary operator");
    return zl_nil();
}

int   zl_len_list(Value v) { return v.type == V_LIST ? v.nitems : 0; }
Value zl_item(Value v, int i) { return *v.items[i]; }

/* x[i] = v : mutate a list element in place (items array is shared). */
void zl_set(Value list, Value idx, Value val)
{
    if (list.type != V_LIST) rt_error("can only index-assign a list");
    int i = (int)idx.num;
    if (i < 0 || i >= list.nitems) rt_error("index-assign out of range");
    *list.items[i] = val;
}

Value zl_index(Value seq, Value idx)
{
    if (seq.type != V_LIST) rt_error("only lists can be indexed");
    if (idx.type != V_NUM)  rt_error("list index must be a number");
    int i = (int)idx.num;
    if (i < 0 || i >= seq.nitems) rt_error("list index out of range");
    return *seq.items[i];
}

/* ---- built-in functions ---- */

static void list_push_str(Value *list, int *cap, const char *p, int len)
{
    if (list->nitems == *cap) {
        *cap *= 2;
        list->items = realloc(list->items, sizeof(Value*) * (size_t)(*cap));
    }
    char *buf = malloc((size_t)len + 1);
    memcpy(buf, p, (size_t)len);
    buf[len] = '\0';
    list->items[list->nitems] = malloc(sizeof(Value));
    Value v = zl_nil(); v.type = V_STR; v.str = buf;
    *list->items[list->nitems] = v;
    list->nitems++;
}

/* poke/peek/window stay simulated: poke/peek would mean patching another
 * live process's memory (ptrace injection - cheat-engine territory, not
 * needed for a working language), and window is GUI-toolkit specific with
 * no Linux equivalent implemented here. kill/start/rm/copy/move/run got a
 * REAL Linux implementation below (ordinary process/file syscalls - the
 * same thing any shell script or Python's os module can already do). */
static const char *SIMULATED[] = {
    "poke", "peek", "window", NULL
};

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
static int is_simulated(const char *name)
{
    for (int i = 0; SIMULATED[i]; i++)
        if (strcmp(name, SIMULATED[i]) == 0) return 1;
    return 0;
}

static Value builtin(const char *name, Value *args, int nargs)
{
    if (strcmp(name, "print") == 0) {
        for (int i = 0; i < nargs; i++) {
            char *s = to_string(args[i]);
            if (i) printf(" ");
            printf("%s", s);
            free(s);
        }
        printf("\n");
        return zl_nil();
    }

    if (strcmp(name, "input") == 0) {
        if (nargs > 0) { char *p = to_string(args[0]); printf("%s", p); free(p); }
        char line[512];
        if (fgets(line, sizeof(line), stdin)) {
            line[strcspn(line, "\r\n")] = '\0';
            return zl_str(line);
        }
        return zl_str("");
    }

    if (strcmp(name, "len") == 0) {
        if (nargs < 1) rt_error("len needs an argument");
        if (args[0].type == V_STR)  return zl_num((double)strlen(args[0].str));
        if (args[0].type == V_LIST) return zl_num((double)args[0].nitems);
        rt_error("len needs a string or a list");
    }

    if (strcmp(name, "lines") == 0) {
        if (nargs < 1 || args[0].type != V_STR) rt_error("lines needs a string");
        Value list = zl_nil(); list.type = V_LIST; list.nitems = 0;
        int cap = 8;
        list.items = malloc(sizeof(Value*) * (size_t)cap);
        const char *s = args[0].str, *start = s;
        while (*s) {
            if (*s == '\n') {
                int L = (int)(s - start);
                if (L > 0 && start[L-1] == '\r') L--;
                list_push_str(&list, &cap, start, L);
                start = s + 1;
            }
            s++;
        }
        if (s > start) {
            int L = (int)(s - start);
            if (L > 0 && start[L-1] == '\r') L--;
            list_push_str(&list, &cap, start, L);
        }
        return list;
    }

    if (strcmp(name, "ends") == 0) {
        if (nargs < 2 || args[0].type != V_STR || args[1].type != V_STR)
            rt_error("ends needs two strings");
        size_t tl = strlen(args[0].str), sl = strlen(args[1].str);
        return zl_bool(tl >= sl && strcmp(args[0].str + tl - sl, args[1].str) == 0);
    }

    if (strcmp(name, "has") == 0) {
        if (nargs < 2 || args[0].type != V_STR || args[1].type != V_STR)
            rt_error("has needs two strings");
        return zl_bool(strstr(args[0].str, args[1].str) != NULL);
    }

    if (strcmp(name, "at") == 0) {
        if (nargs < 2 || args[0].type != V_STR || args[1].type != V_NUM)
            rt_error("at needs a string and a number");
        size_t L = strlen(args[0].str);
        /* range-test the DOUBLE - the cast is only defined once we know
         * it fits. NaN fails the test, so it reads as out of range. */
        if (!(args[1].num >= 0 && args[1].num < (double)L)) return zl_str("");
        char b[2]; b[0] = args[0].str[(int)args[1].num]; b[1] = '\0';
        return zl_str(b);
    }

    /* push(list, item) -> a NEW list, by TIP TRACKING.
     * Kept identical to interp.c's copy - see the long comment there.
     * Each items array carries a shared counter, .tip: how many of its
     * slots have ever been handed out. Append IN PLACE only when the
     * source is the newest tip (nitems == *tip) and there is spare
     * capacity; a push branched off the same base sees nitems != *tip
     * and copies, so the two results never share slot m.
     *
     * Appending unconditionally (the original) was a use-after-free;
     * copying every time (the fix for it) was O(n) per push and so
     * O(n^2) to build a list. This is the version that is both. */
    if (strcmp(name, "push") == 0) {
        if (nargs < 2 || args[0].type != V_LIST)
            rt_error("push needs a list and an item");
        Value src = args[0];
        int m = src.nitems;

        if (src.items && src.tip && *src.tip == m && m < src.cap) {
            src.items[m] = malloc(sizeof(Value));
            *src.items[m] = args[1];
            *src.tip = m + 1;
            src.nitems = m + 1;      /* same array, same tip, same cap */
            return src;
        }

        int want = m + 1;
        int newcap = (want > 1073741823) ? want : want * 2;
        if (newcap < 8) newcap = 8;
        Value v = zl_nil(); v.type = V_LIST; v.nitems = want; v.cap = newcap;
        v.items = malloc(sizeof(Value*) * (size_t)newcap);
        if (m > 0) memcpy(v.items, src.items, sizeof(Value*) * (size_t)m);
        v.items[m] = malloc(sizeof(Value));
        *v.items[m] = args[1];
        v.tip = malloc(sizeof(int)); *v.tip = want;
        return v;
    }

    if (strcmp(name, "num") == 0) {
        if (nargs < 1) rt_error("num needs an argument");
        if (args[0].type == V_NUM) return args[0];
        if (args[0].type == V_STR) return zl_num(atof(args[0].str));
        rt_error("num needs a string or number");
    }

    /* trunc() IS "truncated toward zero" and stays defined outside
     * 64-bit range; the long long round-trip made int(+inf) NEGATIVE. */
    if (strcmp(name, "int") == 0) {
        if (nargs < 1) rt_error("int needs an argument");
        if (args[0].type == V_NUM) return zl_num(trunc(args[0].num));
        if (args[0].type == V_STR) return zl_num(trunc(atof(args[0].str)));
        rt_error("int needs a number or string");
    }

    if (strcmp(name, "abs") == 0) {
        if (nargs < 1 || args[0].type != V_NUM) rt_error("abs needs a number");
        double v = args[0].num; return zl_num(v < 0 ? -v : v);
    }
    /* IEEE-754 minNum/maxNum: a NaN on either side is ignored, so the
     * answer does not depend on which operand it was. */
    if (strcmp(name, "min") == 0) {
        if (nargs < 2 || args[0].type != V_NUM || args[1].type != V_NUM) rt_error("min needs two numbers");
        if (args[0].num != args[0].num) return args[1];
        if (args[1].num != args[1].num) return args[0];
        return args[0].num <= args[1].num ? args[0] : args[1];
    }
    if (strcmp(name, "max") == 0) {
        if (nargs < 2 || args[0].type != V_NUM || args[1].type != V_NUM) rt_error("max needs two numbers");
        if (args[0].num != args[0].num) return args[1];
        if (args[1].num != args[1].num) return args[0];
        return args[0].num >= args[1].num ? args[0] : args[1];
    }
    if (strcmp(name, "contains") == 0 || strcmp(name, "index_of") == 0) {
        if (nargs < 2 || args[0].type != V_LIST) rt_error("contains/index_of need a list and a value");
        int want_index = (strcmp(name, "index_of") == 0);
        for (int i = 0; i < args[0].nitems; i++)
            if (values_equal(*args[0].items[i], args[1]))
                return want_index ? zl_num(i) : zl_bool(1);
        return want_index ? zl_num(-1) : zl_bool(0);
    }
    if (strcmp(name, "sort") == 0) {
        if (nargs < 1 || args[0].type != V_LIST) rt_error("sort needs a list");
        int m = args[0].nitems;
        Value v = zl_nil(); v.type = V_LIST; v.nitems = m;
        v.items = malloc(sizeof(Value*) * (m > 0 ? m : 1));
        for (int i = 0; i < m; i++) { v.items[i] = malloc(sizeof(Value)); *v.items[i] = *args[0].items[i]; }
        for (int i = 1; i < m; i++) {
            Value *key = v.items[i]; int j = i - 1;
            while (j >= 0 && value_compare(v.items[j], key) > 0) { v.items[j+1] = v.items[j]; j--; }
            v.items[j+1] = key;
        }
        return v;
    }

    if (strcmp(name, "slice") == 0) {
        if (nargs < 3 || args[0].type != V_STR || args[1].type != V_NUM || args[2].type != V_NUM)
            rt_error("slice needs a string and two numbers");
        /* Both ends clamp into 0..len, BEFORE the narrowing to int - an
         * end index past INT_MAX used to land on INT_MIN, where the
         * clamp could no longer rescue it and the result came back "". */
        int L = (int)strlen(args[0].str);
        int a = clamp_index(args[1].num, 0, L), b = clamp_index(args[2].num, 0, L);
        if (a > b) a = b;
        int m = b - a;
        char *buf = malloc((size_t)m + 1);
        memcpy(buf, args[0].str + a, (size_t)m);
        buf[m] = '\0';
        Value v = zl_nil(); v.type = V_STR; v.str = buf; return v;
    }

    if (strcmp(name, "find") == 0) {
        if (nargs < 2 || args[0].type != V_STR || args[1].type != V_STR)
            rt_error("find needs two strings");
        char *p = strstr(args[0].str, args[1].str);
        if (!p) return zl_num(-1);
        return zl_num((double)(p - args[0].str));
    }

    if (strcmp(name, "upper") == 0 || strcmp(name, "lower") == 0) {
        if (nargs < 1 || args[0].type != V_STR) rt_error("upper/lower needs a string");
        int up = (strcmp(name, "upper") == 0);
        size_t L = strlen(args[0].str);
        char *buf = malloc(L + 1);
        for (size_t i = 0; i < L; i++) {
            char ch = args[0].str[i];
            if (up  && ch >= 'a' && ch <= 'z') ch = (char)(ch - 32);
            if (!up && ch >= 'A' && ch <= 'Z') ch = (char)(ch + 32);
            buf[i] = ch;
        }
        buf[L] = '\0';
        Value v = zl_nil(); v.type = V_STR; v.str = buf; return v;
    }

    if (strcmp(name, "join") == 0) {
        if (nargs < 2 || args[0].type != V_LIST || args[1].type != V_STR)
            rt_error("join needs a list and a separator string");
        size_t cap = 1;
        char *out = malloc(cap); out[0] = '\0';
        for (int i = 0; i < args[0].nitems; i++) {
            char *part = to_string(*args[0].items[i]);
            cap += strlen(part) + strlen(args[1].str);
            out = realloc(out, cap);
            if (i) strcat(out, args[1].str);
            strcat(out, part);
            free(part);
        }
        Value v = zl_nil(); v.type = V_STR; v.str = out; return v;
    }

    if (strcmp(name, "split") == 0) {
        if (nargs < 2 || args[0].type != V_STR || args[1].type != V_STR)
            rt_error("split needs two strings");
        Value list = zl_nil(); list.type = V_LIST; list.nitems = 0;
        int cap = 8;
        list.items = malloc(sizeof(Value*) * (size_t)cap);
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

    if (strcmp(name, "read") == 0) {
        if (nargs < 1 || args[0].type != V_STR) rt_error("read needs a filename");
        FILE *f = fopen(args[0].str, "rb");
        if (!f) rt_error("read: can't open that file");
        fseek(f, 0, SEEK_END); long sz = ftell(f); rewind(f);
        char *buf = malloc((size_t)sz + 1);
        size_t got = fread(buf, 1, (size_t)sz, f);
        buf[got] = '\0'; fclose(f);
        Value v = zl_nil(); v.type = V_STR; v.str = buf;
        return v;
    }

    if (strcmp(name, "write") == 0) {
        if (nargs < 2 || args[0].type != V_STR) rt_error("write needs a filename and text");
        FILE *f = fopen(args[0].str, "wb");
        if (!f) rt_error("write: can't create that file");
        char *text = to_string(args[1]);
        fputs(text, f); free(text); fclose(f);
        return zl_nil();
    }

    if (strcmp(name, "code") == 0) {
        if (nargs < 1 || args[0].type != V_STR) rt_error("code needs a string");
        return zl_num((double)(unsigned char)args[0].str[0]);
    }

    if (strcmp(name, "write_bytes") == 0) {
        if (nargs < 2 || args[0].type != V_STR || args[1].type != V_LIST)
            rt_error("write_bytes needs a filename and a list of byte values");
        FILE *f = fopen(args[0].str, "wb");
        if (!f) rt_error("write_bytes: can't create that file");
        for (int i = 0; i < args[1].nitems; i++)
            fputc((unsigned char)(long long)args[1].items[i]->num, f);
        fclose(f);
        return zl_nil();
    }

    if (strcmp(name, "dir") == 0) {
        const char *path = (nargs > 0 && args[0].type == V_STR) ? args[0].str : ".";
        int count = 0;
        char **names = os_dir(path, &count);
        Value list = zl_nil(); list.type = V_LIST; list.nitems = count;
        list.items = malloc(sizeof(Value*) * (count > 0 ? count : 1));
        for (int i = 0; i < count; i++) {
            list.items[i] = malloc(sizeof(Value));
            *list.items[i] = zl_str(names[i]);
            free(names[i]);
        }
        free(names);
        return list;
    }

    if (strcmp(name, "rm") == 0) {
        if (nargs < 1 || args[0].type != V_STR) rt_error("rm needs a filename");
        if (remove(args[0].str) != 0) rt_error("rm: couldn't remove that path");
        return zl_nil();
    }

    if (strcmp(name, "move") == 0) {
        if (nargs < 2 || args[0].type != V_STR || args[1].type != V_STR)
            rt_error("move needs a source and destination path");
        if (rename(args[0].str, args[1].str) != 0) rt_error("move: rename failed");
        return zl_nil();
    }

    if (strcmp(name, "copy") == 0) {
        if (nargs < 2 || args[0].type != V_STR || args[1].type != V_STR)
            rt_error("copy needs a source and destination path");
        FILE *in = fopen(args[0].str, "rb");
        if (!in) rt_error("copy: can't open source file");
        FILE *outf = fopen(args[1].str, "wb");
        if (!outf) { fclose(in); rt_error("copy: can't create destination file"); }
        char buf[65536]; size_t n;
        while ((n = fread(buf, 1, sizeof(buf), in)) > 0) fwrite(buf, 1, n, outf);
        fclose(in); fclose(outf);
        return zl_nil();
    }

    /* start(path[, args...]) - launch a program, don't wait for it, return its pid */
    if (strcmp(name, "start") == 0) {
        if (nargs < 1 || args[0].type != V_STR) rt_error("start needs a program path");
        pid_t child = fork();
        if (child < 0) rt_error("start: fork failed");
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
        return zl_num((double)child);
    }

    /* run(command) - run a shell command, wait for it, return its exit code */
    if (strcmp(name, "run") == 0) {
        if (nargs < 1 || args[0].type != V_STR) rt_error("run needs a command string");
        int status = system(args[0].str);
        return zl_num((double)(status == -1 ? -1 : WEXITSTATUS(status)));
    }

    /* kill(name_or_pid) - terminate a process (SIGTERM) by exact `procs()` name or numeric pid */
    if (strcmp(name, "kill") == 0) {
        if (nargs < 1) rt_error("kill needs a process name or pid");
        long pid;
        if (args[0].type == V_NUM) {
            pid = (long)args[0].num;
        } else if (args[0].type == V_STR) {
            pid = find_pid_by_name(args[0].str);
            if (pid < 0) return zl_bool(0);
        } else {
            rt_error("kill needs a process name (string) or pid (number)");
        }
        return zl_bool(kill((pid_t)pid, SIGTERM) == 0);
    }

    if (strcmp(name, "procs") == 0) {
        int count = 0;
        char **names = os_procs(&count);
        Value list = zl_nil(); list.type = V_LIST; list.nitems = count;
        list.items = malloc(sizeof(Value*) * (count > 0 ? count : 1));
        for (int i = 0; i < count; i++) {
            list.items[i] = malloc(sizeof(Value));
            *list.items[i] = zl_str(names[i]);
            free(names[i]);
        }
        free(names);
        return list;
    }

    if (is_simulated(name)) {
        printf("[sim] %s(", name);
        for (int i = 0; i < nargs; i++) {
            char *s = to_string(args[i]);
            if (i) printf(", ");
            printf("%s", s);
            free(s);
        }
        printf(")\n");
        return zl_nil();
    }

    /* ---- extended builtins (math / string / list / type) ---- */
    if (strcmp(name, "nil") == 0)    { return zl_nil(); }
    if (strcmp(name, "pi") == 0)     { return zl_num(3.14159265358979323846); }
    if (strcmp(name, "e") == 0)      { return zl_num(2.71828182845904523536); }
    if (strcmp(name, "assert") == 0) {
        if (nargs<1 || !zl_truthy(args[0])) {
            if (nargs>=2 && args[1].type==V_STR) fprintf(stderr,"assertion failed: %s\n", args[1].str);
            else rt_error("assertion failed");
            exit(1);
        }
        return zl_nil();
    }
    if (strcmp(name, "seed") == 0)   { srand((unsigned)(long long)args[0].num); return zl_nil(); }
    if (strcmp(name, "random") == 0) { return zl_num((double)rand() / ((double)RAND_MAX + 1.0)); }
    if (strcmp(name, "randint") == 0){
        long long lo=(long long)args[0].num, hi=(long long)args[1].num;
        if (hi<lo){ long long t=lo; lo=hi; hi=t; }
        return zl_num((double)(lo + rand() % (hi - lo + 1)));
    }
    if (strcmp(name, "sin") == 0)   { return zl_num(sin(args[0].num)); }
    if (strcmp(name, "cos") == 0)   { return zl_num(cos(args[0].num)); }
    if (strcmp(name, "tan") == 0)   { return zl_num(tan(args[0].num)); }
    if (strcmp(name, "log") == 0)   { return zl_num(log(args[0].num)); }
    if (strcmp(name, "exp") == 0)   { return zl_num(exp(args[0].num)); }
    if (strcmp(name, "atan") == 0)  { return zl_num(atan(args[0].num)); }
    if (strcmp(name, "sqrt") == 0)  { if (nargs<1) rt_error("sqrt needs a number");  return zl_num(sqrt(args[0].num)); }
    if (strcmp(name, "pow") == 0)   { if (nargs<2) rt_error("pow needs two numbers"); return zl_num(pow(args[0].num, args[1].num)); }
    if (strcmp(name, "floor") == 0) { if (nargs<1) rt_error("floor needs a number"); return zl_num(floor(args[0].num)); }
    if (strcmp(name, "ceil") == 0)  { if (nargs<1) rt_error("ceil needs a number");  return zl_num(ceil(args[0].num)); }
    if (strcmp(name, "round") == 0) { if (nargs<1) rt_error("round needs a number");  return zl_num(round(args[0].num)); }
    if (strcmp(name, "sign") == 0)  { double x=args[0].num; return zl_num(x>0?1:(x<0?-1:0)); }
    if (strcmp(name, "gcd") == 0)   { long long a=(long long)args[0].num,b=(long long)args[1].num; if(a<0)a=-a; if(b<0)b=-b; while(b){long long t=a%b;a=b;b=t;} return zl_num((double)a); }
    if (strcmp(name, "bool") == 0)  { return zl_bool(zl_truthy(args[0])); }
    if (strcmp(name, "type") == 0)  {
        const char *t="nil";
        switch (args[0].type){case V_NUM:t="num";break;case V_STR:t="str";break;case V_BOOL:t="bool";break;case V_LIST:t="list";break;default:t="nil";}
        return zl_str(t);
    }
    if (strcmp(name, "str") == 0)   { char *s=to_string(args[0]); Value v=zl_nil(); v.type=V_STR; v.str=s; return v; }
    /* chr(code): range-checked, not truncated to a byte - chr(321) used
     * to BE chr(65). Code 0 is refused: a zl string is a NUL-terminated
     * C string, so it cannot hold a NUL, and "" loses it silently. */
    if (strcmp(name, "chr") == 0)   {
        if (nargs<1||args[0].type!=V_NUM) rt_error("chr needs a number");
        double d = args[0].num;
        if (!(d >= 1 && d <= 255)) {
            if (d == 0) rt_error("chr(0): a zl string cannot hold a NUL byte");
            rt_error("chr needs a code in 1..255");
        }
        char b[2]; b[0]=(char)(int)d; b[1]='\0'; return zl_str(b);
    }
    /* sum: every element must be a number (or a bool, worth 0/1). It
     * used to read .num off any element, and .num is never written for
     * a string/list/nil, so sum(["a","b"]) returned heap garbage. */
    if (strcmp(name, "sum") == 0)   {
        if (args[0].type!=V_LIST) rt_error("sum needs a list");
        double s=0;
        for(int i=0;i<args[0].nitems;i++) {
            ValueType t = args[0].items[i]->type;
            if (t != V_NUM && t != V_BOOL) rt_error("sum needs a list of numbers");
            s += args[0].items[i]->num;
        }
        return zl_num(s);
    }
    if (strcmp(name, "first") == 0) { if (args[0].type!=V_LIST||args[0].nitems==0) rt_error("first needs a non-empty list"); return *args[0].items[0]; }
    if (strcmp(name, "last") == 0)  { if (args[0].type!=V_LIST||args[0].nitems==0) rt_error("last needs a non-empty list"); return *args[0].items[args[0].nitems-1]; }
    if (strcmp(name, "starts") == 0){ if (args[0].type!=V_STR||args[1].type!=V_STR) rt_error("starts needs two strings"); return zl_bool(strncmp(args[0].str,args[1].str,strlen(args[1].str))==0); }
    /* range: the count is worked out as a double and refused if it will
     * not fit in a list. The old (int) narrowing let range(2^31) build a
     * list whose reported length was INT32_MIN. */
    if (strcmp(name, "range") == 0) {
        if (nargs<1) rt_error("range needs a count");
        double dlo=0, dhi;
        if (nargs>=2){ dlo=args[0].num; dhi=args[1].num; } else dhi=args[0].num;
        /* A NaN bound makes dhi > dlo false, so the range is empty -
         * which is what it already was, only without the undefined cast. */
        double dcnt = dhi > dlo ? dhi - dlo : 0;
        if (dcnt > 100000000.0) rt_error("range count is too large to build");
        int cnt = (int)dcnt;
        long long lo = cnt > 0 ? exact_i64(dlo, "range") : 0;
        Value v=zl_nil(); v.type=V_LIST; v.nitems=cnt; v.cap=cnt; v.items=malloc(sizeof(Value*)*(size_t)(cnt>0?cnt:1));
        if (!v.items) rt_error("out of memory building a range");
        for (int i=0;i<cnt;i++){ v.items[i]=malloc(sizeof(Value)); *v.items[i]=zl_num((double)(lo+i)); }
        return v;
    }
    if (strcmp(name, "reverse") == 0) {
        if (args[0].type==V_STR){ size_t L=strlen(args[0].str); char*b=malloc(L+1); for(size_t i=0;i<L;i++) b[i]=args[0].str[L-1-i]; b[L]='\0'; Value v=zl_nil(); v.type=V_STR; v.str=b; return v; }
        if (args[0].type==V_LIST){ int m=args[0].nitems; Value v=zl_nil(); v.type=V_LIST; v.nitems=m; v.cap=m; v.items=malloc(sizeof(Value*)*(m>0?m:1)); for(int i=0;i<m;i++){v.items[i]=malloc(sizeof(Value)); *v.items[i]=*args[0].items[m-1-i];} return v; }
        rt_error("reverse needs a string or list");
    }
    if (strcmp(name, "repeat") == 0) {
        if (args[0].type!=V_STR||args[1].type!=V_NUM) rt_error("repeat needs a string and a count");
        int n=clamp_index(args[1].num, 0, 100000000); size_t L=strlen(args[0].str);
        if (L > 0 && (size_t)n > 100000000u / L) rt_error("repeat result is too large to build");
        char*b=malloc(L*(size_t)n+1);
        if (!b) rt_error("out of memory in repeat");
        for (int i=0;i<n;i++) memcpy(b+(size_t)i*L, args[0].str, L); b[L*(size_t)n]='\0';
        Value v=zl_nil(); v.type=V_STR; v.str=b; return v;
    }
    if (strcmp(name, "trim") == 0) {
        if (args[0].type!=V_STR) rt_error("trim needs a string");
        const char*s=args[0].str; while(*s==' '||*s=='\t'||*s=='\n'||*s=='\r')s++;
        const char*e=s+strlen(s); while(e>s&&(e[-1]==' '||e[-1]=='\t'||e[-1]=='\n'||e[-1]=='\r'))e--;
        size_t L=(size_t)(e-s); char*b=malloc(L+1); memcpy(b,s,L); b[L]='\0'; Value v=zl_nil(); v.type=V_STR; v.str=b; return v;
    }
    /* The empty needle occurs at every position, so it occurs len+1
     * times. Returning 0 contradicted find, has and `in`, all of which
     * delegate to strstr and report it found at index 0. */
    if (strcmp(name, "count") == 0) {
        if (args[0].type!=V_STR||args[1].type!=V_STR) rt_error("count needs two strings");
        const char*o=args[1].str; if(!*o) return zl_num((double)strlen(args[0].str)+1);
        int c=0; const char*p=args[0].str; while((p=strstr(p,o))){c++; p+=strlen(o);} return zl_num(c);
    }
    if (strcmp(name, "hex") == 0)   {
        if (nargs<1||args[0].type!=V_NUM) rt_error("hex needs a number");
        char b[HEX_BUF]; hex_of_double(b, args[0].num); return zl_str(b);
    }
    if (strcmp(name, "pad") == 0) {
        /* A negative width used to be cast to size_t, becoming SIZE_MAX,
         * so the fill loop ran off the end of the buffer and killed the
         * process. Clamp while the width is still signed. */
        if (args[0].type!=V_STR||args[1].type!=V_NUM) rt_error("pad needs a string and width");
        int w=clamp_index(args[1].num, 0, 100000000); size_t L=strlen(args[0].str); size_t o=((size_t)w>L)?(size_t)w:L;
        char*b=malloc(o+1);
        if (!b) rt_error("out of memory in pad");
        memcpy(b,args[0].str,L); for(size_t i=L;i<o;i++)b[i]=' '; b[o]='\0'; Value v=zl_nil(); v.type=V_STR; v.str=b; return v;
    }
    if (strcmp(name, "replace") == 0) {
        if (args[0].type!=V_STR||args[1].type!=V_STR||args[2].type!=V_STR) rt_error("replace needs three strings");
        const char*s=args[0].str,*o=args[1].str,*nw=args[2].str; size_t ol=strlen(o),nl=strlen(nw);
        if (ol==0){ Value v=zl_nil(); v.type=V_STR; v.str=_strdup(s); return v; }
        int c=0; const char*p=s; while((p=strstr(p,o))){c++; p+=ol;}
        size_t out=strlen(s)-(size_t)c*ol+(size_t)c*nl; char*b=malloc(out+1),*w=b; p=s; const char*q;
        while((q=strstr(p,o))){ memcpy(w,p,(size_t)(q-p)); w+=q-p; memcpy(w,nw,nl); w+=nl; p=q+ol; }
        strcpy(w,p); Value v=zl_nil(); v.type=V_STR; v.str=b; return v;
    }
    if (strcmp(name, "insert") == 0) {
        if (args[0].type!=V_LIST||args[1].type!=V_NUM) rt_error("insert needs a list, index, value");
        int m=args[0].nitems, idx=clamp_index(args[1].num, 0, m);
        Value v=zl_nil(); v.type=V_LIST; v.nitems=m+1; v.cap=m+1; v.items=malloc(sizeof(Value*)*(size_t)(m+1)); int k=0;
        for(int i=0;i<idx;i++){v.items[k]=malloc(sizeof(Value)); *v.items[k]=*args[0].items[i]; k++;}
        v.items[k]=malloc(sizeof(Value)); *v.items[k]=args[2]; k++;
        for(int i=idx;i<m;i++){v.items[k]=malloc(sizeof(Value)); *v.items[k]=*args[0].items[i]; k++;}
        return v;
    }
    if (strcmp(name, "remove") == 0) {
        if (args[0].type!=V_LIST||args[1].type!=V_NUM) rt_error("remove needs a list and index");
        int m=args[0].nitems;
        if (!(args[1].num >= 0 && args[1].num < (double)m)) rt_error("remove index out of range");
        int idx=(int)args[1].num;
        Value v=zl_nil(); v.type=V_LIST; v.nitems=m-1; v.cap=(m-1>0)?m-1:1; v.items=malloc(sizeof(Value*)*(size_t)(m>1?m-1:1)); int k=0;
        for(int i=0;i<m;i++){ if(i==idx)continue; v.items[k]=malloc(sizeof(Value)); *v.items[k]=*args[0].items[i]; k++; }
        return v;
    }

    /* ---- bitwise: zl has no bitwise OPERATORS, so these are the way.
     * Every argument goes in through exact_i64 and every answer comes
     * back out through bit_result, so a value the double cannot hold
     * exactly is an error at BOTH ends instead of a silent lie at
     * either. A shift count outside 0..63 gives 0. Identical to
     * interp.c's copy - see the policy comment there. ---- */
    if (strcmp(name, "band") == 0) {
        if (nargs<2||args[0].type!=V_NUM||args[1].type!=V_NUM) rt_error("band needs two numbers");
        return bit_result(exact_i64(args[0].num,"band") & exact_i64(args[1].num,"band"), "band");
    }
    if (strcmp(name, "bor") == 0) {
        if (nargs<2||args[0].type!=V_NUM||args[1].type!=V_NUM) rt_error("bor needs two numbers");
        return bit_result(exact_i64(args[0].num,"bor") | exact_i64(args[1].num,"bor"), "bor");
    }
    if (strcmp(name, "bxor") == 0) {
        if (nargs<2||args[0].type!=V_NUM||args[1].type!=V_NUM) rt_error("bxor needs two numbers");
        return bit_result(exact_i64(args[0].num,"bxor") ^ exact_i64(args[1].num,"bxor"), "bxor");
    }
    /* bnot must be an involution; bit_result is what makes
     * bnot(bnot(x)) == x hold for every x it accepts, including at
     * INT64_MIN, where ~x is INT64_MAX and the nearest double to that
     * is 2^63 - one past the top of the range. */
    if (strcmp(name, "bnot") == 0) {
        if (nargs<1||args[0].type!=V_NUM) rt_error("bnot needs a number");
        return bit_result(~exact_i64(args[0].num,"bnot"), "bnot");
    }
    /* The shift COUNT is a selector, not an operand: a count outside
     * 0..63 gives 0 for ANY magnitude, so it is truncated and then
     * range-tested as a double instead of going through exact_i64. */
    if (strcmp(name, "shl") == 0) {
        if (nargs<2||args[0].type!=V_NUM||args[1].type!=V_NUM) rt_error("shl needs two numbers");
        double dn = trunc(args[1].num);
        if (!(dn >= 0 && dn <= 63)) return zl_num(0);
        long long a=exact_i64(args[0].num,"shl"), n=(long long)dn;
        return bit_result((long long)((unsigned long long)a << n), "shl");
    }
    if (strcmp(name, "shr") == 0) {   /* arithmetic (sign-filling) shift */
        if (nargs<2||args[0].type!=V_NUM||args[1].type!=V_NUM) rt_error("shr needs two numbers");
        double dn = trunc(args[1].num);
        if (!(dn >= 0 && dn <= 63)) return zl_num(0);
        long long a=exact_i64(args[0].num,"shr"), n=(long long)dn;
        if (a<0) return bit_result(-1 - (long long)((unsigned long long)(-1 - a) >> n), "shr");
        return bit_result((long long)((unsigned long long)a >> n), "shr");
    }

    /* ---- more math ---- */
    if (strcmp(name, "asin") == 0)  { if (nargs<1||args[0].type!=V_NUM) rt_error("asin needs a number");  return zl_num(asin(args[0].num)); }
    if (strcmp(name, "acos") == 0)  { if (nargs<1||args[0].type!=V_NUM) rt_error("acos needs a number");  return zl_num(acos(args[0].num)); }
    if (strcmp(name, "log2") == 0)  { if (nargs<1||args[0].type!=V_NUM) rt_error("log2 needs a number");  return zl_num(log2(args[0].num)); }
    if (strcmp(name, "log10") == 0) { if (nargs<1||args[0].type!=V_NUM) rt_error("log10 needs a number"); return zl_num(log10(args[0].num)); }
    if (strcmp(name, "trunc") == 0) { if (nargs<1||args[0].type!=V_NUM) rt_error("trunc needs a number"); return zl_num(trunc(args[0].num)); }
    /* C's hypot() scales its operands, so it does not overflow or
     * underflow on the way to a representable result; the hand-rolled
     * sqrt(a*a+b*b) made hypot(1e200,0) inf and hypot(3e-200,4e-200) 0. */
    if (strcmp(name, "hypot") == 0) {
        if (nargs<2||args[0].type!=V_NUM||args[1].type!=V_NUM) rt_error("hypot needs two numbers");
        return zl_num(hypot(args[0].num, args[1].num));
    }
    if (strcmp(name, "fmod") == 0) {  /* b == 0 gives nan, like / gives inf */
        if (nargs<2||args[0].type!=V_NUM||args[1].type!=V_NUM) rt_error("fmod needs two numbers");
        return zl_num(fmod(args[0].num, args[1].num));
    }
    if (strcmp(name, "clamp") == 0) {
        if (nargs<3||args[0].type!=V_NUM||args[1].type!=V_NUM||args[2].type!=V_NUM) rt_error("clamp needs three numbers");
        double x=args[0].num, lo=args[1].num, hi=args[2].num;
        if (x<lo) return zl_num(lo);
        if (x>hi) return zl_num(hi);
        return zl_num(x);
    }

    /* ---- more strings ---- */
    if (strcmp(name, "ltrim") == 0) {
        if (nargs<1||args[0].type!=V_STR) rt_error("ltrim needs a string");
        const char*s=args[0].str; while(*s==' '||*s=='\t'||*s=='\n'||*s=='\r')s++;
        return zl_str(s);
    }
    if (strcmp(name, "rtrim") == 0) {
        if (nargs<1||args[0].type!=V_STR) rt_error("rtrim needs a string");
        const char*s=args[0].str; const char*e=s+strlen(s);
        while(e>s&&(e[-1]==' '||e[-1]=='\t'||e[-1]=='\n'||e[-1]=='\r'))e--;
        size_t L=(size_t)(e-s); char*b=malloc(L+1); memcpy(b,s,L); b[L]='\0';
        Value v=zl_nil(); v.type=V_STR; v.str=b; return v;
    }
    if (strcmp(name, "title") == 0) {  /* upper-case the first letter of each word */
        if (nargs<1||args[0].type!=V_STR) rt_error("title needs a string");
        size_t L=strlen(args[0].str); char*b=malloc(L+1); int at_start=1;
        for (size_t i=0;i<L;i++) {
            char ch=args[0].str[i];
            int alnum=(ch>='a'&&ch<='z')||(ch>='A'&&ch<='Z')||(ch>='0'&&ch<='9');
            if (at_start && ch>='a' && ch<='z') ch=(char)(ch-32);
            b[i]=ch; at_start = !alnum;
        }
        b[L]='\0'; Value v=zl_nil(); v.type=V_STR; v.str=b; return v;
    }
    if (strcmp(name, "swapcase") == 0) {
        if (nargs<1||args[0].type!=V_STR) rt_error("swapcase needs a string");
        size_t L=strlen(args[0].str); char*b=malloc(L+1);
        for (size_t i=0;i<L;i++) {
            char ch=args[0].str[i];
            if (ch>='a'&&ch<='z') ch=(char)(ch-32);
            else if (ch>='A'&&ch<='Z') ch=(char)(ch+32);
            b[i]=ch;
        }
        b[L]='\0'; Value v=zl_nil(); v.type=V_STR; v.str=b; return v;
    }
    if (strcmp(name, "index_at") == 0) {  /* character code at position i, -1 off the end */
        if (nargs<2||args[0].type!=V_STR||args[1].type!=V_NUM) rt_error("index_at needs a string and a number");
        size_t L=strlen(args[0].str);
        if (!(args[1].num >= 0 && args[1].num < (double)L)) return zl_num(-1);
        return zl_num((double)(unsigned char)args[0].str[(int)args[1].num]);
    }

    /* ---- more lists ---- */
    if (strcmp(name, "concat") == 0) {
        if (nargs<2||args[0].type!=V_LIST||args[1].type!=V_LIST) rt_error("concat needs two lists");
        int m=args[0].nitems+args[1].nitems;
        Value v=zl_nil(); v.type=V_LIST; v.nitems=m; v.cap=m; v.items=malloc(sizeof(Value*)*(m>0?m:1)); int k=0;
        for(int i=0;i<args[0].nitems;i++){v.items[k]=malloc(sizeof(Value)); *v.items[k]=*args[0].items[i]; k++;}
        for(int i=0;i<args[1].nitems;i++){v.items[k]=malloc(sizeof(Value)); *v.items[k]=*args[1].items[i]; k++;}
        return v;
    }
    if (strcmp(name, "fill") == 0) {
        if (nargs<2||args[0].type!=V_NUM) rt_error("fill needs a count and a value");
        if (args[0].num > 100000000.0) rt_error("fill count is too large to build");
        int m=clamp_index(args[0].num, 0, 100000000);
        Value v=zl_nil(); v.type=V_LIST; v.nitems=m; v.cap=m; v.items=malloc(sizeof(Value*)*(size_t)(m>0?m:1));
        if (!v.items) rt_error("out of memory in fill");
        for(int i=0;i<m;i++){v.items[i]=malloc(sizeof(Value)); *v.items[i]=args[1];}
        return v;
    }
    if (strcmp(name, "flat") == 0) {   /* flattens ONE level */
        if (nargs<1||args[0].type!=V_LIST) rt_error("flat needs a list");
        int m=0;
        for(int i=0;i<args[0].nitems;i++) m += (args[0].items[i]->type==V_LIST) ? args[0].items[i]->nitems : 1;
        Value v=zl_nil(); v.type=V_LIST; v.nitems=m; v.cap=m; v.items=malloc(sizeof(Value*)*(m>0?m:1)); int k=0;
        for(int i=0;i<args[0].nitems;i++){
            Value *it=args[0].items[i];
            if (it->type==V_LIST) { for(int j=0;j<it->nitems;j++){v.items[k]=malloc(sizeof(Value)); *v.items[k]=*it->items[j]; k++;} }
            else { v.items[k]=malloc(sizeof(Value)); *v.items[k]=*it; k++; }
        }
        return v;
    }
    /* take/drop: n clamped to 0..len IN THE DOUBLE DOMAIN. Narrowing
     * first meant any n >= 2^31 landed on INT32_MIN, which the old
     * `if (n<0) n=0` then read as zero - so take(xs, 2^31) returned
     * nothing where take(xs, 2^31 - 1) returned everything. */
    if (strcmp(name, "take") == 0) {
        if (nargs<2||args[0].type!=V_LIST||args[1].type!=V_NUM) rt_error("take needs a list and a number");
        int m=args[0].nitems, n=clamp_index(args[1].num, 0, m);
        Value v=zl_nil(); v.type=V_LIST; v.nitems=n; v.cap=n; v.items=malloc(sizeof(Value*)*(n>0?n:1));
        for(int i=0;i<n;i++){v.items[i]=malloc(sizeof(Value)); *v.items[i]=*args[0].items[i];}
        return v;
    }
    if (strcmp(name, "drop") == 0) {
        if (nargs<2||args[0].type!=V_LIST||args[1].type!=V_NUM) rt_error("drop needs a list and a number");
        int m=args[0].nitems, n=clamp_index(args[1].num, 0, m); int c=m-n;
        Value v=zl_nil(); v.type=V_LIST; v.nitems=c; v.cap=c; v.items=malloc(sizeof(Value*)*(c>0?c:1));
        for(int i=0;i<c;i++){v.items[i]=malloc(sizeof(Value)); *v.items[i]=*args[0].items[n+i];}
        return v;
    }

    /* ---- system ---- */
    if (strcmp(name, "now") == 0) {    /* milliseconds since this process started */
        return zl_num((double)clock() * 1000.0 / (double)CLOCKS_PER_SEC);
    }
    if (strcmp(name, "exit") == 0) {
        exit((nargs>=1 && args[0].type==V_NUM) ? (int)args[0].num : 0);
    }
    if (strcmp(name, "env") == 0) {
        if (nargs<1||args[0].type!=V_STR) rt_error("env needs a string");
        const char *val = getenv(args[0].str);
        return zl_str(val ? val : "");
    }

    rt_error("unknown function");
    return zl_nil();
}

Value zl_calln(const char *name, int n, ...)
{
    Value *args = malloc(sizeof(Value) * (n > 0 ? n : 1));
    va_list ap;
    va_start(ap, n);
    for (int i = 0; i < n; i++) args[i] = va_arg(ap, Value);
    va_end(ap);
    Value r = builtin(name, args, n);
    free(args);
    return r;
}

/* =============================================================
 * THE zlx_ BRIDGE - for the LLVM backend (compilel.c) ONLY.
 *
 * compilel emits UNBOXED values (i64, double, ptr). runtime.c's builtins
 * take and return a BOXED 48-byte Value. These helpers cross that boundary
 * WITHOUT the Value struct ever appearing in the emitted IR: the compiled
 * code stack-allocates raw 48-byte slots and hands over only POINTERS to
 * them, so the Win64 struct-return ABI never comes into play.
 *
 * zlx_call can reach the static builtin() because it lives in this file.
 * These are NEW and used only by compilel - the interpreter and the C
 * backend do not touch them, so there is no parity obligation here.
 * ============================================================= */

void zlx_num (Value *out, double n)      { *out = zl_num(n); }
void zlx_str (Value *out, const char *s) { *out = zl_str(s); }
void zlx_bool(Value *out, int b)         { *out = zl_bool(b); }
void zlx_call(Value *out, const char *name, Value *args, int n) { *out = builtin(name, args, n); }

double      zlx_as_num (Value *v) { return v->num; }
const char *zlx_as_str (Value *v) { return v->str; }
long long   zlx_as_int (Value *v) { return (long long)v->num; }
int         zlx_as_bool(Value *v) { return v->num != 0; }
