/* snap.c - drag a window to an edge and it takes that half.
 *
 * `wm_resize` has existed since the compositor was written and NOTHING HAS
 * EVER CALLED IT. Windows could be moved and closed and never made a
 * different size, which is why the desktop always looks like whatever layout()
 * decided at boot.
 *
 * THE PART PEOPLE FORGET IS THE RESTORE RECTANGLE
 * -----------------------------------------------
 * Snapping is four lines of arithmetic. What makes a snap implementation good
 * or bad is what happens when you drag the window back OFF the edge, and the
 * two ways to get that wrong are both easy:
 *
 *   1. not storing the pre-snap geometry at all, so un-snapping leaves the
 *      window at half width forever
 *   2. storing it on EVERY snap, so left-then-right overwrites the original
 *      with the left half, and un-snapping restores... the left half
 *
 * (2) is the one that survives review, because it looks right until you snap
 * twice. So the rule here is: the restore rectangle is captured on the
 * transition from unsnapped to snapped, and at no other time.
 *
 * THE RECTANGLES TILE EXACTLY
 * ---------------------------
 * Two halves of an odd width must still cover the work area with no seam and
 * no overlap, so the left half is w/2 and the right half is w - w/2 rather
 * than w/2 twice. Same vertically. On a 1365-pixel-wide screen the naive
 * version leaves a one-pixel strip of wallpaper down the middle, which is the
 * kind of thing that is invisible in a screenshot and obvious on a desk.
 *
 * The work area excludes the header bar and the dock. A "maximised" window
 * that goes under the dock cannot reach its own status bar.
 */

#define SNAP_NONE   0
#define SNAP_LEFT   1
#define SNAP_RIGHT  2
#define SNAP_MAX    3
#define SNAP_TL     4
#define SNAP_TR     5
#define SNAP_BL     6
#define SNAP_BR     7

#ifndef SNAP_WINDOWS
#define SNAP_WINDOWS 12          /* matches WM_MAX in ui.h */
#endif

/* How close to an edge counts as "at" it. In pixels of the pointer position,
 * not of the window - you aim with the cursor. */
#define SNAP_EDGE   16

struct saved_rect { int x, y, w, h; };

static struct saved_rect saved[SNAP_WINDOWS];
static int               zone[SNAP_WINDOWS];

void snap_reset(void)
{
    for (int i = 0; i < SNAP_WINDOWS; i++) {
        zone[i] = SNAP_NONE;
        saved[i].x = saved[i].y = saved[i].w = saved[i].h = 0;
    }
}

int snap_state(int win)
{
    if (win < 0 || win >= SNAP_WINDOWS) return SNAP_NONE;
    return zone[win];
}

/* ---- which zone does a pointer at (px,py) mean? --------------------------
 * Corners win over edges: a pointer in the top-left corner is inside both the
 * left strip and the top strip, and "quarter" is the more specific intent.
 * Checking corners first is the whole of that rule. */
int snap_zone_for_point(int px, int py, int sw, int sh)
{
    int left   = px <= SNAP_EDGE;
    int right  = px >= sw - 1 - SNAP_EDGE;
    int top    = py <= SNAP_EDGE;
    int bottom = py >= sh - 1 - SNAP_EDGE;

    if (left  && top)    return SNAP_TL;
    if (right && top)    return SNAP_TR;
    if (left  && bottom) return SNAP_BL;
    if (right && bottom) return SNAP_BR;
    if (left)            return SNAP_LEFT;
    if (right)           return SNAP_RIGHT;
    if (top)             return SNAP_MAX;
    return SNAP_NONE;
}

/* ---- the geometry -------------------------------------------------------
 * Written out rather than computed from a table, because every one of these
 * is an assertion in the harness and a table would let two of them share a
 * bug. `reserve_top` is the header bar, `reserve_bot` the dock. */
void snap_rect(int z, int sw, int sh, int reserve_top, int reserve_bot,
               int *x, int *y, int *w, int *h)
{
    int ax = 0;
    int ay = reserve_top;
    int aw = sw;
    int ah = sh - reserve_top - reserve_bot;
    if (ah < 0) ah = 0;

    int lw = aw / 2;              /* left half  */
    int rw = aw - lw;             /* right half - together they are exactly aw */
    int th = ah / 2;              /* top half   */
    int bh = ah - th;             /* bottom half                              */

    switch (z) {
    case SNAP_LEFT:  *x = ax;      *y = ay;      *w = lw; *h = ah; break;
    case SNAP_RIGHT: *x = ax + lw; *y = ay;      *w = rw; *h = ah; break;
    case SNAP_MAX:   *x = ax;      *y = ay;      *w = aw; *h = ah; break;
    case SNAP_TL:    *x = ax;      *y = ay;      *w = lw; *h = th; break;
    case SNAP_TR:    *x = ax + lw; *y = ay;      *w = rw; *h = th; break;
    case SNAP_BL:    *x = ax;      *y = ay + th; *w = lw; *h = bh; break;
    case SNAP_BR:    *x = ax + lw; *y = ay + th; *w = rw; *h = bh; break;
    default:         *x = ax;      *y = ay;      *w = aw; *h = ah; break;
    }
}

/* ---- apply and release ---------------------------------------------------
 * These do not call wm_move/wm_resize themselves. The caller does, with the
 * rectangle handed back - which is what lets the whole of this file be
 * asserted on the host with no compositor, no framebuffer and no boot.
 *
 * Returns 1 if the caller should apply *x,*y,*w,*h; 0 if nothing changes.
 */
int snap_apply(int win, int z, int cur_x, int cur_y, int cur_w, int cur_h,
               int sw, int sh, int reserve_top, int reserve_bot,
               int *x, int *y, int *w, int *h)
{
    if (win < 0 || win >= SNAP_WINDOWS) return 0;
    if (z == SNAP_NONE) return 0;
    if (zone[win] == z) return 0;             /* already there; do nothing */

    /* THE ONE RULE. Capture the restore rectangle only on the transition INTO
     * a snapped state. Snapping left and then right must still restore the
     * size the window had before it ever touched an edge. */
    if (zone[win] == SNAP_NONE) {
        saved[win].x = cur_x;
        saved[win].y = cur_y;
        saved[win].w = cur_w;
        saved[win].h = cur_h;
    }
    zone[win] = z;
    snap_rect(z, sw, sh, reserve_top, reserve_bot, x, y, w, h);
    return 1;
}

int snap_release(int win, int *x, int *y, int *w, int *h)
{
    if (win < 0 || win >= SNAP_WINDOWS) return 0;
    if (zone[win] == SNAP_NONE) return 0;
    *x = saved[win].x;
    *y = saved[win].y;
    *w = saved[win].w;
    *h = saved[win].h;
    zone[win] = SNAP_NONE;
    return 1;
}

/* A window the user has dragged by hand is no longer snapped, and its stored
 * restore rectangle is stale. Forgetting to call this is how "un-snap" ends up
 * teleporting a window back to somewhere it has not been for ten minutes. */
void snap_note_moved(int win)
{
    if (win < 0 || win >= SNAP_WINDOWS) return;
    zone[win] = SNAP_NONE;
}

/* A window that closes must not leave its state behind for whatever opens in
 * the same slot next. */
void snap_note_closed(int win)
{
    if (win < 0 || win >= SNAP_WINDOWS) return;
    zone[win] = SNAP_NONE;
    saved[win].x = saved[win].y = saved[win].w = saved[win].h = 0;
}

/* ---- the keyboard half --------------------------------------------------
 * MOD_SUPER has been tracked by input.c since it was written and used for
 * nothing at all. Super+Left/Right snap to that half; Super+Up maximises;
 * Super+Down un-snaps, or does nothing if the window was never snapped.
 *
 * Left from an already-left-snapped window goes to the top-left quarter, and
 * again to the bottom-left - which is how you build a four-up layout without
 * touching the mouse. */
#define SK_LEFT  1
#define SK_RIGHT 2
#define SK_UP    3
#define SK_DOWN  4

int snap_key_zone(int win, int dir)
{
    int cur = snap_state(win);
    switch (dir) {
    case SK_UP:    return SNAP_MAX;
    case SK_DOWN:  return SNAP_NONE;
    case SK_LEFT:
        if (cur == SNAP_LEFT)  return SNAP_TL;
        if (cur == SNAP_TL)    return SNAP_BL;
        if (cur == SNAP_RIGHT) return SNAP_MAX;   /* back through the middle */
        return SNAP_LEFT;
    case SK_RIGHT:
        if (cur == SNAP_RIGHT) return SNAP_TR;
        if (cur == SNAP_TR)    return SNAP_BR;
        if (cur == SNAP_LEFT)  return SNAP_MAX;
        return SNAP_RIGHT;
    }
    return SNAP_NONE;
}
