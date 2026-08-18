/* css.c - the bounded CSS engine.
 *
 * The browser's home page said, honestly, "the stylesheet is the one compiled
 * into layout.c; a page cannot bring its own." This is that limit removed for
 * the part of CSS a document actually uses, and left in place for the rest.
 *
 * THREE THINGS IT DOES, in the order they happen:
 *
 *   1. PARSE. A stylesheet is rules; a rule is a selector group and a
 *      declaration block. Comments, at-rules and anything unparseable are
 *      skipped to the next safe point rather than aborting the sheet - a
 *      stylesheet with one bad rule is the normal case on the real web, and a
 *      parser that gives up on it renders nothing.
 *
 *   2. MATCH. A selector is compound selectors separated by descendant
 *      combinators, read RIGHT TO LEFT against the ancestor path. Right to
 *      left is not a micro-optimisation here, it is what makes the match a
 *      single backward walk with no recursion and no backtracking array.
 *
 *   3. CASCADE. Specificity (ids, then classes, then types), source order as
 *      the tie-break, and the style= attribute above everything. Same order
 *      the spec gives, minus !important, which is not supported and is
 *      ignored rather than mis-ranked.
 *
 * NO HEAP AND NO FAILURE PATH. Fixed arrays; strings are offsets into one
 * arena. Every limit refuses new work and records it in `overflow` instead of
 * growing or scribbling, so a hostile stylesheet costs a truncated sheet
 * rather than the kernel.
 */
#include "css.h"

#define MAX_RULES  192          /* a rule is one selector + its block      */
#define MAX_SELS   384          /* comma groups make these outnumber rules */
#define MAX_DECLS  1024
#define MAX_COMP   6            /* compound selectors per selector         */
#define ARENA      12288

struct comp {                   /* one compound selector: div#id.a.b       */
    unsigned short tag, tag_len;
    unsigned short id,  id_len;
    unsigned short cls, cls_len;      /* one class; more chain via cls2    */
    unsigned short cls2, cls2_len;
};

struct sel {
    struct comp c[MAX_COMP];
    unsigned char n;            /* compounds, path order (leftmost first)  */
    int spec;                   /* id*10000 + class*100 + type             */
    unsigned short decl0, ndecl;
    int order;                  /* source order, for equal specificity     */
};

struct decl {
    unsigned char prop;         /* P_* below                               */
    int v;                      /* value, pre-decoded                      */
    unsigned char unit;         /* U_* below                               */
};

/* properties, as a dense index rather than the sparse CSS_P_* bit */
enum { P_COLOR = 1, P_BG, P_SIZE, P_WEIGHT, P_STYLE, P_FAMILY, P_ALIGN,
       P_DECOR, P_DISPLAY, P_MT, P_MB, P_ML, P_MR, P_PT, P_PB, P_PL, P_PR };

enum { U_PX = 0, U_EM, U_PCT };

static struct sel  sels[MAX_SELS];
static struct decl decls[MAX_DECLS];
static char  arena[ARENA];
static int   nsels, ndecls, aused, nrules, overflow, order_seq;

void css_reset(void)
{
    nsels = ndecls = aused = nrules = overflow = order_seq = 0;
}

int css_rules(void)       { return nrules; }
int css_decls(void)       { return ndecls; }
int css_arena_used(void)  { return aused; }
int css_overflowed(void)  { return overflow; }

/* ---- the small stuff -------------------------------------------------- */

static int is_ws(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f'; }
static int lower(int c)  { return (c >= 'A' && c <= 'Z') ? c + 32 : c; }

/* If a comment starts at i, return the index past it; otherwise return i.
 *
 * COMMENTS ARE WHITESPACE AND CAN APPEAR ANYWHERE - between a selector and its
 * brace, between a property and its colon, inside a value. Stripping them in a
 * pre-pass would need a copy of the sheet, and there is no heap to put one in,
 * so every scanner that walks raw text calls this instead. Found by the gate:
 * `/●* c *●/ p /●* c *●/ { color: red }` parsed the comment as part of the
 * selector and the rule silently never matched. */
static int skip_comment(const char *s, int len, int i)
{
    if (i + 1 < len && s[i] == '/' && s[i + 1] == '*') {
        i += 2;
        while (i + 1 < len && !(s[i] == '*' && s[i + 1] == '/')) i++;
        return (i + 1 < len) ? i + 2 : len;
    }
    return i;
}

static int at_comment(const char *s, int len, int i)
{
    return i + 1 < len && s[i] == '/' && s[i + 1] == '*';
}

/* trim whitespace AND comments from both ends of s[0..*len) */
static const char *trim_cs(const char *s, int *len)
{
    for (;;) {
        while (*len > 0 && is_ws(*s)) { s++; (*len)--; }
        if (at_comment(s, *len, 0)) {
            int k = skip_comment(s, *len, 0);
            s += k; *len -= k;
            continue;
        }
        break;
    }
    for (;;) {
        while (*len > 0 && is_ws(s[*len - 1])) (*len)--;
        /* a trailing comment: find an opener whose close runs to the end */
        int cut = -1;
        for (int i = 0; i + 1 < *len; i++)
            if (s[i] == '/' && s[i + 1] == '*' && skip_comment(s, *len, i) >= *len) { cut = i; break; }
        if (cut >= 0) { *len = cut; continue; }
        break;
    }
    return s;
}

static int ieq(const char *a, int alen, const char *b)
{
    int i = 0;
    for (; i < alen; i++) {
        if (!b[i]) return 0;
        if (lower((unsigned char)a[i]) != lower((unsigned char)b[i])) return 0;
    }
    return b[i] == 0;
}

/* Copy into the arena and return the offset, or -1 when full. Offsets are
 * unsigned short in the structs, so the arena cannot exceed 65535 either -
 * ARENA is well under that and this checks anyway, because the day someone
 * raises it is the day the truncation would go silent. */
static int intern(const char *s, int len)
{
    if (len < 0) len = 0;
    if (len > 255) len = 255;              /* no selector part is longer   */
    if (aused + len + 1 > ARENA || aused + len + 1 > 65535) return -1;
    int off = aused;
    for (int i = 0; i < len; i++) arena[aused++] = s[i];
    arena[aused++] = 0;
    return off;
}

static int digits(const char *s, int len, int *i, int *out)
{
    int v = 0, got = 0;
    while (*i < len && s[*i] >= '0' && s[*i] <= '9') { v = v * 10 + (s[*i] - '0'); (*i)++; got = 1; }
    *out = v;
    return got;
}

/* ---- colours ------------------------------------------------------------
 * Hex plus the sixteen HTML colour names and a few more that show up in real
 * documents. A name that is not here returns -1, which the caller treats as
 * "the declaration did not parse" - so an unknown colour leaves the inherited
 * one rather than painting something invented. */
struct named { const char *name; int rgb; };
static const struct named colours[] = {
    { "black", 0x000000 }, { "white",  0xFFFFFF }, { "red",     0xFF0000 },
    { "green", 0x008000 }, { "blue",   0x0000FF }, { "gray",    0x808080 },
    { "grey",  0x808080 }, { "silver", 0xC0C0C0 }, { "navy",    0x000080 },
    { "teal",  0x008080 }, { "olive",  0x808000 }, { "purple",  0x800080 },
    { "maroon", 0x800000 }, { "lime",  0x00FF00 }, { "aqua",    0x00FFFF },
    { "cyan",  0x00FFFF }, { "fuchsia", 0xFF00FF }, { "magenta", 0xFF00FF },
    { "yellow", 0xFFFF00 }, { "orange", 0xFFA500 }, { "pink",   0xFFC0CB },
    { "brown", 0xA52A2A }, { "gold",   0xFFD700 }, { "indigo",  0x4B0082 },
    { "violet", 0xEE82EE }, { "beige", 0xF5F5DC }, { "tan",     0xD2B48C },
    { "crimson", 0xDC143C }, { "salmon", 0xFA8072 }, { "khaki",  0xF0E68C },
    { "transparent", -1 }, { 0, 0 }
};

static int hexval(int c)
{
    c = lower(c);
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static int parse_colour(const char *s, int len)
{
    while (len > 0 && is_ws(*s)) { s++; len--; }
    while (len > 0 && is_ws(s[len - 1])) len--;
    if (len <= 0) return -1;

    if (*s == '#') {
        int n = len - 1, v = 0;
        if (n != 3 && n != 6) return -1;
        for (int i = 1; i < len; i++) {
            int h = hexval((unsigned char)s[i]);
            if (h < 0) return -1;
            v = (v << 4) | h;
        }
        if (n == 3) {   /* #abc -> #aabbcc */
            int r = (v >> 8) & 0xF, g = (v >> 4) & 0xF, b = v & 0xF;
            v = (r << 20) | (r << 16) | (g << 12) | (g << 8) | (b << 4) | b;
        }
        return v;
    }
    /* rgb(r,g,b) - common enough in real documents to be worth the ten lines */
    if (len > 4 && ieq(s, 3, "rgb")) {
        int i = 3;
        while (i < len && s[i] != '(') i++;
        if (i >= len) return -1;
        i++;
        int c[3] = { 0, 0, 0 };
        for (int k = 0; k < 3; k++) {
            while (i < len && (is_ws(s[i]) || s[i] == ',')) i++;
            if (!digits(s, len, &i, &c[k])) return -1;
            if (c[k] > 255) c[k] = 255;
        }
        return (c[0] << 16) | (c[1] << 8) | c[2];
    }
    for (int i = 0; colours[i].name; i++)
        if (ieq(s, len, colours[i].name)) return colours[i].rgb;
    return -1;
}

/* ---- parsing a declaration block --------------------------------------- */

/* a length: number plus optional unit. Returns 0 when it is not a length. */
static int parse_len(const char *s, int len, int *val, int *unit)
{
    int i = 0, neg = 0;
    while (i < len && is_ws(s[i])) i++;
    if (i < len && (s[i] == '-' || s[i] == '+')) { neg = (s[i] == '-'); i++; }
    int whole = 0;
    if (!digits(s, len, &i, &whole)) return 0;
    int frac = 0, fdigits = 0;
    if (i < len && s[i] == '.') {
        i++;
        while (i < len && s[i] >= '0' && s[i] <= '9' && fdigits < 2) {
            frac = frac * 10 + (s[i] - '0'); i++; fdigits++;
        }
        while (i < len && s[i] >= '0' && s[i] <= '9') i++;
    }
    while (fdigits < 2) { frac *= 10; fdigits++; }     /* hundredths        */

    *unit = U_PX;
    if (i < len && s[i] == '%') { *unit = U_PCT; i++; }
    else if (i + 1 < len && lower((unsigned char)s[i]) == 'e' &&
             lower((unsigned char)s[i + 1]) == 'm') { *unit = U_EM; i += 2; }
    else if (i + 1 < len && lower((unsigned char)s[i]) == 'p' &&
             lower((unsigned char)s[i + 1]) == 'x') { i += 2; }
    else if (i + 1 < len && lower((unsigned char)s[i]) == 'r' &&
             i + 2 < len && lower((unsigned char)s[i + 1]) == 'e' &&
             lower((unsigned char)s[i + 2]) == 'm') { *unit = U_EM; i += 3; }

    /* ONE SCALE FOR BOTH RELATIVE UNITS: hundredths of the parent value, so
     * resolve() is the same arithmetic either way. 2em and 200% are both 200.
     * Carrying em in hundredths of an em while % stayed in whole percent made
     * resolve() out by 100x - the gate caught `font-size: 150%` computing 2400
     * px against a 16px parent. */
    if (*unit == U_PX)       *val = whole;
    else if (*unit == U_PCT) *val = whole;            /* already per-hundred  */
    else                     *val = whole * 100 + frac;   /* em -> hundredths */
    if (neg) *val = -(*val);
    return 1;
}

static void add_decl(int prop, int v, int unit)
{
    if (ndecls >= MAX_DECLS) { overflow = 1; return; }
    decls[ndecls].prop = (unsigned char)prop;
    decls[ndecls].v = v;
    decls[ndecls].unit = (unsigned char)unit;
    ndecls++;
}

/* One `name: value` pair. Shorthands that expand to several properties are
 * expanded here rather than at compute time, so the cascade sees the same
 * thing whether the author wrote `margin` or `margin-top`. */
static void parse_decl(const char *n, int nl, const char *v, int vl)
{
    n = trim_cs(n, &nl);
    v = trim_cs(v, &vl);
    if (nl <= 0 || vl <= 0) return;

    int val, unit;

    if (ieq(n, nl, "color")) {
        int c = parse_colour(v, vl);
        if (c >= 0) add_decl(P_COLOR, c, U_PX);
        return;
    }
    if (ieq(n, nl, "background-color") || ieq(n, nl, "background")) {
        int c = parse_colour(v, vl);
        if (c >= 0) add_decl(P_BG, c, U_PX);
        return;
    }
    if (ieq(n, nl, "font-size")) {
        if (ieq(v, vl, "small"))       { add_decl(P_SIZE, 85,  U_PCT); return; }
        if (ieq(v, vl, "x-small"))     { add_decl(P_SIZE, 70,  U_PCT); return; }
        if (ieq(v, vl, "large"))       { add_decl(P_SIZE, 120, U_PCT); return; }
        if (ieq(v, vl, "x-large"))     { add_decl(P_SIZE, 150, U_PCT); return; }
        if (ieq(v, vl, "xx-large"))    { add_decl(P_SIZE, 200, U_PCT); return; }
        if (ieq(v, vl, "medium"))      { add_decl(P_SIZE, 100, U_PCT); return; }
        if (parse_len(v, vl, &val, &unit) && val > 0) add_decl(P_SIZE, val, unit);
        return;
    }
    if (ieq(n, nl, "font-weight")) {
        if (ieq(v, vl, "bold") || ieq(v, vl, "bolder") || ieq(v, vl, "700") ||
            ieq(v, vl, "800") || ieq(v, vl, "900") || ieq(v, vl, "600"))
            add_decl(P_WEIGHT, 1, U_PX);
        else if (ieq(v, vl, "normal") || ieq(v, vl, "400") ||
                 ieq(v, vl, "300") || ieq(v, vl, "lighter") || ieq(v, vl, "500"))
            add_decl(P_WEIGHT, 0, U_PX);
        return;
    }
    if (ieq(n, nl, "font-style")) {
        if (ieq(v, vl, "italic") || ieq(v, vl, "oblique")) add_decl(P_STYLE, 1, U_PX);
        else if (ieq(v, vl, "normal")) add_decl(P_STYLE, 0, U_PX);
        return;
    }
    if (ieq(n, nl, "font-family") || ieq(n, nl, "font")) {
        /* the only distinction the one atlas can honour */
        int mono = 0;
        for (int i = 0; i + 3 < vl; i++)
            if (lower((unsigned char)v[i]) == 'm' && ieq(v + i, 4, "mono")) { mono = 1; break; }
        for (int i = 0; i + 9 <= vl; i++)
            if (ieq(v + i, 9, "monospace")) { mono = 1; break; }
        add_decl(P_FAMILY, mono, U_PX);
        return;
    }
    if (ieq(n, nl, "text-align")) {
        if (ieq(v, vl, "center"))     add_decl(P_ALIGN, CSS_ALIGN_CENTER, U_PX);
        else if (ieq(v, vl, "right")) add_decl(P_ALIGN, CSS_ALIGN_RIGHT,  U_PX);
        else if (ieq(v, vl, "left"))  add_decl(P_ALIGN, CSS_ALIGN_LEFT,   U_PX);
        return;
    }
    if (ieq(n, nl, "text-decoration") || ieq(n, nl, "text-decoration-line")) {
        if (ieq(v, vl, "underline"))  add_decl(P_DECOR, 1, U_PX);
        else if (ieq(v, vl, "none"))  add_decl(P_DECOR, 0, U_PX);
        return;
    }
    if (ieq(n, nl, "display")) {
        if (ieq(v, vl, "none"))        add_decl(P_DISPLAY, CSS_DISP_NONE,   U_PX);
        else if (ieq(v, vl, "inline")) add_decl(P_DISPLAY, CSS_DISP_INLINE, U_PX);
        else                           add_decl(P_DISPLAY, CSS_DISP_BLOCK,  U_PX);
        return;
    }

    /* margin / padding, longhand and shorthand */
    static const struct { const char *name; int p; } one[] = {
        { "margin-top", P_MT }, { "margin-bottom", P_MB },
        { "margin-left", P_ML }, { "margin-right", P_MR },
        { "padding-top", P_PT }, { "padding-bottom", P_PB },
        { "padding-left", P_PL }, { "padding-right", P_PR }, { 0, 0 }
    };
    for (int i = 0; one[i].name; i++)
        if (ieq(n, nl, one[i].name)) {
            if (parse_len(v, vl, &val, &unit)) add_decl(one[i].p, val, unit);
            return;
        }

    int is_m = ieq(n, nl, "margin"), is_p = ieq(n, nl, "padding");
    if (is_m || is_p) {
        /* up to four values, CSS order: top right bottom left */
        int vals[4], units[4], k = 0, i = 0;
        while (k < 4 && i < vl) {
            while (i < vl && is_ws(v[i])) i++;
            if (i >= vl) break;
            int st = i;
            while (i < vl && !is_ws(v[i])) i++;
            if (!parse_len(v + st, i - st, &vals[k], &units[k])) {
                /* `margin: 0 auto` is the centring idiom; auto is not a length
                 * and must not abort the whole declaration */
                vals[k] = 0; units[k] = U_PX;
            }
            k++;
        }
        if (k == 0) return;
        int t = vals[0], r = vals[0], b = vals[0], l = vals[0];
        int tu = units[0], ru = units[0], bu = units[0], lu = units[0];
        if (k >= 2) { r = vals[1]; ru = units[1]; l = vals[1]; lu = units[1]; }
        if (k >= 3) { b = vals[2]; bu = units[2]; }
        if (k >= 4) { l = vals[3]; lu = units[3]; }
        add_decl(is_m ? P_MT : P_PT, t, tu);
        add_decl(is_m ? P_MR : P_PR, r, ru);
        add_decl(is_m ? P_MB : P_PB, b, bu);
        add_decl(is_m ? P_ML : P_PL, l, lu);
        return;
    }
    /* anything else: silently not supported, which is the honest behaviour -
     * a property we cannot act on must not become a property we half act on */
}

/* ---- parsing a selector ------------------------------------------------- */

/* One compound: type, #id and .class parts in any order. Returns 0 if the text
 * held nothing usable. */
static int parse_comp(const char *s, int len, struct comp *c, int *spec)
{
    c->tag = c->id = c->cls = c->cls2 = 0;
    c->tag_len = c->id_len = c->cls_len = c->cls2_len = 0;
    int got = 0, i = 0;
    while (i < len) {
        if (s[i] == '*') { i++; got = 1; continue; }        /* matches all  */
        if (s[i] == '#' || s[i] == '.') {
            char kind = s[i];
            i++;
            int st = i;
            while (i < len && s[i] != '.' && s[i] != '#' && s[i] != ':' &&
                   s[i] != '[') i++;
            int off = intern(s + st, i - st);
            if (off < 0) return 0;
            if (kind == '#') {
                c->id = (unsigned short)off; c->id_len = (unsigned short)(i - st);
                *spec += 10000;
            } else if (!c->cls_len) {
                c->cls = (unsigned short)off; c->cls_len = (unsigned short)(i - st);
                *spec += 100;
            } else if (!c->cls2_len) {
                c->cls2 = (unsigned short)off; c->cls2_len = (unsigned short)(i - st);
                *spec += 100;
            }
            got = 1;
            continue;
        }
        /* a pseudo-class or attribute selector: not supported, and the whole
         * selector is REFUSED rather than matched too broadly. Matching
         * `a:hover` as `a` would style every link permanently. */
        if (s[i] == ':' || s[i] == '[') return 0;
        int st = i;
        while (i < len && s[i] != '.' && s[i] != '#' && s[i] != ':' && s[i] != '[') i++;
        if (i > st) {
            int off = intern(s + st, i - st);
            if (off < 0) return 0;
            c->tag = (unsigned short)off; c->tag_len = (unsigned short)(i - st);
            *spec += 1;
            got = 1;
        }
    }
    return got;
}

/* One selector out of a comma group, e.g. "article .body p". Combinators other
 * than descendant (>, +, ~) are refused for the same reason pseudo-classes
 * are: treating `a > b` as `a b` matches strictly more than the author asked
 * for, and over-matching a stylesheet is worse than ignoring one rule. */
static int parse_sel(const char *s, int len, int d0, int nd)
{
    if (nsels >= MAX_SELS) { overflow = 1; return 0; }
    struct sel *sl = &sels[nsels];
    sl->n = 0; sl->spec = 0;
    int i = 0;
    while (i < len) {
        /* comments count as whitespace here, and separate compounds exactly as
         * a space does - which is what the spec says and what a real sheet
         * relies on */
        while (i < len && (is_ws(s[i]) || at_comment(s, len, i))) {
            if (at_comment(s, len, i)) i = skip_comment(s, len, i);
            else i++;
        }
        if (i >= len) break;
        if (s[i] == '>' || s[i] == '+' || s[i] == '~') return 0;
        int st = i;
        while (i < len && !is_ws(s[i]) && !at_comment(s, len, i)) {
            if (s[i] == '>' || s[i] == '+' || s[i] == '~') return 0;
            i++;
        }
        if (sl->n >= MAX_COMP) return 0;
        if (!parse_comp(s + st, i - st, &sl->c[sl->n], &sl->spec)) return 0;
        sl->n++;
    }
    if (sl->n == 0) return 0;
    sl->decl0 = (unsigned short)d0;
    sl->ndecl = (unsigned short)nd;
    sl->order = order_seq++;
    nsels++;
    return 1;
}

/* ---- the sheet ---------------------------------------------------------- */

int css_add_sheet(const char *src, int len)
{
    if (!src || len <= 0) return 0;
    int i = 0, took = 0;

    while (i < len) {
        /* skip whitespace and comments */
        while (i < len) {
            if (is_ws(src[i])) { i++; continue; }
            if (i + 1 < len && src[i] == '/' && src[i + 1] == '*') {
                i += 2;
                while (i + 1 < len && !(src[i] == '*' && src[i + 1] == '/')) i++;
                i = (i + 1 < len) ? i + 2 : len;
                continue;
            }
            break;
        }
        if (i >= len) break;

        /* an at-rule: @media, @import, @font-face. Skip its block entirely
         * rather than guessing - a @media whose rules were applied
         * unconditionally would style a print stylesheet onto the screen. */
        if (src[i] == '@') {
            int depth = 0;
            while (i < len) {
                if (src[i] == '{') depth++;
                else if (src[i] == '}') { depth--; i++; if (depth <= 0) break; continue; }
                else if (src[i] == ';' && depth == 0) { i++; break; }
                i++;
            }
            continue;
        }

        int sel_start = i;
        while (i < len && src[i] != '{' && src[i] != '}') i++;
        if (i >= len) break;
        if (src[i] == '}') { i++; continue; }        /* stray brace: recover */
        int sel_end = i;
        i++;                                          /* past the {          */

        int blk_start = i;
        while (i < len && src[i] != '}') i++;
        int blk_end = i;
        if (i < len) i++;                             /* past the }          */

        /* the declarations first, so every selector in the group can point at
         * the same run of them */
        int d0 = ndecls;
        int j = blk_start;
        while (j < blk_end) {
            int ns = j;
            while (j < blk_end && src[j] != ':' && src[j] != ';') j++;
            if (j >= blk_end || src[j] == ';') { j++; continue; }
            int ne = j;
            j++;                                      /* past the :          */
            int vs = j;
            while (j < blk_end && src[j] != ';') j++;
            parse_decl(src + ns, ne - ns, src + vs, j - vs);
            if (j < blk_end) j++;
        }
        int nd = ndecls - d0;
        if (nd == 0) continue;                        /* nothing to apply    */

        /* then the comma group */
        int k = sel_start, any = 0;
        while (k < sel_end) {
            int st = k;
            while (k < sel_end && src[k] != ',') k++;
            if (nsels >= MAX_SELS) { overflow = 1; break; }
            if (parse_sel(src + st, k - st, d0, nd)) any = 1;
            if (k < sel_end) k++;
        }
        if (any) { nrules++; took++; }
        else if (nd) ndecls = d0;   /* no selector took them: give them back */
    }
    return took;
}

/* ---- matching ----------------------------------------------------------- */

static int cls_has(const struct css_elem *e, const char *want, int wlen)
{
    if (!e->cls || e->cls_len <= 0 || wlen <= 0) return 0;
    int i = 0;
    while (i < e->cls_len) {
        while (i < e->cls_len && is_ws(e->cls[i])) i++;
        int st = i;
        while (i < e->cls_len && !is_ws(e->cls[i])) i++;
        if (i - st == wlen) {
            int k = 0;
            while (k < wlen && lower((unsigned char)e->cls[st + k]) ==
                               lower((unsigned char)want[k])) k++;
            if (k == wlen) return 1;
        }
    }
    return 0;
}

static int comp_matches(const struct comp *c, const struct css_elem *e)
{
    if (c->tag_len) {
        if (!e->tag || e->tag_len != c->tag_len) return 0;
        for (int i = 0; i < c->tag_len; i++)
            if (lower((unsigned char)e->tag[i]) != lower((unsigned char)arena[c->tag + i]))
                return 0;
    }
    if (c->id_len) {
        if (!e->id || e->id_len != c->id_len) return 0;
        for (int i = 0; i < c->id_len; i++)
            if (e->id[i] != arena[c->id + i]) return 0;
    }
    if (c->cls_len  && !cls_has(e, &arena[c->cls],  c->cls_len))  return 0;
    if (c->cls2_len && !cls_has(e, &arena[c->cls2], c->cls2_len)) return 0;
    return 1;
}

/* Right to left: the last compound must match the element itself, then each
 * earlier compound must match SOME ancestor, nearest first. Because descendant
 * is the only combinator, a greedy walk is exact - there is no case where
 * skipping a matching ancestor would have let a later compound match. */
static int sel_matches(const struct sel *sl, const struct css_elem *path, int n)
{
    if (n <= 0) return 0;
    if (!comp_matches(&sl->c[sl->n - 1], &path[n - 1])) return 0;
    int ci = sl->n - 2, pi = n - 2;
    while (ci >= 0) {
        if (pi < 0) return 0;
        if (comp_matches(&sl->c[ci], &path[pi])) { ci--; }
        pi--;
    }
    return 1;
}

/* ---- the cascade -------------------------------------------------------- */

static int resolve(int v, int unit, int parent)
{
    if (unit == U_PX)  return v;
    if (unit == U_EM)  return parent * v / 100;
    return parent * v / 100;                       /* U_PCT, same arithmetic */
}

static void apply(struct css_style *o, const struct decl *d, int parent_size)
{
    switch (d->prop) {
    case P_COLOR:   o->rgb = d->v;       o->has |= CSS_P_COLOR;   break;
    case P_BG:      o->bg = d->v;        o->has |= CSS_P_BG;      break;
    case P_SIZE:    o->size = resolve(d->v, d->unit, parent_size);
                    if (o->size < 1) o->size = 1;
                    o->has |= CSS_P_SIZE;   break;
    case P_WEIGHT:  o->bold = d->v;      o->has |= CSS_P_WEIGHT;  break;
    case P_STYLE:   o->italic = d->v;    o->has |= CSS_P_STYLE;   break;
    case P_FAMILY:  o->mono = d->v;      o->has |= CSS_P_FAMILY;  break;
    case P_ALIGN:   o->align = d->v;     o->has |= CSS_P_ALIGN;   break;
    case P_DECOR:   o->underline = d->v; o->has |= CSS_P_DECOR;   break;
    case P_DISPLAY: o->display = d->v;   o->has |= CSS_P_DISPLAY; break;
    case P_MT: o->margin_t = resolve(d->v, d->unit, parent_size); o->has |= CSS_P_MARGIN_T; break;
    case P_MB: o->margin_b = resolve(d->v, d->unit, parent_size); o->has |= CSS_P_MARGIN_B; break;
    case P_ML: o->margin_l = resolve(d->v, d->unit, parent_size); o->has |= CSS_P_MARGIN_L; break;
    case P_MR: o->margin_r = resolve(d->v, d->unit, parent_size); o->has |= CSS_P_MARGIN_R; break;
    case P_PT: o->pad_t = resolve(d->v, d->unit, parent_size); o->has |= CSS_P_PAD_T; break;
    case P_PB: o->pad_b = resolve(d->v, d->unit, parent_size); o->has |= CSS_P_PAD_B; break;
    case P_PL: o->pad_l = resolve(d->v, d->unit, parent_size); o->has |= CSS_P_PAD_L; break;
    case P_PR: o->pad_r = resolve(d->v, d->unit, parent_size); o->has |= CSS_P_PAD_R; break;
    default: break;
    }
}

void css_compute(const struct css_elem *path, int n, int parent_size,
                 const char *inline_style, int inline_len,
                 struct css_style *out)
{
    if (!out) return;
    if (parent_size <= 0) parent_size = 16;

    /* Selection sort by (specificity, source order) WITHOUT a scratch array:
     * repeatedly find the lowest-ranked selector that still matches and has
     * not been applied, and apply it. MAX_SELS is 384 and a document has a
     * handful of matching rules, so this is n_matched passes over the sheet
     * rather than a sort of the whole sheet - and it needs no memory, which
     * is the constraint that decides it. */
    long prev_key = -1;
    for (;;) {
        long best = -1;
        int  bi = -1;
        for (int i = 0; i < nsels; i++) {
            long key = (long)sels[i].spec * 100000L + sels[i].order;
            if (key <= prev_key) continue;
            if (best >= 0 && key >= best) continue;
            if (!sel_matches(&sels[i], path, n)) continue;
            best = key; bi = i;
        }
        if (bi < 0) break;
        for (int k = 0; k < sels[bi].ndecl; k++)
            apply(out, &decls[sels[bi].decl0 + k], parent_size);
        prev_key = best;
    }

    /* the style= attribute, above every rule regardless of specificity */
    if (inline_style && inline_len > 0) {
        int d0 = ndecls, j = 0;
        while (j < inline_len) {
            int ns = j;
            while (j < inline_len && inline_style[j] != ':' && inline_style[j] != ';') j++;
            if (j >= inline_len || inline_style[j] == ';') { j++; continue; }
            int ne = j;
            j++;
            int vs = j;
            while (j < inline_len && inline_style[j] != ';') j++;
            parse_decl(inline_style + ns, ne - ns, inline_style + vs, j - vs);
            if (j < inline_len) j++;
        }
        for (int k = d0; k < ndecls; k++) apply(out, &decls[k], parent_size);
        ndecls = d0;              /* inline declarations are not kept */
    }
}
