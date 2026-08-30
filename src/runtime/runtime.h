/* runtime.h - the runtime library for COMPILED programs.
 *
 * When my compiler turns a .zl file into C, the generated C code
 * doesn't re-implement "what is a value" or "how does + work" -
 * it calls these helpers. This is the small library every compiled
 * program links against. It's basically the interpreter's brain,
 * repackaged so generated code can call it.
 */
#ifndef RUNTIME_H
#define RUNTIME_H

typedef enum { V_NIL, V_NUM, V_STR, V_BOOL, V_LIST, V_FN } ValueType;

/* ===========================================================================
 * THE ITEMS ARRAY CARRIES ITS OWN HEADER.
 *
 * `cap` and `tip` used to be fields of Value. They are not properties of a
 * VALUE, they are properties of the ARRAY - and push() depends on exactly
 * that. Its tip tracking splits a list's state in two:
 *
 *     nitems              PER VALUE   how much of the array this value sees
 *     items, cap, tip     SHARED      the array, and its high-water mark
 *
 * Two values that alias one array must agree about cap and tip, or a push
 * branched off an older version appends into a slot the newer one is already
 * using. That was a real use-after-free here once; runtime.c's comment above
 * push() has the history.
 *
 * Keeping cap and tip inside Value made the sharing a convention that every
 * copy had to preserve by hand. Putting them in front of the array makes it
 * STRUCTURAL - there is one copy, at a fixed offset before items[0], and
 * every value pointing at that array reaches the same one. It also drops the
 * separate malloc that `int *tip` needed.
 *
 * SAFE ONLY BECAUSE THE ARRAY NEVER MOVES UNDER AN ALIAS. push() never
 * reallocs: its fast path appends inside existing capacity, and its slow path
 * allocates a FRESH array and copies. The one realloc in the runtime
 * (list_push_str) runs on a list still under construction, before any other
 * value can point at it.
 * ========================================================================= */
typedef struct { int cap; int tip; } ZlArrHdr;

/* 8 bytes, so items[0] keeps the pointer alignment malloc already gave us -
 * on 32-bit (the kernel builds -m32) and 64-bit alike. */
#define ZL_ARR_HDR   ((int)sizeof(ZlArrHdr))
#define zl_arr_hdr(items)  ((ZlArrHdr *)((char *)(items) - ZL_ARR_HDR))

/* SIXTEEN BYTES, and the two unions are why.
 *
 * `type` stays first - zl_nil() memsets the whole struct and several places
 * switch on type before touching anything else.
 *
 * The unions are ANONYMOUS on purpose: every existing `v.num`, `v.str`,
 * `v.items`, `v.fnptr`, `v.nitems` and `v.fnargs` in the runtime keeps
 * compiling unchanged, so shrinking the struct did not become a rename of
 * several thousand field accesses. The pairs are disjoint by type, so no
 * value ever needs two members of one union at once:
 *
 *     nitems  V_LIST      fnargs  V_FN
 *     num     V_NUM/BOOL  str     V_STR   items  V_LIST   fnptr  V_FN
 *
 * THE COST OF THAT CONVENIENCE: reading a member that is not the active one
 * is now garbage rather than a zero. Every read must be guarded by `type`.
 * The runtime already switches on type first everywhere; that is no longer a
 * style preference. */
typedef struct Value {
    ValueType type;
    union {
        int    nitems;       /* V_LIST                                   */
        int    fnargs;       /* V_FN - how many Value params it takes    */
    };
    union {
        double         num;   /* V_NUM, and V_BOOL (0/1)                 */
        char          *str;   /* V_STR                                   */
        struct Value **items; /* V_LIST - see ZlArrHdr above             */
        void          *fnptr; /* V_FN - the compiled zl_fn_NAME function */
    };
} Value;

/* making values */
Value zl_nil(void);
Value zl_num(double n);
Value zl_str(const char *s);
Value zl_bool(int b);
Value zl_list_n(int count, ...);          /* zl_list_n(3, a, b, c) */

/* using values */
int   zl_truthy(Value v);
Value zl_binop(const char *op, Value a, Value b);
Value zl_unop(const char *op, Value a);
Value zl_index(Value seq, Value idx);
void  zl_set(Value list, Value idx, Value val);   /* x[i] = v (in place) */
int   zl_len_list(Value v);               /* how many items in a list */
Value zl_item(Value v, int i);            /* the i-th item of a list  */

/* built-in functions: zl_calln("print", 2, a, b) */
Value zl_calln(const char *name, int n, ...);

/* first-class functions. A user function used as a VALUE (passed to another
 * function, stored in a variable) becomes a V_FN carrying its compiled
 * address; calling a variable that holds one goes through zl_callv, which
 * casts the pointer to the arity the call site actually uses. The
 * interpreter has had this since the start (its V_FN points at the AST
 * node); this is the compiled equivalent. */
Value zl_fn(void *fnptr, int nargs);
Value zl_callv(Value f, int n, ...);

#endif /* RUNTIME_H */
