/* nativegen.c - Floor 4: a REAL x86-64 backend (now with functions).
 *
 * Walks the AST of a zl program and GENERATES x86-64 machine code,
 * then wraps it in a PE .exe. No C compiler touches the output.
 *
 * Integer subset: variables, + - * / %, comparisons, and/or/not,
 * if/else, while, user functions + recursion, and exit(expr) which
 * returns the value as the process exit code (verify via %ERRORLEVEL%).
 *
 * Strategy: STACK MACHINE (result in rax, operands push/pop). Jumps
 * and calls use BACKPATCHING. My own calling convention: caller
 * pushes args right-to-left, callee returns result in rax, caller
 * cleans up the args.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "lexer.h"
#include "parser.h"

/* ---- code buffer ---- */
static unsigned char code[1 << 20];
static int codelen = 0;

static void backend_limit(const char *what)
{
    fprintf(stderr, "native: %s limit exceeded\n", what);
    exit(1);
}

static void b(unsigned char x)
{
    if (codelen >= (int)sizeof(code)) backend_limit("1 MiB code buffer");
    code[codelen++] = x;
}
static void b4(int v) { for (int i=0;i<4;i++) b((unsigned char)((v>>(8*i))&0xFF)); }
static void bytes(const unsigned char *p, int n) { for (int i=0;i<n;i++) b(p[i]); }
static void patch4(int at, int v) { for (int i=0;i<4;i++) code[at+i]=(unsigned char)((v>>(8*i))&0xFF); }

/* ---- functions: name -> code offset, plus call-site fixups ---- */
static char fnames[128][MAX_TEXT];
static int  foffset[128];
static int  nfuncs = 0;
static int  fn_index(const char *name) { for(int i=0;i<nfuncs;i++) if(!strcmp(fnames[i],name)) return i; return -1; }

static struct { int pos; char name[MAX_TEXT]; } fixups[4096];
static int nfixups = 0;

/* print(int) support: a hand-assembled print_int routine (itoa +
 * WriteFile). Call sites are backpatched to its offset. */
static int print_int_off = -1;
static int pfix[4096];
static int npfix = 0;

/* print(num) support: a print_num routine that reproduces interp.c's
 * value_to_string for a double - whole numbers print through the integer
 * path, and (for now) a fractional number is refused at run time rather
 * than printed wrong. Call sites are backpatched to its offset. */
static int print_num_off = -1;
static int pnfix[4096];
static int npnfix = 0;

/* print("literal") support: a print_str routine + string data. */
static int print_str_off = -1;
static int psfix[4096];              /* call sites -> print_str          */
static int npsfix = 0;
static struct { int leapos; char text[256]; int len; } sfix[1024]; /* lea disps -> string data */
static int nsfix = 0;

/* Linux port: no import table needed at all - stdout write and process exit
 * are raw syscalls (SYS_write=1, SYS_exit=60), issued with the `syscall`
 * instruction. This replaces the Windows build's kernel32.dll IAT calls
 * (GetStdHandle/WriteFile/ExitProcess) everywhere they were used. */

/* ---- current scope: params (positive rbp offsets) + locals (negative) ---- */
static char params[64][MAX_TEXT]; static int nparams = 0;
static char locals[256][MAX_TEXT]; static int nlocals = 0;

static int local_slot(const char *name)
{
    for (int i=0;i<nlocals;i++) if(!strcmp(locals[i],name)) return i;
    if (nlocals >= (int)(sizeof(locals) / sizeof(locals[0])))
        backend_limit("local variable");
    strncpy(locals[nlocals], name, MAX_TEXT-1); locals[nlocals][MAX_TEXT-1]='\0';
    return nlocals++;
}
/* rbp-relative displacement for a variable */
static int var_disp(const char *name)
{
    for (int i=0;i<nparams;i++) if(!strcmp(params[i],name)) return 16 + 8*i;  /* args above ret addr */
    return -8 * (local_slot(name) + 1);                                       /* locals below rbp   */
}

/* ---- enclosing loops, for break/continue ----
 * break jumps FORWARD to the loop end (backpatched once the end offset is
 * known); continue jumps BACKWARD to the loop's continue point, which for a
 * while loop is the condition test at the top (re-testing it, exactly like
 * interp.c's while re-evaluates the guard each iteration). A stack, so nested
 * loops work. Only N_WHILE pushes here - N_FOR is not in the native subset. */
#define MAX_LOOPS 64
#define MAX_BREAKS 256
static int loop_cont[MAX_LOOPS];               /* continue target code offset       */
static int loop_breaks[MAX_LOOPS][MAX_BREAKS]; /* break rel32 disp sites to patch    */
static int loop_nbreaks[MAX_LOOPS];
static int nloops = 0;

/* collect the local variables a body assigns (excluding params) */
static void collect_locals(Node *n)
{
    if (!n) return;
    if (n->type == N_ASSIGN && n->a->type == N_IDENT) var_disp(n->a->text);
    if (n->type == N_FOR) var_disp(n->text);
    for (int i=0;i<n->nkids;i++) collect_locals(n->kids[i]);
    collect_locals(n->a); collect_locals(n->b); collect_locals(n->c);
}

/* =====================================================================
 * NUMBER TYPES - INT (i64) or NUM (double), inferred to a fixpoint.
 *
 * zl has ONE number type and it is a double (interp.c's Value.num). A value
 * that is PROVABLY whole is machined as an i64 for speed; anything else is a
 * double. That is exactly the rule the LLVM backend (compilel.c) settled on -
 * INT < NUM, widening is cvtsi2sd, and NUM never narrows back - and the two
 * fast backends must agree, so this copies it. compilel's strings, lists and
 * exact-division refinement are not here (this backend has none of them), so
 * this is that rule with only numbers left in it.
 *
 *   literal with a '.'          NUM        the lexer's only float form
 *   literal without one         INT
 *   true / false                INT        (0 / 1)
 *   comparison, and, or, not    INT        (0 / 1)
 *   -x                          same as x
 *   + - *                       NUM if either side is NUM, else INT
 *   /                           NUM, ALWAYS  (so 7 / 3 is 2.33333)
 *   call f(...)                 f's return type
 *   variable                    the JOIN of everything assigned to it
 *
 * A parameter's type is the join of every argument passed at any call site,
 * a return type the join of everything returned; those feed each other, so
 * infer_types runs to a fixpoint. Everything starts INT and only rises. */
typedef int Ty;
#define T_INT 0
#define T_NUM 1

typedef struct {
    Node *node;                  /* the N_FN (0 for the top level)        */
    char  vnames[256][MAX_TEXT]; /* params first (nparams), then locals   */
    Ty    vty[256];
    int   nvars;
    int   nparams;
    Ty    ret;
} TyFn;
static TyFn tyfns[128];   /* index-parallel to fnames/foffset           */
static TyFn tytop;        /* the top-level scope                        */
static TyFn *cur_ty = 0;  /* whose names resolve during infer and emit  */

static int is_cmp_op(const char *op)
{
    return !strcmp(op,"==")||!strcmp(op,"!=")||!strcmp(op,"<")||
           !strcmp(op,">") ||!strcmp(op,"<=")||!strcmp(op,">=");
}
static int tyf_find(TyFn *f, const char *name)
{
    for (int i=0;i<f->nvars;i++) if(!strcmp(f->vnames[i],name)) return i;
    return -1;
}
static void tyf_add(TyFn *f, const char *name)
{
    if (tyf_find(f,name) >= 0 || f->nvars >= 256) return;
    strncpy(f->vnames[f->nvars], name, MAX_TEXT-1); f->vnames[f->nvars][MAX_TEXT-1]='\0';
    f->vty[f->nvars] = T_INT; f->nvars++;
}
/* the names a scope binds - params (added by the caller) plus everything it
 * assigns, exactly what collect_locals gives var_disp, so the two scopes see
 * the same set of names. */
static void collect_ty_vars(TyFn *f, Node *n)
{
    if (!n) return;
    if (n->type==N_ASSIGN && n->a->type==N_IDENT) tyf_add(f, n->a->text);
    if (n->type==N_FOR) tyf_add(f, n->text);
    for (int i=0;i<n->nkids;i++) collect_ty_vars(f, n->kids[i]);
    collect_ty_vars(f,n->a); collect_ty_vars(f,n->b); collect_ty_vars(f,n->c);
}
static Ty var_ty(TyFn *f, const char *name)
{
    if (f) { int i=tyf_find(f,name); if (i>=0) return f->vty[i]; }
    return T_INT;                                   /* unknown - an int */
}
static Ty expr_ty(TyFn *f, Node *n)
{
    if (!n) return T_INT;
    switch (n->type) {
        case N_NUMBER: return strchr(n->text,'.') ? T_NUM : T_INT;
        case N_BOOL:   return T_INT;
        case N_IDENT:  return var_ty(f, n->text);
        case N_UNARY:  return !strcmp(n->text,"-") ? expr_ty(f,n->a) : T_INT;
        case N_BINARY: {
            const char *op = n->text;
            if (is_cmp_op(op) || !strcmp(op,"and") || !strcmp(op,"or")) return T_INT;
            if (!strcmp(op,"/")) return T_NUM;      /* real division, always */
            return (expr_ty(f,n->a)==T_NUM || expr_ty(f,n->b)==T_NUM) ? T_NUM : T_INT;
        }
        case N_CALL:
            if (n->a && n->a->type==N_IDENT) { int i=fn_index(n->a->text); if (i>=0) return tyfns[i].ret; }
            return T_INT;
        default: return T_INT;
    }
}
/* the type an assignment STORES, given what the target already holds */
static Ty combine_ty(const char *op, Ty cur, Ty v)
{
    if (!op[0]) return v;                                       /* plain '=' */
    if (is_cmp_op(op) || !strcmp(op,"and") || !strcmp(op,"or")) return T_INT;
    if (!strcmp(op,"/")) return T_NUM;
    return (cur==T_NUM || v==T_NUM) ? T_NUM : T_INT;
}
static void raise_ty(Ty *slot, Ty t, int *changed) { if (t > *slot) { *slot = t; *changed = 1; } }

static void infer_walk(TyFn *f, Node *n, int *changed)
{
    if (!n) return;
    if (n->type==N_ASSIGN && n->a->type==N_IDENT) {
        int i = tyf_find(f, n->a->text);
        if (i>=0) raise_ty(&f->vty[i], combine_ty(n->text, f->vty[i], expr_ty(f,n->b)), changed);
    }
    if (n->type==N_RETURN && f && n->a) raise_ty(&f->ret, expr_ty(f,n->a), changed);
    if (n->type==N_CALL && n->a && n->a->type==N_IDENT) {
        int k = fn_index(n->a->text);
        if (k>=0) {
            TyFn *callee = &tyfns[k];
            for (int j=0;j<n->nkids && j<callee->nparams;j++)
                raise_ty(&callee->vty[j], expr_ty(f, n->kids[j]), changed);
        }
    }
    for (int i=0;i<n->nkids;i++) infer_walk(f, n->kids[i], changed);
    infer_walk(f,n->a,changed); infer_walk(f,n->b,changed); infer_walk(f,n->c,changed);
}
static Ty param_ty(TyFn *callee, int i) { return i < callee->nparams ? callee->vty[i] : T_INT; }

/* exit(code): code must already be in edi. `mov eax,60; syscall` - does
 * not return. Replaces the Windows build's call [rip+ExitProcess]. */
static void emit_exit_syscall(void)
{
    unsigned char x[]={0xB8,0x3C,0x00,0x00,0x00,   /* mov eax,60 (SYS_exit) */
                        0x0F,0x05};                 /* syscall               */
    bytes(x,7);
}

static void gen_expr(Node *n);
static void gen_block(Node *block);

static void emit_binop(const char *op)
{
    if      (!strcmp(op,"+")) { unsigned char x[]={0x48,0x01,0xC8}; bytes(x,3); }
    else if (!strcmp(op,"-")) { unsigned char x[]={0x48,0x29,0xC8}; bytes(x,3); }
    else if (!strcmp(op,"*")) { unsigned char x[]={0x48,0x0F,0xAF,0xC1}; bytes(x,4); }
    else if (!strcmp(op,"/")) { unsigned char x[]={0x48,0x99,0x48,0xF7,0xF9}; bytes(x,5); }
    else if (!strcmp(op,"%")) { unsigned char x[]={0x48,0x99,0x48,0xF7,0xF9,0x48,0x89,0xD0}; bytes(x,8); }
    else {
        unsigned char cmp[]={0x48,0x39,0xC8}; bytes(cmp,3);
        unsigned char cc;
        if      (!strcmp(op,"==")) cc=0x94;
        else if (!strcmp(op,"!=")) cc=0x95;
        else if (!strcmp(op,"<"))  cc=0x9C;
        else if (!strcmp(op,">"))  cc=0x9F;
        else if (!strcmp(op,"<=")) cc=0x9E;
        else if (!strcmp(op,">=")) cc=0x9D;
        else { fprintf(stderr,"native: unknown op %s\n",op); exit(1); }
        b(0x0F); b(cc); b(0xC0);
        unsigned char mz[]={0x48,0x0F,0xB6,0xC0}; bytes(mz,4);
    }
}

/* rewrite the value in rax from machine type `have` to `want`. INT -> NUM is
 * the widening the type rules actually produce (cvtsi2sd); the reverse is
 * defensive - unreachable when inference is right - and truncates. */
static void coerce_rax(Ty have, Ty want)
{
    if (have == want) return;
    if (want == T_NUM) {                                   /* i64 -> double */
        unsigned char x[]={0xF2,0x48,0x0F,0x2A,0xC0,      /* cvtsi2sd xmm0,rax */
                           0x66,0x48,0x0F,0x7E,0xC0};      /* movq rax,xmm0     */
        bytes(x,10);
    } else {                                               /* double -> i64 */
        unsigned char x[]={0x66,0x48,0x0F,0x6E,0xC0,      /* movq xmm0,rax     */
                           0xF2,0x48,0x0F,0x2C,0xC0};      /* cvttsd2si rax,xmm0*/
        bytes(x,10);
    }
}

/* xmm0 (op) xmm1 -> rax. Arithmetic leaves a double bit-pattern; a comparison
 * leaves the 0/1 bool as an i64, matching interp's number-comparison result. */
static void emit_fbinop(const char *op)
{
    unsigned char store[]={0x66,0x48,0x0F,0x7E,0xC0};      /* movq rax,xmm0 */
    if      (!strcmp(op,"+")) { unsigned char x[]={0xF2,0x0F,0x58,0xC1}; bytes(x,4); bytes(store,5); }
    else if (!strcmp(op,"-")) { unsigned char x[]={0xF2,0x0F,0x5C,0xC1}; bytes(x,4); bytes(store,5); }
    else if (!strcmp(op,"*")) { unsigned char x[]={0xF2,0x0F,0x59,0xC1}; bytes(x,4); bytes(store,5); }
    else if (!strcmp(op,"/")) { unsigned char x[]={0xF2,0x0F,0x5E,0xC1}; bytes(x,4); bytes(store,5); }
    else {
        unsigned char cmp[]={0x66,0x0F,0x2F,0xC1}; bytes(cmp,4);   /* comisd xmm0,xmm1 */
        unsigned char cc;
        if      (!strcmp(op,"==")) cc=0x94;   /* sete  */
        else if (!strcmp(op,"!=")) cc=0x95;   /* setne */
        else if (!strcmp(op,"<"))  cc=0x92;   /* setb  (CF: xmm0<xmm1) */
        else if (!strcmp(op,">"))  cc=0x97;   /* seta  */
        else if (!strcmp(op,"<=")) cc=0x96;   /* setbe */
        else if (!strcmp(op,">=")) cc=0x93;   /* setae */
        else { fprintf(stderr,"nativegen: unknown float op %s\n",op); exit(1); }
        b(0x0F); b(cc); b(0xC0);                                   /* setcc al */
        unsigned char mz[]={0x48,0x0F,0xB6,0xC0}; bytes(mz,4);     /* movzx rax,al */
    }
}

/* KERNEL INTRINSICS - raw memory and port I/O, emitted inline.
 *
 * These are the ops a kernel needs that a user program does not: read/write
 * a physical address, talk to a device port. They are the reason nativegen
 * exists on the kernel path - once it can emit these, a kernel compiled by
 * nativegen touches hardware with no C compiler anywhere in the pipeline.
 *
 * Everything is i64 in the int subset, so an address or a value is just what
 * gen_expr left in rax. Returns 1 if `name` was an intrinsic and code was
 * emitted (leaving the result, if any, in rax); 0 if it is not one.
 *
 * NOTE: in/out are privileged (ring 0). A nativegen program using inb/outb
 * will #GP if run as an ordinary Linux process - that is correct, they are
 * for a kernel. peek/poke are plain memory and run fine anywhere.
 */
static int emit_intrinsic(Node *call)
{
    const char *name = call->a->text;
    int nargs = call->nkids;

    /* --- peek: load 1/2/4/8 bytes from [addr] into rax --- */
    if (nargs == 1 && !strcmp(name, "peek8"))  { gen_expr(call->kids[0]);
        { unsigned char x[]={0x0F,0xB6,0x00}; bytes(x,3); } return 1; }   /* movzx eax,byte[rax] */
    if (nargs == 1 && !strcmp(name, "peek16")) { gen_expr(call->kids[0]);
        { unsigned char x[]={0x0F,0xB7,0x00}; bytes(x,3); } return 1; }   /* movzx eax,word[rax] */
    if (nargs == 1 && !strcmp(name, "peek32")) { gen_expr(call->kids[0]);
        { unsigned char x[]={0x8B,0x00};      bytes(x,2); } return 1; }   /* mov eax,[rax] (zext) */
    if (nargs == 1 && !strcmp(name, "peek64")) { gen_expr(call->kids[0]);
        { unsigned char x[]={0x48,0x8B,0x00}; bytes(x,3); } return 1; }   /* mov rax,[rax] */

    /* --- poke: store the low 1/2/4/8 bytes of val at [addr] --- */
    if (nargs == 2 && (!strcmp(name,"poke8")||!strcmp(name,"poke16")||
                       !strcmp(name,"poke32")||!strcmp(name,"poke64"))) {
        gen_expr(call->kids[0]); b(0x50);        /* addr -> push */
        gen_expr(call->kids[1]);                 /* val  -> rax  */
        b(0x59);                                 /* pop rcx (addr) */
        if      (!strcmp(name,"poke8"))  { unsigned char x[]={0x88,0x01};      bytes(x,2); } /* mov [rcx],al  */
        else if (!strcmp(name,"poke16")) { unsigned char x[]={0x66,0x89,0x01}; bytes(x,3); } /* mov [rcx],ax  */
        else if (!strcmp(name,"poke32")) { unsigned char x[]={0x89,0x01};      bytes(x,2); } /* mov [rcx],eax */
        else                             { unsigned char x[]={0x48,0x89,0x01}; bytes(x,3); } /* mov [rcx],rax */
        { unsigned char z[]={0x48,0x31,0xC0}; bytes(z,3); }   /* xor rax,rax - poke is void */
        return 1;
    }

    /* --- inb(port): read a byte from a device port into rax --- */
    if (nargs == 1 && !strcmp(name, "inb")) {
        gen_expr(call->kids[0]);                 /* port -> rax */
        unsigned char x[]={0x89,0xC2,            /* mov edx,eax  (port -> dx) */
                           0xEC,                 /* in  al,dx                 */
                           0x0F,0xB6,0xC0};      /* movzx eax,al              */
        bytes(x,6);
        return 1;
    }

    /* --- outb(port, val): write a byte to a device port --- */
    if (nargs == 2 && !strcmp(name, "outb")) {
        gen_expr(call->kids[0]); b(0x50);        /* port -> push */
        gen_expr(call->kids[1]);                 /* val  -> rax  */
        b(0x59);                                 /* pop rcx (port) */
        unsigned char x[]={0x89,0xCA,            /* mov edx,ecx  (port -> dx) */
                           0xEE};                /* out dx,al                 */
        bytes(x,3);
        { unsigned char z[]={0x48,0x31,0xC0}; bytes(z,3); }   /* xor rax,rax - outb is void */
        return 1;
    }

    return 0;
}

/* emit a call to a user function, args already the concern of caller */
static void emit_user_call(Node *call)
{
    int nargs = call->nkids;
    TyFn *callee = &tyfns[fn_index(call->a->text)];
    /* push args right-to-left so param[0] ends up at [rbp+16], coercing each
     * to the parameter's inferred type on the way in */
    for (int i = nargs-1; i >= 0; i--) {
        gen_expr(call->kids[i]);
        coerce_rax(expr_ty(cur_ty, call->kids[i]), param_ty(callee, i));
        b(0x50);                                  /* push rax */
    }
    b(0xE8);                                       /* call rel32 (backpatched) */
    if (nfixups >= (int)(sizeof(fixups) / sizeof(fixups[0])))
        backend_limit("call fixup");
    fixups[nfixups].pos = codelen;
    strncpy(fixups[nfixups].name, call->a->text, MAX_TEXT-1);
    fixups[nfixups].name[MAX_TEXT-1]='\0';
    nfixups++;
    b4(0);
    if (nargs > 0) {                               /* add rsp, 8*nargs (caller cleanup) */
        unsigned char x[]={0x48,0x81,0xC4}; bytes(x,3); b4(8*nargs);
    }
}

static void gen_expr(Node *n)
{
    switch (n->type) {
        case N_NUMBER: {
            /* A float literal (the lexer's only '.' form) goes out as its
             * IEEE754 bit pattern; an integer literal as itself. Both are a
             * plain 8-byte immediate into rax - the machine can't tell them
             * apart, the static type does. */
            unsigned long long v;
            if (strchr(n->text,'.')) { union { double d; unsigned long long u; } bits; bits.d = strtod(n->text,0); v = bits.u; }
            else                     { v = (unsigned long long)atoll(n->text); }
            b(0x48); b(0xB8); for(int i=0;i<8;i++) b((unsigned char)((v>>(8*i))&0xFF));
            break;
        }
        case N_BOOL: {
            /* interp.c: make_bool(text=="true"). This backend has no bool
             * type, so a bool is the integer 1/0 - the same representation
             * the LLVM backend uses (see compilel.c's KNOWN GAP note). */
            long long v = !strcmp(n->text,"true");
            b(0x48); b(0xB8); for(int i=0;i<8;i++) b((unsigned char)((v>>(8*i))&0xFF));
            break;
        }
        case N_IDENT: {
            unsigned char x[]={0x48,0x8B,0x85}; bytes(x,3); b4(var_disp(n->text));
            break;
        }
        case N_UNARY:
            gen_expr(n->a);
            if (!strcmp(n->text,"-")) {
                if (expr_ty(cur_ty, n->a) == T_NUM) {
                    /* negate a double by flipping its IEEE sign bit, which is
                     * exact for every value including 0.0 -> -0.0 */
                    b(0x48); b(0xB9);                          /* mov rcx, imm64 */
                    { unsigned long long m=0x8000000000000000ULL; for(int i=0;i<8;i++) b((unsigned char)((m>>(8*i))&0xFF)); }
                    unsigned char x[]={0x48,0x31,0xC8}; bytes(x,3);  /* xor rax,rcx */
                } else { unsigned char x[]={0x48,0xF7,0xD8}; bytes(x,3); }  /* neg rax */
            }
            else { unsigned char c[]={0x48,0x83,0xF8,0x00,0x0F,0x94,0xC0,0x48,0x0F,0xB6,0xC0}; bytes(c,11); }
            break;
        case N_BINARY:
            if (!strcmp(n->text,"and") || !strcmp(n->text,"or")) {
                /* SHORT-CIRCUIT, same as the interpreter and the C
                 * backend: the right side is only reached when the left
                 * has not already decided the answer. This used to
                 * evaluate both and then AND/OR the two booleans, which
                 * gave the right value but ran the right side's calls. */
                int is_and = !strcmp(n->text,"and");
                gen_expr(n->a);
                unsigned char cz[]={0x48,0x83,0xF8,0x00}; bytes(cz,4);  /* cmp rax,0 */
                b(0x0F); b(is_and ? 0x84 : 0x85);                       /* je/jne short */
                int p_short = codelen; b4(0);
                gen_expr(n->b);
                unsigned char tb[]={0x48,0x83,0xF8,0x00,0x0F,0x95,0xC0,0x48,0x0F,0xB6,0xC0};
                bytes(tb,11);                              /* rax = (b != 0) */
                b(0xE9); int p_end = codelen; b4(0);                    /* jmp end */
                patch4(p_short, codelen - (p_short + 4));               /* short: */
                if (is_and) { unsigned char z[]={0x48,0x31,0xC0}; bytes(z,3); }        /* xor rax,rax */
                else { unsigned char o[]={0x48,0xC7,0xC0,0x01,0x00,0x00,0x00}; bytes(o,7); } /* mov rax,1 */
                patch4(p_end, codelen - (p_end + 4));                   /* end: */
            } else {
                Ty ta = expr_ty(cur_ty, n->a), tb = expr_ty(cur_ty, n->b);
                /* '/' is real division so it is always a double; otherwise a
                 * NUM operand on either side pulls the whole op onto the float
                 * path. Everything else stays on the exact integer path. */
                int wantfloat = !strcmp(n->text,"/") || ta==T_NUM || tb==T_NUM;
                if (wantfloat && !strcmp(n->text,"%")) {
                    fprintf(stderr,"nativegen: floating-point %% (fmod) is not supported yet\n"); exit(1);
                }
                if (wantfloat) {
                    gen_expr(n->a); coerce_rax(ta, T_NUM); b(0x50);   /* push a (double) */
                    gen_expr(n->b); coerce_rax(tb, T_NUM);
                    unsigned char m1[]={0x66,0x48,0x0F,0x6E,0xC8}; bytes(m1,5); /* movq xmm1,rax (b) */
                    b(0x58);                                          /* pop rax (a)   */
                    unsigned char m0[]={0x66,0x48,0x0F,0x6E,0xC0}; bytes(m0,5); /* movq xmm0,rax (a) */
                    emit_fbinop(n->text);
                } else {
                    gen_expr(n->a); b(0x50);
                    gen_expr(n->b);
                    unsigned char mv[]={0x48,0x89,0xC1}; bytes(mv,3);
                    b(0x58);
                    emit_binop(n->text);
                }
            }
            break;
        case N_CALL:
            if (n->a->type == N_IDENT && fn_index(n->a->text) >= 0) {
                emit_user_call(n);
            } else if (n->a->type == N_IDENT && emit_intrinsic(n)) {
                /* peek/poke/inb/outb - handled inline, result (if any) in rax */
            } else {
                fprintf(stderr,"native: can't call '%s' in int subset\n",
                        n->a->type==N_IDENT ? n->a->text : "?");
                exit(1);
            }
            break;
        default:
            fprintf(stderr,"native: cannot compile this expression (int subset)\n");
            exit(1);
    }
}

static void emit_fn_epilogue(void)
{
    unsigned char x[]={0x48,0x89,0xEC,0x5D,0xC3}; bytes(x,5);  /* mov rsp,rbp; pop rbp; ret */
}

static void gen_stmt(Node *n)
{
    switch (n->type) {
        case N_ASSIGN: {
            if (n->a->type != N_IDENT) { fprintf(stderr,"native: bad assign\n"); exit(1); }
            Ty slot = var_ty(cur_ty, n->a->text);
            if (n->text[0]) {                                   /* x op= v */
                if (slot == T_NUM) {                            /* float compound */
                    if (!strcmp(n->text,"%")) { fprintf(stderr,"nativegen: floating-point %% (fmod) is not supported yet\n"); exit(1); }
                    unsigned char ld[]={0x48,0x8B,0x85}; bytes(ld,3); b4(var_disp(n->a->text)); /* mov rax,[x] */
                    b(0x50);                                                   /* push x */
                    gen_expr(n->b); coerce_rax(expr_ty(cur_ty,n->b), T_NUM);   /* rax = v (double) */
                    unsigned char m1[]={0x66,0x48,0x0F,0x6E,0xC8}; bytes(m1,5);/* movq xmm1,rax (v) */
                    b(0x58);                                                   /* pop rax (x) */
                    unsigned char m0[]={0x66,0x48,0x0F,0x6E,0xC0}; bytes(m0,5);/* movq xmm0,rax (x) */
                    emit_fbinop(n->text);                                      /* rax = x op v */
                } else {                                        /* integer compound */
                    gen_expr(n->b);                                           /* rax = v */
                    unsigned char mv[]={0x48,0x89,0xC1}; bytes(mv,3);         /* mov rcx,rax */
                    unsigned char ld[]={0x48,0x8B,0x85}; bytes(ld,3);         /* mov rax,[x] */
                    b4(var_disp(n->a->text));
                    emit_binop(n->text);                                      /* rax = x op v */
                }
            } else {
                gen_expr(n->b);                                 /* rax = value */
                coerce_rax(expr_ty(cur_ty, n->b), slot);        /* widen to the slot's type */
            }
            unsigned char x[]={0x48,0x89,0x85}; bytes(x,3); b4(var_disp(n->a->text));
            break;
        }
        case N_RETURN:
            if (n->a) { gen_expr(n->a); coerce_rax(expr_ty(cur_ty,n->a), cur_ty ? cur_ty->ret : T_INT); }
            emit_fn_epilogue();
            break;
        case N_EXPRSTMT: {
            Node *e = n->a;
            if (e->type == N_CALL && e->a->type == N_IDENT && !strcmp(e->a->text,"exit")) {
                gen_expr(e->kids[0]);
                unsigned char mv[]={0x89,0xC7}; bytes(mv,2);        /* mov edi,eax */
                emit_exit_syscall();
                break;
            }
            if (e->type == N_CALL && e->a->type == N_IDENT && !strcmp(e->a->text,"print")) {
                Node *arg = e->kids[0];
                if (arg->type == N_STRING) {
                    if (nsfix >= (int)(sizeof(sfix) / sizeof(sfix[0])))
                        backend_limit("string fixup");
                    if (npsfix >= (int)(sizeof(psfix) / sizeof(psfix[0])))
                        backend_limit("print-string fixup");
                    /* print a string literal: lea rsi,[rip+str]; mov edx,len; call print_str */
                    int slen = (int)strlen(arg->text) + 1;   /* +newline */
                    b(0x48); b(0x8D); b(0x35);               /* lea rsi,[rip+disp] */
                    sfix[nsfix].leapos = codelen; b4(0);
                    strncpy(sfix[nsfix].text, arg->text, 255); sfix[nsfix].text[255]='\0';
                    sfix[nsfix].len = slen; nsfix++;
                    b(0xBA); b4(slen);                       /* mov edx, slen */
                    b(0xE8); psfix[npsfix++] = codelen; b4(0); /* call print_str */
                } else if (expr_ty(cur_ty, arg) == T_NUM) {
                    if (npnfix >= (int)(sizeof(pnfix) / sizeof(pnfix[0])))
                        backend_limit("print-number fixup");
                    gen_expr(arg);                           /* double -> rax */
                    b(0xE8); pnfix[npnfix++] = codelen; b4(0); /* call print_num */
                } else {
                    if (npfix >= (int)(sizeof(pfix) / sizeof(pfix[0])))
                        backend_limit("print-integer fixup");
                    gen_expr(arg);                           /* value -> rax */
                    b(0xE8); pfix[npfix++] = codelen; b4(0); /* call print_int */
                }
                break;
            }
            gen_expr(e);   /* a call used for effect; discard result */
            break;
        }
        case N_IF: {
            gen_expr(n->a);
            unsigned char cz[]={0x48,0x83,0xF8,0x00}; bytes(cz,4);
            b(0x0F); b(0x84); int p_else = codelen; b4(0);      /* je else */
            gen_block(n->b);                                     /* then    */
            if (n->c) {
                b(0xE9); int p_end = codelen; b4(0);            /* jmp end */
                patch4(p_else, codelen - (p_else + 4));         /* else:   */
                gen_block(n->c);
                patch4(p_end, codelen - (p_end + 4));           /* end:    */
            } else {
                patch4(p_else, codelen - (p_else + 4));         /* no else: je -> after then */
            }
            break;
        }
        case N_WHILE: {
            int start = codelen;
            gen_expr(n->a);
            unsigned char cz[]={0x48,0x83,0xF8,0x00}; bytes(cz,4);
            b(0x0F); b(0x84); int p_end = codelen; b4(0);
            if (nloops >= MAX_LOOPS) { fprintf(stderr,"native: loops nested too deep\n"); exit(1); }
            int li = nloops++;
            loop_cont[li] = start;                 /* continue re-tests the condition */
            loop_nbreaks[li] = 0;
            gen_block(n->b);
            b(0xE9); b4(start - (codelen + 4));
            int end_pos = codelen; patch4(p_end, end_pos - (p_end + 4));
            for (int i=0;i<loop_nbreaks[li];i++)   /* every break lands past the loop */
                patch4(loop_breaks[li][i], end_pos - (loop_breaks[li][i] + 4));
            nloops--;
            break;
        }
        case N_BLOCK:
            gen_block(n);
            break;
        case N_BREAK: {
            if (!nloops) { fprintf(stderr,"native: break outside a loop\n"); exit(1); }
            int li = nloops - 1;
            if (loop_nbreaks[li] >= MAX_BREAKS) { fprintf(stderr,"native: too many breaks in one loop\n"); exit(1); }
            b(0xE9);                               /* jmp rel32, patched to loop end  */
            loop_breaks[li][loop_nbreaks[li]++] = codelen; b4(0);
            break;
        }
        case N_CONTINUE:
            if (!nloops) { fprintf(stderr,"native: continue outside a loop\n"); exit(1); }
            b(0xE9); b4(loop_cont[nloops-1] - (codelen + 4));  /* jmp back to condition */
            break;
        default:
            fprintf(stderr,"native: statement not supported in int subset\n");
            exit(1);
    }
}

static void gen_block(Node *block)
{
    if (!block) return;
    if (block->type == N_BLOCK || block->type == N_PROGRAM)
        for (int i=0;i<block->nkids;i++) gen_stmt(block->kids[i]);
    else
        gen_stmt(block);
}

/* emit one user function's code, recording its start offset */
static void gen_function(Node *fn)
{
    foffset[fn_index(fn->text)] = codelen;
    cur_ty = &tyfns[fn_index(fn->text)];

    nparams = 0;
    for (int i=0;i<fn->nkids;i++) {
        if (nparams >= (int)(sizeof(params) / sizeof(params[0])))
            backend_limit("function parameter");
        strncpy(params[nparams],fn->kids[i]->text,MAX_TEXT-1);
        params[nparams][MAX_TEXT-1]='\0'; nparams++;
    }
    nlocals = 0;
    collect_locals(fn->a);
    int frame = ((nlocals * 8 + 15) / 16) * 16;

    b(0x55);                                             /* push rbp */
    { unsigned char x[]={0x48,0x89,0xE5}; bytes(x,3); }  /* mov rbp,rsp */
    { unsigned char x[]={0x48,0x81,0xEC}; bytes(x,3); b4(frame); }  /* sub rsp,frame */
    gen_block(fn->a);
    emit_fn_epilogue();                                  /* safety return */
}

/* emit the print_int routine: converts rax (a non-negative int) to
 * decimal ASCII + newline and writes it to stdout via a raw `write` syscall
 * (SYS_write=1, fd=1). Hand-encoded, dynamically label-patched (no more
 * hardcoded byte offsets - those were fragile and specific to the old
 * WriteFile call sequence this replaces). */
static void emit_print_int(void)
{
    print_int_off = codelen;
    b(0x55);                                                   /* push rbp */
    { unsigned char x[]={0x48,0x89,0xE5}; bytes(x,3); }        /* mov rbp,rsp */
    { unsigned char x[]={0x48,0x83,0xEC,0x20}; bytes(x,4); }   /* sub rsp,0x20 */
    { unsigned char x[]={0x48,0x8D,0x75,0xEF}; bytes(x,4); }   /* lea rsi,[rbp-0x11] */
    { unsigned char x[]={0xC6,0x06,0x0A}; bytes(x,3); }        /* mov byte[rsi],0x0A newline */
    { unsigned char x[]={0x48,0x85,0xC0}; bytes(x,3); }        /* test rax,rax */
    b(0x0F); b(0x85); int p_loop = codelen; b4(0);             /* jnz L_loop */
    { unsigned char x[]={0x48,0xFF,0xCE}; bytes(x,3); }        /* dec rsi */
    { unsigned char x[]={0xC6,0x06,0x30}; bytes(x,3); }        /* mov byte[rsi],'0' */
    b(0xE9); int p_write1 = codelen; b4(0);                    /* jmp L_write */

    int L_loop = codelen; patch4(p_loop, L_loop - (p_loop + 4));
    { unsigned char x[]={0xB9,0x0A,0x00,0x00,0x00}; bytes(x,5); } /* mov ecx,10 */
    int L_next = codelen;
    { unsigned char x[]={0x48,0x85,0xC0}; bytes(x,3); }        /* test rax,rax */
    b(0x0F); b(0x84); int p_write2 = codelen; b4(0);           /* jz L_write */
    { unsigned char x[]={0x48,0x31,0xD2}; bytes(x,3); }        /* xor rdx,rdx */
    { unsigned char x[]={0x48,0xF7,0xF1}; bytes(x,3); }        /* div rcx */
    { unsigned char x[]={0x80,0xC2,0x30}; bytes(x,3); }        /* add dl,'0' */
    { unsigned char x[]={0x48,0xFF,0xCE}; bytes(x,3); }        /* dec rsi */
    { unsigned char x[]={0x88,0x16}; bytes(x,2); }             /* mov [rsi],dl */
    b(0xE9); b4(L_next - (codelen + 4));                       /* jmp L_next */

    int L_write = codelen;
    patch4(p_write1, L_write - (p_write1 + 4));
    patch4(p_write2, L_write - (p_write2 + 4));
    { unsigned char x[]={0x48,0x8D,0x55,0xF0}; bytes(x,4); }   /* lea rdx,[rbp-0x10] */
    { unsigned char x[]={0x48,0x29,0xF2}; bytes(x,3); }        /* sub rdx,rsi  (rdx=length) */
    { unsigned char x[]={0xBF,0x01,0x00,0x00,0x00}; bytes(x,5); } /* mov edi,1 (fd=stdout) */
    { unsigned char x[]={0xB8,0x01,0x00,0x00,0x00}; bytes(x,5); } /* mov eax,1 (SYS_write) */
    { unsigned char x[]={0x0F,0x05}; bytes(x,2); }             /* syscall */
    { unsigned char x[]={0x48,0x89,0xEC}; bytes(x,3); }        /* mov rsp,rbp */
    b(0x5D);                                                    /* pop rbp */
    b(0xC3);                                                    /* ret */
}

/* emit print_str: input rsi=ptr, rdx=len -> write(1, rsi, rdx) to stdout.
 * On Linux the caller's rsi/rdx are already exactly what the write syscall
 * wants, so this collapses to a two-instruction body - no stack frame, no
 * saved handle, no GetStdHandle lookup needed at all. */
static void emit_print_str(void)
{
    print_str_off = codelen;
    { unsigned char x[]={0xBF,0x01,0x00,0x00,0x00}; bytes(x,5); } /* mov edi,1 (fd=stdout) */
    { unsigned char x[]={0xB8,0x01,0x00,0x00,0x00}; bytes(x,5); } /* mov eax,1 (SYS_write) */
    { unsigned char x[]={0x0F,0x05}; bytes(x,2); }             /* syscall */
    b(0xC3);                                                    /* ret */
}
/* emit print_num: input rax = a double's bit pattern. Reproduces interp.c's
 * value_to_string V_NUM arm: if the value is a whole number it prints through
 * the integer path (that arm's "%lld" of (long long)v.num); a fractional
 * number needs "%g", which this backend has no C library for, so it is
 * REFUSED at run time (exit code 7) rather than printed wrong. Whole is the
 * only case run_tests exercises (e.g. 100 / 4 -> 25.0 -> "25"). Must be
 * emitted AFTER emit_print_int so print_int_off is known. */
static void emit_print_num(void)
{
    print_num_off = codelen;
    /* prologue: push rbp; mov rbp,rsp; sub rsp,0x20 (shadow for a later call) */
    { unsigned char x[]={0x55,0x48,0x89,0xE5,0x48,0x83,0xEC,0x20}; bytes(x,8); }
    { unsigned char x[]={0x66,0x48,0x0F,0x6E,0xC0}; bytes(x,5); }  /* movq xmm0,rax    (v)          */
    { unsigned char x[]={0xF2,0x48,0x0F,0x2C,0xC8}; bytes(x,5); }  /* cvttsd2si rcx,xmm0  ((ll)v)   */
    { unsigned char x[]={0xF2,0x48,0x0F,0x2A,0xC9}; bytes(x,5); }  /* cvtsi2sd xmm1,rcx  ((dbl)ll)  */
    { unsigned char x[]={0x66,0x0F,0x2F,0xC1}; bytes(x,4); }       /* comisd xmm0,xmm1              */
    b(0x0F); b(0x8A); int p_jp  = codelen; b4(0);                  /* jp  frac  (unordered/NaN)     */
    b(0x0F); b(0x85); int p_jne = codelen; b4(0);                  /* jne frac  (not whole)         */
    /* whole. print_int is unsigned-only (a pre-existing nativegen limit, also
     * hit by negative ints), so a NEGATIVE whole would print garbage - refuse
     * it too rather than lie. */
    { unsigned char x[]={0x48,0x85,0xC9}; bytes(x,3); }            /* test rcx,rcx                  */
    b(0x0F); b(0x88); int p_js = codelen; b4(0);                   /* js  frac  (negative)          */
    /* value = (long long)v, in rcx. Unwind our frame and TAIL-jump into
     * print_int, whose own ret returns to print_num's caller. */
    { unsigned char x[]={0x48,0x89,0xC8}; bytes(x,3); }            /* mov rax,rcx                   */
    { unsigned char x[]={0x48,0x89,0xEC,0x5D}; bytes(x,4); }       /* mov rsp,rbp; pop rbp          */
    b(0xE9); b4(print_int_off - (codelen + 4));                    /* jmp print_int                 */
    /* frac: refuse - exit(7). */
    int frac = codelen;
    patch4(p_jp,  frac - (p_jp  + 4));
    patch4(p_jne, frac - (p_jne + 4));
    patch4(p_js,  frac - (p_js  + 4));
    { unsigned char x[]={0xBF,0x07,0x00,0x00,0x00}; bytes(x,5); }  /* mov edi,7 */
    emit_exit_syscall();
    b(0xC3);                                                       /* ret (unreached)               */
}

/* ---- ELF64 writer ----
 * A static ET_EXEC with exactly one PT_LOAD segment (R+X) covering the
 * whole file - headers plus machine code. No imports, no dynamic linker,
 * no sections: the code makes raw syscalls directly, so nothing else is
 * needed to run. This is a much smaller format than the PE build needed,
 * because that build's WriteFile/ExitProcess calls required an import
 * table (IAT/ILT/hint-name entries/kernel32.dll's name) that a Linux
 * syscall has no equivalent of. */
static unsigned char elfbuf[(1<<20) + 512];
static void eu16(int o, unsigned v){ elfbuf[o]=(unsigned char)(v&0xFF); elfbuf[o+1]=(unsigned char)((v>>8)&0xFF); }
static void eu32(int o, unsigned v){ for(int i=0;i<4;i++) elfbuf[o+i]=(unsigned char)((v>>(8*i))&0xFF); }
static void eu64(int o, unsigned long long v){ for(int i=0;i<8;i++) elfbuf[o+i]=(unsigned char)((v>>(8*i))&0xFF); }

#define ELF_BASE_VADDR 0x400000ULL
/* a fixed, generously-aligned address inside the zero-filled scratch BSS the
 * writer reserves past the code (see write_elf). A program that wants to
 * peek/poke real memory can use this without knowing its own load layout.
 * 0x420000 is well past any code nativegen emits (a few KiB) and inside the
 * 64 KiB scratch, so it is always mapped writable. */
#define NATIVE_SCRATCH_SIZE 0x20000ULL
#define NATIVE_SCRATCH_ADDR 0x420000ULL
#define EHDR_SIZE 64
#define PHDR_SIZE 56

static void write_elf(const char *path)
{
    memset(elfbuf, 0, sizeof(elfbuf));
    int hdrsize = EHDR_SIZE + PHDR_SIZE;
    unsigned long long entry  = ELF_BASE_VADDR + (unsigned long long)hdrsize;
    unsigned long long filesz = (unsigned long long)hdrsize + (unsigned long long)codelen;

    /* e_ident */
    elfbuf[0]=0x7F; elfbuf[1]='E'; elfbuf[2]='L'; elfbuf[3]='F';
    elfbuf[4]=2;  /* ELFCLASS64    */
    elfbuf[5]=1;  /* ELFDATA2LSB   */
    elfbuf[6]=1;  /* EV_CURRENT    */
    elfbuf[7]=0;  /* ELFOSABI_SYSV - bytes 8-15 stay zero padding */

    eu16(16, 2);            /* e_type = ET_EXEC        */
    eu16(18, 0x3E);          /* e_machine = EM_X86_64   */
    eu32(20, 1);              /* e_version               */
    eu64(24, entry);          /* e_entry                 */
    eu64(32, EHDR_SIZE);      /* e_phoff                 */
    eu64(40, 0);              /* e_shoff (no sections)   */
    eu32(48, 0);              /* e_flags                 */
    eu16(52, EHDR_SIZE);      /* e_ehsize                */
    eu16(54, PHDR_SIZE);      /* e_phentsize             */
    eu16(56, 1);              /* e_phnum                 */
    eu16(58, 0);              /* e_shentsize             */
    eu16(60, 0);              /* e_shnum                 */
    eu16(62, 0);              /* e_shstrndx              */

    /* A 64 KiB zero-filled scratch region past the code: memsz > filesz, so
     * the loader maps [base+filesz, base+memsz) as zeroed writable memory - a
     * BSS. peek/poke now have somewhere to write. NATIVE_SCRATCH below is the
     * fixed address a program can use for it. */
    unsigned long long memsz = filesz + NATIVE_SCRATCH_SIZE;

    int ph = EHDR_SIZE;
    eu32(ph+0,  1);           /* p_type = PT_LOAD        */
    eu32(ph+4,  7);           /* p_flags = PF_R|PF_W|PF_X - writable, so a
                                 program can peek/poke its own scratch region */
    eu64(ph+8,  0);           /* p_offset                */
    eu64(ph+16, ELF_BASE_VADDR); /* p_vaddr              */
    eu64(ph+24, ELF_BASE_VADDR); /* p_paddr              */
    eu64(ph+32, filesz);      /* p_filesz                */
    eu64(ph+40, memsz);       /* p_memsz - includes the scratch BSS */
    eu64(ph+48, 0x1000);      /* p_align                 */

    memcpy(elfbuf + hdrsize, code, (size_t)codelen);

    FILE *f = fopen(path, "wb");
    if (!f) { fprintf(stderr, "can't write %s\n", path); exit(1); }
    fwrite(elfbuf, 1, filesz, f);
    fclose(f);
    chmod(path, 0755);
}

int main(int argc, char **argv)
{
    if (argc < 2) { fprintf(stderr,"usage: nativegen <file.zl>\n"); return 1; }

    int count;
    Token *toks = lex_file(argv[1], &count);
    Node *prog = parse(toks, count);

    /* register function names first (so calls resolve, incl. recursion) */
    for (int i=0;i<prog->nkids;i++)
        if (prog->kids[i]->type == N_FN) {
            if (nfuncs >= (int)(sizeof(fnames) / sizeof(fnames[0])))
                backend_limit("function");
            strncpy(fnames[nfuncs], prog->kids[i]->text, MAX_TEXT-1);
            fnames[nfuncs][MAX_TEXT-1]='\0'; nfuncs++;
        }

    /* ---- number-type inference (INT/NUM), run to a fixpoint before any
     * code is emitted, so every emit site knows the machine type it holds.
     * tyfns is index-parallel to fnames, and params occupy each scope's
     * first nparams slots (added before the body's locals). ---- */
    tytop.node=0; tytop.nvars=0; tytop.nparams=0; tytop.ret=T_INT;
    for (int i=0;i<prog->nkids;i++)
        if (prog->kids[i]->type != N_FN) collect_ty_vars(&tytop, prog->kids[i]);
    for (int i=0, k=0; i<prog->nkids; i++) {
        if (prog->kids[i]->type != N_FN) continue;
        Node *fn = prog->kids[i];
        tyfns[k].node=fn; tyfns[k].nvars=0; tyfns[k].nparams=fn->nkids; tyfns[k].ret=T_INT;
        for (int p=0;p<fn->nkids;p++) tyf_add(&tyfns[k], fn->kids[p]->text);
        collect_ty_vars(&tyfns[k], fn->a);
        k++;
    }
    { int changed=1;
      while (changed) {
          changed=0;
          for (int i=0;i<prog->nkids;i++)
              if (prog->kids[i]->type != N_FN) infer_walk(&tytop, prog->kids[i], &changed);
          for (int i=0, k=0; i<prog->nkids; i++)
              if (prog->kids[i]->type == N_FN) infer_walk(&tyfns[k++], prog->kids[i]->a, &changed);
      }
    }

    /* ---- main (entry) first, at offset 0 ---- */
    cur_ty = &tytop;
    nparams = 0; nlocals = 0;
    for (int i=0;i<prog->nkids;i++) if (prog->kids[i]->type != N_FN) collect_locals(prog->kids[i]);
    int frame = ((nlocals * 8 + 15) / 16) * 16;
    b(0x55);
    { unsigned char x[]={0x48,0x89,0xE5}; bytes(x,3); }
    { unsigned char x[]={0x48,0x81,0xEC}; bytes(x,3); b4(frame); }
    for (int i=0;i<prog->nkids;i++) if (prog->kids[i]->type != N_FN) gen_stmt(prog->kids[i]);
    /* fall-through: exit(0) */
    { unsigned char x[]={0x31,0xFF}; bytes(x,2); }             /* xor edi,edi */
    emit_exit_syscall();

    /* ---- then each function ---- */
    for (int i=0;i<prog->nkids;i++)
        if (prog->kids[i]->type == N_FN) gen_function(prog->kids[i]);

    /* ---- then the print helpers ---- */
    emit_print_int();
    emit_print_str();
    emit_print_num();   /* after print_int: it tail-jumps into print_int */

    /* ---- string literal data + fix the lea disps that point to it ---- */
    for (int i=0;i<nsfix;i++) {
        int str_off = codelen;
        for (const char *p = sfix[i].text; *p; p++) b((unsigned char)*p);
        b('\n');                                        /* print adds a newline */
        patch4(sfix[i].leapos, str_off - (sfix[i].leapos + 4));
    }

    /* ---- backpatch function call sites ---- */
    for (int i=0;i<nfixups;i++) {
        int fi = fn_index(fixups[i].name);
        if (fi < 0) { fprintf(stderr,"native: unknown function '%s'\n",fixups[i].name); return 1; }
        patch4(fixups[i].pos, foffset[fi] - (fixups[i].pos + 4));
    }
    /* ---- backpatch print(int), print(str) and print(num) call sites ---- */
    for (int i=0;i<npfix;i++)  patch4(pfix[i],  print_int_off - (pfix[i]  + 4));
    for (int i=0;i<npsfix;i++) patch4(psfix[i], print_str_off - (psfix[i] + 4));
    for (int i=0;i<npnfix;i++) patch4(pnfix[i], print_num_off - (pnfix[i] + 4));

    write_elf("native_out");
    printf("nativegen: wrote native_out (%d bytes x86-64 ELF, %d functions, no C compiler, no libc)\n", codelen, nfuncs);
    return 0;
}
