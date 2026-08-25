/* compilef.c - the FAST (unboxed) C backend for the NUMERIC subset.
 *
 * The proof-of-concept for "static types -> C speed". compile.c emits
 * boxed `Value` (zl_num, zl_binop, malloc-per-op); this emits raw
 * `long long` C with native operators, so cl.exe optimizes it to real
 * C speed. It handles the numeric subset only (ints, arithmetic,
 * comparisons, if/while, functions, recursion, print) - the case where
 * boxing hurts most.
 *
 *   your.zl  --[compilef]-->  outf.c  --[cl -O2]-->  outf.exe   (C-fast)
 *
 * This is a SEPARATE file: it never touches compile.c, so the
 * self-hosting fixpoint is untouched. It only demonstrates the payoff
 * of unboxing before we build the real static-typed backend.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

typedef struct { char names[512][MAX_TEXT]; int count; } NameSet;
static void set_add(NameSet *s, const char *n) {
    for (int i=0;i<s->count;i++) if(!strcmp(s->names[i],n)) return;
    if (s->count < 512) { strncpy(s->names[s->count],n,MAX_TEXT-1); s->names[s->count][MAX_TEXT-1]='\0'; s->count++; }
}
static int set_has(NameSet *s, const char *n) {
    for (int i=0;i<s->count;i++) if(!strcmp(s->names[i],n)) return 1;
    return 0;
}
static NameSet g_funcs, g_globals;

static void collect_vars(Node *n, NameSet *out) {
    if (!n) return;
    if (n->type == N_ASSIGN && n->a->type == N_IDENT) set_add(out, n->a->text);
    if (n->type == N_FOR) set_add(out, n->text);
    for (int i=0;i<n->nkids;i++) collect_vars(n->kids[i], out);
    collect_vars(n->a, out); collect_vars(n->b, out); collect_vars(n->c, out);
}

/* just the loop variables: a loop BINDS its variable, so it stays local
 * even when a global shares the name (same rule as compile.c/interp.c). */
static void collect_loop_vars(Node *n, NameSet *out) {
    if (!n) return;
    if (n->type == N_FN) return;
    if (n->type == N_FOR) set_add(out, n->text);
    for (int i=0;i<n->nkids;i++) collect_loop_vars(n->kids[i], out);
    collect_loop_vars(n->a, out); collect_loop_vars(n->b, out); collect_loop_vars(n->c, out);
}

static FILE *out;
static void emit_expr(Node *n);

static void emit_expr(Node *n) {
    switch (n->type) {
        case N_NUMBER: fprintf(out, "%lldLL", (long long)atoll(n->text)); break;
        case N_BOOL:   fprintf(out, "%d", strcmp(n->text,"true")==0);     break;
        case N_IDENT:  fprintf(out, "v_%s", n->text);                     break;
        case N_UNARY:
            fputc('(', out);
            fputs(!strcmp(n->text,"-") ? "-" : "!", out);
            emit_expr(n->a);
            fputc(')', out);
            break;
        case N_BINARY: {
            const char *op = n->text, *c = op;
            if (!strcmp(op,"and")) c = "&&";
            else if (!strcmp(op,"or")) c = "||";
            fputc('(', out); emit_expr(n->a); fprintf(out," %s ", c); emit_expr(n->b); fputc(')', out);
            break;
        }
        case N_CALL:
            fprintf(out, "zl_fn_%s(", n->a->text);
            for (int i=0;i<n->nkids;i++){ if(i) fputs(", ", out); emit_expr(n->kids[i]); }
            fputc(')', out);
            break;
        default:
            fprintf(stderr, "compilef: expression not in the numeric subset (type %d)\n", n->type);
            exit(1);
    }
}

static void emit_block(Node *b);

static void emit_stmt(Node *n) {
    switch (n->type) {
        /* n->text holds the operator for a compound `op=`, and is empty
         * for a plain `=` (the parser no longer desugars op= into
         * `x = x op v`, so this has to apply the operator itself). */
        case N_ASSIGN:
            fprintf(out, "  v_%s %s= ", n->a->text, n->text);
            emit_expr(n->b); fputs(";\n", out);
            break;
        case N_IF:
            fputs("  if (", out); emit_expr(n->a); fputs(") {\n", out);
            emit_block(n->b);
            fputs("  }", out);
            if (n->c && n->c->nkids) { fputs(" else {\n", out); emit_block(n->c); fputs("  }", out); }
            fputc('\n', out);
            break;
        case N_WHILE:
            fputs("  while (", out); emit_expr(n->a); fputs(") {\n", out);
            emit_block(n->b);
            fputs("  }\n", out);
            break;
        case N_RETURN:
            fputs("  return ", out); if (n->a) emit_expr(n->a); else fputs("0", out); fputs(";\n", out);
            break;
        case N_EXPRSTMT: {
            Node *e = n->a;
            if (e->type == N_CALL && e->a->type == N_IDENT && !strcmp(e->a->text,"print")) {
                fputs("  printf(\"%lld\\n\", (long long)(", out); emit_expr(e->kids[0]); fputs("));\n", out);
                break;
            }
            fputs("  ", out); emit_expr(e); fputs(";\n", out);
            break;
        }
        default:
            fprintf(stderr, "compilef: statement not in the numeric subset (type %d)\n", n->type);
            exit(1);
    }
}

static void emit_block(Node *b) {
    if (!b) return;
    for (int i=0;i<b->nkids;i++) emit_stmt(b->kids[i]);
}

static void emit_function(Node *fn) {
    fprintf(out, "long long zl_fn_%s(", fn->text);
    if (fn->nkids == 0) fputs("void", out);
    for (int i=0;i<fn->nkids;i++){ if(i) fputs(", ", out); fprintf(out, "long long v_%s", fn->kids[i]->text); }
    fputs(") {\n", out);
    NameSet locals; locals.count = 0;
    collect_vars(fn->a, &locals);
    NameSet loopvars; loopvars.count = 0;
    collect_loop_vars(fn->a, &loopvars);
    for (int i=0;i<locals.count;i++) {
        int isp = 0;
        for (int p=0;p<fn->nkids;p++) if(!strcmp(locals.names[i], fn->kids[p]->text)) isp = 1;
        if (!isp && (!set_has(&g_globals, locals.names[i])
                     || set_has(&loopvars, locals.names[i])))
            fprintf(out, "  long long v_%s = 0;\n", locals.names[i]);
    }
    emit_block(fn->a);
    fputs("  return 0;\n}\n\n", out);
}

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "usage: compilef <file.zl>\n"); return 1; }
    int count;
    Token *toks = lex_file(argv[1], &count);
    Node  *prog = parse(toks, count);

    g_funcs.count = 0;
    for (int i=0;i<prog->nkids;i++) if (prog->kids[i]->type == N_FN) set_add(&g_funcs, prog->kids[i]->text);

    out = fopen("outf.c", "wb");
    if (!out) { fprintf(stderr, "can't write outf.c\n"); return 1; }

    fputs("/* GENERATED by compilef - unboxed numeric */\n#include <stdio.h>\n\n", out);
    for (int i=0;i<prog->nkids;i++)
        if (prog->kids[i]->type == N_FN) {
            fprintf(out, "long long zl_fn_%s(", prog->kids[i]->text);
            if (prog->kids[i]->nkids == 0) fputs("void", out);
            for (int p=0;p<prog->kids[i]->nkids;p++){ if(p) fputs(", ", out); fputs("long long", out); }
            fputs(");\n", out);
        }
    fputc('\n', out);

    g_globals.count = 0;
    for (int i=0;i<prog->nkids;i++) if (prog->kids[i]->type != N_FN) collect_vars(prog->kids[i], &g_globals);
    for (int i=0;i<g_globals.count;i++) fprintf(out, "long long v_%s = 0;\n", g_globals.names[i]);
    fputc('\n', out);

    for (int i=0;i<prog->nkids;i++) if (prog->kids[i]->type == N_FN) emit_function(prog->kids[i]);

    fputs("int main(void) {\n", out);
    for (int i=0;i<prog->nkids;i++) if (prog->kids[i]->type != N_FN) emit_stmt(prog->kids[i]);
    fputs("  return 0;\n}\n", out);
    fclose(out);
    printf("compilef: wrote outf.c (unboxed long long)\n");
    return 0;
}
