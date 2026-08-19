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

/* MAX_RULES IS NOT ENFORCED ANYWHERE and never was - `nrules` is only a
 * counter, and MAX_SELS is the ceiling that actually refuses work. Left as the
 * documented intent rather than deleted, and named here so the next reader
 * does not spend an afternoon looking for the check. */
#define MAX_RULES  192          /* a rule is one selector + its block      */

/* MAX_SELS, MAX_DECLS AND ARENA NOW LIVE IN css.h AND THE STORAGE IS THE
 * CALLER'S. See the block above css_set_arena there for the measurement that
 * forced it: at 384 selectors Wikipedia's skin yielded 235 rules with
 * css_overflowed() set, and at 4096 it yields 375 with it clear. 4096 * 112
 * bytes cannot be BSS in this kernel, so it is not BSS any more.
 *
 * MAX_DECLS WAS 1024 AND THE BOX PROPERTIES MADE THAT TOO SMALL, measured
 * rather than guessed. Every shorthand is expanded at parse time, so one
 * ordinary modern rule
 *
 *   .card { display:flex; gap:8px; padding:16px; border:1px solid #ccc;
 *           border-radius:8px; width:100%; max-width:960px; margin:0 auto }
 *
 * is 1+2+4+5+1+1+1+4 = 19 declarations, not 8. At 1024 that is 54 rules,
 * against MAX_SELS of 384 - the declaration array would have become the limit
 * that bites first, and it bites by REFUSING the rest of the sheet, so a page
 * would silently lose its footer's styling. 3072 keeps the two arrays at
 * roughly the same ceiling (384 selectors * ~8 declarations).
 *
 * struct decl was reordered (v first) so it is 8 bytes rather than 12, which
 * is why the array could triple for 12 KB. That reasoning still holds; only
 * the ceiling moved. At CSS_MAX_SELS 4096 the same ratio gives 32768
 * declarations, and the arena scales with it - every selector part is interned
 * separately (there is no dedup), so the arena is a function of the selector
 * count and not of the sheet's size. */
#define MAX_SELS   CSS_MAX_SELS
#define MAX_DECLS  CSS_MAX_DECLS
#define MAX_COMP   6            /* compound selectors per selector         */
#define ARENA      CSS_ARENA

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

/* v first: with prop and unit trailing, the struct is 8 bytes instead of the
 * 12 it was when a 1-byte field led. That is 4 bytes * MAX_DECLS of BSS for a
 * field reorder, which is why the array could triple and still only cost 12 KB
 * more. */
struct decl {
    int v;                      /* value, pre-decoded                      */
    unsigned char prop;         /* P_* below                               */
    unsigned char unit;         /* U_* below, or CSS_TRACK_* for P_GRID_TRACK */
};

/* properties, as a dense index rather than the sparse CSS_P_ / CSS_Q_ bit.
 * P_COUNT is checked against the width of struct decl.prop below - the whole
 * scheme rests on this list staying under 256, and the day it does not the
 * truncation would be silent. */
enum { P_COLOR = 1, P_BG, P_SIZE, P_WEIGHT, P_STYLE, P_FAMILY, P_ALIGN,
       P_DECOR, P_DISPLAY, P_MT, P_MB, P_ML, P_MR, P_PT, P_PB, P_PL, P_PR,
       /* the box */
       P_WIDTH, P_HEIGHT, P_MIN_W, P_MAX_W, P_MIN_H, P_MAX_H, P_BOXSIZE,
       P_BT, P_BR, P_BB, P_BL, P_BRGB, P_RADIUS,
       /* out of flow */
       P_FLOAT, P_CLEAR, P_POSITION, P_TOP, P_RIGHT, P_BOTTOM, P_LEFT,
       P_OVERFLOW,
       /* flex */
       P_FLEX_DIR, P_FLEX_WRAP, P_JUSTIFY, P_ALIGN_IT, P_ALIGN_SELF,
       P_GAP_ROW, P_GAP_COL, P_GROW, P_SHRINK, P_BASIS,
       /* grid: a track list is not one value, so it is emitted as a CLEAR
        * followed by one TRACK declaration per track. That keeps the cascade
        * unchanged - a later rule's CLEAR wipes an earlier rule's tracks
        * exactly the way a later `color` overwrites an earlier one. */
       P_GRID_CLEAR, P_GRID_TRACK,
       P_COUNT };                             /* 51 as of this change */

_Static_assert(P_COUNT <= 256, "struct decl.prop is one byte");
_Static_assert(MAX_DECLS <= 65535, "struct sel.decl0 is an unsigned short");

/* U_AUTO IS A UNIT, NOT A VALUE, and that is deliberate. `auto` reaches the
 * computed style as CSS_AUTO, but carrying it as a unit means the sentinel is
 * produced in exactly one place (resolve) rather than by every parser that
 * might otherwise write CSS_AUTO into a decl's `v` by hand. */
enum { U_PX = 0, U_EM, U_PCT, U_AUTO };

/* CLAMPS, so that no product in resolve() can overflow an int. A length is
 * capped at 100000 px (an order of magnitude past any screen), a percentage at
 * 10000 (100x), and em at 10000 hundredths (100em). resolve multiplies a
 * clamped parent (<= 100000) by a clamped relative value (<= 10000), which is
 * 1e9 - inside INT_MAX with room. Without these `width: 99999999999999px` is
 * signed overflow, which is undefined behaviour and not merely a big number. */
#define LEN_MAX  100000
#define REL_MAX  10000

/* ---- the storage, which this file does not own ----------------------------
 * See css.h. The literals there are the caller's only way to size a region for
 * private structs, and these two asserts are what stop them drifting: add a
 * field to struct sel and the build stops HERE, rather than handing the engine
 * an array a quarter short and losing a page's footer to an overflow flag. */
_Static_assert(sizeof(struct sel)  == CSS_SEL_BYTES,
               "CSS_SEL_BYTES in css.h no longer matches struct sel - "
               "every caller sizing a region from it is now short");
_Static_assert(sizeof(struct decl) == CSS_DECL_BYTES,
               "CSS_DECL_BYTES in css.h no longer matches struct decl");

static struct sel  *sels;
static struct decl *decls;
static char  *arena;
static int   max_sels, max_decls, arena_size;
static int   nsels, ndecls, aused, nrules, overflow, order_seq;

void css_set_arena(void *s, int ms, void *d, int md, char *a, int abytes)
{
    sels       = (struct sel *)s;
    max_sels   = (s && ms > 0) ? ms : 0;
    decls      = (struct decl *)d;
    max_decls  = (d && md > 0) ? md : 0;
    arena      = a;
    arena_size = (a && abytes > 0) ? abytes : 0;
    css_reset();
}

int css_sel_cap(void)     { return max_sels; }
int css_decl_cap(void)    { return max_decls; }
int css_arena_cap(void)   { return arena_size; }

void css_reset(void)
{
    nsels = ndecls = aused = nrules = overflow = order_seq = 0;
}

int css_rules(void)       { return nrules; }
int css_sels(void)        { return nsels; }
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
 * see css.h, where that ceiling is now stated as the real limit rather than
 * "ARENA is well under that". "The day someone raises it is the day the
 * truncation would go silent" was written here, and it was right:
 *
 * A FULL ARENA REFUSED SELECTORS WITHOUT SETTING `overflow`, AND THAT WAS THE
 * ONE PATH THAT DID. parse_comp turns a -1 from here into `return 0`, and
 * parse_sel turns that into "drop this selector" - the SAME return value it
 * uses for `a:hover`, which is a deliberate refusal that must not raise the
 * flag. So the two were indistinguishable, and a sheet that ran out of arena
 * was silently truncated while css_overflowed() said 0.
 *
 * Measured on the sheet csstest builds for exactly this case: 3,743 rules in,
 * arena 65,535 of 65,536, 2,185 selectors taken, 1,558 refused, overflow 0.
 * The gate did not catch it because at MAX_SELS 384 the SELECTOR array always
 * filled first and set the flag - so "a sheet that exhausts the arena reports
 * overflow" passed while testing the other array. Raising MAX_SELS is what
 * made the arena the binding limit and the silence visible.
 *
 * The flag belongs HERE rather than in parse_comp because this is the function
 * that knows the difference between "no room" and "not supported". */
static int intern(const char *s, int len)
{
    if (len < 0) len = 0;
    if (len > 255) len = 255;              /* no selector part is longer   */
    if (aused + len + 1 > arena_size || aused + len + 1 > 65535) {
        overflow = 1;
        return -1;
    }
    int off = aused;
    for (int i = 0; i < len; i++) arena[aused++] = s[i];
    arena[aused++] = 0;
    return off;
}

/* CLAMPS RATHER THAN WRAPS. `width: 99999999999999px` is a plausible line in a
 * hostile sheet and the old `v = v * 10 + d` on it is signed overflow - which
 * is undefined behaviour, so the answer is not "a big wrong number", it is
 * whatever the optimiser decides. It still CONSUMES every digit, because the
 * caller's position has to end up past the number either way. */
static int digits(const char *s, int len, int *i, int *out)
{
    int v = 0, got = 0;
    while (*i < len && s[*i] >= '0' && s[*i] <= '9') {
        if (v < 100000000) v = v * 10 + (s[*i] - '0');
        (*i)++; got = 1;
    }
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

/* a length: number plus optional unit. Returns 0 when it is not a length.
 *
 * `*ip` comes in at the first character to look at and goes out PAST the unit,
 * which is what lets a single-value property tell `12px` from `12px solid red`
 * - the second is malformed and must be dropped rather than read as 12px. The
 * old signature could not say how far it got, which is right for the margin
 * shorthand (that one is walking tokens it split itself) and wrong for
 * anything taking one value. */
static int parse_len_at(const char *s, int len, int *ip, int *val, int *unit)
{
    int i = *ip, neg = 0;
    while (i < len && is_ws(s[i])) i++;
    if (i < len && (s[i] == '-' || s[i] == '+')) { neg = (s[i] == '-'); i++; }
    int whole = 0;
    if (!digits(s, len, &i, &whole)) return 0;
    if (whole > 1000000) whole = 1000000;      /* keeps whole*100 in an int */
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

    /* clamp per unit, so resolve()'s multiply cannot overflow - see LEN_MAX */
    int cap = (*unit == U_PX) ? LEN_MAX : REL_MAX;
    if (*val > cap) *val = cap;

    if (neg) *val = -(*val);
    *ip = i;
    return 1;
}

static int parse_len(const char *s, int len, int *val, int *unit)
{
    int i = 0;
    return parse_len_at(s, len, &i, val, unit);
}

/* ---- @media -----------------------------------------------------------------
 * MEASURED, on the English Wikipedia article's skin: 103,793 bytes across 64
 * blocks in 16 distinct conditions - 38% of the stylesheet - was being skipped
 * wholesale. Skipping was the right call while nothing could evaluate a
 * condition, because a @media print block applied unconditionally styles the
 * screen with the print sheet. It is the wrong call once the conditions can be
 * read, and every one of Wikipedia's sixteen is the same simple shape:
 * `screen and (min-width:640px)`.
 *
 * EVALUATED AT PARSE TIME, and that is the honest limitation of this design.
 * A sheet is parsed once; re-evaluating on a window resize would mean
 * re-parsing, and an external sheet's text is not kept after it is interned -
 * so it would mean re-FETCHING. A resize therefore does not re-run the
 * queries. That is a real limit and it is a far smaller one than discarding
 * 38% of every modern stylesheet.
 *
 * ANYTHING NOT UNDERSTOOD DOES NOT MATCH. `calc()`, `not`, `hover`,
 * `prefers-*`, `orientation` - all refuse the block, which is exactly the
 * behaviour that shipped before. Widening what matches can only ever be a
 * deliberate act, never an accident of parsing. */
static int viewport_w;

void css_viewport(int w) { viewport_w = w > 0 ? w : 0; }
int  css_viewport_get(void) { return viewport_w; }

/* one comma-free query: `screen and (min-width:640px)` */
static int media_query_ok(const char *s, int len)
{
    int i = 0, saw_type = 0, ok = 1;
    while (i < len) {
        while (i < len && is_ws(s[i])) i++;
        if (i >= len) break;

        if (s[i] == '(') {
            int j = ++i;
            while (j < len && s[j] != ')') j++;
            /* feature : value */
            int c = i;
            while (c < j && s[c] != ':') c++;
            if (c >= j) return 0;                  /* a bare feature: refuse */
            const char *f = s + i;
            int flen = c - i;
            f = trim_cs(f, &flen);
            const char *v = s + c + 1;
            int vlen = j - c - 1;
            v = trim_cs(v, &vlen);
            int val, unit;
            if (!parse_len(v, vlen, &val, &unit) || unit != U_PX) return 0;
            if (!viewport_w) return 0;             /* no width known: refuse */
            if (ieq(f, flen, "min-width")) { if (viewport_w < val) ok = 0; }
            else if (ieq(f, flen, "max-width")) { if (viewport_w > val) ok = 0; }
            else return 0;                         /* a feature we cannot judge */
            i = (j < len) ? j + 1 : len;
            continue;
        }

        int t0 = i;
        while (i < len && !is_ws(s[i]) && s[i] != '(') i++;
        int tlen = i - t0;
        if (ieq(s + t0, tlen, "and")) continue;
        if (ieq(s + t0, tlen, "only")) continue;
        if (ieq(s + t0, tlen, "screen") || ieq(s + t0, tlen, "all")) { saw_type = 1; continue; }
        /* `not`, `print`, `speech`, and anything else - refuse the block */
        return 0;
    }
    (void)saw_type;
    return ok;
}

/* the whole condition: comma-separated queries, any one of which may match */
static int media_matches(const char *s, int len)
{
    int i = 0;
    while (i <= len) {
        int j = i;
        while (j < len && s[j] != ',') j++;
        if (j > i && media_query_ok(s + i, j - i)) return 1;
        if (j >= len) break;
        i = j + 1;
    }
    return 0;
}

/* ONE value and nothing after it: `auto` when the property allows it, or a
 * length that consumes the whole string. Everything added in this change goes
 * through here rather than through parse_len, because `width: 1px solid red`
 * must be dropped and `width: calc(100% - 20px)` must be dropped, not
 * half-parsed into a number that then moves pixels. */
static int len_one(const char *v, int vl, int *val, int *unit, int allow_auto)
{
    v = trim_cs(v, &vl);
    if (vl <= 0) return 0;
    if (allow_auto && ieq(v, vl, "auto")) { *val = 0; *unit = U_AUTO; return 1; }
    int i = 0;
    if (!parse_len_at(v, vl, &i, val, unit)) return 0;
    while (i < vl && (is_ws(v[i]) || at_comment(v, vl, i))) {
        if (at_comment(v, vl, i)) i = skip_comment(v, vl, i);
        else i++;
    }
    return i >= vl;
}

/* A UNITLESS number, in HUNDREDTHS, because this kernel has no floating point
 * on any path and `flex-grow: 0.5` is real CSS. 100 means 1. A unit of any
 * kind makes it not a number, which is how the `flex` shorthand tells
 * `flex: 2 3` (grow 2, shrink 3) from `flex: 2 3px` (grow 2, basis 3px). */
static int num100(const char *v, int vl, int *out)
{
    v = trim_cs(v, &vl);
    if (vl <= 0) return 0;
    int i = 0, neg = 0;
    if (v[i] == '-' || v[i] == '+') { neg = (v[i] == '-'); i++; }
    int whole = 0;
    int got = digits(v, vl, &i, &whole);
    if (!got && !(i < vl && v[i] == '.')) return 0;      /* `.5` is a number */
    if (whole > REL_MAX) whole = REL_MAX;
    int frac = 0, fd = 0;
    if (i < vl && v[i] == '.') {
        i++;
        while (i < vl && v[i] >= '0' && v[i] <= '9' && fd < 2) {
            frac = frac * 10 + (v[i] - '0'); i++; fd++;
        }
        while (i < vl && v[i] >= '0' && v[i] <= '9') i++;
    }
    while (fd < 2) { frac *= 10; fd++; }
    while (i < vl && is_ws(v[i])) i++;
    if (i < vl) return 0;                     /* a unit, or junk: not a number */
    *out = whole * 100 + frac;
    if (neg) *out = -(*out);
    return 1;
}

/* Split a value into whitespace-separated tokens, NOT splitting inside
 * parentheses: `border: 1px solid rgb(1, 2, 3)` is three tokens, and a splitter
 * that made it five would reject the declaration for having too many.
 *
 * Returns the TOTAL count even when that exceeds `max`, so `flex: 1 2 3 4 5`
 * can be refused for being five rather than silently read as its first three.
 * Only the first `max` are stored. */
struct tok { const char *s; int len; };

static int tokenise(const char *v, int vl, struct tok *t, int max)
{
    int i = 0, n = 0;
    while (i < vl) {
        while (i < vl && (is_ws(v[i]) || at_comment(v, vl, i))) {
            if (at_comment(v, vl, i)) i = skip_comment(v, vl, i);
            else i++;
        }
        if (i >= vl) break;
        int st = i, depth = 0;
        while (i < vl && (depth > 0 || !is_ws(v[i]))) {
            if (v[i] == '(') depth++;
            else if (v[i] == ')' && depth > 0) depth--;
            i++;
        }
        if (n < max) { t[n].s = v + st; t[n].len = i - st; }
        if (n < 1000000) n++;                 /* cannot wrap on any real input */
    }
    return n;
}

static void add_decl(int prop, int v, int unit)
{
    if (ndecls >= max_decls) { overflow = 1; return; }
    decls[ndecls].prop = (unsigned char)prop;
    decls[ndecls].v = v;
    decls[ndecls].unit = (unsigned char)unit;
    ndecls++;
}

/* ---- the box -----------------------------------------------------------
 * width, height, the four min/max constraints, box-sizing and overflow.
 * Returns 1 when the name was one of these, whether or not the VALUE parsed -
 * "recognised and refused" and "not a property we support" must not be the
 * same answer, or a bad value would fall through and be tried as something
 * else. */
static int decl_box(const char *n, int nl, const char *v, int vl)
{
    static const struct { const char *name; int p; } sz[] = {
        { "width", P_WIDTH }, { "height", P_HEIGHT },
        { "min-width", P_MIN_W }, { "max-width", P_MAX_W },
        { "min-height", P_MIN_H }, { "max-height", P_MAX_H }, { 0, 0 }
    };
    for (int i = 0; sz[i].name; i++) {
        if (!ieq(n, nl, sz[i].name)) continue;
        int val, unit;
        /* `none` is the initial value of max-width/max-height and is spelled
         * CSS_AUTO in css.h, which is the same "there is no constraint" that
         * `auto` means on width. */
        if (ieq(v, vl, "none") || ieq(v, vl, "max-content") ||
            ieq(v, vl, "min-content") || ieq(v, vl, "fit-content")) {
            add_decl(sz[i].p, 0, U_AUTO);
            return 1;
        }
        if (!len_one(v, vl, &val, &unit, 1)) return 1;
        if (unit != U_AUTO && val < 0) return 1;   /* negative sizes are invalid */
        add_decl(sz[i].p, val, unit);
        return 1;
    }
    if (ieq(n, nl, "box-sizing")) {
        if (ieq(v, vl, "border-box"))       add_decl(P_BOXSIZE, CSS_BOX_BORDER, U_PX);
        else if (ieq(v, vl, "content-box")) add_decl(P_BOXSIZE, CSS_BOX_CONTENT, U_PX);
        return 1;
    }
    /* ONE overflow FIELD FOR BOTH AXES - css.h has `int overflow`, not a pair.
     * So overflow-x and overflow-y both write it and the last one in the
     * cascade wins, and the two-value `overflow: hidden auto` keeps only the
     * x value. That is a simplification, not an oversight: naming it here
     * because a reader who assumes per-axis clipping will be wrong. */
    if (ieq(n, nl, "overflow") || ieq(n, nl, "overflow-x") || ieq(n, nl, "overflow-y")) {
        struct tok t[2];
        int cnt = tokenise(v, vl, t, 2);
        if (cnt < 1 || cnt > 2) return 1;
        int o;
        if (ieq(t[0].s, t[0].len, "visible"))     o = CSS_OVER_VISIBLE;
        else if (ieq(t[0].s, t[0].len, "hidden") ||
                 ieq(t[0].s, t[0].len, "clip"))   o = CSS_OVER_HIDDEN;
        else if (ieq(t[0].s, t[0].len, "scroll") ||
                 ieq(t[0].s, t[0].len, "auto"))   o = CSS_OVER_SCROLL;
        else return 1;
        add_decl(P_OVERFLOW, o, U_PX);
        return 1;
    }
    return 0;
}

/* ---- borders -------------------------------------------------------------
 * css.h carries four widths, ONE colour and no style at all. Two consequences
 * a reader has to know:
 *
 *   - border-left-color and border-color write the same field, so a box with
 *     four differently coloured edges gets the last one on all four.
 *   - THE WIDTH CARRIES THE STYLE. Real CSS draws nothing when border-style is
 *     `none`, whatever the width says; with no style field the only way to
 *     honour `border-style: none` is to compute the width to 0, which is what
 *     happens here. The converse is not honoured: `border-width: 2px` with no
 *     style at all draws 2px here and nothing in a real browser. That is the
 *     direction that loses a border rather than inventing one.
 */
static int border_style(const char *s, int len, int *draws)
{
    if (ieq(s, len, "none") || ieq(s, len, "hidden")) { *draws = 0; return 1; }
    if (ieq(s, len, "solid")  || ieq(s, len, "dotted") || ieq(s, len, "dashed") ||
        ieq(s, len, "double") || ieq(s, len, "groove") || ieq(s, len, "ridge")  ||
        ieq(s, len, "inset")  || ieq(s, len, "outset")) { *draws = 1; return 1; }
    return 0;
}

static int border_width_kw(const char *s, int len, int *px)
{
    if (ieq(s, len, "thin"))   { *px = 1; return 1; }
    if (ieq(s, len, "medium")) { *px = 3; return 1; }
    if (ieq(s, len, "thick"))  { *px = 5; return 1; }
    return 0;
}

static const int BSIDE[4] = { P_BT, P_BR, P_BB, P_BL };

/* `1px solid #ccc` in any order, any part omitted. `sides` is a bitmask of the
 * four BSIDE entries so the same code serves `border` and `border-left`. */
static void border_short(const char *v, int vl, int sides)
{
    struct tok t[4];
    int cnt = tokenise(v, vl, t, 4);
    if (cnt < 1 || cnt > 3) return;      /* the shorthand takes at most three */
    int have_w = 0, w = 0, wu = U_PX;
    int have_s = 0, draws = 1;
    int have_c = 0, col = -1;
    for (int i = 0; i < cnt; i++) {
        int tmp, tu;
        if (!have_s && border_style(t[i].s, t[i].len, &draws)) { have_s = 1; continue; }
        if (!have_w && border_width_kw(t[i].s, t[i].len, &tmp)) {
            w = tmp; wu = U_PX; have_w = 1; continue;
        }
        if (!have_w && len_one(t[i].s, t[i].len, &tmp, &tu, 0) && tmp >= 0) {
            w = tmp; wu = tu; have_w = 1; continue;
        }
        if (!have_c) {
            int c = parse_colour(t[i].s, t[i].len);
            if (c >= 0) { col = c; have_c = 1; continue; }
        }
        return;                          /* a token nothing recognised */
    }
    if (!have_w) { w = 3; wu = U_PX; }   /* the shorthand resets width to medium */
    if (have_s && !draws) { w = 0; wu = U_PX; }
    for (int i = 0; i < 4; i++)
        if (sides & (1 << i)) add_decl(BSIDE[i], w, wu);
    /* the shorthand resets the colour too, and -1 is how css.h spells "the
     * author gave none, use the text colour" */
    add_decl(P_BRGB, have_c ? col : -1, U_PX);
}

static int decl_border(const char *n, int nl, const char *v, int vl)
{
    static const struct { const char *name; int sides; } side[] = {
        { "top", 1 }, { "right", 2 }, { "bottom", 4 }, { "left", 8 }, { 0, 0 }
    };
    int val, unit;

    if (ieq(n, nl, "border")) { border_short(v, vl, 15); return 1; }

    if (ieq(n, nl, "border-color")) {
        int c = parse_colour(v, vl);
        if (c >= 0) add_decl(P_BRGB, c, U_PX);
        return 1;
    }
    if (ieq(n, nl, "border-radius")) {
        /* one value. The `8px / 4px` elliptical form and the four-corner form
         * both take the first value rather than being refused - a box with one
         * rounded corner drawn on all four is closer to the page than a box
         * with none. */
        struct tok t[1];
        int cnt = tokenise(v, vl, t, 1);
        if (cnt < 1) return 1;
        if (!len_one(t[0].s, t[0].len, &val, &unit, 0) || val < 0) return 1;
        add_decl(P_RADIUS, val, unit);
        return 1;
    }
    if (ieq(n, nl, "border-width") || ieq(n, nl, "border-style")) {
        int is_style = ieq(n, nl, "border-style");
        struct tok t[4];
        int cnt = tokenise(v, vl, t, 4);
        if (cnt < 1 || cnt > 4) return 1;
        int vv[4], uu[4];
        for (int i = 0; i < cnt; i++) {
            if (is_style) {
                int draws;
                if (!border_style(t[i].s, t[i].len, &draws)) return 1;
                /* only `none`/`hidden` can be expressed without a style field:
                 * a real style leaves the width where the cascade put it */
                if (draws) return 1;
                vv[i] = 0; uu[i] = U_PX;
            } else if (border_width_kw(t[i].s, t[i].len, &vv[i])) {
                uu[i] = U_PX;
            } else if (!len_one(t[i].s, t[i].len, &vv[i], &uu[i], 0) || vv[i] < 0) {
                return 1;
            }
        }
        int idx[4] = { 0, cnt > 1 ? 1 : 0, cnt > 2 ? 2 : 0, cnt > 3 ? 3 : (cnt > 1 ? 1 : 0) };
        for (int i = 0; i < 4; i++) add_decl(BSIDE[i], vv[idx[i]], uu[idx[i]]);
        return 1;
    }
    for (int i = 0; side[i].name; i++) {
        char buf[24];
        int bl = 0;
        buf[bl++] = 'b'; buf[bl++] = 'o'; buf[bl++] = 'r'; buf[bl++] = 'd';
        buf[bl++] = 'e'; buf[bl++] = 'r'; buf[bl++] = '-';
        for (const char *p = side[i].name; *p; p++) buf[bl++] = *p;
        buf[bl] = 0;
        if (ieq(n, nl, buf)) { border_short(v, vl, side[i].sides); return 1; }

        int base = bl;
        buf[bl++] = '-'; buf[bl++] = 'w'; buf[bl++] = 'i'; buf[bl++] = 'd';
        buf[bl++] = 't'; buf[bl++] = 'h'; buf[bl] = 0;
        if (ieq(n, nl, buf)) {
            if (border_width_kw(v, vl, &val)) { add_decl(BSIDE[i], val, U_PX); return 1; }
            if (len_one(v, vl, &val, &unit, 0) && val >= 0) add_decl(BSIDE[i], val, unit);
            return 1;
        }
        bl = base;
        buf[bl++] = '-'; buf[bl++] = 'c'; buf[bl++] = 'o'; buf[bl++] = 'l';
        buf[bl++] = 'o'; buf[bl++] = 'r'; buf[bl] = 0;
        if (ieq(n, nl, buf)) {
            int c = parse_colour(v, vl);
            if (c >= 0) add_decl(P_BRGB, c, U_PX);
            return 1;
        }
        bl = base;
        buf[bl++] = '-'; buf[bl++] = 's'; buf[bl++] = 't'; buf[bl++] = 'y';
        buf[bl++] = 'l'; buf[bl++] = 'e'; buf[bl] = 0;
        if (ieq(n, nl, buf)) {
            int draws;
            if (border_style(v, vl, &draws) && !draws) add_decl(BSIDE[i], 0, U_PX);
            return 1;
        }
    }
    return 0;
}

/* ---- out of flow: float, clear, position and the four offsets ----------- */
static int decl_pos(const char *n, int nl, const char *v, int vl)
{
    if (ieq(n, nl, "float")) {
        if (ieq(v, vl, "none"))       add_decl(P_FLOAT, CSS_FLOAT_NONE, U_PX);
        else if (ieq(v, vl, "left"))  add_decl(P_FLOAT, CSS_FLOAT_LEFT, U_PX);
        else if (ieq(v, vl, "right")) add_decl(P_FLOAT, CSS_FLOAT_RIGHT, U_PX);
        return 1;
    }
    if (ieq(n, nl, "clear")) {
        if (ieq(v, vl, "none"))       add_decl(P_CLEAR, CSS_CLEAR_NONE, U_PX);
        else if (ieq(v, vl, "left"))  add_decl(P_CLEAR, CSS_CLEAR_LEFT, U_PX);
        else if (ieq(v, vl, "right")) add_decl(P_CLEAR, CSS_CLEAR_RIGHT, U_PX);
        else if (ieq(v, vl, "both"))  add_decl(P_CLEAR, CSS_CLEAR_BOTH, U_PX);
        return 1;
    }
    if (ieq(n, nl, "position")) {
        if (ieq(v, vl, "static"))        add_decl(P_POSITION, CSS_POS_STATIC, U_PX);
        else if (ieq(v, vl, "relative")) add_decl(P_POSITION, CSS_POS_RELATIVE, U_PX);
        else if (ieq(v, vl, "absolute")) add_decl(P_POSITION, CSS_POS_ABSOLUTE, U_PX);
        else if (ieq(v, vl, "fixed"))    add_decl(P_POSITION, CSS_POS_FIXED, U_PX);
        else if (ieq(v, vl, "sticky"))   add_decl(P_POSITION, CSS_POS_STICKY, U_PX);
        return 1;
    }
    static const struct { const char *name; int p; } off[] = {
        { "top", P_TOP }, { "right", P_RIGHT },
        { "bottom", P_BOTTOM }, { "left", P_LEFT }, { 0, 0 }
    };
    for (int i = 0; off[i].name; i++) {
        if (!ieq(n, nl, off[i].name)) continue;
        int val, unit;
        /* negative offsets are legal and used constantly, so no sign check */
        if (len_one(v, vl, &val, &unit, 1)) add_decl(off[i].p, val, unit);
        return 1;
    }
    return 0;
}

/* ---- flex ---------------------------------------------------------------- */

/* justify-content's full value set. align-items and align-self use the subset
 * css.h names; nothing enforces that, because a document asking for
 * `align-items: space-between` is asking for something meaningless rather than
 * something dangerous. */
static int kw_justify(const char *s, int len, int *out)
{
    if (ieq(s, len, "flex-start") || ieq(s, len, "start") || ieq(s, len, "left"))
        *out = CSS_J_START;
    else if (ieq(s, len, "flex-end") || ieq(s, len, "end") || ieq(s, len, "right"))
        *out = CSS_J_END;
    else if (ieq(s, len, "center"))        *out = CSS_J_CENTER;
    else if (ieq(s, len, "space-between")) *out = CSS_J_BETWEEN;
    else if (ieq(s, len, "space-around"))  *out = CSS_J_AROUND;
    else if (ieq(s, len, "space-evenly"))  *out = CSS_J_EVENLY;
    else if (ieq(s, len, "stretch"))       *out = CSS_J_STRETCH;
    else if (ieq(s, len, "baseline") || ieq(s, len, "first baseline"))
        *out = CSS_J_BASELINE;
    else return 0;
    return 1;
}

static int kw_flex_dir(const char *s, int len, int *out)
{
    if (ieq(s, len, "row"))                 *out = CSS_ROW;
    else if (ieq(s, len, "row-reverse"))    *out = CSS_ROW_REVERSE;
    else if (ieq(s, len, "column"))         *out = CSS_COLUMN;
    else if (ieq(s, len, "column-reverse")) *out = CSS_COLUMN_REVERSE;
    else return 0;
    return 1;
}

static int kw_flex_wrap(const char *s, int len, int *out)
{
    if (ieq(s, len, "nowrap"))            *out = CSS_NOWRAP;
    else if (ieq(s, len, "wrap"))         *out = CSS_WRAP;
    else if (ieq(s, len, "wrap-reverse")) *out = CSS_WRAP_REVERSE;
    else return 0;
    return 1;
}

/* THE `flex` SHORTHAND, which is where implementations go wrong.
 *   flex: 1        -> 1 1 0%      flex: none    -> 0 0 auto
 *   flex: auto     -> 1 1 auto    flex: initial -> 0 1 auto
 *   flex: <n>      -> n 1 0%      flex: 30px    -> 1 1 30px
 * A bare number is grow then shrink; anything with a unit, or `auto`, is the
 * basis. More than three values is invalid and the whole thing is dropped. */
static void flex_short(const char *v, int vl)
{
    struct tok t[4];
    int cnt = tokenise(v, vl, t, 4);
    if (cnt < 1 || cnt > 3) return;

    if (cnt == 1) {
        int g = -1;
        if (ieq(t[0].s, t[0].len, "none"))         g = 0;
        else if (ieq(t[0].s, t[0].len, "auto"))    g = 1;
        else if (ieq(t[0].s, t[0].len, "initial")) g = 2;
        if (g >= 0) {
            add_decl(P_GROW,   g == 1 ? 100 : 0, U_PX);
            add_decl(P_SHRINK, g == 0 ? 0 : 100, U_PX);
            add_decl(P_BASIS,  0, U_AUTO);
            return;
        }
    }

    int grow = -1, shrink = -1, bv = 0, bu = -1;
    for (int i = 0; i < cnt; i++) {
        int num;
        if (num100(t[i].s, t[i].len, &num)) {
            if (num < 0) return;                  /* negative grow/shrink */
            if (grow < 0) grow = num;
            else if (shrink < 0) shrink = num;
            else return;                          /* three bare numbers */
            continue;
        }
        if (bu < 0 && ieq(t[i].s, t[i].len, "content")) { bv = 0; bu = U_AUTO; continue; }
        int val, unit;
        if (bu < 0 && len_one(t[i].s, t[i].len, &val, &unit, 1) &&
            (unit == U_AUTO || val >= 0)) { bv = val; bu = unit; continue; }
        return;                                   /* unrecognised token */
    }
    if (grow < 0 && bu < 0) return;
    if (grow < 0) grow = 100;                     /* `flex: 30px` is `1 1 30px` */
    if (shrink < 0) shrink = 100;
    if (bu < 0) { bv = 0; bu = U_PX; }            /* `flex: 1` is `1 1 0%`      */
    add_decl(P_GROW,   grow,  U_PX);
    add_decl(P_SHRINK, shrink, U_PX);
    add_decl(P_BASIS,  bv, bu);
}

static int decl_flex(const char *n, int nl, const char *v, int vl)
{
    int val, unit, kv;

    if (ieq(n, nl, "flex")) { flex_short(v, vl); return 1; }

    if (ieq(n, nl, "flex-direction")) {
        if (kw_flex_dir(v, vl, &kv)) add_decl(P_FLEX_DIR, kv, U_PX);
        return 1;
    }
    if (ieq(n, nl, "flex-wrap")) {
        if (kw_flex_wrap(v, vl, &kv)) add_decl(P_FLEX_WRAP, kv, U_PX);
        return 1;
    }
    if (ieq(n, nl, "flex-flow")) {
        struct tok t[2];
        int cnt = tokenise(v, vl, t, 2);
        if (cnt < 1 || cnt > 2) return 1;
        int dir = -1, wrap = -1;
        for (int i = 0; i < cnt; i++) {
            if (dir < 0 && kw_flex_dir(t[i].s, t[i].len, &kv))  { dir = kv;  continue; }
            if (wrap < 0 && kw_flex_wrap(t[i].s, t[i].len, &kv)) { wrap = kv; continue; }
            return 1;                              /* malformed: drop it whole */
        }
        if (dir >= 0)  add_decl(P_FLEX_DIR, dir, U_PX);
        if (wrap >= 0) add_decl(P_FLEX_WRAP, wrap, U_PX);
        return 1;
    }
    if (ieq(n, nl, "justify-content")) {
        /* `normal` is start on the main axis and stretch on the cross one, so
         * it cannot live in the shared keyword table */
        if (ieq(v, vl, "normal"))        add_decl(P_JUSTIFY, CSS_J_START, U_PX);
        else if (kw_justify(v, vl, &kv)) add_decl(P_JUSTIFY, kv, U_PX);
        return 1;
    }
    if (ieq(n, nl, "align-items")) {
        if (ieq(v, vl, "normal"))        add_decl(P_ALIGN_IT, CSS_J_STRETCH, U_PX);
        else if (kw_justify(v, vl, &kv)) add_decl(P_ALIGN_IT, kv, U_PX);
        return 1;
    }
    if (ieq(n, nl, "align-self")) {
        if (ieq(v, vl, "auto"))          add_decl(P_ALIGN_SELF, CSS_J_AUTO, U_PX);
        else if (ieq(v, vl, "normal"))   add_decl(P_ALIGN_SELF, CSS_J_STRETCH, U_PX);
        else if (kw_justify(v, vl, &kv)) add_decl(P_ALIGN_SELF, kv, U_PX);
        return 1;
    }
    /* align-content IS RECOGNISED AND DROPPED, and that is a defect in css.h
     * rather than a choice here: the struct has justify, align_items and
     * align_self and no align_content. Writing it into align_items would be
     * actively wrong - align-content distributes the LINES of a wrapped
     * container and has no effect at all on a single-line one, so aliasing it
     * would turn the common `align-content: center` on an unwrapped row into a
     * cross-axis move that a real browser does not make. Consumed here so it
     * cannot fall through and be mistaken for something else. */
    if (ieq(n, nl, "align-content")) return 1;

    if (ieq(n, nl, "flex-grow") || ieq(n, nl, "flex-shrink")) {
        int num;
        if (!num100(v, vl, &num) || num < 0) return 1;
        add_decl(ieq(n, nl, "flex-grow") ? P_GROW : P_SHRINK, num, U_PX);
        return 1;
    }
    if (ieq(n, nl, "flex-basis")) {
        if (ieq(v, vl, "content")) { add_decl(P_BASIS, 0, U_AUTO); return 1; }
        if (len_one(v, vl, &val, &unit, 1) && (unit == U_AUTO || val >= 0))
            add_decl(P_BASIS, val, unit);
        return 1;
    }

    /* gap. `gap: 8px 16px` is row then column, which is the one thing about it
     * that is easy to get backwards. grid-gap is the old spelling and is still
     * all over the web. */
    if (ieq(n, nl, "gap") || ieq(n, nl, "grid-gap")) {
        struct tok t[3];
        int cnt = tokenise(v, vl, t, 3);
        if (cnt < 1 || cnt > 2) return 1;
        int rv, ru, cv, cu;
        if (!len_one(t[0].s, t[0].len, &rv, &ru, 0) || rv < 0) return 1;
        if (cnt == 2) {
            if (!len_one(t[1].s, t[1].len, &cv, &cu, 0) || cv < 0) return 1;
        } else { cv = rv; cu = ru; }
        add_decl(P_GAP_ROW, rv, ru);
        add_decl(P_GAP_COL, cv, cu);
        return 1;
    }
    if (ieq(n, nl, "row-gap") || ieq(n, nl, "grid-row-gap")) {
        if (len_one(v, vl, &val, &unit, 0) && val >= 0) add_decl(P_GAP_ROW, val, unit);
        return 1;
    }
    if (ieq(n, nl, "column-gap") || ieq(n, nl, "grid-column-gap")) {
        if (len_one(v, vl, &val, &unit, 0) && val >= 0) add_decl(P_GAP_COL, val, unit);
        return 1;
    }
    return 0;
}

/* ---- grid ----------------------------------------------------------------
 * One track: a length, a percentage, `fr` or `auto`. `fr` has to be checked
 * before the length parser gets it, because `1fr` would otherwise read as 1px
 * with a stray `fr` after it - the same half-parse this file refuses
 * everywhere else. */
static int track_one(const char *s, int len, int *val, int *unit)
{
    if (ieq(s, len, "auto") || ieq(s, len, "min-content") ||
        ieq(s, len, "max-content")) { *val = 0; *unit = CSS_TRACK_AUTO; return 1; }
    if (len > 2 && lower((unsigned char)s[len - 2]) == 'f' &&
                   lower((unsigned char)s[len - 1]) == 'r') {
        int i = 0, v, u;
        if (!parse_len_at(s, len - 2, &i, &v, &u) || u != U_PX || v < 0) return 0;
        while (i < len - 2 && is_ws(s[i])) i++;
        if (i != len - 2) return 0;
        *val = v; *unit = CSS_TRACK_FR;            /* 2.5fr truncates to 2fr */
        return 1;
    }
    int v, u;
    if (!len_one(s, len, &v, &u, 0) || v < 0) return 0;
    if (u == U_PCT) { *val = v; *unit = CSS_TRACK_PCT; return 1; }
    if (u == U_PX)  { *val = v; *unit = CSS_TRACK_PX;  return 1; }
    return 0;               /* an em track: refused rather than mis-resolved */
}

static int grid_add(int *vals, int *units, int *n, int val, int unit)
{
    if (*n >= CSS_GRID_MAX) { overflow = 1; return 0; }   /* refuse and record */
    vals[*n] = val; units[*n] = unit; (*n)++;
    return 1;
}

/* A track list, with repeat() expanded HERE rather than carried into the
 * computed style - `repeat(3, 1fr)` is how people actually write a three
 * column grid, and layout.c should never have to know the difference.
 * Returns 0 for a malformed list, in which case NOTHING has been emitted:
 * the caller builds into a local array and only emits on success, so
 * `repeat(3` unterminated leaves no half-grid behind. */
static int grid_list(const char *v, int vl, int *vals, int *units, int *n, int depth)
{
    int i = 0;
    while (i < vl) {
        while (i < vl && (is_ws(v[i]) || at_comment(v, vl, i))) {
            if (at_comment(v, vl, i)) i = skip_comment(v, vl, i);
            else i++;
        }
        if (i >= vl) break;
        int st = i, dep = 0;
        while (i < vl && (dep > 0 || !is_ws(v[i]))) {
            if (v[i] == '(') dep++;
            else if (v[i] == ')' && dep > 0) dep--;
            i++;
        }
        if (dep != 0) return 0;                     /* `repeat(3` never closes */
        const char *t = v + st;
        int tl = i - st;
        if (tl > 7 && ieq(t, 7, "repeat(")) {
            if (depth > 0) return 0;                /* nested repeat: refused  */
            if (t[tl - 1] != ')') return 0;
            const char *in = t + 7;
            int inl = tl - 8, k = 0, times = 0;
            while (k < inl && is_ws(in[k])) k++;
            if (!digits(in, inl, &k, &times)) return 0;
            while (k < inl && is_ws(in[k])) k++;
            if (k >= inl || in[k] != ',') return 0;
            k++;
            if (times <= 0) return 0;
            /* repeat(999999, 1fr) does not loop 999999 times: the count is cut
             * to the ceiling first, and grid_add refuses past it anyway */
            if (times > CSS_GRID_MAX) { times = CSS_GRID_MAX; overflow = 1; }
            for (int r = 0; r < times; r++)
                if (!grid_list(in + k, inl - k, vals, units, n, depth + 1)) return 0;
            continue;
        }
        int val, unit;
        if (!track_one(t, tl, &val, &unit)) return 0;
        if (!grid_add(vals, units, n, val, unit)) break;   /* full: truncate  */
    }
    return 1;
}

/* the columns half of `grid-template` / `grid`: the text after the ONE
 * top-level slash, ignoring slashes inside parentheses and quotes (a
 * `grid-template-areas` string can contain one). Zero or several slashes and
 * the shorthand is ignored whole, which is the cheap-or-nothing the brief
 * asked for - the rows and areas halves are not parsed at all. */
static int slash_tail(const char *v, int vl, int *off)
{
    int depth = 0, found = -1, count = 0;
    char q = 0;
    for (int i = 0; i < vl; i++) {
        char c = v[i];
        if (q) { if (c == q) q = 0; continue; }
        if (c == '"' || c == '\'') { q = c; continue; }
        if (c == '(') depth++;
        else if (c == ')') { if (depth > 0) depth--; }
        else if (c == '/' && depth == 0) { found = i; count++; }
    }
    if (count != 1) return 0;
    *off = found + 1;
    return 1;
}

static int decl_grid(const char *n, int nl, const char *v, int vl)
{
    int vals[CSS_GRID_MAX], units[CSS_GRID_MAX], cnt = 0;

    if (ieq(n, nl, "grid-template-columns")) {
        if (ieq(v, vl, "none")) { add_decl(P_GRID_CLEAR, 0, U_PX); return 1; }
        if (!grid_list(v, vl, vals, units, &cnt, 0) || cnt <= 0) return 1;
        add_decl(P_GRID_CLEAR, 0, U_PX);
        for (int i = 0; i < cnt; i++) add_decl(P_GRID_TRACK, vals[i], units[i]);
        return 1;
    }
    if (ieq(n, nl, "grid-template") || ieq(n, nl, "grid")) {
        int off;
        if (!slash_tail(v, vl, &off)) return 1;
        if (!grid_list(v + off, vl - off, vals, units, &cnt, 0) || cnt <= 0) return 1;
        add_decl(P_GRID_CLEAR, 0, U_PX);
        for (int i = 0; i < cnt; i++) add_decl(P_GRID_TRACK, vals[i], units[i]);
        return 1;
    }
    return 0;
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
        /* the fallback stays "unknown becomes block": `display: table`,
         * `list-item`, `flow-root` and every value invented since all lay out
         * closer to a block than to anything else here, and a document that
         * says `display: table` must not vanish. */
        if (ieq(v, vl, "none"))               add_decl(P_DISPLAY, CSS_DISP_NONE, U_PX);
        else if (ieq(v, vl, "inline"))        add_decl(P_DISPLAY, CSS_DISP_INLINE, U_PX);
        else if (ieq(v, vl, "inline-block"))  add_decl(P_DISPLAY, CSS_DISP_INLINE_BLOCK, U_PX);
        else if (ieq(v, vl, "flex"))          add_decl(P_DISPLAY, CSS_DISP_FLEX, U_PX);
        else if (ieq(v, vl, "inline-flex"))   add_decl(P_DISPLAY, CSS_DISP_INLINE_FLEX, U_PX);
        /* inline-grid folds onto grid: css.h has no separate constant for it,
         * and a grid that is inline-level differs only in how its PARENT
         * places it, which layout.c decides from the parent's display. */
        else if (ieq(v, vl, "grid") || ieq(v, vl, "inline-grid"))
                                              add_decl(P_DISPLAY, CSS_DISP_GRID, U_PX);
        else                                  add_decl(P_DISPLAY, CSS_DISP_BLOCK, U_PX);
        return;
    }

    /* margin / padding, longhand and shorthand. `auto` is legal on margin and
     * not on padding, which is the whole reason the table carries a flag. */
    static const struct { const char *name; int p; int au; } one[] = {
        { "margin-top", P_MT, 1 }, { "margin-bottom", P_MB, 1 },
        { "margin-left", P_ML, 1 }, { "margin-right", P_MR, 1 },
        { "padding-top", P_PT, 0 }, { "padding-bottom", P_PB, 0 },
        { "padding-left", P_PL, 0 }, { "padding-right", P_PR, 0 }, { 0, 0, 0 }
    };
    for (int i = 0; one[i].name; i++)
        if (ieq(n, nl, one[i].name)) {
            if (len_one(v, vl, &val, &unit, one[i].au)) add_decl(one[i].p, val, unit);
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
            if (is_m && ieq(v + st, i - st, "auto")) {
                /* `margin: 0 auto` IS the page-centring idiom, and auto is not
                 * a length - it has to survive to layout.c as CSS_AUTO. It used
                 * to be flattened to 0 here, which centred nothing. */
                vals[k] = 0; units[k] = U_AUTO;
            } else if (!parse_len(v + st, i - st, &vals[k], &units[k])) {
                /* a token that is neither: 0, rather than aborting the whole
                 * declaration. That is what shipped and what `margin: 0 auto`
                 * relied on before U_AUTO existed; keeping it means a padding
                 * with a stray `auto` still gets its other three sides. */
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

    if (decl_box(n, nl, v, vl))    return;
    if (decl_border(n, nl, v, vl)) return;
    if (decl_pos(n, nl, v, vl))    return;
    if (decl_flex(n, nl, v, vl))   return;
    if (decl_grid(n, nl, v, vl))   return;
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
    if (nsels >= max_sels) { overflow = 1; return 0; }
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
    /* NO STORAGE, NO RULES, AND SAY SO. Every array below refuses on its own
     * with `overflow` set, so this is not needed for safety - it is needed for
     * HONESTY: a sheet with no rules in it would otherwise return 0 with
     * overflow clear, which reads as "that stylesheet was empty" rather than
     * "nobody called css_set_arena". Those two need to look different. */
    if (!sels || !decls || !arena) { overflow = 1; return 0; }
    int i = 0, took = 0, media_depth = 0;

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
            /* @media WHOSE CONDITION MATCHES IS NOT AN AT-RULE TO SKIP - its
             * rules are the page's rules, at the same specificity, in source
             * order. Stepping INTO the block and letting the ordinary loop
             * carry on is the whole implementation; `media_depth` only exists
             * so the block's closing brace is recognised as such rather than
             * as a stray one. Everything else at-shaped - @font-face, @import,
             * @supports, @keyframes - is skipped exactly as before. */
            if (i + 6 <= len && ieq(src + i, 6, "@media")) {
                int c = i + 6;
                while (c < len && src[c] != '{' && src[c] != ';' && src[c] != '}') c++;
                if (c < len && src[c] == '{' &&
                    media_matches(src + i + 6, c - (i + 6))) {
                    i = c + 1;
                    media_depth++;
                    continue;
                }
            }
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
        if (src[i] == '}') {
            /* the close of a @media we stepped into, or a genuinely stray
             * brace. Both are recovered the same way; the counter is what
             * stops a matched block's own closer being mistaken for damage. */
            if (media_depth > 0) media_depth--;
            i++;
            continue;
        }
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
            if (nsels >= max_sels) { overflow = 1; break; }
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

/* THE ONE PLACE CSS_AUTO IS PRODUCED. css.h picked INT_MIN for it precisely
 * because no arithmetic here can land on it: `parent` is clamped to
 * [0, LEN_MAX] and `v` to at most REL_MAX by parse_len, so the product is at
 * most 1e9 and the result is clamped to +-LEN_MAX before it leaves. Negating
 * the sentinel is the only way to produce it by accident, and nothing here
 * negates a resolved value. */
static int resolve(int v, int unit, int parent)
{
    if (unit == U_AUTO) return CSS_AUTO;
    if (unit == U_PX)   return v;
    if (parent < 0) parent = 0;
    if (parent > LEN_MAX) parent = LEN_MAX;
    int r = parent * v / 100;                 /* U_EM and U_PCT: one scale */
    if (r >  LEN_MAX) r =  LEN_MAX;
    if (r < -LEN_MAX) r = -LEN_MAX;
    return r;
}

/* THE SPLIT IS BY UNIT, NOT BY PROPERTY, and getting that wrong is a bug the
 * gate caught rather than a subtlety avoided: `min-height: 2em` against a 16px
 * font in a 400px block resolved to 800 when the property alone chose the
 * parent. An em is a share of the FONT wherever it appears; only a percentage
 * on a box property is a share of the containing block. */
static int resolve_box(int v, int unit, int ps, int pw)
{
    return resolve(v, unit, unit == U_EM ? ps : pw);
}

/* `ps` resolves em everywhere and % on the font properties; `pw` resolves % on
 * the box properties. THEY ARE DIFFERENT NUMBERS and used to be the same one,
 * which made `width: 50%` compute to 8px against a 16px parent.
 *
 * MARGIN AND PADDING PERCENTAGES STILL RESOLVE AGAINST ps. That is wrong per
 * the spec - every percentage margin and padding, including the vertical ones,
 * is a share of the containing block's WIDTH - and it is what shipped. Fixing
 * it changes the geometry of every page that uses a percentage margin, which
 * is a separate change with its own regression risk and its own gate; doing it
 * inside this one would make a flexbox bug and a margin bug indistinguishable. */
static void apply(struct css_style *o, const struct decl *d, int ps, int pw)
{
    switch (d->prop) {
    case P_COLOR:   o->rgb = d->v;       o->has |= CSS_P_COLOR;   break;
    case P_BG:      o->bg = d->v;        o->has |= CSS_P_BG;      break;
    case P_SIZE:    o->size = resolve(d->v, d->unit, ps);
                    if (o->size < 1) o->size = 1;
                    o->has |= CSS_P_SIZE;   break;
    case P_WEIGHT:  o->bold = d->v;      o->has |= CSS_P_WEIGHT;  break;
    case P_STYLE:   o->italic = d->v;    o->has |= CSS_P_STYLE;   break;
    case P_FAMILY:  o->mono = d->v;      o->has |= CSS_P_FAMILY;  break;
    case P_ALIGN:   o->align = d->v;     o->has |= CSS_P_ALIGN;   break;
    case P_DECOR:   o->underline = d->v; o->has |= CSS_P_DECOR;   break;
    case P_DISPLAY: o->display = d->v;   o->has |= CSS_P_DISPLAY; break;
    case P_MT: o->margin_t = resolve(d->v, d->unit, ps); o->has |= CSS_P_MARGIN_T; break;
    case P_MB: o->margin_b = resolve(d->v, d->unit, ps); o->has |= CSS_P_MARGIN_B; break;
    case P_ML: o->margin_l = resolve(d->v, d->unit, ps); o->has |= CSS_P_MARGIN_L; break;
    case P_MR: o->margin_r = resolve(d->v, d->unit, ps); o->has |= CSS_P_MARGIN_R; break;
    case P_PT: o->pad_t = resolve(d->v, d->unit, ps); o->has |= CSS_P_PAD_T; break;
    case P_PB: o->pad_b = resolve(d->v, d->unit, ps); o->has |= CSS_P_PAD_B; break;
    case P_PL: o->pad_l = resolve(d->v, d->unit, ps); o->has |= CSS_P_PAD_L; break;
    case P_PR: o->pad_r = resolve(d->v, d->unit, ps); o->has |= CSS_P_PAD_R; break;

    /* the box: every percentage below is a share of the containing block */
    case P_WIDTH:  o->width  = resolve_box(d->v, d->unit, ps, pw); o->has2 |= CSS_Q_WIDTH;  break;
    case P_HEIGHT: o->height = resolve_box(d->v, d->unit, ps, pw); o->has2 |= CSS_Q_HEIGHT; break;
    case P_MIN_W:  o->min_w  = resolve_box(d->v, d->unit, ps, pw); o->has2 |= CSS_Q_MIN_W;  break;
    case P_MAX_W:  o->max_w  = resolve_box(d->v, d->unit, ps, pw); o->has2 |= CSS_Q_MAX_W;  break;
    case P_MIN_H:  o->min_h  = resolve_box(d->v, d->unit, ps, pw); o->has2 |= CSS_Q_MIN_H;  break;
    case P_MAX_H:  o->max_h  = resolve_box(d->v, d->unit, ps, pw); o->has2 |= CSS_Q_MAX_H;  break;
    case P_BOXSIZE: o->box_sizing = d->v; o->has2 |= CSS_Q_BOXSIZING; break;
    case P_BT: o->border_t = resolve_box(d->v, d->unit, ps, pw); o->has2 |= CSS_Q_BORDER_T; break;
    case P_BR: o->border_r = resolve_box(d->v, d->unit, ps, pw); o->has2 |= CSS_Q_BORDER_R; break;
    case P_BB: o->border_b = resolve_box(d->v, d->unit, ps, pw); o->has2 |= CSS_Q_BORDER_B; break;
    case P_BL: o->border_l = resolve_box(d->v, d->unit, ps, pw); o->has2 |= CSS_Q_BORDER_L; break;
    case P_BRGB:   o->border_rgb = d->v; o->has2 |= CSS_Q_BORDER_RGB; break;
    case P_RADIUS: o->radius = resolve_box(d->v, d->unit, ps, pw); o->has2 |= CSS_Q_RADIUS; break;

    /* out of flow */
    case P_FLOAT:    o->floatv = d->v;   o->has2 |= CSS_Q_FLOAT;    break;
    case P_CLEAR:    o->clearv = d->v;   o->has2 |= CSS_Q_CLEAR;    break;
    case P_POSITION: o->position = d->v; o->has2 |= CSS_Q_POSITION; break;
    case P_TOP:    o->top    = resolve_box(d->v, d->unit, ps, pw); o->has2 |= CSS_Q_TOP;    break;
    case P_RIGHT:  o->right  = resolve_box(d->v, d->unit, ps, pw); o->has2 |= CSS_Q_RIGHT;  break;
    case P_BOTTOM: o->bottom = resolve_box(d->v, d->unit, ps, pw); o->has2 |= CSS_Q_BOTTOM; break;
    case P_LEFT:   o->left   = resolve_box(d->v, d->unit, ps, pw); o->has2 |= CSS_Q_LEFT;   break;
    case P_OVERFLOW: o->overflow = d->v; o->has2 |= CSS_Q_OVERFLOW; break;

    /* flex. GROW AND SHRINK ARE ALREADY HUNDREDTHS and must not go through
     * resolve - 100 is 1, and dividing it by anything is how a flex line ends
     * up allocating zero. */
    case P_FLEX_DIR:   o->flex_dir = d->v;    o->has2 |= CSS_Q_FLEX_DIR;   break;
    case P_FLEX_WRAP:  o->flex_wrap = d->v;   o->has2 |= CSS_Q_FLEX_WRAP;  break;
    case P_JUSTIFY:    o->justify = d->v;     o->has2 |= CSS_Q_JUSTIFY;    break;
    case P_ALIGN_IT:   o->align_items = d->v; o->has2 |= CSS_Q_ALIGN_IT;   break;
    case P_ALIGN_SELF: o->align_self = d->v;  o->has2 |= CSS_Q_ALIGN_SELF; break;
    case P_GAP_ROW: o->gap_row = resolve_box(d->v, d->unit, ps, pw); o->has2 |= CSS_Q_GAP_ROW; break;
    case P_GAP_COL: o->gap_col = resolve_box(d->v, d->unit, ps, pw); o->has2 |= CSS_Q_GAP_COL; break;
    case P_GROW:   o->grow = d->v;   o->has2 |= CSS_Q_GROW;   break;
    case P_SHRINK: o->shrink = d->v; o->has2 |= CSS_Q_SHRINK; break;
    case P_BASIS:  o->basis = resolve_box(d->v, d->unit, ps, pw); o->has2 |= CSS_Q_BASIS; break;

    /* grid. d->unit here is a CSS_TRACK_*, not a U_*, and the two namespaces
     * are deliberately separate: `fr` is a share of the leftover space, so it
     * cannot be resolved to pixels until layout.c knows the container's width.
     * A percentage track is left unresolved for the same reason - css.h asks
     * for the unit, so it gets the unit. */
    case P_GRID_CLEAR: o->n_grid_cols = 0; o->has2 |= CSS_Q_GRID_COLS; break;
    case P_GRID_TRACK:
        if (o->n_grid_cols < CSS_GRID_MAX) {
            o->grid_col_v[o->n_grid_cols] = d->v;
            o->grid_col_unit[o->n_grid_cols] = d->unit;
            o->n_grid_cols++;
        }
        break;
    default: break;
    }
}

/* The initial value of every property, in one place, because a struct with
 * forty fields is exactly how one field ends up garbage on layout.c's path and
 * zero on the harness's. `width` is CSS_AUTO and NOT 0 - that is the whole
 * difference between "as wide as it needs" and "invisible". */
void css_style_init(struct css_style *out, int size)
{
    if (!out) return;
    if (size <= 0) size = 16;

    out->has = 0; out->has2 = 0;
    out->rgb = -1; out->bg = -1;
    out->size = size;
    out->bold = out->italic = out->mono = out->underline = 0;
    out->align = CSS_ALIGN_LEFT;
    out->display = CSS_DISP_INLINE;
    out->margin_t = out->margin_b = out->margin_l = out->margin_r = 0;
    out->pad_t = out->pad_b = out->pad_l = out->pad_r = 0;

    out->width = out->height = CSS_AUTO;
    out->min_w = 0; out->max_w = CSS_AUTO;
    out->min_h = 0; out->max_h = CSS_AUTO;
    out->box_sizing = CSS_BOX_CONTENT;
    out->border_t = out->border_r = out->border_b = out->border_l = 0;
    out->border_rgb = -1;
    out->radius = 0;

    out->floatv = CSS_FLOAT_NONE;
    out->clearv = CSS_CLEAR_NONE;
    out->position = CSS_POS_STATIC;
    out->top = out->right = out->bottom = out->left = CSS_AUTO;
    out->overflow = CSS_OVER_VISIBLE;

    out->flex_dir = CSS_ROW;
    out->flex_wrap = CSS_NOWRAP;
    out->justify = CSS_J_START;
    out->align_items = CSS_J_STRETCH;
    out->align_self = CSS_J_AUTO;
    out->gap_row = out->gap_col = 0;
    out->grow = 0; out->shrink = 100;      /* the spec's 0 and 1, in hundredths */
    out->basis = CSS_AUTO;

    out->n_grid_cols = 0;
    for (int i = 0; i < CSS_GRID_MAX; i++) {
        out->grid_col_v[i] = 0;
        out->grid_col_unit[i] = CSS_TRACK_AUTO;
    }
}

void css_compute(const struct css_elem *path, int n,
                 int parent_size, int parent_width,
                 const char *inline_style, int inline_len,
                 struct css_style *out)
{
    if (!out) return;
    if (parent_size <= 0) parent_size = 16;
    if (parent_width < 0) parent_width = 0;   /* 0 is legal: a zero-width box */

    /* Selection sort by (specificity, source order) WITHOUT a scratch array:
     * repeatedly find the lowest-ranked selector that still matches and has
     * not been applied, and apply it. The sheet holds thousands and a document has a
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
            apply(out, &decls[sels[bi].decl0 + k], parent_size, parent_width);
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
        for (int k = d0; k < ndecls; k++) apply(out, &decls[k], parent_size, parent_width);
        ndecls = d0;              /* inline declarations are not kept */
    }
}
