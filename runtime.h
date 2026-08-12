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

typedef struct Value {
    ValueType      type;
    double         num;      /* V_NUM, and V_BOOL (0/1) */
    char          *str;      /* V_STR                   */
    struct Value **items;    /* V_LIST                  */
    int            nitems;
    int            cap;      /* V_LIST spare capacity (amortized push)  */
    int           *tip;      /* V_LIST slots handed out - see zl push() */
    void          *fnptr;    /* V_FN - the compiled zl_fn_NAME function  */
    int            fnargs;   /* V_FN - how many Value params it takes    */
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
