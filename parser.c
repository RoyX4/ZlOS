/* parser.c - stage 2 of the compiler.
 *
 * The lexer gave us a FLAT list of tokens:
 *
 *     NUMBER 2   SYMBOL +   NUMBER 3   SYMBOL *   NUMBER 4
 *
 * A flat list has no idea that '*' should happen before '+'.
 * The parser's whole job is to turn that flat list into a TREE
 * whose SHAPE encodes the meaning:
 *
 *     BINARY +
 *       NUMBER 2
 *       BINARY *
 *         NUMBER 3
 *         NUMBER 4
 *
 * '3 * 4' sits DEEPER in the tree, so it happens first. Precedence
 * is not a rule we check later - it is baked into the shape here.
 *
 * We use "recursive descent": one function per level of the grammar.
 * The lower-precedence functions call the higher-precedence ones,
 * so the tighter-binding operators end up deeper in the tree.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"     /* NodeType and Node now live here */

static Node *new_node(NodeType type)
{
    Node *n = calloc(1, sizeof(Node));   /* calloc = all slots NULL/0 */
    if (n == NULL) { fprintf(stderr, "out of memory\n"); exit(1); }
    n->type = type;
    return n;
}

static void set_text(Node *n, const char *s)
{
    strncpy(n->text, s, MAX_TEXT - 1);
    n->text[MAX_TEXT - 1] = '\0';
}

/* append a child to a node's kids[] array, growing it as needed */
static void add_kid(Node *parent, Node *kid)
{
    parent->kids = realloc(parent->kids,
                           (size_t)(parent->nkids + 1) * sizeof(Node *));
    if (parent->kids == NULL) { fprintf(stderr, "out of memory\n"); exit(1); }
    parent->kids[parent->nkids++] = kid;
}

/* =============================================================
 * PARSER STATE - a cursor walking the token array
 * ============================================================= */

static Token *toks;
static int    ntoks;
static int    pos;

static Token *cur(void)        { return &toks[pos]; }
static TokenType curtype(void) { return toks[pos].type; }
static Token *advance(void)    { return &toks[pos++]; }

static void parse_error(const char *msg)
{
    fprintf(stderr, "line %d: %s (got '%s')\n",
            cur()->line, msg, cur()->text);
    exit(1);
}

/* is the current token this exact text? (used for keywords, which
 * are already type-guarded by their callers) */
static int is_text(const char *s)
{
    return strcmp(cur()->text, s) == 0;
}

/* is the current token this exact SYMBOL? This is stricter than
 * is_text: it also checks the token is a symbol, not a string. That
 * matters because the string literal "-" has text "-" but must NOT
 * be mistaken for the minus operator. (Bug found by self-hosting.) */
static int is_sym(const char *s)
{
    return cur()->type == T_SYMBOL && strcmp(cur()->text, s) == 0;
}

/* if the current token is this SYMBOL, eat it and return 1 */
static int match_text(const char *s)
{
    if (is_sym(s)) { advance(); return 1; }
    return 0;
}

static void expect_text(const char *s)
{
    if (!match_text(s)) {
        char buf[64];
        snprintf(buf, sizeof(buf), "expected '%s'", s);
        parse_error(buf);
    }
}

/* newlines separate statements; between statements we skip any run */
static void skip_newlines(void)
{
    while (curtype() == T_NEWLINE) advance();
}

/* forward declarations (the grammar is recursive) */
static Node *parse_expr(void);
static Node *parse_statement(void);
static Node *parse_block(void);

/* =============================================================
 * EXPRESSIONS - lowest precedence at the top, highest at the bottom.
 *
 * Each function does ONE precedence level, then calls the next
 * (tighter) level for its operands. Because 'mul' is called from
 * inside 'add', anything with '*' lands deeper than anything with
 * '+'. That depth IS the precedence.
 * ============================================================= */

/* =============================================================
 * f-STRINGS -  f"total {a + b}"  ->  "total " + str(a + b)
 *
 * There is NO new node type. The parser rewrites an f-string into the
 * concatenation the language already had, so the interpreter, the C
 * emitter and every other backend get interpolation for free without
 * learning anything.
 *
 * '{{' and '}}' are literal braces. A '"' cannot appear inside {} -
 * the lexer ends the string at it - so keep the slots quote-free.
 * ============================================================= */

/* like set_text, but for a slice that has no NUL of its own */
static Node *new_string_node(const char *s, int len)
{
    Node *n = new_node(N_STRING);
    if (len >= MAX_TEXT) len = MAX_TEXT - 1;
    memcpy(n->text, s, (size_t)len);
    n->text[len] = '\0';
    return n;
}

/* Parse one expression out of its own private token array, then put
 * the main cursor back exactly where it was. The nodes copy their text
 * out of the tokens, so the little array can be freed straight after. */
static Node *parse_slot_expr(const char *src, int line)
{
    int    nsub = 0;
    Token *sub  = lex_text(src, &nsub);

    Token *save_toks  = toks;
    int    save_ntoks = ntoks;
    int    save_pos   = pos;

    toks = sub; ntoks = nsub; pos = 0;

    skip_newlines();
    if (curtype() == T_EOF) {
        fprintf(stderr, "line %d: f-string has an empty {}\n", line);
        exit(1);
    }
    Node *e = parse_expr();
    skip_newlines();
    if (curtype() != T_EOF) {
        fprintf(stderr, "line %d: f-string slot has leftover text (got '%s')\n",
                line, cur()->text);
        exit(1);
    }

    toks = save_toks; ntoks = save_ntoks; pos = save_pos;
    free(sub);
    return e;
}

/* every spliced value goes through str(), so '+' is always text-join */
static Node *wrap_in_str(Node *e)
{
    Node *callee = new_node(N_IDENT);
    set_text(callee, "str");

    Node *call = new_node(N_CALL);
    call->a = callee;
    add_kid(call, e);
    return call;
}

static Node *concat(Node *left, Node *right)
{
    if (left == NULL) return right;
    Node *bin = new_node(N_BINARY);
    set_text(bin, "+");
    bin->a = left;
    bin->b = right;
    return bin;
}

static Node *build_fstring(const char *s, int line)
{
    char  buf[MAX_TEXT];
    int   blen = 0;
    Node *acc  = NULL;
    int   i    = 0;

    for (;;) {
        char c = s[i];

        if (c == '{' && s[i + 1] == '{') {          /* {{ -> literal { */
            if (blen < MAX_TEXT - 1) buf[blen++] = '{';
            i += 2;
            continue;
        }
        if (c == '}' && s[i + 1] == '}') {          /* }} -> literal } */
            if (blen < MAX_TEXT - 1) buf[blen++] = '}';
            i += 2;
            continue;
        }

        if (c == '\0' || c == '{') {
            if (blen > 0) {                          /* flush literal text */
                acc  = concat(acc, new_string_node(buf, blen));
                blen = 0;
            }
            if (c == '\0') break;

            i++;                                     /* eat '{' */
            int start = i;
            while (s[i] != '}' && s[i] != '\0') i++;
            if (s[i] == '\0') {
                fprintf(stderr, "line %d: f-string has '{' with no matching '}'\n",
                        line);
                exit(1);
            }

            char slot[MAX_TEXT];
            int  slen = i - start;
            if (slen >= MAX_TEXT) slen = MAX_TEXT - 1;
            memcpy(slot, s + start, (size_t)slen);
            slot[slen] = '\0';
            i++;                                     /* eat '}' */

            acc = concat(acc, wrap_in_str(parse_slot_expr(slot, line)));
            continue;
        }

        if (c == '}') {
            fprintf(stderr,
                    "line %d: f-string has a lone '}' - write '}}' for a literal brace\n",
                    line);
            exit(1);
        }

        if (blen < MAX_TEXT - 1) buf[blen++] = c;
        i++;
    }

    /* f"" still has to BE a string, not nothing */
    if (acc == NULL) acc = new_string_node("", 0);
    return acc;
}

/* primary := NUMBER | STRING | FSTRING | IDENT | true | false
 *          | ( expr ) | [ list ]                        */
static Node *parse_primary(void)
{
    Token *t = cur();

    if (t->type == T_NUMBER) {
        advance();
        Node *n = new_node(N_NUMBER);
        set_text(n, t->text);
        return n;
    }
    if (t->type == T_STRING) {
        advance();
        Node *n = new_node(N_STRING);
        set_text(n, t->text);
        return n;
    }
    if (t->type == T_FSTRING) {
        /* copy first: building the tree swaps the token cursor around */
        char raw[MAX_TEXT];
        int  line = t->line;
        strncpy(raw, t->text, MAX_TEXT - 1);
        raw[MAX_TEXT - 1] = '\0';
        advance();
        return build_fstring(raw, line);
    }
    if (t->type == T_KEYWORD && (is_text("true") || is_text("false"))) {
        Node *n = new_node(N_BOOL);
        set_text(n, t->text);
        advance();
        return n;
    }
    if (t->type == T_IDENT) {
        advance();
        Node *n = new_node(N_IDENT);
        set_text(n, t->text);
        return n;
    }
    if (is_sym("(")) {
        advance();
        Node *n = parse_expr();
        expect_text(")");
        return n;                    /* parens just group - no node needed */
    }
    if (is_sym("[")) {
        advance();
        Node *n = new_node(N_LIST);
        if (!is_text("]")) {
            add_kid(n, parse_expr());
            while (match_text(",")) add_kid(n, parse_expr());
        }
        expect_text("]");
        return n;
    }

    parse_error("expected a value");
    return NULL;                     /* unreachable */
}

/* postfix := primary ( call | member | index | danger )*
 *   call   := ( args )
 *   member := . IDENT
 *   index  := [ expr ]
 *   danger := !                                          */
static Node *parse_postfix(void)
{
    Node *node = parse_primary();

    for (;;) {
        if (is_sym("(")) {                        /* a function call */
            advance();
            Node *call = new_node(N_CALL);
            call->a = node;
            if (!is_sym(")")) {
                add_kid(call, parse_expr());
                while (match_text(",")) add_kid(call, parse_expr());
            }
            expect_text(")");
            node = call;
        }
        else if (is_sym(".")) {                   /* member access */
            advance();
            if (curtype() != T_IDENT) parse_error("expected a name after '.'");
            Node *m = new_node(N_MEMBER);
            m->a = node;
            set_text(m, cur()->text);
            advance();
            node = m;
        }
        else if (is_sym("[")) {                   /* index access */
            advance();
            Node *ix = new_node(N_INDEX);
            ix->a = node;
            ix->b = parse_expr();
            expect_text("]");
            node = ix;
        }
        else if (is_sym("!")) {                   /* the danger marker */
            advance();
            Node *d = new_node(N_DANGER);
            d->a = node;
            node = d;
        }
        else {
            break;
        }
    }
    return node;
}

/* unary := - unary | postfix */
static Node *parse_unary(void)
{
    if (is_sym("-")) {
        advance();
        Node *n = new_node(N_UNARY);
        set_text(n, "-");
        n->a = parse_unary();
        return n;
    }
    return parse_postfix();
}

/* helper: build a left-associative chain of binary ops.
 * loops while the current token is one of the given operators. */
static Node *parse_binary_left(Node *(*next)(void), const char *ops[])
{
    Node *node = next();
    for (;;) {
        int matched = 0;
        for (int i = 0; ops[i] != NULL; i++) {
            if (is_sym(ops[i])) {
                char op[MAX_TEXT];
                strncpy(op, cur()->text, MAX_TEXT - 1);
                op[MAX_TEXT - 1] = '\0';
                advance();
                Node *bin = new_node(N_BINARY);
                set_text(bin, op);
                bin->a = node;
                bin->b = next();
                node = bin;
                matched = 1;
                break;
            }
        }
        if (!matched) break;
    }
    return node;
}

/* mul := unary ( (* | / | %) unary )*     <- tightest binding */
static Node *parse_mul(void)
{
    static const char *ops[] = { "*", "/", "%", NULL };
    return parse_binary_left(parse_unary, ops);
}

/* add := mul ( (+ | -) mul )* */
static Node *parse_add(void)
{
    static const char *ops[] = { "+", "-", NULL };
    return parse_binary_left(parse_mul, ops);
}

/* cmp := add ( (== != >= <= > <) add )* */
static Node *parse_cmp(void)
{
    static const char *ops[] = { "==", "!=", ">=", "<=", ">", "<", NULL };
    return parse_binary_left(parse_add, ops);
}

/* is the token AFTER the current one this exact keyword?
 * Safe at any position: the token array always ends in T_EOF, so if
 * the current token is a keyword there is always one more after it. */
static int next_is_keyword(const char *s)
{
    return toks[pos + 1].type == T_KEYWORD && strcmp(toks[pos + 1].text, s) == 0;
}

/* in := cmp ( ('in' | 'not' 'in') cmp )*
 *
 * A word-operator sitting between compare and 'not'.
 *
 * 'a not in b' is built as  not (a in b)  - one new operator for the
 * engines to learn instead of two, and 'not' keeps its single meaning.
 *
 * This does NOT capture the 'in' of a for-header: parse_for eats its
 * own 'in' before it ever asks for an expression, and the iterable it
 * then parses is always followed by '{'. So 'for x in xs {' still
 * parses as a for-loop, not as the expression 'x in xs'. */
static Node *parse_in(void)
{
    Node *node = parse_cmp();

    for (;;) {
        int negate = 0;

        if (curtype() == T_KEYWORD && is_text("not") && next_is_keyword("in")) {
            advance();               /* eat 'not'; the 'in' goes below */
            negate = 1;
        } else if (!(curtype() == T_KEYWORD && is_text("in"))) {
            break;                   /* a lone 'not' is the prefix operator */
        }

        advance();                   /* eat 'in' */
        Node *bin = new_node(N_BINARY);
        set_text(bin, "in");
        bin->a = node;
        bin->b = parse_cmp();

        if (negate) {
            Node *neg = new_node(N_UNARY);
            set_text(neg, "not");
            neg->a = bin;
            node = neg;
        } else {
            node = bin;
        }
    }
    return node;
}

/* 'not' is a word-operator, tighter than and/or, looser than compare */
static Node *parse_not(void)
{
    if (curtype() == T_KEYWORD && is_text("not")) {
        advance();
        Node *n = new_node(N_UNARY);
        set_text(n, "not");
        n->a = parse_not();
        return n;
    }
    return parse_in();
}

/* and := not ( 'and' not )* */
static Node *parse_and(void)
{
    Node *node = parse_not();
    while (curtype() == T_KEYWORD && is_text("and")) {
        advance();
        Node *bin = new_node(N_BINARY);
        set_text(bin, "and");
        bin->a = node;
        bin->b = parse_not();
        node = bin;
    }
    return node;
}

/* or := and ( 'or' and )*      <- loosest binding */
static Node *parse_or(void)
{
    Node *node = parse_and();
    while (curtype() == T_KEYWORD && is_text("or")) {
        advance();
        Node *bin = new_node(N_BINARY);
        set_text(bin, "or");
        bin->a = node;
        bin->b = parse_and();
        node = bin;
    }
    return node;
}

/* ternary := or ( '?' ternary ':' ternary )?   <- looser than 'or'
 *
 * Right-associative: the else-branch recurses into ternary again, so
 *   a ? b : c ? d : e   is   a ? b : (c ? d : e)
 * The then-branch may also be a ternary; the ':' ends it either way. */
static Node *parse_ternary(void)
{
    Node *cond = parse_or();
    if (!is_sym("?")) return cond;

    advance();                       /* eat '?' */
    Node *n = new_node(N_TERNARY);
    n->a = cond;
    n->b = parse_ternary();          /* value if true  */
    expect_text(":");
    n->c = parse_ternary();          /* value if false */
    return n;
}

static Node *parse_expr(void)
{
    return parse_ternary();
}

/* =============================================================
 * STATEMENTS
 * ============================================================= */

/* block := { (statement)* } */
static Node *parse_block(void)
{
    expect_text("{");
    Node *block = new_node(N_BLOCK);
    skip_newlines();
    while (!is_text("}") && curtype() != T_EOF) {
        add_kid(block, parse_statement());
        skip_newlines();
    }
    expect_text("}");
    return block;
}

/* if := 'if' expr block ('elif' expr block)* ('else' block)? */
static Node *parse_if(void)
{
    advance();                       /* eat 'if' (or 'elif' - same shape) */
    Node *n = new_node(N_IF);
    n->a = parse_expr();             /* condition */
    n->b = parse_block();            /* then-block */
    if (curtype() == T_KEYWORD && is_text("elif")) {
        /* 'elif' IS 'else if'. The recursive call eats the 'elif'
           exactly the way it would eat an 'if', so the tree we build
           is indistinguishable from the one 'else if' produces. */
        Node *blk = new_node(N_BLOCK);
        add_kid(blk, parse_if());
        n->c = blk;
        return n;
    }
    if (curtype() == T_KEYWORD && is_text("else")) {
        advance();
        if (curtype() == T_KEYWORD && is_text("if")) {
            /* 'else if' -> wrap a nested if as the else-block */
            Node *blk = new_node(N_BLOCK);
            add_kid(blk, parse_if());
            n->c = blk;
        } else {
            n->c = parse_block();    /* plain else-block */
        }
    }
    return n;
}

/* while := 'while' expr block */
static Node *parse_while(void)
{
    advance();                       /* eat 'while' */
    Node *n = new_node(N_WHILE);
    n->a = parse_expr();             /* condition */
    n->b = parse_block();            /* body */
    return n;
}

/* =============================================================
 * DESUGARING - the three loop forms below are REWRITTEN, here in the
 * parser, into the while/if/assign tree the language already had.
 *
 * That is the whole trick: no new NodeType, so the interpreter, the C
 * emitter and every other backend understand them without learning a
 * single thing. It also means break and continue keep working, because
 * what they end up inside really is an ordinary while.
 *
 * None of 'to', 'step', 'do' or 'loop' becomes a reserved word. Each
 * is recognised only in a position where the old grammar had a syntax
 * error, so no text that already parses parses differently, and all
 * four stay usable as ordinary variable names.
 * ============================================================= */

/* Two desugared loops can nest, and each needs its own bookkeeping
 * variables. This counter keeps their names apart. It is reset per
 * parse() so the same source always yields the same tree. */
static int g_hidden;

/* A name for that bookkeeping. The '__zl_' prefix plus a per-loop
 * number is what keeps it clear of whatever the program calls things. */
static Node *hidden_ident(const char *what, int id)
{
    Node *n = new_node(N_IDENT);
    snprintf(n->text, MAX_TEXT, "__zl_%s%d", what, id);
    return n;
}

static Node *plain_ident(const char *name)
{
    Node *n = new_node(N_IDENT);
    set_text(n, name);
    return n;
}

static Node *bool_node(const char *word)      /* "true" or "false" */
{
    Node *n = new_node(N_BOOL);
    set_text(n, word);
    return n;
}

static Node *num_node(const char *literal)
{
    Node *n = new_node(N_NUMBER);
    set_text(n, literal);
    return n;
}

static Node *binary(const char *op, Node *left, Node *right)
{
    Node *n = new_node(N_BINARY);
    set_text(n, op);
    n->a = left;
    n->b = right;
    return n;
}

static Node *assign_to(Node *target, Node *value)
{
    Node *n = new_node(N_ASSIGN);
    n->a = target;
    n->b = value;
    return n;
}

/* if COND { ONE-STATEMENT } */
static Node *if_then(Node *cond, Node *stmt)
{
    Node *blk = new_node(N_BLOCK);
    add_kid(blk, stmt);
    Node *n = new_node(N_IF);
    n->a = cond;
    n->b = blk;
    return n;
}

/* copy one block's statements into another, so the user's body ends up
 * directly inside the generated while rather than one level down */
static void splice_block(Node *dest, Node *src)
{
    for (int i = 0; i < src->nkids; i++) add_kid(dest, src->kids[i]);
}

/* 'for' VAR '=' START 'to' LIMIT ('step' STEP)? block
 *
 * An INCLUSIVE numeric range. A negative step counts down.
 *
 *     i = START
 *     lim = LIMIT ;  st = STEP ;  first = true
 *     while true {
 *         if first { first = false } else { i = i + st }
 *         if (st < 0 ? i < lim : i > lim) { break }
 *         BODY
 *     }
 *
 * Two decisions worth spelling out:
 *
 *   The counter is bumped at the TOP of the body, not the bottom. Put
 *   it at the bottom and a 'continue' in BODY jumps straight past it
 *   and the loop spins forever. The 'first' flag is what pays for the
 *   top position - and it also means the first value of i is exactly
 *   START, with no arithmetic done to it (START - STEP + STEP is not
 *   always START in floating point).
 *
 *   LIMIT and STEP are evaluated ONCE, into lim and st, before the
 *   loop - so 'for i = 1 to len(xs)' does not re-measure xs every
 *   iteration, and a body that changes them cannot derail the count.
 */
static Node *parse_for_range(const char *var)
{
    int id = g_hidden++;

    advance();                       /* eat '=' */
    Node *start = parse_expr();

    if (!(curtype() == T_IDENT && is_text("to")))
        parse_error("expected 'to' in a numeric for-range");
    advance();
    Node *limit = parse_expr();

    Node *stepv;
    if (curtype() == T_IDENT && is_text("step")) {
        advance();
        stepv = parse_expr();
    } else {
        stepv = num_node("1");
    }

    Node *body = parse_block();

    /* if first { first = false } else { i = i + st } */
    Node *bump = new_node(N_IF);
    bump->a = hidden_ident("first", id);
    bump->b = new_node(N_BLOCK);
    add_kid(bump->b, assign_to(hidden_ident("first", id), bool_node("false")));
    bump->c = new_node(N_BLOCK);
    add_kid(bump->c, assign_to(plain_ident(var),
                               binary("+", plain_ident(var),
                                           hidden_ident("st", id))));

    /* if (st < 0 ? i < lim : i > lim) { break }
     * A ternary, not 'and'/'or': only the relevant comparison runs. */
    Node *test = new_node(N_TERNARY);
    test->a = binary("<", hidden_ident("st", id), num_node("0"));
    test->b = binary("<", plain_ident(var), hidden_ident("lim", id));
    test->c = binary(">", plain_ident(var), hidden_ident("lim", id));

    Node *inner = new_node(N_BLOCK);
    add_kid(inner, bump);
    add_kid(inner, if_then(test, new_node(N_BREAK)));
    splice_block(inner, body);

    Node *loop = new_node(N_WHILE);
    loop->a = bool_node("true");
    loop->b = inner;

    Node *out = new_node(N_BLOCK);
    add_kid(out, assign_to(plain_ident(var), start));
    add_kid(out, assign_to(hidden_ident("lim", id), limit));
    add_kid(out, assign_to(hidden_ident("st", id), stepv));
    add_kid(out, assign_to(hidden_ident("first", id), bool_node("true")));
    add_kid(out, loop);
    return out;
}

/* for := 'for' IDENT 'in' expr block               (over a list)
 *      | 'for' IDENT '=' expr 'to' expr ('step' expr)? block
 *
 * 'for i =' used to be a syntax error, so the second form is purely
 * additive - and 'to'/'step' are only looked for on this path. */
static Node *parse_for(void)
{
    advance();                       /* eat 'for' */
    if (curtype() != T_IDENT) parse_error("expected a loop variable after 'for'");

    char var[MAX_TEXT];
    strncpy(var, cur()->text, MAX_TEXT - 1);
    var[MAX_TEXT - 1] = '\0';
    advance();

    if (is_sym("=")) return parse_for_range(var);

    if (!(curtype() == T_KEYWORD && is_text("in")))
        parse_error("expected 'in' or '=' after the loop variable");
    advance();

    Node *n = new_node(N_FOR);
    set_text(n, var);                /* the loop variable name */
    n->a = parse_expr();             /* the thing to loop over */
    n->b = parse_block();            /* the body */
    return n;
}

/* do := 'do' block 'while' expr        - the body always runs once
 *
 *     again = true
 *     while (again ? true : COND) { again = false ; BODY }
 *
 * The ternary is doing real work: it stops COND from being evaluated
 * before the body has run even once, which is the entire difference
 * between this and a plain while. (A plain 'or' would not do - the C
 * backend evaluates both of its operands.) */
static Node *parse_do_while(void)
{
    int id = g_hidden++;

    advance();                       /* eat 'do' */
    Node *body = parse_block();

    if (!(curtype() == T_KEYWORD && is_text("while")))
        parse_error("expected 'while' after the body of a 'do'");
    advance();
    Node *cond = parse_expr();

    Node *inner = new_node(N_BLOCK);
    add_kid(inner, assign_to(hidden_ident("again", id), bool_node("false")));
    splice_block(inner, body);

    Node *test = new_node(N_TERNARY);
    test->a = hidden_ident("again", id);
    test->b = bool_node("true");
    test->c = cond;

    Node *loop = new_node(N_WHILE);
    loop->a = test;
    loop->b = inner;

    Node *out = new_node(N_BLOCK);
    add_kid(out, assign_to(hidden_ident("again", id), bool_node("true")));
    add_kid(out, loop);
    return out;
}

/* loop := 'loop' block        - forever, until a 'break' */
static Node *parse_loop(void)
{
    advance();                       /* eat 'loop' */
    Node *n = new_node(N_WHILE);
    n->a = bool_node("true");
    n->b = parse_block();
    return n;
}

/* fn := 'fn' IDENT '(' params ')' block */
static Node *parse_fn(void)
{
    advance();                       /* eat 'fn' */
    if (curtype() != T_IDENT) parse_error("expected a function name after 'fn'");
    Node *n = new_node(N_FN);
    set_text(n, cur()->text);
    advance();

    expect_text("(");
    if (!is_text(")")) {
        do {
            if (curtype() != T_IDENT) parse_error("expected a parameter name");
            Node *p = new_node(N_IDENT);
            set_text(p, cur()->text);
            advance();
            add_kid(n, p);
        } while (match_text(","));
    }
    expect_text(")");

    n->a = parse_block();
    return n;
}

/* return := 'return' expr? */
static Node *parse_return(void)
{
    advance();                       /* eat 'return' */
    Node *n = new_node(N_RETURN);
    if (curtype() != T_NEWLINE && !is_text("}") && curtype() != T_EOF) {
        n->a = parse_expr();
    }
    return n;
}

/* is the token AFTER the current one this exact symbol? */
static int next_is_sym(const char *s)
{
    return toks[pos + 1].type == T_SYMBOL && strcmp(toks[pos + 1].text, s) == 0;
}

/* 'do' and 'loop' are ordinary identifiers that only lead a statement
 * when a '{' is glued straight onto them. An identifier followed by
 * '{' was a syntax error before this existed, so nothing that used to
 * parse changes meaning, and 'do = 1' or 'loop(3)' still work. */
static int starts_block_word(const char *word)
{
    return curtype() == T_IDENT && is_text(word) && next_is_sym("{");
}

/* a statement is either a keyword-led form, or an expression that
 * may turn out to be an assignment (target = value). */
static Node *parse_statement(void)
{
    if (curtype() == T_KEYWORD) {
        if (is_text("if"))     return parse_if();
        if (is_text("while"))  return parse_while();
        if (is_text("for"))    return parse_for();
        if (is_text("fn"))     return parse_fn();
        if (is_text("return")) return parse_return();
        if (is_text("break"))    { advance(); return new_node(N_BREAK); }
        if (is_text("continue")) { advance(); return new_node(N_CONTINUE); }
    }

    if (starts_block_word("do"))   return parse_do_while();
    if (starts_block_word("loop")) return parse_loop();

    Node *expr = parse_expr();

    if (is_sym("=")) {              /* it's an assignment */
        advance();
        Node *n = new_node(N_ASSIGN);
        n->a = expr;                 /* the target (left side)  */
        n->b = parse_expr();         /* the value  (right side) */
        return n;
    }

    /* compound assignment:  x op= v
     *
     * The operator is carried ON the N_ASSIGN node (its text field, which
     * a plain `=` leaves empty), and n->b is just the right-hand VALUE.
     *
     * It used to desugar to `x = x op v` by putting the SAME Node into
     * two places in the tree - once as the assignment target and once as
     * the binary's left operand. For an index target that meant the
     * subscript was evaluated TWICE, once for the read and once for the
     * write, so `xs[idx()] += 1` called idx() twice. It also left one
     * Node with two parents, which no tree pass could safely free. */
    if (is_sym("+=") || is_sym("-=") || is_sym("*=") || is_sym("/=") || is_sym("%=")) {
        char op[2]; op[0] = cur()->text[0]; op[1] = '\0';
        advance();
        Node *n = new_node(N_ASSIGN);
        set_text(n, op);
        n->a = expr;                 /* the target */
        n->b = parse_expr();         /* the right-hand value */
        return n;
    }

    Node *stmt = new_node(N_EXPRSTMT);
    stmt->a = expr;
    return stmt;
}

/* program := (statement)* EOF */
/* =============================================================
 * IMPORTS - see docs/design/design_imports.md
 *
 * `import mathkit` means: load stdlib/mathkit.zl, parse it, and SPLICE
 * its top-level definitions into this program, right here. The splice
 * happens at PARSE time, so interp, compile, compilef, compilel and
 * nativegen all get imports without a single backend change - the same
 * trick that made the for-range desugaring free.
 *
 * v1 is the "include" model: no namespaces. The stdlib modules already
 * prefix every function (mk_, ix_, tk_, ...) precisely because a zl
 * definition is global, and that convention is what makes this trivial.
 *
 * Two things this MUST get right:
 *   - the splice targets the PROGRAM node, not a statement. An import
 *     yields N top-level definitions, and a statement slot holds one; a
 *     wrapping block would hide those `fn`s from the backends, whose
 *     top-level loops look for N_FN at depth 0.
 *   - parsing a file while parsing a file clobbers the cursor, because
 *     toks/ntoks/pos/g_hidden are file-scope statics. They are saved and
 *     restored around the nested parse.
 * ============================================================= */

#define MAX_IMPORTS 256
static char g_imported[MAX_IMPORTS][MAX_TEXT];
static int  g_nimported = 0;

static int already_imported(const char *name)
{
    for (int i = 0; i < g_nimported; i++)
        if (strcmp(g_imported[i], name) == 0) return 1;
    return 0;
}

/* Record BEFORE loading, so a cycle (a imports b imports a) terminates
 * rather than recursing until the stack runs out. */
static void mark_imported(const char *name)
{
    if (g_nimported >= MAX_IMPORTS) {
        fprintf(stderr, "parse error: too many imports (max %d)\n", MAX_IMPORTS);
        exit(1);
    }
    strncpy(g_imported[g_nimported], name, MAX_TEXT - 1);
    g_imported[g_nimported][MAX_TEXT - 1] = '\0';
    g_nimported++;
}

static int file_exists(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

/* ./<name>.zl, then ./stdlib/<name>.zl, then $ZL_STDLIB/<name>.zl.
 * First match wins. Writes the winner into `out` and returns 1. */
static int resolve_module(const char *name, char *out, size_t outsz)
{
    snprintf(out, outsz, "%s.zl", name);
    if (file_exists(out)) return 1;
    snprintf(out, outsz, "stdlib/%s.zl", name);
    if (file_exists(out)) return 1;
    const char *env = getenv("ZL_STDLIB");
    if (env && *env) {
        snprintf(out, outsz, "%s/%s.zl", env, name);
        if (file_exists(out)) return 1;
    }
    return 0;
}

static Node *parse_program(void);

/* parse `import a, b, c` and splice each module's top level into prog */
static void parse_import_into(Node *prog)
{
    advance();                                  /* past `import` */
    for (;;) {
        if (curtype() != T_IDENT) {
            fprintf(stderr, "parse error line %d: import needs a module name\n",
                    cur()->line);
            exit(1);
        }
        char name[MAX_TEXT];
        strncpy(name, cur()->text, MAX_TEXT - 1);
        name[MAX_TEXT - 1] = '\0';
        advance();

        if (!already_imported(name)) {
            char path[512];
            if (!resolve_module(name, path, sizeof(path))) {
                const char *env = getenv("ZL_STDLIB");
                fprintf(stderr,
                        "parse error: module '%s' not found (searched ./%s.zl, "
                        "./stdlib/%s.zl", name, name, name);
                if (env && *env) fprintf(stderr, ", %s/%s.zl", env, name);
                fprintf(stderr, ")\n");
                exit(1);
            }
            mark_imported(name);                /* BEFORE loading: breaks cycles */

            /* the nested parse clobbers the cursor - save it */
            Token *save_toks = toks; int save_ntoks = ntoks;
            int save_pos = pos, save_hidden = g_hidden;

            int mcount = 0;
            Token *mtoks = lex_file(path, &mcount);
            Node  *mod   = parse(mtoks, mcount);

            toks = save_toks; ntoks = save_ntoks;
            pos  = save_pos;  g_hidden = save_hidden;

            /* Splice the module's DEFINITIONS - its functions and its
             * top-level constants - and skip everything else.
             *
             * The stdlib modules are written to be runnable on their own,
             * so most of them end in demo output (mathkit alone has 24
             * top-level print calls). Importing a module must not run its
             * demo, and zl has no `if __name__ == "__main__"` to guard it
             * with. Constants cannot be skipped along with it, though -
             * ansi.zl is 19 of them (CSI, RESET, ...) and its functions
             * are useless without them. So: N_FN and N_ASSIGN come in,
             * bare top-level statements do not.
             *
             * The cost: a module whose top level does real side-effecting
             * setup (a `seed(42)`, say) will not get it on import. None of
             * the 108 modules here does, and a module needing that can be
             * given an explicit init function to call. */
            for (int i = 0; i < mod->nkids; i++) {
                NodeType t = mod->kids[i]->type;
                if (t == N_FN || t == N_ASSIGN) add_kid(prog, mod->kids[i]);
            }
        }

        if (match_text(",")) continue;   /* `import a, b, c` */
        break;
    }
}

static Node *parse_program(void)
{
    Node *prog = new_node(N_PROGRAM);
    skip_newlines();
    while (curtype() != T_EOF) {
        if (curtype() == T_KEYWORD && is_text("import")) parse_import_into(prog);
        else add_kid(prog, parse_statement());
        skip_newlines();
    }
    return prog;
}

/* =============================================================
 * PRINTING THE TREE - so we can SEE that it worked
 * ============================================================= */

static void indent(int depth) { for (int i = 0; i < depth; i++) printf("  "); }

void print_node(Node *n, int depth)
{
    if (n == NULL) return;
    indent(depth);

    switch (n->type) {
        case N_PROGRAM:
            printf("PROGRAM\n");
            for (int i = 0; i < n->nkids; i++) print_node(n->kids[i], depth + 1);
            break;
        case N_BLOCK:
            printf("BLOCK\n");
            for (int i = 0; i < n->nkids; i++) print_node(n->kids[i], depth + 1);
            break;
        case N_NUMBER: printf("NUMBER %s\n", n->text); break;
        case N_STRING: printf("STRING \"%s\"\n", n->text); break;
        case N_IDENT:  printf("IDENT %s\n", n->text); break;
        case N_BOOL:   printf("BOOL %s\n", n->text); break;
        case N_LIST:
            printf("LIST\n");
            for (int i = 0; i < n->nkids; i++) print_node(n->kids[i], depth + 1);
            break;
        case N_BINARY:
            printf("BINARY %s\n", n->text);
            print_node(n->a, depth + 1);
            print_node(n->b, depth + 1);
            break;
        case N_UNARY:
            printf("UNARY %s\n", n->text);
            print_node(n->a, depth + 1);
            break;
        case N_TERNARY:
            printf("TERNARY ?:\n");
            indent(depth + 1); printf("cond:\n");
            print_node(n->a, depth + 2);
            indent(depth + 1); printf("then:\n");
            print_node(n->b, depth + 2);
            indent(depth + 1); printf("else:\n");
            print_node(n->c, depth + 2);
            break;
        case N_DANGER:
            printf("DANGER !\n");
            print_node(n->a, depth + 1);
            break;
        case N_CALL:
            printf("CALL\n");
            indent(depth + 1); printf("callee:\n");
            print_node(n->a, depth + 2);
            for (int i = 0; i < n->nkids; i++) {
                indent(depth + 1); printf("arg:\n");
                print_node(n->kids[i], depth + 2);
            }
            break;
        case N_MEMBER:
            printf("MEMBER .%s\n", n->text);
            print_node(n->a, depth + 1);
            break;
        case N_INDEX:
            printf("INDEX\n");
            print_node(n->a, depth + 1);
            print_node(n->b, depth + 1);
            break;
        case N_ASSIGN:
            printf("ASSIGN\n");
            indent(depth + 1); printf("target:\n");
            print_node(n->a, depth + 2);
            indent(depth + 1); printf("value:\n");
            print_node(n->b, depth + 2);
            break;
        case N_IF:
            printf("IF\n");
            indent(depth + 1); printf("cond:\n");
            print_node(n->a, depth + 2);
            indent(depth + 1); printf("then:\n");
            print_node(n->b, depth + 2);
            if (n->c) {
                indent(depth + 1); printf("else:\n");
                print_node(n->c, depth + 2);
            }
            break;
        case N_FOR:
            printf("FOR %s in\n", n->text);
            print_node(n->a, depth + 1);
            print_node(n->b, depth + 1);
            break;
        case N_WHILE:
            printf("WHILE\n");
            print_node(n->a, depth + 1);
            print_node(n->b, depth + 1);
            break;
        case N_FN:
            printf("FN %s\n", n->text);
            for (int i = 0; i < n->nkids; i++) {
                indent(depth + 1); printf("param: %s\n", n->kids[i]->text);
            }
            print_node(n->a, depth + 1);
            break;
        case N_RETURN:
            printf("RETURN\n");
            print_node(n->a, depth + 1);
            break;
        case N_EXPRSTMT:
            printf("EXPR\n");
            print_node(n->a, depth + 1);
            break;
    }
}

/* =============================================================
 * ENTRY POINT - what the interpreter calls
 * ============================================================= */

Node *parse(Token *tokens, int ntokens)
{
    toks    = tokens;
    ntoks   = ntokens;
    pos     = 0;
    g_hidden = 0;
    return parse_program();
}

/* =============================================================
 * MAIN - the standalone parser demo (prints the tree).
 *
 * Switched OFF when building the interpreter (which has its own
 * main), via -DBUILD_INTERP.
 * ============================================================= */

#ifndef BUILD_INTERP
int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: parser <file>\n");
        return 1;
    }

    int    count;
    Token *tokens = lex_file(argv[1], &count);

    Node *program = parse(tokens, count);
    print_node(program, 0);

    return 0;
}
#endif
