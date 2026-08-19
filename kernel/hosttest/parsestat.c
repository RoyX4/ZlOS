/* parsestat.c - how much of a real page actually fits, in numbers.
 *
 * WHY THIS EXISTS AS A COMMITTED FILE. Every claim in browser-render-run.md
 * §11 and §12 - "nodes 8192/8192", "text arena 196,607/196,608", "235 rules
 * taken, css_overflowed() == 1", "375 at MAX_SELS 4096" - came from a
 * throwaway program that was not kept. The next person to raise a cap had two
 * choices: trust a number in a document, or rebuild the measurement. The
 * document itself says "rebuild it rather than trusting the numbers", which is
 * the right instruction and a bad position to put someone in.
 *
 * So it is a file. Point it at a page and a stylesheet and it prints what each
 * of the three arrays did with them.
 *
 *     ./parsestat page.html skin.css [viewport]
 *
 * IT IS NOT A GATE AND IT ASSERTS NOTHING, deliberately. Every other harness
 * here has a right answer to compare against; this one's output depends on
 * whatever page you fed it, and a "checks, 0 failed" line would be a claim
 * about the web rather than about this code. It measures. You read it.
 *
 * The corpus is not committed either - a 1 MB HTML file and a 272 KB
 * stylesheet are somebody else's content and would be stale within a month.
 * The two used for every number in the docs are:
 *
 *     curl 'https://en.wikipedia.org/wiki/Linux' -o linux.html
 *     # then the two <link rel=stylesheet> hrefs out of it, in order:
 *     curl 'https://en.wikipedia.org/w/load.php?...&only=styles&skin=vector-2022'
 *
 * Fetch them, concatenate the sheets in document order (cascade order), run
 * this, and compare with the table in browser-render-run.md.
 *
 * THE MEASURE FUNCTION IS SYNTHETIC, the same one htmltest uses - every glyph
 * half an em wide. The run COUNT barely moves with the metrics and the run
 * count is what this is for; a real font would make the height meaningful and
 * the whole thing dependent on fb.c, which is the coupling layout.c is built
 * to avoid.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../html.h"
#include "../css.h"
#include "../layout.h"

/* The storage, as ordinary statics - see htmltest.c's host_arenas for why this
 * is BSS here and a slice of memmap.h's HI_DOM in the kernel. Sized from the
 * headers so that raising a cap and re-running this needs one edit, not two. */
static unsigned char node_mem[HTML_NODES_BYTES] __attribute__((aligned(8)));
static char          text_mem[HTML_ARENA];
static unsigned char sel_mem[CSS_SELS_BYTES]   __attribute__((aligned(8)));
static unsigned char decl_mem[CSS_DECLS_BYTES] __attribute__((aligned(8)));
static char          cssa_mem[CSS_ARENA];
static struct lay_run run_mem[LAY_MAX_RUNS];

static int fake_measure(const char *s, int len, int size, int style)
{
    (void)s;
    int w = len * size / 2;
    if (style & LS_MONO) w = len * size * 6 / 10;
    if (style & LS_BOLD) w = w * 11 / 10;
    return w;
}

static char *slurp(const char *path, int *len)
{
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); exit(2); }
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *b = malloc((size_t)n + 1);
    if (!b || fread(b, 1, (size_t)n, f) != (size_t)n) { perror("read"); exit(2); }
    b[n] = 0;
    fclose(f);
    *len = (int)n;
    return b;
}

/* percent-of-cap, printed beside every number, because "9,886 runs" says
 * nothing without "of how many" - and it was the missing half of that pair
 * that let the run array look fine while html.c was dropping 7,807 nodes. */
static void line(const char *what, long used, long cap)
{
    printf("  %-14s %9ld / %-9ld  %3ld%%%s\n", what, used, cap,
           cap > 0 ? used * 100 / cap : 0,
           (cap > 0 && used >= cap) ? "   FULL" : "");
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "usage: %s page.html sheet.css [viewport]\n", argv[0]);
        return 2;
    }
    int viewport = argc > 3 ? atoi(argv[3]) : 1036;   /* the browser's width */

    int hlen, clen;
    char *html = slurp(argv[1], &hlen);
    char *css  = slurp(argv[2], &clen);

    html_set_arena(node_mem, HTML_MAX_NODES, text_mem, HTML_ARENA);
    css_set_arena(sel_mem, CSS_MAX_SELS, decl_mem, CSS_MAX_DECLS,
                  cssa_mem, CSS_ARENA);
    lay_set_arena(run_mem, LAY_MAX_RUNS);

    /* THE WHOLE FILE, not browser.c's truncated copy of it. browser.c is not
     * linked here on purpose: this measures the three parsers against the page
     * as served, so raising DOC_MAX and raising HTML_MAX_NODES stay separate
     * questions with separate answers. Compare hlen against browser_doc_cap()
     * by hand if you want to know which one bites first. */
    printf("input\n");
    printf("  document     %9d bytes  (as served, not truncated)\n", hlen);
    printf("  stylesheet   %9d bytes\n", clen);
    printf("  viewport     %9d px\n\n", viewport);

    int n = html_parse(html, hlen);
    printf("html.c\n");
    line("nodes", n, html_node_cap());
    line("text arena", html_arena_used(), html_arena_cap());
    printf("  %-14s %9d          %s\n", "dropped", html_dropped(),
           html_dropped() ? "   nodes the array had no room for" : "");
    printf("  %-14s %9d inline, %d external\n\n", "stylesheets",
           html_sheets(), html_css_links());

    /* THE ORDER MATTERS: css.c evaluates @media at PARSE time, so the viewport
     * has to be set before the sheet goes in. Getting this backwards makes
     * every width query refuse and costs ~38% of a modern stylesheet, silently
     * - which is the behaviour that shipped before @media was evaluated. */
    css_viewport(viewport);
    int took = css_add_sheet(css, clen);
    printf("css.c\n");
    printf("  %-14s %9d\n", "rules taken", took);
    /* SELECTORS, NOT RULES. `p, a, span {}` is one rule and three selectors,
     * and it is the SELECTOR array that refuses work - so a table of "rules
     * taken" against a cap named MAX_SELS was comparing two different things.
     * That is most of why "235 rules" looked like a stylesheet limit rather
     * than an array limit for as long as it did. */
    line("selectors", css_sels(), css_sel_cap());
    line("declarations", css_decls(), css_decl_cap());
    line("string arena", css_arena_used(), css_arena_cap());
    printf("  %-14s %9d%s\n\n", "overflowed", css_overflowed(),
           css_overflowed() ? "          rules were REFUSED" : "");

    lay_set_measure(fake_measure);
    lay_set_image(0);
    int h = lay_run_doc(viewport, 16);
    printf("layout.c\n");
    line("runs", lay_count(), lay_run_cap());
    printf("  %-14s %9d%s\n", "overflowed", lay_overflowed(),
           lay_overflowed() ? "          runs were DROPPED" : "");
    printf("  %-14s %9d px in %d lines\n", "height", h, lay_lines());

    free(html);
    free(css);
    return 0;
}
