/* cssstub.c - a deterministic stand-in for css.c, so layout.c stays testable
 * while css.c is being rewritten in the same checkout.
 *
 * WHY THIS EXISTS AND WHAT IT IS NOT. It is not a second CSS engine and it is
 * not a fallback the kernel ever links: `layout.c` is the file under test here,
 * and it talks to css.c through exactly the surface in css.h. A file that is
 * mid-rewrite does not compile, and a layout engine that cannot be built cannot
 * be gated - so the harness gets a second implementation of that surface which
 * is small enough to be obviously right and which never changes under it.
 *
 * The trade is deliberate and it is the same one `fake_measure` in htmltest.c
 * makes: every number this returns can be worked out by hand from the sheet
 * text, so an assertion that fails is a layout bug rather than a cascade bug.
 * When the real css.c lands, htmltest is built against BOTH and must pass
 * against both; anywhere they disagree is a finding, not a nuisance.
 *
 * WHAT IT SUPPORTS, honestly: type / .class / #id / compound selectors, the
 * descendant combinator, comma groups, specificity with source order as the
 * tie-break, the style= attribute above everything, and every property in
 * struct css_style. What it does NOT support: !important, at-rules (skipped),
 * pseudo-classes (the rule is dropped), attribute selectors (dropped), child
 * and sibling combinators (dropped), and more than two classes in one compound.
 * A dropped rule is COUNTED, never half-applied - the same discipline css.c
 * states for its own limits.
 *
 * NO HEAP, like everything it stands in for. Fixed arrays, one string arena.
 */
#include "../../src/web/css.h"

#define S_RULES  192
#define S_DECLS  1024
#define S_ARENA  12288
#define S_COMP   4           /* compound selectors per selector            */
#define S_TRACKS 64          /* grid tracks, pooled across all rules       */

/* properties, dense */
enum {
    P_NONE = 0,
    P_COLOR, P_BG, P_SIZE, P_WEIGHT, P_STYLE, P_FAMILY, P_ALIGN, P_DECOR,
    P_DISPLAY,
    P_MT, P_MB, P_ML, P_MR,
    P_PT, P_PB, P_PL, P_PR,
    P_W, P_H, P_MINW, P_MAXW, P_MINH, P_MAXH,
    P_BOXSIZING,
    P_BT, P_BR_, P_BB, P_BL, P_BRGB, P_RADIUS,
    P_FLOAT, P_CLEAR, P_POSITION, P_TOP, P_RIGHT, P_BOTTOM, P_LEFT, P_OVERFLOW,
    P_FLEXDIR, P_FLEXWRAP, P_JUSTIFY, P_ALIGNIT, P_ALIGNSELF,
    P_GAPROW, P_GAPCOL, P_GROW, P_SHRINK, P_BASIS,
    P_GRID
};

/* units */
enum { U_PX = 0, U_EM, U_PCT, U_KW, U_AUTO };

struct sdecl {
    unsigned short prop;
    unsigned char  unit;
    int v;                    /* px, em-hundredths, percent, or a keyword  */
    unsigned short t0, tn;    /* P_GRID: a slice of the track pool         */
};

struct scomp {
    unsigned short tag, tag_len;
    unsigned short id,  id_len;
    unsigned short c1,  c1_len;
    unsigned short c2,  c2_len;
};

struct srule {
    struct scomp c[S_COMP];
    unsigned char n;
    int spec, order;
    unsigned short d0, dn;
};

static struct srule rules[S_RULES];
static struct sdecl decls[S_DECLS];
static char arena[S_ARENA];
static int  track_v[S_TRACKS], track_u[S_TRACKS];
static int  nrules, ndecls, aused, ntracks, norder, sdropped;

void css_reset(void)
{
    nrules = ndecls = aused = ntracks = norder = sdropped = 0;
}

int css_rules(void)      { return nrules; }
int css_decls(void)      { return ndecls; }
int css_arena_used(void) { return aused; }
int css_overflowed(void) { return sdropped; }

/* ---- tiny character helpers - no libc, same rule as the kernel ---------- */
static int is_sp(char c)  { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f'; }
static int is_dig(char c) { return c >= '0' && c <= '9'; }
static char lower(char c) { return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c; }

static int eq_ci(const char *a, int alen, const char *b)
{
    int i = 0;
    for (; i < alen; i++) { if (!b[i] || lower(a[i]) != lower(b[i])) return 0; }
    return b[i] == 0;
}

static int put(const char *s, int len, unsigned short *off, unsigned short *olen)
{
    if (len < 0) len = 0;
    if (aused + len > S_ARENA) return 0;
    *off = (unsigned short)aused;
    *olen = (unsigned short)len;
    for (int i = 0; i < len; i++) arena[aused++] = s[i];
    return 1;
}

/* ---- values -------------------------------------------------------------- */
static int hexv(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

struct named { const char *name; int rgb; };
static const struct named NAMED[] = {
    { "black", 0x000000 }, { "white", 0xFFFFFF }, { "red", 0xFF0000 },
    { "green", 0x008000 }, { "blue", 0x0000FF }, { "gray", 0x808080 },
    { "grey", 0x808080 }, { "silver", 0xC0C0C0 }, { "yellow", 0xFFFF00 },
    { "navy", 0x000080 }, { "teal", 0x008080 }, { "orange", 0xFFA500 },
    { "transparent", -1 }, { 0, 0 }
};

/* a colour, or -1 when the token is not one */
static int colour_of(const char *s, int len)
{
    if (len >= 4 && s[0] == '#') {
        int d[6], nd = 0;
        for (int i = 1; i < len && nd < 6; i++) {
            int h = hexv(s[i]);
            if (h < 0) return -1;
            d[nd++] = h;
        }
        if (nd == 3) return (d[0] * 17 << 16) | (d[1] * 17 << 8) | (d[2] * 17);
        if (nd == 6) return (d[0] << 20) | (d[1] << 16) | (d[2] << 12) |
                            (d[3] << 8)  | (d[4] << 4)  | d[5];
        return -1;
    }
    for (int i = 0; NAMED[i].name; i++)
        if (eq_ci(s, len, NAMED[i].name)) return NAMED[i].rgb;
    return -1;
}

/* A LENGTH, and `auto` is not one. Returns 1 on success. em is carried as
 * hundredths so `1.5em` survives with no floating point anywhere - the same
 * choice css.h documents for flex-grow. */
static int length_of(const char *s, int len, int *v, int *unit)
{
    if (eq_ci(s, len, "auto")) { *v = 0; *unit = U_AUTO; return 1; }
    int i = 0, neg = 0;
    if (i < len && (s[i] == '-' || s[i] == '+')) { neg = (s[i] == '-'); i++; }
    if (i >= len || (!is_dig(s[i]) && s[i] != '.')) return 0;
    int whole = 0, frac = 0, fdig = 0;
    while (i < len && is_dig(s[i])) { if (whole < 100000) whole = whole * 10 + (s[i] - '0'); i++; }
    if (i < len && s[i] == '.') {
        i++;
        while (i < len && is_dig(s[i])) { if (fdig < 2) { frac = frac * 10 + (s[i] - '0'); fdig++; } i++; }
    }
    while (fdig < 2) { frac *= 10; fdig++; }
    const char *u = s + i;
    int ulen = len - i;
    if (ulen >= 1 && u[0] == '%')                      { *unit = U_PCT; *v = whole; }
    else if (eq_ci(u, ulen, "em") || eq_ci(u, ulen, "rem")) { *unit = U_EM; *v = whole * 100 + frac; }
    else                                                { *unit = U_PX; *v = whole; }
    if (neg) *v = -*v;
    return 1;
}

/* ---- the keyword tables -------------------------------------------------- */
struct kw { const char *name; int v; };
static int kw_of(const char *s, int len, const struct kw *t, int dflt)
{
    for (int i = 0; t[i].name; i++) if (eq_ci(s, len, t[i].name)) return t[i].v;
    return dflt;
}
static const struct kw KW_DISPLAY[] = {
    { "inline", CSS_DISP_INLINE }, { "block", CSS_DISP_BLOCK },
    { "none", CSS_DISP_NONE }, { "inline-block", CSS_DISP_INLINE_BLOCK },
    { "flex", CSS_DISP_FLEX }, { "inline-flex", CSS_DISP_INLINE_FLEX },
    { "grid", CSS_DISP_GRID }, { "inline-grid", CSS_DISP_GRID }, { 0, 0 }
};
static const struct kw KW_ALIGNTXT[] = {
    { "left", CSS_ALIGN_LEFT }, { "center", CSS_ALIGN_CENTER },
    { "centre", CSS_ALIGN_CENTER }, { "right", CSS_ALIGN_RIGHT }, { 0, 0 }
};
static const struct kw KW_FLOAT[] = {
    { "none", CSS_FLOAT_NONE }, { "left", CSS_FLOAT_LEFT },
    { "right", CSS_FLOAT_RIGHT }, { 0, 0 }
};
static const struct kw KW_CLEAR[] = {
    { "none", CSS_CLEAR_NONE }, { "left", CSS_CLEAR_LEFT },
    { "right", CSS_CLEAR_RIGHT }, { "both", CSS_CLEAR_BOTH }, { 0, 0 }
};
static const struct kw KW_POS[] = {
    { "static", CSS_POS_STATIC }, { "relative", CSS_POS_RELATIVE },
    { "absolute", CSS_POS_ABSOLUTE }, { "fixed", CSS_POS_FIXED },
    { "sticky", CSS_POS_STICKY }, { 0, 0 }
};
static const struct kw KW_DIR[] = {
    { "row", CSS_ROW }, { "row-reverse", CSS_ROW_REVERSE },
    { "column", CSS_COLUMN }, { "column-reverse", CSS_COLUMN_REVERSE }, { 0, 0 }
};
static const struct kw KW_WRAP[] = {
    { "nowrap", CSS_NOWRAP }, { "wrap", CSS_WRAP },
    { "wrap-reverse", CSS_WRAP_REVERSE }, { 0, 0 }
};
static const struct kw KW_JUST[] = {
    { "flex-start", CSS_J_START }, { "start", CSS_J_START }, { "left", CSS_J_START },
    { "flex-end", CSS_J_END }, { "end", CSS_J_END }, { "right", CSS_J_END },
    { "center", CSS_J_CENTER }, { "space-between", CSS_J_BETWEEN },
    { "space-around", CSS_J_AROUND }, { "space-evenly", CSS_J_EVENLY },
    { "stretch", CSS_J_STRETCH }, { "baseline", CSS_J_BASELINE },
    { "auto", CSS_J_AUTO }, { "normal", CSS_J_STRETCH }, { 0, 0 }
};
static const struct kw KW_OVER[] = {
    { "visible", CSS_OVER_VISIBLE }, { "hidden", CSS_OVER_HIDDEN },
    { "clip", CSS_OVER_HIDDEN }, { "scroll", CSS_OVER_SCROLL },
    { "auto", CSS_OVER_SCROLL }, { 0, 0 }
};
static const struct kw KW_BOXS[] = {
    { "content-box", CSS_BOX_CONTENT }, { "border-box", CSS_BOX_BORDER }, { 0, 0 }
};

/* ---- declaration parsing ------------------------------------------------- */
static int add_decl(int prop, int v, int unit)
{
    if (ndecls >= S_DECLS) { sdropped++; return 0; }
    decls[ndecls].prop = (unsigned short)prop;
    decls[ndecls].unit = (unsigned char)unit;
    decls[ndecls].v = v;
    decls[ndecls].t0 = decls[ndecls].tn = 0;
    ndecls++;
    return 1;
}

static int add_len(int prop, const char *s, int len)
{
    int v, u;
    if (!length_of(s, len, &v, &u)) return 0;
    return add_decl(prop, v, u);
}

/* split a value into up to 4 space-separated tokens */
static int tokens(const char *s, int len, const char **tv, int *tl, int max)
{
    int n = 0, i = 0;
    while (i < len && n < max) {
        while (i < len && is_sp(s[i])) i++;
        if (i >= len) break;
        int j = i, depth = 0;
        while (j < len && (depth || !is_sp(s[j]))) {
            if (s[j] == '(') depth++;
            else if (s[j] == ')' && depth) depth--;
            j++;
        }
        tv[n] = s + i; tl[n] = j - i; n++;
        i = j;
    }
    return n;
}

static void box4(int pa, int pb, int pl, int pr, const char *s, int len)
{
    const char *tv[4]; int tl[4];
    int n = tokens(s, len, tv, tl, 4);
    if (n == 1)      { add_len(pa, tv[0], tl[0]); add_len(pb, tv[0], tl[0]);
                       add_len(pl, tv[0], tl[0]); add_len(pr, tv[0], tl[0]); }
    else if (n == 2) { add_len(pa, tv[0], tl[0]); add_len(pb, tv[0], tl[0]);
                       add_len(pl, tv[1], tl[1]); add_len(pr, tv[1], tl[1]); }
    else if (n == 3) { add_len(pa, tv[0], tl[0]);
                       add_len(pl, tv[1], tl[1]); add_len(pr, tv[1], tl[1]);
                       add_len(pb, tv[2], tl[2]); }
    else if (n >= 4) { add_len(pa, tv[0], tl[0]); add_len(pr, tv[1], tl[1]);
                       add_len(pb, tv[2], tl[2]); add_len(pl, tv[3], tl[3]); }
}

/* grid-template-columns, with repeat() expanded here exactly as css.h says the
 * real engine expands it - layout.c must never meet a repeat(). */
static void grid_tracks(const char *s, int len)
{
    if (ndecls >= S_DECLS) { sdropped++; return; }
    int t0 = ntracks;
    int i = 0;
    while (i < len) {
        while (i < len && is_sp(s[i])) i++;
        if (i >= len) break;
        int j = i, depth = 0;
        while (j < len && (depth || !is_sp(s[j]))) {
            if (s[j] == '(') depth++;
            else if (s[j] == ')' && depth) depth--;
            j++;
        }
        const char *tok = s + i; int tlen = j - i;
        i = j;
        if (tlen > 7 && eq_ci(tok, 6, "repeat") && tok[6] == '(') {
            /* repeat(N, <tracks>) */
            const char *in = tok + 7; int inlen = tlen - 7;
            if (inlen > 0 && in[inlen - 1] == ')') inlen--;
            int k = 0, cnt = 0;
            while (k < inlen && is_dig(in[k])) { cnt = cnt * 10 + (in[k] - '0'); k++; }
            while (k < inlen && (is_sp(in[k]) || in[k] == ',')) k++;
            if (cnt > CSS_GRID_MAX) cnt = CSS_GRID_MAX;
            for (int rep = 0; rep < cnt; rep++) {
                int p = k;
                while (p < inlen) {
                    while (p < inlen && is_sp(in[p])) p++;
                    if (p >= inlen) break;
                    int q = p;
                    while (q < inlen && !is_sp(in[q])) q++;
                    if (ntracks < S_TRACKS) {
                        int v, u;
                        const char *tt = in + p; int tn2 = q - p;
                        if (tn2 > 2 && eq_ci(tt + tn2 - 2, 2, "fr")) {
                            length_of(tt, tn2 - 2, &v, &u);
                            track_v[ntracks] = v; track_u[ntracks] = CSS_TRACK_FR;
                        } else if (eq_ci(tt, tn2, "auto")) {
                            track_v[ntracks] = 0; track_u[ntracks] = CSS_TRACK_AUTO;
                        } else if (length_of(tt, tn2, &v, &u)) {
                            track_v[ntracks] = v;
                            track_u[ntracks] = (u == U_PCT) ? CSS_TRACK_PCT : CSS_TRACK_PX;
                        } else { track_v[ntracks] = 0; track_u[ntracks] = CSS_TRACK_AUTO; }
                        ntracks++;
                    } else sdropped++;
                    p = q;
                }
            }
            continue;
        }
        if (ntracks < S_TRACKS) {
            int v, u;
            if (tlen > 2 && eq_ci(tok + tlen - 2, 2, "fr")) {
                length_of(tok, tlen - 2, &v, &u);
                track_v[ntracks] = v; track_u[ntracks] = CSS_TRACK_FR;
            } else if (eq_ci(tok, tlen, "auto")) {
                track_v[ntracks] = 0; track_u[ntracks] = CSS_TRACK_AUTO;
            } else if (length_of(tok, tlen, &v, &u)) {
                track_v[ntracks] = v;
                track_u[ntracks] = (u == U_PCT) ? CSS_TRACK_PCT : CSS_TRACK_PX;
            } else { track_v[ntracks] = 0; track_u[ntracks] = CSS_TRACK_AUTO; }
            ntracks++;
        } else sdropped++;
    }
    decls[ndecls].prop = P_GRID;
    decls[ndecls].unit = U_KW;
    decls[ndecls].v = 0;
    decls[ndecls].t0 = (unsigned short)t0;
    decls[ndecls].tn = (unsigned short)(ntracks - t0);
    ndecls++;
}

static void one_decl(const char *p, int plen, const char *v, int vlen)
{
    const char *tv[4]; int tl[4];
    int c;
    if      (eq_ci(p, plen, "color"))            { c = colour_of(v, vlen); add_decl(P_COLOR, c, U_KW); }
    else if (eq_ci(p, plen, "background") ||
             eq_ci(p, plen, "background-color")) { c = colour_of(v, vlen); add_decl(P_BG, c, U_KW); }
    else if (eq_ci(p, plen, "font-size"))        add_len(P_SIZE, v, vlen);
    else if (eq_ci(p, plen, "font-weight"))
        add_decl(P_WEIGHT, (eq_ci(v, vlen, "bold") || eq_ci(v, vlen, "bolder") ||
                            (vlen == 3 && v[0] >= '6')) ? 1 : 0, U_KW);
    else if (eq_ci(p, plen, "font-style"))
        add_decl(P_STYLE, (eq_ci(v, vlen, "italic") || eq_ci(v, vlen, "oblique")) ? 1 : 0, U_KW);
    else if (eq_ci(p, plen, "font-family"))
        add_decl(P_FAMILY, (vlen >= 9 && eq_ci(v, 9, "monospace")) ? 1 : 0, U_KW);
    else if (eq_ci(p, plen, "text-align"))       add_decl(P_ALIGN, kw_of(v, vlen, KW_ALIGNTXT, CSS_ALIGN_LEFT), U_KW);
    else if (eq_ci(p, plen, "text-decoration"))  add_decl(P_DECOR, eq_ci(v, vlen, "none") ? 0 : 1, U_KW);
    else if (eq_ci(p, plen, "display"))          add_decl(P_DISPLAY, kw_of(v, vlen, KW_DISPLAY, CSS_DISP_INLINE), U_KW);
    else if (eq_ci(p, plen, "margin"))           box4(P_MT, P_MB, P_ML, P_MR, v, vlen);
    else if (eq_ci(p, plen, "margin-top"))       add_len(P_MT, v, vlen);
    else if (eq_ci(p, plen, "margin-bottom"))    add_len(P_MB, v, vlen);
    else if (eq_ci(p, plen, "margin-left"))      add_len(P_ML, v, vlen);
    else if (eq_ci(p, plen, "margin-right"))     add_len(P_MR, v, vlen);
    else if (eq_ci(p, plen, "padding"))          box4(P_PT, P_PB, P_PL, P_PR, v, vlen);
    else if (eq_ci(p, plen, "padding-top"))      add_len(P_PT, v, vlen);
    else if (eq_ci(p, plen, "padding-bottom"))   add_len(P_PB, v, vlen);
    else if (eq_ci(p, plen, "padding-left"))     add_len(P_PL, v, vlen);
    else if (eq_ci(p, plen, "padding-right"))    add_len(P_PR, v, vlen);
    else if (eq_ci(p, plen, "width"))            add_len(P_W, v, vlen);
    else if (eq_ci(p, plen, "height"))           add_len(P_H, v, vlen);
    else if (eq_ci(p, plen, "min-width"))        add_len(P_MINW, v, vlen);
    else if (eq_ci(p, plen, "max-width"))        add_len(P_MAXW, v, vlen);
    else if (eq_ci(p, plen, "min-height"))       add_len(P_MINH, v, vlen);
    else if (eq_ci(p, plen, "max-height"))       add_len(P_MAXH, v, vlen);
    else if (eq_ci(p, plen, "box-sizing"))       add_decl(P_BOXSIZING, kw_of(v, vlen, KW_BOXS, CSS_BOX_CONTENT), U_KW);
    else if (eq_ci(p, plen, "border")) {
        int n = tokens(v, vlen, tv, tl, 4);
        for (int i = 0; i < n; i++) {
            int lv, lu;
            if (eq_ci(tv[i], tl[i], "none")) {
                add_decl(P_BT, 0, U_PX); add_decl(P_BR_, 0, U_PX);
                add_decl(P_BB, 0, U_PX); add_decl(P_BL, 0, U_PX);
            } else if ((c = colour_of(tv[i], tl[i])) >= 0) add_decl(P_BRGB, c, U_KW);
            else if (length_of(tv[i], tl[i], &lv, &lu) && lu != U_AUTO) {
                add_decl(P_BT, lv, lu); add_decl(P_BR_, lv, lu);
                add_decl(P_BB, lv, lu); add_decl(P_BL, lv, lu);
            }
        }
    }
    else if (eq_ci(p, plen, "border-width"))         box4(P_BT, P_BB, P_BL, P_BR_, v, vlen);
    else if (eq_ci(p, plen, "border-top-width"))     add_len(P_BT, v, vlen);
    else if (eq_ci(p, plen, "border-right-width"))   add_len(P_BR_, v, vlen);
    else if (eq_ci(p, plen, "border-bottom-width"))  add_len(P_BB, v, vlen);
    else if (eq_ci(p, plen, "border-left-width"))    add_len(P_BL, v, vlen);
    else if (eq_ci(p, plen, "border-color"))         { c = colour_of(v, vlen); add_decl(P_BRGB, c, U_KW); }
    else if (eq_ci(p, plen, "border-radius"))        add_len(P_RADIUS, v, vlen);
    else if (eq_ci(p, plen, "float"))            add_decl(P_FLOAT, kw_of(v, vlen, KW_FLOAT, CSS_FLOAT_NONE), U_KW);
    else if (eq_ci(p, plen, "clear"))            add_decl(P_CLEAR, kw_of(v, vlen, KW_CLEAR, CSS_CLEAR_NONE), U_KW);
    else if (eq_ci(p, plen, "position"))         add_decl(P_POSITION, kw_of(v, vlen, KW_POS, CSS_POS_STATIC), U_KW);
    else if (eq_ci(p, plen, "top"))              add_len(P_TOP, v, vlen);
    else if (eq_ci(p, plen, "right"))            add_len(P_RIGHT, v, vlen);
    else if (eq_ci(p, plen, "bottom"))           add_len(P_BOTTOM, v, vlen);
    else if (eq_ci(p, plen, "left"))             add_len(P_LEFT, v, vlen);
    else if (eq_ci(p, plen, "overflow") ||
             eq_ci(p, plen, "overflow-x") ||
             eq_ci(p, plen, "overflow-y"))       add_decl(P_OVERFLOW, kw_of(v, vlen, KW_OVER, CSS_OVER_VISIBLE), U_KW);
    else if (eq_ci(p, plen, "flex-direction"))   add_decl(P_FLEXDIR, kw_of(v, vlen, KW_DIR, CSS_ROW), U_KW);
    else if (eq_ci(p, plen, "flex-wrap"))        add_decl(P_FLEXWRAP, kw_of(v, vlen, KW_WRAP, CSS_NOWRAP), U_KW);
    else if (eq_ci(p, plen, "justify-content"))  add_decl(P_JUSTIFY, kw_of(v, vlen, KW_JUST, CSS_J_START), U_KW);
    else if (eq_ci(p, plen, "align-items"))      add_decl(P_ALIGNIT, kw_of(v, vlen, KW_JUST, CSS_J_STRETCH), U_KW);
    else if (eq_ci(p, plen, "align-self"))       add_decl(P_ALIGNSELF, kw_of(v, vlen, KW_JUST, CSS_J_AUTO), U_KW);
    else if (eq_ci(p, plen, "gap")) {
        int n = tokens(v, vlen, tv, tl, 4);
        if (n >= 1) add_len(P_GAPROW, tv[0], tl[0]);
        add_len(P_GAPCOL, tv[n >= 2 ? 1 : 0], tl[n >= 2 ? 1 : 0]);
    }
    else if (eq_ci(p, plen, "row-gap"))          add_len(P_GAPROW, v, vlen);
    else if (eq_ci(p, plen, "column-gap"))       add_len(P_GAPCOL, v, vlen);
    /* GROW AND SHRINK ARE HUNDREDTHS - length_of already carries em that way,
     * so a unitless 1 must be scaled by hand or `flex-grow: 1` means 0.01 */
    else if (eq_ci(p, plen, "flex-grow")) {
        int lv, lu; if (length_of(v, vlen, &lv, &lu)) add_decl(P_GROW, lv * 100, U_KW);
    }
    else if (eq_ci(p, plen, "flex-shrink")) {
        int lv, lu; if (length_of(v, vlen, &lv, &lu)) add_decl(P_SHRINK, lv * 100, U_KW);
    }
    else if (eq_ci(p, plen, "flex-basis"))       add_len(P_BASIS, v, vlen);
    else if (eq_ci(p, plen, "flex")) {
        int n = tokens(v, vlen, tv, tl, 4);
        int lv, lu;
        if (n == 1 && eq_ci(tv[0], tl[0], "none")) { add_decl(P_GROW, 0, U_KW); add_decl(P_SHRINK, 0, U_KW); }
        else if (n >= 1) {
            if (length_of(tv[0], tl[0], &lv, &lu)) add_decl(P_GROW, lv * 100, U_KW);
            if (n >= 2 && length_of(tv[1], tl[1], &lv, &lu) && lu != U_AUTO) add_decl(P_SHRINK, lv * 100, U_KW);
            else if (n == 1) add_decl(P_SHRINK, 100, U_KW);
            if (n >= 3) add_len(P_BASIS, tv[2], tl[2]);
            else if (n >= 1) add_decl(P_BASIS, 0, U_PX);
        }
    }
    else if (eq_ci(p, plen, "grid-template-columns")) grid_tracks(v, vlen);
    /* anything else is a property this stub does not carry. Silently ignored
     * rather than counted: an unknown property is not a malformed sheet. */
}

static int parse_block(const char *s, int i, int len, unsigned short *d0, unsigned short *dn)
{
    *d0 = (unsigned short)ndecls;
    while (i < len && s[i] != '}') {
        while (i < len && (is_sp(s[i]) || s[i] == ';')) i++;
        if (i >= len || s[i] == '}') break;
        int ps = i;
        while (i < len && s[i] != ':' && s[i] != ';' && s[i] != '}') i++;
        if (i >= len || s[i] != ':') { while (i < len && s[i] != ';' && s[i] != '}') i++; continue; }
        int pe = i; i++;
        int vs = i;
        while (i < len && s[i] != ';' && s[i] != '}') i++;
        int ve = i;
        while (pe > ps && is_sp(s[pe - 1])) pe--;
        while (vs < ve && is_sp(s[vs])) vs++;
        while (ve > vs && is_sp(s[ve - 1])) ve--;
        if (pe > ps && ve > vs) one_decl(s + ps, pe - ps, s + vs, ve - vs);
    }
    *dn = (unsigned short)(ndecls - *d0);
    return i;
}

/* one compound selector: tag#id.a.b */
static int parse_compound(const char *s, int len, struct scomp *c)
{
    c->tag = c->tag_len = c->id = c->id_len = 0;
    c->c1 = c->c1_len = c->c2 = c->c2_len = 0;
    int i = 0;
    if (i < len && s[i] != '.' && s[i] != '#' && s[i] != '*') {
        int j = i;
        while (j < len && s[j] != '.' && s[j] != '#') j++;
        if (!put(s + i, j - i, &c->tag, &c->tag_len)) return 0;
        i = j;
    } else if (i < len && s[i] == '*') i++;
    while (i < len) {
        char k = s[i++];
        int j = i;
        while (j < len && s[j] != '.' && s[j] != '#') j++;
        if (j == i) return 0;
        if (k == '#') { if (!put(s + i, j - i, &c->id, &c->id_len)) return 0; }
        else if (k == '.') {
            if (!c->c1_len) { if (!put(s + i, j - i, &c->c1, &c->c1_len)) return 0; }
            else if (!c->c2_len) { if (!put(s + i, j - i, &c->c2, &c->c2_len)) return 0; }
            else return 0;         /* three classes: refuse the rule outright */
        } else return 0;
        i = j;
    }
    return 1;
}

static void add_rule(const char *s, int len, unsigned short d0, unsigned short dn)
{
    if (nrules >= S_RULES) { sdropped++; return; }
    struct srule *r = &rules[nrules];
    r->n = 0;
    /* pseudo-classes, attribute selectors and the child/sibling combinators
     * are not carried. REFUSED WHOLE rather than matched approximately - a
     * rule that half-matches is worse than one that does not apply. */
    for (int i = 0; i < len; i++)
        if (s[i] == ':' || s[i] == '[' || s[i] == '>' || s[i] == '+' || s[i] == '~') { sdropped++; return; }
    int i = 0;
    while (i < len) {
        while (i < len && is_sp(s[i])) i++;
        if (i >= len) break;
        int j = i;
        while (j < len && !is_sp(s[j])) j++;
        if (r->n >= S_COMP) { sdropped++; return; }
        if (!parse_compound(s + i, j - i, &r->c[r->n])) { sdropped++; return; }
        r->n++;
        i = j;
    }
    if (!r->n) { sdropped++; return; }
    int spec = 0;
    for (int k = 0; k < r->n; k++) {
        if (r->c[k].id_len) spec += 10000;
        if (r->c[k].c1_len) spec += 100;
        if (r->c[k].c2_len) spec += 100;
        if (r->c[k].tag_len) spec += 1;
    }
    r->spec = spec;
    r->order = norder++;
    r->d0 = d0; r->dn = dn;
    nrules++;
}

int css_add_sheet(const char *src, int len)
{
    if (!src || len <= 0) return 0;
    int taken = 0, i = 0;
    while (i < len) {
        if (is_sp(src[i])) { i++; continue; }
        if (src[i] == '/' && i + 1 < len && src[i + 1] == '*') {
            i += 2;
            while (i + 1 < len && !(src[i] == '*' && src[i + 1] == '/')) i++;
            i += 2;
            continue;
        }
        if (src[i] == '@') {           /* at-rules: skipped to a safe point */
            int depth = 0;
            while (i < len) {
                if (src[i] == '{') depth++;
                else if (src[i] == '}') { depth--; if (depth <= 0) { i++; break; } }
                else if (src[i] == ';' && !depth) { i++; break; }
                i++;
            }
            continue;
        }
        int ss = i;
        while (i < len && src[i] != '{' && src[i] != '}') i++;
        if (i >= len) break;
        if (src[i] == '}') { i++; continue; }
        int se = i; i++;
        unsigned short d0, dn;
        i = parse_block(src, i, len, &d0, &dn);
        if (i < len && src[i] == '}') i++;
        /* comma groups: one rule per selector, all sharing the block */
        int a = ss;
        while (a <= se) {
            int b = a;
            while (b < se && src[b] != ',') b++;
            int x = a, y = b;
            while (x < y && is_sp(src[x])) x++;
            while (y > x && is_sp(src[y - 1])) y--;
            if (y > x) { add_rule(src + x, y - x, d0, dn); taken++; }
            a = b + 1;
        }
    }
    return taken;
}

/* ---- matching ------------------------------------------------------------ */
static int has_class(const struct css_elem *e, const char *c, int clen)
{
    if (!clen || !e->cls || e->cls_len <= 0) return 0;
    int i = 0;
    while (i < e->cls_len) {
        while (i < e->cls_len && is_sp(e->cls[i])) i++;
        int j = i;
        while (j < e->cls_len && !is_sp(e->cls[j])) j++;
        if (j - i == clen) {
            int k = 0;
            while (k < clen && lower(e->cls[i + k]) == lower(c[k])) k++;
            if (k == clen) return 1;
        }
        i = j;
    }
    return 0;
}

static int comp_matches(const struct scomp *c, const struct css_elem *e)
{
    if (c->tag_len) {
        if (e->tag_len != c->tag_len) return 0;
        for (int i = 0; i < c->tag_len; i++)
            if (lower(e->tag[i]) != lower(arena[c->tag + i])) return 0;
    }
    if (c->id_len) {
        if (e->id_len != c->id_len) return 0;
        for (int i = 0; i < c->id_len; i++)
            if (e->id[i] != arena[c->id + i]) return 0;
    }
    if (c->c1_len && !has_class(e, arena + c->c1, c->c1_len)) return 0;
    if (c->c2_len && !has_class(e, arena + c->c2, c->c2_len)) return 0;
    return 1;
}

/* right to left, exactly as css.h describes: the rightmost compound must match
 * the element, the rest must appear among its ancestors in order */
static int rule_matches(const struct srule *r, const struct css_elem *path, int n)
{
    if (n <= 0) return 0;
    int k = r->n - 1;
    if (!comp_matches(&r->c[k], &path[n - 1])) return 0;
    k--;
    int i = n - 2;
    while (k >= 0) {
        if (i < 0) return 0;
        if (comp_matches(&r->c[k], &path[i])) { k--; }
        i--;
    }
    return 1;
}

/* ---- computing ----------------------------------------------------------- */
void css_style_init(struct css_style *o, int size)
{
    if (!o) return;
    o->has = 0; o->has2 = 0;
    o->rgb = -1; o->bg = -1;
    o->size = size > 0 ? size : 16;
    o->bold = o->italic = o->mono = o->underline = 0;
    o->align = CSS_ALIGN_LEFT;
    o->display = CSS_DISP_INLINE;
    o->margin_t = o->margin_b = o->margin_l = o->margin_r = 0;
    o->pad_t = o->pad_b = o->pad_l = o->pad_r = 0;
    o->width = o->height = CSS_AUTO;
    o->min_w = 0; o->max_w = CSS_AUTO;
    o->min_h = 0; o->max_h = CSS_AUTO;
    o->box_sizing = CSS_BOX_CONTENT;
    o->border_t = o->border_r = o->border_b = o->border_l = 0;
    o->border_rgb = -1;
    o->radius = 0;
    o->floatv = CSS_FLOAT_NONE;
    o->clearv = CSS_CLEAR_NONE;
    o->position = CSS_POS_STATIC;
    o->top = o->right = o->bottom = o->left = CSS_AUTO;
    o->overflow = CSS_OVER_VISIBLE;
    o->flex_dir = CSS_ROW;
    o->flex_wrap = CSS_NOWRAP;
    o->justify = CSS_J_START;
    o->align_items = CSS_J_STRETCH;
    o->align_self = CSS_J_AUTO;
    o->gap_row = o->gap_col = 0;
    o->grow = 0; o->shrink = 100;
    o->basis = CSS_AUTO;
    o->n_grid_cols = 0;
    for (int i = 0; i < CSS_GRID_MAX; i++) { o->grid_col_v[i] = 0; o->grid_col_unit[i] = CSS_TRACK_AUTO; }
}

static int resolve(const struct sdecl *d, int psize, int pwidth, int pct_base)
{
    switch (d->unit) {
    case U_AUTO: return CSS_AUTO;
    case U_EM:   return psize * d->v / 100;
    case U_PCT:  return pct_base * d->v / 100;
    default:     return d->v;
    }
    (void)pwidth;
}

static void apply(struct css_style *o, const struct sdecl *d, int psize, int pwidth)
{
    int v;
    switch (d->prop) {
    case P_COLOR:   o->rgb = d->v; o->has |= CSS_P_COLOR; break;
    case P_BG:      o->bg  = d->v; o->has |= CSS_P_BG; break;
    case P_SIZE:    o->size = resolve(d, psize, pwidth, psize); o->has |= CSS_P_SIZE; break;
    case P_WEIGHT:  o->bold = d->v; o->has |= CSS_P_WEIGHT; break;
    case P_STYLE:   o->italic = d->v; o->has |= CSS_P_STYLE; break;
    case P_FAMILY:  o->mono = d->v; o->has |= CSS_P_FAMILY; break;
    case P_ALIGN:   o->align = d->v; o->has |= CSS_P_ALIGN; break;
    case P_DECOR:   o->underline = d->v; o->has |= CSS_P_DECOR; break;
    case P_DISPLAY: o->display = d->v; o->has |= CSS_P_DISPLAY; break;
    /* margin/padding percentages resolve against the parent FONT SIZE. That
     * is wrong per the spec and css.h says so explicitly - it is what shipped
     * and this stub must agree with css.c, not with the spec. */
    case P_MT: o->margin_t = resolve(d, psize, pwidth, psize); o->has |= CSS_P_MARGIN_T; break;
    case P_MB: o->margin_b = resolve(d, psize, pwidth, psize); o->has |= CSS_P_MARGIN_B; break;
    case P_ML: o->margin_l = resolve(d, psize, pwidth, psize); o->has |= CSS_P_MARGIN_L; break;
    case P_MR: o->margin_r = resolve(d, psize, pwidth, psize); o->has |= CSS_P_MARGIN_R; break;
    case P_PT: o->pad_t = resolve(d, psize, pwidth, psize); o->has |= CSS_P_PAD_T; break;
    case P_PB: o->pad_b = resolve(d, psize, pwidth, psize); o->has |= CSS_P_PAD_B; break;
    case P_PL: o->pad_l = resolve(d, psize, pwidth, psize); o->has |= CSS_P_PAD_L; break;
    case P_PR: o->pad_r = resolve(d, psize, pwidth, psize); o->has |= CSS_P_PAD_R; break;
    case P_W:    o->width  = resolve(d, psize, pwidth, pwidth); o->has2 |= CSS_Q_WIDTH; break;
    case P_H:    o->height = resolve(d, psize, pwidth, pwidth); o->has2 |= CSS_Q_HEIGHT; break;
    case P_MINW: o->min_w  = resolve(d, psize, pwidth, pwidth); o->has2 |= CSS_Q_MIN_W; break;
    case P_MAXW: o->max_w  = resolve(d, psize, pwidth, pwidth); o->has2 |= CSS_Q_MAX_W; break;
    case P_MINH: o->min_h  = resolve(d, psize, pwidth, pwidth); o->has2 |= CSS_Q_MIN_H; break;
    case P_MAXH: o->max_h  = resolve(d, psize, pwidth, pwidth); o->has2 |= CSS_Q_MAX_H; break;
    case P_BOXSIZING: o->box_sizing = d->v; o->has2 |= CSS_Q_BOXSIZING; break;
    case P_BT: v = resolve(d, psize, pwidth, pwidth); o->border_t = v == CSS_AUTO ? 0 : v; o->has2 |= CSS_Q_BORDER_T; break;
    case P_BR_: v = resolve(d, psize, pwidth, pwidth); o->border_r = v == CSS_AUTO ? 0 : v; o->has2 |= CSS_Q_BORDER_R; break;
    case P_BB: v = resolve(d, psize, pwidth, pwidth); o->border_b = v == CSS_AUTO ? 0 : v; o->has2 |= CSS_Q_BORDER_B; break;
    case P_BL: v = resolve(d, psize, pwidth, pwidth); o->border_l = v == CSS_AUTO ? 0 : v; o->has2 |= CSS_Q_BORDER_L; break;
    case P_BRGB: o->border_rgb = d->v; o->has2 |= CSS_Q_BORDER_RGB; break;
    case P_RADIUS: v = resolve(d, psize, pwidth, pwidth); o->radius = v == CSS_AUTO ? 0 : v; o->has2 |= CSS_Q_RADIUS; break;
    case P_FLOAT: o->floatv = d->v; o->has2 |= CSS_Q_FLOAT; break;
    case P_CLEAR: o->clearv = d->v; o->has2 |= CSS_Q_CLEAR; break;
    case P_POSITION: o->position = d->v; o->has2 |= CSS_Q_POSITION; break;
    case P_TOP:    o->top    = resolve(d, psize, pwidth, pwidth); o->has2 |= CSS_Q_TOP; break;
    case P_RIGHT:  o->right  = resolve(d, psize, pwidth, pwidth); o->has2 |= CSS_Q_RIGHT; break;
    case P_BOTTOM: o->bottom = resolve(d, psize, pwidth, pwidth); o->has2 |= CSS_Q_BOTTOM; break;
    case P_LEFT:   o->left   = resolve(d, psize, pwidth, pwidth); o->has2 |= CSS_Q_LEFT; break;
    case P_OVERFLOW: o->overflow = d->v; o->has2 |= CSS_Q_OVERFLOW; break;
    case P_FLEXDIR:  o->flex_dir = d->v; o->has2 |= CSS_Q_FLEX_DIR; break;
    case P_FLEXWRAP: o->flex_wrap = d->v; o->has2 |= CSS_Q_FLEX_WRAP; break;
    case P_JUSTIFY:  o->justify = d->v; o->has2 |= CSS_Q_JUSTIFY; break;
    case P_ALIGNIT:  o->align_items = d->v; o->has2 |= CSS_Q_ALIGN_IT; break;
    case P_ALIGNSELF: o->align_self = d->v; o->has2 |= CSS_Q_ALIGN_SELF; break;
    case P_GAPROW: v = resolve(d, psize, pwidth, pwidth); o->gap_row = v == CSS_AUTO ? 0 : v; o->has2 |= CSS_Q_GAP_ROW; break;
    case P_GAPCOL: v = resolve(d, psize, pwidth, pwidth); o->gap_col = v == CSS_AUTO ? 0 : v; o->has2 |= CSS_Q_GAP_COL; break;
    case P_GROW:   o->grow = d->v; o->has2 |= CSS_Q_GROW; break;
    case P_SHRINK: o->shrink = d->v; o->has2 |= CSS_Q_SHRINK; break;
    case P_BASIS:  o->basis = resolve(d, psize, pwidth, pwidth); o->has2 |= CSS_Q_BASIS; break;
    case P_GRID: {
        o->n_grid_cols = 0;
        for (int i = 0; i < d->tn && o->n_grid_cols < CSS_GRID_MAX; i++) {
            int k = d->t0 + i;
            if (k >= S_TRACKS) break;
            o->grid_col_v[o->n_grid_cols] = track_v[k];
            o->grid_col_unit[o->n_grid_cols] = track_u[k];
            o->n_grid_cols++;
        }
        o->has2 |= CSS_Q_GRID_COLS;
        break;
    }
    default: break;
    }
}

void css_compute(const struct css_elem *path, int n,
                 int parent_size, int parent_width,
                 const char *inline_style, int inline_len,
                 struct css_style *out)
{
    if (!out || !path || n <= 0) return;
    /* Apply in ascending (spec, order) by repeatedly picking the next rule
     * after the last one applied. O(rules^2) per element over a 192-entry
     * array, which is fine here and is obviously correct - a stub that has to
     * be debugged is worse than no stub. */
    int done_spec = -1, done_order = -1;
    for (;;) {
        int pick = -1;
        for (int i = 0; i < nrules; i++) {
            if (rules[i].spec < done_spec ||
                (rules[i].spec == done_spec && rules[i].order <= done_order)) continue;
            if (pick < 0 || rules[i].spec < rules[pick].spec ||
                (rules[i].spec == rules[pick].spec && rules[i].order < rules[pick].order))
                pick = i;
        }
        if (pick < 0) break;
        done_spec = rules[pick].spec; done_order = rules[pick].order;
        if (!rule_matches(&rules[pick], path, n)) continue;
        for (int d = 0; d < rules[pick].dn; d++)
            apply(out, &decls[rules[pick].d0 + d], parent_size, parent_width);
    }
    /* the style= attribute beats every rule */
    if (inline_style && inline_len > 0) {
        int save = ndecls;
        unsigned short d0, dn;
        parse_block(inline_style, 0, inline_len, &d0, &dn);
        for (int d = 0; d < dn; d++)
            apply(out, &decls[d0 + d], parent_size, parent_width);
        ndecls = save;
    }
}
