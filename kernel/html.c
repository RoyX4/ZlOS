/* html.c - an HTML tokenizer and tree builder, with no heap.
 *
 * THE TREE IS AN ARRAY AND THE EDGES ARE INDICES, not pointers - the same
 * shape as wm.c's zorder and for the same reason: there is no allocator in
 * this kernel, so a node cannot be malloc'd and a child list cannot be a list.
 * A fixed array of nodes with parent/first_child/next_sibling INDICES is a
 * tree in every way that matters, it costs one static array, and -1 is a
 * perfectly good null.
 *
 * TEXT IS COPIED INTO AN ARENA rather than pointed at in the source. It has to
 * be: entities decode to something shorter than they were written (&amp; -> &)
 * and whitespace collapses, so the text a layout wants is not a substring of
 * the document. Copying once at parse time means the layout - which runs again
 * on every resize - never has to decode anything.
 *
 * BE LIBERAL. Real HTML is malformed and a browser that refuses a page is a
 * browser nobody uses. Every error path here RECOVERS: an unclosed <p> is
 * closed by the next block, a stray </div> with no matching open is dropped, a
 * '<' that starts nothing is text. There is no way to make this fault, and
 * that is a deliberate property rather than an accident - hosttest/htmltest.c
 * asserts it against deliberately broken input.
 *
 * What it does NOT do, and these are decisions rather than omissions:
 *   - no scripting: <script> contents are DISCARDED, not rendered. <style> is
 *     no longer discarded - its text is handed to css.c as the document's own
 *     stylesheet - but it is still never RENDERED, because a stylesheet shown
 *     as body text is worse than one ignored
 *   - no character set beyond ASCII: bytes >= 0x80 become '?'
 *   - no DOCTYPE handling beyond skipping it, no namespaces, no <table>
 */

#include "html.h"

#define MAX_NODES  HTML_MAX_NODES   /* 8,239 open tags in a Wikipedia article */
#define ARENA      196608  /* ~99 KB of visible text in that same article */
#define MAX_DEPTH  32

struct node {
    short kind;          /* HN_ELEM or HN_TEXT                          */
    short tag;           /* HT_* when kind is HN_ELEM                   */
    int   parent;
    int   first;         /* first child, -1 for none                    */
    int   last;          /* last child - makes append O(1)              */
    int   next;          /* next sibling                                */
    int   toff, tlen;    /* text: where it lives in the arena           */
    int   aoff, alen;    /* an <a>'s href, or an <img>'s alt            */
    /* THE THREE ATTRIBUTES CSS NEEDS, and no others. Every other attribute
     * still hands its arena back the moment it is read - keeping them all
     * would turn the arena into the document's whole attribute surface for
     * the sake of properties nothing can act on. */
    int   coff, clen;    /* class                                       */
    int   ioff, ilen;    /* id                                          */
    int   soff, slen;    /* style=                                      */
};

static struct node nodes[MAX_NODES];
static int  nnodes;
static char arena[ARENA];
static int  used;
static int  title_off, title_len;
static int  n_dropped;               /* recoveries, for the harness to see */

/* The document's own stylesheets, as spans of the SOURCE rather than copies.
 * A page's <style> can be tens of kilobytes, and copying sheets into the node
 * arena would let a styled page starve its own text. The source buffer
 * outlives the parse - browser.c holds the document - and css.c interns the
 * few hundred bytes it actually keeps. */
#define MAX_SHEETS 32                /* a Wikipedia article carries 16 */
static struct { int off, len; } sheets[MAX_SHEETS];
static int nsheets;
static struct { int off, len; } scripts[MAX_SHEETS];
static int nscripts;
static const char *doc_src;          /* what those offsets are relative to */

/* ---- the tag table ---------------------------------------------------------
 * The supported set and no more. An unknown tag is not an error: it becomes
 * HT_UNKNOWN, which layout treats as a span - so a page full of <section> and
 * <article> renders as text rather than as nothing. */
struct tag_ent { const char *name; short tag; };

static const struct tag_ent tags[] = {
    { "html", HT_HTML }, { "head", HT_HEAD }, { "body", HT_BODY },
    { "title", HT_TITLE },
    { "h1", HT_H1 }, { "h2", HT_H2 }, { "h3", HT_H3 },
    { "h4", HT_H4 }, { "h5", HT_H5 }, { "h6", HT_H6 },
    { "p", HT_P }, { "br", HT_BR }, { "hr", HT_HR }, { "a", HT_A },
    { "ul", HT_UL }, { "ol", HT_OL }, { "li", HT_LI },
    { "strong", HT_STRONG }, { "em", HT_EM }, { "b", HT_B }, { "i", HT_I },
    { "code", HT_CODE }, { "pre", HT_PRE },
    { "div", HT_DIV }, { "span", HT_SPAN }, { "img", HT_IMG },
    { "script", HT_SCRIPT }, { "style", HT_STYLE },
    { "table", HT_TABLE }, { "tr", HT_TR }, { "td", HT_TD }, { "th", HT_TH },
    { "thead", HT_THEAD }, { "tbody", HT_TBODY }, { "tfoot", HT_TFOOT },
    { "caption", HT_CAPTION },
    { 0, 0 }
};

static int lower(int c) { return (c >= 'A' && c <= 'Z') ? c + 32 : c; }

static int name_eq(const char *s, int len, const char *lit)
{
    int i = 0;
    for (; i < len; i++) {
        if (!lit[i] || lower(s[i]) != lit[i]) return 0;
    }
    return lit[i] == 0;
}

static short tag_of(const char *s, int len)
{
    for (int i = 0; tags[i].name; i++)
        if (name_eq(s, len, tags[i].name)) return tags[i].tag;
    return HT_UNKNOWN;
}

/* Void elements never have children and never need a close tag. Getting this
 * list wrong is how <br> swallows the rest of a document. */
static int is_void(short t)
{
    return t == HT_BR || t == HT_HR || t == HT_IMG;
}

/* Elements that start a new block. Used for the implied-close rules below and
 * by layout.c, which asks the same question about the same tags - so it is
 * answered once, here, and exported. */
int html_is_block(int t)
{
    switch (t) {
    case HT_HTML: case HT_BODY: case HT_DIV:
    case HT_P: case HT_PRE: case HT_UL: case HT_OL: case HT_LI:
    case HT_H1: case HT_H2: case HT_H3: case HT_H4: case HT_H5: case HT_H6:
    case HT_HR:
    case HT_TABLE: case HT_TR: case HT_TD: case HT_TH:
    case HT_THEAD: case HT_TBODY: case HT_TFOOT: case HT_CAPTION:
        return 1;
    default:
        return 0;
    }
}

/* ---- the arena ------------------------------------------------------------ */
static int arena_put(char c)
{
    if (used >= ARENA - 1) return 0;
    arena[used++] = c;
    return 1;
}

/* ---- nodes ---------------------------------------------------------------- */
static int node_new(short kind, short tag, int parent)
{
    if (nnodes >= MAX_NODES) return -1;
    int i = nnodes++;
    nodes[i].kind = kind;
    nodes[i].tag = tag;
    nodes[i].parent = parent;
    nodes[i].first = nodes[i].last = nodes[i].next = -1;
    nodes[i].toff = nodes[i].tlen = 0;
    nodes[i].aoff = nodes[i].alen = 0;
    nodes[i].coff = nodes[i].clen = 0;
    nodes[i].ioff = nodes[i].ilen = 0;
    nodes[i].soff = nodes[i].slen = 0;
    if (parent >= 0) {
        if (nodes[parent].last < 0) nodes[parent].first = i;
        else nodes[nodes[parent].last].next = i;
        nodes[parent].last = i;
    }
    return i;
}

/* ---- entities --------------------------------------------------------------
 * The five that matter plus numeric references. Anything unrecognised is left
 * as the literal text it was written as, which is what every real browser does
 * and is strictly better than dropping it. */
struct ent { const char *name; char ch; };
static const struct ent ents[] = {
    { "amp", '&' }, { "lt", '<' }, { "gt", '>' }, { "quot", '"' },
    { "apos", '\'' }, { "nbsp", ' ' }, { 0, 0 }
};

/* Reads an entity starting at src[i] (which is the '&'). Returns the index
 * just past it and stores the decoded character, or returns i unchanged when
 * this is not an entity at all. */
static int entity(const char *src, int len, int i, char *out)
{
    int j = i + 1;
    if (j < len && src[j] == '#') {
        int v = 0, k = j + 1, hex = 0;
        if (k < len && (src[k] == 'x' || src[k] == 'X')) { hex = 1; k++; }
        int start = k;
        while (k < len) {
            int c = lower(src[k]), d;
            if (c >= '0' && c <= '9') d = c - '0';
            else if (hex && c >= 'a' && c <= 'f') d = c - 'a' + 10;
            else break;
            v = v * (hex ? 16 : 10) + d;
            if (v > 0x10FFFF) v = '?';       /* no overflow, ever */
            k++;
        }
        if (k == start || k >= len || src[k] != ';') return i;
        *out = (v >= 32 && v < 127) ? (char)v : '?';
        return k + 1;
    }
    int start = j;
    while (j < len && ((lower(src[j]) >= 'a' && lower(src[j]) <= 'z'))) j++;
    if (j >= len || src[j] != ';' || j == start) return i;
    for (int e = 0; ents[e].name; e++)
        if (name_eq(src + start, j - start, ents[e].name)) {
            *out = ents[e].ch;
            return j + 1;
        }
    return i;                                 /* unknown: keep it literal */
}

/* ---- the parse -------------------------------------------------------------
 * One pass, one open-element stack, no lookahead beyond the current tag.
 */
static int  stack[MAX_DEPTH];
static int  sp;
static short stag[MAX_DEPTH];

static int cur(void) { return sp > 0 ? stack[sp - 1] : 0; }

static void push(int n, short t)
{
    if (sp < MAX_DEPTH) { stack[sp] = n; stag[sp] = t; sp++; }
    else n_dropped++;               /* too deep: the node exists, unstacked */
}

/* Close back to and including the nearest open `t`. Returns 0 and changes
 * nothing when there is no such element - that is the stray-close-tag case and
 * it must not pop anything, or one stray </div> unwinds the whole document.
 *
 * UNKNOWN ELEMENTS ARE MATCHED BY NAME, not by tag id. Every unsupported tag
 * shares the id HT_UNKNOWN, so matching on the id alone would let </footer>
 * close an open <section>. The name is kept in the node's spare attribute
 * slot for exactly this, and the cost is one string compare on a close tag
 * that was going to be rare anyway. Without it, one unclosed unknown element
 * swallows the entire rest of the document - which is what it did.
 */
static int close_to(short t, const char *nm, int nlen)
{
    int found = -1;
    for (int i = sp - 1; i >= 0; i--) {
        if (stag[i] != t) continue;
        if (t == HT_UNKNOWN) {
            int n = stack[i];
            if (nodes[n].alen != nlen) continue;
            int same = 1;
            for (int k = 0; k < nlen && same; k++)
                if (lower(nm[k]) != arena[nodes[n].aoff + k]) same = 0;
            if (!same) continue;
        }
        found = i;
        break;
    }
    if (found < 0) { n_dropped++; return 0; }
    sp = found;
    return 1;
}

/* An open <p> or <li> is closed by the next block-level start tag. This is the
 * single rule that makes real-world unclosed paragraphs lay out correctly, and
 * without it every subsequent block nests one level deeper than the last. */
static void imply_close(short t)
{
    if (!html_is_block(t)) return;
    while (sp > 0) {
        short o = stag[sp - 1];
        if (o == HT_P && t != HT_P) { sp--; continue; }
        if (o == HT_P && t == HT_P) { sp--; break; }
        if (o == HT_LI && t == HT_LI) { sp--; break; }
        if (o == HT_LI && (t == HT_UL || t == HT_OL)) break;   /* nested list */
        /* TABLES ARE WRITTEN WITHOUT CLOSE TAGS far more often than they are
         * written with them: `<tr><td>a<td>b<tr><td>c` is legal and common.
         * Without these three rules every cell nests inside the previous one
         * and a table becomes a staircase running off the right edge. */
        if ((o == HT_TD || o == HT_TH) && (t == HT_TD || t == HT_TH)) { sp--; break; }
        if ((o == HT_TD || o == HT_TH) && t == HT_TR) { sp--; continue; }
        if (o == HT_TR && t == HT_TR) { sp--; break; }
        if (o == HT_TR && (t == HT_TBODY || t == HT_THEAD || t == HT_TFOOT)) { sp--; continue; }
        break;
    }
}

/* Whitespace collapses to a single space everywhere except inside <pre>, where
 * it is preserved exactly. That one exception is why `pre_depth` exists. */
static int pre_depth;

static int in_pre(void)
{
    for (int i = 0; i < sp; i++) if (stag[i] == HT_PRE) return 1;
    return pre_depth > 0;
}

static int is_ws(int c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

/* Emit a text node for src[i..end). Returns nothing; an empty result (all
 * whitespace, collapsed away) creates no node at all, which keeps the tree
 * free of the blank text nodes that indentation would otherwise produce. */
static void emit_text(const char *src, int len, int from, int to)
{
    int keep_ws = in_pre();
    int off = used, n = 0;
    int last_ws = 0;
    for (int i = from; i < to; ) {
        char c = src[i];
        if (c == '&') {
            char dec;
            int j = entity(src, len, i, &dec);
            if (j != i) {
                if (arena_put(dec)) n++;
                last_ws = 0;
                i = j;
                continue;
            }
        }
        if (!keep_ws && is_ws(c)) {
            if (!last_ws && n > 0) { if (arena_put(' ')) n++; }
            else if (!last_ws && n == 0) { if (arena_put(' ')) n++; }
            last_ws = 1;
            i++;
            continue;
        }
        last_ws = 0;
        if ((unsigned char)c >= 0x80) c = '?';
        if (c == '\r') { i++; continue; }
        if (!keep_ws && c == '\n') c = ' ';
        if (arena_put(c)) n++;
        i++;
    }
    if (n == 0) return;
    /* a text node that is nothing but the one collapsed space carries no
     * content of its own - but it IS a word separator, so it is kept when it
     * sits between two other nodes and dropped at the very start of a parent */
    if (n == 1 && arena[off] == ' ') {
        int p = cur();
        if (p < 0 || nodes[p].last < 0) { used = off; return; }
    }
    int t = node_new(HN_TEXT, HT_NONE, cur());
    if (t < 0) { used = off; n_dropped++; return; }
    nodes[t].toff = off;
    nodes[t].tlen = n;
}

/* Copy an attribute value into the arena. Handles "quoted", 'quoted' and
 * unquoted-until-whitespace, because real pages use all three. */
static int attr_value(const char *src, int len, int *pi, int *voff, int *vlen)
{
    int i = *pi;
    while (i < len && is_ws(src[i])) i++;
    if (i >= len || src[i] != '=') { *pi = i; return 0; }
    i++;
    while (i < len && is_ws(src[i])) i++;
    if (i >= len) { *pi = i; return 0; }
    char q = 0;
    if (src[i] == '"' || src[i] == '\'') { q = src[i]; i++; }
    int off = used, n = 0;
    while (i < len) {
        char c = src[i];
        if (q && c == q) { i++; break; }
        if (!q && (is_ws(c) || c == '>')) break;
        if (c == '&') {
            char dec;
            int j = entity(src, len, i, &dec);
            if (j != i) { if (arena_put(dec)) n++; i = j; continue; }
        }
        if ((unsigned char)c >= 0x80) c = '?';
        if (arena_put(c)) n++;
        i++;
    }
    *pi = i;
    *voff = off;
    *vlen = n;
    return 1;
}

/* Skip to the matching close tag without building anything from the contents.
 * <script> and <style> hold code, not prose, and a tokenizer that treats their
 * bodies as text puts a stylesheet on the screen. */
static int skip_raw(const char *src, int len, int i, const char *name)
{
    while (i < len) {
        if (src[i] == '<' && i + 1 < len && src[i + 1] == '/') {
            int j = i + 2, s = j;
            while (j < len && !is_ws(src[j]) && src[j] != '>') j++;
            if (name_eq(src + s, j - s, name)) {
                while (j < len && src[j] != '>') j++;
                return j < len ? j + 1 : len;
            }
        }
        i++;
    }
    return len;
}

void html_reset(void)
{
    nnodes = 0;
    used = 0;
    sp = 0;
    pre_depth = 0;
    title_off = title_len = 0;
    n_dropped = 0;
    nsheets = 0;
    nscripts = 0;
    doc_src = 0;
}

int html_parse(const char *src, int len)
{
    html_reset();
    doc_src = src;                   /* the sheets are spans of THIS buffer */
    if (!src || len <= 0) { node_new(HN_ELEM, HT_HTML, -1); return 0; }

    int root = node_new(HN_ELEM, HT_HTML, -1);
    push(root, HT_HTML);

    int i = 0, text_from = 0;
    while (i < len) {
        if (src[i] != '<') { i++; continue; }

        /* comment, CDATA or doctype - none of them are content */
        if (i + 3 < len && src[i + 1] == '!' ) {
            if (i > text_from) emit_text(src, len, text_from, i);
            if (src[i + 2] == '-' && src[i + 3] == '-') {
                int j = i + 4;
                while (j + 2 < len && !(src[j] == '-' && src[j+1] == '-' && src[j+2] == '>')) j++;
                i = (j + 2 < len) ? j + 3 : len;
            } else {
                int j = i;
                while (j < len && src[j] != '>') j++;
                i = (j < len) ? j + 1 : len;
            }
            text_from = i;
            continue;
        }

        int close = (i + 1 < len && src[i + 1] == '/');
        int ns = i + 1 + (close ? 1 : 0);
        int ne = ns;
        while (ne < len && !is_ws(src[ne]) && src[ne] != '>' && src[ne] != '/') ne++;
        if (ne == ns) {                       /* a bare '<' - it is TEXT */
            i++;
            continue;
        }

        if (i > text_from) emit_text(src, len, text_from, i);
        short t = tag_of(src + ns, ne - ns);

        if (close) {
            int j = ne;
            while (j < len && src[j] != '>') j++;
            i = (j < len) ? j + 1 : len;
            text_from = i;
            if (is_void(t)) continue;         /* </br> is nothing */
            close_to(t, src + ns, ne - ns);
            continue;
        }

        /* attributes: href, alt, and the three css.c needs. The rest are
         * parsed and discarded, because parsing them is what makes '>' inside
         * a quoted attribute value not end the tag. */
        int j = ne;
        int href_off = 0, href_len = 0;
        int cls_off = 0, cls_len = 0;
        int id_off = 0, id_len = 0;
        int sty_off = 0, sty_len = 0;
        int self_close = 0;
        while (j < len && src[j] != '>') {
            while (j < len && (is_ws(src[j]) || src[j] == '/')) {
                if (src[j] == '/') self_close = 1;
                j++;
            }
            if (j >= len || src[j] == '>') break;
            int as = j;
            while (j < len && !is_ws(src[j]) && src[j] != '=' && src[j] != '>') j++;
            int alen2 = j - as;
            int voff = 0, vlen = 0;
            int save = used;
            int got = attr_value(src, len, &j, &voff, &vlen);
            int is_href = name_eq(src + as, alen2, "href") ||
                          (t == HT_IMG && name_eq(src + as, alen2, "alt"));
            if (got && is_href && !href_len)                            { href_off = voff; href_len = vlen; }
            else if (got && name_eq(src + as, alen2, "class") && !cls_len) { cls_off = voff; cls_len = vlen; }
            else if (got && name_eq(src + as, alen2, "id")    && !id_len)  { id_off = voff;  id_len = vlen; }
            else if (got && name_eq(src + as, alen2, "style") && !sty_len) { sty_off = voff; sty_len = vlen; }
            else if (got) used = save;        /* not wanted: give the arena back */
            if (!got) { /* a valueless attribute, e.g. <input disabled> */ }
        }
        i = (j < len) ? j + 1 : len;
        text_from = i;

        if (t == HT_SCRIPT || t == HT_STYLE) {
            int raw0 = i;
            i = skip_raw(src, len, i, t == HT_SCRIPT ? "script" : "style");
            /* A <style> element's TEXT is the document's stylesheet, so it is
             * kept - as a span of the source, uncopied. <script> is still
             * discarded: there is no engine, and keeping it would only mean
             * holding a megabyte of JavaScript the renderer cannot use.
             * Several <style> blocks concatenate in document order, which is
             * also their cascade order. */
            if (t == HT_SCRIPT && nscripts < MAX_SHEETS && i > raw0) {
                /* SCRIPTS ARE KEPT NOW, for the same reason stylesheets are:
                 * there is an interpreter to give them to. js.c runs a bounded
                 * subset - see its header for exactly what - so a script this
                 * kernel cannot run fails inside the interpreter with a
                 * message, rather than being silently dropped here. */
                int end = i;
                while (end > raw0 && src[end - 1] != '<') end--;
                if (end > raw0) end--; else end = i;
                if (end > raw0) {
                    scripts[nscripts].off = raw0;
                    scripts[nscripts].len = end - raw0;
                    nscripts++;
                }
            }
            if (t == HT_STYLE && nsheets < MAX_SHEETS && i > raw0) {
                int end = i;
                /* skip_raw returns the index PAST </style>; walk back to the
                 * '<' so the closing tag is not part of the sheet */
                while (end > raw0 && src[end - 1] != '<') end--;
                if (end > raw0) end--; else end = i;
                if (end > raw0) {
                    sheets[nsheets].off = raw0;
                    sheets[nsheets].len = end - raw0;
                    nsheets++;
                }
            }
            text_from = i;
            continue;
        }

        /* The tree is rooted at an implicit <html> that exists before the
         * document is read, because a document may not have one and the tree
         * still needs a root. When the document DOES have one, it is the same
         * element - not a child of it. Get this wrong and every real page
         * renders one level deeper than every fragment. */
        if (t == HT_HTML && sp == 1 && nodes[root].first < 0) continue;

        imply_close(t);
        int n = node_new(HN_ELEM, t, cur());
        if (n < 0) { n_dropped++; continue; }
        nodes[n].aoff = href_off;
        nodes[n].alen = href_len;
        nodes[n].coff = cls_off; nodes[n].clen = cls_len;
        nodes[n].ioff = id_off;  nodes[n].ilen = id_len;
        nodes[n].soff = sty_off; nodes[n].slen = sty_len;

        /* an unsupported element keeps its NAME here instead of an href, so
         * its close tag can be matched exactly - see close_to() */
        if (t == HT_UNKNOWN) {
            int off = used, cnt = 0;
            for (int k = ns; k < ne; k++)
                if (arena_put((char)lower(src[k]))) cnt++;
            nodes[n].aoff = off;
            nodes[n].alen = cnt;
        }

        if (t == HT_TITLE) {
            /* the title's text is wanted as a string, not as a rendered node */
            int te = i;
            while (te < len && !(src[te] == '<' && te + 1 < len && src[te+1] == '/')) te++;
            int off = used, cnt = 0;
            for (int k = i; k < te; k++) {
                char c = src[k];
                if (is_ws(c)) c = ' ';
                if ((unsigned char)c >= 0x80) c = '?';
                if (arena_put(c)) cnt++;
            }
            title_off = off;
            title_len = cnt;
            i = skip_raw(src, len, i, "title");
            text_from = i;
            continue;
        }

        if (!is_void(t) && !self_close) push(n, t);
    }
    if (len > text_from) emit_text(src, len, text_from, len);
    return nnodes;
}

/* ---- accessors -------------------------------------------------------------
 * Everything above is private. layout.c sees a tree through these and nothing
 * else, which is what lets the host harness link layout.c without a kernel.
 */
int html_count(void)          { return nnodes; }
int html_root(void)           { return nnodes > 0 ? 0 : -1; }
int html_dropped(void)        { return n_dropped; }

int html_kind(int i)   { return (unsigned)i < (unsigned)nnodes ? nodes[i].kind : -1; }
int html_tag(int i)    { return (unsigned)i < (unsigned)nnodes ? nodes[i].tag : HT_NONE; }
int html_parent(int i) { return (unsigned)i < (unsigned)nnodes ? nodes[i].parent : -1; }
int html_first(int i)  { return (unsigned)i < (unsigned)nnodes ? nodes[i].first : -1; }
int html_next(int i)   { return (unsigned)i < (unsigned)nnodes ? nodes[i].next : -1; }

const char *html_text(int i, int *len)
{
    if ((unsigned)i >= (unsigned)nnodes || nodes[i].kind != HN_TEXT) {
        if (len) *len = 0;
        return "";
    }
    if (len) *len = nodes[i].tlen;
    return arena + nodes[i].toff;
}

/* An <a>'s href or an <img>'s alt, and nothing else. The same slot holds an
 * unknown element's NAME (see close_to), which is private - returning it here
 * would make a <section> look like it had an href. */
const char *html_href(int i, int *len)
{
    if ((unsigned)i >= (unsigned)nnodes || !nodes[i].alen ||
        (nodes[i].tag != HT_A && nodes[i].tag != HT_IMG)) {
        if (len) *len = 0;
        return "";
    }
    if (len) *len = nodes[i].alen;
    return arena + nodes[i].aoff;
}

/* class, id and style=. Total like every other accessor: an element without
 * one answers "" and length 0, which is exactly what css.c wants to see. */
const char *html_class(int i, int *len)
{
    if ((unsigned)i >= (unsigned)nnodes || !nodes[i].clen) { if (len) *len = 0; return ""; }
    if (len) *len = nodes[i].clen;
    return arena + nodes[i].coff;
}

const char *html_id(int i, int *len)
{
    if ((unsigned)i >= (unsigned)nnodes || !nodes[i].ilen) { if (len) *len = 0; return ""; }
    if (len) *len = nodes[i].ilen;
    return arena + nodes[i].ioff;
}

const char *html_style_attr(int i, int *len)
{
    if ((unsigned)i >= (unsigned)nnodes || !nodes[i].slen) { if (len) *len = 0; return ""; }
    if (len) *len = nodes[i].slen;
    return arena + nodes[i].soff;
}

int html_sheets(void) { return nsheets; }

int html_scripts(void) { return nscripts; }

const char *html_script(int k, int *len)
{
    if ((unsigned)k >= (unsigned)nscripts || !doc_src) { if (len) *len = 0; return ""; }
    if (len) *len = scripts[k].len;
    return doc_src + scripts[k].off;
}

const char *html_sheet(int k, int *len)
{
    if ((unsigned)k >= (unsigned)nsheets || !doc_src) { if (len) *len = 0; return ""; }
    if (len) *len = sheets[k].len;
    return doc_src + sheets[k].off;
}

/* The element's own tag NAME, which is what a CSS type selector matches
 * against. HT_UNKNOWN already keeps its name in the href slot (see close_to),
 * so this reaches <section> and <article> too. */
const char *html_tagname(int i, int *len)
{
    static const char *names[] = {
        "", "html", "head", "body", "title",
        "h1", "h2", "h3", "h4", "h5", "h6",
        "p", "br", "hr", "a",
        "ul", "ol", "li",
        "strong", "em", "b", "i",
        "code", "pre",
        "div", "span", "img",
        "script", "style",
        "table", "tr", "td", "th", "thead", "tbody", "tfoot", "caption"
    };
    if ((unsigned)i >= (unsigned)nnodes || nodes[i].kind != HN_ELEM) {
        if (len) *len = 0;
        return "";
    }
    int t = nodes[i].tag;
    if (t == HT_UNKNOWN) {
        if (len) *len = nodes[i].alen;
        return nodes[i].alen ? arena + nodes[i].aoff : "";
    }
    if (t <= 0 || t >= (int)(sizeof names / sizeof names[0])) { if (len) *len = 0; return ""; }
    const char *s = names[t];
    int n = 0;
    while (s[n]) n++;
    if (len) *len = n;
    return s;
}

const char *html_title(int *len)
{
    if (len) *len = title_len;
    return title_len ? arena + title_off : "";
}

/* How deep is node i? Walked rather than stored, because it is only ever asked
 * by the harness and by the list-indent code, neither of which is hot. */
int html_depth(int i)
{
    int d = 0;
    while ((unsigned)i < (unsigned)nnodes && nodes[i].parent >= 0) {
        i = nodes[i].parent;
        if (++d > MAX_NODES) return d;        /* a cycle cannot happen; prove it */
    }
    return d;
}

int html_max_depth(void)
{
    int m = 0;
    for (int i = 0; i < nnodes; i++) {
        int d = html_depth(i);
        if (d > m) m = d;
    }
    return m;
}

int html_arena_used(void) { return used; }
