/* js.c - the bounded JavaScript interpreter. See js.h for the scope and for
 * why "a JS engine is unbounded" and "a JS interpreter is bounded" are
 * different claims.
 *
 * THREE STAGES, the same three interp_kernel.c uses for zl:
 *
 *   1. LEX. Source to tokens. Numbers, strings, names, punctuation, and the
 *      keyword set. Comments and whitespace vanish here.
 *   2. PARSE. Tokens to a tree, by precedence climbing - one table of binding
 *      powers rather than one function per precedence level, because the
 *      twelve-function version is where a missing level hides.
 *   3. EVAL. Walk the tree. Values are a tagged union in a fixed pool; scopes
 *      are a stack of name/value slots with a frame pointer, which is what
 *      makes recursion work without an allocator.
 *
 * NO HEAP AND NO GC. Strings are interned into one arena and never freed, so a
 * program that builds strings in a loop will exhaust it - and STOP, with an
 * error, rather than corrupt anything. That is the honest behaviour available
 * without an allocator, and it is stated in js.h rather than discovered.
 *
 * THE RECURSION LIMIT IS REAL AND CHECKED. This kernel's compositor already
 * overflowed a 16 KiB stack once; a tree-walking evaluator recurses once per
 * nested expression, so eval depth is counted and refused at a ceiling rather
 * than left to hit the guard page.
 */
#include "js.h"

#define MAX_TOK    4096
#define MAX_NODES  4096
#define MAX_VARS   256
#define MAX_FRAMES 64
#define ARENA      16384
#define MAX_OUT    8192
#define MAX_DEPTH  128
#define MAX_ARR    64
#define ARR_CAP    256

/* ---- values ---------------------------------------------------------------- */
enum { V_UNDEF = 0, V_NULL, V_NUM, V_STR, V_BOOL, V_FUNC, V_ARR };

struct val {
    int kind;
    double num;        /* V_NUM, V_BOOL                     */
    int    str, slen;  /* V_STR: offset into the arena       */
    int    node;       /* V_FUNC: the function's AST node    */
    int    arr;        /* V_ARR: index into arrays[]         */
};

struct arr { struct val v[ARR_CAP]; int n; };

/* ---- tokens ---------------------------------------------------------------- */
enum {
    T_EOF = 0, T_NUM, T_STR, T_NAME,
    T_PUNCT, T_KW
};

struct tok { int kind, pos, len; double num; int str, slen; int kw; };

enum {
    K_VAR = 1, K_LET, K_CONST, K_IF, K_ELSE, K_WHILE, K_FOR, K_FUNCTION,
    K_RETURN, K_TRUE, K_FALSE, K_NULL, K_UNDEF, K_BREAK, K_CONTINUE
};

/* ---- AST ------------------------------------------------------------------- */
enum {
    N_NUM = 1, N_STR, N_BOOL, N_NULL, N_UNDEF, N_NAME,
    N_BIN, N_UN, N_ASSIGN, N_CALL, N_INDEX, N_MEMBER, N_ARRAY, N_COND,
    N_VARDECL, N_IF, N_WHILE, N_FOR, N_BLOCK, N_FUNC, N_RETURN,
    N_EXPRSTMT, N_BREAK, N_CONTINUE, N_SEQ, N_POSTFIX
};

struct node {
    int kind;
    int a, b, c, d;      /* child indices, -1 for none  */
    int op;              /* operator / flags            */
    double num;
    int str, slen;       /* names and string literals   */
};

/* ---- state ----------------------------------------------------------------- */
static struct tok  toks[MAX_TOK];
static int         ntok, tp;
static struct node nodes[MAX_NODES];
static int         nnodes;
static char        arena[ARENA];
static int         aused;
static struct arr  arrays[MAX_ARR];
static int         narr;

struct var { int name, nlen; struct val v; };
static struct var vars[MAX_VARS];
static int nvars;
static int frames[MAX_FRAMES];
static int nframes;

static char out[MAX_OUT];
static int  outn;
static char errbuf[128];
static int  haderr;
static int  depth;

static char resbuf[128];
static int  reslen;

/* control-flow signals from eval - a tree walker has no other way to unwind */
enum { F_NONE = 0, F_RETURN, F_BREAK, F_CONTINUE };
static int flow;
static struct val retval;

int js_nodes_used(void) { return nnodes; }
int js_vars_used(void)  { return nvars; }
int js_arena_used(void) { return aused; }

static void err(const char *m)
{
    if (haderr) return;
    haderr = 1;
    int i = 0;
    while (m[i] && i < (int)sizeof errbuf - 1) { errbuf[i] = m[i]; i++; }
    errbuf[i] = 0;
}

const char *js_error(void) { return haderr ? errbuf : ""; }

/* ---- the arena ------------------------------------------------------------- */
static int intern(const char *s, int n)
{
    if (aused + n + 1 > ARENA) { err("string arena exhausted"); return -1; }
    int off = aused;
    for (int i = 0; i < n; i++) arena[aused++] = s[i];
    arena[aused++] = 0;
    return off;
}

static int is_sp(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }
static int is_dig(char c) { return c >= '0' && c <= '9'; }
static int is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '$';
}
static int is_an(char c) { return is_alpha(c) || is_dig(c); }

static int streqn(const char *a, int an, const char *b)
{
    int i = 0;
    for (; i < an; i++) { if (!b[i] || a[i] != b[i]) return 0; }
    return b[i] == 0;
}

static int kw_of(const char *s, int n)
{
    if (streqn(s, n, "var")) return K_VAR;
    if (streqn(s, n, "let")) return K_LET;
    if (streqn(s, n, "const")) return K_CONST;
    if (streqn(s, n, "if")) return K_IF;
    if (streqn(s, n, "else")) return K_ELSE;
    if (streqn(s, n, "while")) return K_WHILE;
    if (streqn(s, n, "for")) return K_FOR;
    if (streqn(s, n, "function")) return K_FUNCTION;
    if (streqn(s, n, "return")) return K_RETURN;
    if (streqn(s, n, "true")) return K_TRUE;
    if (streqn(s, n, "false")) return K_FALSE;
    if (streqn(s, n, "null")) return K_NULL;
    if (streqn(s, n, "undefined")) return K_UNDEF;
    if (streqn(s, n, "break")) return K_BREAK;
    if (streqn(s, n, "continue")) return K_CONTINUE;
    return 0;
}

/* ---- lexer ----------------------------------------------------------------- */
/* the multi-character operators, longest first - checking "=" before "==" is
 * how a comparison silently becomes an assignment */
static const char *ops[] = {
    "===", "!==", "<<=", ">>=",
    "==", "!=", "<=", ">=", "&&", "||", "++", "--",
    "+=", "-=", "*=", "/=", "%=",
    "+", "-", "*", "/", "%", "<", ">", "!", "=", "(", ")", "{", "}",
    "[", "]", ";", ",", ".", ":", "?", 0
};

static void lex(const char *s, int n)
{
    int i = 0;
    ntok = 0;
    while (i < n && ntok < MAX_TOK - 1) {
        if (is_sp(s[i])) { i++; continue; }
        if (i + 1 < n && s[i] == '/' && s[i + 1] == '/') {
            while (i < n && s[i] != '\n') i++;
            continue;
        }
        if (i + 1 < n && s[i] == '/' && s[i + 1] == '*') {
            i += 2;
            while (i + 1 < n && !(s[i] == '*' && s[i + 1] == '/')) i++;
            i = (i + 1 < n) ? i + 2 : n;
            continue;
        }
        struct tok *t = &toks[ntok];
        t->pos = i; t->kw = 0; t->str = -1; t->slen = 0; t->num = 0;

        if (is_dig(s[i]) || (s[i] == '.' && i + 1 < n && is_dig(s[i + 1]))) {
            double v = 0;
            while (i < n && is_dig(s[i])) { v = v * 10 + (s[i] - '0'); i++; }
            if (i < n && s[i] == '.') {
                i++;
                double f = 0.1;
                while (i < n && is_dig(s[i])) { v += (s[i] - '0') * f; f /= 10; i++; }
            }
            t->kind = T_NUM; t->num = v;
            ntok++;
            continue;
        }
        if (s[i] == '"' || s[i] == '\'') {
            char q = s[i++];
            char buf[512];
            int bn = 0;
            while (i < n && s[i] != q && bn < (int)sizeof buf - 1) {
                if (s[i] == '\\' && i + 1 < n) {
                    i++;
                    char c = s[i++];
                    if (c == 'n') buf[bn++] = '\n';
                    else if (c == 't') buf[bn++] = '\t';
                    else buf[bn++] = c;
                } else buf[bn++] = s[i++];
            }
            if (i < n) i++;
            t->kind = T_STR;
            t->str = intern(buf, bn);
            t->slen = bn;
            if (t->str < 0) return;
            ntok++;
            continue;
        }
        if (is_alpha(s[i])) {
            int st = i;
            while (i < n && is_an(s[i])) i++;
            int k = kw_of(s + st, i - st);
            t->kind = k ? T_KW : T_NAME;
            t->kw = k;
            t->str = st; t->slen = i - st;   /* names point at the SOURCE */
            ntok++;
            continue;
        }
        int matched = 0;
        for (int o = 0; ops[o]; o++) {
            int ol = 0;
            while (ops[o][ol]) ol++;
            if (i + ol <= n) {
                int k = 0;
                while (k < ol && s[i + k] == ops[o][k]) k++;
                if (k == ol) {
                    t->kind = T_PUNCT;
                    t->str = i; t->slen = ol;
                    i += ol;
                    ntok++;
                    matched = 1;
                    break;
                }
            }
        }
        if (!matched) { err("unexpected character"); return; }
    }
    toks[ntok].kind = T_EOF;
    toks[ntok].pos = n;
    toks[ntok].slen = 0;
}

/* the token text, for punctuation, comes from the source */
static const char *src_base;

static int tok_is(int i, const char *lit)
{
    if (i > ntok) return 0;
    if (toks[i].kind != T_PUNCT) return 0;
    return streqn(src_base + toks[i].str, toks[i].slen, lit);
}
static int at(const char *lit)  { return tok_is(tp, lit); }
static int atkw(int k)          { return toks[tp].kind == T_KW && toks[tp].kw == k; }
static void adv(void)           { if (tp < ntok) tp++; }
static int eat(const char *lit) { if (at(lit)) { adv(); return 1; } return 0; }
static void want(const char *lit)
{
    if (!eat(lit)) err("expected a token that was not there");
}

/* ---- nodes ----------------------------------------------------------------- */
static int nnew(int kind)
{
    if (nnodes >= MAX_NODES) { err("script too large"); return -1; }
    int i = nnodes++;
    nodes[i].kind = kind;
    nodes[i].a = nodes[i].b = nodes[i].c = nodes[i].d = -1;
    nodes[i].op = 0; nodes[i].num = 0; nodes[i].str = -1; nodes[i].slen = 0;
    return i;
}

static int parse_expr(void);
static int parse_stmt(void);

/* THE PARSER RECURSES TOO, and only the evaluator was counted. Found by the
 * gate: 400 nested parentheses parsed happily, because `((((1))))` builds no
 * nodes and evaluates one level deep - the depth is spent entirely in
 * parse_primary -> parse_expr -> parse_primary, on the C stack, where nothing
 * was watching. This kernel's compositor has already overflowed a stack once;
 * a hostile page must hit a ceiling and get an error, not the guard page. */
static int pdepth;

static int parse_primary(void)
{
    if (haderr) return -1;
    struct tok *t = &toks[tp];
    if (t->kind == T_NUM) { int n = nnew(N_NUM); if (n >= 0) nodes[n].num = t->num; adv(); return n; }
    if (t->kind == T_STR) {
        int n = nnew(N_STR);
        if (n >= 0) { nodes[n].str = t->str; nodes[n].slen = t->slen; }
        adv(); return n;
    }
    if (t->kind == T_KW) {
        if (t->kw == K_TRUE || t->kw == K_FALSE) {
            int n = nnew(N_BOOL);
            if (n >= 0) nodes[n].num = (t->kw == K_TRUE);
            adv(); return n;
        }
        if (t->kw == K_NULL)  { adv(); return nnew(N_NULL); }
        if (t->kw == K_UNDEF) { adv(); return nnew(N_UNDEF); }
        if (t->kw == K_FUNCTION) {
            adv();
            int n = nnew(N_FUNC);
            if (n < 0) return -1;
            if (toks[tp].kind == T_NAME) {
                nodes[n].str = toks[tp].str; nodes[n].slen = toks[tp].slen;
                adv();
            }
            want("(");
            /* parameters chain through .d on a list of N_NAME nodes */
            int head = -1, tail = -1;
            while (!at(")") && !haderr && toks[tp].kind != T_EOF) {
                int p = nnew(N_NAME);
                if (p < 0) return -1;
                nodes[p].str = toks[tp].str; nodes[p].slen = toks[tp].slen;
                adv();
                if (head < 0) head = p; else nodes[tail].d = p;
                tail = p;
                if (!eat(",")) break;
            }
            want(")");
            nodes[n].b = head;
            nodes[n].a = parse_stmt();      /* the body block */
            return n;
        }
    }
    if (t->kind == T_NAME) {
        int n = nnew(N_NAME);
        if (n >= 0) { nodes[n].str = t->str; nodes[n].slen = t->slen; }
        adv(); return n;
    }
    if (at("(")) {
        adv();
        if (++pdepth > MAX_DEPTH) { err("expression nested too deeply"); pdepth--; return -1; }
        int e = parse_expr();
        pdepth--;
        want(")");
        return e;
    }
    if (at("[")) {
        adv();
        int n = nnew(N_ARRAY);
        if (n < 0) return -1;
        int head = -1, tail = -1;
        while (!at("]") && !haderr && toks[tp].kind != T_EOF) {
            int e = parse_expr();
            if (e < 0) break;
            if (head < 0) head = e; else nodes[tail].d = e;
            tail = e;
            if (!eat(",")) break;
        }
        want("]");
        nodes[n].a = head;
        return n;
    }
    err("expected an expression");
    return -1;
}

static int parse_postfix(void)
{
    int e = parse_primary();
    for (;;) {
        if (haderr || e < 0) return e;
        if (at("(")) {
            adv();
            int n = nnew(N_CALL);
            if (n < 0) return -1;
            nodes[n].a = e;
            int head = -1, tail = -1;
            while (!at(")") && !haderr && toks[tp].kind != T_EOF) {
                int arg = parse_expr();
                if (arg < 0) break;
                if (head < 0) head = arg; else nodes[tail].d = arg;
                tail = arg;
                if (!eat(",")) break;
            }
            want(")");
            nodes[n].b = head;
            e = n;
            continue;
        }
        if (at("[")) {
            adv();
            int n = nnew(N_INDEX);
            if (n < 0) return -1;
            nodes[n].a = e;
            nodes[n].b = parse_expr();
            want("]");
            e = n;
            continue;
        }
        if (at(".")) {
            adv();
            int n = nnew(N_MEMBER);
            if (n < 0) return -1;
            nodes[n].a = e;
            nodes[n].str = toks[tp].str; nodes[n].slen = toks[tp].slen;
            adv();
            e = n;
            continue;
        }
        if (at("++") || at("--")) {
            int n = nnew(N_POSTFIX);
            if (n < 0) return -1;
            nodes[n].op = at("++") ? '+' : '-';
            nodes[n].a = e;
            adv();
            e = n;
            continue;
        }
        return e;
    }
}

static int parse_unary(void)
{
    if (at("!") || at("-") || at("+")) {
        int op = src_base[toks[tp].str];
        adv();
        int n = nnew(N_UN);
        if (n < 0) return -1;
        nodes[n].op = op;
        nodes[n].a = parse_unary();
        return n;
    }
    if (at("++") || at("--")) {
        int op = src_base[toks[tp].str];
        adv();
        int n = nnew(N_ASSIGN);
        if (n < 0) return -1;
        nodes[n].op = (op == '+') ? '1' : '2';   /* pre-inc / pre-dec */
        nodes[n].a = parse_unary();
        return n;
    }
    return parse_postfix();
}

/* binding powers. One table, not one function per level - the twelve-function
 * version is where a missing precedence level hides. */
static int bp_of(int *op)
{
    if (at("*") || at("/") || at("%")) { *op = src_base[toks[tp].str]; return 60; }
    if (at("+") || at("-"))            { *op = src_base[toks[tp].str]; return 50; }
    if (at("<=")) { *op = 'L'; return 40; }
    if (at(">=")) { *op = 'G'; return 40; }
    if (at("<"))  { *op = '<'; return 40; }
    if (at(">"))  { *op = '>'; return 40; }
    if (at("===")) { *op = 'E'; return 30; }
    if (at("!==")) { *op = 'N'; return 30; }
    if (at("=="))  { *op = 'E'; return 30; }
    if (at("!="))  { *op = 'N'; return 30; }
    if (at("&&")) { *op = 'A'; return 20; }
    if (at("||")) { *op = 'O'; return 10; }
    return 0;
}

static int parse_bin(int minbp)
{
    int lhs = parse_unary();
    for (;;) {
        if (haderr || lhs < 0) return lhs;
        int op = 0;
        int bp = bp_of(&op);
        if (bp == 0 || bp < minbp) return lhs;
        adv();
        int rhs = parse_bin(bp + 1);
        int n = nnew(N_BIN);
        if (n < 0) return -1;
        nodes[n].op = op;
        nodes[n].a = lhs;
        nodes[n].b = rhs;
        lhs = n;
    }
}

static int parse_assign(void)
{
    int lhs = parse_bin(1);
    if (haderr || lhs < 0) return lhs;
    if (at("?")) {
        adv();
        int n = nnew(N_COND);
        if (n < 0) return -1;
        nodes[n].a = lhs;
        nodes[n].b = parse_assign();
        want(":");
        nodes[n].c = parse_assign();
        return n;
    }
    int op = 0;
    if (at("="))       op = '=';
    else if (at("+=")) op = '+';
    else if (at("-=")) op = '-';
    else if (at("*=")) op = '*';
    else if (at("/=")) op = '/';
    else if (at("%=")) op = '%';
    if (!op) return lhs;
    adv();
    int n = nnew(N_ASSIGN);
    if (n < 0) return -1;
    nodes[n].op = op;
    nodes[n].a = lhs;
    nodes[n].b = parse_assign();
    return n;
}

static int parse_expr(void) { return parse_assign(); }

static int parse_block(void)
{
    int n = nnew(N_BLOCK);
    if (n < 0) return -1;
    want("{");
    int head = -1, tail = -1;
    while (!at("}") && toks[tp].kind != T_EOF && !haderr) {
        int st = parse_stmt();
        if (st < 0) break;
        if (head < 0) head = st; else nodes[tail].d = st;
        tail = st;
    }
    want("}");
    nodes[n].a = head;
    return n;
}

static int parse_stmt(void)
{
    if (haderr) return -1;
    if (at("{")) {
        if (++pdepth > MAX_DEPTH) { err("blocks nested too deeply"); pdepth--; return -1; }
        int b = parse_block();
        pdepth--;
        return b;
    }
    if (at(";")) { adv(); return nnew(N_BLOCK); }

    if (atkw(K_VAR) || atkw(K_LET) || atkw(K_CONST)) {
        adv();
        int n = nnew(N_VARDECL);
        if (n < 0) return -1;
        nodes[n].str = toks[tp].str; nodes[n].slen = toks[tp].slen;
        adv();
        if (eat("=")) nodes[n].a = parse_expr();
        eat(";");
        return n;
    }
    if (atkw(K_IF)) {
        adv();
        int n = nnew(N_IF);
        if (n < 0) return -1;
        want("("); nodes[n].a = parse_expr(); want(")");
        nodes[n].b = parse_stmt();
        if (atkw(K_ELSE)) { adv(); nodes[n].c = parse_stmt(); }
        return n;
    }
    if (atkw(K_WHILE)) {
        adv();
        int n = nnew(N_WHILE);
        if (n < 0) return -1;
        want("("); nodes[n].a = parse_expr(); want(")");
        nodes[n].b = parse_stmt();
        return n;
    }
    if (atkw(K_FOR)) {
        adv();
        int n = nnew(N_FOR);
        if (n < 0) return -1;
        want("(");
        if (!at(";")) nodes[n].a = parse_stmt(); else adv();   /* init */
        if (!at(";")) nodes[n].b = parse_expr();
        want(";");
        if (!at(")")) nodes[n].c = parse_expr();
        want(")");
        nodes[n].d = -1;
        int body = parse_stmt();
        /* the body hangs off op-as-index because .d is the sibling link */
        nodes[n].op = body;
        return n;
    }
    if (atkw(K_FUNCTION)) {
        int f = parse_primary();          /* parses the whole declaration */
        return f;
    }
    if (atkw(K_RETURN)) {
        adv();
        int n = nnew(N_RETURN);
        if (n < 0) return -1;
        if (!at(";") && !at("}") && toks[tp].kind != T_EOF) nodes[n].a = parse_expr();
        eat(";");
        return n;
    }
    if (atkw(K_BREAK))    { adv(); eat(";"); return nnew(N_BREAK); }
    if (atkw(K_CONTINUE)) { adv(); eat(";"); return nnew(N_CONTINUE); }

    int n = nnew(N_EXPRSTMT);
    if (n < 0) return -1;
    nodes[n].a = parse_expr();
    eat(";");
    return n;
}

/* ---- values ---------------------------------------------------------------- */
static struct val mkundef(void) { struct val v; v.kind = V_UNDEF; v.num = 0; v.str = -1; v.slen = 0; v.node = -1; v.arr = -1; return v; }
static struct val mknum(double d) { struct val v = mkundef(); v.kind = V_NUM; v.num = d; return v; }
static struct val mkbool(int b)   { struct val v = mkundef(); v.kind = V_BOOL; v.num = b ? 1 : 0; return v; }
static struct val mkstr(int off, int len) { struct val v = mkundef(); v.kind = V_STR; v.str = off; v.slen = len; return v; }

static int truthy(struct val v)
{
    switch (v.kind) {
    case V_UNDEF: case V_NULL: return 0;
    case V_NUM: case V_BOOL:   return v.num != 0;
    case V_STR:                return v.slen != 0;
    default:                   return 1;
    }
}

/* number to text, without libc. Integers print as integers - "1" not
 * "1.000000" - because a page that writes a count wants a count. */
static int numtext(double d, char *b, int max)
{
    int n = 0, neg = 0;
    if (d < 0) { neg = 1; d = -d; }
    long long ip = (long long)d;
    double fr = d - (double)ip;
    char tmp[32];
    int k = 0;
    if (ip == 0) tmp[k++] = '0';
    while (ip > 0 && k < 30) { tmp[k++] = (char)('0' + (int)(ip % 10)); ip /= 10; }
    if (neg && n < max - 1) b[n++] = '-';
    while (k > 0 && n < max - 1) b[n++] = tmp[--k];
    if (fr > 0.0000001) {
        if (n < max - 1) b[n++] = '.';
        for (int i = 0; i < 6 && n < max - 1; i++) {
            fr *= 10;
            int dg = (int)fr;
            if (dg > 9) dg = 9;
            b[n++] = (char)('0' + dg);
            fr -= dg;
            if (fr < 0.0000001) break;
        }
    }
    b[n] = 0;
    return n;
}

static int val_text(struct val v, char *b, int max)
{
    int n = 0;
    switch (v.kind) {
    case V_UNDEF: { const char *s = "undefined"; while (*s && n < max - 1) b[n++] = *s++; break; }
    case V_NULL:  { const char *s = "null";      while (*s && n < max - 1) b[n++] = *s++; break; }
    case V_BOOL:  { const char *s = v.num ? "true" : "false"; while (*s && n < max - 1) b[n++] = *s++; break; }
    case V_NUM:   n = numtext(v.num, b, max); break;
    case V_STR:   for (int i = 0; i < v.slen && n < max - 1; i++) b[n++] = arena[v.str + i]; break;
    case V_FUNC:  { const char *s = "function"; while (*s && n < max - 1) b[n++] = *s++; break; }
    case V_ARR: {
        struct arr *a = &arrays[v.arr];
        for (int i = 0; i < a->n; i++) {
            if (i && n < max - 1) b[n++] = ',';
            char t[64];
            int tn = val_text(a->v[i], t, (int)sizeof t);
            for (int k = 0; k < tn && n < max - 1; k++) b[n++] = t[k];
        }
        break;
    }
    }
    b[n] = 0;
    return n;
}

static double to_num(struct val v)
{
    if (v.kind == V_NUM || v.kind == V_BOOL) return v.num;
    if (v.kind == V_STR) {
        double d = 0;
        int i = 0, neg = 0;
        if (i < v.slen && arena[v.str + i] == '-') { neg = 1; i++; }
        int any = 0;
        while (i < v.slen && is_dig(arena[v.str + i])) { d = d * 10 + (arena[v.str + i] - '0'); i++; any = 1; }
        if (i < v.slen && arena[v.str + i] == '.') {
            i++;
            double f = 0.1;
            while (i < v.slen && is_dig(arena[v.str + i])) { d += (arena[v.str + i] - '0') * f; f /= 10; i++; any = 1; }
        }
        if (!any) return 0;
        return neg ? -d : d;
    }
    return 0;
}

/* ---- scopes ---------------------------------------------------------------- */
static int find_var(const char *name, int nlen)
{
    /* innermost first, and never below the current frame's floor - which is
     * what stops a function seeing its caller's locals */
    int floor = nframes > 0 ? frames[nframes - 1] : 0;
    for (int i = nvars - 1; i >= floor; i--)
        if (vars[i].nlen == nlen) {
            int k = 0;
            while (k < nlen && src_base[vars[i].name + k] == name[k]) k++;
            if (k == nlen) return i;
        }
    /* globals are always visible */
    for (int i = (nframes > 0 ? frames[0] : 0) - 1; i >= 0; i--)
        if (vars[i].nlen == nlen) {
            int k = 0;
            while (k < nlen && src_base[vars[i].name + k] == name[k]) k++;
            if (k == nlen) return i;
        }
    return -1;
}

static int declare(int name, int nlen, struct val v)
{
    if (nvars >= MAX_VARS) { err("too many variables"); return -1; }
    vars[nvars].name = name;
    vars[nvars].nlen = nlen;
    vars[nvars].v = v;
    return nvars++;
}

/* ---- output ---------------------------------------------------------------- */
static void emit(const char *s, int n)
{
    for (int i = 0; i < n && outn < MAX_OUT - 1; i++) out[outn++] = s[i];
    out[outn] = 0;
}

/* ---- eval ------------------------------------------------------------------ */
static struct val eval(int n);

static struct val call_func(int fnode, int argnode)
{
    struct val r = mkundef();
    if (nframes >= MAX_FRAMES) { err("call depth exceeded"); return r; }

    /* evaluate the arguments in the CALLER's scope, before the frame moves */
    struct val argv[16];
    int argc = 0;
    for (int a = argnode; a >= 0 && argc < 16; a = nodes[a].d)
        argv[argc++] = eval(a);
    if (haderr) return r;

    int save_vars = nvars;
    frames[nframes++] = nvars;
    int p = nodes[fnode].b;
    int i = 0;
    while (p >= 0) {
        declare(nodes[p].str, nodes[p].slen, i < argc ? argv[i] : mkundef());
        p = nodes[p].d;
        i++;
    }
    eval(nodes[fnode].a);
    if (flow == F_RETURN) { r = retval; flow = F_NONE; }
    nframes--;
    nvars = save_vars;
    return r;
}

/* the built-ins, matched by name at the call site. There is no object model,
 * so console.log and document.write are recognised as MEMBER calls rather than
 * being real properties of real objects - which is exactly the shortcut that
 * stops this being a DOM. */
static int name_is(int node, const char *lit)
{
    return streqn(src_base + nodes[node].str, nodes[node].slen, lit);
}

static struct val eval(int n)
{
    struct val v = mkundef();
    if (n < 0 || haderr) return v;
    if (++depth > MAX_DEPTH) { err("expression nested too deeply"); depth--; return v; }

    struct node *nd = &nodes[n];
    switch (nd->kind) {
    case N_NUM:   v = mknum(nd->num); break;
    case N_STR:   v = mkstr(nd->str, nd->slen); break;
    case N_BOOL:  v = mkbool((int)nd->num); break;
    case N_NULL:  v.kind = V_NULL; break;
    case N_UNDEF: break;

    case N_NAME: {
        int i = find_var(src_base + nd->str, nd->slen);
        if (i >= 0) v = vars[i].v;
        break;
    }

    case N_FUNC: {
        v.kind = V_FUNC; v.node = n;
        if (nd->slen > 0) declare(nd->str, nd->slen, v);   /* a declaration */
        break;
    }

    case N_ARRAY: {
        if (narr >= MAX_ARR) { err("too many arrays"); break; }
        struct arr *a = &arrays[narr];
        a->n = 0;
        v.kind = V_ARR; v.arr = narr++;
        for (int e = nd->a; e >= 0 && a->n < ARR_CAP; e = nodes[e].d)
            a->v[a->n++] = eval(e);
        break;
    }

    case N_INDEX: {
        struct val base = eval(nd->a);
        struct val idx = eval(nd->b);
        int i = (int)to_num(idx);
        if (base.kind == V_ARR) {
            struct arr *a = &arrays[base.arr];
            if (i >= 0 && i < a->n) v = a->v[i];
        } else if (base.kind == V_STR) {
            if (i >= 0 && i < base.slen) {
                int off = intern(arena + base.str + i, 1);
                if (off >= 0) v = mkstr(off, 1);
            }
        }
        break;
    }

    case N_MEMBER: {
        struct val base = eval(nd->a);
        if (streqn(src_base + nd->str, nd->slen, "length")) {
            if (base.kind == V_ARR) v = mknum(arrays[base.arr].n);
            else if (base.kind == V_STR) v = mknum(base.slen);
        }
        break;
    }

    case N_CALL: {
        int callee = nd->a;
        /* console.log(...) and document.write(...) */
        if (nodes[callee].kind == N_MEMBER) {
            int obj = nodes[callee].a;
            int is_log = (nodes[obj].kind == N_NAME && name_is(obj, "console") &&
                          streqn(src_base + nodes[callee].str, nodes[callee].slen, "log"));
            int is_wr  = (nodes[obj].kind == N_NAME && name_is(obj, "document") &&
                          streqn(src_base + nodes[callee].str, nodes[callee].slen, "write"));
            if (is_log || is_wr) {
                int first = 1;
                for (int a = nd->b; a >= 0; a = nodes[a].d) {
                    struct val av = eval(a);
                    char b[256];
                    int bn = val_text(av, b, (int)sizeof b);
                    if (!first && is_log) emit(" ", 1);
                    emit(b, bn);
                    first = 0;
                }
                if (is_log) emit("\n", 1);
                break;
            }
            /* arr.push(x) */
            if (streqn(src_base + nodes[callee].str, nodes[callee].slen, "push")) {
                struct val base = eval(nodes[callee].a);
                if (base.kind == V_ARR) {
                    struct arr *a = &arrays[base.arr];
                    for (int x = nd->b; x >= 0; x = nodes[x].d)
                        if (a->n < ARR_CAP) a->v[a->n++] = eval(x);
                    v = mknum(a->n);
                }
                break;
            }
            /* s.charAt(i) */
            if (streqn(src_base + nodes[callee].str, nodes[callee].slen, "charAt")) {
                struct val base = eval(nodes[callee].a);
                struct val iv = nd->b >= 0 ? eval(nd->b) : mknum(0);
                int i = (int)to_num(iv);
                if (base.kind == V_STR && i >= 0 && i < base.slen) {
                    int off = intern(arena + base.str + i, 1);
                    if (off >= 0) v = mkstr(off, 1);
                }
                break;
            }
            break;
        }
        struct val f = eval(callee);
        if (f.kind != V_FUNC) { err("called something that is not a function"); break; }
        v = call_func(f.node, nd->b);
        break;
    }

    case N_UN: {
        struct val a = eval(nd->a);
        if (nd->op == '!') v = mkbool(!truthy(a));
        else if (nd->op == '-') v = mknum(-to_num(a));
        else v = mknum(to_num(a));
        break;
    }

    case N_BIN: {
        int op = nd->op;
        if (op == 'A') { struct val a = eval(nd->a); v = truthy(a) ? eval(nd->b) : a; break; }
        if (op == 'O') { struct val a = eval(nd->a); v = truthy(a) ? a : eval(nd->b); break; }
        struct val a = eval(nd->a), b = eval(nd->b);
        if (op == '+' && (a.kind == V_STR || b.kind == V_STR)) {
            char ta[256], tb[256];
            int na = val_text(a, ta, (int)sizeof ta);
            int nb = val_text(b, tb, (int)sizeof tb);
            if (aused + na + nb + 1 > ARENA) { err("string arena exhausted"); break; }
            int off = aused;
            for (int i = 0; i < na; i++) arena[aused++] = ta[i];
            for (int i = 0; i < nb; i++) arena[aused++] = tb[i];
            arena[aused++] = 0;
            v = mkstr(off, na + nb);
            break;
        }
        double x = to_num(a), y = to_num(b);
        switch (op) {
        case '+': v = mknum(x + y); break;
        case '-': v = mknum(x - y); break;
        case '*': v = mknum(x * y); break;
        case '/': v = mknum(y == 0 ? 0 : x / y); break;
        case '%': v = mknum(y == 0 ? 0 : (double)((long long)x % (long long)y)); break;
        case '<': v = mkbool(x < y); break;
        case '>': v = mkbool(x > y); break;
        case 'L': v = mkbool(x <= y); break;
        case 'G': v = mkbool(x >= y); break;
        case 'E':
            if (a.kind == V_STR && b.kind == V_STR) {
                int eq = (a.slen == b.slen);
                for (int i = 0; eq && i < a.slen; i++)
                    if (arena[a.str + i] != arena[b.str + i]) eq = 0;
                v = mkbool(eq);
            } else v = mkbool(x == y);
            break;
        case 'N':
            if (a.kind == V_STR && b.kind == V_STR) {
                int eq = (a.slen == b.slen);
                for (int i = 0; eq && i < a.slen; i++)
                    if (arena[a.str + i] != arena[b.str + i]) eq = 0;
                v = mkbool(!eq);
            } else v = mkbool(x != y);
            break;
        }
        break;
    }

    case N_COND: v = truthy(eval(nd->a)) ? eval(nd->b) : eval(nd->c); break;

    case N_POSTFIX: {
        struct val old = eval(nd->a);
        if (nodes[nd->a].kind == N_NAME) {
            int i = find_var(src_base + nodes[nd->a].str, nodes[nd->a].slen);
            if (i >= 0) vars[i].v = mknum(to_num(old) + (nd->op == '+' ? 1 : -1));
        }
        v = mknum(to_num(old));
        break;
    }

    case N_ASSIGN: {
        int target = nd->a;
        if (nd->op == '1' || nd->op == '2') {          /* ++x / --x */
            struct val old = eval(target);
            struct val nv = mknum(to_num(old) + (nd->op == '1' ? 1 : -1));
            if (nodes[target].kind == N_NAME) {
                int i = find_var(src_base + nodes[target].str, nodes[target].slen);
                if (i >= 0) vars[i].v = nv;
            }
            v = nv;
            break;
        }
        struct val rhs = eval(nd->b);
        if (nd->op != '=') {
            struct val cur = eval(target);
            if (nd->op == '+' && (cur.kind == V_STR || rhs.kind == V_STR)) {
                char ta[256], tb[256];
                int na = val_text(cur, ta, (int)sizeof ta);
                int nb = val_text(rhs, tb, (int)sizeof tb);
                if (aused + na + nb + 1 > ARENA) { err("string arena exhausted"); break; }
                int off = aused;
                for (int i = 0; i < na; i++) arena[aused++] = ta[i];
                for (int i = 0; i < nb; i++) arena[aused++] = tb[i];
                arena[aused++] = 0;
                rhs = mkstr(off, na + nb);
            } else {
                double x = to_num(cur), y = to_num(rhs);
                switch (nd->op) {
                case '+': rhs = mknum(x + y); break;
                case '-': rhs = mknum(x - y); break;
                case '*': rhs = mknum(x * y); break;
                case '/': rhs = mknum(y == 0 ? 0 : x / y); break;
                case '%': rhs = mknum(y == 0 ? 0 : (double)((long long)x % (long long)y)); break;
                }
            }
        }
        if (nodes[target].kind == N_NAME) {
            int i = find_var(src_base + nodes[target].str, nodes[target].slen);
            if (i >= 0) vars[i].v = rhs;
            else declare(nodes[target].str, nodes[target].slen, rhs);  /* implicit global */
        } else if (nodes[target].kind == N_INDEX) {
            struct val base = eval(nodes[target].a);
            int idx = (int)to_num(eval(nodes[target].b));
            if (base.kind == V_ARR) {
                struct arr *a = &arrays[base.arr];
                if (idx >= 0 && idx < ARR_CAP) {
                    while (a->n <= idx) a->v[a->n++] = mkundef();
                    a->v[idx] = rhs;
                }
            }
        }
        v = rhs;
        break;
    }

    case N_VARDECL: {
        struct val init = nd->a >= 0 ? eval(nd->a) : mkundef();
        declare(nd->str, nd->slen, init);
        break;
    }

    case N_IF:
        if (truthy(eval(nd->a))) eval(nd->b);
        else if (nd->c >= 0) eval(nd->c);
        break;

    case N_WHILE: {
        int guard = 0;
        while (truthy(eval(nd->a)) && !haderr) {
            eval(nd->b);
            if (flow == F_BREAK) { flow = F_NONE; break; }
            if (flow == F_CONTINUE) flow = F_NONE;
            if (flow == F_RETURN) break;
            if (++guard > 1000000) { err("loop ran too long"); break; }
        }
        break;
    }

    case N_FOR: {
        int save = nvars;
        if (nd->a >= 0) eval(nd->a);
        int guard = 0;
        while (!haderr) {
            if (nd->b >= 0 && !truthy(eval(nd->b))) break;
            eval(nd->op);                     /* the body */
            if (flow == F_BREAK) { flow = F_NONE; break; }
            if (flow == F_CONTINUE) flow = F_NONE;
            if (flow == F_RETURN) break;
            if (nd->c >= 0) eval(nd->c);
            if (++guard > 1000000) { err("loop ran too long"); break; }
        }
        nvars = save;
        break;
    }

    case N_BLOCK: {
        int save = nvars;
        for (int s = nd->a; s >= 0 && !haderr; s = nodes[s].d) {
            eval(s);
            if (flow != F_NONE) break;
        }
        nvars = save;
        break;
    }

    case N_RETURN:
        retval = nd->a >= 0 ? eval(nd->a) : mkundef();
        flow = F_RETURN;
        break;

    case N_BREAK:    flow = F_BREAK; break;
    case N_CONTINUE: flow = F_CONTINUE; break;

    case N_EXPRSTMT: v = eval(nd->a); break;
    }

    depth--;
    return v;
}

/* ---- the entry point ------------------------------------------------------- */
void js_reset(void)
{
    ntok = tp = nnodes = aused = nvars = nframes = narr = 0;
    pdepth = 0;
    outn = 0; out[0] = 0;
    haderr = 0; errbuf[0] = 0;
    flow = F_NONE; depth = 0;
    reslen = 0; resbuf[0] = 0;
}

int js_eval(const char *src, int len)
{
    js_reset();
    src_base = src;
    lex(src, len);
    if (haderr) return -1;
    tp = 0;

    /* HOIST FUNCTION DECLARATIONS. A script that calls a function defined
     * lower down is normal JS and would otherwise fail; one pass over the top
     * level before running is the bounded version of hoisting. */
    int prog[512];
    int np = 0;
    while (toks[tp].kind != T_EOF && !haderr && np < 512) {
        int s = parse_stmt();
        if (s < 0) break;
        prog[np++] = s;
    }
    if (haderr) return -1;

    for (int i = 0; i < np; i++)
        if (nodes[prog[i]].kind == N_FUNC && nodes[prog[i]].slen > 0) {
            struct val f = mkundef();
            f.kind = V_FUNC; f.node = prog[i];
            declare(nodes[prog[i]].str, nodes[prog[i]].slen, f);
        }

    struct val last = mkundef();
    for (int i = 0; i < np && !haderr; i++) {
        if (nodes[prog[i]].kind == N_FUNC) continue;    /* already hoisted */
        last = eval(prog[i]);
        if (flow != F_NONE) break;
    }
    if (haderr) return -1;
    reslen = val_text(last, resbuf, (int)sizeof resbuf);
    return 0;
}

const char *js_result(int *len) { if (len) *len = reslen; return resbuf; }
const char *js_output(int *len) { if (len) *len = outn; return out; }
