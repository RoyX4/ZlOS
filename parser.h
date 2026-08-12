/* parser.h - the shared tree definition.
 *
 * The parser BUILDS this tree. The interpreter WALKS it. Both need
 * to agree on what a node looks like, so it lives here.
 */
#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    N_NUMBER, N_STRING, N_IDENT, N_BOOL, N_LIST,
    N_BINARY,    /* left  OP  right                     */
    N_UNARY,     /* OP operand  (e.g. -x, not x)        */
    N_TERNARY,   /* cond ? then : else  (an EXPRESSION) */
    N_DANGER,    /* operand!    (the "i mean it" mark)  */
    N_CALL,      /* callee(args...)                     */
    N_MEMBER,    /* object.field                        */
    N_INDEX,     /* object[index]                       */
    N_ASSIGN,    /* target = value                      */
    N_IF,        /* if cond {then} else {else}          */
    N_FOR,       /* for var in iterable {body}          */
    N_WHILE,     /* while cond {body}                   */
    N_FN,        /* fn name(params) {body}              */
    N_RETURN,    /* return value                        */
    N_BREAK,     /* break out of a loop                 */
    N_CONTINUE,  /* skip to the next loop iteration     */
    N_EXPRSTMT,  /* an expression used as a statement   */
    N_BLOCK,     /* { statements... }                   */
    N_PROGRAM    /* the whole file                      */
} NodeType;

typedef struct Node {
    NodeType type;
    char     text[MAX_TEXT];   /* operator, name, or literal value  */

    struct Node *a;            /* generic child slots...            */
    struct Node *b;
    struct Node *c;

    struct Node **kids;        /* for lists: args, params, stmts    */
    int          nkids;
} Node;

/* Parse a token array into a tree. Entry point for the parser. */
Node *parse(Token *tokens, int ntokens);

/* Pretty-print a tree (used by parser.exe's demo). */
void print_node(Node *n, int depth);

#endif /* PARSER_H */
