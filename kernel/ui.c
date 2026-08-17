/* ui.c - an immediate-mode toolkit. No allocation, no widget tree.
 *
 * IMMEDIATE MODE WAS FORCED, NOT CHOSEN. A retained tree (Qt, GTK,
 * SerenityOS's LibGUI) allocates an object per widget and holds parent/child
 * pointers - a tree needs a heap, and a tree of children IS a list. zlOS has
 * neither: the kernel subset's zl_list_n is a hard fault. So ui_button("OK")
 * RETURNS whether it was clicked, nothing is allocated, and widget state stays
 * in the app where it already lives.
 *
 * THE CONSEQUENCE, AND IT IS THE THING MOST LIKELY TO BE GOT WRONG:
 * hit testing re-runs app_draw with drawing switched OFF - the same trick
 * intel_modeset_dry() uses for its 35-step sequence. So a widget must RETURN
 * whether it fired and must NEVER take an action as an argument, because C
 * evaluates arguments eagerly: ui_button("Delete", delete_everything()) would
 * delete everything on every hit-test pass, twice per click. That is the same
 * language behaviour that forced MS_STEP to be a macro rather than a flag.
 *
 * LAYOUT IS A FLOWING CURSOR and that is the entire algorithm. A widget asks
 * for a size, is placed at the cursor, the cursor advances by size + gap, and
 * rows wrap at the content width. No tree walk, no constraint solver, no
 * second pass.
 */

#include "ui.h"

void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);
void fb_rrect(int x, int y, int w, int h, int r, unsigned int rgb);
void fb_text_prop(int px, int py, const char *s, unsigned int fg);
void fb_text_prop(int px, int py, const char *s, unsigned int fg);
int  fb_text_prop_w(const char *s);
int  fb_text_prop_h(void);
int  fb_cell_w(void);
int  fb_cell_h(void);

/* ---- the theme ------------------------------------------------------------
 * One struct, every colour and every metric. Before this they were spread
 * across kernel.zl and fb.c as literals picked by eye per window. */
static struct ui_theme theme;

const struct ui_theme *ui_theme(void) { return &theme; }
void ui_theme_set(const struct ui_theme *t) { theme = *t; }

void ui_theme_init(int scale)
{
    if (scale < 1) scale = 1;
    /* Deep navy, nested rounded panels, focused-blue title gradients with an
     * accent underline. This is the look that already exists - extended, not
     * replaced. Do not introduce a second visual system. */
    theme.bg        = 0x141A2E;
    theme.panel     = 0x1E2A44;
    theme.panel_hi  = 0x27354F;
    theme.text      = 0xE4EDFF;
    theme.text_dim  = 0x8FA0C0;
    theme.accent    = 0x55D6FF;
    theme.border    = 0x141A2A;
    theme.danger    = 0xE05561;
    theme.title     = 0x305CA8;
    theme.title_bot = 0x16285C;
    theme.title_off = 0x243350;

    theme.scale   = scale;
    theme.pad     = 12 * scale;      /* every one of these is on the scale */
    theme.gap     =  8 * scale;
    theme.row_h   = 24 * scale;
    theme.radius  =  5 * scale;
    theme.title_h = 28 * scale;
}

/* ---- the layout cursor ---------------------------------------------------- */
static struct {
    int x, y, w, h;          /* the content box, inside the padding */
    int cx, cy;              /* where the next widget goes          */
    int row_h;               /* tallest thing in the current row    */
    int mode;                /* UI_DRAW or UI_HITTEST               */
    int px, py, click;       /* the pointer, and whether it is down */
    int fired;               /* which widget index fired, or -1     */
    int index;               /* widget counter, for identity        */
    int in_row;              /* 1 while ui_row() is open            */
} L;

void ui_begin(int x, int y, int w, int h, int mode, int px, int py, int click)
{
    L.x = x + theme.pad;
    L.y = y + theme.pad;
    L.w = w - 2 * theme.pad;
    L.h = h - 2 * theme.pad;
    L.cx = L.x;
    L.cy = L.y;
    L.row_h = 0;
    L.mode = mode;
    L.px = px; L.py = py; L.click = click;
    L.fired = -1;
    L.index = 0;
    L.in_row = 0;
}

int ui_fired(void) { return L.fired; }

void ui_row(void)    { L.in_row = 1; }
void ui_endrow(void) { L.in_row = 0; L.cx = L.x; L.cy += L.row_h + theme.gap; L.row_h = 0; }

/* Place a widget of this size and return where it landed. The ONE function
 * that knows about the cursor; every widget below is a call to this plus
 * some drawing. */
static void place(int w, int h, int *ox, int *oy)
{
    if (w > L.w) w = L.w;
    if (L.in_row && L.cx + w > L.x + L.w) {     /* wrap */
        L.cx = L.x;
        L.cy += L.row_h + theme.gap;
        L.row_h = 0;
    }
    *ox = L.cx;
    *oy = L.cy;
    if (h > L.row_h) L.row_h = h;
    if (L.in_row) {
        L.cx += w + theme.gap;
    } else {
        L.cx = L.x;
        L.cy += h + theme.gap;
        L.row_h = 0;
    }
}

static int hit(int x, int y, int w, int h)
{
    return L.px >= x && L.px < x + w && L.py >= y && L.py < y + h;
}

/* Every widget that can fire funnels through this, so "did it fire" is decided
 * in exactly one place and cannot drift between widgets. In UI_HITTEST mode
 * nothing is drawn and this is the only thing that happens. */
static int fire(int x, int y, int w, int h)
{
    int me = L.index++;
    if (!L.click || !hit(x, y, w, h)) return 0;
    L.fired = me;
    return 1;
}

/* A proportional layout cannot ask "length times cell" any more - it has to
 * MEASURE. That is the part of item 4 that touches every widget, and the
 * reason a toolkit needs one function for it rather than a multiply spread
 * across ten call sites. */
static int text_w(const char *s) { return fb_text_prop_w(s); }
static int text_h(void)          { return fb_text_prop_h(); }

/* ---- the widgets ---------------------------------------------------------- */
void ui_label(const char *s)
{
    int x, y;
    place(text_w(s), text_h(), &x, &y);
    if (L.mode == UI_DRAW) fb_text_prop(x, y, s, theme.text);
}

void ui_label_dim(const char *s)
{
    int x, y;
    place(text_w(s), text_h(), &x, &y);
    if (L.mode == UI_DRAW) fb_text_prop(x, y, s, theme.text_dim);
}

void ui_bar(int pct)
{
    int x, y, w = L.w, h = UI_S2(&theme);
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    place(w, h, &x, &y);
    if (L.mode != UI_DRAW) return;
    fb_rrect(x, y, w, h, h / 2, theme.panel_hi);
    if (pct) fb_rrect(x, y, w * pct / 100, h, h / 2, theme.accent);
}

int ui_button(const char *s)
{
    int w = text_w(s) + 2 * UI_S3(&theme), h = theme.row_h;
    int x, y;
    place(w, h, &x, &y);
    int over = hit(x, y, w, h);
    int fired = fire(x, y, w, h);
    if (L.mode == UI_DRAW) {
        /* accent on hover, danger only on press. One button, three states -
         * and it is the difference between "drawn" and "designed". */
        unsigned face = over ? (L.click ? theme.accent : theme.panel_hi) : theme.panel_hi;
        fb_rrect(x, y, w, h, UI_S1(&theme), face);
        fb_text_prop(x + UI_S3(&theme), y + (h - text_h()) / 2, s,
                     over && L.click ? theme.border : theme.text);
    }
    return fired;
}

void ui_sep(void)
{
    int x, y;
    place(L.w, 1, &x, &y);
    if (L.mode == UI_DRAW) fb_fill_px(x, y, L.w, 1, theme.border);
}

void ui_space(int n)
{
    int x, y;
    place(1, n > 0 ? n : theme.gap, &x, &y);
    (void)x; (void)y;
}

/* Note the int* - the widget owns no state. The app already has the variable;
 * a retained toolkit would have made a second copy of it and then needed a way
 * to keep the two in step. */
int ui_toggle(const char *s, int *on)
{
    /* A SWITCH IS A PILL, NOT A CIRCLE. The track has to be visibly wider than
     * it is tall or the knob fills it and the whole control reads as a round
     * button - which says "press me", not "I am on or off". Caught by looking
     * at a rendered frame; every assertion about it passed while it was wrong,
     * because "does it toggle" and "does it look like a toggle" are different
     * questions and only one of them has a test. */
    int kh = theme.row_h * 2 / 3;             /* shorter than a full row      */
    int kw = kh * 2;                          /* ...and twice as wide as tall */
    int w = kw + theme.gap + text_w(s), h = theme.row_h;
    int x, y;
    place(w, h, &x, &y);
    int fired = fire(x, y, w, h);
    if (fired) *on = !*on;
    if (L.mode == UI_DRAW) {
        int ty = y + (h - kh) / 2;            /* centre the track in the row  */
        int pad = UI_S1(&theme) / 2;
        int d = kh - 2 * pad;                 /* the knob                     */
        fb_rrect(x, ty, kw, kh, kh / 2, *on ? theme.accent : theme.panel_hi);
        fb_rrect(*on ? x + kw - d - pad : x + pad, ty + pad, d, d, d / 2,
                 *on ? theme.border : theme.text_dim);
        fb_text_prop(x + kw + theme.gap, y + (h - text_h()) / 2, s, theme.text);
    }
    return fired;
}

/* The slider is what PROVES wm.c's pointer grab: once pressed it must keep
 * tracking after the pointer leaves its rectangle, which only works because
 * the window that owns the grab keeps receiving the events. */
int ui_slider(int *v, int lo, int hi)
{
    int w = L.w, h = theme.row_h;
    int x, y;
    place(w, h, &x, &y);
    int fired = fire(x, y, w, h);
    if (hi <= lo) hi = lo + 1;
    if (fired) {
        int t = (L.px - x) * (hi - lo) / (w ? w : 1) + lo;
        if (t < lo) t = lo;
        if (t > hi) t = hi;
        *v = t;
    }
    if (L.mode == UI_DRAW) {
        int track = UI_S2(&theme);
        int ty = y + (h - track) / 2;
        fb_rrect(x, ty, w, track, track / 2, theme.panel_hi);
        int pos = (*v - lo) * w / (hi - lo);
        fb_rrect(x, ty, pos, track, track / 2, theme.accent);
        int knob = theme.row_h - UI_S1(&theme);
        int kx = x + pos - knob / 2;
        if (kx < x) kx = x;
        if (kx > x + w - knob) kx = x + w - knob;
        fb_rrect(kx, y + UI_S1(&theme) / 2, knob, knob, knob / 2, theme.text);
    }
    return fired;
}

/* A number with a label, right-aligned - the System Monitor's readouts. No
 * itoa in this kernel, so it is built here, digits backwards into a buffer. */
void ui_num(const char *s, int v)
{
    char buf[16];
    int n = 0, neg = v < 0;
    unsigned u = neg ? (unsigned)(-v) : (unsigned)v;
    if (!u) buf[n++] = '0';
    while (u && n < 12) { buf[n++] = (char)('0' + u % 10u); u /= 10u; }
    if (neg && n < 15) buf[n++] = '-';
    for (int i = 0; i < n / 2; i++) {
        char t = buf[i]; buf[i] = buf[n - 1 - i]; buf[n - 1 - i] = t;
    }
    buf[n] = 0;

    int x, y;
    place(L.w, text_h(), &x, &y);
    if (L.mode != UI_DRAW) return;
    fb_text_prop(x, y, s, theme.text_dim);
    fb_text_prop(x + L.w - text_w(buf), y, buf, theme.text);
}

/* ---- ui_list_row and ui_scroll --------------------------------------------
 * Both were held back until clipping was settled, and for a real reason: a
 * scrolled list draws rows that are PARTLY outside their own viewport, so
 * without a scissor the overflow lands on whatever is next to it. fb_clip is
 * now proven - zero pixels escape a set scissor across every primitive - so
 * these are safe to build.
 *
 * They are still immediate mode. A list does not own its items and there is no
 * item array anywhere: the app calls ui_list_row once per thing it has, in a
 * loop it already writes, and the row returns whether it was clicked. That is
 * the only shape available without a heap, and it is also the nicer one.
 */
void fb_clip(int x, int y, int w, int h);
void fb_clip_none(void);

static struct {
    int on;                  /* inside a scroll region?                    */
    int x, y, w, h;          /* the viewport, in screen coordinates        */
    int off;                 /* how far down the content is scrolled       */
    int content;             /* total content height, measured as we go    */
    int save_cy, save_h;     /* the cursor state ui_scroll_end restores     */
    int cx0, cy0, cx1, cy1;  /* the scissor to put back                     */
} S;

/* A selectable row: full width, one row high, highlighted when the pointer is
 * over it. `selected` marks the current one - passed in rather than stored,
 * because the app already knows which of its things is selected and a second
 * copy here would be a second thing to keep in step. */
int ui_list_row(const char *s, int selected)
{
    int x, y, w = L.w, h = theme.row_h;
    place(w, h, &x, &y);

    /* Cheap rejection FIRST. A list can be thousands of rows and only ~20 are
     * ever visible; drawing the rest and letting the scissor throw them away
     * would make scrolling cost O(items) instead of O(visible). The scissor is
     * a correctness guarantee, not a substitute for not drawing. */
    if (S.on && (y + h < S.y || y > S.y + S.h)) {
        L.index++;                      /* keep widget identity stable */
        return 0;
    }

    int over = hit(x, y, w, h);
    int fired = fire(x, y, w, h);
    if (L.mode == UI_DRAW) {
        if (selected)  fb_rrect(x, y, w, h, UI_S1(&theme), theme.accent);
        else if (over) fb_rrect(x, y, w, h, UI_S1(&theme), theme.panel_hi);
        fb_text_prop(x + UI_S2(&theme), y + (h - text_h()) / 2, s,
                     selected ? theme.border : theme.text);
    }
    return fired;
}

/* Open a scrolling viewport `h` pixels tall. `off` is the app's scroll
 * position - again, the app's variable, not ours. Everything between this and
 * ui_scroll_end is clipped to the viewport and shifted up by off. */
void ui_scroll_begin(int h, int *off)
{
    int x, y;
    place(L.w, h, &x, &y);

    S.on = 1;
    S.x = x; S.y = y; S.w = L.w; S.h = h;
    S.off = off ? *off : 0;
    S.content = 0;
    S.save_cy = L.cy;
    S.save_h = L.h;

    if (L.mode == UI_DRAW) fb_clip(x, y, L.w, h);
    /* the cursor moves INTO the viewport, offset by the scroll position */
    L.cy = y - S.off;
    L.cx = L.x;
}

void ui_scroll_end(int *off)
{
    S.content = L.cy + S.off - S.y;     /* how tall the content turned out */
    if (L.mode == UI_DRAW) {
        fb_clip_none();
        /* the scrollbar, only when there is something to scroll. A bar that
         * is always there but sometimes full-height is a bar that means
         * nothing. */
        if (S.content > S.h) {
            int bw = UI_S1(&theme);
            int bx = S.x + S.w - bw;
            int th2 = S.h * S.h / S.content;
            if (th2 < UI_S6(&theme)) th2 = UI_S6(&theme);
            int ty = S.y + (S.h - th2) * S.off / (S.content - S.h);
            fb_rrect(bx, S.y, bw, S.h, bw / 2, theme.panel_hi);
            fb_rrect(bx, ty, bw, th2, bw / 2, theme.text_dim);
        }
    }
    /* clamp the app's scroll position to what the content turned out to be -
     * it cannot know that before the loop it just ran */
    if (off) {
        int max = S.content - S.h;
        if (max < 0) max = 0;
        if (*off > max) *off = max;
        if (*off < 0) *off = 0;
    }
    L.cy = S.save_cy + S.h + theme.gap;
    L.cx = L.x;
    L.h = S.save_h;
    S.on = 0;
}

int ui_scroll_content(void) { return S.content; }
