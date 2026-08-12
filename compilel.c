/* compilel.c - the LLVM backend for zl (integer + floating point + strings).
 *
 * Emits LLVM IR (text, out.ll) instead of C. clang then optimizes it:
 *   your.zl --[compilel]--> out.ll --[clang -O2]--> out.exe  (LLVM-fast)
 *
 * LLVM IR is lower-level than C: every operation is one typed instruction
 * producing a fresh SSA temporary. Values are unboxed.
 *
 * Supported: arithmetic, comparisons, and/or/not, variables, if/else,
 * while, break/continue, functions (fn/return/call), print, strings
 * (literals, + as concatenation, == / !=, truthiness, len).
 *
 * Temporaries are NAMED (%tN) on purpose: LLVM only demands strict
 * sequential numbering for UNNAMED values (%0, %1, ...), so naming them
 * lets us emit in any order we like. Same trick for labels.
 *
 * Separate file; never touches compile.c, so the self-host fixpoint is safe.
 *
 * =====================================================================
 * THE REPRESENTATION RULE  (read this before touching the type code)
 * =====================================================================
 *
 * zl has ONE number type and it is a C double - interp.c stores every
 * number in Value.num, a double, and there is no integer type anywhere
 * in the language. So the only correct answer for 7 / 3 is 2.33333 and
 * for 2.5 * 2.0 is 5.
 *
 * A backend that made every number a double would match the interpreter
 * exactly, but it would also be slow: measured with clang -O2, the
 * collatz benchmark costs 68 ms on i64 and 122 ms on double, because
 * every `%` has to round-trip through fptosi/srem/sitofp.
 *
 * So this backend keeps TWO machine representations of the one zl number
 * type and picks between them statically:
 *
 *      T_INT  ->  i64      a number that is provably a whole number
 *      T_NUM  ->  double   any number at all
 *
 * T_INT is a SUBSET of T_NUM (every i64 zl can produce fits a double
 * exactly up to 2^53, which is the same limit the interpreter has), so
 * widening INT -> NUM with sitofp is always value-preserving and NUM
 * never has to narrow back. The lattice is one step high:  INT  <  NUM.
 *
 * Which type does an expression have?
 *
 *      literal with a '.'          NUM      (the lexer's only float form)
 *      literal without one         INT
 *      true / false                INT      (0 / 1, as before)
 *      comparison, and, or, not    INT      (0 / 1)
 *      -x                          same as x
 *      + - *                       NUM if either side is NUM, else INT
 *      %                           NUM if either side is NUM, else INT
 *      /                           NUM, ALWAYS - this is the rule that
 *                                  makes 7 / 3 come out 2.33333
 *      call f(...)                 f's return type
 *      variable                    the variable's type
 *
 * A variable's type is the JOIN of every value assigned to it, a
 * function's return type the join of everything it returns, and a
 * parameter's type the join of every argument passed at any call site.
 * Those three feed each other, so infer_types() runs them to a fixpoint.
 * Everything starts at INT and only ever rises, so it terminates.
 *
 * The consequence worth knowing: `n = n / 2` makes n a double for the
 * whole program, even when every division happens to be exact. That is
 * the honest cost of zl's `/` and it is what the interpreter does too.
 *
 * PRINTING copies interp.c's value_to_string verbatim: a whole number
 * prints through "%lld" (so 5, not 5.000000) and anything else through
 * "%g" (so 2.33333). An INT prints through "%lld" directly; a NUM goes
 * to the @zl_printnum helper below, which makes that same choice at
 * runtime. Both engines call the same UCRT printf, so the digits match.
 *
 * =====================================================================
 * STRINGS  (the third machine representation)
 * =====================================================================
 *
 *      T_STR  ->  ptr      a NUL-terminated byte string
 *
 * That is exactly interp.c's representation: Value.str is a `char *`
 * filled by make_str with malloc(strlen(s)+1) + strcpy. So a zl string
 * here is the same thing the interpreter has, and the operations below
 * are the same C library calls the interpreter makes.
 *
 * T_STR is NOT part of the INT < NUM chain - it is INCOMPARABLE with
 * both. A number can never become a string and a string can never
 * become a number, so there is no coercion between them and coerce()
 * REFUSES the mix out loud. The one place inference could ask for it is
 * a variable, parameter or return value that is a number down one path
 * and a string down another; that program simply does not compile here
 * yet, which is slower than the interpreter but never wrong.
 *
 * The operations, each a transcription of the interpreter:
 *
 *   + with a string on EITHER side   eval_plus's last arm: turn both
 *                                    operands into text and join them.
 *                                    So "n=" + 5 is "n=5" while 2 + 3
 *                                    is still 5 - the overload is
 *                                    resolved statically here, from the
 *                                    operand types, instead of at run
 *                                    time from the value tags.
 *   == / !=                          values_equal: DIFFERENT types are
 *                                    never equal, so a string against a
 *                                    number folds to a constant here;
 *                                    two strings go to strcmp.
 *   truthiness                       is_truthy's V_STR arm: a string is
 *                                    true when it is not empty.
 *   len(s)                           strlen, as the len builtin does.
 *
 * - * / % and < > <= >= are numbers-only in the interpreter (they call
 * runtime_error on a string), so they are refused here rather than
 * given a meaning zl does not have.
 *
 * MEMORY: concatenation mallocs and nothing is ever freed. That is
 * deliberate - it is exactly what interp.c does (make_str and eval_plus
 * both malloc, neither frees) and freeing is a separate decision for
 * the whole language, not something for one backend to invent.
 *
 * KNOWN GAP, and the reason for the is_boolish() refusal below: zl's
 * booleans are a distinct type in the interpreter (V_BOOL prints as
 * "true"/"false", and true + 1 is the string "true1"), but this backend
 * has no T_BOOL - it represents true/false as the integers 1/0. So
 * print(1 < 2) already says 1 where the interpreter says true. That
 * predates strings and is a separate slice; what strings add is that
 * "x=" + (a < b) would silently say "x=1" instead of "x=false", so a
 * concatenation with a SYNTACTICALLY boolean operand is refused. A bool
 * that has been through a variable first is not caught - that hole
 * closes when T_BOOL lands, not before.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

#define REFLEN (MAX_TEXT + 32)   /* room for "@v_" + a name, or "%tNNNN" */

/* The machine representations.
 *
 *   T_NONE   nothing has been assigned yet. The BOTTOM of the lattice:
 *            it joins with anything, which is what lets a slot that
 *            starts out zeroed (calloc) rise to a string or a list. By
 *            emission time a NONE slot can only hold the zero it was
 *            initialised with, so it is machined as an i64.
 *   T_INT    a number that is provably whole            -> i64
 *   T_NUM    any number at all                          -> double
 *   T_STR    a NUL-terminated byte string               -> ptr
 *   >=T_LIST0  a list of a particular element type      -> ptr
 *
 * The two NUMBER ones are ordered INT < NUM and join by max. T_STR and
 * the list types are different kinds of thing entirely and join with
 * neither (see the STRINGS and LISTS notes in the header). */
typedef int Ty;
#define T_NONE  0
#define T_BOOL  1      /* true/false - an i64 0/1 that PRINTS as true/false */
#define T_INT   2
#define T_NUM   3
#define T_STR   4
#define T_LIST0 5      /* T_LIST0 + i is the list type g_listelem[i] */

static Ty  g_listelem[256];    /* the element type of each list type   */
static int g_nlisttypes = 0;

static int is_str(Ty t)  { return t == T_STR; }
static int is_list(Ty t) { return t >= T_LIST0; }
/* T_BOOL counts as number-like: it is an i64 0/1 in the machine, and
 * interp.c happily does `true + 1` (make_bool's .num is 0/1). Ordering it
 * BELOW T_INT means the existing "join by max" widens a bool to whatever
 * number it meets, so `true + 1` types as INT and prints 2 - matching the
 * reference. Only PRINTING and str() treat it specially. */
static int is_numlike(Ty t) { return t == T_NONE || t == T_BOOL || t == T_INT || t == T_NUM; }
static Ty  elem_of(Ty t) { return g_listelem[t - T_LIST0]; }

/* The list type whose elements are `elem`, interned so that two lists of
 * the same thing are the SAME Ty and compare with ==.
 *
 * An INT element is folded to NUM on the way in. interp.c has one number
 * type and it is a double - Value.num - so every number in a list is a
 * double there, and making the machine element type depend on whether the
 * literal happened to be written 1 or 1.0 would make `xs = [1]` and
 * `xs = [1.5]` two incompatible types for the same variable. Elements are
 * therefore always doubles, exactly as in the interpreter; only scalars
 * get to be i64. */
static Ty ty_list(Ty elem)
{
    if (elem == T_INT) elem = T_NUM;
    for (int i = 0; i < g_nlisttypes; i++)
        if (g_listelem[i] == elem) return T_LIST0 + i;
    if (g_nlisttypes >= 256) { fprintf(stderr,"compilel: too many list types\n"); exit(1); }
    g_listelem[g_nlisttypes] = elem;
    return T_LIST0 + g_nlisttypes++;
}

/* the JOIN of two types - the one type that can hold both - or 0 when
 * there is no such type (a number and a string, say). */
static int ty_join(Ty a, Ty b, Ty *out)
{
    if (a == b)      { *out = a; return 1; }
    if (a == T_NONE) { *out = b; return 1; }
    if (b == T_NONE) { *out = a; return 1; }
    if (is_numlike(a) && is_numlike(b)) { *out = a > b ? a : b; return 1; }
    if (is_list(a) && is_list(b)) {
        Ty e;
        if (!ty_join(elem_of(a), elem_of(b), &e)) return 0;
        *out = ty_list(e);
        return 1;
    }
    return 0;
}

static const char *llty(Ty t)
{
    if (t == T_STR || is_list(t)) return "ptr";
    return t == T_NUM ? "double" : "i64";
}
static const char *zero_of(Ty t)
{
    if (is_list(t)) return "@.listempty";
    if (t == T_STR) return "@.strempty";
    return t == T_NUM ? "0x0000000000000000" : "0";
}

/* --- small name sets ------------------------------------------- */

typedef struct { char names[512][MAX_TEXT]; int count; } NameSet;

static int set_index(const NameSet *s, const char *n) {
    for (int i=0;i<s->count;i++) if(!strcmp(s->names[i],n)) return i;
    return -1;
}
static void set_add(NameSet *s, const char *n) {
    if (set_index(s,n) >= 0) return;
    if (s->count<512){ strncpy(s->names[s->count],n,MAX_TEXT-1); s->names[s->count][MAX_TEXT-1]='\0'; s->count++; }
}
static int set_has(const NameSet *s, const char *n) { return set_index(s,n) >= 0; }

/* everything known about one user function. Params occupy the first
 * nparams slots of `locals` (set_add keeps insertion order), so a
 * parameter's type lives in localty at its own name's index. */
typedef struct {
    Node   *node;            /* the N_FN                             */
    NameSet locals;          /* params first, then assigned names    */
    Ty      localty[512];    /* parallel to locals.names             */
    Ty      ret;
    int     nparams;
} FnInfo;

static NameSet g_globals;   /* top-level variables   -> @v_NAME  */
static Ty      g_globalty[512];
static NameSet g_fns;       /* user function names   -> @fn_NAME */
static FnInfo *g_fninfo[512];   /* parallel to g_fns             */
static FnInfo *g_curfn = 0;     /* function being typed/emitted, 0 = top level */

static void collect_vars(Node *n, NameSet *out) {
    if (!n) return;
    if (n->type==N_ASSIGN && n->a->type==N_IDENT) set_add(out, n->a->text);
    if (n->type==N_FOR) set_add(out, n->text);   /* the loop variable binds */
    for (int i=0;i<n->nkids;i++) collect_vars(n->kids[i], out);
    collect_vars(n->a,out); collect_vars(n->b,out); collect_vars(n->c,out);
}

/* --- string literals become private globals -------------------- */

static Node *g_strs[512];
static int   g_nstr = 0;

static void collect_strs(Node *n) {
    if (!n) return;
    if (n->type==N_STRING && g_nstr<512) g_strs[g_nstr++] = n;
    for (int i=0;i<n->nkids;i++) collect_strs(n->kids[i]);
    collect_strs(n->a); collect_strs(n->b); collect_strs(n->c);
}
static int str_index(Node *n) {
    for (int i=0;i<g_nstr;i++) if (g_strs[i]==n) return i;
    return -1;
}

/* --- output state ---------------------------------------------- */

static FILE *out;
static int tmp = 0;          /* named-temporary counter */
static int lbl = 0;          /* label-set counter       */
static int terminated = 0;   /* has the current block ended in br/ret? */

static int newtmp(void) { return ++tmp; }

/* which runtime helpers this program turned out to need - see the
 * HELPER_ strings near the bottom */
static int g_used_mod = 0;        /* called @zl_mod                       */
static int g_used_printnum = 0;   /* called @zl_printnum                  */
static int g_used_printbool = 0;  /* printed a bool - needs true/false    */
static int g_used_listbox = 0;    /* boxed a list arg for a builtin       */
static int g_used_sat = 0;        /* used the fptosi.sat intrinsic        */
static int g_used_concat = 0;     /* called @zl_concat                    */
static int g_used_intstr = 0;     /* called @zl_intstr                    */
static int g_used_numstr = 0;     /* called @zl_numstr                    */
static int g_used_strcmp = 0;     /* called @strcmp                       */
static int g_used_strlen = 0;     /* called @strlen directly (len)        */
static int g_used_listrt = 0;     /* used the list runtime (HELPER_LISTRT) */
static int g_used_bridge = 0;     /* called a builtin through the zlx_ bridge */

/* sizeof(Value) in runtime.h - the size of one boxed-value slot the bridge
 * stack-allocates. runtime.c guards this equality with a static assert.
 * Was 48 before first-class functions added fnptr+fnargs to Value. */
#define VALSZ 64

/* the builtin names referenced through the bridge, each emitted once as a
 * private @.bname.N constant so @zlx_call can be handed a name pointer */
static char g_bnames[256][MAX_TEXT];
static int  g_nbnames = 0;
static int  bname_ref(const char *name) {
    for (int i=0;i<g_nbnames;i++) if (!strcmp(g_bnames[i],name)) return i;
    if (g_nbnames >= 256) { fprintf(stderr,"compilel: too many builtin names\n"); exit(1); }
    strncpy(g_bnames[g_nbnames], name, MAX_TEXT-1);
    g_bnames[g_nbnames][MAX_TEXT-1] = '\0';
    return g_nbnames++;
}

/* enclosing loops: each entry is a label-set id. 'break' always goes to
 * loop_end_ID; 'continue' goes to loopcont[]_ID, which is "loop_cond" for
 * a while (re-test the condition) but "loop_step" for a for (advance the
 * index FIRST, or the loop spins). A stack, so nesting works. */
static int loopids[64];
static const char *loopcont[64];
static int nloops = 0;

/* where does this name live - a function-local alloca, or a global? */
static void var_slot(const char *name, char *buf) {
    if (g_curfn && set_has(&g_curfn->locals, name)) sprintf(buf, "%%l_%s", name);
    else                                            sprintf(buf, "@v_%s", name);
}

/* =====================================================================
 * TYPES - inference, run to a fixpoint before a single line is emitted
 * ===================================================================== */

/* where a name's type is stored, or 0 when the name is unknown (a
 * builtin, or a read of something never assigned) */
static Ty *var_ty_slot(FnInfo *f, const char *name) {
    if (f) { int i = set_index(&f->locals, name); if (i >= 0) return &f->localty[i]; }
    int i = set_index(&g_globals, name);
    if (i >= 0) return &g_globalty[i];
    return 0;
}
static Ty var_ty(FnInfo *f, const char *name) {
    Ty *s = var_ty_slot(f, name);
    return s ? *s : T_INT;
}
/* the declared type of parameter i - by NAME, so a repeated parameter
 * name resolves to the same slot the stores in emit_fn use */
static Ty param_ty(FnInfo *f, int i) {
    if (i >= f->nparams) return T_INT;
    return var_ty(f, f->node->kids[i]->text);
}

static int is_cmp_op(const char *op) {
    return !strcmp(op,"==")||!strcmp(op,"!=")||!strcmp(op,"<")||
           !strcmp(op,">") ||!strcmp(op,"<=")||!strcmp(op,">=");
}

/* Is this expression OBVIOUSLY a zl boolean? interp.c keeps V_BOOL apart
 * from V_NUM and prints it "true"/"false", but this backend has no
 * T_BOOL and carries a bool as the integer 1/0 - so joining one to a
 * string would produce "1" where the interpreter produces "true".
 * Syntactic, and deliberately so: it catches the direct forms and says
 * so, and cannot see a bool that went through a variable first. See the
 * KNOWN GAP paragraph in the header. */
static int is_boolish(Node *n) {
    if (!n) return 0;
    if (n->type == N_BOOL) return 1;
    if (n->type == N_UNARY  && !strcmp(n->text,"not")) return 1;
    if (n->type == N_BINARY && (is_cmp_op(n->text) || !strcmp(n->text,"and") ||
                                !strcmp(n->text,"or") || !strcmp(n->text,"in"))) return 1;
    return 0;
}

/* =====================================================================
 * EXACT DIVISION - the one refinement to "/ is always a double"
 * =====================================================================
 *
 * `/` is real division, so `n = n / 2` normally makes n a double for
 * the whole program and drags every other operation on it along. That
 * is correct, and on collatz it costs 2.2x.
 *
 * But zl has no integer-division operator, so the way you write an
 * exact halving is to ASK FIRST:
 *
 *      if n % 2 == 0 { n = n / 2 }              (bench/c1_collatz)
 *      if n % d == 0 { ... e = n / d ... }      (bench/c4_divisors)
 *
 * Inside that branch the quotient IS the integer `n sdiv 2`, exactly,
 * so the division does not have to leave i64. This pass finds those
 * divisions before inference runs and records them; expr_ty and
 * emit_expr then both ask is_exact_div() and stay on the i64 path.
 * Recording them up front rather than threading a context through both
 * phases is what keeps the two phases from ever disagreeing.
 *
 * It is sound because:
 *   - the branch (or loop body) only runs when the guard just held, and
 *     the guard is re-evaluated on every entry;
 *   - the scan below stops at the first statement that could change
 *     either operand, so the value divided is the value tested;
 *   - the guard's own `%` already trapped on the two divisors sdiv
 *     cannot survive (0, and -1 against INT64_MIN), so reaching the
 *     division means the sdiv is safe.
 *
 * It deliberately recognises ONE shape. Anything it does not recognise
 * simply stays a double - slower, never wrong.
 */

static Node *g_exact[512];      /* the N_BINARY "/" nodes proven exact */
static int   g_nexact = 0;

static int is_exact_div(Node *n) {
    for (int i=0;i<g_nexact;i++) if (g_exact[i]==n) return 1;
    return 0;
}

/* an operand simple enough to compare by text and to re-read safely */
static int is_simple(Node *n) {
    return n && (n->type==N_IDENT || (n->type==N_NUMBER && !strchr(n->text,'.')));
}
static int same_simple(Node *x, Node *y) {
    return is_simple(x) && is_simple(y) && x->type==y->type && !strcmp(x->text,y->text);
}

/* does `c` say "A % B == 0"? (either way round) */
static int is_divisible_guard(Node *c, Node **A, Node **B) {
    if (!c || c->type!=N_BINARY || strcmp(c->text,"==")) return 0;
    Node *m = 0;
    if (c->a && c->a->type==N_BINARY && !strcmp(c->a->text,"%") &&
        c->b && c->b->type==N_NUMBER && !strcmp(c->b->text,"0")) m = c->a;
    if (c->b && c->b->type==N_BINARY && !strcmp(c->b->text,"%") &&
        c->a && c->a->type==N_NUMBER && !strcmp(c->a->text,"0")) m = c->b;
    if (!m || !is_simple(m->a) || !is_simple(m->b)) return 0;
    *A = m->a; *B = m->b;
    return 1;
}

static int assigns_name(Node *n, const char *name) {
    if (!n || !name) return 0;
    if (n->type==N_ASSIGN && n->a && n->a->type==N_IDENT && !strcmp(n->a->text,name)) return 1;
    if (n->type==N_FOR && !strcmp(n->text,name)) return 1;   /* the loop var binds */
    for (int i=0;i<n->nkids;i++) if (assigns_name(n->kids[i],name)) return 1;
    return assigns_name(n->a,name)||assigns_name(n->b,name)||assigns_name(n->c,name);
}
static int has_call(Node *n) {
    if (!n) return 0;
    if (n->type==N_CALL) return 1;
    for (int i=0;i<n->nkids;i++) if (has_call(n->kids[i])) return 1;
    return has_call(n->a)||has_call(n->b)||has_call(n->c);
}

static void mark_divs(Node *n, Node *A, Node *B) {
    if (!n) return;
    if (n->type==N_BINARY && !strcmp(n->text,"/") &&
        same_simple(n->a,A) && same_simple(n->b,B) && g_nexact<512)
        g_exact[g_nexact++] = n;
    for (int i=0;i<n->nkids;i++) mark_divs(n->kids[i],A,B);
    mark_divs(n->a,A,B); mark_divs(n->b,A,B); mark_divs(n->c,A,B);
}

/* walk the guarded block in statement order, marking A/B divisions
 * until something could change A or B */
static void scan_guarded_block(Node *blk, Node *A, Node *B) {
    const char *an = A->type==N_IDENT ? A->text : 0;
    const char *bn = B->type==N_IDENT ? B->text : 0;
    if (!blk || blk->type != N_BLOCK) return;
    for (int i=0;i<blk->nkids;i++) {
        Node *s = blk->kids[i];
        if (!assigns_name(s,an) && !assigns_name(s,bn)) { mark_divs(s,A,B); continue; }
        /* This statement writes an operand. A simple `x = expr` still
         * evaluates expr against the tested value, so the divisions in
         * expr count; a loop or a branch can reach its division again
         * AFTER the write, so those get nothing. Either way, stop. */
        if (s->type==N_ASSIGN && !assigns_name(s->b,an) && !assigns_name(s->b,bn))
            mark_divs(s->b,A,B);
        return;
    }
}

static void find_exact_divs(Node *n, FnInfo *f) {
    if (!n) return;
    if (n->type==N_IF || n->type==N_WHILE) {
        Node *A, *B;
        if (is_divisible_guard(n->a,&A,&B)) {
            /* assigns_name only sees this subtree. If an operand is a
             * global, a call in the branch could rewrite it out of
             * sight, so refuse the whole thing. */
            int outer = (A->type==N_IDENT && !(f && set_has(&f->locals,A->text)))
                     || (B->type==N_IDENT && !(f && set_has(&f->locals,B->text)));
            if (!(outer && has_call(n->b))) scan_guarded_block(n->b, A, B);
        }
    }
    for (int i=0;i<n->nkids;i++) find_exact_divs(n->kids[i],f);
    find_exact_divs(n->a,f); find_exact_divs(n->b,f); find_exact_divs(n->c,f);
}

/* Builtins reachable through the runtime.c bridge when CALLED IN AN
 * EXPRESSION (see emit_builtin_call). SCALAR in, SCALAR out only: a list
 * cannot be boxed into a Value slot in the IR, so any builtin that takes or
 * returns a list is NOT here and still errors as before. The return type
 * drives the unbox step:
 *      number-returning -> T_NUM   (unbox @zlx_as_num, a double)
 *      string-returning -> T_STR   (unbox @zlx_as_str, a ptr)
 * Numbers are always unboxed as a double even when the builtin's result is
 * whole (code, int, ...): a whole double prints exactly as the interpreter
 * prints it, so this is correct and never truncates.
 *
 * Left OUT on purpose: builtins taking/returning lists (unbridgeable here);
 * nondeterministic ones (random, randint, seed, now) so every enabled
 * builtin is parity-testable; nil-returning side-effect ones (print, seed,
 * assert, write, exit); and BOOL-returning ones (starts, has, contains,
 * ...) until this backend has a real bool type - carrying a bool as 0/1
 * would print "1" where the interpreter prints "true" (the header's KNOWN
 * GAP). str/len/push stay special-cased in emit_expr and are not here. */
static int builtin_bridge_ty(const char *name, Ty *out) {
    static const char *NUMS[] = {
        "abs","sqrt","pow","floor","ceil","round","sin","cos","tan","log",
        "exp","atan","asin","acos","log2","log10","trunc","hypot","fmod",
        "min","max","sign","gcd","clamp","num","int","code","find","count",
        "index_at","band","bor","bxor","bnot","shl","shr","pi","e",
        "sum","index_of", 0 };   /* stage 3: take a LIST - now boxable */
    static const char *STRS[] = {
        "upper","lower","trim","ltrim","rtrim","title","swapcase","slice",
        "chr","hex","pad","replace","repeat","type","env","at",
        "join", 0 };             /* stage 3: takes a LIST              */
    /* stage 3: now that this backend HAS a bool type, the make_bool
     * builtins can be bridged - they print true/false like the reference
     * instead of 1/0. Only the ones whose arguments are scalars: contains
     * takes a list and is still blocked on that, not on the bool. */
    static const char *BOOLS[] = { "has","starts","ends","bool",
                                   "contains", 0 };  /* takes a LIST   */
    for (int i=0; BOOLS[i]; i++) if (!strcmp(name, BOOLS[i])) { *out = T_BOOL; return 1; }

/* DELIBERATELY NOT BRIDGED YET - two separate blockers, neither of them
 * about the return type:
 *
 * 1. TAKES A LIST: sum, index_of, contains, join. The bridge boxes scalar
 *    arguments only ("passing a list to a builtin is not supported yet"),
 *    so these refuse at compile time regardless of what they return.
 *
 * 2. RETURNS A BOOL: has, starts, ends, contains, bool. These would
 *    compile and run, but print WRONG: this type system has no bool, so
 *    make_bool comes back as T_NUM and prints 1/0 where the interpreter
 *    - the reference - prints true/false. Bridging them would trade an
 *    honest compile-time refusal for a silently wrong answer, which is
 *    backwards (nativegen refuses to print a fractional double for the
 *    same reason). Fix the missing bool type first, then bridge these. */
    for (int i=0; NUMS[i]; i++) if (!strcmp(name, NUMS[i])) { *out = T_NUM; return 1; }
    for (int i=0; STRS[i]; i++) if (!strcmp(name, STRS[i])) { *out = T_STR; return 1; }
    return 0;
}

static Ty expr_ty(Node *n, FnInfo *f) {
    if (!n) return T_INT;
    switch (n->type) {
        case N_NUMBER: return strchr(n->text,'.') ? T_NUM : T_INT;
        case N_BOOL:   return T_BOOL;   /* true/false literal */
        case N_STRING: return T_STR;
        case N_IDENT:  return var_ty(f, n->text);
        /* -x keeps x's type; `not x` is a bool */
        case N_UNARY:  return !strcmp(n->text,"-") ? expr_ty(n->a,f) : T_BOOL;
        case N_DANGER: return expr_ty(n->a, f);   /* '!' is a pass-through */
        case N_TERNARY: {
            /* interp.c returns whichever branch is taken, so the static
             * type is the one that holds both - the JOIN of the two. A
             * join that cannot be made (a number branch and a string
             * branch) leaves the emit-time coerce to refuse it, so fall
             * back to the then-type rather than inventing one here. */
            Ty tb = expr_ty(n->b,f), tc = expr_ty(n->c,f), j;
            return ty_join(tb, tc, &j) ? j : tb;
        }
        case N_BINARY: {
            const char *op = n->text;
            /* comparisons and the logical operators yield true/false,
             * exactly as interp.c's make_bool does */
            if (is_cmp_op(op) || !strcmp(op,"and") || !strcmp(op,"or") || !strcmp(op,"in"))
                return T_BOOL;
            Ty ta = expr_ty(n->a,f), tb = expr_ty(n->b,f);
            /* eval_plus: a string on either side makes the whole thing a
             * string join. Everything else is numbers-only, and emitting
             * it is where a string operand gets refused. */
            if (!strcmp(op,"+") && (is_str(ta) || is_str(tb))) return T_STR;
            /* the rule: / is real division, unless a divisibility guard
             * has already proved this one exact (see find_exact_divs) */
            if (!strcmp(op,"/"))
                return (ta==T_INT && tb==T_INT && is_exact_div(n)) ? T_INT : T_NUM;
            return (ta==T_NUM || tb==T_NUM) ? T_NUM : T_INT;
        }
        case N_LIST: {
            /* the element type is the join of every element, and an empty
             * literal is a list of NONE - which joins with any other list,
             * so `out = []` then `out = push(out, "a")` types fine. */
            Ty e = T_NONE;
            for (int i=0;i<n->nkids;i++) {
                Ty j;
                if (ty_join(e, expr_ty(n->kids[i],f), &j)) e = j;
            }
            return ty_list(e);
        }
        case N_INDEX: {
            Ty t = expr_ty(n->a,f);
            return is_list(t) ? elem_of(t) : T_INT;   /* emit_expr refuses non-lists */
        }
        case N_CALL: {
            if (n->a && n->a->type==N_IDENT) {
                int i = set_index(&g_fns, n->a->text);
                if (i >= 0) return g_fninfo[i]->ret;
                /* push(xs, v) -> a list that holds both what xs held and v */
                if (!strcmp(n->a->text,"push") && n->nkids == 2) {
                    Ty j, want = ty_list(expr_ty(n->kids[1],f));
                    return ty_join(expr_ty(n->kids[0],f), want, &j) ? j : want;
                }
                /* a bridged builtin has a known scalar return type, so a
                 * variable that stores its result types correctly */
                Ty brt;
                if (builtin_bridge_ty(n->a->text, &brt)) return brt;
            }
            return T_INT;
        }
        default: return T_INT;   /* unsupported node - emit_expr will complain */
    }
}

/* the type a compound `op=` STORES, given what the target already holds.
 *
 * The `+` arm is what makes `s += 5` keep s a string: eval_plus turns
 * both sides into text as soon as either one is text, so the answer is a
 * string even though the right operand is a number. Everything else is a
 * join, and a join that cannot be made (a number into a string slot)
 * leaves the slot alone so that coerce() is the one to refuse it. */
static Ty combine_ty(const char *op, Ty cur, Ty v) {
    if (!op[0]) return v;                                       /* plain `=` */
    if (!strcmp(op,"+") && (is_str(cur) || is_str(v))) return T_STR;
    if (!strcmp(op,"/") && is_numlike(cur) && is_numlike(v)) return T_NUM;
    Ty j;
    return ty_join(cur, v, &j) ? j : cur;
}

static Ty assign_ty(Node *n, FnInfo *f) {
    return combine_ty(n->text, var_ty(f, n->a->text), expr_ty(n->b, f));
}

static void raise_ty(Ty *slot, Ty t, int *changed) {
    Ty j;
    if (!ty_join(*slot, t, &j)) return;   /* no such type: coerce() refuses it */
    if (j != *slot) { *slot = j; *changed = 1; }
}

/* Raise whatever an assignment writes into. A name raises its own slot;
 * `xs[i] = v` says nothing about i but everything about xs, which has to
 * be a list of v's type - and `xs[i][j] = v` says xs is a list of lists
 * of it, which is the recursion. */
static void raise_target(Node *t, Ty v, FnInfo *f, int *changed) {
    if (!t) return;
    if (t->type == N_IDENT) {
        Ty *s = var_ty_slot(f, t->text);
        if (s) raise_ty(s, v, changed);
    } else if (t->type == N_INDEX) {
        raise_target(t->a, ty_list(v), f, changed);
    }
}

static void infer_walk(Node *n, FnInfo *f, int *changed) {
    if (!n) return;
    if (n->type==N_ASSIGN && n->a->type==N_IDENT) {
        Ty *s = var_ty_slot(f, n->a->text);
        if (s) raise_ty(s, assign_ty(n, f), changed);
    }
    if (n->type==N_ASSIGN && n->a->type==N_INDEX) {
        Ty base = expr_ty(n->a->a, f);
        Ty cur  = is_list(base) ? elem_of(base) : T_NONE;
        raise_target(n->a, combine_ty(n->text, cur, expr_ty(n->b, f)), f, changed);
    }
    if (n->type==N_FOR) {
        /* the loop variable takes each element of the sequence in turn,
         * so its type is the sequence's element type. The sequence type
         * can itself rise during inference (a list built by push), which
         * is why this runs inside the fixpoint loop with everything else. */
        Ty st = expr_ty(n->a, f);
        if (is_list(st)) {
            Ty *s = var_ty_slot(f, n->text);
            if (s) raise_ty(s, elem_of(st), changed);
        }
    }
    /* Type every list literal, even one whose value is only ever handed
     * straight to a builtin. Asking is what INTERNS the list type, and
     * %zlist is emitted only when g_nlisttypes says one exists - so without
     * this, `print(sum([1,2,3]))` emits a getelementptr on an undeclared
     * %zlist. Nothing here consumes the answer; interning is the point. */
    if (n->type==N_LIST) (void)expr_ty(n, f);
    if (n->type==N_RETURN && f && n->a) raise_ty(&f->ret, expr_ty(n->a, f), changed);
    if (n->type==N_CALL && n->a && n->a->type==N_IDENT) {
        int i = set_index(&g_fns, n->a->text);
        if (i >= 0) {
            FnInfo *callee = g_fninfo[i];
            for (int k=0;k<n->nkids && k<callee->nparams;k++) {
                Ty *s = var_ty_slot(callee, callee->node->kids[k]->text);
                if (s) raise_ty(s, expr_ty(n->kids[k], f), changed);
            }
        }
    }
    for (int i=0;i<n->nkids;i++) infer_walk(n->kids[i], f, changed);
    infer_walk(n->a,f,changed); infer_walk(n->b,f,changed); infer_walk(n->c,f,changed);
}

/* Everything starts INT and only rises to NUM, and there are finitely
 * many slots, so this loop cannot run forever. */
static void infer_types(Node *prog) {
    int changed = 1;
    while (changed) {
        changed = 0;
        for (int i=0;i<prog->nkids;i++) {
            Node *k = prog->kids[i];
            if (k->type == N_FN) infer_walk(k->a, g_fninfo[set_index(&g_fns,k->text)], &changed);
            else                 infer_walk(k, 0, &changed);
        }
    }
}

/* =====================================================================
 * EMISSION
 * ===================================================================== */

static Ty emit_expr(Node *n, char *ref);
static void emit_stmt(Node *n);
static void emit_block(Node *n);

/* rewrite `ref` so it has type `want`. INT -> NUM is the widening the
 * type rules actually produce; the other direction is unreachable if
 * inference is right, and is implemented (saturating, so never poison)
 * only so a mistake shows up as a wrong number rather than as IR clang
 * refuses to parse. */
/* Can a `have` list be stored where a `want` list is expected with no
 * machine conversion at all? Every list is one pointer, so the only
 * question is whether the ELEMENT boxes hold the same machine type. They
 * do when the element types match, and vacuously when the value is an
 * empty literal - a list of NONE has no boxes to disagree about, which is
 * what lets `xs = []` initialise a list of anything. */
static int list_fits(Ty have, Ty want) {
    if (!is_list(have) || !is_list(want)) return 0;
    Ty he = elem_of(have), we = elem_of(want);
    if (he == T_NONE || he == we) return 1;
    return list_fits(he, we);
}

static void coerce(char *ref, Ty have, Ty want) {
    if (have == want) return;
    if (is_list(have) || is_list(want)) {
        if (list_fits(have, want)) return;   /* the same pointer, either way */
        fprintf(stderr, "compilel: a variable, parameter or return value holds "
                        "values of two different kinds (a list and something "
                        "else, or lists of different things); not supported yet\n");
        exit(1);
    }
    /* There is no conversion between a number and a string, because the
     * interpreter has none: a Value is a V_NUM or a V_STR and stays it.
     * Reaching here means one variable, parameter or return value is a
     * number down one path and a string down another, which this backend
     * cannot represent in a single machine slot. Say so. */
    if (is_str(have) || is_str(want)) {
        fprintf(stderr, "compilel: a variable, parameter or return value is a "
                        "number on one path and a string on another; not supported yet\n");
        exit(1);
    }
    /* T_NONE is machined as an i64, so it needs no conversion to T_INT */
    if (is_numlike(have) && is_numlike(want) &&
        (have==T_NUM) == (want==T_NUM)) return;
    int t = newtmp();
    if (want == T_NUM) fprintf(out, "  %%t%d = sitofp i64 %s to double\n", t, ref);
    else { g_used_sat = 1;
           fprintf(out, "  %%t%d = call i64 @llvm.fptosi.sat.i64.f64(double %s)\n", t, ref); }
    snprintf(ref, REFLEN, "%%t%d", t);
}

/* rewrite `ref` into a T_STR, the way value_to_string turns any value
 * into text before eval_plus joins it. A string is already text; a
 * number goes through the same "%lld or %g?" choice printing makes. */
static void to_text(char *ref, Ty have) {
    if (is_str(have)) return;
    if (is_list(have)) {
        fprintf(stderr, "compilel: turning a list into text is not supported yet\n");
        exit(1);
    }
    int t = newtmp();
    if (have == T_NUM) { g_used_numstr = 1;
        fprintf(out, "  %%t%d = call ptr @zl_numstr(double %s)\n", t, ref); }
    else               { g_used_intstr = 1;
        fprintf(out, "  %%t%d = call ptr @zl_intstr(i64 %s)\n", t, ref); }
    snprintf(ref, REFLEN, "%%t%d", t);
}

/* is this value truthy? interp.c: a number is truthy when it is != 0,
 * which for a double makes NaN truthy (every comparison against NaN is
 * false, so `!= 0` is true). `fcmp une` reproduces that exactly. */
static int emit_truth(const char *ref, Ty t) {
    /* interp.c's V_STR arm: a string is true when it is not empty, which
     * for a NUL-terminated string is one byte load. (The `v.str &&` half
     * of that test never fires here - every T_STR value is either a
     * literal, a concatenation result or @.strempty, none of them null.) */
    if (t == T_STR) {
        int b = newtmp();
        fprintf(out, "  %%t%d = load i8, ptr %s\n", b, ref);
        int c = newtmp();
        fprintf(out, "  %%t%d = icmp ne i8 %%t%d, 0\n", c, b);
        return c;
    }
    /* interp.c's V_LIST arm: a list is true when it has any items */
    if (is_list(t)) {
        int f = newtmp();
        fprintf(out, "  %%t%d = getelementptr inbounds %%zlist, ptr %s, i32 0, i32 0\n", f, ref);
        int m = newtmp();
        fprintf(out, "  %%t%d = load i64, ptr %%t%d\n", m, f);
        int c = newtmp();
        fprintf(out, "  %%t%d = icmp sgt i64 %%t%d, 0\n", c, m);
        return c;
    }
    int c = newtmp();
    if (t == T_NUM) fprintf(out, "  %%t%d = fcmp une double %s, 0x0000000000000000\n", c, ref);
    else            fprintf(out, "  %%t%d = icmp ne i64 %s, 0\n", c, ref);
    return c;
}

/* =====================================================================
 * LISTS - the machine shape, and the three primitives on it
 * =====================================================================
 *
 * A zl list is a POINTER to a header
 *
 *      %zlist = { i64 nitems, i64 cap, ptr tip, ptr items }
 *
 * which is interp.c's Value minus the fields a list does not use, and
 * `items` points at an array of POINTERS TO BOXES, one box per element,
 * exactly as interp.c's `Value **items` does. That indirection is not
 * decoration - it is observable:
 *
 *      a = [1, 2, 3]
 *      b = push(a, 4)      # copies the POINTER array, not the boxes
 *      b[0] = 99
 *      print(a[0])         # 99, in the interpreter and here
 *
 * push shares element boxes with its source, so a write through one list
 * is seen by the other. Storing elements inline would quietly disagree.
 *
 * Every box is 8 bytes, because every element machine type is: a number
 * element is a double (see ty_list - interp.c has no integer type, so a
 * list element is always a double there too), and a string or a nested
 * list is a pointer.
 *
 * THE HEADER IS IMMUTABLE. Nothing ever writes an existing one; push
 * allocates a new header even on its in-place path. That is what makes a
 * plain `ys = xs` - which shares the pointer here, where interp.c copies
 * the four fields - behave the same in both engines: a copy of something
 * that never changes is indistinguishable from a reference to it.
 *
 * MEMORY: headers, item arrays and boxes are malloc'd and never freed,
 * which is exactly what interp.c does (N_LIST and push both malloc, and
 * nothing in the interpreter frees a list). See the MEMORY note in the
 * header - freeing is a decision for the whole language.
 */

/* narrow an index to an i64, the way interp.c narrows idx.num */
static void to_index(char *ref, Ty t) {
    if (is_str(t) || is_list(t)) {
        fprintf(stderr, "compilel: a list index must be a number\n"); exit(1);
    }
    if (t != T_NUM) return;                  /* already an i64 */
    g_used_sat = 1;
    int r = newtmp();
    fprintf(out, "  %%t%d = call i64 @llvm.fptosi.sat.i64.f64(double %s)\n", r, ref);
    snprintf(ref, REFLEN, "%%t%d", r);
}

/* put a value in a fresh 8-byte box and leave `ref` pointing at the box */
static void emit_box(char *ref, Ty elem) {
    g_used_listrt = 1;
    int b = newtmp();
    fprintf(out, "  %%t%d = call ptr @malloc(i64 8)\n", b);
    fprintf(out, "  store %s %s, ptr %%t%d\n", llty(elem), ref, b);
    snprintf(ref, REFLEN, "%%t%d", b);
}

/* Which list types need a structural == helper. One is generated per
 * ELEMENT type, and a list of lists needs its element's helper too, so
 * asking for one asks for everything it will call. */
static int g_eqneed[256];
static int g_neqneed = 0;

static void need_eq(Ty t) {
    for (int i=0;i<g_neqneed;i++) if (g_eqneed[i]==t) return;
    if (g_neqneed >= 256) { fprintf(stderr,"compilel: too many list types\n"); exit(1); }
    g_eqneed[g_neqneed++] = t;
    g_used_listrt = 1;
    Ty e = elem_of(t);
    if (is_list(e)) need_eq(e);
    if (is_str(e))  g_used_strcmp = 1;
}

/* the same, for the printer that renders "[a, b, c]" */
static int g_prneed[256];
static int g_nprneed = 0;
static int g_used_listpr = 0;

static void need_pr(Ty t) {
    for (int i=0;i<g_nprneed;i++) if (g_prneed[i]==t) return;
    if (g_nprneed >= 256) { fprintf(stderr,"compilel: too many list types\n"); exit(1); }
    g_prneed[g_nprneed++] = t;
    g_used_listrt = 1;
    g_used_listpr = 1;
    Ty e = elem_of(t);
    if (is_list(e)) need_pr(e);
    if (e == T_NUM) g_used_numstr = 1;   /* the same "%lld or %g?" choice */
}

/* --- expressions ------------------------------------------------ */

/* Call a builtin b(a0, a1, ...) IN AN EXPRESSION through the runtime.c
 * bridge (see runtime.c's zlx_ block). The Value struct never crosses the
 * IR boundary: the arguments are boxed into stack-allocated 48-byte slots,
 * @zlx_call runs the boxed builtin() on them, and the result is unboxed by
 * `rt` (from builtin_bridge_ty). The interpreter builds its whole args
 * array before dispatch, so every argument is evaluated first here too. */
static void emit_builtin_call(Node *n, char *ref, Ty rt) {
    int nargs = n->nkids;
    if (nargs > 16) { fprintf(stderr,"compilel: too many arguments\n"); exit(1); }

    char aref[16][REFLEN]; Ty aty[16];
    for (int i=0;i<nargs;i++) aty[i] = emit_expr(n->kids[i], aref[i]);
    for (int i=0;i<nargs;i++) {
        if (is_list(aty[i])) {
            /* A list argument is boxed element-by-element below; only a
             * list of scalars can be, because each element is copied
             * through zlx_list_set_num/_str. A list of lists would need
             * the box itself converted recursively - not done yet. */
            if (is_list(elem_of(aty[i]))) {
                fprintf(stderr, "compilel: passing a list OF LISTS to a builtin "
                                "is not supported yet\n");
                exit(1);
            }
            continue;
        }
        /* a number goes in as a double (zlx_num's parameter); a string goes
         * in as its pointer (zlx_str). coerce widens INT/NONE -> double. */
        if (is_numlike(aty[i])) coerce(aref[i], aty[i], T_NUM);
    }

    g_used_bridge = 1;
    int bi = bname_ref(n->a->text);

    /* Reclaim the arg/result slots on the way out, so a builtin called
     * inside a loop does not grow the stack every iteration - clang does not
     * hoist a non-entry-block alloca of its own accord. The unboxed result
     * is a value COPY (a double, or a ptr into heap that zl_str/to_string
     * malloc'd), so it outlives the restore below. */
    int sp = newtmp();
    fprintf(out, "  %%t%d = call ptr @llvm.stacksave.p0()\n", sp);

    /* the args array: nargs contiguous 48-byte Value slots, 8-aligned */
    int A = newtmp();
    fprintf(out, "  %%t%d = alloca [%d x i8], align 8\n", A, nargs*VALSZ);
    for (int i=0;i<nargs;i++) {
        int s = newtmp();
        fprintf(out, "  %%t%d = getelementptr inbounds i8, ptr %%t%d, i64 %d\n", s, A, i*VALSZ);
        if (is_list(aty[i])) {
            /* copy the zlist into a real V_LIST, one element at a time:
             *   n = lst->nitems;  zlx_list_new(slot, n)
             *   for (k = 0; k < n; k++)
             *       box = lst->items[k];  set_num/_str(slot, k, *box)
             * emitted inline because the element type is static here. */
            g_used_listbox = 1;
            int id = ++lbl;
            Ty el = elem_of(aty[i]);
            int fn_ = newtmp(), nn = newtmp();
            fprintf(out, "  %%t%d = getelementptr inbounds %%zlist, ptr %s, i32 0, i32 0\n", fn_, aref[i]);
            fprintf(out, "  %%t%d = load i64, ptr %%t%d\n", nn, fn_);
            fprintf(out, "  call void @zlx_list_new(ptr %%t%d, i64 %%t%d)\n", s, nn);
            int fi = newtmp(), it = newtmp();
            fprintf(out, "  %%t%d = getelementptr inbounds %%zlist, ptr %s, i32 0, i32 3\n", fi, aref[i]);
            fprintf(out, "  %%t%d = load ptr, ptr %%t%d\n", it, fi);
            /* the counter lives in an alloca rather than a phi: a phi needs
             * the name of the block we arrived from, and this is emitted in
             * the middle of an expression where that is not tracked. mem2reg
             * turns it back into a register anyway. */
            int ctr = newtmp();
            fprintf(out, "  %%t%d = alloca i64, align 8\n", ctr);
            fprintf(out, "  store i64 0, ptr %%t%d\n", ctr);
            fprintf(out, "  br label %%lb_head_%d\n", id);
            fprintf(out, "lb_head_%d:\n", id);
            int k = newtmp(), cmp = newtmp();
            fprintf(out, "  %%t%d = load i64, ptr %%t%d\n", k, ctr);
            fprintf(out, "  %%t%d = icmp slt i64 %%t%d, %%t%d\n", cmp, k, nn);
            fprintf(out, "  br i1 %%t%d, label %%lb_body_%d, label %%lb_end_%d\n", cmp, id, id);
            fprintf(out, "lb_body_%d:\n", id);
            int bp = newtmp(), bx = newtmp(), vv = newtmp();
            fprintf(out, "  %%t%d = getelementptr inbounds ptr, ptr %%t%d, i64 %%t%d\n", bp, it, k);
            fprintf(out, "  %%t%d = load ptr, ptr %%t%d\n", bx, bp);
            if (is_str(el)) {
                fprintf(out, "  %%t%d = load ptr, ptr %%t%d\n", vv, bx);
                fprintf(out, "  call void @zlx_list_set_str(ptr %%t%d, i64 %%t%d, ptr %%t%d)\n", s, k, vv);
            } else {
                fprintf(out, "  %%t%d = load double, ptr %%t%d\n", vv, bx);
                fprintf(out, "  call void @zlx_list_set_num(ptr %%t%d, i64 %%t%d, double %%t%d)\n", s, k, vv);
            }
            int nk = newtmp();
            fprintf(out, "  %%t%d = add i64 %%t%d, 1\n", nk, k);
            fprintf(out, "  store i64 %%t%d, ptr %%t%d\n", nk, ctr);
            fprintf(out, "  br label %%lb_head_%d\n", id);
            fprintf(out, "lb_end_%d:\n", id);
        } else if (is_str(aty[i]))
            fprintf(out, "  call void @zlx_str(ptr %%t%d, ptr %s)\n", s, aref[i]);
        else
            fprintf(out, "  call void @zlx_num(ptr %%t%d, double %s)\n", s, aref[i]);
    }

    int R = newtmp();
    fprintf(out, "  %%t%d = alloca [%d x i8], align 8\n", R, VALSZ);
    fprintf(out, "  call void @zlx_call(ptr %%t%d, ptr @.bname.%d, ptr %%t%d, i32 %d)\n",
            R, bi, A, nargs);

    int v = newtmp();
    if (rt == T_STR) {
        fprintf(out, "  %%t%d = call ptr @zlx_as_str(ptr %%t%d)\n", v, R);
    } else if (rt == T_BOOL) {
        /* a bool is machined as an i64 0/1, but it crosses the bridge in a
         * Value whose .num is 0 or 1 - so unbox as a double and narrow.
         * The value is exactly 0.0 or 1.0, so the conversion is exact. */
        int d = newtmp();
        fprintf(out, "  %%t%d = call double @zlx_as_num(ptr %%t%d)\n", d, R);
        fprintf(out, "  %%t%d = fptosi double %%t%d to i64\n", v, d);
    } else {
        fprintf(out, "  %%t%d = call double @zlx_as_num(ptr %%t%d)\n", v, R);
    }
    fprintf(out, "  call void @llvm.stackrestore.p0(ptr %%t%d)\n", sp);
    sprintf(ref, "%%t%d", v);
}

/* emit the instructions for an expression; write its value reference
 * (e.g. "%t5" or a literal "42") into `ref` and return its type. */
static Ty emit_expr(Node *n, char *ref) {
    switch (n->type) {
        case N_NUMBER:
            /* the lexer's float form is digits '.' digits, with no
             * exponent, so a '.' is the whole test. The value goes out
             * as the IEEE754 bit pattern: exact, and never at the mercy
             * of how many digits we chose to print. */
            if (strchr(n->text,'.')) {
                union { double d; unsigned long long u; } bits;
                bits.d = strtod(n->text, 0);
                snprintf(ref, REFLEN, "0x%016llX", bits.u);
                return T_NUM;
            }
            sprintf(ref, "%lld", (long long)atoll(n->text));
            return T_INT;
        case N_BOOL:
            sprintf(ref, "%d", strcmp(n->text,"true")==0);
            return T_BOOL;
        case N_STRING: {
            /* the literal already lives in a private constant; its
             * address IS the value. The lexer resolved the escapes, so
             * n->text holds the bytes the program meant. */
            int i = str_index(n);
            if (i < 0) { fprintf(stderr,"compilel: too many string literals\n"); exit(1); }
            snprintf(ref, REFLEN, "@.str.%d", i);
            return T_STR;
        }
        case N_LIST: {
            /* interp.c's N_LIST: allocate the array first, then evaluate
             * the elements left to right and store each one. Same order,
             * so an element with a side effect happens at the same point. */
            Ty lt = expr_ty(n, g_curfn), el = elem_of(lt);
            g_used_listrt = 1;
            int L = newtmp();
            fprintf(out, "  %%t%d = call ptr @zl_list_new(i64 %d)\n", L, n->nkids);
            for (int i=0;i<n->nkids;i++) {
                char v[REFLEN]; Ty tv = emit_expr(n->kids[i], v);
                coerce(v, tv, el);
                emit_box(v, el);
                int s = newtmp();
                fprintf(out, "  %%t%d = call ptr @zl_slot(ptr %%t%d, i64 %d)\n", s, L, i);
                fprintf(out, "  store ptr %s, ptr %%t%d\n", v, s);
            }
            sprintf(ref, "%%t%d", L);
            return lt;
        }
        case N_INDEX: {
            char l[REFLEN]; Ty tl = emit_expr(n->a, l);
            if (!is_list(tl)) {
                fprintf(stderr, "compilel: only lists can be indexed\n"); exit(1);
            }
            char i[REFLEN]; Ty ti = emit_expr(n->b, i);
            to_index(i, ti);
            g_used_listrt = 1;
            int b = newtmp();
            fprintf(out, "  %%t%d = call ptr @zl_index(ptr %s, i64 %s, ptr @.msgread)\n", b, l, i);
            Ty el = elem_of(tl);
            int v = newtmp();
            fprintf(out, "  %%t%d = load %s, ptr %%t%d\n", v, llty(el), b);
            sprintf(ref, "%%t%d", v);
            return el;
        }
        case N_IDENT: {
            char slot[REFLEN]; var_slot(n->text, slot);
            Ty t = var_ty(g_curfn, n->text);
            int r = newtmp();
            fprintf(out, "  %%t%d = load %s, ptr %s\n", r, llty(t), slot);
            sprintf(ref, "%%t%d", r);
            return t;
        }
        case N_TERNARY: {
            /* cond ? then : else - ONLY the taken branch runs, exactly
             * like interp.c's N_TERNARY (the untaken side may be a divide
             * by zero or a side effect you must not reach). The result is
             * the JOIN of the two branch types; each branch is coerced up
             * to it. The extra tj/ej blocks give the phi KNOWN predecessor
             * labels even when a branch expression ends in some other block
             * (a nested ternary, or an and/or) - the same device the
             * short-circuit and/or above uses. */
            Ty rt = expr_ty(n, g_curfn);
            int id = ++lbl;
            char c[REFLEN]; Ty tc = emit_expr(n->a, c);
            int cc = emit_truth(c, tc);
            fprintf(out, "  br i1 %%t%d, label %%tern_then_%d, label %%tern_else_%d\n", cc, id, id);

            fprintf(out, "tern_then_%d:\n", id);
            char tb[REFLEN]; Ty tt = emit_expr(n->b, tb);
            coerce(tb, tt, rt);
            fprintf(out, "  br label %%tern_tj_%d\n", id);
            fprintf(out, "tern_tj_%d:\n", id);
            fprintf(out, "  br label %%tern_end_%d\n", id);

            fprintf(out, "tern_else_%d:\n", id);
            char eb[REFLEN]; Ty et = emit_expr(n->c, eb);
            coerce(eb, et, rt);
            fprintf(out, "  br label %%tern_ej_%d\n", id);
            fprintf(out, "tern_ej_%d:\n", id);
            fprintf(out, "  br label %%tern_end_%d\n", id);

            fprintf(out, "tern_end_%d:\n", id);
            int r = newtmp();
            fprintf(out, "  %%t%d = phi %s [ %s, %%tern_tj_%d ], [ %s, %%tern_ej_%d ]\n",
                    r, llty(rt), tb, id, eb, id);
            sprintf(ref, "%%t%d", r);
            return rt;
        }
        case N_DANGER:
            /* the '!' "i mean it" marker has no runtime effect - interp.c's
             * N_DANGER arm just evaluates what it wraps, so we do too. */
            return emit_expr(n->a, ref);
        case N_MEMBER:
            /* interp.c refuses this outright ("member access (.) isn't
             * supported yet"); records are a separate designed feature
             * (docs/design/design_records.md). Refuse in the same words. */
            fprintf(stderr, "compilel: member access (.) isn't supported yet\n");
            exit(1);
        case N_UNARY: {
            char a[REFLEN]; Ty ta = emit_expr(n->a, a);
            if (!strcmp(n->text,"-")) {
                if (is_str(ta) || is_list(ta)) {   /* interp: "cannot negate a non-number" */
                    fprintf(stderr, "compilel: cannot negate a %s\n",
                            is_str(ta) ? "string" : "list"); exit(1);
                }
                int t = newtmp();
                if (ta == T_NUM) fprintf(out, "  %%t%d = fneg double %s\n", t, a);
                else             fprintf(out, "  %%t%d = sub i64 0, %s\n", t, a);
                sprintf(ref, "%%t%d", t);
                return ta;
            }
            /* not: 1 when the operand is falsy */
            int c = emit_truth(a, ta);
            int x = newtmp(); fprintf(out, "  %%t%d = xor i1 %%t%d, true\n", x, c);
            int t = newtmp(); fprintf(out, "  %%t%d = zext i1 %%t%d to i64\n", t, x);
            sprintf(ref, "%%t%d", t);
            return T_BOOL;
        }
        case N_BINARY: {
            const char *op = n->text;

            /* `and` and `or` SHORT-CIRCUIT, so they have to be handled
             * BEFORE the operands are emitted - the interpreter, the C
             * backend and the native backend all stop at the left
             * operand when it has already decided the answer, and this
             * one used to emit both sides and then `and`/`or` the two
             * i1s, which ran the right side's calls (and its traps, like
             * the `x != 0 and 100/x` guard) anyway.
             *
             * The extra sc_join block exists so the phi's second
             * predecessor label is KNOWN: the right operand may itself
             * be an and/or and so may end in a block other than sc_rhs.
             * clang folds both trivial blocks away. */
            if (!strcmp(op,"and") || !strcmp(op,"or")) {
                int is_and = !strcmp(op,"and");
                int id = ++lbl;
                char a[REFLEN]; Ty ta = emit_expr(n->a, a);
                int ca = emit_truth(a, ta);
                if (is_and)
                    fprintf(out, "  br i1 %%t%d, label %%sc_rhs_%d, label %%sc_short_%d\n", ca, id, id);
                else
                    fprintf(out, "  br i1 %%t%d, label %%sc_short_%d, label %%sc_rhs_%d\n", ca, id, id);

                fprintf(out, "sc_short_%d:\n", id);
                fprintf(out, "  br label %%sc_end_%d\n", id);

                fprintf(out, "sc_rhs_%d:\n", id);
                char b[REFLEN]; Ty tb = emit_expr(n->b, b);
                int cb = emit_truth(b, tb);
                int rb = newtmp();
                fprintf(out, "  %%t%d = zext i1 %%t%d to i64\n", rb, cb);
                fprintf(out, "  br label %%sc_join_%d\n", id);

                fprintf(out, "sc_join_%d:\n", id);
                fprintf(out, "  br label %%sc_end_%d\n", id);

                fprintf(out, "sc_end_%d:\n", id);
                int t = newtmp();
                fprintf(out, "  %%t%d = phi i64 [ %d, %%sc_short_%d ], [ %%t%d, %%sc_join_%d ]\n",
                        t, is_and ? 0 : 1, id, rb, id);
                sprintf(ref, "%%t%d", t);
                return T_BOOL;
            }

            char a[REFLEN], b[REFLEN];
            Ty ta = emit_expr(n->a, a);
            Ty tb = emit_expr(n->b, b);

            /* --- a list on either side -----------------------------
             * values_equal's V_LIST arm is the only operator zl gives a
             * list besides `+`: same length, the same items array, or
             * element by element. A list against a NON-list is never
             * equal (values_equal opens on the type tags), and that is
             * static here, so it folds. */
            if (is_list(ta) || is_list(tb)) {
                int eq = !strcmp(op,"=="), ne = !strcmp(op,"!=");
                if ((eq || ne) && (!is_list(ta) || !is_list(tb))) {
                    sprintf(ref, "%d", ne); return T_INT;
                }
                if (eq || ne) {
                    /* An empty literal is a list of NONE and has no boxes,
                     * so it can be walked by any element type's comparison;
                     * that is what list_fits says. Two lists of genuinely
                     * different things could only ever be equal when both
                     * are empty, which is not worth a special case. */
                    Ty c = list_fits(ta,tb) ? tb : list_fits(tb,ta) ? ta : -1;
                    if (c < 0) {
                        fprintf(stderr, "compilel: comparing lists of two different "
                                        "kinds of thing is not supported yet\n");
                        exit(1);
                    }
                    need_eq(c);
                    int r = newtmp();
                    fprintf(out, "  %%t%d = call i64 @zl_eq_%d(ptr %s, ptr %s)\n",
                            r, c - T_LIST0, a, b);
                    if (eq) { sprintf(ref, "%%t%d", r); return T_INT; }
                    int z = newtmp();
                    fprintf(out, "  %%t%d = xor i64 %%t%d, 1\n", z, r);
                    sprintf(ref, "%%t%d", z);
                    return T_INT;
                }
                fprintf(stderr, "compilel: '%s' on a list is not supported yet\n", op);
                exit(1);
            }

            /* --- a string on either side ---------------------------
             * Both operands have already been emitted, which is what
             * the interpreter does too: eval_binary is handed two
             * finished values, so the side effects of BOTH happen even
             * when the answer turns out to be a constant. */
            if (is_str(ta) || is_str(tb)) {
                int eq = !strcmp(op,"=="), ne = !strcmp(op,"!=");
                if (eq || ne) {
                    /* values_equal opens with `if (l.type != r.type) return 0`.
                     * The types are static here, so a string against a
                     * number folds to false (or true for !=) with no
                     * comparison at all - which is the answer, not a
                     * shortcut: "5" == 5 is false in zl. */
                    if (ta != tb) { sprintf(ref, "%d", ne); return T_BOOL; }
                    g_used_strcmp = 1;
                    int c = newtmp();
                    fprintf(out, "  %%t%d = call i32 @strcmp(ptr %s, ptr %s)\n", c, a, b);
                    int p = newtmp();
                    fprintf(out, "  %%t%d = icmp %s i32 %%t%d, 0\n", p, eq ? "eq" : "ne", c);
                    int t = newtmp();
                    fprintf(out, "  %%t%d = zext i1 %%t%d to i64\n", t, p);
                    sprintf(ref, "%%t%d", t);
                    return T_BOOL;
                }
                if (!strcmp(op,"+")) {
                    /* eval_plus's last arm: value_to_string both sides,
                     * then join. See the KNOWN GAP note in the header
                     * for why a boolean operand is refused instead. */
                    if (is_boolish(n->a) || is_boolish(n->b)) {
                        fprintf(stderr, "compilel: joining a boolean onto a string needs the "
                                        "boolean type this backend does not have yet\n");
                        exit(1);
                    }
                    to_text(a, ta); to_text(b, tb);
                    g_used_concat = 1;
                    int t = newtmp();
                    fprintf(out, "  %%t%d = call ptr @zl_concat(ptr %s, ptr %s)\n", t, a, b);
                    sprintf(ref, "%%t%d", t);
                    return T_STR;
                }
                /* everything else is numbers-only in eval_binary, which
                 * answers a string with runtime_error("this operator
                 * needs numbers"). Refuse rather than invent a meaning. */
                fprintf(stderr, "compilel: '%s' needs numbers, not a string\n", op);
                exit(1);
            }

            Ty wide = (ta==T_NUM || tb==T_NUM) ? T_NUM : T_INT;

            if (!strcmp(op,"/")) {
                /* Must reach the SAME verdict expr_ty did, or the store
                 * this feeds gets a value of the wrong LLVM type. */
                if (ta==T_INT && tb==T_INT && is_exact_div(n)) {
                    int t = newtmp();
                    fprintf(out, "  %%t%d = sdiv i64 %s, %s\n", t, a, b);
                    sprintf(ref, "%%t%d", t);
                    return T_INT;
                }
                /* zl's / is REAL division: 7 / 3 is 2.33333, never 2 */
                coerce(a, ta, T_NUM); coerce(b, tb, T_NUM);
                int t = newtmp();
                fprintf(out, "  %%t%d = fdiv double %s, %s\n", t, a, b);
                sprintf(ref, "%%t%d", t);
                return T_NUM;
            }
            if (!strcmp(op,"%")) {
                coerce(a, ta, wide); coerce(b, tb, wide);
                int t = newtmp();
                if (wide == T_NUM) {
                    g_used_mod = 1;
                    fprintf(out, "  %%t%d = call double @zl_mod(double %s, double %s)\n", t, a, b);
                } else
                    fprintf(out, "  %%t%d = srem i64 %s, %s\n", t, a, b);
                sprintf(ref, "%%t%d", t);
                return wide;
            }
            const char *ar = 0, *fr = 0;
            if      (!strcmp(op,"+")) { ar="add"; fr="fadd"; }
            else if (!strcmp(op,"-")) { ar="sub"; fr="fsub"; }
            else if (!strcmp(op,"*")) { ar="mul"; fr="fmul"; }
            if (ar) {
                coerce(a, ta, wide); coerce(b, tb, wide);
                int t = newtmp();
                fprintf(out, "  %%t%d = %s %s %s, %s\n", t,
                        wide==T_NUM ? fr : ar, llty(wide), a, b);
                sprintf(ref, "%%t%d", t);
                return wide;
            }
            /* comparisons. The float predicates are the ORDERED ones
             * except for !=, because C's != is true when either side is
             * NaN and `une` is the predicate that says so. */
            const char *cc = 0, *fc = 0;
            if      (!strcmp(op,"==")) { cc="eq";  fc="oeq"; }
            else if (!strcmp(op,"!=")) { cc="ne";  fc="une"; }
            else if (!strcmp(op,"<"))  { cc="slt"; fc="olt"; }
            else if (!strcmp(op,">"))  { cc="sgt"; fc="ogt"; }
            else if (!strcmp(op,"<=")) { cc="sle"; fc="ole"; }
            else if (!strcmp(op,">=")) { cc="sge"; fc="oge"; }
            if (!cc) { fprintf(stderr,"compilel: unknown op %s\n", op); exit(1); }
            coerce(a, ta, wide); coerce(b, tb, wide);
            int c = newtmp();
            fprintf(out, "  %%t%d = %s %s %s %s, %s\n", c,
                    wide==T_NUM ? "fcmp" : "icmp", wide==T_NUM ? fc : cc,
                    llty(wide), a, b);
            int t = newtmp(); fprintf(out,"  %%t%d = zext i1 %%t%d to i64\n", t, c);
            sprintf(ref,"%%t%d",t);
            return T_BOOL;
        }
        case N_CALL: {
            /* str(x) -> x as text, exactly value_to_string. Reuses to_text,
             * the same value->string path `+` uses when it joins a string
             * with a number, so str(v) and ("" + v) agree. (A list argument
             * is not supported yet - to_text errors on it, matching + .) */
            if (n->a->type == N_IDENT && !set_has(&g_fns, n->a->text) &&
                !strcmp(n->a->text, "str")) {
                if (n->nkids != 1) { fprintf(stderr,"compilel: str takes one argument\n"); exit(1); }
                Ty ta = emit_expr(n->kids[0], ref);
                to_text(ref, ta);
                return T_STR;
            }
            /* len(s). Checked AFTER g_fns because eval_call resolves a
             * user-defined function first, so `fn len(...)` shadows the
             * builtin in the interpreter and must here too. */
            if (n->a->type == N_IDENT && !set_has(&g_fns, n->a->text) &&
                !strcmp(n->a->text, "len")) {
                if (n->nkids != 1) { fprintf(stderr,"compilel: len takes one argument\n"); exit(1); }
                char a[REFLEN]; Ty ta = emit_expr(n->kids[0], a);
                /* the len builtin: strlen for a string, nitems for a list */
                if (is_list(ta)) {
                    int f = newtmp();
                    fprintf(out, "  %%t%d = getelementptr inbounds %%zlist, ptr %s, i32 0, i32 0\n", f, a);
                    int t = newtmp();
                    fprintf(out, "  %%t%d = load i64, ptr %%t%d\n", t, f);
                    sprintf(ref, "%%t%d", t);
                    return T_INT;
                }
                if (!is_str(ta)) {
                    fprintf(stderr, "compilel: len needs a string or a list\n");
                    exit(1);
                }
                g_used_strlen = 1;
                int t = newtmp();
                fprintf(out, "  %%t%d = call i64 @strlen(ptr %s)\n", t, a);
                sprintf(ref, "%%t%d", t);
                return T_INT;
            }
            /* push(list, item) -> a NEW list with item on the end. The
             * tip tracking that makes it amortized O(1) without letting
             * two pushes off one base share a slot lives in @zl_push;
             * here we only have to box the item at the RESULT list's
             * element type, since that is the type the boxes hold. */
            if (n->a->type == N_IDENT && !set_has(&g_fns, n->a->text) &&
                !strcmp(n->a->text, "push")) {
                if (n->nkids != 2) { fprintf(stderr,"compilel: push takes a list and an item\n"); exit(1); }
                Ty rt = expr_ty(n, g_curfn);
                if (!is_list(rt)) { fprintf(stderr,"compilel: push needs a list\n"); exit(1); }
                char a[REFLEN]; Ty ta = emit_expr(n->kids[0], a);
                if (!is_list(ta)) { fprintf(stderr,"compilel: push needs a list\n"); exit(1); }
                char v[REFLEN]; Ty tv = emit_expr(n->kids[1], v);
                Ty el = elem_of(rt);
                coerce(v, tv, el);
                emit_box(v, el);
                g_used_listrt = 1;
                int t = newtmp();
                fprintf(out, "  %%t%d = call ptr @zl_push(ptr %s, ptr %s)\n", t, a, v);
                sprintf(ref, "%%t%d", t);
                return rt;
            }
            /* a scalar builtin, called through the runtime.c bridge. After
             * str/len/push (special-cased above) and before user functions,
             * matching eval_call: a user-defined fn of the same name wins. */
            {
                Ty brt;
                if (n->a->type == N_IDENT && !set_has(&g_fns, n->a->text) &&
                    builtin_bridge_ty(n->a->text, &brt)) {
                    emit_builtin_call(n, ref, brt);
                    return brt;
                }
            }
            if (n->a->type != N_IDENT || !set_has(&g_fns, n->a->text)) {
                fprintf(stderr, "compilel: only user-defined functions can be called in an expression\n");
                exit(1);
            }
            if (n->nkids > 16) { fprintf(stderr,"compilel: too many arguments\n"); exit(1); }
            FnInfo *callee = g_fninfo[set_index(&g_fns, n->a->text)];
            char argv_[16][REFLEN]; Ty at[16];
            for (int i=0;i<n->nkids;i++) at[i] = emit_expr(n->kids[i], argv_[i]);
            for (int i=0;i<n->nkids;i++) coerce(argv_[i], at[i], param_ty(callee,i));
            int t = newtmp();
            fprintf(out, "  %%t%d = call %s @fn_%s(", t, llty(callee->ret), n->a->text);
            for (int i=0;i<n->nkids;i++)
                fprintf(out, "%s%s %s", i?", ":"", llty(param_ty(callee,i)), argv_[i]);
            fprintf(out, ")\n");
            sprintf(ref, "%%t%d", t);
            return callee->ret;
        }
        default:
            fprintf(stderr, "compilel: expression not supported yet (type %d)\n", n->type);
            exit(1);   /* noreturn, so every path above really does return a Ty */
    }
}

/* --- statements -------------------------------------------------- */

static void emit_print(Node *call) {
    if (call->nkids != 1) {
        fprintf(stderr, "compilel: print takes exactly one argument here\n");
        exit(1);
    }
    Node *arg = call->kids[0];
    char v[REFLEN]; Ty t = emit_expr(arg, v);
    if (is_list(t)) {
        /* value_to_string's V_LIST arm: "[a, b, c]", then print's newline */
        need_pr(t);
        fprintf(out, "  call void @zl_pr_%d(ptr %s)\n", t - T_LIST0, v);
        int r = newtmp();
        fprintf(out, "  %%t%d = call i32 (ptr, ...) @printf(ptr @.pnl)\n", r);
    } else if (t == T_STR) {
        /* value_to_string's V_STR arm is the string itself */
        int r = newtmp();   /* capture printf's ignored result (named) */
        fprintf(out, "  %%t%d = call i32 (ptr, ...) @printf(ptr @.fmts, ptr %s)\n", r, v);
    } else if (t == T_BOOL) {
        /* value_to_string's V_BOOL arm: the words true/false, not 0/1.
         * Without this the SPEED backend disagrees with the reference on
         * something as ordinary as print(1 > 0). */
        g_used_printbool = 1;
        int c = newtmp(), sel = newtmp(), r = newtmp();
        fprintf(out, "  %%t%d = icmp ne i64 %s, 0\n", c, v);
        fprintf(out, "  %%t%d = select i1 %%t%d, ptr @.strtrue, ptr @.strfalse\n", sel, c);
        fprintf(out, "  %%t%d = call i32 (ptr, ...) @printf(ptr @.fmts, ptr %%t%d)\n", r, sel);
    } else if (t == T_NUM) {
        /* an INT is a whole number by construction, so only a double
         * needs the runtime "%lld or %g?" choice interp.c makes */
        g_used_printnum = 1;
        fprintf(out, "  call void @zl_printnum(double %s)\n", v);
    } else {
        int r = newtmp();
        fprintf(out, "  %%t%d = call i32 (ptr, ...) @printf(ptr @.fmt, i64 %s)\n", r, v);
    }
}

static void emit_if(Node *n) {
    int id = ++lbl;
    char c[REFLEN]; Ty tc = emit_expr(n->a, c);
    int t = emit_truth(c, tc);
    if (n->c) fprintf(out, "  br i1 %%t%d, label %%if_then_%d, label %%if_else_%d\n", t, id, id);
    else      fprintf(out, "  br i1 %%t%d, label %%if_then_%d, label %%if_end_%d\n",  t, id, id);

    fprintf(out, "if_then_%d:\n", id); terminated = 0;
    emit_block(n->b);
    if (!terminated) fprintf(out, "  br label %%if_end_%d\n", id);

    if (n->c) {
        fprintf(out, "if_else_%d:\n", id); terminated = 0;
        emit_block(n->c);
        if (!terminated) fprintf(out, "  br label %%if_end_%d\n", id);
    }
    /* the join block always exists so nothing dangles; if it turns out to
     * be unreachable, the enclosing fallback 'ret' still terminates it. */
    fprintf(out, "if_end_%d:\n", id); terminated = 0;
}

static void emit_while(Node *n) {
    int id = ++lbl;
    fprintf(out, "  br label %%loop_cond_%d\n", id);

    fprintf(out, "loop_cond_%d:\n", id); terminated = 0;
    char c[REFLEN]; Ty tc = emit_expr(n->a, c);
    int t = emit_truth(c, tc);
    fprintf(out, "  br i1 %%t%d, label %%loop_body_%d, label %%loop_end_%d\n", t, id, id);

    fprintf(out, "loop_body_%d:\n", id); terminated = 0;
    if (nloops >= 64) { fprintf(stderr,"compilel: loops nested too deep\n"); exit(1); }
    loopcont[nloops] = "loop_cond";        /* continue re-tests the condition */
    loopids[nloops++] = id;
    emit_block(n->b);
    nloops--;
    if (!terminated) fprintf(out, "  br label %%loop_cond_%d\n", id);

    fprintf(out, "loop_end_%d:\n", id); terminated = 0;
}

/* `for var in seq { body }`, exec's N_FOR arm: seq must be a list; bind
 * var to each element in turn and run the body. nitems and the items
 * array are read ONCE, before the loop - interp.c snapshots them into its
 * `Value seq` copy, so a body that reassigns the source variable or
 * pushes onto the list cannot change what this loop iterates.
 *
 * The index is a phi, not an alloca: an alloca here would sit in whatever
 * block precedes the loop and, when this for is nested in another loop,
 * re-run every outer iteration and grow the stack. The phi has exactly
 * two predecessors - loop_pre (index 0) and loop_step (index+1) - because
 * 'continue' is routed to loop_step, not into the phi. */
static void emit_for(Node *n) {
    char s[REFLEN]; Ty ts = emit_expr(n->a, s);
    if (!is_list(ts)) {
        fprintf(stderr, "compilel: 'for' can only loop over a list\n"); exit(1);
    }
    Ty el = elem_of(ts);
    int id = ++lbl;

    /* a named predecessor block for the phi, holding the one-time snapshot */
    fprintf(out, "  br label %%loop_pre_%d\n", id);
    fprintf(out, "loop_pre_%d:\n", id); terminated = 0;
    int f0 = newtmp();
    fprintf(out, "  %%t%d = getelementptr inbounds %%zlist, ptr %s, i32 0, i32 0\n", f0, s);
    int nit = newtmp();
    fprintf(out, "  %%t%d = load i64, ptr %%t%d\n", nit, f0);
    int f3 = newtmp();
    fprintf(out, "  %%t%d = getelementptr inbounds %%zlist, ptr %s, i32 0, i32 3\n", f3, s);
    int arr = newtmp();
    fprintf(out, "  %%t%d = load ptr, ptr %%t%d\n", arr, f3);
    fprintf(out, "  br label %%loop_cond_%d\n", id);

    /* iphi is the current index; inext (defined later, in loop_step) is
     * the phi's back-edge value. A forward reference to a NAMED temp is
     * legal - only unnamed %N values must be numbered in order. */
    int iphi  = newtmp();
    int inext = newtmp();
    fprintf(out, "loop_cond_%d:\n", id); terminated = 0;
    fprintf(out, "  %%t%d = phi i64 [ 0, %%loop_pre_%d ], [ %%t%d, %%loop_step_%d ]\n",
            iphi, id, inext, id);
    int more = newtmp();
    fprintf(out, "  %%t%d = icmp slt i64 %%t%d, %%t%d\n", more, iphi, nit);
    fprintf(out, "  br i1 %%t%d, label %%loop_body_%d, label %%loop_end_%d\n", more, id, id);

    fprintf(out, "loop_body_%d:\n", id); terminated = 0;
    int slot = newtmp();
    fprintf(out, "  %%t%d = getelementptr inbounds ptr, ptr %%t%d, i64 %%t%d\n", slot, arr, iphi);
    int box = newtmp();
    fprintf(out, "  %%t%d = load ptr, ptr %%t%d\n", box, slot);
    int val = newtmp();
    fprintf(out, "  %%t%d = load %s, ptr %%t%d\n", val, llty(el), box);
    char v[REFLEN]; snprintf(v, REFLEN, "%%t%d", val);
    Ty lvt = var_ty(g_curfn, n->text);
    coerce(v, el, lvt);                     /* boxes hold el; the var may be wider */
    char lslot[REFLEN]; var_slot(n->text, lslot);
    fprintf(out, "  store %s %s, ptr %s\n", llty(lvt), v, lslot);

    if (nloops >= 64) { fprintf(stderr,"compilel: loops nested too deep\n"); exit(1); }
    loopcont[nloops] = "loop_step";         /* continue must advance the index */
    loopids[nloops++] = id;
    emit_block(n->b);
    nloops--;
    if (!terminated) fprintf(out, "  br label %%loop_step_%d\n", id);

    fprintf(out, "loop_step_%d:\n", id); terminated = 0;
    fprintf(out, "  %%t%d = add i64 %%t%d, 1\n", inext, iphi);
    fprintf(out, "  br label %%loop_cond_%d\n", id);

    fprintf(out, "loop_end_%d:\n", id); terminated = 0;
}

/* Apply a compound `op=`. The target's old value is in `a` (type `at`),
 * the right operand in `v` (type `tv`, its own node `rhs` so a boolean
 * can still be spotted). Leaves the result in `v` and returns its type -
 * eval_binary on the two, which is why the string arm joins rather than
 * adds. Shared by the two targets an assignment can have, a name and a
 * list element, so they cannot drift apart. */
static Ty emit_compound(const char *o, char *a, Ty at, char *v, Ty tv, Node *rhs) {
    if (is_list(at) || is_list(tv)) {
        fprintf(stderr, "compilel: '%s=' on a list is not supported yet\n", o);
        exit(1);
    }
    /* `s += 5` on a string is eval_plus again, so it joins rather
     * than adds - and it is the shape tests/test_syntax.zl pins. */
    if (is_str(at) || is_str(tv)) {
        if (strcmp(o,"+")) {
            fprintf(stderr, "compilel: '%s=' needs numbers, not a string\n", o);
            exit(1);
        }
        if (is_boolish(rhs)) {
            fprintf(stderr, "compilel: joining a boolean onto a string needs the "
                            "boolean type this backend does not have yet\n");
            exit(1);
        }
        to_text(a, at); to_text(v, tv);
        g_used_concat = 1;
        int t = newtmp();
        fprintf(out, "  %%t%d = call ptr @zl_concat(ptr %s, ptr %s)\n", t, a, v);
        snprintf(v, REFLEN, "%%t%d", t);
        return T_STR;
    }
    coerce(v, tv, at);
    int t = newtmp();
    if (!strcmp(o,"/")) {
        fprintf(out, "  %%t%d = fdiv double %s, %s\n", t, a, v);
    } else if (!strcmp(o,"%")) {
        if (at == T_NUM) { g_used_mod = 1;
            fprintf(out, "  %%t%d = call double @zl_mod(double %s, double %s)\n", t, a, v); }
        else fprintf(out, "  %%t%d = srem i64 %s, %s\n", t, a, v);
    } else {
        const char *ar = 0, *fr = 0;
        if      (!strcmp(o,"+")) { ar="add"; fr="fadd"; }
        else if (!strcmp(o,"-")) { ar="sub"; fr="fsub"; }
        else if (!strcmp(o,"*")) { ar="mul"; fr="fmul"; }
        if (!ar) { fprintf(stderr,"compilel: unknown op %s=\n", o); exit(1); }
        fprintf(out, "  %%t%d = %s %s %s, %s\n", t, at==T_NUM?fr:ar, llty(at), a, v);
    }
    snprintf(v, REFLEN, "%%t%d", t);
    return at;
}

/* `xs[i] = v`, exec's N_INDEX arm: evaluate the container, then the
 * subscript, then RANGE-CHECK, and only then run the right side. The
 * store goes through the element's BOX, so a list that shares that box
 * with another list sees the write - which is what the interpreter does,
 * since it assigns through `*list.items[i]`. */
static void emit_index_assign(Node *n) {
    char l[REFLEN]; Ty tl = emit_expr(n->a->a, l);
    if (!is_list(tl)) {
        fprintf(stderr, "compilel: can only index-assign a list\n"); exit(1);
    }
    char i[REFLEN]; Ty ti = emit_expr(n->a->b, i);
    to_index(i, ti);
    g_used_listrt = 1;
    int bx = newtmp();
    fprintf(out, "  %%t%d = call ptr @zl_index(ptr %s, i64 %s, ptr @.msgwrite)\n", bx, l, i);
    char box[REFLEN]; snprintf(box, REFLEN, "%%t%d", bx);

    Ty el = elem_of(tl);
    char v[REFLEN]; Ty tv;
    if (n->text[0]) {
        int old = newtmp();     /* read before the right side runs */
        fprintf(out, "  %%t%d = load %s, ptr %s\n", old, llty(el), box);
        char a[REFLEN]; snprintf(a, REFLEN, "%%t%d", old);
        tv = emit_expr(n->b, v);
        tv = emit_compound(n->text, a, el, v, tv, n->b);
    } else {
        tv = emit_expr(n->b, v);
    }
    coerce(v, tv, el);
    fprintf(out, "  store %s %s, ptr %s\n", llty(el), v, box);
}

static void emit_assign(Node *n) {
    if (n->a->type == N_INDEX) { emit_index_assign(n); return; }
    if (n->a->type != N_IDENT) {
        fprintf(stderr, "compilel: can only assign to a name or a list index\n");
        exit(1);
    }
    /* the variable's type is the join of everything assigned to it, so
     * it is exactly the type of the value about to be stored */
    Ty vt = var_ty(g_curfn, n->a->text);
    char v[REFLEN]; Ty tv = emit_expr(n->b, v);
    char slot[REFLEN]; var_slot(n->a->text, slot);

    /* n->text holds the operator for a compound `op=` (empty for a
     * plain `=`): load the old value and apply it here, since the
     * parser no longer desugars op= into `x = x op v`. */
    if (n->text[0]) {
        int old = newtmp();
        fprintf(out, "  %%t%d = load %s, ptr %s\n", old, llty(vt), slot);
        char a[REFLEN]; snprintf(a, REFLEN, "%%t%d", old);
        tv = emit_compound(n->text, a, vt, v, tv, n->b);
    }
    coerce(v, tv, vt);
    fprintf(out, "  store %s %s, ptr %s\n", llty(vt), v, slot);
}

static void emit_stmt(Node *n) {
    switch (n->type) {
        case N_BLOCK:  emit_block(n);  break;
        case N_ASSIGN: emit_assign(n); break;
        case N_IF:     emit_if(n);     break;
        case N_WHILE:  emit_while(n);  break;
        case N_FOR:    emit_for(n);    break;
        case N_BREAK:
            if (!nloops) { fprintf(stderr,"compilel: break outside a loop\n"); exit(1); }
            fprintf(out, "  br label %%loop_end_%d\n", loopids[nloops-1]);
            terminated = 1;
            break;
        case N_CONTINUE:
            if (!nloops) { fprintf(stderr,"compilel: continue outside a loop\n"); exit(1); }
            fprintf(out, "  br label %%%s_%d\n", loopcont[nloops-1], loopids[nloops-1]);
            terminated = 1;
            break;
        case N_RETURN: {
            if (g_curfn) {
                Ty rt = g_curfn->ret;
                if (n->a) {
                    char v[REFLEN]; Ty t = emit_expr(n->a, v);
                    coerce(v, t, rt);
                    fprintf(out, "  ret %s %s\n", llty(rt), v);
                } else fprintf(out, "  ret %s %s\n", llty(rt), zero_of(rt));
            } else {
                /* top-level return is main's exit code, so an i32 */
                if (n->a) {
                    char v[REFLEN]; Ty t = emit_expr(n->a, v);
                    coerce(v, t, T_INT);
                    int r = newtmp();
                    fprintf(out, "  %%t%d = trunc i64 %s to i32\n", r, v);
                    fprintf(out, "  ret i32 %%t%d\n", r);
                } else fprintf(out, "  ret i32 0\n");
            }
            terminated = 1;
            break;
        }
        case N_EXPRSTMT: {
            Node *e = n->a;
            if (e->type==N_CALL && e->a->type==N_IDENT && !strcmp(e->a->text,"print")) {
                emit_print(e);
                break;
            }
            char v[REFLEN]; emit_expr(e, v);   /* evaluate for effect */
            (void)v;
            break;
        }
        case N_FN:
            /* top-level functions are collected and emitted by main(); a
             * function nested inside another function or block is not
             * supported yet. interp.c allows it (it stores the fn as a
             * variable), but nested/first-class functions need a machine
             * function type this backend does not have - a separate feature. */
            fprintf(stderr, "compilel: a function defined inside another function "
                            "or block is not supported yet (only top-level fn)\n");
            exit(1);
        default:
            fprintf(stderr, "compilel: statement not supported yet (type %d)\n", n->type);
            exit(1);
    }
}

/* run a block's statements; anything after a terminator is dead code */
static void emit_block(Node *n) {
    for (int i=0;i<n->nkids;i++) {
        if (terminated) break;
        emit_stmt(n->kids[i]);
    }
}

/* --- functions ---------------------------------------------------- */

static void emit_fn(FnInfo *f) {
    Node *fn = f->node;
    g_curfn = f; terminated = 0;

    fprintf(out, "define %s @fn_%s(", llty(f->ret), fn->text);
    for (int i=0;i<fn->nkids;i++)
        fprintf(out, "%s%s %%a_p%d", i?", ":"", llty(param_ty(f,i)), i);
    fprintf(out, ") {\nentry:\n");

    /* locals live in alloca slots, never in the @v_ globals */
    for (int i=0;i<f->locals.count;i++) {
        const char *nm = f->locals.names[i];
        fprintf(out, "  %%l_%s = alloca %s\n", nm, llty(f->localty[i]));
        fprintf(out, "  store %s %s, ptr %%l_%s\n", llty(f->localty[i]), zero_of(f->localty[i]), nm);
    }
    for (int i=0;i<fn->nkids;i++)
        fprintf(out, "  store %s %%a_p%d, ptr %%l_%s\n",
                llty(param_ty(f,i)), i, fn->kids[i]->text);

    emit_block(fn->a);
    if (!terminated) fprintf(out, "  ret %s %s\n", llty(f->ret), zero_of(f->ret));
    fputs("}\n\n", out);

    g_curfn = 0;
}

/* --- driver -------------------------------------------------------- */

static void emit_str_constant(int i) {
    const char *s = g_strs[i]->text;
    size_t len = strlen(s) + 1;   /* + the NUL */
    fprintf(out, "@.str.%d = private constant [%u x i8] c\"", i, (unsigned)len);
    for (const unsigned char *p=(const unsigned char*)s; *p; p++) {
        if (*p >= 0x20 && *p < 0x7f && *p != '"' && *p != '\\') fputc(*p, out);
        else fprintf(out, "\\%02X", *p);
    }
    fputs("\\00\"\n", out);
}

/* The two runtime helpers, both straight transcriptions of interp.c.
 *
 * zl_mod    <- eval_binary's "%": an INTEGER modulo, with the two
 *              divisor cases that are hardware TRAPS rather than wrong
 *              answers (0 -> fmod's NaN, -1 -> 0) answered without
 *              dividing. The fptosi.sat form is used instead of a plain
 *              fptosi so an out-of-range double is a saturated number
 *              rather than LLVM poison.
 * zl_printnum <- value_to_string's V_NUM case: a whole number prints as
 *              "%lld" and everything else as "%g". The range test in
 *              front of the fptosi is what keeps that conversion legal;
 *              NaN and the infinities fail it and print through "%g",
 *              which is what the interpreter does too.
 *
 * They are emitted only when something actually calls them, and AFTER
 * the code that does (LLVM resolves global references across the whole
 * module, so a forward reference is fine). Emitting them always cost
 * nothing at runtime - clang drops a private function with no callers -
 * but it did change the module enough to relink the integer benchmarks
 * into different, unnecessarily-different binaries. A program with no
 * floating point in it should compile to exactly what it compiled to
 * before this file learned about floating point, and now it does.
 */
static const char *HELPER_MOD =
"define private double @zl_mod(double %a, double %b) {\n"
"entry:\n"
"  %bi = call i64 @llvm.fptosi.sat.i64.f64(double %b)\n"
"  %isz = icmp eq i64 %bi, 0\n"
"  br i1 %isz, label %byzero, label %chkneg\n"
"byzero:\n"
"  %nan = call double @fmod(double %a, double 0x0000000000000000)\n"
"  ret double %nan\n"
"chkneg:\n"
"  %isn1 = icmp eq i64 %bi, -1\n"
"  br i1 %isn1, label %negone, label %divide\n"
"negone:\n"
"  ret double 0x0000000000000000\n"
"divide:\n"
"  %ai = call i64 @llvm.fptosi.sat.i64.f64(double %a)\n"
"  %r = srem i64 %ai, %bi\n"
"  %rd = sitofp i64 %r to double\n"
"  ret double %rd\n"
"}\n\n";

static const char *HELPER_PRINTNUM =
"@.fmtg = private constant [4 x i8] c\"%g\\0A\\00\"\n"
"define private void @zl_printnum(double %v) {\n"
"entry:\n"
"  %ge = fcmp oge double %v, 0xC3E0000000000000\n"
"  %lt = fcmp olt double %v, 0x43E0000000000000\n"
"  %inrange = and i1 %ge, %lt\n"
"  br i1 %inrange, label %maybe, label %asdouble\n"
"maybe:\n"
"  %i = fptosi double %v to i64\n"
"  %back = sitofp i64 %i to double\n"
"  %whole = fcmp oeq double %back, %v\n"
"  br i1 %whole, label %asint, label %asdouble\n"
"asint:\n"
"  %p1 = call i32 (ptr, ...) @printf(ptr @.fmt, i64 %i)\n"
"  ret void\n"
"asdouble:\n"
"  %p2 = call i32 (ptr, ...) @printf(ptr @.fmtg, double %v)\n"
"  ret void\n"
"}\n\n";

/* The string helpers, again straight transcriptions of interp.c.
 *
 * zl_concat  <- eval_plus's last arm: malloc(strlen(a)+strlen(b)+1) and
 *               copy both in. The second memcpy carries the NUL. It
 *               NEVER FREES, exactly like the interpreter (make_str and
 *               eval_plus both malloc and neither frees) - see the
 *               MEMORY note in the header.
 * zl_intstr  <- value_to_string's "%lld" branch, for a value this
 *               backend already knows is whole. Deliberately NOT routed
 *               through a double: an i64 prints here the same way print
 *               already prints one, so the two agree with each other.
 * zl_numstr  <- value_to_string's V_NUM case entire, the same
 *               whole-number test @zl_printnum makes, into a buffer
 *               instead of onto stdout.
 *
 * 32 and 64 bytes are what the conversions need: "%lld" is at most 20
 * characters and interp.c's own buffer for both branches is 64.
 */
static const char *HELPER_STRFMT =
"@.fmtd  = private constant [5 x i8] c\"%lld\\00\"\n"
"declare i32 @snprintf(ptr, i64, ptr, ...)\n\n";

static const char *HELPER_CONCAT =
"define private ptr @zl_concat(ptr %a, ptr %b) {\n"
"entry:\n"
"  %la = call i64 @strlen(ptr %a)\n"
"  %lb = call i64 @strlen(ptr %b)\n"
"  %sum = add i64 %la, %lb\n"
"  %need = add i64 %sum, 1\n"
"  %p = call ptr @malloc(i64 %need)\n"
"  %c1 = call ptr @memcpy(ptr %p, ptr %a, i64 %la)\n"
"  %tail = getelementptr i8, ptr %p, i64 %la\n"
"  %lb1 = add i64 %lb, 1\n"
"  %c2 = call ptr @memcpy(ptr %tail, ptr %b, i64 %lb1)\n"
"  ret ptr %p\n"
"}\n\n";

static const char *HELPER_INTSTR =
"define private ptr @zl_intstr(i64 %v) {\n"
"entry:\n"
"  %p = call ptr @malloc(i64 32)\n"
"  %r = call i32 (ptr, i64, ptr, ...) @snprintf(ptr %p, i64 32, ptr @.fmtd, i64 %v)\n"
"  ret ptr %p\n"
"}\n\n";

static const char *HELPER_NUMSTR =
"@.fmtgb = private constant [3 x i8] c\"%g\\00\"\n"
"define private ptr @zl_numstr(double %v) {\n"
"entry:\n"
"  %p = call ptr @malloc(i64 64)\n"
"  %ge = fcmp oge double %v, 0xC3E0000000000000\n"
"  %lt = fcmp olt double %v, 0x43E0000000000000\n"
"  %inrange = and i1 %ge, %lt\n"
"  br i1 %inrange, label %maybe, label %asdouble\n"
"maybe:\n"
"  %i = fptosi double %v to i64\n"
"  %back = sitofp i64 %i to double\n"
"  %whole = fcmp oeq double %back, %v\n"
"  br i1 %whole, label %asint, label %asdouble\n"
"asint:\n"
"  %r1 = call i32 (ptr, i64, ptr, ...) @snprintf(ptr %p, i64 64, ptr @.fmtd, i64 %i)\n"
"  ret ptr %p\n"
"asdouble:\n"
"  %r2 = call i32 (ptr, i64, ptr, ...) @snprintf(ptr %p, i64 64, ptr @.fmtgb, double %v)\n"
"  ret ptr %p\n"
"}\n\n";

/* The list runtime, again straight transcriptions of interp.c.
 *
 * zl_rterror <- runtime_error: flush stdout FIRST (it is block-buffered
 *               when redirected, so without this the output that explains
 *               how we got here dies with the process), then the message
 *               on stderr, then exit(1). Byte for byte the interpreter's
 *               "runtime error: %s\n".
 * zl_list_new <- eval's N_LIST arm: nitems and cap both the element
 *               count, tip NULL. A fresh list is not the tip of anything,
 *               so the first push off it copies - which is what makes two
 *               pushes off one literal independent.
 * zl_index   <- the bounds test both N_INDEX and index-assign make. The
 *               caller passes the message so the two keep their own
 *               wording ("list index out of range" against
 *               "index-assign out of range").
 * zl_push    <- the push builtin ENTIRE, tip tracking included: append
 *               into the source array only when this list is the newest
 *               tip of it (nitems == *tip) and there is capacity, else
 *               copy the pointer array. Read the long comment on push in
 *               interp.c before changing a line of it - two earlier
 *               versions of that function were wrong in opposite
 *               directions (aliasing, then O(n^2)).
 *               The header returned is always a FRESH allocation, even on
 *               the in-place path, because interp.c returns a COPY of the
 *               struct there and the caller's own header must not move.
 */
static const char *HELPER_LISTRT =
"@.rtfmt    = private constant [19 x i8] c\"runtime error: %s\\0A\\00\"\n"
"@.msgread  = private constant [24 x i8] c\"list index out of range\\00\"\n"
"@.msgwrite = private constant [26 x i8] c\"index-assign out of range\\00\"\n"
"declare ptr @__acrt_iob_func(i32)\n"
"declare i32 @fflush(ptr)\n"
"declare i32 @fprintf(ptr, ptr, ...)\n"
"declare void @exit(i32)\n"
"define private void @zl_rterror(ptr %msg) {\n"
"entry:\n"
"  %so = call ptr @__acrt_iob_func(i32 1)\n"
"  %fl = call i32 @fflush(ptr %so)\n"
"  %se = call ptr @__acrt_iob_func(i32 2)\n"
"  %pr = call i32 (ptr, ptr, ...) @fprintf(ptr %se, ptr @.rtfmt, ptr %msg)\n"
"  call void @exit(i32 1)\n"
"  unreachable\n"
"}\n\n"
"define private ptr @zl_list_new(i64 %n) {\n"
"entry:\n"
"  %h = call ptr @malloc(i64 32)\n"
"  %any = icmp sgt i64 %n, 0\n"
"  %slots = select i1 %any, i64 %n, i64 1\n"
"  %bytes = mul i64 %slots, 8\n"
"  %arr = call ptr @malloc(i64 %bytes)\n"
"  %f0 = getelementptr inbounds %zlist, ptr %h, i32 0, i32 0\n"
"  store i64 %n, ptr %f0\n"
"  %f1 = getelementptr inbounds %zlist, ptr %h, i32 0, i32 1\n"
"  store i64 %n, ptr %f1\n"
"  %f2 = getelementptr inbounds %zlist, ptr %h, i32 0, i32 2\n"
"  store ptr null, ptr %f2\n"
"  %f3 = getelementptr inbounds %zlist, ptr %h, i32 0, i32 3\n"
"  store ptr %arr, ptr %f3\n"
"  ret ptr %h\n"
"}\n\n"
"define private ptr @zl_slot(ptr %l, i64 %i) {\n"
"entry:\n"
"  %f3 = getelementptr inbounds %zlist, ptr %l, i32 0, i32 3\n"
"  %arr = load ptr, ptr %f3\n"
"  %p = getelementptr inbounds ptr, ptr %arr, i64 %i\n"
"  ret ptr %p\n"
"}\n\n"
"define private ptr @zl_index(ptr %l, i64 %i, ptr %msg) {\n"
"entry:\n"
"  %f0 = getelementptr inbounds %zlist, ptr %l, i32 0, i32 0\n"
"  %n = load i64, ptr %f0\n"
"  %lo = icmp slt i64 %i, 0\n"
"  %hi = icmp sge i64 %i, %n\n"
"  %bad = or i1 %lo, %hi\n"
"  br i1 %bad, label %oops, label %ok\n"
"oops:\n"
"  call void @zl_rterror(ptr %msg)\n"
"  unreachable\n"
"ok:\n"
"  %s = call ptr @zl_slot(ptr %l, i64 %i)\n"
"  %b = load ptr, ptr %s\n"
"  ret ptr %b\n"
"}\n\n"
"define private ptr @zl_push(ptr %l, ptr %box) {\n"
"entry:\n"
"  %f0 = getelementptr inbounds %zlist, ptr %l, i32 0, i32 0\n"
"  %m = load i64, ptr %f0\n"
"  %f1 = getelementptr inbounds %zlist, ptr %l, i32 0, i32 1\n"
"  %cap = load i64, ptr %f1\n"
"  %f2 = getelementptr inbounds %zlist, ptr %l, i32 0, i32 2\n"
"  %tip = load ptr, ptr %f2\n"
"  %f3 = getelementptr inbounds %zlist, ptr %l, i32 0, i32 3\n"
"  %items = load ptr, ptr %f3\n"
"  %hasi = icmp ne ptr %items, null\n"
"  %hast = icmp ne ptr %tip, null\n"
"  %live = and i1 %hasi, %hast\n"
"  br i1 %live, label %chktip, label %copy\n"
"chktip:\n"
"  %tv = load i64, ptr %tip\n"
"  %newest = icmp eq i64 %tv, %m\n"
"  %room = icmp slt i64 %m, %cap\n"
"  %inplace = and i1 %newest, %room\n"
"  br i1 %inplace, label %fast, label %copy\n"
"fast:\n"
"  %fs = getelementptr inbounds ptr, ptr %items, i64 %m\n"
"  store ptr %box, ptr %fs\n"
"  %m1 = add i64 %m, 1\n"
"  store i64 %m1, ptr %tip\n"
"  %fh = call ptr @malloc(i64 32)\n"
"  %g0 = getelementptr inbounds %zlist, ptr %fh, i32 0, i32 0\n"
"  store i64 %m1, ptr %g0\n"
"  %g1 = getelementptr inbounds %zlist, ptr %fh, i32 0, i32 1\n"
"  store i64 %cap, ptr %g1\n"
"  %g2 = getelementptr inbounds %zlist, ptr %fh, i32 0, i32 2\n"
"  store ptr %tip, ptr %g2\n"
"  %g3 = getelementptr inbounds %zlist, ptr %fh, i32 0, i32 3\n"
"  store ptr %items, ptr %g3\n"
"  ret ptr %fh\n"
"copy:\n"
"  %want = add i64 %m, 1\n"
"  %huge = icmp sgt i64 %want, 1073741823\n"
"  %dbl = mul i64 %want, 2\n"
"  %nc0 = select i1 %huge, i64 %want, i64 %dbl\n"
"  %tiny = icmp slt i64 %nc0, 8\n"
"  %nc = select i1 %tiny, i64 8, i64 %nc0\n"
"  %nb = mul i64 %nc, 8\n"
"  %arr = call ptr @malloc(i64 %nb)\n"
"  %some = icmp sgt i64 %m, 0\n"
"  br i1 %some, label %docopy, label %filled\n"
"docopy:\n"
"  %mb = mul i64 %m, 8\n"
"  %cp = call ptr @memcpy(ptr %arr, ptr %items, i64 %mb)\n"
"  br label %filled\n"
"filled:\n"
"  %cs = getelementptr inbounds ptr, ptr %arr, i64 %m\n"
"  store ptr %box, ptr %cs\n"
"  %nt = call ptr @malloc(i64 8)\n"
"  store i64 %want, ptr %nt\n"
"  %ch = call ptr @malloc(i64 32)\n"
"  %h0 = getelementptr inbounds %zlist, ptr %ch, i32 0, i32 0\n"
"  store i64 %want, ptr %h0\n"
"  %h1 = getelementptr inbounds %zlist, ptr %ch, i32 0, i32 1\n"
"  store i64 %nc, ptr %h1\n"
"  %h2 = getelementptr inbounds %zlist, ptr %ch, i32 0, i32 2\n"
"  store ptr %nt, ptr %h2\n"
"  %h3 = getelementptr inbounds %zlist, ptr %ch, i32 0, i32 3\n"
"  store ptr %arr, ptr %h3\n"
"  ret ptr %ch\n"
"}\n\n";

static const char *HELPER_LISTPR =
"@.plb  = private constant [2 x i8] c\"[\\00\"\n"
"@.prb  = private constant [2 x i8] c\"]\\00\"\n"
"@.psep = private constant [3 x i8] c\", \\00\"\n"
"@.pnl  = private constant [2 x i8] c\"\\0A\\00\"\n"
"@.pfs  = private constant [3 x i8] c\"%s\\00\"\n"
"@.pfd  = private constant [5 x i8] c\"%lld\\00\"\n\n";

/* The two generated-per-type helpers. Both walk one list with the SAME
 * shape - load nitems and items once, then an index in an alloca that
 * clang's mem2reg turns back into a register - and differ only in what
 * they do with an element, which is where the element type comes in.
 * A list of lists calls its element type's helper, which need_eq /
 * need_pr already asked for. */

static void emit_eq_helper(Ty t) {
    Ty e = elem_of(t);
    const char *E = llty(e);
    fprintf(out, "define private i64 @zl_eq_%d(ptr %%a, ptr %%b) {\n", t - T_LIST0);
    fputs("entry:\n"
          "  %ip = alloca i64\n"
          "  %fa = getelementptr inbounds %zlist, ptr %a, i32 0, i32 0\n"
          "  %na = load i64, ptr %fa\n"
          "  %fb = getelementptr inbounds %zlist, ptr %b, i32 0, i32 0\n"
          "  %nb = load i64, ptr %fb\n"
          "  %samelen = icmp eq i64 %na, %nb\n"
          "  br i1 %samelen, label %ident, label %no\n"
          "ident:\n"
          "  %ja = getelementptr inbounds %zlist, ptr %a, i32 0, i32 3\n"
          "  %pa = load ptr, ptr %ja\n"
          "  %jb = getelementptr inbounds %zlist, ptr %b, i32 0, i32 3\n"
          "  %pb = load ptr, ptr %jb\n"
          "  %same = icmp eq ptr %pa, %pb\n"
          "  br i1 %same, label %yes, label %walk\n"
          "walk:\n"
          "  store i64 0, ptr %ip\n"
          "  br label %loop\n"
          "loop:\n"
          "  %i = load i64, ptr %ip\n"
          "  %more = icmp slt i64 %i, %na\n"
          "  br i1 %more, label %body, label %yes\n"
          "body:\n"
          "  %sa = getelementptr inbounds ptr, ptr %pa, i64 %i\n"
          "  %xa = load ptr, ptr %sa\n"
          "  %sb = getelementptr inbounds ptr, ptr %pb, i64 %i\n"
          "  %xb = load ptr, ptr %sb\n", out);
    fprintf(out, "  %%va = load %s, ptr %%xa\n", E);
    fprintf(out, "  %%vb = load %s, ptr %%xb\n", E);
    if (e == T_NUM)          /* values_equal: l.num == r.num, so nan != nan */
        fputs("  %eq = fcmp oeq double %va, %vb\n", out);
    else if (e == T_STR)     /* values_equal: strcmp(l.str, r.str) == 0 */
        fputs("  %c = call i32 @strcmp(ptr %va, ptr %vb)\n"
              "  %eq = icmp eq i32 %c, 0\n", out);
    else if (is_list(e)) {
        fprintf(out, "  %%c = call i64 @zl_eq_%d(ptr %%va, ptr %%vb)\n", e - T_LIST0);
        fputs("  %eq = icmp ne i64 %c, 0\n", out);
    } else                   /* T_NONE: an empty list, so never reached */
        fputs("  %eq = icmp eq i64 %va, %vb\n", out);
    fputs("  br i1 %eq, label %next, label %no\n"
          "next:\n"
          "  %i1 = add i64 %i, 1\n"
          "  store i64 %i1, ptr %ip\n"
          "  br label %loop\n"
          "yes:\n"
          "  ret i64 1\n"
          "no:\n"
          "  ret i64 0\n"
          "}\n\n", out);
}

static void emit_pr_helper(Ty t) {
    Ty e = elem_of(t);
    fprintf(out, "define private void @zl_pr_%d(ptr %%l) {\n", t - T_LIST0);
    fputs("entry:\n"
          "  %ip = alloca i64\n"
          "  store i64 0, ptr %ip\n"
          "  %f0 = getelementptr inbounds %zlist, ptr %l, i32 0, i32 0\n"
          "  %n = load i64, ptr %f0\n"
          "  %f3 = getelementptr inbounds %zlist, ptr %l, i32 0, i32 3\n"
          "  %arr = load ptr, ptr %f3\n"
          "  %o1 = call i32 (ptr, ...) @printf(ptr @.plb)\n"
          "  br label %loop\n"
          "loop:\n"
          "  %i = load i64, ptr %ip\n"
          "  %more = icmp slt i64 %i, %n\n"
          "  br i1 %more, label %body, label %done\n"
          "body:\n"
          "  %first = icmp eq i64 %i, 0\n"
          "  br i1 %first, label %item, label %sep\n"
          "sep:\n"
          "  %o2 = call i32 (ptr, ...) @printf(ptr @.psep)\n"
          "  br label %item\n"
          "item:\n"
          "  %s = getelementptr inbounds ptr, ptr %arr, i64 %i\n"
          "  %x = load ptr, ptr %s\n", out);
    fprintf(out, "  %%v = load %s, ptr %%x\n", llty(e));
    if (e == T_NUM)          /* value_to_string's V_NUM case, whole or not */
        fputs("  %vs = call ptr @zl_numstr(double %v)\n"
              "  %o3 = call i32 (ptr, ...) @printf(ptr @.pfs, ptr %vs)\n", out);
    else if (e == T_STR)     /* value_to_string's V_STR case is the string */
        fputs("  %o3 = call i32 (ptr, ...) @printf(ptr @.pfs, ptr %v)\n", out);
    else if (is_list(e))
        fprintf(out, "  call void @zl_pr_%d(ptr %%v)\n", e - T_LIST0);
    else                     /* T_NONE: an empty list, so never reached */
        fputs("  %o3 = call i32 (ptr, ...) @printf(ptr @.pfd, i64 %v)\n", out);
    fputs("  %i1 = add i64 %i, 1\n"
          "  store i64 %i1, ptr %ip\n"
          "  br label %loop\n"
          "done:\n"
          "  %o4 = call i32 (ptr, ...) @printf(ptr @.prb)\n"
          "  ret void\n"
          "}\n\n", out);
}

static void emit_helpers(void) {
    if (g_used_mod || g_used_sat)
        fputs("declare i64 @llvm.fptosi.sat.i64.f64(double)\n", out);
    if (g_used_mod) {
        fputs("declare double @fmod(double, double)\n", out);
        fputs(HELPER_MOD, out);
    }
    if (g_used_printnum) fputs(HELPER_PRINTNUM, out);
    if (g_used_printbool) {
        fputs("@.strtrue  = private constant [5 x i8] c\"true\\00\"\n", out);
        fputs("@.strfalse = private constant [6 x i8] c\"false\\00\"\n", out);
    }

    if (g_used_strcmp) fputs("declare i32 @strcmp(ptr, ptr)\n", out);
    if (g_used_strlen || g_used_concat) fputs("declare i64 @strlen(ptr)\n", out);
    if (g_used_concat || g_used_intstr || g_used_numstr || g_used_listrt)
        fputs("declare ptr @malloc(i64)\n", out);
    /* one declaration, however many callers: @zl_concat and @zl_push both
     * memcpy, and LLVM rejects the same symbol declared twice */
    if (g_used_concat || g_used_listrt)
        fputs("declare ptr @memcpy(ptr, ptr, i64)\n", out);
    if (g_used_intstr || g_used_numstr) fputs(HELPER_STRFMT, out);
    if (g_used_concat) fputs(HELPER_CONCAT, out);
    if (g_used_intstr) fputs(HELPER_INTSTR, out);
    if (g_used_numstr) fputs(HELPER_NUMSTR, out);

    if (g_used_listrt) fputs(HELPER_LISTRT, out);
    if (g_used_listpr) fputs(HELPER_LISTPR, out);
    for (int i=0;i<g_neqneed;i++) emit_eq_helper(g_eqneed[i]);
    for (int i=0;i<g_nprneed;i++) emit_pr_helper(g_prneed[i]);

    /* the bridge to runtime.c's boxed builtins. Only declared when a
     * builtin was actually called in an expression, so a program that uses
     * none compiles to byte-for-byte what it did before. The definitions
     * live in runtime.c, which the output now links against. */
    if (g_used_bridge) {
        fputs("declare void @zlx_num(ptr, double)\n", out);
        fputs("declare void @zlx_str(ptr, ptr)\n", out);
        fputs("declare void @zlx_call(ptr, ptr, ptr, i32)\n", out);
        if (g_used_listbox) {
            fputs("declare void @zlx_list_new(ptr, i64)\n", out);
            fputs("declare void @zlx_list_set_num(ptr, i64, double)\n", out);
            fputs("declare void @zlx_list_set_str(ptr, i64, ptr)\n", out);
        }
        fputs("declare double @zlx_as_num(ptr)\n", out);
        fputs("declare ptr @zlx_as_str(ptr)\n", out);
        fputs("declare ptr @llvm.stacksave.p0()\n", out);
        fputs("declare void @llvm.stackrestore.p0(ptr)\n", out);
        for (int i=0;i<g_nbnames;i++) {
            const char *s = g_bnames[i];
            fprintf(out, "@.bname.%d = private constant [%u x i8] c\"",
                    i, (unsigned)(strlen(s) + 1));
            for (const unsigned char *p=(const unsigned char*)s; *p; p++) {
                if (*p >= 0x20 && *p < 0x7f && *p != '"' && *p != '\\') fputc(*p, out);
                else fprintf(out, "\\%02X", *p);
            }
            fputs("\\00\"\n", out);
        }
    }
}

int main(int argc, char **argv) {
    if (argc<2){ fprintf(stderr,"usage: compilel <file.zl>\n"); return 1; }
    int count;
    Token *toks = lex_file(argv[1], &count);
    Node *prog = parse(toks, count);

    /* --- symbol tables, then types, then any output at all --------- */
    collect_strs(prog);   /* before emission: str_index has to answer */

    g_fns.count = 0;
    for (int i=0;i<prog->nkids;i++)
        if (prog->kids[i]->type == N_FN) set_add(&g_fns, prog->kids[i]->text);

    g_globals.count = 0;
    for (int i=0;i<prog->nkids;i++)
        if (prog->kids[i]->type != N_FN) collect_vars(prog->kids[i], &g_globals);

    for (int i=0;i<prog->nkids;i++) {
        Node *k = prog->kids[i];
        if (k->type != N_FN) continue;
        int fi = set_index(&g_fns, k->text);
        if (g_fninfo[fi]) continue;          /* a redefinition: first wins */
        FnInfo *f = calloc(1, sizeof *f);    /* calloc => every type is T_INT */
        if (!f) { fprintf(stderr,"compilel: out of memory\n"); return 1; }
        f->node = k;
        for (int p=0;p<k->nkids;p++) set_add(&f->locals, k->kids[p]->text);
        f->nparams = k->nkids;
        collect_vars(k->a, &f->locals);
        g_fninfo[fi] = f;
    }

    /* the exact-division facts are fixed before inference starts, so
     * expr_ty and emit_expr always read the same answer */
    for (int i=0;i<prog->nkids;i++) {
        Node *k = prog->kids[i];
        if (k->type == N_FN) find_exact_divs(k->a, g_fninfo[set_index(&g_fns,k->text)]);
        else                 find_exact_divs(k, 0);
    }

    infer_types(prog);

    out = fopen("out.ll","wb");
    if (!out){ fprintf(stderr,"can't write out.ll\n"); return 1; }

    fputs("; GENERATED by compilel - unboxed LLVM IR\n", out);
    fputs("declare i32 @printf(ptr, ...)\n", out);
    fputs("@.fmt  = private constant [6 x i8] c\"%lld\\0A\\00\"\n", out);
    fputs("@.fmts = private constant [4 x i8] c\"%s\\0A\\00\"\n\n", out);

    /* the list header shape, and what a list slot holds before anything
     * has been assigned to it. Emitted only when the program has a list
     * type at all - inference has already run, so g_nlisttypes says so -
     * which keeps a program with no lists in it compiling to byte for
     * byte what it compiled to before this file learned about them. */
    if (g_nlisttypes) {
        fputs("%zlist = type { i64, i64, ptr, ptr }\n", out);
        fputs("@.listempty = private constant %zlist "
              "{ i64 0, i64 0, ptr null, ptr null }\n\n", out);
    }

    for (int i=0;i<g_nstr;i++) emit_str_constant(i);
    /* what a string slot holds before anything is assigned to it. Only
     * a literal can make a type T_STR in the first place, so g_nstr is
     * exactly the condition - a program with no strings in it emits
     * byte-for-byte what it emitted before this file learned about
     * them, which is what keeps the benchmarks comparable. */
    if (g_nstr) fputs("@.strempty = private constant [1 x i8] c\"\\00\"\n\n", out);

    for (int i=0;i<g_globals.count;i++)
        fprintf(out, "@v_%s = global %s %s\n", g_globals.names[i],
                llty(g_globalty[i]), zero_of(g_globalty[i]));
    fputs("\n", out);

    for (int i=0;i<g_fns.count;i++) emit_fn(g_fninfo[i]);

    fputs("define i32 @main() {\nentry:\n", out);
    terminated = 0;
    for (int i=0;i<prog->nkids;i++) {
        if (terminated) break;
        if (prog->kids[i]->type != N_FN) emit_stmt(prog->kids[i]);
    }
    if (!terminated) fputs("  ret i32 0\n", out);
    fputs("}\n\n", out);

    /* last, now that the flags say which ones are reachable */
    emit_helpers();
    fclose(out);
    printf("compilel: wrote out.ll (LLVM IR)\n");
    return 0;
}
