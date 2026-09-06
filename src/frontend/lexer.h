/* lexer.h - the shared vocabulary between the lexer and the parser.
 *
 * The lexer PRODUCES tokens. The parser CONSUMES them. Both sides
 * need to agree on what a token looks like, so the definition lives
 * here, in one place, instead of being copied into both files.
 */
#ifndef LEXER_H
#define LEXER_H

#define MAX_TEXT 128

/* ---- THE FRONTEND TRAP (2026-09-04, F-1) ---------------------------------
 * die()/parse_error()/make_token()/string_put() used to be a bare exit(1),
 * which is correct for a hosted zl/gcc invocation and fatal for zlOS: exit is
 * #defined to k_exit() there (freestanding/zl_freestanding.h), which
 * kfatal()s and spins forever - so a script with one typo halted the whole
 * machine, because kernel/src/core/exec.c's decline path for a NULL program
 * was correct but unreachable.
 *
 * zl_frontend_fail() is the shared failure exit every one of those call
 * sites now goes through (see lexer.c for its definition). It lives in the
 * frontend, not in interp.c, because interp.c is not linked into
 * compile/compilel/nativegen at all - the frontend is the one thing common
 * to every binary that can hit a syntax error. Declared here so both
 * lexer.c and parser.c can call it, and so interp.c's zl_parse_guarded /
 * zl_lex_guarded can arm the trap it checks.
 *
 * Same shape as interp.c's runtime_error() trap: armed, a failure longjmps
 * to whoever armed it; unarmed (every hosted compile/compilel/nativegen/
 * parser_demo/lexer_demo invocation, and any use of the frontend outside
 * zl_parse_guarded/zl_lex_guarded), it is exit(1) exactly as it always was.
 */
#ifndef ZL_FREESTANDING
#include <setjmp.h>
typedef jmp_buf zf_jmp_buf;
#define zf_setjmp  setjmp
#define zf_longjmp longjmp
#else
/* zl_freestanding.h is included before this header in every freestanding
 * translation unit and already supplies zi_jmp_buf/zi_setjmp/zi_longjmp -
 * reuse them so there is exactly one setjmp shape in the kernel build. */
#define zf_jmp_buf  zi_jmp_buf
#define zf_setjmp   zi_setjmp
#define zf_longjmp  zi_longjmp
#endif

extern zf_jmp_buf zf_trap;
extern int        zf_trap_armed;

/* Print the message first (fprintf(stderr, ...) - already #defined to
 * k_printf under ZL_FREESTANDING, so it reaches the terminal there too),
 * THEN call this. */
void zl_frontend_fail(void);

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

/* ---- NUMBER TOKEN CANONICALISATION (F-2/F-3, 2026-09-04) -----------------
 * A NUMBER token's text is decimal digits, straight off the page (the lexer
 * only rewrites hex at lex time - see lex_number's comment). Every backend
 * used to decide for itself what that text MEANS:
 *
 *   - interp.c already used atof() - correct, because zl numbers are always
 *     doubles.
 *   - compile.c pasted the text verbatim into generated C: zl_num(010) is
 *     octal 8 in C, not 10, and zl_num(09) is a C compile error (09 is not a
 *     valid octal digit sequence). A double literal wider than a double can
 *     hold, like 18446744073709551616, is even worse: C parses that as an
 *     INTEGER constant first and only converts it to double afterwards, and
 *     an integer constant with no type that can hold it is undefined -
 *     gcc's observed behaviour is to truncate it mod 2^64 before the
 *     conversion, silently landing on the wrong double.
 *   - compilel.c and nativegen.c used atoll(), which SATURATES on overflow
 *     (returns LLONG_MAX/MIN) with no diagnostic at all.
 *
 * zl_num_canon() is the one parse every backend now agrees to: strtod(), the
 * same function interp.c always trusted, so "what does this literal mean"
 * has exactly one answer across all five engines. zl_num_exact_i64() is for
 * compilel/nativegen specifically: they are an INTEGER subset by design (see
 * their own T_INT/T_NUM split), so a literal that is not exactly an int64 -
 * not "close to", exactly - is refused rather than silently saturated. */
double zl_num_canon(const char *text);
int    zl_num_exact_i64(const char *text, long long *out);

#endif /* LEXER_H */
