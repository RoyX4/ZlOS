/* lexer.h - the shared vocabulary between the lexer and the parser.
 *
 * The lexer PRODUCES tokens. The parser CONSUMES them. Both sides
 * need to agree on what a token looks like, so the definition lives
 * here, in one place, instead of being copied into both files.
 */
#ifndef LEXER_H
#define LEXER_H

#define MAX_TEXT 128

typedef enum {
    T_EOF,      /* end of the file                       */
    T_NEWLINE,  /* end of a statement (we have no ';')   */
    T_NUMBER,   /* 5    999    3.14                      */
    T_STRING,   /* "hello"                               */
    T_FSTRING,  /* f"hi {name}"  - the parser splices it */
    T_IDENT,    /* x    name    print    dir             */
    T_KEYWORD,  /* if   else    for      fn     return   */
    T_SYMBOL    /* {  }  (  )  ==  >=  +  .  !  ...      */
} TokenType;

typedef struct {
    TokenType type;
    char      text[MAX_TEXT];
    int       line;          /* for error messages */
} Token;

/* Human-readable name for a token type (for printing). */
const char *type_name(TokenType t);

/* Read an entire file into a malloc'd, NUL-terminated string. */
char *read_whole_file(const char *path);

/* Tokenize a whole file. Returns a malloc'd array of tokens ending
 * with a single T_EOF token. *out_count receives how many there are
 * (including the EOF). */
Token *lex_file(const char *path, int *out_count);

/* The same, but for text already in memory rather than on disk. The
 * parser uses this to lex the little snippets inside an f-string's
 * braces. Caller frees the returned array; 'src' is not touched. */
Token *lex_text(const char *src, int *out_count);

#endif /* LEXER_H */
