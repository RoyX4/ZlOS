/* compile.c - stage 4: the COMPILER.
 *
 * The interpreter WALKS the tree and does it, live, every run.
 * The compiler instead WALKS the tree once and WRITES OUT a C
 * program (out.c) that does the same thing. That C is then handed
 * to the real C compiler, producing a standalone .exe that runs on
 * its own - no interpreter needed.
 *
 *   your.zl  --[this]-->  out.c  --[cl]-->  out.exe
 *
 * The front end (lexer + parser) is IDENTICAL to the interpreter's.
 * Only this last stage is different. That's the whole point: all
 * the earlier work is reused, we just changed the ending.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

/* =============================================================
 * A tiny set of names (for tracking variables and functions)
 * ============================================================= */

/* NAMESET_MAX was 256 and set_add() silently drops anything past it - no
 * error, no truncation warning, just a name that never joins the set. That is
 * exactly what a zlOS app suite hit: kernel.zl plus its app-suite imports
 * (apps_registry.zl and the category modules) pushed the GLOBAL-VARIABLE
 * count past 256, so main()'s own pre-existing shell loop locals (pending,
 * crow, ccol, ...) silently stopped being recognised as globals and gcc
 * reported them "undeclared" - a cap inside the COMPILER surfacing as a
 * compile error in code that never changed. Same shape as the maze's
 * undersized shared board this suite's own brief warns about: a fixed size
 * that was fine at the old scale and silently wrong at the new one. 1024 is
 * headroom, not a guess - the kernel is nowhere near it either way, and
 * set_add()'s bound stops it from ever overflowing the array again. */
#define NAMESET_MAX 1024

typedef struct {
    char names[NAMESET_MAX][MAX_TEXT];
    int  count;
} NameSet;

static void set_add(NameSet *s, const char *name)
{
    for (int i = 0; i < s->count; i++)
        if (strcmp(s->names[i], name) == 0) return;   /* already there */
    if (s->count < NAMESET_MAX) {
        strncpy(s->names[s->count], name, MAX_TEXT - 1);
        s->names[s->count][MAX_TEXT - 1] = '\0';
        s->count++;
    }
}

static int set_has(NameSet *s, const char *name)
{
    for (int i = 0; i < s->count; i++)
        if (strcmp(s->names[i], name) == 0) return 1;
    return 0;
}

static int set_index(NameSet *s, const char *name)
{
    for (int i = 0; i < s->count; i++)
        if (strcmp(s->names[i], name) == 0) return i;
    return -1;
}

/* the names of every user-defined function - so a CALL can tell a
 * real function apart from a built-in like print/dir. */
static NameSet g_funcs;

/* each function's parameter count, index-parallel to g_funcs. Needed to
 * emit a function VALUE: zl_callv has to cast the pointer back to the
 * exact signature, so the arity travels with the pointer. */
static int g_func_arity[NAMESET_MAX];

/* the names bound in the scope being emitted (params + locals, or the
 * top-level's globals). A call to a name in here is an INDIRECT call
 * through a variable holding a function value, not a builtin. */
static NameSet *g_scope = 0;

/* the names of every top-level (global) variable. A function that
 * assigns one of these should MUTATE the global, not make a local
 * copy - so we must NOT re-declare it as a local. This matches the
 * interpreter, and is what lets a parser share a `pos` cursor. */
static NameSet g_globals;

/* =============================================================
 * Collecting the variable names that need declaring.
 *
 * In C, every variable must be declared. So before emitting a
 * function body (or main), we scan it for every name that gets
 * assigned or used as a loop variable, and declare them all up top.
 * We do NOT descend into nested functions - they have their own scope.
 * ============================================================= */

static void collect_vars(Node *n, NameSet *out)
{
    if (n == NULL) return;
    switch (n->type) {
        case N_ASSIGN:
            if (n->a->type == N_IDENT) set_add(out, n->a->text);
            collect_vars(n->b, out);
            break;
        case N_FOR:
            set_add(out, n->text);          /* the loop variable */
            collect_vars(n->a, out);
            collect_vars(n->b, out);
            break;
        case N_WHILE:
            collect_vars(n->a, out);
            collect_vars(n->b, out);
            break;
        case N_IF:
            collect_vars(n->a, out);
            collect_vars(n->b, out);
            collect_vars(n->c, out);
            break;
        case N_BLOCK:
        case N_PROGRAM:
            for (int i = 0; i < n->nkids; i++) collect_vars(n->kids[i], out);
            break;
        case N_FN:
            /* do NOT recurse - nested function has its own scope */
            break;
        default:
            /* expressions can't declare variables; nothing to do */
            break;
    }
}

/* just the loop variables, same walk. They need telling apart from the
 * names a body merely assigns, because a loop variable is BOUND by the
 * loop and so stays local even when a global shares its name. */
static void collect_loop_vars(Node *n, NameSet *out)
{
    if (n == NULL) return;
    if (n->type == N_FN) return;        /* nested function: its own scope */
    if (n->type == N_FOR) set_add(out, n->text);
    for (int i = 0; i < n->nkids; i++) collect_loop_vars(n->kids[i], out);
    collect_loop_vars(n->a, out);
    collect_loop_vars(n->b, out);
    collect_loop_vars(n->c, out);
}

/* =============================================================
 * Emitting C
 * ============================================================= */

/* write a zl string as a valid C string literal, escaping specials */
static void emit_c_string(FILE *out, const char *s)
{
    fputc('"', out);
    for (const char *p = s; *p; p++) {
        switch (*p) {
            case '"':  fputs("\\\"", out); break;
            case '\\': fputs("\\\\", out); break;
            case '\n': fputs("\\n", out);  break;
            case '\t': fputs("\\t", out);  break;
            case '\r': fputs("\\r", out);  break;
            default:   fputc(*p, out);     break;
        }
    }
    fputc('"', out);
}

/* Does this subtree use the 'in' operator anywhere?
 *
 * Only programs that answer yes get the zl_in helper written into
 * out.c. That keeps the generated C for every other program EXACTLY
 * what it was before 'in' existed - including compiler.zl's, whose
 * byte-for-byte output is the self-hosting fixpoint. */
static int uses_in(Node *n)
{
    if (n == NULL) return 0;
    if (n->type == N_BINARY && strcmp(n->text, "in") == 0) return 1;
    if (uses_in(n->a) || uses_in(n->b) || uses_in(n->c)) return 1;
    for (int i = 0; i < n->nkids; i++)
        if (uses_in(n->kids[i])) return 1;
    return 0;
}

/* 'in' is the only operator whose meaning depends on the runtime type
 * of an operand, so it can't be a plain zl_binop string. This helper
 * reuses the existing contains()/has() built-ins - the container is on
 * the right in zl but comes first in both built-ins, hence the swap. */
static void emit_in_helper(FILE *out)
{
    fputs("/* x in xs -> contains(xs, x);  sub in text -> has(text, sub) */\n", out);
    fputs("static Value zl_in(Value l, Value r) {\n", out);
    fputs("    if (r.type == V_LIST) return zl_calln(\"contains\", 2, r, l);\n", out);
    fputs("    return zl_calln(\"has\", 2, r, l);\n", out);
    fputs("}\n\n", out);
}

static void emit_expr(FILE *out, Node *n);

/* Emit `head(extra_before, arg0, arg1, ...)` but force LEFT-TO-RIGHT
 * evaluation of arg0..argN-1 via a GNU statement-expression. C leaves the
 * evaluation order of a function call's arguments unspecified, and gcc's
 * order can differ from MSVC's - this bit for real on Linux: a list literal
 * built from side-effecting calls, e.g. [dq_pop_front(q), dq_pop_front(q)],
 * came out reversed under gcc even though the zl language (like the
 * interpreter) evaluates left to right. Sequencing each argument into its
 * own temporary before the call removes the ambiguity outright. extra_before
 * is a pre-formatted prefix (e.g. a quoted builtin name + arg count), or
 * NULL for none. */
static int g_tmp = 0;
static void emit_seq_call(FILE *out, const char *head, const char *extra_before, Node **kids, int nkids)
{
    fputs("({ ", out);
    int base = g_tmp;
    for (int i = 0; i < nkids; i++) {
        fprintf(out, "Value _t%d = ", base + i);
        emit_expr(out, kids[i]);
        fputs("; ", out);
    }
    g_tmp += nkids;
    fputs(head, out); fputc('(', out);
    if (extra_before) fputs(extra_before, out);
    for (int i = 0; i < nkids; i++) {
        if (i || extra_before) fputs(", ", out);
        fprintf(out, "_t%d", base + i);
    }
    fputs("); })", out);
}

/* emit a call node - user function or built-in */
static void emit_call(FILE *out, Node *n)
{
    if (n->a->type != N_IDENT) {
        fprintf(stderr, "compile: can only call a plain name\n");
        exit(1);
    }
    const char *name = n->a->text;

    /* A name bound as a VARIABLE in this scope shadows everything else: it
     * holds a function value at run time, so the call goes indirect through
     * zl_callv. This is what makes `fn ix_sort_by(xs, key) { ... key(x) ... }`
     * work - `key` is a parameter, not a known function name. */
    if (g_scope && set_has(g_scope, name)) {
        char extra[MAX_TEXT + 16];
        snprintf(extra, sizeof(extra), "v_%s, %d", name, n->nkids);
        emit_seq_call(out, "zl_callv", extra, n->kids, n->nkids);
    } else if (set_has(&g_funcs, name)) {
        /* a real user function -> direct C call: zl_fn_name(a, b) */
        char head[MAX_TEXT + 8];
        snprintf(head, sizeof(head), "zl_fn_%s", name);
        emit_seq_call(out, head, NULL, n->kids, n->nkids);
    } else {
        /* a built-in -> zl_calln("name", count, a, b) */
        char extra[MAX_TEXT + 16];
        int p = snprintf(extra, sizeof(extra), "\"");
        for (const char *s = name; *s && p < (int)sizeof(extra) - 8; s++) {
            if (*s == '"' || *s == '\\') extra[p++] = '\\';
            extra[p++] = *s;
        }
        p += snprintf(extra + p, sizeof(extra) - (size_t)p, "\", %d", n->nkids);
        emit_seq_call(out, "zl_calln", extra, n->kids, n->nkids);
    }
}

/* every expression becomes a C expression that produces a Value */
static void emit_expr(FILE *out, Node *n)
{
    switch (n->type) {
        case N_NUMBER: fprintf(out, "zl_num(%s)", n->text); break;
        case N_BOOL:   fprintf(out, "zl_bool(%d)", strcmp(n->text, "true") == 0); break;
        case N_STRING:
            fprintf(out, "zl_str(");
            emit_c_string(out, n->text);
            fputc(')', out);
            break;
        /* variable read - unless the name is a user FUNCTION not shadowed by
         * a variable in scope, in which case reading it yields a function
         * VALUE (the compiled equivalent of the interpreter's V_FN). That is
         * what lets a function be passed as an argument. */
        case N_IDENT:
            if ((!g_scope || !set_has(g_scope, n->text)) && set_has(&g_funcs, n->text))
                fprintf(out, "zl_fn((void*)zl_fn_%s, %d)",
                        n->text, g_func_arity[set_index(&g_funcs, n->text)]);
            else
                fprintf(out, "v_%s", n->text);
            break;

        case N_LIST: {
            char extra[16];
            snprintf(extra, sizeof(extra), "%d", n->nkids);
            emit_seq_call(out, "zl_list_n", extra, n->kids, n->nkids);
            break;
        }

        case N_BINARY:
            /* `and` and `or` SHORT-CIRCUIT, so they cannot go through
             * zl_binop - that is a function call, and C evaluates both
             * of its arguments. C's own && and || are the short-circuit,
             * exactly as the ternary below uses C's ?:. Matches the
             * interpreter, which special-cases these in eval(). */
            if (strcmp(n->text, "and") == 0 || strcmp(n->text, "or") == 0) {
                fputs("zl_bool(zl_truthy(", out);
                emit_expr(out, n->a);
                fputs(strcmp(n->text, "and") == 0 ? ") && zl_truthy(" : ") || zl_truthy(", out);
                emit_expr(out, n->b);
                fputs("))", out);
                break;
            }
            if (strcmp(n->text, "in") == 0) {
                /* not a zl_binop: 'in' picks contains() or has() by the
                 * runtime type of the right operand. See emit_in_helper. */
                fputs("zl_in(", out);
                emit_expr(out, n->a);
                fputs(", ", out);
                emit_expr(out, n->b);
                fputc(')', out);
                break;
            }
            fprintf(out, "zl_binop(");
            emit_c_string(out, n->text);
            fputs(", ", out); emit_expr(out, n->a);
            fputs(", ", out); emit_expr(out, n->b);
            fputc(')', out);
            break;

        case N_UNARY:
            fprintf(out, "zl_unop(");
            emit_c_string(out, n->text);
            fputs(", ", out); emit_expr(out, n->a);
            fputc(')', out);
            break;

        case N_TERNARY:
            /* C's own ?: is the short-circuit. The untaken branch is
             * never evaluated, exactly as in the interpreter. */
            fputs("(zl_truthy(", out); emit_expr(out, n->a);
            fputs(") ? ", out);       emit_expr(out, n->b);
            fputs(" : ", out);        emit_expr(out, n->c);
            fputc(')', out);
            break;

        case N_DANGER:              /* the '!' marker: no runtime effect */
            emit_expr(out, n->a);
            break;

        case N_CALL:  emit_call(out, n); break;

        case N_INDEX:
            fprintf(out, "zl_index(");
            emit_expr(out, n->a);
            fputs(", ", out);
            emit_expr(out, n->b);
            fputc(')', out);
            break;

        default:
            fprintf(stderr, "compile: cannot emit this expression\n");
            exit(1);
    }
}

static void emit_stmt(FILE *out, Node *n, int indent);

static void emit_block(FILE *out, Node *block, int indent)
{
    for (int i = 0; i < block->nkids; i++)
        emit_stmt(out, block->kids[i], indent);
}

static void pad(FILE *out, int indent) { for (int i = 0; i < indent; i++) fputs("    ", out); }

static void emit_stmt(FILE *out, Node *n, int indent)
{
    switch (n->type) {
        case N_EXPRSTMT:
            pad(out, indent); emit_expr(out, n->a); fputs(";\n", out);
            break;

        /* n->text is empty for a plain `=`, and holds the operator for a
         * compound `op=`. For an index target the compound form puts the
         * container and the subscript in temporaries first, so a
         * subscript with a side effect runs ONCE - matching the
         * interpreter, which reads the element once too. */
        case N_ASSIGN:
            pad(out, indent);
            if (n->a->type == N_INDEX) {              /* x[i] = v */
                if (n->text[0]) {
                    fputs("{ Value _c = ", out); emit_expr(out, n->a->a);
                    fputs("; Value _i = ", out);  emit_expr(out, n->a->b);
                    fputs("; zl_set(_c, _i, zl_binop(", out);
                    emit_c_string(out, n->text);
                    fputs(", zl_index(_c, _i), ", out);
                    emit_expr(out, n->b);
                    fputs(")); }\n", out);
                } else {
                    fputs("zl_set(", out);
                    emit_expr(out, n->a->a);          /* the list  */
                    fputs(", ", out);
                    emit_expr(out, n->a->b);          /* the index */
                    fputs(", ", out);
                    emit_expr(out, n->b);             /* the value */
                    fputs(");\n", out);
                }
            } else if (n->text[0]) {
                fprintf(out, "v_%s = zl_binop(", n->a->text);
                emit_c_string(out, n->text);
                fprintf(out, ", v_%s, ", n->a->text);
                emit_expr(out, n->b);
                fputs(");\n", out);
            } else {
                fprintf(out, "v_%s = ", n->a->text);
                emit_expr(out, n->b);
                fputs(";\n", out);
            }
            break;

        case N_IF:
            pad(out, indent); fputs("if (zl_truthy(", out); emit_expr(out, n->a); fputs(")) {\n", out);
            emit_block(out, n->b, indent + 1);
            pad(out, indent); fputs("}", out);
            if (n->c) { fputs(" else {\n", out); emit_block(out, n->c, indent + 1); pad(out, indent); fputs("}", out); }
            fputs("\n", out);
            break;

        case N_FOR: {
            /* for v in seq {...}  ->  loop over the list's items */
            pad(out, indent); fputs("{\n", out);
            pad(out, indent + 1); fputs("Value _seq = ", out); emit_expr(out, n->a); fputs(";\n", out);
            pad(out, indent + 1);
            fprintf(out, "for (int _i = 0; _i < zl_len_list(_seq); _i++) {\n");
            pad(out, indent + 2); fprintf(out, "v_%s = zl_item(_seq, _i);\n", n->text);
            emit_block(out, n->b, indent + 2);
            pad(out, indent + 1); fputs("}\n", out);
            pad(out, indent); fputs("}\n", out);
            break;
        }

        case N_WHILE:
            pad(out, indent); fputs("while (zl_truthy(", out); emit_expr(out, n->a); fputs(")) {\n", out);
            emit_block(out, n->b, indent + 1);
            pad(out, indent); fputs("}\n", out);
            break;

        case N_RETURN:
            pad(out, indent); fputs("return ", out);
            if (n->a) emit_expr(out, n->a); else fputs("zl_nil()", out);
            fputs(";\n", out);
            break;

        case N_BLOCK:
            emit_block(out, n, indent);
            break;

        case N_BREAK:    pad(out, indent); fputs("break;\n", out);    break;
        case N_CONTINUE: pad(out, indent); fputs("continue;\n", out); break;

        default:
            pad(out, indent); emit_expr(out, n); fputs(";\n", out);
            break;
    }
}

/* emit one user function definition */
static void emit_function(FILE *out, Node *fn)
{
    /* signature: Value zl_fn_name(Value v_p0, Value v_p1) */
    fprintf(out, "Value zl_fn_%s(", fn->text);
    if (fn->nkids == 0) fputs("void", out);
    for (int i = 0; i < fn->nkids; i++) {
        if (i) fputs(", ", out);
        fprintf(out, "Value v_%s", fn->kids[i]->text);
    }
    fputs(") {\n", out);

    /* declare the local variables (assigned names, minus the params) */
    NameSet locals; locals.count = 0;
    collect_vars(fn->a, &locals);
    /* a for-loop variable BINDS, like a parameter: the loop introduces
     * the name, so it is declared here even when a global shares it,
     * instead of writing through to that global. Matches the
     * interpreter, which defines the body's loop variables in the call
     * scope (see define_loop_vars in interp.c). */
    NameSet loopvars; loopvars.count = 0;
    collect_loop_vars(fn->a, &loopvars);
    for (int i = 0; i < locals.count; i++) {
        int is_param = 0;
        for (int p = 0; p < fn->nkids; p++)
            if (strcmp(locals.names[i], fn->kids[p]->text) == 0) is_param = 1;
        /* skip params (already declared as args) and globals (shared) */
        if (!is_param && (!set_has(&g_globals, locals.names[i])
                          || set_has(&loopvars, locals.names[i])))
            fprintf(out, "    Value v_%s = zl_nil();\n", locals.names[i]);
    }

    /* the names this body can see as VARIABLES: params, its own locals, and
     * the shared globals. A call to one of these is an indirect call through
     * a function value; a call to anything else is a direct call or builtin. */
    NameSet scope; scope.count = 0;
    for (int p = 0; p < fn->nkids; p++) set_add(&scope, fn->kids[p]->text);
    for (int i = 0; i < locals.count; i++)  set_add(&scope, locals.names[i]);
    for (int i = 0; i < g_globals.count; i++) set_add(&scope, g_globals.names[i]);
    g_scope = &scope;

    emit_block(out, fn->a, 1);
    fputs("    return zl_nil();\n", out);   /* fall-through result */
    fputs("}\n\n", out);
    g_scope = 0;
}

/* emit the function prototype (so functions can call each other / recurse) */
static void emit_prototype(FILE *out, Node *fn)
{
    fprintf(out, "Value zl_fn_%s(", fn->text);
    if (fn->nkids == 0) fputs("void", out);
    for (int i = 0; i < fn->nkids; i++) {
        if (i) fputs(", ", out);
        fputs("Value", out);
    }
    fputs(");\n", out);
}

/* =============================================================
 * MAIN - drive the whole compile
 * ============================================================= */

int main(int argc, char **argv)
{
    if (argc < 2) { fprintf(stderr, "usage: compile <file.zl>\n"); return 1; }

    int    count;
    Token *tokens  = lex_file(argv[1], &count);
    Node  *program = parse(tokens, count);

    /* pass 1: find every user function name, and record its arity so a
     * function used as a value can be cast back to the right signature */
    g_funcs.count = 0;
    for (int i = 0; i < program->nkids; i++)
        if (program->kids[i]->type == N_FN) {
            set_add(&g_funcs, program->kids[i]->text);
            int fi = set_index(&g_funcs, program->kids[i]->text);
            if (fi >= 0 && fi < NAMESET_MAX) g_func_arity[fi] = program->kids[i]->nkids;
        }

    FILE *out = fopen("out.c", "wb");
    if (!out) { fprintf(stderr, "can't write out.c\n"); return 1; }

    fputs("/* GENERATED by my compiler - do not edit */\n", out);
    fputs("#include \"runtime.h\"\n\n", out);

    if (uses_in(program)) emit_in_helper(out);

    /* function prototypes */
    for (int i = 0; i < program->nkids; i++)
        if (program->kids[i]->type == N_FN)
            emit_prototype(out, program->kids[i]);
    fputs("\n", out);

    /* global variables = top-level assigned names.
     * Collected into g_globals BEFORE emitting functions, so
     * emit_function knows which names are shared globals. */
    g_globals.count = 0;
    for (int i = 0; i < program->nkids; i++)
        if (program->kids[i]->type != N_FN)
            collect_vars(program->kids[i], &g_globals);
    for (int i = 0; i < g_globals.count; i++)
        fprintf(out, "Value v_%s;\n", g_globals.names[i]);
    fputs("\n", out);

    /* function definitions */
    for (int i = 0; i < program->nkids; i++)
        if (program->kids[i]->type == N_FN)
            emit_function(out, program->kids[i]);

    /* main() = the top-level statements (skipping function defs). At the top
     * level the globals are the variables in scope. */
    g_scope = &g_globals;
    fputs("int main(void) {\n", out);
    for (int i = 0; i < program->nkids; i++)
        if (program->kids[i]->type != N_FN)
            emit_stmt(out, program->kids[i], 1);
    fputs("    return 0;\n}\n", out);
    g_scope = 0;

    fclose(out);
    printf("wrote out.c\n");
    return 0;
}
