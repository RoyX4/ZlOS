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

static int dp(int n, int q8) { return (n * q8 + 128) >> 8; }

void ui_theme_init_q8(int scale_q8)
{
    if (scale_q8 < 192) scale_q8 = 192;
    if (scale_q8 > 768) scale_q8 = 768;
    /* ---- ONE PALETTE, and kernel.zl's is the one -- DECISIONS.md item E -----
     * "Do not introduce a second visual system" is what this comment used to
     * say, and a second visual system is exactly what these eleven values were.
     * kernel.zl's rgb() constants painted the header bar, the dock and the two
     * legacy app bodies; this struct painted EVERY WINDOW FRAME ON SCREEN, and
     * the two agreed on 2 of 10 roles. Two cyans and two panel colours were up
     * at the same time. visual-speed-northstar.md names "duplicated palette
     * roles" as a thing that must not ship.
     *
     * WHICH ONE IS THE SOURCE OF TRUTH IS NOT A TASTE CALL, which is why this
     * could be closed rather than escalated. docs/design/zlOS-design-northstar.
     * html:13 says, in its own words, what it is:
     *
     *     -- the zlOS palette, straight from kernel.zl's rgb() theme --
     *
     * The reference was transcribed FROM kernel.zl. So "agree with the
     * reference" and "agree with kernel.zl" are the same instruction, and this
     * file was the only one of the three that had drifted - DECISIONS #33
     * counted it at 11 of 21 roles for kernel.zl against 2 of 10 here.
     *
     * Every value below is now a named token of that reference, and the token
     * is in the comment so the next drift is a diff and not an argument.
     * REVERSING IS THIS BLOCK, nothing else - no call site names a colour. */
    theme.bg        = 0x1A1E32;   /* --wall-top   the desktop behind it all  */
    theme.panel     = 0x05060A;   /* --panel      .win body. WAS 0x1E2A44,   */
                                  /*              a mid-navy against the     */
                                  /*              reference's near-black.    */
                                  /*              visual-speed-northstar.md  */
                                  /*              §identity 3 asks for       */
                                  /*              "near-black content" too.  */
    /* THE ONE ROLE THE REFERENCE DOES NOT DEFINE, flagged rather than faked.
     * It is a static mockup: its own step list has "Widget toolkit - clickable
     * buttons, scrollbars" as QUEUED, so there is no button face in it to copy.
     * --panel-2 (#0b0e18) is its nearest "raised surface", and it is a ~3%
     * luminance step off --panel - which would make every button, slider track
     * and toggle in ui.c effectively invisible. So this takes --line-soft,
     * which IS a reference token, sits clearly above the panel, and stays below
     * --line so a hairline still reads on top of a control. */
    theme.panel_hi  = 0x1A2136;   /* --line-soft  raised: control faces      */
    theme.text      = 0xD2E4FF;   /* --txt-hi                                */
    theme.text_dim  = 0x96A5C3;   /* --txt-dim                               */
    theme.accent    = 0x60D2EB;   /* --accent     was 0x55D6FF - THE SECOND  */
                                  /*              CYAN, next to kernel.zl's  */
                                  /*              ACCENT on the same screen. */
    theme.border    = 0x26304A;   /* --line       .win border                */
    theme.danger    = 0xE05A5A;   /* --crit                                  */
    theme.title     = 0x182238;   /* focused chrome: quiet, not a blue slab  */
    theme.title_bot = 0x0B0E18;   /* focus belongs to border/shadow, not fill */
    /* The unfocused title bar is a GRADIENT in the reference and in kernel.zl
     * (:794, tbt/tbb) and was a flat slab here, because the struct had one
     * field for it. Both ends now, and they are the reference's own two stops:
     * .win:not(.focus) .titlebar{linear-gradient(180deg,#2a3550,#182238)}. */
    theme.title_off = 0x121722;   /* unfocused chrome, top                   */
    theme.title_off_bot = 0x080A10; /* ...and bottom                         */
    theme.wallpaper_top = 0x1A1E32;
    theme.wallpaper_bot = 0x0A0C16;
    theme.bar_top   = 0x161B29;
    theme.bar_bot   = 0x090B12;
    theme.bar_hi    = 0x26304A;
    theme.chrome    = 0x0B0E18;
    theme.chrome_line = 0x05060A;
    theme.text_hi   = 0xD2E4FF;
    theme.ok        = 0x5BD66E;

    /* ---- metrics, v10 SS6.10 -----------------------------------------------
     * Counted out of the prototype's stylesheet rather than picked by eye, and
     * then snapped onto the 4/8/12/16/24 scale this file already enforces -
     * which is the whole of "adopt its structure, keep our system":
     *
     *   border-radius   11..16px, mode 13/14   -> 12 (was 5)
     *   gap             8 and 9 dominate       -> 8, unchanged
     *   row height      26/30/32/34            -> 28 (was 24)
     *   padding         9..14                  -> 12, unchanged
     *
     * THE RADIUS IS THE ONE THAT CHANGES HOW IT READS. At 5 the corners are a
     * bevel; at 12 they are the soft rectangles the prototype is built out of,
     * and it is the single largest visual difference between the two. The
     * nested inner rrect stays exactly one pixel tighter, so the hairline
     * border still follows the outer curve instead of cutting across it.
     *
     * The prototype's silhouette is a 16px outer radius and a 36px title bar.
     * Those two measurements matter more than another colour tweak: together
     * they stop a window reading as a square debug panel with a label nailed
     * across its top.
     */
    theme.scale_q8 = scale_q8;
    theme.scale   = (scale_q8 + 128) >> 8;
    if (theme.scale < 1) theme.scale = 1;
    theme.pad     = dp(12, scale_q8);
    theme.gap     = dp( 8, scale_q8);
    theme.row_h   = dp(28, scale_q8);
    theme.radius  = dp(16, scale_q8);
    theme.title_h = dp(36, scale_q8);
}

void ui_theme_init(int scale) { ui_theme_init_q8(scale * 256); }

unsigned ui_color(int role)
{
    const unsigned *first = &theme.bg;
    if ((unsigned)role >= UI_COLOR_COUNT) return theme.danger;
    return first[role];
}

int ui_metric(int role)
{
    switch (role) {
    case UI_METRIC_PAD: return theme.pad;
    case UI_METRIC_GAP: return theme.gap;
    case UI_METRIC_ROW_H: return theme.row_h;
    case UI_METRIC_RADIUS: return theme.radius;
    case UI_METRIC_TITLE_H: return theme.title_h;
    case UI_METRIC_SCALE_Q8: return theme.scale_q8;
    default: return 0;
    }
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

/* ---- keyboard focus -------------------------------------------------------
 * Window focus was a title-bar hue and an accent underline; a focused CONTROL
 * had no indicator at all, and no way to move between controls without the
 * mouse.
 *
 * THE FOCUS INDEX LIVES HERE, NOT IN L. L is reset by every ui_begin, and
 * focus has to survive between passes or it would be forgotten between the
 * hit-test and the draw - and between frames, which is the entire point of it.
 * Which widget is focused is still the APP's to choose (ui_set_focus), the way
 * every other piece of widget state is; this only remembers the number.
 *
 * -1 is "nothing focused", and it is the default: a desktop that boots with a
 * ring already on some arbitrary control looks broken. */
static int focus_idx = -1;
static int focus_activate;      /* one-shot: the focused widget fires */
static int focus_count;         /* how many fired-capable widgets last pass */

void ui_set_focus(int idx)   { focus_idx = idx; }
int  ui_focus_get(void)      { return focus_idx; }
int  ui_widget_count(void)   { return focus_count; }
void ui_activate_focus(void) { focus_activate = 1; }

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
    focus_count = 0;      /* recounted every pass, so it follows the layout */
}

/* Consume the one-shot activation. Called by the app AFTER it has re-run its
 * widget sequence, so the flag covers exactly one pass and an Enter cannot
 * fire the same control again on the next repaint. */
void ui_end_activate(void) { focus_activate = 0; }

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
 * nothing is drawn and this is the only thing that happens.
 *
 * The keyboard path goes through the SAME funnel, deliberately. A widget must
 * not be able to tell a click from an Enter - the moment it can, the two paths
 * drift and one of them grows a bug the other does not have. */
static int fire(int x, int y, int w, int h)
{
    int me = L.index++;
    if (me + 1 > focus_count) focus_count = me + 1;
    if (focus_activate && me == focus_idx) return 1;
    if (!L.click || !hit(x, y, w, h)) return 0;
    L.fired = me;
    return 1;
}

/* The ring, drawn by each firing widget at the end of its own draw block.
 *
 * L.index has already been advanced by fire(), so `L.index - 1` is the calling
 * widget's own id - which means this needs no argument and cannot be passed
 * the wrong one. It draws OUTSIDE the control's rect so it never covers the
 * label, and in the accent so it reads as "the keyboard is here" rather than
 * as another border. */
static void focus_ring(int x, int y, int w, int h)
{
    if (L.mode != UI_DRAW || focus_idx < 0 || L.index - 1 != focus_idx) return;
    int g = UI_S1(&theme) / 2;
    int x0 = x - g, y0 = y - g, x1 = x + w + g, y1 = y + h + g;
    unsigned c = theme.accent;
    fb_fill_px(x0, y0, x1 - x0, 1, c);
    fb_fill_px(x0, y1 - 1, x1 - x0, 1, c);
    fb_fill_px(x0, y0, 1, y1 - y0, c);
    fb_fill_px(x1 - 1, y0, 1, y1 - y0, c);
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
        focus_ring(x, y, w, h);
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
        focus_ring(x, y, w, h);
    }
    return fired;
}

int ui_toggle_value(const char *s, int on)
{
    int v = on ? 1 : 0;
    ui_toggle(s, &v);
    return v;
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
        /* THE SPAN IS THE NUMBER OF VALUES, NOT THE GAP BETWEEN THE ENDS.
         *
         * This was (hi - lo), which divides the track into (hi - lo) buckets
         * for (hi - lo + 1) values - so the last value is only reachable at
         * px == x + w exactly, one pixel PAST the track's last pixel. The
         * maximum could not be selected at all, and both clamps below were
         * dead code because the expression could never exceed hi.
         *
         * Found by driving the Settings app's own scale slider (1..3) from a
         * harness: clicking the rightmost pixel of a 388 px track gave
         * 387*2/388 + 1 = 2. Every slider in the toolkit had it, so pointer
         * speed could not reach 400 either.
         *
         * With (hi - lo + 1) the track divides into equal buckets per value,
         * the last pixel selects hi, and the clamp catches the genuine
         * px == x + w case that a pointer grab can produce mid-drag. */
        int span = hi - lo + 1;
        int t = (L.px - x) * span / (w ? w : 1) + lo;
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
        focus_ring(x, y, w, h);
    }
    return fired;
}

int ui_slider_value(int v, int lo, int hi)
{
    ui_slider(&v, lo, hi);
    return v;
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
void fb_clip_get(int *x0, int *y0, int *x1, int *y1);

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
        focus_ring(x, y, w, h);
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

    /* NARROW THE SCISSOR, DO NOT REPLACE IT.
     *
     * wm_repaint has already clipped this app to its own client rectangle, and
     * that is the guarantee the whole layering rests on: an app which draws at
     * -500,-500 physically cannot produce a pixel. Calling fb_clip() with the
     * viewport alone throws that away and re-opens the app to anywhere the
     * viewport reaches, which at a scroll position past the window edge is
     * off the window entirely. Intersect instead. */
    if (L.mode == UI_DRAW) {
        fb_clip_get(&S.cx0, &S.cy0, &S.cx1, &S.cy1);
        int nx0 = x     > S.cx0 ? x     : S.cx0;
        int ny0 = y     > S.cy0 ? y     : S.cy0;
        int nx1 = x + L.w < S.cx1 ? x + L.w : S.cx1;
        int ny1 = y + h   < S.cy1 ? y + h   : S.cy1;
        fb_clip(nx0, ny0, nx1 - nx0, ny1 - ny0);
    }
    /* the cursor moves INTO the viewport, offset by the scroll position */
    L.cy = y - S.off;
    L.cx = L.x;
}

void ui_scroll_end(int *off)
{
    S.content = L.cy + S.off - S.y;     /* how tall the content turned out */
    if (L.mode == UI_DRAW) {
        /* RESTORE, NOT REMOVE. This was fb_clip_none(), which does not put the
         * caller's scissor back - it deletes it. wm_repaint sets the client
         * rectangle before calling an app; the moment that app closed a scroll
         * region every widget it drew afterwards was free to paint over the
         * whole screen, including other windows and the desktop furniture.
         *
         * S.cx0..cy1 have been declared "the scissor to put back" since this
         * function was written and nothing ever read them - the fix was half
         * built and then never wired up, which is exactly the project's
         * "the code exists is not the code works" class. */
        fb_clip(S.cx0, S.cy0, S.cx1 - S.cx0, S.cy1 - S.cy0);
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

/* zl numbers cross the runtime boundary by value, while the C toolkit keeps a
 * scroll offset through an out-parameter. App draws are sequential and scroll
 * regions cannot nest, so one bridge slot preserves the exact C semantics
 * without inventing a retained widget object. */
static int zl_scroll_off;
void ui_scroll_begin_value(int h, int off)
{
    zl_scroll_off = off;
    ui_scroll_begin(h, &zl_scroll_off);
}
int ui_scroll_end_value(void)
{
    ui_scroll_end(&zl_scroll_off);
    return zl_scroll_off;
}
