/* lexer.c - stage 1 of the compiler.
 *
 * Turns raw source text into a flat list of tokens.
 *
 *   print("hi " + name)
 *
 * becomes
 *
 *   IDENT 'print'  SYMBOL '('  STRING 'hi '  SYMBOL '+'
 *   IDENT 'name'   SYMBOL ')'  NEWLINE  EOF
 *
 * The lexer does NOT care whether the program makes sense.
 * Its only job is chopping. The parser judges.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lexer.h"        /* Token, TokenType and MAX_TEXT now live here */

typedef struct {
    const char *src;
    int         pos;
    int         line;
} Lexer;

/* ---------------------------------------------------------------
 * WHICH WORDS ARE RESERVED
 *
 * A keyword is a word the language steals from you. If "if" is a
 * keyword, nobody can ever write  if = 5  as a variable.
 * Every word added here is a name your users lose forever.
 *
 * DECIDED:
 *   built-ins (print, kill, dir, poke, window) are NOT keywords.
 *   They are ordinary identifiers, so adding 200 PC-control
 *   built-ins later costs zero reserved words.
 *
 *   '!' means ONLY "i mean it, this is dangerous".
 *   Negation is the word 'not', so '!' stays rare enough to
 *   still look like a warning.  ('!=' is untouched - it is one
 *   atomic symbol, not a use of '!'.)
 * --------------------------------------------------------------- */
static int is_keyword(const char *word)
{
    static const char *keywords[] = {
        "if", "else", "elif", "for", "in", "fn", "return", "while",
        "not", "and", "or",          /* word operators, not && || */
        "true", "false",
        "break", "continue",         /* loop control */
        "import",                    /* keyword #16 - see design_imports.md */
        NULL
    };

    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* --- tiny helpers ---------------------------------------------- */

static char peek(Lexer *lx)
{
    return lx->src[lx->pos];
}

static char peek_next(Lexer *lx)
{
    if (lx->src[lx->pos] == '\0') return '\0';
    return lx->src[lx->pos + 1];
}

static char advance(Lexer *lx)
{
    return lx->src[lx->pos++];
}

static Token make_token(TokenType type, const char *start, int len, int line)
{
    Token t;
    t.type = type;
    t.line = line;

    if (len >= MAX_TEXT) len = MAX_TEXT - 1;   /* never overflow */
    memcpy(t.text, start, (size_t)len);
    t.text[len] = '\0';
    return t;
}

static void die(int line, const char *msg, char c)
{
    fprintf(stderr, "line %d: %s '%c'\n", line, msg, c);
    exit(1);
}

/* --- the four things we know how to chop ----------------------- */

/* 5   42   3.14 */
static Token lex_number(Lexer *lx)
{
    int start = lx->pos;
    int line  = lx->line;

    while (isdigit((unsigned char)peek(lx))) advance(lx);

    if (peek(lx) == '.' && isdigit((unsigned char)peek_next(lx))) {
        advance(lx);                                  /* eat the dot */
        while (isdigit((unsigned char)peek(lx))) advance(lx);
    }

    return make_token(T_NUMBER, lx->src + start, lx->pos - start, line);
}

/* value of one hex digit, or -1 if it isn't one */
static int hex_val(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/* "hello"  -> the token text is  hello  (quotes stripped)
 *
 * Escape sequences are translated here: \n becomes a real newline,
 * \t a tab, etc. We can't just copy the raw source span for this,
 * because the translated text is shorter than what's on the page,
 * so we build the token text character by character.
 *
 * Known escapes:  \n \t \r \\ \"  and  \xNN  (exactly two hex digits).
 *
 * ANYTHING ELSE IS LEFT EXACTLY AS WRITTEN - the backslash is kept.
 * That matters: a lexer that quietly swallows the backslash of an
 * escape it doesn't recognise changes the meaning of text nobody
 * meant to escape. Keeping it verbatim means the only strings whose
 * meaning ever changes are the ones using an escape we do define.
 *
 * NUL IS REFUSED. Every string in this toolchain is a NUL-terminated C
 * string, so a zl string cannot hold a NUL byte - and `\0` used to be
 * ACCEPTED, which meant "a\0b" silently became "a" and the rest of the
 * literal was thrown away with no diagnostic. A representation that
 * cannot carry the character should say so, so `\0` and `\x00` are now
 * lex errors. (chr(0) refuses for the same reason.) Making them work
 * instead would mean giving zl length-carrying strings - a change to
 * every string built-in and every backend, not to this decoder. */
static Token lex_string(Lexer *lx)
{
    int  line = lx->line;
    advance(lx);                                      /* eat opening " */

    Token t;
    t.type = T_STRING;
    t.line = line;
    int out = 0;

    while (peek(lx) != '"') {
        char c = peek(lx);
        if (c == '\0' || c == '\n') {
            fprintf(stderr, "line %d: string never closed\n", line);
            exit(1);
        }

        if (c != '\\') {
            if (out < MAX_TEXT - 1) t.text[out++] = c;
            advance(lx);
            continue;
        }

        /* An escape. src is NUL-terminated, so src[pos+1] is always a
           safe read; the deeper reads below are each guarded by having
           already seen a non-NUL character before them. */
        char e     = lx->src[lx->pos + 1];
        int  real  = -1;              /* -1 means "not an escape I know" */
        int  eaten = 2;               /* source characters the escape uses */

        switch (e) {
            case 'n':  real = '\n'; break;
            case 't':  real = '\t'; break;
            case 'r':  real = '\r'; break;
            case '0':  real = '\0'; break;
            case '\\': real = '\\'; break;
            case '"':  real = '"';  break;
            case 'x': {
                int hi = hex_val(lx->src[lx->pos + 2]);
                int lo = (hi < 0) ? -1 : hex_val(lx->src[lx->pos + 3]);
                if (lo >= 0) {                        /* \xNN */
                    real  = hi * 16 + lo;
                    eaten = 4;
                }
                break;                /* \x with bad digits: left literal */
            }
            default: break;                           /* unknown: literal */
        }

        if (real < 0) {
            if (out < MAX_TEXT - 1) t.text[out++] = '\\';
            advance(lx);                              /* only the backslash */
            continue;
        }

        if (real == 0) {
            fprintf(stderr, "line %d: a string cannot contain a NUL byte "
                            "(\\0 or \\x00) - zl strings are NUL-terminated\n", line);
            exit(1);
        }

        if (out < MAX_TEXT - 1) t.text[out++] = (char)real;
        lx->pos += eaten;
    }

    t.text[out] = '\0';
    advance(lx);                                      /* eat closing " */
    return t;
}

/* name   print   my_var   x2 */
static Token lex_word(Lexer *lx)
{
    int start = lx->pos;
    int line  = lx->line;

    while (isalnum((unsigned char)peek(lx)) || peek(lx) == '_') advance(lx);

    Token t = make_token(T_IDENT, lx->src + start, lx->pos - start, line);
    if (is_keyword(t.text)) t.type = T_KEYWORD;
    return t;
}

/* {  }  ==  >=  !  .  + ... */
static Token lex_symbol(Lexer *lx)
{
    int  start = lx->pos;
    int  line  = lx->line;
    char c     = advance(lx);

    /* two-character symbols must be checked BEFORE one-character ones,
       or "==" would come out as two separate "=" tokens. */
    if ((c == '=' || c == '!' || c == '<' || c == '>') && peek(lx) == '=') {
        advance(lx);
        return make_token(T_SYMBOL, lx->src + start, 2, line);
    }
    /* compound assignment: += -= *= /= %= (two-char, checked before one) */
    if ((c == '+' || c == '-' || c == '*' || c == '/' || c == '%') && peek(lx) == '=') {
        advance(lx);
        return make_token(T_SYMBOL, lx->src + start, 2, line);
    }

    /* '?' and ':' are only ever the two halves of a ternary. Neither
       starts a two-character symbol, so there is nothing above to
       shadow: adding them here can only turn text that used to be a
       lexer ERROR into text that lexes. */
    if (strchr("(){}[],.+-*/%=!<>?:", c) == NULL) {
        die(line, "I don't know this character:", c);
    }

    return make_token(T_SYMBOL, lx->src + start, 1, line);
}

/* --- the main loop --------------------------------------------- */

static Token next_token(Lexer *lx)
{
    /* skip spaces and tabs, but NOT newlines - those are real tokens */
    for (;;) {
        char c = peek(lx);
        if (c == ' ' || c == '\t' || c == '\r') {
            advance(lx);
        } else if (c == '#') {                        /* comment to end of line */
            while (peek(lx) != '\n' && peek(lx) != '\0') advance(lx);
        } else {
            break;
        }
    }

    char c = peek(lx);

    if (c == '\0') return make_token(T_EOF, "", 0, lx->line);

    if (c == '\n') {
        int line = lx->line;
        advance(lx);
        lx->line++;
        return make_token(T_NEWLINE, "\\n", 2, line);
    }

    if (isdigit((unsigned char)c))            return lex_number(lx);
    if (c == '"')                             return lex_string(lx);

    /* f"..." - an INTERPOLATED string. The prefix is opt-in on purpose.
     * Plain "..." keeps its old meaning exactly, which it has to:
     * compiler.zl emits C source, so its literals are full of braces
     * (") {\n" and friends). Making every string interpolate would
     * change the meaning of text that already parses.
     *
     * This only fires where a token STARTS, so the 'f' inside the
     * string "f" is untouched - that f is eaten by lex_string, and
     * never reaches this line. The only text whose meaning changes is
     * a bare identifier 'f' glued directly to an opening quote. */
    if (c == 'f' && peek_next(lx) == '"') {
        advance(lx);                               /* eat the 'f' */
        Token t = lex_string(lx);
        t.type = T_FSTRING;
        return t;
    }

    if (isalpha((unsigned char)c) || c == '_') return lex_word(lx);

    return lex_symbol(lx);
}

/* --- printing so we can see it work ---------------------------- */

const char *type_name(TokenType t)
{
    switch (t) {
        case T_EOF:     return "EOF    ";
        case T_NEWLINE: return "NEWLINE";
        case T_NUMBER:  return "NUMBER ";
        case T_STRING:  return "STRING ";
        case T_FSTRING: return "FSTRING";
        case T_IDENT:   return "IDENT  ";
        case T_KEYWORD: return "KEYWORD";
        case T_SYMBOL:  return "SYMBOL ";
    }
    return "???";
}

char *read_whole_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        fprintf(stderr, "can't open '%s'\n", path);
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *buf = malloc((size_t)size + 1);
    if (buf == NULL) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }

    size_t got = fread(buf, 1, (size_t)size, f);
    buf[got] = '\0';
    fclose(f);
    return buf;
}

/* ---------------------------------------------------------------
 * lex_file - tokenize an entire file into a malloc'd array.
 *
 * This is what the PARSER calls. Instead of printing tokens one at a
 * time (like the demo main below), it collects every token into an
 * array the parser can walk back and forth over.
 * --------------------------------------------------------------- */
Token *lex_file(const char *path, int *out_count)
{
    char  *source = read_whole_file(path);
    Token *tokens = lex_text(source, out_count);
    free(source);
    return tokens;
}

/* lex_text - the same chopping, on text already in memory.
 *
 * lex_file used to hold this loop directly. It was split out so the
 * parser can lex the snippet inside an f-string's braces without
 * writing it to a temporary file first. */
Token *lex_text(const char *src, int *out_count)
{
    Lexer lx;
    lx.src  = src;
    lx.pos  = 0;
    lx.line = 1;

    int    cap    = 64;
    int    count  = 0;
    Token *tokens = malloc((size_t)cap * sizeof(Token));
    if (tokens == NULL) { fprintf(stderr, "out of memory\n"); exit(1); }

    for (;;) {
        if (count == cap) {
            cap *= 2;
            tokens = realloc(tokens, (size_t)cap * sizeof(Token));
            if (tokens == NULL) { fprintf(stderr, "out of memory\n"); exit(1); }
        }

        Token t = next_token(&lx);
        tokens[count++] = t;
        if (t.type == T_EOF) break;
    }

    *out_count = count;
    return tokens;
}

/* ---------------------------------------------------------------
 * The standalone lexer demo.
 *
 * When we build the PARSER, we compile lexer.c together with
 * parser.c - but parser.c has its own main(). Two main()s won't
 * link. So the parser's build defines BUILD_PARSER, which switches
 * this demo main OFF and leaves only lex_file() for the parser.
 * --------------------------------------------------------------- */
#ifndef BUILD_PARSER
int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: lexer <file>\n");
        return 1;
    }

    int    count;
    Token *tokens = lex_file(argv[1], &count);

    for (int i = 0; i < count; i++) {
        printf("line %2d | %s | %s\n",
               tokens[i].line, type_name(tokens[i].type), tokens[i].text);
    }

    free(tokens);
    return 0;
}
#endif
