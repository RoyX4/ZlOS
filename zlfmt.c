/* zlfmt.c - the zl re-indenter.
 *
 * This is Option C from docs/design/design_tooling.md §3, and it is
 * deliberately NOT an AST pretty-printer. That version is a trap here:
 *
 *   - lexer.c:272-273 throws comments away inside skip_whitespace, so they
 *     never become tokens. A formatter that rebuilds source from the tree
 *     would emit examples/life.zl with its entire 25-line explanation gone.
 *   - lexer.c:88 silently truncates token text at MAX_TEXT (128). A tool
 *     whose job is "rewrite this file from the token stream" turns that
 *     latent truncation into silent data loss the day someone writes a
 *     200-character string literal.
 *
 * So: never reconstruct text. Only rewrite leading whitespace. Every byte of
 * a line after its indent is memcpy'd from the original buffer. The lexer is
 * used ONLY to classify braces - so a '{' inside a string or a comment does
 * not move the indent - and never to regenerate anything. Comments and long
 * literals are then safe by construction rather than by care.
 *
 * Why line-granular rewriting is sound in zl specifically: zl has no
 * multi-line string literals (lexer.c:160-165 hard-errors on a newline
 * inside a string) AND no multi-line expressions - the parser treats
 * T_NEWLINE as a statement terminator, so
 *
 *     x = add(1,
 *             2)
 *
 * is already a parse error ("expected a value (got '\n')"). Every physical
 * line is therefore a complete lexical AND syntactic unit, which is what
 * makes per-line indentation complete rather than approximate. In Python or
 * C this design would not work.
 *
 * Build:  gcc -O2 -Wall -DBUILD_PARSER -o zlfmt zlfmt.c lexer.c -lm
 *         (BUILD_PARSER switches off the demo main() in lexer.c)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "lexer.h"

#define DEFAULT_INDENT 4

/* --- a growable output buffer ---------------------------------- */

typedef struct {
    char  *data;
    size_t len;
    size_t cap;
} Buf;

static void buf_reserve(Buf *b, size_t extra)
{
    if (b->data != NULL && b->len + extra + 1 <= b->cap) return;

    size_t want = b->cap ? b->cap : 4096;
    while (want < b->len + extra + 1) want *= 2;

    char *grown = realloc(b->data, want);
    if (grown == NULL) {
        fprintf(stderr, "zlfmt: out of memory\n");
        exit(2);
    }
    b->data = grown;
    b->cap  = want;
}

static void buf_add(Buf *b, const char *s, size_t n)
{
    buf_reserve(b, n);
    memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
}

static void buf_spaces(Buf *b, int n)
{
    if (n <= 0) return;
    buf_reserve(b, (size_t)n);
    memset(b->data + b->len, ' ', (size_t)n);
    b->len += (size_t)n;
    b->data[b->len] = '\0';
}

/* --- input ------------------------------------------------------ */

/* read_whole_file() in lexer.c fseek()s to find the size, which does not
 * work on a pipe. The editor integration formats the UNSAVED buffer, so
 * reading stdin is not optional - VS Code's copy of the document is the
 * one that must be formatted, not whatever is still on disk. */
static char *read_stream(FILE *f)
{
    Buf b = {0};
    char chunk[8192];
    size_t n;

    while ((n = fread(chunk, 1, sizeof chunk, f)) > 0) buf_add(&b, chunk, n);

    if (b.data == NULL) {           /* empty input: still return "" */
        buf_reserve(&b, 1);
        b.data[0] = '\0';
    }
    return b.data;
}

/* --- the re-indenter -------------------------------------------- */

static int count_lines(const char *src)
{
    int n = 1;
    for (const char *p = src; *p != '\0'; p++)
        if (*p == '\n') n++;
    return n;
}

static int is_hspace(char c) { return c == ' ' || c == '\t' || c == '\r'; }

/* Returns a malloc'd formatted copy of src. Never returns NULL: on a lex
 * error the lexer exits(1) before we have opened, truncated, or written
 * anything, so a file that does not lex is left exactly as it was. */
static char *reindent(const char *src, int indent_width)
{
    int    ntok = 0;
    Token *toks = lex_text(src, &ntok);       /* exits(1) on a lex error */

    int nlines = count_lines(src);

    /* Per physical line (1-based), counted from tokens only. A '{' inside
     * "a { b" or # a { b never reaches us: strings are one T_STRING token,
     * f-strings are one T_FSTRING token (so f"{x}" cannot corrupt the
     * depth), and comments produce no tokens at all. */
    int  *opens = calloc((size_t)nlines + 2, sizeof(int));
    int  *closes = calloc((size_t)nlines + 2, sizeof(int));
    int  *lead   = calloc((size_t)nlines + 2, sizeof(int));  /* '}' before any other token */
    char *other  = calloc((size_t)nlines + 2, 1);            /* seen a non-'}' token yet */

    if (!opens || !closes || !lead || !other) {
        fprintf(stderr, "zlfmt: out of memory\n");
        exit(2);
    }

    for (int i = 0; i < ntok; i++) {
        const Token *t = &toks[i];
        if (t->type == T_EOF || t->type == T_NEWLINE) continue;

        int L = t->line;
        if (L < 1 || L > nlines) continue;                   /* defensive */

        if (t->type == T_SYMBOL && strcmp(t->text, "{") == 0) {
            opens[L]++;
            other[L] = 1;
        } else if (t->type == T_SYMBOL && strcmp(t->text, "}") == 0) {
            closes[L]++;
            if (!other[L]) lead[L]++;   /* deliberately does NOT set other[] */
        } else {                        /* so "}}" counts both as leading   */
            other[L] = 1;
        }
    }

    free(toks);

    Buf out = {0};
    buf_reserve(&out, strlen(src) + 64);

    int         depth = 0;
    int         L     = 1;
    const char *p     = src;

    while (*p != '\0') {
        const char *eol      = strchr(p, '\n');
        const char *line_end = eol ? eol : p + strlen(p);

        const char *s = p;                  /* first non-blank byte  */
        while (s < line_end && is_hspace(*s)) s++;
        const char *e = line_end;           /* one past the last one */
        while (e > s && is_hspace(e[-1])) e--;

        if (e > s) {
            /* A leading '}' closes the block this line is being placed in,
             * so it must dedent BEFORE the line is emitted. The remaining
             * closers on the line take effect after it.
             *
             * NB: design_tooling.md §3.2's pseudocode applies the full close
             * count in the trailing update, which double-counts the leading
             * ones - a bare "}" would drive depth to -1. The leading closers
             * are subtracted out here instead. */
            depth -= lead[L];
            if (depth < 0) depth = 0;

            buf_spaces(&out, depth * indent_width);
            buf_add(&out, s, (size_t)(e - s));

            depth += opens[L] - (closes[L] - lead[L]);
            if (depth < 0) depth = 0;
        }
        /* A blank line emits nothing at all - not even its old trailing
         * spaces. Stripping trailing whitespace is safe for the same reason
         * the rest of this is: no string can reach the end of a line without
         * its closing quote coming first. */

        if (eol == NULL) break;             /* no trailing newline: keep it that way */

        buf_add(&out, "\n", 1);
        p = eol + 1;
        L++;
    }

    free(opens);
    free(closes);
    free(lead);
    free(other);

    if (out.data == NULL) {                 /* empty input */
        out.data = calloc(1, 1);
        if (out.data == NULL) { fprintf(stderr, "zlfmt: out of memory\n"); exit(2); }
    }
    return out.data;
}

/* --- output ----------------------------------------------------- */

/* Write via a temp file in the same directory and rename() over the top, so
 * a crash or a full disk cannot leave a half-written source file behind.
 * rename() within one directory is atomic. */
static int write_atomically(const char *path, const char *text)
{
    size_t plen = strlen(path);
    char  *tmp  = malloc(plen + 16);
    if (tmp == NULL) { fprintf(stderr, "zlfmt: out of memory\n"); exit(2); }
    snprintf(tmp, plen + 16, "%s.zlfmt.tmp", path);

    FILE *f = fopen(tmp, "wb");
    if (f == NULL) {
        fprintf(stderr, "zlfmt: can't create '%s'\n", tmp);
        free(tmp);
        return 0;
    }

    size_t n = strlen(text);
    if (n > 0 && fwrite(text, 1, n, f) != n) {
        fprintf(stderr, "zlfmt: write failed for '%s'\n", tmp);
        fclose(f);
        remove(tmp);
        free(tmp);
        return 0;
    }
    if (fclose(f) != 0) {
        fprintf(stderr, "zlfmt: write failed for '%s'\n", tmp);
        remove(tmp);
        free(tmp);
        return 0;
    }

    struct stat st;                          /* keep the original mode */
    if (stat(path, &st) == 0) chmod(tmp, st.st_mode & 07777);

    if (rename(tmp, path) != 0) {
        fprintf(stderr, "zlfmt: can't replace '%s'\n", path);
        remove(tmp);
        free(tmp);
        return 0;
    }

    free(tmp);
    return 1;
}

/* --- driver ----------------------------------------------------- */

static void usage(void)
{
    fprintf(stderr,
        "usage: zlfmt [options] <file.zl>...\n"
        "       zlfmt [options] -            # read stdin, write stdout\n"
        "\n"
        "Re-indents zl source. Rewrites leading whitespace and strips trailing\n"
        "whitespace; every other byte is copied through untouched.\n"
        "\n"
        "options:\n"
        "  -w, --write        rewrite each file in place (only if it changes)\n"
        "  -c, --check        write nothing; list files that need formatting\n"
        "                     and exit 1 if there are any\n"
        "  -i, --indent N     spaces per level (default %d)\n"
        "  -h, --help         this text\n"
        "\n"
        "exit: 0 ok   1 --check found unformatted files   2 bad usage\n",
        DEFAULT_INDENT);
}

int main(int argc, char **argv)
{
    int write_mode  = 0;
    int check_mode  = 0;
    int indent      = DEFAULT_INDENT;
    int first_file  = argc;

    int i = 1;
    for (; i < argc; i++) {
        const char *a = argv[i];

        if (strcmp(a, "-w") == 0 || strcmp(a, "--write") == 0) {
            write_mode = 1;
        } else if (strcmp(a, "-c") == 0 || strcmp(a, "--check") == 0) {
            check_mode = 1;
        } else if (strcmp(a, "-i") == 0 || strcmp(a, "--indent") == 0) {
            if (i + 1 >= argc) { fprintf(stderr, "zlfmt: --indent needs a number\n"); return 2; }
            indent = atoi(argv[++i]);
            if (indent < 1 || indent > 16) {
                fprintf(stderr, "zlfmt: --indent must be 1..16\n");
                return 2;
            }
        } else if (strcmp(a, "-h") == 0 || strcmp(a, "--help") == 0) {
            usage();
            return 0;
        } else if (a[0] == '-' && a[1] != '\0' && strcmp(a, "-") != 0) {
            fprintf(stderr, "zlfmt: unknown option '%s'\n", a);
            return 2;
        } else {
            break;                                   /* first filename */
        }
    }
    first_file = i;

    if (first_file >= argc) { usage(); return 2; }

    if (write_mode && check_mode) {
        fprintf(stderr, "zlfmt: --write and --check are mutually exclusive\n");
        return 2;
    }

    int needs_format = 0;

    for (i = first_file; i < argc; i++) {
        const char *path   = argv[i];
        int         is_stdin = (strcmp(path, "-") == 0);

        if (is_stdin && (write_mode || check_mode)) {
            fprintf(stderr, "zlfmt: '-' cannot be used with --write or --check\n");
            return 2;
        }

        char *src = is_stdin ? read_stream(stdin) : read_whole_file(path);
        char *dst = reindent(src, indent);           /* exits(1) if it does not lex */

        int changed = (strcmp(src, dst) != 0);

        if (check_mode) {
            if (changed) {
                printf("%s\n", path);
                needs_format = 1;
            }
        } else if (write_mode) {
            if (changed && !write_atomically(path, dst)) {
                free(src);
                free(dst);
                return 2;
            }
        } else {
            fputs(dst, stdout);
        }

        free(src);
        free(dst);
    }

    return needs_format ? 1 : 0;
}
