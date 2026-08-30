/* nativeval.c - Floor 4 brick 6: the BOXED-VALUE native backend.
 *
 * Walks a zl AST and emits x86-64 that computes with TAGGED VALUES by
 * calling the hand-written runtime in nativert.c (zl_add, zl_print, ...).
 * Unlike nativegen.c (integers only, inline ops), this backend supports
 * the full value model - ints, strings, lists - because every operation
 * defers to the runtime. Output is a PE .exe with NO C compiler and NO
 * libc involved: it imports only kernel32, exactly like raw_rt.exe.
 *
 * We get the entire runtime by including nativert.c as a library
 * (NATIVERT_LIB suppresses its proof program and driver main).
 *
 * ---- STACK ALIGNMENT (the one thing that will bite) ----
 * The zl_ runtime routines expect to be entered with rsp == 8 (mod 16),
 * i.e. the caller keeps rsp 16-byte aligned right before every `call`.
 * A bare `push rax` breaks that. So when we need to save an operand
 * across a nested sub-expression (which may itself call a runtime
 * routine), we save 16 bytes at a time (spill/reload), never 8.
 *
 * Stage 1 (this file so far): numbers, + - * / %, comparisons,
 * and/or/not, unary minus, string literals, variables, and print().
 * Control flow and user functions come in the next increment.
 */
#define NATIVERT_LIB
#include "nativert.c"

#include "lexer.h"
#include "parser.h"

/* ---- scoping ---------------------------------------------------------
 * Matching the interpreter's model (and the documented gotcha): variables
 * assigned at TOP LEVEL are GLOBALS. Functions see those globals, plus
 * their own PARAMETERS, plus any name they assign that is neither a param
 * nor a global (a true function-local). A function assigning to a name
 * that IS a top-level global writes THROUGH to the global - that is the
 * "function-locals leak into globals" footgun the self-host relies on.
 *
 * Globals live in the first nglobals*8 bytes of the arena (reserved at
 * entry). A global is reached indirectly through G_BASE. Params and
 * locals are rbp-relative in the current function frame. */
static char gnames[8192][MAX_TEXT]; static int nglobals = 0;
static int global_index(const char *name)
{
    for (int i=0;i<nglobals;i++) if(!strcmp(gnames[i],name)) return i;
    return -1;
}
static int register_global(const char *name)
{
    int i = global_index(name);
    if (i >= 0) return i;
    strncpy(gnames[nglobals], name, MAX_TEXT-1); gnames[nglobals][MAX_TEXT-1]='\0';
    return nglobals++;
}

/* current function context (empty at top level) */
static int  in_fn = 0;
static char params[256][MAX_TEXT];  static int nparams = 0;
static char flocals[8192][MAX_TEXT]; static int nflocals = 0;
static int  param_index(const char *name){ for(int i=0;i<nparams;i++) if(!strcmp(params[i],name)) return i; return -1; }
static int  flocal_slot(const char *name)
{
    for (int i=0;i<nflocals;i++) if(!strcmp(flocals[i],name)) return i;
    strncpy(flocals[nflocals], name, MAX_TEXT-1); flocals[nflocals][MAX_TEXT-1]='\0';
    return nflocals++;
}

/* collect the globals a top-level body assigns (runs before codegen) */
static void collect_globals(Node *n)
{
    if (!n) return;
    if (n->type == N_ASSIGN && n->a && n->a->type == N_IDENT) register_global(n->a->text);
    if (n->type == N_FOR && n->text[0]) register_global(n->text);   /* the loop var */
    for (int i=0;i<n->nkids;i++) collect_globals(n->kids[i]);
    collect_globals(n->a); collect_globals(n->b); collect_globals(n->c);
}
/* pre-scan a function body for its true locals (names assigned that are
 * neither params nor globals) */
static void collect_flocals(Node *n)
{
    if (!n) return;
    if (n->type == N_ASSIGN && n->a && n->a->type == N_IDENT
        && param_index(n->a->text) < 0 && global_index(n->a->text) < 0)
        flocal_slot(n->a->text);
    if (n->type == N_FOR && n->text[0]
        && param_index(n->text) < 0 && global_index(n->text) < 0)
        flocal_slot(n->text);   /* the loop var, if not a param/global */
    for (int i=0;i<n->nkids;i++) collect_flocals(n->kids[i]);
    collect_flocals(n->a); collect_flocals(n->b); collect_flocals(n->c);
}

/* ---- user functions: name -> code offset, plus call-site fixups ---- */
static char fnames[1024][MAX_TEXT];
static int  foffset[1024];
static int  nfuncs = 0;
static int  fn_index(const char *name){ for(int i=0;i<nfuncs;i++) if(!strcmp(fnames[i],name)) return i; return -1; }
static struct { int pos; char name[MAX_TEXT]; } ufix[65536];
static int  nufix = 0;

/* ---- small emit helpers specific to codegen ---- */
static void mov_rax_imm64(long long v)   /* mov rax, imm64 */
{
    b(0x48); b(0xB8);
    for (int i=0;i<8;i++) b((unsigned char)((v>>(8*i))&0xFF));
}
static void ev_mov_rcx_rax(void) { unsigned char x[]={0x48,0x89,0xC1}; bs(x,3); }
static void ev_mov_rdx_rax(void) { unsigned char x[]={0x48,0x89,0xC2}; bs(x,3); }
static void mov_rdx_imm(int v)   { b(0x48); b(0xC7); b(0xC2); b4(v); }   /* mov rdx,imm32 (guarded out of nativert.c) */

/* save rax across a nested sub-expression: 16 bytes to preserve alignment */
static void spill_rax(void)  { unsigned char x[]={0x48,0x83,0xEC,0x10, 0x48,0x89,0x04,0x24}; bs(x,8); } /* sub rsp,16 ; mov [rsp],rax */
static void reload_rcx(void) { unsigned char x[]={0x48,0x8B,0x0C,0x24, 0x48,0x83,0xC4,0x10}; bs(x,8); } /* mov rcx,[rsp] ; add rsp,16 */

static void mov_rcx_mem(unsigned rva) { b(0x48); b(0x8B); b(0x0D); rip32(rva); } /* mov rcx,[rip+X] */

/* load variable `name` into rax, honouring param / global / local scope */
static void gen_load_var(const char *name)
{
    if (in_fn) {
        int p = param_index(name);
        if (p >= 0) { unsigned char x[]={0x48,0x8B,0x85}; bs(x,3); b4(16 + 8*p); return; }  /* mov rax,[rbp+16+8p] */
        if (global_index(name) < 0) { unsigned char x[]={0x48,0x8B,0x85}; bs(x,3); b4(-8*(flocal_slot(name)+1)); return; }
    }
    /* global (top level, or a function reading a top-level name) */
    int gi = global_index(name);
    if (gi < 0) gi = register_global(name);          /* read-before-write: treat as global */
    mov_rax_mem(G_BASE);                              /* rax = arena base */
    { unsigned char x[]={0x48,0x8B,0x80}; bs(x,3); b4(8*gi); }   /* mov rax,[rax+8*gi] */
}

/* store rax into variable `name`, honouring scope (value is in rax) */
static void gen_store_var(const char *name)
{
    if (in_fn) {
        int p = param_index(name);
        if (p >= 0) { unsigned char x[]={0x48,0x89,0x85}; bs(x,3); b4(16 + 8*p); return; }  /* mov [rbp+16+8p],rax */
        if (global_index(name) < 0) { unsigned char x[]={0x48,0x89,0x85}; bs(x,3); b4(-8*(flocal_slot(name)+1)); return; }
    }
    int gi = global_index(name);
    if (gi < 0) gi = register_global(name);
    mov_rcx_mem(G_BASE);                              /* rcx = arena base (rax holds the value) */
    { unsigned char x[]={0x48,0x89,0x81}; bs(x,3); b4(8*gi); }   /* mov [rcx+8*gi],rax */
}

static void gen_expr(Node *n);
static void emit_user_call(Node *call);

/* evaluate a and b, leaving a in rcx and b in rdx (alignment-safe) */
static void gen_pair(Node *a, Node *b)
{
    gen_expr(a);
    spill_rax();
    gen_expr(b);
    ev_mov_rdx_rax();       /* b -> rdx */
    reload_rcx();           /* a -> rcx */
}
static void gen_two_operands(Node *n) { gen_pair(n->a, n->b); }

/* a builtin: name -> (runtime routine, argcount). Only the ones the
 * self-host needs; stdlib builtins are not wired into the native path. */
static int builtin_routine(const char *name, int **routine)
{
    if (!strcmp(name,"len"))   { *routine=&off_len;    return 1; }
    if (!strcmp(name,"push"))  { *routine=&off_push;   return 2; }
    if (!strcmp(name,"at"))    { *routine=&off_at;     return 2; }
    if (!strcmp(name,"has"))   { *routine=&off_has;    return 2; }
    if (!strcmp(name,"read"))  { *routine=&off_read;   return 1; }
    if (!strcmp(name,"write")) { *routine=&off_writef; return 2; }
    if (!strcmp(name,"print")) { *routine=&off_print;  return 1; }
    return 0;   /* not a wired builtin */
}

static int *binop_routine(const char *op)
{
    if (!strcmp(op,"+"))  return &off_add;
    if (!strcmp(op,"-"))  return &off_sub;
    if (!strcmp(op,"*"))  return &off_mul;
    if (!strcmp(op,"/"))  return &off_div;
    if (!strcmp(op,"%"))  return &off_mod;
    if (!strcmp(op,"==")) return &off_eq;
    if (!strcmp(op,"!=")) return &off_ne;
    if (!strcmp(op,"<"))  return &off_lt;
    if (!strcmp(op,"<=")) return &off_le;
    if (!strcmp(op,">"))  return &off_gt;
    if (!strcmp(op,">=")) return &off_ge;
    return 0;
}

static void gen_expr(Node *n)
{
    switch (n->type) {
        case N_NUMBER:
            mov_rax_imm64(((long long)atoll(n->text) << 1) | 1);   /* tagged int */
            break;

        case N_STRING: {
            int len = (int)strlen(n->text);
            lea_rcx(add_data(n->text, len));   /* rcx = bytes */
            mov_edx_imm(len);                  /* rdx = len   */
            call_to(&off_strlit);              /* rax = heap string */
            break;
        }

        case N_IDENT:
            gen_load_var(n->text);
            break;

        case N_UNARY:
            gen_expr(n->a);
            if (!strcmp(n->text,"-")) {
                /* negate a tagged int: -(2n+1)+2 = 2(-n)+1 */
                unsigned char x[]={0x48,0xF7,0xD8, 0x48,0x83,0xC0,0x02}; bs(x,7);  /* neg rax ; add rax,2 */
            } else {   /* not: truthy then flip, retag */
                ev_mov_rcx_rax();
                call_to(&off_truthy);                    /* rax = raw 0/1 */
                unsigned char x[]={0x48,0x83,0xF0,0x01, 0x48,0x8D,0x44,0x00,0x01}; bs(x,9); /* xor rax,1 ; lea rax,[rax+rax+1] */
            }
            break;

        case N_BINARY: {
            if (!strcmp(n->text,"and") || !strcmp(n->text,"or")) {
                /* evaluate both to raw truthiness, combine, retag.
                 * (non-short-circuit, matching the integer backend) */
                gen_expr(n->a); ev_mov_rcx_rax(); call_to(&off_truthy);
                spill_rax();
                gen_expr(n->b); ev_mov_rcx_rax(); call_to(&off_truthy);
                reload_rcx();                            /* rcx = truthy(a) */
                if (!strcmp(n->text,"and")) { unsigned char x[]={0x48,0x21,0xC8}; bs(x,3); }  /* and rax,rcx */
                else                        { unsigned char x[]={0x48,0x09,0xC8}; bs(x,3); }  /* or  rax,rcx */
                unsigned char t[]={0x48,0x8D,0x44,0x00,0x01}; bs(t,5);   /* lea rax,[rax+rax+1] retag */
                break;
            }
            int *r = binop_routine(n->text);
            if (!r) { fprintf(stderr,"nativeval: unknown operator '%s'\n", n->text); exit(1); }
            gen_two_operands(n);
            call_to(r);
            break;
        }

        case N_BOOL:
            mov_rax_imm64(!strcmp(n->text,"true") ? 3 : 1);   /* tagged true/false */
            break;

        case N_DANGER:          /* the '!' marker: no runtime effect */
            gen_expr(n->a);
            break;

        case N_LIST: {
            /* rbx holds the list under construction; save it (16-byte,
             * alignment-safe) so nested lists nest correctly. The runtime
             * preserves rbx across calls, so it survives element eval. */
            unsigned char sv[]={0x48,0x83,0xEC,0x10, 0x48,0x89,0x1C,0x24}; bs(sv,8);  /* sub rsp,16 ; mov [rsp],rbx */
            mov_rcx_imm(n->nkids);
            call_to(&off_lnew);
            { unsigned char x[]={0x48,0x89,0xC3}; bs(x,3); }   /* mov rbx,rax */
            for (int i=0;i<n->nkids;i++) {
                gen_expr(n->kids[i]);                          /* rax = element */
                { unsigned char x[]={0x49,0x89,0xC0}; bs(x,3); }        /* mov r8,rax   value */
                { unsigned char x[]={0x48,0x89,0xD9}; bs(x,3); }        /* mov rcx,rbx  list  */
                mov_rdx_imm(i);                                         /* mov rdx,i    raw   */
                call_to(&off_lset);
            }
            { unsigned char x[]={0x48,0x89,0xD8}; bs(x,3); }   /* mov rax,rbx  result */
            unsigned char rs[]={0x48,0x8B,0x1C,0x24, 0x48,0x83,0xC4,0x10}; bs(rs,8);  /* mov rbx,[rsp] ; add rsp,16 */
            break;
        }

        case N_INDEX:
            gen_pair(n->a, n->b);      /* list -> rcx, index -> rdx (tagged) */
            call_to(&off_index);
            break;

        case N_CALL:
            if (n->a->type == N_IDENT && fn_index(n->a->text) >= 0) {
                emit_user_call(n);
            } else if (n->a->type == N_IDENT) {
                int *routine; int arity = builtin_routine(n->a->text, &routine);
                if (arity == 0) { fprintf(stderr,"nativeval: builtin '%s' is not wired into the native backend\n", n->a->text); exit(1); }
                if (n->nkids != arity) { fprintf(stderr,"nativeval: '%s' expects %d arg(s)\n", n->a->text, arity); exit(1); }
                if (arity == 1) { gen_expr(n->kids[0]); ev_mov_rcx_rax(); }
                else            { gen_pair(n->kids[0], n->kids[1]); }
                call_to(routine);
                if (!strcmp(n->a->text,"print")) mov_rax_imm64(1);   /* print's value is nil-ish */
            } else {
                fprintf(stderr,"nativeval: can only call a plain name\n");
                exit(1);
            }
            break;

        default:
            fprintf(stderr,"nativeval: cannot compile this expression yet (type %d)\n", n->type);
            exit(1);
    }
}

static void gen_stmt(Node *n);

static void gen_block(Node *block)
{
    if (!block) return;
    if (block->type == N_BLOCK || block->type == N_PROGRAM)
        for (int i=0;i<block->nkids;i++) gen_stmt(block->kids[i]);
    else
        gen_stmt(block);
}

/* evaluate a condition to a raw 0/1 in rax (via zl_truthy), then emit a
 * `test rax,rax`. The caller places the conditional jump. */
static void gen_cond(Node *cond)
{
    gen_expr(cond);
    ev_mov_rcx_rax();
    call_to(&off_truthy);                       /* rax = raw 0/1 */
    unsigned char t[]={0x48,0x85,0xC0}; bs(t,3); /* test rax,rax  */
}

static void gen_stmt(Node *n)
{
    switch (n->type) {
        case N_ASSIGN:
            if (n->a->type != N_IDENT) { fprintf(stderr,"nativeval: bad assignment target\n"); exit(1); }
            gen_expr(n->b);
            gen_store_var(n->a->text);
            break;

        case N_IF: {
            gen_cond(n->a);
            b(0x0F); b(0x84); int p_else = codelen; b4(0);   /* je else/end */
            gen_block(n->b);                                  /* then        */
            if (n->c) {
                b(0xE9); int p_end = codelen; b4(0);          /* jmp end     */
                patch4(p_else, codelen - (p_else + 4));        /* else:       */
                gen_block(n->c);
                patch4(p_end, codelen - (p_end + 4));          /* end:        */
            } else {
                patch4(p_else, codelen - (p_else + 4));        /* je -> after then */
            }
            break;
        }

        case N_WHILE: {
            int start = codelen;
            gen_cond(n->a);
            b(0x0F); b(0x84); int p_end = codelen; b4(0);     /* je end */
            gen_block(n->b);
            b(0xE9); b4(start - (codelen + 4));                /* jmp start */
            patch4(p_end, codelen - (p_end + 4));
            break;
        }

        case N_FOR: {
            /* for v in seq { body } - iterate the list's items.
             * _seq and _idx live in a reserved 16-aligned stack region
             * ([rsp+0], [rsp+8]) that persists across the body's own
             * stack use; rsp stays 16-aligned throughout the loop. */
            { unsigned char x[]={0x48,0x83,0xEC,0x20}; bs(x,4); }         /* sub rsp,32 */
            gen_expr(n->a);                                              /* rax = seq  */
            { unsigned char x[]={0x48,0x89,0x04,0x24}; bs(x,4); }        /* mov [rsp],rax  _seq */
            { unsigned char x[]={0x48,0xC7,0x44,0x24,0x08,0,0,0,0}; bs(x,9); } /* mov qword[rsp+8],0  _idx */
            int loop = codelen;
            { unsigned char x[]={0x48,0x8B,0x0C,0x24}; bs(x,4); }        /* mov rcx,[rsp]  _seq */
            call_to(&off_len);
            { unsigned char x[]={0x48,0xD1,0xF8}; bs(x,3); }             /* sar rax,1  raw len */
            { unsigned char x[]={0x48,0x8B,0x4C,0x24,0x08}; bs(x,5); }   /* mov rcx,[rsp+8] _idx */
            { unsigned char x[]={0x48,0x39,0xC1}; bs(x,3); }             /* cmp rcx,rax */
            b(0x0F); b(0x8D); int p_end = codelen; b4(0);               /* jge end */
            { unsigned char x[]={0x48,0x8B,0x0C,0x24}; bs(x,4); }        /* mov rcx,[rsp]  list */
            { unsigned char x[]={0x48,0x8B,0x54,0x24,0x08}; bs(x,5); }   /* mov rdx,[rsp+8] _idx raw */
            { unsigned char x[]={0x48,0x8D,0x54,0x12,0x01}; bs(x,5); }   /* lea rdx,[rdx+rdx+1] tag it */
            call_to(&off_index);                                        /* rax = element */
            gen_store_var(n->text);                                     /* v = element   */
            gen_block(n->b);                                            /* body          */
            { unsigned char x[]={0x48,0x8B,0x44,0x24,0x08}; bs(x,5); }   /* mov rax,[rsp+8] */
            { unsigned char x[]={0x48,0xFF,0xC0}; bs(x,3); }             /* inc rax */
            { unsigned char x[]={0x48,0x89,0x44,0x24,0x08}; bs(x,5); }   /* mov [rsp+8],rax */
            b(0xE9); b4(loop - (codelen + 4));                          /* jmp loop */
            patch4(p_end, codelen - (p_end + 4));
            { unsigned char x[]={0x48,0x83,0xC4,0x20}; bs(x,4); }        /* add rsp,32 */
            break;
        }

        case N_RETURN:
            if (n->a) gen_expr(n->a);
            else mov_rax_imm64(1);          /* no value -> tagged 0 */
            { unsigned char x[]={0x48,0x89,0xEC,0x5D,0xC3}; bs(x,5); }  /* mov rsp,rbp; pop rbp; ret */
            break;

        case N_EXPRSTMT: {
            Node *e = n->a;
            if (e->type == N_CALL && e->a->type == N_IDENT && !strcmp(e->a->text,"print")) {
                gen_expr(e->kids[0]);       /* value -> rax */
                ev_mov_rcx_rax();
                call_to(&off_print);        /* runtime tag dispatch */
                break;
            }
            /* any other expression-statement: evaluate for effect, drop result */
            gen_expr(e);
            break;
        }

        default:
            fprintf(stderr,"nativeval: statement type %d not supported yet\n", n->type);
            exit(1);
    }
}

/* Call a user function. Outgoing args go in a 16-aligned block below rsp,
 * written with `mov` (not `push`) so rsp never moves mid-evaluation and
 * every nested runtime call stays 16-aligned. The callee reads param i at
 * [rbp+16+8i]. Result comes back in rax. */
static void emit_user_call(Node *call)
{
    int nargs = call->nkids;
    int block = ((8*nargs + 15) / 16) * 16;             /* round up to 16 */
    if (block) { b(0x48); b(0x81); b(0xEC); b4(block); } /* sub rsp,block */
    for (int i=0;i<nargs;i++) {
        gen_expr(call->kids[i]);                        /* rax = arg i (rsp stays aligned) */
        unsigned char x[]={0x48,0x89,0x44,0x24,(unsigned char)(8*i)}; bs(x,5);  /* mov [rsp+8i],rax */
    }
    b(0xE8);                                            /* call rel32 (backpatched) */
    ufix[nufix].pos = codelen;
    strncpy(ufix[nufix].name, call->a->text, MAX_TEXT-1); ufix[nufix].name[MAX_TEXT-1]='\0';
    nufix++;
    b4(0);
    if (block) { b(0x48); b(0x81); b(0xC4); b4(block); } /* add rsp,block */
}

/* Emit one user function's body. Params live above the return address at
 * [rbp+16+8i]; true locals below rbp. */
static void gen_function(Node *fn)
{
    foffset[fn_index(fn->text)] = codelen;

    in_fn = 1;
    nparams = 0;
    for (int i=0;i<fn->nkids;i++) { strncpy(params[nparams], fn->kids[i]->text, MAX_TEXT-1); params[nparams][MAX_TEXT-1]='\0'; nparams++; }
    nflocals = 0;
    collect_flocals(fn->a);
    int frame = ((nflocals * 8 + 15) / 16) * 16;
    if (frame < 16) frame = 16;                          /* room for spills */

    b(0x55);                                             /* push rbp */
    { unsigned char x[]={0x48,0x89,0xE5}; bs(x,3); }     /* mov rbp,rsp */
    { unsigned char x[]={0x48,0x81,0xEC}; bs(x,3); b4(frame); }  /* sub rsp,frame */
    gen_block(fn->a);
    /* safety epilogue if the body falls through without returning */
    mov_rax_imm64(1);                                    /* tagged 0 */
    { unsigned char x[]={0x48,0x89,0xEC,0x5D,0xC3}; bs(x,5); }
    in_fn = 0;
}

int main(int argc, char **argv)
{
    if (argc < 2) { fprintf(stderr,"usage: nativeval <file.zl>\n"); return 1; }

    int count;
    Token *toks = lex_file(argv[1], &count);
    Node  *prog = parse(toks, count);

    /* register function names first, so calls (incl. recursion) resolve */
    for (int i=0;i<prog->nkids;i++)
        if (prog->kids[i]->type == N_FN) {
            strncpy(fnames[nfuncs], prog->kids[i]->text, MAX_TEXT-1);
            fnames[nfuncs][MAX_TEXT-1]='\0'; nfuncs++;
        }

    rt_init_literals();

    /* every top-level assignment is a global; reserve a slot per global */
    for (int i=0;i<prog->nkids;i++)
        if (prog->kids[i]->type != N_FN) collect_globals(prog->kids[i]);

    /* ---- entry (RVA 0x1000): arena + reserved global slots ---- */
    b(0x55);                                              /* push rbp            */
    { unsigned char x[]={0x48,0x89,0xE5}; bs(x,3); }      /* mov rbp,rsp         */
    { unsigned char x[]={0x48,0x81,0xEC}; bs(x,3); b4(0x40); }    /* sub rsp,0x40 */
    { unsigned char x[]={0x48,0x83,0xE4,0xF0}; bs(x,4); } /* and rsp,-16         */
    { unsigned char x[]={0x31,0xC9}; bs(x,2); }           /* xor ecx,ecx  (NULL) */
    { unsigned char x[]={0xBA,0x00,0x00,0x00,0x04}; bs(x,5); }        /* mov edx,64MB */
    { unsigned char x[]={0x41,0xB8,0x00,0x30,0x00,0x00}; bs(x,6); }   /* mov r8d,0x3000 */
    { unsigned char x[]={0x41,0xB9,0x04,0x00,0x00,0x00}; bs(x,6); }   /* mov r9d,4 */
    call_iat(IAT_VALLOC);
    mov_mem_rax(G_BASE);                                  /* base                */
    if (nglobals) { b(0x48); b(0x05); b4(nglobals*8); }   /* add rax,nglobals*8  */
    mov_mem_rax(G_BUMP);                                  /* bump past the globals */

    in_fn = 0;
    for (int i=0;i<prog->nkids;i++)
        if (prog->kids[i]->type != N_FN) gen_stmt(prog->kids[i]);

    /* ExitProcess(0) */
    { unsigned char x[]={0x31,0xC9}; bs(x,2); }
    call_iat(IAT_EXIT);

    /* ---- each user function, then the runtime ---- */
    for (int i=0;i<prog->nkids;i++)
        if (prog->kids[i]->type == N_FN) gen_function(prog->kids[i]);

    emit_runtime();
    resolve_runtime_calls();
    for (int i=0;i<nufix;i++) {
        int fi = fn_index(ufix[i].name);
        if (fi < 0) { fprintf(stderr,"nativeval: unknown function '%s'\n", ufix[i].name); return 1; }
        patch4(ufix[i].pos, foffset[fi] - (ufix[i].pos + 4));
    }
    write_pe("nvout.exe");
    printf("nativeval: wrote nvout.exe (%d bytes code, %d data, no libc)\n", codelen, datalen);
    return 0;
}
