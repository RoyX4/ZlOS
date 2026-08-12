/* nativert.c - Floor 4: THE NO-C RUNTIME, hand-assembled x86-64.
 *
 * Emits raw_rt.exe: a Windows executable containing the zl runtime
 * written directly as machine code, using only kernel32 syscalls.
 * No malloc, no printf, no libc, no C runtime in the output.
 *
 * This file GROWS one brick at a time (docs/design/design_native_runtime.md
 * section 6). Each brick ends with a runnable proof program.
 *
 *   brick 1  heap from the OS (VirtualAlloc)       DONE (was nativeheap.c)
 *   brick 2  tagged values + tag dispatch          DONE (was nativetag.c)
 *   brick 3  heap strings + concat + equality      DONE
 *   brick 4  zl_add dispatch, comparisons, truthy  DONE
 *   brick 5  list objects + recursive printing     DONE
 *   brick 6a zl_at + zl_has (string builtins the   DONE
 *            self-host needs; the rest wait for nativeval.c)
 *   brick 7  file I/O via syscalls (read/write)    DONE  <-- this commit
 *   brick 6  nativeval.c codegen calling these     next (the big one)
 *   brick 8  compile compiler.zl -> native self-host
 *
 * ---- VALUE REPRESENTATION ----
 * Every zl value is one 8-byte word.
 *   low bit 1 -> INTEGER, stored as (n<<1)|1. Recover with `sar 1`.
 *   low bit 0 -> POINTER to a heap object:
 *        [+0]  type tag   0=string  1=list  3=nil
 *        [+8]  length / count
 *        [+16] payload
 * Booleans fold into tagged ints: false = 1 (0<<1|1), true = 3 (1<<1|1).
 *
 * Two things fall out of this for free:
 *   - adding two tagged ints is ONE instruction: lea rax,[rcx+rdx-1]
 *   - comparing two tagged ints needs no untagging at all, because
 *     2a+1 < 2b+1 exactly when a < b. The tag preserves order.
 *
 * ---- CALLING CONVENTION (ours; we own both sides) ----
 * args in rcx, rdx, r8; result in rax. Routines that use rsi/rdi (for
 * rep movsb) save and restore them. Every routine keeps rsp 16-byte
 * aligned at nested call sites, which is what Win32 requires.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- image layout ----------------------------------------------------
 * file 0x0000  headers
 * file 0x0200  .text   RVA 0x1000  raw 0x1800  virtual 0x2000
 * file 0x1A00  .idata  RVA 0x3000  raw 0x0400  virtual 0x1000
 */
/* Sized generously so the boxed backend can compile a large program
 * (compiler.zl). The runtime's rip-relative refs to the IAT/globals/data
 * are all derived from IDATA_RVA, so growing these constants is safe -
 * everything recomputes at emit time. .text has room up to IDATA_RVA. */
#define TEXT_RVA   0x1000
#define TEXT_FILE  0x200
#define TEXT_RAW   0x80000        /* 512 KB of machine code             */
#define TEXT_VSIZE 0x80000
#define IDATA_RVA  0x81000
#define IDATA_FILE (TEXT_FILE + TEXT_RAW)
#define IDATA_RAW  0x40000        /* 256 KB: imports + string data       */
#define FILE_SIZE  (IDATA_FILE + IDATA_RAW)

/* IAT entry RVAs (6 imports; the IAT block starts at IDATA_RVA+0x80) */
#define IAT_VALLOC (IDATA_RVA + 0x80)
#define IAT_GETSTD (IDATA_RVA + 0x88)
#define IAT_WRITE  (IDATA_RVA + 0x90)
#define IAT_EXIT   (IDATA_RVA + 0x98)
#define IAT_CREATE (IDATA_RVA + 0xA0)
#define IAT_READ   (IDATA_RVA + 0xA8)
#define IAT_CLOSE  (IDATA_RVA + 0xB0)

/* writable globals, living in the RW .idata section */
#define G_BASE     (IDATA_RVA + 0xC0)   /* arena base   */
#define G_BUMP     (IDATA_RVA + 0xC8)   /* bump pointer */
#define DATA_RVA   (IDATA_RVA + 0x200)  /* static bytes grow from here */

/* ---- static data segment (string literals) ---- */
static unsigned char dataseg[IDATA_RAW - 0x200];
static int datalen = 0;
static unsigned add_data(const char *s, int n)
{
    unsigned rva = DATA_RVA + datalen;
    memcpy(dataseg + datalen, s, n);
    datalen += (n + 7) & ~7;
    return rva;
}
static unsigned S_NL, S_NIL, S_UNK, S_LB, S_RB, S_SEP;  /* literals the runtime itself needs */

/* ---- code buffer ---- */
static unsigned char code[TEXT_RAW];
static int codelen = 0;

static void b(unsigned char x)   { code[codelen++] = x; }
static void b4(int v)            { for (int i=0;i<4;i++) b((unsigned char)((v>>(8*i))&0xFF)); }
static void bs(const unsigned char *p, int n) { for (int i=0;i<n;i++) b(p[i]); }
static void patch4(int at, int v){ for (int i=0;i<4;i++) code[at+i]=(unsigned char)((v>>(8*i))&0xFF); }

/* rip-relative disp32 resolving to `rva`. After b4, codelen is the offset
 * of the next instruction, which is exactly what rip will hold. */
static void rip32(unsigned rva)
{
    int at = codelen; b4(0);
    patch4(at, (int)((int)rva - (int)(TEXT_RVA + codelen)));
}
static void call_iat(unsigned rva)    { b(0xFF); b(0x15); rip32(rva); }          /* call [rip+X]    */
static void mov_mem_rax(unsigned rva) { b(0x48); b(0x89); b(0x05); rip32(rva); } /* mov [rip+X],rax */
static void mov_mem_rcx(unsigned rva) { b(0x48); b(0x89); b(0x0D); rip32(rva); } /* mov [rip+X],rcx */
static void mov_rax_mem(unsigned rva) { b(0x48); b(0x8B); b(0x05); rip32(rva); } /* mov rax,[rip+X] */
static void lea_rsi(unsigned rva)     { b(0x48); b(0x8D); b(0x35); rip32(rva); } /* lea rsi,[rip+X] */
static void lea_rcx(unsigned rva)     { b(0x48); b(0x8D); b(0x0D); rip32(rva); } /* lea rcx,[rip+X] */

static void mov_rcx_imm(int v)        { b(0x48); b(0xC7); b(0xC1); b4(v); }      /* mov rcx,imm32 sign-ext */
static void mov_edx_imm(int v)        { b(0xBA); b4(v); }                        /* mov edx,imm32   */

/* internal calls, backpatched once every routine offset is known.
 * Sized for a big program: the boxed backend emits a runtime call per
 * operation, so compiling compiler.zl produces many thousands. */
static struct { int at; int *target; } cfix[262144];
static int ncfix = 0;
static void call_to(int *t) { b(0xE8); cfix[ncfix].at = codelen; cfix[ncfix].target = t; ncfix++; b4(0); }

/* rel32 forward jumps, landed later */
static int jmp_fwd(void) { b(0xE9);          int at=codelen; b4(0); return at; }
static int jz_fwd (void) { b(0x0F); b(0x84); int at=codelen; b4(0); return at; }
static int jnz_fwd(void) { b(0x0F); b(0x85); int at=codelen; b4(0); return at; }
static int jne_fwd(void) { b(0x0F); b(0x85); int at=codelen; b4(0); return at; }
static int jns_fwd(void) { b(0x0F); b(0x89); int at=codelen; b4(0); return at; }
static void land(int at) { patch4(at, codelen - (at + 4)); }
static void jmp_back(int to) { b(0xE9); int at=codelen; b4(0); patch4(at, to - (at + 4)); }

/* `frame` must be a multiple of 16 so rsp stays aligned at nested calls. */
static void prologue(int frame)
{
    b(0x55);                                        /* push rbp        */
    { unsigned char x[]={0x48,0x89,0xE5}; bs(x,3); }/* mov rbp,rsp     */
    b(0x48); b(0x83); b(0xEC); b((unsigned char)frame);  /* sub rsp,frame */
}
static void epilogue(void) { b(0xC9); b(0xC3); }    /* leave ; ret     */

/* Win32 stack-argument stores (5th+ arg goes at [rsp+0x20], [rsp+0x28], ...) */
static void st_arg32(int off, int imm) { unsigned char x[]={0xC7,0x44,0x24,(unsigned char)off}; bs(x,4); b4(imm); }        /* mov dword[rsp+off],imm */
static void st_arg0 (int off)          { unsigned char x[]={0x48,0xC7,0x44,0x24,(unsigned char)off,0,0,0,0}; bs(x,9); }  /* mov qword[rsp+off],0   */

/* routine offsets */
static int off_alloc=0, off_int=0, off_untag=0, off_addi=0, off_mul=0;
static int off_sub=0, off_div=0, off_mod=0;
static int off_strlit=0, off_concat=0, off_streq=0, off_len=0;
static int off_write=0, off_print=0, off_pint=0, off_pbytes=0;
static int off_add=0, off_eq=0, off_ne=0, off_truthy=0;
static int off_intstr=0, off_tostr=0;
static int off_lt=0, off_le=0, off_gt=0, off_ge=0;
static int off_lnew=0, off_lset=0, off_index=0, off_push=0, off_lcat=0;
static int off_at=0, off_has=0;
static int off_cstr=0, off_read=0, off_writef=0;

/* ===================================================================
 * bricks 1+2: arena, tagging, integer arithmetic
 * =================================================================== */

/* zl_alloc(rcx = nbytes) -> rax. Bump allocator; never frees. A compile
 * run is short-lived, so the whole GC question is deferred by design. */
static void emit_alloc(void)
{
    off_alloc = codelen;
    unsigned char x[] = {
        0x48,0x83,0xC1,0x07,        /* add rcx,7   round up to 8      */
        0x48,0x83,0xE1,0xF8         /* and rcx,-8                     */
    };
    bs(x, sizeof(x));
    mov_rax_mem(G_BUMP);            /* rax = current bump = result    */
    { unsigned char y[]={0x48,0x01,0xC1}; bs(y,3); }   /* add rcx,rax */
    mov_mem_rcx(G_BUMP);
    b(0xC3);
}

/* zl_int(rcx = n) -> rax = (n<<1)|1 */
static void emit_int(void)
{
    off_int = codelen;
    unsigned char x[] = { 0x48,0x8D,0x44,0x09,0x01, 0xC3 };   /* lea rax,[rcx+rcx+1] */
    bs(x, sizeof(x));
}

/* zl_untag(rcx = v) -> rax = v>>1, ARITHMETIC so negatives survive */
static void emit_untag(void)
{
    off_untag = codelen;
    unsigned char x[] = { 0x48,0x89,0xC8, 0x48,0xD1,0xF8, 0xC3 };
    bs(x, sizeof(x));
}

/* zl_add_int(rcx=a, rdx=b) -> rax. (2a+1)+(2b+1)-1 == 2(a+b)+1, so the
 * tag survives and this is a single lea with no untag/retag. */
static void emit_addi(void)
{
    off_addi = codelen;
    unsigned char x[] = { 0x48,0x8D,0x44,0x11,0xFF, 0xC3 };   /* lea rax,[rcx+rdx-1] */
    bs(x, sizeof(x));
}

/* zl_mul(rcx=a, rdx=b) -> rax. Multiply genuinely needs untag/retag. */
static void emit_mul(void)
{
    off_mul = codelen;
    unsigned char x[] = {
        0x48,0xD1,0xF9,             /* sar rcx,1                      */
        0x48,0xD1,0xFA,             /* sar rdx,1                      */
        0x48,0x89,0xC8,             /* mov rax,rcx                    */
        0x48,0x0F,0xAF,0xC2,        /* imul rax,rdx                   */
        0x48,0x8D,0x44,0x00,0x01,   /* lea rax,[rax+rax+1]  retag     */
        0xC3
    };
    bs(x, sizeof(x));
}

/* zl_sub(rcx=a, rdx=b) -> rax. (2a+1)-(2b+1) = 2(a-b); +1 re-tags. */
static void emit_sub(void)
{
    off_sub = codelen;
    unsigned char x[] = {
        0x48,0x89,0xC8,             /* mov rax,rcx                    */
        0x48,0x29,0xD0,             /* sub rax,rdx     = 2(a-b)        */
        0x48,0x83,0xC0,0x01,        /* add rax,1       retag           */
        0xC3
    };
    bs(x, sizeof(x));
}

/* zl_div(rcx=a, rdx=b) -> rax. Untag both, idiv (truncates toward 0,
 * matching the interpreter on exact divisions), retag the quotient. */
static void emit_div(void)
{
    off_div = codelen;
    unsigned char x[] = {
        0x48,0x89,0xC8,             /* mov rax,rcx                    */
        0x48,0xD1,0xF8,             /* sar rax,1       a               */
        0x49,0x89,0xD0,             /* mov r8,rdx                     */
        0x49,0xD1,0xF8,             /* sar r8,1        b (before cqo)  */
        0x48,0x99,                  /* cqo             sign-extend     */
        0x49,0xF7,0xF8,             /* idiv r8         rax=quotient     */
        0x48,0x8D,0x44,0x00,0x01,   /* lea rax,[rax+rax+1]  retag       */
        0xC3
    };
    bs(x, sizeof(x));
}

/* zl_mod(rcx=a, rdx=b) -> rax. Same as div, but keep the remainder (rdx). */
static void emit_mod(void)
{
    off_mod = codelen;
    unsigned char x[] = {
        0x48,0x89,0xC8,             /* mov rax,rcx                    */
        0x48,0xD1,0xF8,             /* sar rax,1       a               */
        0x49,0x89,0xD0,             /* mov r8,rdx                     */
        0x49,0xD1,0xF8,             /* sar r8,1        b               */
        0x48,0x99,                  /* cqo                             */
        0x49,0xF7,0xF8,             /* idiv r8         rdx=remainder    */
        0x48,0x89,0xD0,             /* mov rax,rdx                     */
        0x48,0x8D,0x44,0x00,0x01,   /* lea rax,[rax+rax+1]  retag       */
        0xC3
    };
    bs(x, sizeof(x));
}

/* ===================================================================
 * brick 3: heap strings.  layout: [+0]=0 [+8]=length [+16]=bytes
 * =================================================================== */

/* zl_str_lit(rcx = raw bytes, rdx = len) -> rax = heap string object.
 * Every string literal in a compiled program goes through this. */
static void emit_strlit(void)
{
    off_strlit = codelen;
    prologue(0x40);
    unsigned char a[] = {
        0x48,0x89,0x4D,0xF8,        /* mov [rbp-8],rcx    src         */
        0x48,0x89,0x55,0xF0,        /* mov [rbp-16],rdx   len         */
        0x48,0x89,0x75,0xE8,        /* mov [rbp-24],rsi   save        */
        0x48,0x89,0x7D,0xE0,        /* mov [rbp-32],rdi   save        */
        0x48,0x8D,0x4A,0x10         /* lea rcx,[rdx+16]   nbytes      */
    };
    bs(a, sizeof(a));
    call_to(&off_alloc);
    unsigned char c[] = {
        0x48,0xC7,0x00,0,0,0,0,     /* mov qword[rax],0   type=string */
        0x48,0x8B,0x55,0xF0,        /* mov rdx,[rbp-16]               */
        0x48,0x89,0x50,0x08,        /* mov [rax+8],rdx    length      */
        0x48,0x89,0x45,0xD8,        /* mov [rbp-40],rax   save obj    */
        0x48,0x8B,0x75,0xF8,        /* mov rsi,[rbp-8]    src         */
        0x48,0x8D,0x78,0x10,        /* lea rdi,[rax+16]   dst         */
        0x48,0x8B,0x4D,0xF0,        /* mov rcx,[rbp-16]   count       */
        0xF3,0xA4,                  /* rep movsb                      */
        0x48,0x8B,0x45,0xD8,        /* mov rax,[rbp-40]               */
        0x48,0x8B,0x75,0xE8,        /* mov rsi,[rbp-24]   restore     */
        0x48,0x8B,0x7D,0xE0         /* mov rdi,[rbp-32]   restore     */
    };
    bs(c, sizeof(c));
    epilogue();
}

/* zl_concat(rcx=a, rdx=b) -> rax = new string, a followed by b. */
static void emit_concat(void)
{
    off_concat = codelen;
    prologue(0x50);
    unsigned char a[] = {
        0x48,0x89,0x4D,0xF8,        /* mov [rbp-8],rcx    a           */
        0x48,0x89,0x55,0xF0,        /* mov [rbp-16],rdx   b           */
        0x48,0x89,0x75,0xE8,        /* mov [rbp-24],rsi               */
        0x48,0x89,0x7D,0xE0,        /* mov [rbp-32],rdi               */
        0x4C,0x8B,0x41,0x08,        /* mov r8,[rcx+8]     len a       */
        0x4C,0x8B,0x4A,0x08,        /* mov r9,[rdx+8]     len b       */
        0x4C,0x89,0x45,0xD8,        /* mov [rbp-40],r8                */
        0x4C,0x89,0x4D,0xD0,        /* mov [rbp-48],r9                */
        0x4B,0x8D,0x0C,0x08,        /* lea rcx,[r8+r9]                */
        0x48,0x83,0xC1,0x10         /* add rcx,16                     */
    };
    bs(a, sizeof(a));
    call_to(&off_alloc);
    unsigned char c[] = {
        0x48,0xC7,0x00,0,0,0,0,     /* mov qword[rax],0   type=string */
        0x4C,0x8B,0x45,0xD8,        /* mov r8,[rbp-40]                */
        0x4C,0x8B,0x4D,0xD0,        /* mov r9,[rbp-48]                */
        0x4D,0x01,0xC8,             /* add r8,r9          total len   */
        0x4C,0x89,0x40,0x08,        /* mov [rax+8],r8                 */
        0x48,0x89,0x45,0xC8,        /* mov [rbp-56],rax   save obj    */
        /* copy a's bytes */
        0x48,0x8B,0x75,0xF8,        /* mov rsi,[rbp-8]                */
        0x48,0x83,0xC6,0x10,        /* add rsi,16                     */
        0x48,0x8D,0x78,0x10,        /* lea rdi,[rax+16]               */
        0x48,0x8B,0x4D,0xD8,        /* mov rcx,[rbp-40]               */
        0xF3,0xA4,                  /* rep movsb  (leaves rdi at end) */
        /* copy b's bytes straight onto the end */
        0x48,0x8B,0x75,0xF0,        /* mov rsi,[rbp-16]               */
        0x48,0x83,0xC6,0x10,        /* add rsi,16                     */
        0x48,0x8B,0x4D,0xD0,        /* mov rcx,[rbp-48]               */
        0xF3,0xA4,                  /* rep movsb                      */
        0x48,0x8B,0x45,0xC8,        /* mov rax,[rbp-56]               */
        0x48,0x8B,0x75,0xE8,        /* mov rsi,[rbp-24]               */
        0x48,0x8B,0x7D,0xE0         /* mov rdi,[rbp-32]               */
    };
    bs(c, sizeof(c));
    epilogue();
}

/* zl_str_eq(rcx=a, rdx=b) -> rax = tagged bool (3=true, 1=false).
 * Compares BYTES, not pointers. */
static void emit_streq(void)
{
    off_streq = codelen;
    prologue(0x30);
    unsigned char a[] = {
        0x48,0x89,0x75,0xF8,        /* mov [rbp-8],rsi                */
        0x48,0x89,0x7D,0xF0,        /* mov [rbp-16],rdi               */
        0x4C,0x8B,0x41,0x08,        /* mov r8,[rcx+8]                 */
        0x4C,0x8B,0x4A,0x08,        /* mov r9,[rdx+8]                 */
        0x4D,0x39,0xC8              /* cmp r8,r9                      */
    };
    bs(a, sizeof(a));
    int j_false1 = jne_fwd();
    unsigned char c[] = {
        0x48,0x8D,0x71,0x10,        /* lea rsi,[rcx+16]               */
        0x48,0x8D,0x7A,0x10,        /* lea rdi,[rdx+16]               */
        0x4C,0x89,0xC1,             /* mov rcx,r8    byte count       */
        0xF3,0xA6                   /* repe cmpsb                     */
    };
    bs(c, sizeof(c));
    int j_false2 = jne_fwd();
    { unsigned char t[]={0x48,0xC7,0xC0,0x03,0,0,0}; bs(t,7); }   /* mov rax,3  true  */
    int j_done = jmp_fwd();
    land(j_false1); land(j_false2);
    { unsigned char f[]={0x48,0xC7,0xC0,0x01,0,0,0}; bs(f,7); }   /* mov rax,1  false */
    land(j_done);
    unsigned char r[] = {
        0x48,0x8B,0x75,0xF8,        /* mov rsi,[rbp-8]                */
        0x48,0x8B,0x7D,0xF0         /* mov rdi,[rbp-16]               */
    };
    bs(r, sizeof(r));
    epilogue();
}

/* zl_at(rcx = string, rdx = TAGGED index) -> rax = new 1-char string.
 * compiler.zl uses at() to read the source one character at a time, so
 * it must return a string (comparable with ==), not a raw byte. */
static void emit_at(void)
{
    off_at = codelen;
    prologue(0x30);
    unsigned char a[] = {
        0x48,0x89,0x75,0xF8,        /* mov [rbp-8],rsi                */
        0x48,0x89,0x7D,0xF0,        /* mov [rbp-16],rdi               */
        0x48,0xD1,0xFA,             /* sar rdx,1        untag index   */
        0x0F,0xB6,0x44,0x11,0x10,   /* movzx eax,byte[rcx+rdx+16] char*/
        0x48,0x89,0x45,0xE8,        /* mov [rbp-24],rax  save char    */
        0xB9,0x11,0,0,0             /* mov ecx,17   header16 + 1 byte */
    };
    bs(a, sizeof(a));
    call_to(&off_alloc);            /* rax = object                   */
    unsigned char c[] = {
        0x48,0xC7,0x00,0,0,0,0,          /* mov qword[rax],0   type=string */
        0x48,0xC7,0x40,0x08,0x01,0,0,0,  /* mov qword[rax+8],1  length     */
        0x48,0x8B,0x4D,0xE8,             /* mov rcx,[rbp-24]  char         */
        0x88,0x48,0x10,                  /* mov [rax+16],cl                */
        0x48,0x8B,0x75,0xF8,             /* mov rsi,[rbp-8]                */
        0x48,0x8B,0x7D,0xF0              /* mov rdi,[rbp-16]               */
    };
    bs(c, sizeof(c));
    epilogue();
}

/* zl_has(rcx = haystack, rdx = needle) -> rax = tagged bool.
 * Naive substring search: for each start i where the needle can still
 * fit, compare needle_len bytes. compiler.zl only asks has() about short
 * keyword/punctuation strings, so naive is fine.
 * Frame: [rbp-8] hay-bytes, [rbp-16] needle-bytes, [rbp-24] needle-len,
 *        [rbp-32] last-start (= hay_len - needle_len), and rsi/rdi saved
 *        in [rbp-40]/[rbp-48]. r10 = i.  repe cmpsb clobbers rsi/rdi/rcx
 *        each pass, so all live values are reloaded from the frame. */
static void emit_has(void)
{
    off_has = codelen;
    prologue(0x40);
    unsigned char a[] = {
        0x48,0x89,0x75,0xD8,        /* mov [rbp-40],rsi  save         */
        0x48,0x89,0x7D,0xD0,        /* mov [rbp-48],rdi  save         */
        0x4C,0x8B,0x41,0x08,        /* mov r8,[rcx+8]    hay len      */
        0x4C,0x8B,0x4A,0x08,        /* mov r9,[rdx+8]    needle len   */
        0x48,0x8D,0x41,0x10,        /* lea rax,[rcx+16]  hay bytes    */
        0x48,0x89,0x45,0xF8,        /* mov [rbp-8],rax                */
        0x48,0x8D,0x42,0x10,        /* lea rax,[rdx+16]  needle bytes */
        0x48,0x89,0x45,0xF0,        /* mov [rbp-16],rax               */
        0x4C,0x89,0x4D,0xE8         /* mov [rbp-24],r9   needle len   */
    };
    bs(a, sizeof(a));
    /* empty needle -> true */
    { unsigned char t[]={0x4D,0x85,0xC9}; bs(t,3); }        /* test r9,r9 */
    int j_true0 = jz_fwd();
    /* last_start = hay_len - needle_len; if < 0, needle can't fit -> false */
    unsigned char lim[] = {
        0x4D,0x29,0xC8,             /* sub r8,r9         = last start */
        0x4C,0x89,0x45,0xE0,        /* mov [rbp-32],r8                */
        0x4D,0x85,0xC0              /* test r8,r8                     */
    };
    bs(lim, sizeof(lim));
    int j_false0 = codelen; b(0x0F); b(0x8C); b4(0);        /* jl false */
    { unsigned char z[]={0x4D,0x31,0xD2}; bs(z,3); }        /* xor r10,r10  i=0 */

    int L_outer = codelen;
    unsigned char cmp[] = {
        0x4C,0x8B,0x45,0xE0,        /* mov r8,[rbp-32]   last start   */
        0x4D,0x39,0xC2              /* cmp r10,r8                     */
    };
    bs(cmp, sizeof(cmp));
    int j_false1 = codelen; b(0x0F); b(0x8F); b4(0);        /* jg false */
    unsigned char inner[] = {
        0x48,0x8B,0x75,0xF8,        /* mov rsi,[rbp-8]   hay bytes    */
        0x4C,0x01,0xD6,             /* add rsi,r10       + i          */
        0x48,0x8B,0x7D,0xF0,        /* mov rdi,[rbp-16]  needle bytes */
        0x48,0x8B,0x4D,0xE8,        /* mov rcx,[rbp-24]  needle len   */
        0xF3,0xA6                   /* repe cmpsb                     */
    };
    bs(inner, sizeof(inner));
    int j_match = jz_fwd();         /* all bytes equal -> found       */
    { unsigned char inc[]={0x49,0xFF,0xC2}; bs(inc,3); }    /* inc r10 */
    jmp_back(L_outer);

    land(j_match);
    land(j_true0);
    { unsigned char tt[]={0x48,0xC7,0xC0,0x03,0,0,0}; bs(tt,7); }   /* mov rax,3 true */
    int j_done = jmp_fwd();
    patch4(j_false0+2, codelen-(j_false0+6));
    patch4(j_false1+2, codelen-(j_false1+6));
    { unsigned char ff[]={0x48,0xC7,0xC0,0x01,0,0,0}; bs(ff,7); }   /* mov rax,1 false */
    land(j_done);
    unsigned char r[] = {
        0x48,0x8B,0x75,0xD8,        /* mov rsi,[rbp-40]               */
        0x48,0x8B,0x7D,0xD0         /* mov rdi,[rbp-48]               */
    };
    bs(r, sizeof(r));
    epilogue();
}

/* zl_len(rcx = heap object) -> rax = tagged int.
 * Works for strings AND lists, because both store their count at [+8]. */
static void emit_len(void)
{
    off_len = codelen;
    unsigned char x[] = {
        0x48,0x8B,0x41,0x08,        /* mov rax,[rcx+8]                */
        0x48,0x8D,0x44,0x00,0x01,   /* lea rax,[rax+rax+1]  tag it    */
        0xC3
    };
    bs(x, sizeof(x));
}

/* ===================================================================
 * brick 5: list objects.  layout: [+0]=1 [+8]=count [+16]=count*8 slots
 * =================================================================== */

/* zl_list_new(rcx = count) -> rax. Slots are pre-filled with tagged 0
 * so a half-built list is never printable as a wild pointer. */
static void emit_list_new(void)
{
    off_lnew = codelen;
    prologue(0x30);
    { unsigned char s[]={0x48,0x89,0x4D,0xF8}; bs(s,4); }              /* mov [rbp-8],rcx */
    { unsigned char l[]={0x48,0x8D,0x0C,0xCD,0x10,0,0,0}; bs(l,8); }   /* lea rcx,[rcx*8+16] */
    call_to(&off_alloc);
    unsigned char c[] = {
        0x48,0xC7,0x00,0x01,0,0,0,  /* mov qword[rax],1   type=list   */
        0x48,0x8B,0x55,0xF8,        /* mov rdx,[rbp-8]    count       */
        0x48,0x89,0x50,0x08,        /* mov [rax+8],rdx                */
        0x4C,0x8D,0x40,0x10,        /* lea r8,[rax+16]    first slot  */
        0x4D,0x31,0xC9              /* xor r9,r9          i = 0       */
    };
    bs(c, sizeof(c));
    int L = codelen;
    { unsigned char t[]={0x49,0x39,0xD1}; bs(t,3); }                   /* cmp r9,rdx */
    b(0x0F); b(0x8D); int j_end = codelen; b4(0);                      /* jge end    */
    unsigned char fl[] = {
        0x49,0xC7,0x00,0x01,0,0,0,  /* mov qword[r8],1    tagged zero */
        0x49,0x83,0xC0,0x08,        /* add r8,8                       */
        0x49,0xFF,0xC1              /* inc r9                         */
    };
    bs(fl, sizeof(fl));
    jmp_back(L);
    land(j_end);
    epilogue();
}

/* zl_list_set(rcx = list, rdx = RAW index, r8 = value). Build-time only. */
static void emit_list_set(void)
{
    off_lset = codelen;
    unsigned char x[] = {
        0x48,0x8D,0x44,0xD1,0x10,   /* lea rax,[rcx+rdx*8+16]         */
        0x4C,0x89,0x00,             /* mov [rax],r8                   */
        0xC3
    };
    bs(x, sizeof(x));
}

/* zl_index(rcx = list, rdx = TAGGED index) -> rax = element. */
static void emit_index(void)
{
    off_index = codelen;
    unsigned char x[] = {
        0x48,0x89,0xD0,             /* mov rax,rdx                    */
        0x48,0xD1,0xF8,             /* sar rax,1        untag index   */
        0x48,0x8D,0x44,0xC1,0x10,   /* lea rax,[rcx+rax*8+16]         */
        0x48,0x8B,0x00,             /* mov rax,[rax]                  */
        0xC3
    };
    bs(x, sizeof(x));
}

/* zl_push(rcx = list, rdx = value) -> rax = NEW list, one slot longer.
 * Copy-on-write: the original list is untouched, which is what the
 * interpreter's list semantics already do. */
static void emit_push(void)
{
    off_push = codelen;
    prologue(0x40);
    unsigned char a[] = {
        0x48,0x89,0x4D,0xF8,        /* mov [rbp-8],rcx    list        */
        0x48,0x89,0x55,0xF0,        /* mov [rbp-16],rdx   value       */
        0x48,0x89,0x75,0xE8,        /* mov [rbp-24],rsi               */
        0x48,0x89,0x7D,0xE0,        /* mov [rbp-32],rdi               */
        0x4C,0x8B,0x41,0x08,        /* mov r8,[rcx+8]     old count   */
        0x4C,0x89,0x45,0xD8,        /* mov [rbp-40],r8                */
        0x4A,0x8D,0x0C,0xC5,0x18,0,0,0  /* lea rcx,[r8*8+24]          */
    };
    bs(a, sizeof(a));
    call_to(&off_alloc);
    unsigned char c[] = {
        0x48,0xC7,0x00,0x01,0,0,0,  /* mov qword[rax],1               */
        0x4C,0x8B,0x45,0xD8,        /* mov r8,[rbp-40]                */
        0x4D,0x8D,0x48,0x01,        /* lea r9,[r8+1]      new count   */
        0x4C,0x89,0x48,0x08,        /* mov [rax+8],r9                 */
        0x48,0x89,0x45,0xD0,        /* mov [rbp-48],rax               */
        0x48,0x8B,0x75,0xF8,        /* mov rsi,[rbp-8]                */
        0x48,0x83,0xC6,0x10,        /* add rsi,16                     */
        0x48,0x8D,0x78,0x10,        /* lea rdi,[rax+16]               */
        0x48,0x8B,0x4D,0xD8,        /* mov rcx,[rbp-40]               */
        0x48,0xC1,0xE1,0x03,        /* shl rcx,3          bytes       */
        0xF3,0xA4,                  /* rep movsb  (rdi ends at slot n)*/
        0x48,0x8B,0x55,0xF0,        /* mov rdx,[rbp-16]               */
        0x48,0x89,0x17,             /* mov [rdi],rdx      append      */
        0x48,0x8B,0x45,0xD0,        /* mov rax,[rbp-48]               */
        0x48,0x8B,0x75,0xE8,        /* mov rsi,[rbp-24]               */
        0x48,0x8B,0x7D,0xE0         /* mov rdi,[rbp-32]               */
    };
    bs(c, sizeof(c));
    epilogue();
}

/* zl_list_concat(rcx=a, rdx=b) -> rax = new list, a's slots then b's. */
static void emit_list_concat(void)
{
    off_lcat = codelen;
    prologue(0x50);
    unsigned char a[] = {
        0x48,0x89,0x4D,0xF8,        /* mov [rbp-8],rcx                */
        0x48,0x89,0x55,0xF0,        /* mov [rbp-16],rdx               */
        0x48,0x89,0x75,0xE8,        /* mov [rbp-24],rsi               */
        0x48,0x89,0x7D,0xE0,        /* mov [rbp-32],rdi               */
        0x4C,0x8B,0x41,0x08,        /* mov r8,[rcx+8]                 */
        0x4C,0x8B,0x4A,0x08,        /* mov r9,[rdx+8]                 */
        0x4C,0x89,0x45,0xD8,        /* mov [rbp-40],r8                */
        0x4C,0x89,0x4D,0xD0,        /* mov [rbp-48],r9                */
        0x4B,0x8D,0x0C,0x08,        /* lea rcx,[r8+r9]                */
        0x48,0xC1,0xE1,0x03,        /* shl rcx,3                      */
        0x48,0x83,0xC1,0x10         /* add rcx,16                     */
    };
    bs(a, sizeof(a));
    call_to(&off_alloc);
    unsigned char c[] = {
        0x48,0xC7,0x00,0x01,0,0,0,  /* mov qword[rax],1               */
        0x4C,0x8B,0x45,0xD8,        /* mov r8,[rbp-40]                */
        0x4C,0x8B,0x4D,0xD0,        /* mov r9,[rbp-48]                */
        0x4D,0x01,0xC8,             /* add r8,r9                      */
        0x4C,0x89,0x40,0x08,        /* mov [rax+8],r8                 */
        0x48,0x89,0x45,0xC8,        /* mov [rbp-56],rax               */
        0x48,0x8B,0x75,0xF8,        /* mov rsi,[rbp-8]                */
        0x48,0x83,0xC6,0x10,        /* add rsi,16                     */
        0x48,0x8D,0x78,0x10,        /* lea rdi,[rax+16]               */
        0x48,0x8B,0x4D,0xD8,        /* mov rcx,[rbp-40]               */
        0x48,0xC1,0xE1,0x03,        /* shl rcx,3                      */
        0xF3,0xA4,                  /* rep movsb                      */
        0x48,0x8B,0x75,0xF0,        /* mov rsi,[rbp-16]               */
        0x48,0x83,0xC6,0x10,        /* add rsi,16                     */
        0x48,0x8B,0x4D,0xD0,        /* mov rcx,[rbp-48]               */
        0x48,0xC1,0xE1,0x03,        /* shl rcx,3                      */
        0xF3,0xA4,                  /* rep movsb                      */
        0x48,0x8B,0x45,0xC8,        /* mov rax,[rbp-56]               */
        0x48,0x8B,0x75,0xE8,        /* mov rsi,[rbp-24]               */
        0x48,0x8B,0x7D,0xE0         /* mov rdi,[rbp-32]               */
    };
    bs(c, sizeof(c));
    epilogue();
}

/* ===================================================================
 * brick 4: the polymorphic operators
 * =================================================================== */

/* zl_int_str(rcx = tagged int) -> rax = heap string of its decimal form.
 * itoa into a stack buffer (with '-' for negatives), then zl_str_lit. */
static void emit_int_str(void)
{
    off_intstr = codelen;
    prologue(0x40);
    unsigned char a[] = {
        0x48,0x89,0x75,0xF8,        /* mov [rbp-8],rsi                */
        0x48,0x89,0x7D,0xF0,        /* mov [rbp-16],rdi               */
        0x48,0x89,0xC8,             /* mov rax,rcx                    */
        0x48,0xD1,0xF8,             /* sar rax,1   untag              */
        0x48,0x8D,0x75,0xE8,        /* lea rsi,[rbp-0x18]  buffer end */
        0x45,0x31,0xD2,             /* xor r10d,r10d   neg flag       */
        0x48,0x85,0xC0              /* test rax,rax                   */
    };
    bs(a, sizeof(a));
    int j_nn = jns_fwd();
    { unsigned char ng[]={0x41,0xBA,0x01,0,0,0, 0x48,0xF7,0xD8}; bs(ng,9); }  /* mov r10d,1 ; neg rax */
    land(j_nn);
    { unsigned char t[]={0x48,0x85,0xC0}; bs(t,3); }             /* test rax,rax */
    int j_loop = jnz_fwd();
    { unsigned char z[]={0x48,0xFF,0xCE, 0xC6,0x06,0x30}; bs(z,6); }  /* dec rsi ; [rsi]='0' */
    int j_sign = jmp_fwd();
    land(j_loop);
    { unsigned char m[]={0xB9,0x0A,0,0,0}; bs(m,5); }            /* mov ecx,10 */
    int L = codelen;
    { unsigned char t[]={0x48,0x85,0xC0}; bs(t,3); }             /* test rax,rax */
    int j_done = jz_fwd();
    unsigned char dv[]={0x48,0x31,0xD2, 0x48,0xF7,0xF1, 0x80,0xC2,0x30, 0x48,0xFF,0xCE, 0x88,0x16};
    bs(dv, sizeof(dv));                                          /* xor rdx; div rcx; add dl,'0'; dec rsi; [rsi]=dl */
    jmp_back(L);
    land(j_done);
    land(j_sign);
    { unsigned char t[]={0x45,0x85,0xD2}; bs(t,3); }             /* test r10d,r10d */
    int j_build = jz_fwd();
    { unsigned char mn[]={0x48,0xFF,0xCE, 0xC6,0x06,0x2D}; bs(mn,6); }  /* dec rsi ; [rsi]='-' */
    land(j_build);
    unsigned char b2[] = {
        0x48,0x8D,0x55,0xE8,        /* lea rdx,[rbp-0x18]  end        */
        0x48,0x29,0xF2,             /* sub rdx,rsi         = len       */
        0x48,0x89,0xF1              /* mov rcx,rsi         src         */
    };
    bs(b2, sizeof(b2));
    call_to(&off_strlit);           /* rax = string                   */
    unsigned char r[]={0x48,0x8B,0x75,0xF8, 0x48,0x8B,0x7D,0xF0}; bs(r,8);  /* restore rsi,rdi */
    epilogue();
}

/* zl_to_string(rcx = value) -> rax = a string object.
 *   int -> decimal string ; string -> itself ; nil -> "nil".
 * (Lists are not stringified this way in compiler.zl; they get "?".) */
static void emit_to_string(void)
{
    off_tostr = codelen;
    prologue(0x20);
    { unsigned char t[]={0xF6,0xC1,0x01}; bs(t,3); }             /* test cl,1  int? */
    int j_int = jnz_fwd();
    { unsigned char s[]={0x48,0x8B,0x01, 0x48,0x85,0xC0}; bs(s,6); }  /* mov rax,[rcx] ; test rax,rax */
    int j_notstr = jnz_fwd();
    { unsigned char x[]={0x48,0x89,0xC8}; bs(x,3); }             /* mov rax,rcx  (already a string) */
    int j_d1 = jmp_fwd();
    land(j_notstr);
    { unsigned char n[]={0x48,0x83,0xF8,0x03}; bs(n,4); }        /* cmp rax,3   nil? */
    int j_nnil = jne_fwd();
    lea_rcx(S_NIL); mov_edx_imm(3); call_to(&off_strlit);       /* "nil" */
    int j_d2 = jmp_fwd();
    land(j_nnil);
    lea_rcx(S_UNK); mov_edx_imm(1); call_to(&off_strlit);       /* "?" (lists/other) */
    int j_d3 = jmp_fwd();
    land(j_int);
    call_to(&off_intstr);                                       /* rcx=int -> string */
    land(j_d1); land(j_d2); land(j_d3);
    epilogue();
}

/* zl_add(rcx=a, rdx=b) -> rax. Matches the interpreter's `+`:
 *   int  + int   -> numeric add (one lea)
 *   list + list  -> zl_list_concat
 *   anything else-> zl_concat(to_string(a), to_string(b))   (stringify!)
 * The last case is what makes `"len=" + count` work. */
static void emit_add(void)
{
    off_add = codelen;
    prologue(0x30);
    unsigned char a[] = {
        0x48,0x89,0xC8,             /* mov rax,rcx                    */
        0x48,0x21,0xD0,             /* and rax,rdx                    */
        0xA8,0x01                   /* test al,1   both ints?         */
    };
    bs(a, sizeof(a));
    int j_notint = jz_fwd();
    { unsigned char i[]={0x48,0x8D,0x44,0x11,0xFF}; bs(i,5); }   /* lea rax,[rcx+rdx-1]  add */
    int j_d1 = jmp_fwd();

    land(j_notint);
    /* both lists? (both pointers, both type 1) */
    unsigned char m[]={0x48,0x89,0xC8, 0x48,0x09,0xD0, 0xA8,0x01}; bs(m,8);  /* mov rax,rcx ; or rax,rdx ; test al,1 */
    int j_general = jnz_fwd();       /* at least one int -> stringify path */
    { unsigned char t[]={0x48,0x8B,0x01, 0x48,0x83,0xF8,0x01}; bs(t,7); }    /* mov rax,[rcx] ; cmp rax,1 */
    int j_g2 = jne_fwd();
    { unsigned char t[]={0x48,0x8B,0x02, 0x48,0x83,0xF8,0x01}; bs(t,7); }    /* mov rax,[rdx] ; cmp rax,1 */
    int j_g3 = jne_fwd();
    call_to(&off_lcat);
    int j_d2 = jmp_fwd();

    /* general: zl_concat(to_string(a), to_string(b)) */
    land(j_general); land(j_g2); land(j_g3);
    unsigned char sp[]={0x48,0x83,0xEC,0x10, 0x48,0x89,0x14,0x24}; bs(sp,8);  /* sub rsp,16 ; mov [rsp],rdx */
    call_to(&off_tostr);                                        /* to_string(a) -> rax  (rcx=a) */
    { unsigned char x[]={0x48,0x89,0x44,0x24,0x08}; bs(x,5); }   /* mov [rsp+8],rax   str_a */
    { unsigned char x[]={0x48,0x8B,0x0C,0x24}; bs(x,4); }        /* mov rcx,[rsp]     b */
    call_to(&off_tostr);                                        /* to_string(b) -> rax */
    { unsigned char x[]={0x48,0x89,0xC2}; bs(x,3); }            /* mov rdx,rax       str_b */
    { unsigned char x[]={0x48,0x8B,0x4C,0x24,0x08}; bs(x,5); }   /* mov rcx,[rsp+8]   str_a */
    { unsigned char x[]={0x48,0x83,0xC4,0x10}; bs(x,4); }        /* add rsp,16 */
    call_to(&off_concat);

    land(j_d1); land(j_d2);
    epilogue();
}

/* comparison(rcx=a, rdx=b) -> rax = tagged bool.
 * No untagging: 2a+1 < 2b+1 exactly when a < b, so the tagged values
 * can be compared directly. `setcc` is the whole body. */
static void emit_cmp(int *slot, unsigned char cc)
{
    *slot = codelen;
    unsigned char x[] = {
        0x48,0x39,0xD1,             /* cmp rcx,rdx                    */
        0x0F,cc,0xC0,               /* set<cc> al                     */
        0x0F,0xB6,0xC0,             /* movzx eax,al                   */
        0x48,0x8D,0x44,0x00,0x01,   /* lea rax,[rax+rax+1]  tag it    */
        0xC3
    };
    bs(x, sizeof(x));
}

/* zl_eq(rcx=a, rdx=b) -> rax = tagged bool.
 * ints compare by value, strings by bytes, everything else by identity. */
static void emit_eq(void)
{
    off_eq = codelen;
    prologue(0x20);
    unsigned char a[] = {
        0x48,0x89,0xC8, 0x48,0x21,0xD0, 0xA8,0x01   /* both ints? */
    };
    bs(a, sizeof(a));
    int j_notint = jz_fwd();
    unsigned char i[] = {
        0x48,0x39,0xD1, 0x0F,0x94,0xC0, 0x0F,0xB6,0xC0, 0x48,0x8D,0x44,0x00,0x01
    };
    bs(i, sizeof(i));
    int j_d1 = jmp_fwd();

    land(j_notint);
    unsigned char m[] = { 0x48,0x89,0xC8, 0x48,0x09,0xD0, 0xA8,0x01 };  /* either an int? */
    bs(m, sizeof(m));
    int j_ptr1 = jnz_fwd();
    { unsigned char s[]={0x48,0x8B,0x01, 0x48,0x85,0xC0}; bs(s,6); }    /* a a string? */
    int j_ptr2 = jnz_fwd();
    { unsigned char s[]={0x48,0x8B,0x02, 0x48,0x85,0xC0}; bs(s,6); }    /* b a string? */
    int j_ptr3 = jnz_fwd();
    call_to(&off_streq);
    int j_d2 = jmp_fwd();

    land(j_ptr1); land(j_ptr2); land(j_ptr3);
    unsigned char pc[] = {
        0x48,0x39,0xD1, 0x0F,0x94,0xC0, 0x0F,0xB6,0xC0, 0x48,0x8D,0x44,0x00,0x01
    };
    bs(pc, sizeof(pc));

    land(j_d1); land(j_d2);
    epilogue();
}

/* zl_ne = zl_eq with the tagged bool flipped (3 xor 2 == 1, 1 xor 2 == 3). */
static void emit_ne(void)
{
    off_ne = codelen;
    prologue(0x20);
    call_to(&off_eq);
    { unsigned char x[]={0x48,0x83,0xF0,0x02}; bs(x,4); }   /* xor rax,2 */
    epilogue();
}

/* zl_truthy(rcx = value) -> rax = RAW 0 or 1 (not tagged), for jumps.
 *   int    -> nonzero
 *   string -> length  != 0
 *   list   -> count   != 0
 *   nil    -> false */
static void emit_truthy(void)
{
    off_truthy = codelen;
    { unsigned char t[]={0xF6,0xC1,0x01}; bs(t,3); }        /* test cl,1 */
    int j_heap = jz_fwd();
    unsigned char i[] = {
        0x31,0xC0,                  /* xor eax,eax                    */
        0x48,0x83,0xF9,0x01,        /* cmp rcx,1      tagged zero?    */
        0x0F,0x95,0xC0,             /* setne al                       */
        0xC3
    };
    bs(i, sizeof(i));

    land(j_heap);
    { unsigned char n[]={0x48,0x8B,0x01, 0x48,0x83,0xF8,0x03}; bs(n,7); }  /* mov rax,[rcx] ; cmp rax,3 */
    int j_false = jz_fwd();                                  /* nil -> false */
    { unsigned char g[]={0x48,0x83,0xF8,0x01}; bs(g,4); }    /* cmp rax,1 */
    int j_true = codelen; b(0x0F); b(0x87); b4(0);           /* ja  -> unknown type, treat as true */
    unsigned char cnt[] = {
        0x48,0x8B,0x41,0x08,        /* mov rax,[rcx+8]  length/count  */
        0x31,0xD2,                  /* xor edx,edx                    */
        0x48,0x85,0xC0,             /* test rax,rax                   */
        0x0F,0x95,0xC2,             /* setne dl                       */
        0x48,0x89,0xD0,             /* mov rax,rdx                    */
        0xC3
    };
    bs(cnt, sizeof(cnt));
    patch4(j_true+2, codelen - (j_true + 6));
    { unsigned char x[]={0xB8,0x01,0,0,0, 0xC3}; bs(x,6); }  /* mov eax,1 ; ret */
    land(j_false);
    { unsigned char x[]={0x31,0xC0, 0xC3}; bs(x,3); }        /* xor eax,eax ; ret */
}

/* ===================================================================
 * brick 7: file I/O via kernel32 syscalls, no fopen
 * =================================================================== */

/* zl_cstr(rcx = string obj) -> rax = arena copy, NUL-terminated.
 * CreateFileA needs a C string; zl strings carry a length and are not
 * terminated, so make a terminated copy. */
static void emit_cstr(void)
{
    off_cstr = codelen;
    prologue(0x40);
    unsigned char a[] = {
        0x48,0x89,0x75,0xF8,        /* mov [rbp-8],rsi                */
        0x48,0x89,0x7D,0xF0,        /* mov [rbp-16],rdi               */
        0x48,0x89,0x4D,0xE8,        /* mov [rbp-24],rcx   obj         */
        0x4C,0x8B,0x41,0x08,        /* mov r8,[rcx+8]     len         */
        0x4C,0x89,0x45,0xE0,        /* mov [rbp-32],r8                */
        0x49,0x8D,0x48,0x01         /* lea rcx,[r8+1]     len+1       */
    };
    bs(a, sizeof(a));
    call_to(&off_alloc);
    unsigned char c[] = {
        0x48,0x89,0x45,0xD8,        /* mov [rbp-40],rax   buf         */
        0x48,0x8B,0x4D,0xE8,        /* mov rcx,[rbp-24]   obj         */
        0x48,0x8D,0x71,0x10,        /* lea rsi,[rcx+16]   src         */
        0x48,0x89,0xC7,             /* mov rdi,rax        dst         */
        0x48,0x8B,0x4D,0xE0,        /* mov rcx,[rbp-32]   len         */
        0xF3,0xA4,                  /* rep movsb                      */
        0xC6,0x07,0x00,             /* mov byte[rdi],0    terminate   */
        0x48,0x8B,0x45,0xD8,        /* mov rax,[rbp-40]   buf         */
        0x48,0x8B,0x75,0xF8,        /* mov rsi,[rbp-8]                */
        0x48,0x8B,0x7D,0xF0         /* mov rdi,[rbp-16]               */
    };
    bs(c, sizeof(c));
    epilogue();
}

/* zl_read(rcx = filename string) -> rax = heap string of the contents.
 * CreateFileA(GENERIC_READ) -> ReadFile into a 1 MB buffer object ->
 * set the string length to the byte count ReadFile reported. */
static void emit_read(void)
{
    off_read = codelen;
    prologue(0x60);
    call_to(&off_cstr);             /* rax = NUL-terminated name      */
    unsigned char cf[] = {
        0x48,0x89,0xC1,             /* mov rcx,rax    lpFileName      */
        0xBA,0x00,0x00,0x00,0x80,   /* mov edx,0x80000000 GENERIC_READ*/
        0x41,0xB8,0x01,0x00,0x00,0x00, /* mov r8d,1  FILE_SHARE_READ  */
        0x45,0x31,0xC9              /* xor r9d,r9d   lpSecurity=NULL   */
    };
    bs(cf, sizeof(cf));
    st_arg32(0x20, 3);              /* OPEN_EXISTING                  */
    st_arg32(0x28, 0x80);           /* FILE_ATTRIBUTE_NORMAL          */
    st_arg0 (0x30);                 /* hTemplateFile = NULL           */
    call_iat(IAT_CREATE);
    { unsigned char h[]={0x48,0x89,0x45,0xF0}; bs(h,4); }   /* mov [rbp-16],rax  handle */
    /* buffer object: 16-byte header + 1 MB payload */
    { unsigned char al[]={0xB9,0x10,0x00,0x10,0x00}; bs(al,5); }  /* mov ecx,0x100010 */
    call_to(&off_alloc);
    unsigned char ob[] = {
        0x48,0x89,0x45,0xE8,        /* mov [rbp-24],rax   obj         */
        0x48,0xC7,0x00,0,0,0,0      /* mov qword[rax],0   type=string */
    };
    bs(ob, sizeof(ob));
    /* zero the full 8-byte bytesRead slot: ReadFile writes only the low
     * 32 bits, so clear first to read a clean 64-bit length afterward. */
    { unsigned char zr[]={0x48,0xC7,0x45,0xD8,0,0,0,0}; bs(zr,8); }  /* mov qword[rbp-40],0 */
    unsigned char rf[] = {
        0x48,0x8B,0x4D,0xF0,        /* mov rcx,[rbp-16]   handle      */
        0x48,0x8B,0x55,0xE8,        /* mov rdx,[rbp-24]   obj         */
        0x48,0x83,0xC2,0x10,        /* add rdx,16         buffer      */
        0x41,0xB8,0x00,0x00,0x10,0x00, /* mov r8d,0x100000  count     */
        0x4C,0x8D,0x4D,0xD8         /* lea r9,[rbp-40]    &bytesRead  */
    };
    bs(rf, sizeof(rf));
    st_arg0(0x20);                  /* lpOverlapped = NULL            */
    call_iat(IAT_READ);
    /* CloseHandle(handle) BEFORE loading the return value, since it
     * returns a BOOL in rax and would clobber the object pointer. */
    { unsigned char cl[]={0x48,0x8B,0x4D,0xF0}; bs(cl,4); }   /* mov rcx,[rbp-16] */
    call_iat(IAT_CLOSE);
    unsigned char fin[] = {
        0x48,0x8B,0x45,0xE8,        /* mov rax,[rbp-24]   obj         */
        0x48,0x8B,0x55,0xD8,        /* mov rdx,[rbp-40]   bytesRead    */
        0x48,0x89,0x50,0x08         /* mov [rax+8],rdx    length       */
    };
    bs(fin, sizeof(fin));
    epilogue();
}

/* zl_write_file(rcx = filename string, rdx = content string) -> rax = content.
 * CreateFileA(GENERIC_WRITE, CREATE_ALWAYS) -> WriteFile the bytes. */
static void emit_write_file(void)
{
    off_writef = codelen;
    prologue(0x60);
    { unsigned char s[]={0x48,0x89,0x55,0xF8}; bs(s,4); }   /* mov [rbp-8],rdx  content */
    call_to(&off_cstr);             /* rcx=filename already; rax=cstr */
    unsigned char cf[] = {
        0x48,0x89,0xC1,             /* mov rcx,rax    lpFileName      */
        0xBA,0x00,0x00,0x00,0x40,   /* mov edx,0x40000000 GENERIC_WRITE */
        0x41,0xB8,0x01,0x00,0x00,0x00, /* mov r8d,1  FILE_SHARE_READ  */
        0x45,0x31,0xC9             /* xor r9d,r9d   lpSecurity=0      */
    };
    bs(cf, sizeof(cf));
    st_arg32(0x20, 2);              /* CREATE_ALWAYS                  */
    st_arg32(0x28, 0x80);           /* FILE_ATTRIBUTE_NORMAL          */
    st_arg0 (0x30);                 /* hTemplateFile = NULL           */
    call_iat(IAT_CREATE);
    { unsigned char h[]={0x48,0x89,0x45,0xF0}; bs(h,4); }   /* mov [rbp-16],rax  handle */
    unsigned char wf[] = {
        0x48,0x89,0xC1,             /* mov rcx,rax        handle      */
        0x48,0x8B,0x55,0xF8,        /* mov rdx,[rbp-8]    content     */
        0x4C,0x8B,0x42,0x08,        /* mov r8,[rdx+8]     length      */
        0x48,0x83,0xC2,0x10,        /* add rdx,16         buffer      */
        0x4C,0x8D,0x4D,0xD8         /* lea r9,[rbp-40]    &written    */
    };
    bs(wf, sizeof(wf));
    st_arg0(0x20);                  /* lpOverlapped = NULL            */
    call_iat(IAT_WRITE);
    /* CloseHandle so the bytes are flushed and the file is openable by a
     * later read (in this process or another). */
    { unsigned char cl[]={0x48,0x8B,0x4D,0xF0}; bs(cl,4); }   /* mov rcx,[rbp-16] */
    call_iat(IAT_CLOSE);
    { unsigned char r[]={0x48,0x8B,0x45,0xF8}; bs(r,4); }   /* mov rax,[rbp-8]  return content */
    epilogue();
}

/* ===================================================================
 * output
 * =================================================================== */

/* print_bytes(rsi = ptr, rdx = len): one WriteFile to stdout. */
static void emit_print_bytes(void)
{
    off_pbytes = codelen;
    prologue(0x40);
    unsigned char a[] = {
        0x48,0x89,0x55,0xE0,        /* mov [rbp-0x20],rdx  len        */
        0xB9,0xF5,0xFF,0xFF,0xFF    /* mov ecx,-11  STD_OUTPUT_HANDLE */
    };
    bs(a, sizeof(a));
    call_iat(IAT_GETSTD);
    unsigned char c[] = {
        0x48,0x89,0xC1,             /* mov rcx,rax  handle            */
        0x48,0x89,0xF2,             /* mov rdx,rsi  buffer            */
        0x4C,0x8B,0x45,0xE0,        /* mov r8,[rbp-0x20]  len         */
        0x4C,0x8D,0x4D,0xD8,        /* lea r9,[rbp-0x28]  &written    */
        0x48,0xC7,0x44,0x24,0x20,0,0,0,0
    };
    bs(c, sizeof(c));
    call_iat(IAT_WRITE);
    epilogue();
}

/* print_int(rax = raw signed int): decimal ASCII, NO trailing newline.
 * Digits are built backwards into a stack buffer, then one WriteFile.
 * Handles negatives, which the older nativegen print_int does not. */
static void emit_print_int(void)
{
    off_pint = codelen;
    prologue(0x60);
    unsigned char a[] = {
        0x48,0x8D,0x75,0xF0,        /* lea rsi,[rbp-0x10]  buffer end */
        0x45,0x31,0xD2,             /* xor r10d,r10d       neg flag   */
        0x48,0x85,0xC0              /* test rax,rax                   */
    };
    bs(a, sizeof(a));
    int j_nonneg = jns_fwd();
    unsigned char ng[] = {
        0x41,0xBA,0x01,0,0,0,       /* mov r10d,1                     */
        0x48,0xF7,0xD8              /* neg rax                        */
    };
    bs(ng, sizeof(ng));
    land(j_nonneg);

    { unsigned char t[]={0x48,0x85,0xC0}; bs(t,3); }                  /* test rax,rax */
    int j_loop = jnz_fwd();
    { unsigned char z[]={0x48,0xFF,0xCE, 0xC6,0x06,0x30}; bs(z,6); }  /* dec rsi ; [rsi]='0' */
    int j_tosign = jmp_fwd();

    land(j_loop);
    { unsigned char m[]={0xB9,0x0A,0,0,0}; bs(m,5); }                 /* mov ecx,10 */
    int L_next = codelen;
    { unsigned char t[]={0x48,0x85,0xC0}; bs(t,3); }                  /* test rax,rax */
    int j_done = jz_fwd();
    unsigned char dv[] = {
        0x48,0x31,0xD2,             /* xor rdx,rdx                    */
        0x48,0xF7,0xF1,             /* div rcx                        */
        0x80,0xC2,0x30,             /* add dl,'0'                     */
        0x48,0xFF,0xCE,             /* dec rsi                        */
        0x88,0x16                   /* mov [rsi],dl                   */
    };
    bs(dv, sizeof(dv));
    jmp_back(L_next);
    land(j_done);
    land(j_tosign);

    { unsigned char t[]={0x45,0x85,0xD2}; bs(t,3); }                   /* test r10d,r10d */
    int j_write = jz_fwd();
    { unsigned char mn[]={0x48,0xFF,0xCE, 0xC6,0x06,0x2D}; bs(mn,6); } /* dec rsi ; [rsi]='-' */
    land(j_write);

    unsigned char w[] = {
        0x48,0x8D,0x55,0xF0,        /* lea rdx,[rbp-0x10]             */
        0x48,0x29,0xF2,             /* sub rdx,rsi     = length       */
        0x48,0x89,0x55,0xB8,        /* mov [rbp-0x48],rdx             */
        0xB9,0xF5,0xFF,0xFF,0xFF    /* mov ecx,-11                    */
    };
    bs(w, sizeof(w));
    call_iat(IAT_GETSTD);
    unsigned char c[] = {
        0x48,0x89,0xC1,             /* mov rcx,rax                    */
        0x48,0x89,0xF2,             /* mov rdx,rsi                    */
        0x4C,0x8B,0x45,0xB8,        /* mov r8,[rbp-0x48]              */
        0x4C,0x8D,0x4D,0xC8,        /* lea r9,[rbp-0x38]              */
        0x48,0xC7,0x44,0x24,0x20,0,0,0,0
    };
    bs(c, sizeof(c));
    call_iat(IAT_WRITE);
    epilogue();
}

/* zl_write(rcx = value): THE TAG DISPATCH. Writes with no trailing
 * newline, so lists (brick 5) can call it recursively per element. */
static void emit_write(void)
{
    off_write = codelen;
    prologue(0x30);
    { unsigned char s[]={0x48,0x89,0x4D,0xF8}; bs(s,4); }   /* mov [rbp-8],rcx */
    { unsigned char t[]={0xF6,0xC1,0x01}; bs(t,3); }        /* test cl,1  <-- the tag test */
    int j_heap = jz_fwd();

    /* --- low bit set: integer --- */
    { unsigned char i[]={0x48,0x89,0xC8, 0x48,0xD1,0xF8}; bs(i,6); }  /* mov rax,rcx ; sar rax,1 */
    call_to(&off_pint);
    int j_d1 = jmp_fwd();

    /* --- low bit clear: heap object, switch on its type tag --- */
    land(j_heap);
    { unsigned char h[]={0x48,0x8B,0x01, 0x48,0x85,0xC0}; bs(h,6); }  /* mov rax,[rcx] ; test rax,rax */
    int j_not_str = jnz_fwd();
    /* type 0: string */
    unsigned char st[] = {
        0x48,0x8B,0x4D,0xF8,        /* mov rcx,[rbp-8]                */
        0x48,0x8D,0x71,0x10,        /* lea rsi,[rcx+16]  bytes        */
        0x48,0x8B,0x51,0x08         /* mov rdx,[rcx+8]   length       */
    };
    bs(st, sizeof(st));
    call_to(&off_pbytes);
    int j_d2 = jmp_fwd();

    /* type 1: list -> "[a, b, c]", RECURSING through zl_write per element.
     * The loop counter and count live in this frame ([rbp-16], [rbp-24]),
     * so the recursive call, which pushes its own rbp, cannot disturb them. */
    land(j_not_str);
    { unsigned char n[]={0x48,0x83,0xF8,0x01}; bs(n,4); }   /* cmp rax,1   list? */
    int j_notlist = jne_fwd();
    unsigned char li[] = {
        0x48,0x8B,0x4D,0xF8,        /* mov rcx,[rbp-8]                */
        0x48,0x8B,0x41,0x08,        /* mov rax,[rcx+8]   count        */
        0x48,0x89,0x45,0xE8,        /* mov [rbp-24],rax               */
        0x48,0xC7,0x45,0xF0,0,0,0,0 /* mov qword[rbp-16],0   i = 0    */
    };
    bs(li, sizeof(li));
    lea_rsi(S_LB); mov_edx_imm(1); call_to(&off_pbytes);    /* "["            */

    int L_loop = codelen;
    { unsigned char c[]={0x48,0x8B,0x45,0xF0, 0x48,0x3B,0x45,0xE8}; bs(c,8); } /* mov rax,[rbp-16] ; cmp rax,[rbp-24] */
    b(0x0F); b(0x8D); int j_lend = codelen; b4(0);          /* jge  end       */
    { unsigned char t[]={0x48,0x85,0xC0}; bs(t,3); }        /* test rax,rax   */
    int j_nosep = jz_fwd();
    lea_rsi(S_SEP); mov_edx_imm(2); call_to(&off_pbytes);   /* ", "           */
    land(j_nosep);
    unsigned char el[] = {
        0x48,0x8B,0x4D,0xF8,        /* mov rcx,[rbp-8]    list        */
        0x48,0x8B,0x45,0xF0,        /* mov rax,[rbp-16]   i           */
        0x48,0x8D,0x44,0xC1,0x10,   /* lea rax,[rcx+rax*8+16]         */
        0x48,0x8B,0x08              /* mov rcx,[rax]      element     */
    };
    bs(el, sizeof(el));
    call_to(&off_write);            /* <-- recursion                  */
    unsigned char nx[] = {
        0x48,0x8B,0x45,0xF0,        /* mov rax,[rbp-16]               */
        0x48,0xFF,0xC0,             /* inc rax                        */
        0x48,0x89,0x45,0xF0         /* mov [rbp-16],rax               */
    };
    bs(nx, sizeof(nx));
    jmp_back(L_loop);
    land(j_lend);
    lea_rsi(S_RB); mov_edx_imm(1); call_to(&off_pbytes);    /* "]"            */
    int j_d4 = jmp_fwd();

    land(j_notlist);
    { unsigned char n[]={0x48,0x83,0xF8,0x03}; bs(n,4); }   /* cmp rax,3   nil? */
    int j_unk = jne_fwd();
    lea_rsi(S_NIL); mov_edx_imm(3);
    call_to(&off_pbytes);
    int j_d3 = jmp_fwd();

    land(j_unk);
    lea_rsi(S_UNK); mov_edx_imm(1);
    call_to(&off_pbytes);

    land(j_d1); land(j_d2); land(j_d3); land(j_d4);
    epilogue();
}

/* zl_print(rcx = value): zl_write, then a newline. */
static void emit_print(void)
{
    off_print = codelen;
    prologue(0x20);
    call_to(&off_write);
    lea_rsi(S_NL); mov_edx_imm(1);
    call_to(&off_pbytes);
    epilogue();
}

/* ===================================================================
 * the proof program (entry point, RVA 0x1000)
 *
 * Compiled only when this file is built as the standalone proof
 * (raw_rt.exe). When nativeval.c does `#define NATIVERT_LIB` and includes
 * this file to reuse the runtime, the proof and its driver main() are
 * skipped, and nativeval.c supplies its own entry-point codegen instead.
 * =================================================================== */
#ifndef NATIVERT_LIB

static void mov_rcx_rax(void) { unsigned char x[]={0x48,0x89,0xC1}; bs(x,3); }
static void mov_rdx_rax(void) { unsigned char x[]={0x48,0x89,0xC2}; bs(x,3); }
static void mov_rbx_rax(void) { unsigned char x[]={0x48,0x89,0xC3}; bs(x,3); }
static void mov_r12_rax(void) { unsigned char x[]={0x49,0x89,0xC4}; bs(x,3); }
static void mov_r13_rax(void) { unsigned char x[]={0x49,0x89,0xC5}; bs(x,3); }
static void mov_rcx_rbx(void) { unsigned char x[]={0x48,0x89,0xD9}; bs(x,3); }
static void mov_rcx_r12(void) { unsigned char x[]={0x4C,0x89,0xE1}; bs(x,3); }
static void mov_rcx_r13(void) { unsigned char x[]={0x4C,0x89,0xE9}; bs(x,3); }
static void mov_rdx_r12(void) { unsigned char x[]={0x4C,0x89,0xE2}; bs(x,3); }

static void mov_r14_rax(void) { unsigned char x[]={0x49,0x89,0xC6}; bs(x,3); }
static void mov_r15_rax(void) { unsigned char x[]={0x49,0x89,0xC7}; bs(x,3); }
static void mov_rcx_r14(void) { unsigned char x[]={0x4C,0x89,0xF1}; bs(x,3); }
static void mov_rcx_r15(void) { unsigned char x[]={0x4C,0x89,0xF9}; bs(x,3); }
static void mov_rdx_r15(void) { unsigned char x[]={0x4C,0x89,0xFA}; bs(x,3); }
static void mov_r8_r12 (void) { unsigned char x[]={0x4D,0x89,0xE0}; bs(x,3); }
static void mov_r8_r14 (void) { unsigned char x[]={0x4D,0x89,0xF0}; bs(x,3); }
static void mov_rdx_imm(int v){ b(0x48); b(0xC7); b(0xC2); b4(v); }
static void mov_r8_imm (int v){ b(0x49); b(0xC7); b(0xC0); b4(v); }

/* a tagged-int constant, materialised straight into a register: no
 * call to zl_int needed, because the tag is known at compile time. */
#define TAG(n) (((n)<<1)|1)
static void rcx_tag(int n) { mov_rcx_imm(TAG(n)); }
static void rdx_tag(int n) { mov_rdx_imm(TAG(n)); }
static void r8_tag (int n) { mov_r8_imm (TAG(n)); }

/* print(zl_int(n)) */
static void say_int(int n) { mov_rcx_imm(n); call_to(&off_int); mov_rcx_rax(); call_to(&off_print); }
/* rax = zl_str_lit("...") */
static void make_str(const char *s) { int n=(int)strlen(s); lea_rcx(add_data(s,n)); mov_edx_imm(n); call_to(&off_strlit); }
/* print whatever is in rax */
static void say_rax(void) { mov_rcx_rax(); call_to(&off_print); }
/* zl_truthy returns a RAW 0/1 (it feeds jumps), so tag it before printing */
static void say_truthy(void) { call_to(&off_truthy); mov_rcx_rax(); call_to(&off_int); mov_rcx_rax(); call_to(&off_print); }

static void emit_main(void)
{
    unsigned char a[] = {
        0x55,                       /* push rbp                       */
        0x48,0x89,0xE5,             /* mov rbp,rsp                    */
        0x48,0x83,0xEC,0x40,        /* sub rsp,0x40                   */
        0x48,0x83,0xE4,0xF0,        /* and rsp,-16   (brick-1 lesson) */
        /* VirtualAlloc(NULL, 4 MB, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE) */
        0x31,0xC9,                  /* xor ecx,ecx                    */
        0xBA,0x00,0x00,0x40,0x00,   /* mov edx,0x400000               */
        0x41,0xB8,0x00,0x30,0x00,0x00,
        0x41,0xB9,0x04,0x00,0x00,0x00
    };
    bs(a, sizeof(a));
    call_iat(IAT_VALLOC);
    mov_mem_rax(G_BASE);
    mov_mem_rax(G_BUMP);            /* empty arena: bump == base      */

    /* ---- brick 2: tagged integers ---- */
    say_int(42);                                  /* 42              */
    say_int(-7);                                  /* -7   sar sign   */
    say_int(0);                                   /* 0               */

    /* ---- brick 3: heap strings ---- */
    make_str("hello, ");  mov_rbx_rax();          /* rbx = s1        */
    mov_rcx_rbx();  call_to(&off_print);          /* "hello, "       */

    make_str("world");    mov_r12_rax();          /* r12 = s2        */
    mov_rcx_r12();  call_to(&off_print);          /* "world"         */

    mov_rcx_rbx(); mov_rdx_r12();
    call_to(&off_concat);  mov_r13_rax();         /* r13 = s1 + s2   */
    mov_rcx_r13();  call_to(&off_print);          /* "hello, world"  */

    mov_rcx_r13();  call_to(&off_len);  say_rax();/* 12              */

    /* equality against a SEPARATE literal object with the same bytes,
     * so this proves a byte compare rather than a pointer compare. */
    make_str("hello, world");  mov_rdx_rax();
    mov_rcx_r13();  call_to(&off_streq);  say_rax();   /* 1  true    */

    mov_rcx_r13(); mov_rdx_r12();
    call_to(&off_streq);  say_rax();                   /* 0  false   */

    /* empty string is the concat identity, and has length 0 */
    make_str("");  mov_rdx_rax();
    mov_rcx_r13();  call_to(&off_concat);  say_rax();  /* "hello, world" */

    make_str("");  mov_rcx_rax();  call_to(&off_len);  say_rax();  /* 0 */

    /* ---- brick 4: zl_add DISPATCHES on the operand types ---- */
    rcx_tag(20); rdx_tag(22);
    call_to(&off_add);  say_rax();                     /* 42            */

    mov_rcx_rbx(); mov_rdx_r12();                      /* two strings   */
    call_to(&off_add);  say_rax();                     /* hello, world  */

    /* ---- brick 4: comparisons, done on the TAGGED values directly ---- */
    rcx_tag(3);  rdx_tag(5);   call_to(&off_lt);  say_rax();   /* 1  3<5   */
    rcx_tag(5);  rdx_tag(3);   call_to(&off_lt);  say_rax();   /* 0  5<3   */
    rcx_tag(-5); rdx_tag(3);   call_to(&off_lt);  say_rax();   /* 1 signed */
    rcx_tag(5);  rdx_tag(5);   call_to(&off_ge);  say_rax();   /* 1  5>=5  */
    rcx_tag(9);  rdx_tag(2);   call_to(&off_gt);  say_rax();   /* 1  9>2   */

    rcx_tag(7);  rdx_tag(7);   call_to(&off_eq);  say_rax();   /* 1        */
    rcx_tag(7);  rdx_tag(8);   call_to(&off_eq);  say_rax();   /* 0        */
    /* eq on strings routes through zl_str_eq, so it compares bytes */
    make_str("hello, world");  mov_rdx_rax();
    mov_rcx_r13();  call_to(&off_eq);  say_rax();              /* 1        */
    mov_rcx_r13(); mov_rdx_r12();  call_to(&off_ne);  say_rax();/* 1       */

    /* ---- brick 4: truthiness ---- */
    rcx_tag(0);  say_truthy();                         /* 0  zero        */
    rcx_tag(5);  say_truthy();                         /* 1  nonzero     */
    make_str("");   mov_rcx_rax();  say_truthy();      /* 0  empty str   */
    mov_rcx_r12();  say_truthy();                      /* 1  "world"     */

    /* ---- brick 5: lists ---- */
    mov_rcx_imm(3);  call_to(&off_lnew);  mov_r14_rax();       /* L = [_,_,_] */
    mov_rcx_r14(); mov_rdx_imm(0); r8_tag(10); call_to(&off_lset);
    mov_rcx_r14(); mov_rdx_imm(1); r8_tag(20); call_to(&off_lset);
    mov_rcx_r14(); mov_rdx_imm(2); r8_tag(30); call_to(&off_lset);
    mov_rcx_r14(); call_to(&off_print);                /* [10, 20, 30]   */

    mov_rcx_r14(); call_to(&off_len);   say_rax();     /* 3              */
    mov_rcx_r14(); rdx_tag(1); call_to(&off_index); say_rax();  /* 20    */

    mov_rcx_r14(); rdx_tag(40);
    call_to(&off_push);  mov_r15_rax();                /* L2 = L + [40]  */
    mov_rcx_r15(); call_to(&off_print);                /* [10,20,30,40]  */
    mov_rcx_r14(); call_to(&off_print);                /* [10,20,30] - L unchanged */

    /* list + list, again through the zl_add dispatch */
    mov_rcx_r14(); mov_rdx_r15();
    call_to(&off_add);  say_rax();                     /* [10,20,30,10,20,30,40] */

    /* nested: a list holding a string AND another list, so zl_write recurses */
    mov_rcx_imm(2);  call_to(&off_lnew);  mov_rbx_rax();
    mov_rcx_rbx(); mov_rdx_imm(0); mov_r8_r12(); call_to(&off_lset);
    mov_rcx_rbx(); mov_rdx_imm(1); mov_r8_r14(); call_to(&off_lset);
    mov_rcx_rbx(); call_to(&off_print);                /* [world, [10, 20, 30]] */

    mov_rcx_imm(0);  call_to(&off_lnew);  say_rax();   /* []             */

    /* ---- brick 6a: the two string builtins compiler.zl needs ---- */
    /* at("hello, world", 1) -> "e" ; index 7 -> "w" */
    mov_rcx_r13(); rdx_tag(1);  call_to(&off_at);  say_rax();   /* e     */
    mov_rcx_r13(); rdx_tag(7);  call_to(&off_at);  say_rax();   /* w     */
    /* the char is a real string: at(...,0)=="h" via zl_eq */
    mov_rcx_r13(); rdx_tag(0);  call_to(&off_at);  mov_rbx_rax();
    make_str("h");  mov_rdx_rax();  mov_rcx_rbx();
    call_to(&off_eq);  say_rax();                               /* 1     */

    /* has("hello, world", "lo, w") -> true ; ("xyz") -> false */
    make_str("lo, w");  mov_rdx_rax();
    mov_rcx_r13();  call_to(&off_has);  say_rax();              /* 1     */
    make_str("xyz");    mov_rdx_rax();
    mov_rcx_r13();  call_to(&off_has);  say_rax();              /* 0     */
    /* needle at the very end, and an empty needle */
    make_str("world");  mov_rdx_rax();
    mov_rcx_r13();  call_to(&off_has);  say_rax();              /* 1     */
    make_str("");       mov_rdx_rax();
    mov_rcx_r13();  call_to(&off_has);  say_rax();              /* 1     */

    /* ---- brick 7: real file I/O, no fopen ---- */
    /* first, read a file that already exists on disk (isolates read from
     * the write-then-read case). hello.zl is 338 bytes. */
    make_str("hello.zl");  mov_rcx_rax();
    call_to(&off_read);  mov_rcx_rax();  call_to(&off_len);  say_rax();   /* 338 */

    /* write("rt_test.txt", "file io works") then read it back */
    make_str("rt_test.txt");  mov_r14_rax();
    make_str("file io works");  mov_rdx_rax();  mov_rcx_r14();
    call_to(&off_writef);                              /* file written  */
    mov_rcx_r14();  call_to(&off_read);  mov_r15_rax();/* r15 = contents*/
    mov_rcx_r15();  call_to(&off_print);               /* file io works */
    mov_rcx_r15();  call_to(&off_len);   say_rax();    /* 13            */
    /* round-trip equality: read-back == the literal we wrote */
    make_str("file io works");  mov_rdx_rax();
    mov_rcx_r15();  call_to(&off_eq);    say_rax();     /* 1            */

    /* ---- brick 2: the pointer branch, via a nil object ---- */
    mov_rcx_imm(8);
    call_to(&off_alloc);
    { unsigned char x[]={0x48,0xC7,0x00,0x03,0,0,0}; bs(x,7); }   /* mov qword[rax],3 */
    say_rax();                                    /* nil             */

    /* ExitProcess(0) */
    b(0x31); b(0xC9);
    call_iat(IAT_EXIT);
}
#endif /* NATIVERT_LIB */

/* ===================================================================
 * shared driver: literals, runtime emit, call resolution, PE writer.
 * These are used by BOTH the proof (below) and nativeval.c.
 * =================================================================== */

static unsigned char pe[FILE_SIZE];
static void pu16(int o, unsigned v){ pe[o]=v&0xFF; pe[o+1]=(v>>8)&0xFF; }
static void pu32(int o, unsigned v){ for(int i=0;i<4;i++) pe[o+i]=(v>>(8*i))&0xFF; }
static void pu64(int o, unsigned long long v){ for(int i=0;i<8;i++) pe[o+i]=(unsigned char)((v>>(8*i))&0xFF); }

/* Assign the runtime's own string literals FIRST so their RVAs are stable
 * (zl_write's nil/"?"/"[" etc. are baked into the routines by RVA). */
static void rt_init_literals(void)
{
    S_NL  = add_data("\n", 1);
    S_NIL = add_data("nil", 3);
    S_UNK = add_data("?", 1);
    S_LB  = add_data("[", 1);
    S_RB  = add_data("]", 1);
    S_SEP = add_data(", ", 2);
}

/* Emit every runtime routine, in a fixed order. Call AFTER the program's
 * own entry code so the entry sits at RVA 0x1000. */
static void emit_runtime(void)
{
    emit_alloc();
    emit_int();
    emit_untag();
    emit_addi();
    emit_mul();
    emit_sub();
    emit_div();
    emit_mod();
    emit_strlit();
    emit_concat();
    emit_streq();
    emit_at();
    emit_has();
    emit_len();
    emit_list_new();
    emit_list_set();
    emit_index();
    emit_push();
    emit_list_concat();
    emit_int_str();
    emit_to_string();
    emit_add();
    emit_eq();
    emit_ne();
    emit_truthy();
    emit_cmp(&off_lt, 0x9C);        /* setl  */
    emit_cmp(&off_le, 0x9E);        /* setle */
    emit_cmp(&off_gt, 0x9F);        /* setg  */
    emit_cmp(&off_ge, 0x9D);        /* setge */
    emit_cstr();
    emit_read();
    emit_write_file();
    emit_print_bytes();
    emit_print_int();
    emit_write();
    emit_print();
}

/* Backpatch every internal (runtime) call site now that all offsets exist. */
static void resolve_runtime_calls(void)
{
    for (int i=0;i<ncfix;i++)
        patch4(cfix[i].at, *cfix[i].target - (cfix[i].at + 4));
}

/* Assemble the PE and write it. codelen/datalen must be final. */
static void write_pe(const char *path)
{
    if (codelen > TEXT_RAW) { fprintf(stderr,"code overflow: %d > %d\n", codelen, TEXT_RAW); exit(1); }
    if (datalen > (int)sizeof(dataseg)) { fprintf(stderr,"data overflow: %d\n", datalen); exit(1); }

    memset(pe, 0, sizeof(pe));
    pe[0]='M'; pe[1]='Z'; pu32(0x3C,0x40);
    int p=0x40; pe[p]='P'; pe[p+1]='E';
    int coff=p+4;
    pu16(coff+0,0x8664); pu16(coff+2,2); pu16(coff+16,0xF0); pu16(coff+18,0x0022);
    int opt=coff+20;
    pu16(opt+0,0x020B); pu32(opt+4,TEXT_RAW); pu32(opt+8,IDATA_RAW);
    pu32(opt+16,TEXT_RVA); pu32(opt+20,TEXT_RVA); pu64(opt+24,0x140000000ULL);
    pu32(opt+32,0x1000); pu32(opt+36,0x200); pu16(opt+40,6); pu16(opt+48,6);
    pu32(opt+56,IDATA_RVA+IDATA_RAW); pu32(opt+60,0x200); pu16(opt+68,3);  /* SizeOfImage; CONSOLE */
    pu64(opt+72,0x100000ULL); pu64(opt+80,0x1000ULL);
    pu64(opt+88,0x100000ULL); pu64(opt+96,0x1000ULL);
    pu32(opt+108,16);
    int dd=opt+112;
    pu32(dd+1*8+0, IDATA_RVA);       pu32(dd+1*8+4, 40);   /* import table */
    pu32(dd+12*8+0, IDATA_RVA+0x80); pu32(dd+12*8+4, 64);  /* IAT (7+null) */

    int sec=opt+0xF0;
    memcpy(pe+sec,".text",5);
    pu32(sec+8,TEXT_VSIZE); pu32(sec+12,TEXT_RVA); pu32(sec+16,TEXT_RAW); pu32(sec+20,TEXT_FILE);
    pu32(sec+36,0x60000020);                                /* CODE|EXEC|READ  */
    int s2=sec+40;
    memcpy(pe+s2,".idata",6);
    pu32(s2+8,IDATA_RAW); pu32(s2+12,IDATA_RVA); pu32(s2+16,IDATA_RAW); pu32(s2+20,IDATA_FILE);
    pu32(s2+36,0xC0000040);                                 /* DATA|READ|WRITE */

    memcpy(pe+TEXT_FILE, code, codelen);

    int d = IDATA_FILE;
    pu32(d+0,  IDATA_RVA+0x28);   /* OriginalFirstThunk (ILT) */
    pu32(d+12, IDATA_RVA+0x180);  /* Name -> "kernel32.dll"   */
    pu32(d+16, IDATA_RVA+0x80);   /* FirstThunk (IAT)         */
    /* name-entry RVAs, laid out at 0x100,0x110,...  (7 imports) */
    static const int nrva[7] = { 0x100,0x110,0x120,0x130,0x140,0x150,0x160 };
    for (int i=0;i<7;i++) {
        pu64(d+0x28 + i*8, IDATA_RVA + nrva[i]);   /* ILT */
        pu64(d+0x80 + i*8, IDATA_RVA + nrva[i]);   /* IAT (loader overwrites) */
    }
    /* ILT[7] and IAT[7] null terminators are already zero */
    pu16(d+0x100,0); memcpy(pe+d+0x102,"VirtualAlloc",12);
    pu16(d+0x110,0); memcpy(pe+d+0x112,"GetStdHandle",12);
    pu16(d+0x120,0); memcpy(pe+d+0x122,"WriteFile",9);
    pu16(d+0x130,0); memcpy(pe+d+0x132,"ExitProcess",11);
    pu16(d+0x140,0); memcpy(pe+d+0x142,"CreateFileA",11);
    pu16(d+0x150,0); memcpy(pe+d+0x152,"ReadFile",8);
    pu16(d+0x160,0); memcpy(pe+d+0x162,"CloseHandle",11);
    memcpy(pe+d+0x180,"kernel32.dll",12);
    memcpy(pe+d+0x200, dataseg, datalen);

    FILE *f=fopen(path,"wb");
    if(!f){ fprintf(stderr,"can't write %s\n", path); exit(1); }
    fwrite(pe,1,sizeof(pe),f);
    fclose(f);
}

/* ===================================================================
 * standalone proof driver (raw_rt.exe) - skipped when built as a library
 * =================================================================== */
#ifndef NATIVERT_LIB
int main(void)
{
    rt_init_literals();
    emit_main();            /* entry point must sit at RVA 0x1000 */
    emit_runtime();
    resolve_runtime_calls();
    write_pe("raw_rt.exe");
    printf("wrote raw_rt.exe  (%d bytes code, %d bytes data, no libc)\n", codelen, datalen);
    return 0;
}
#endif
