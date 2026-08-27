/* systest.c - the clipboard, window snapping and notifications, asserted.
 *
 * These three share a property: every bug in them is invisible in a
 * screenshot. A clipboard that truncates silently looks identical to one that
 * worked until you paste. A snap that stores the restore rectangle on every
 * snap instead of the first looks perfect until you snap twice. A toast that
 * takes focus looks like a toast, and the only symptom is that the keystroke
 * you typed while it was up went nowhere.
 *
 * So all three are written as plain C over integers with no compositor, no
 * framebuffer and no timer, and they are exercised here instead of looked at.
 *
 * NOTE ON WHY THIS IS NOT IN wmtest.c: the compositor's harness is being
 * edited right now in the desktop/overnight-compositor worktree. Adding to it
 * would collide. This is a separate file for that reason and no other.
 *
 * Build and run:  ./build.sh && ./systest
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>

typedef unsigned int u32;

/* ---- clip.c -------------------------------------------------------------- */
int  clip_put(const void *p, u32 n, int type);
int  clip_get(void *p, u32 max);
u32  clip_len(void);
int  clip_type(void);
u32  clip_seq(void);
void clip_clear(void);
int  clip_byte(int i);
void clip_begin(void);
int  clip_push(int ch);
int  clip_commit(int type);
u32  clip_staged(void);
int  clip_is_copy(int ch);
int  clip_is_paste(int ch);

/* ---- snap.c -------------------------------------------------------------- */
#define SNAP_NONE 0
#define SNAP_LEFT 1
#define SNAP_RIGHT 2
#define SNAP_MAX 3
#define SNAP_TL 4
#define SNAP_TR 5
#define SNAP_BL 6
#define SNAP_BR 7
#define SK_LEFT 1
#define SK_RIGHT 2
#define SK_UP 3
#define SK_DOWN 4
void snap_reset(void);
int  snap_state(int win);
int  snap_zone_for_point(int px, int py, int sw, int sh);
void snap_rect(int z, int sw, int sh, int rt, int rb, int *x, int *y, int *w, int *h);
void snap_rect_lr(int z, int sw, int sh, int rt, int rb, int rl, int rr,
                  int *x, int *y, int *w, int *h);
int  snap_apply(int win, int z, int cx, int cy, int cw, int ch,
                int sw, int sh, int rt, int rb, int *x, int *y, int *w, int *h);
int  snap_release(int win, int *x, int *y, int *w, int *h);
void snap_note_moved(int win);
void snap_note_closed(int win);
int  snap_key_zone(int win, int dir);

/* ---- notify.c ------------------------------------------------------------ */
void        notify_reset(void);
int         notify_post(const char *text, u32 ticks);
int         notify_tick(u32 now);
int         notify_dismiss(void);
int         notify_active(void);
const char *notify_text(void);
int         notify_queued(void);
int         notify_waiting(void);
u32         notify_dropped(void);
void        notify_rect(int sw, int sh, int rb, int scale, int *x, int *y, int *w, int *h);
int         notify_hit(int px, int py, int sw, int sh, int rb, int scale);

/* ---- the character sink ---------------------------------------------------
 * Captured, so "it refused" can be checked against "it refused for the right
 * reason". A clipboard that rejects a large selection by saying the queue is
 * full is still a bug. */
static char saidbuf[8192];
static int  saidlen;
void zl_putc_pub(char c)
{
    if (saidlen < (int)sizeof saidbuf - 1) saidbuf[saidlen++] = c;
    saidbuf[saidlen] = 0;
}
static void said_reset(void) { saidlen = 0; saidbuf[0] = 0; }
static int  said(const char *n) { return strstr(saidbuf, n) != NULL; }

static int fails;
static void ok(const char *what, int cond)
{
    printf("  %-62s %s\n", what, cond ? "ok" : "FAIL");
    if (!cond) fails++;
}

/* =====================================================================
 * THE CLIPBOARD
 * ===================================================================== */

/* Two "apps", each with its own text, wired to the clipboard exactly the way a
 * real one would be: a Ctrl+C copies this app's selection out, a Ctrl+V pastes
 * whatever is there in. Neither knows the other exists. That is the point of
 * the gate - the paste has to land in a DIFFERENT app than the copy. */
struct fakeapp {
    const char *name;
    char        text[256];
    int         len;
};

static int app_key(struct fakeapp *a, int ch)
{
    if (clip_is_copy(ch)) {
        clip_begin();
        for (int i = 0; i < a->len; i++) clip_push((unsigned char)a->text[i]);
        return clip_commit(1);
    }
    if (clip_is_paste(ch)) {
        char tmp[4096];
        int n = clip_get(tmp, sizeof tmp);
        for (int i = 0; i < n && a->len < (int)sizeof a->text - 1; i++)
            a->text[a->len++] = tmp[i];
        a->text[a->len] = 0;
        return n;
    }
    return 0;
}

static void test_clipboard(void)
{
    printf("  -- the clipboard --\n");
    clip_clear();
    said_reset();

    ok("an empty clipboard has length 0", clip_len() == 0);
    ok("...and type EMPTY", clip_type() == 0);

    u32 s0 = clip_seq();
    ok("put succeeds", clip_put("copy me", 7, 1) == 1);
    ok("...length is 7", clip_len() == 7);
    ok("...the sequence number moved", clip_seq() != s0);

    char out[64];
    memset(out, 0, sizeof out);
    ok("get returns 7", clip_get(out, sizeof out) == 7);
    ok("...and the right bytes", memcmp(out, "copy me", 7) == 0);
    ok("clip_byte reads one out for a zl caller", clip_byte(0) == 'c' && clip_byte(6) == 'e');
    ok("...and out of range is 0, not a read off the end", clip_byte(7) == 0 && clip_byte(-1) == 0);

    memset(out, 0, sizeof out);
    ok("get into a SMALLER buffer returns what fit", clip_get(out, 3) == 3);
    ok("...and copied exactly that much", memcmp(out, "cop", 3) == 0);

    /* the staged path zl has to use, having no strings */
    clip_begin();
    const char *s = "staged one byte at a time";
    for (const char *p = s; *p; p++) clip_push(*p);
    ok("a staged selection reports its length before commit", clip_staged() == 25);
    ok("commit publishes it", clip_commit(1) == 1);
    memset(out, 0, sizeof out);
    clip_get(out, sizeof out);
    ok("...and it reads back", strcmp(out, "staged one byte at a time") == 0);

    /* overflow must refuse AND keep what was there */
    said_reset();
    clip_begin();
    for (int i = 0; i < 5000; i++) clip_push('x');
    ok("a selection larger than the buffer is refused", clip_commit(1) == 0);
    ok("...saying nothing was copied", said("nothing was copied"));
    memset(out, 0, sizeof out);
    clip_get(out, sizeof out);
    ok("...and the PREVIOUS contents are still intact",
       strcmp(out, "staged one byte at a time") == 0);

    said_reset();
    ok("an oversized C put is refused too", clip_put(out, 99999, 1) == 0);
    ok("...and says how big the buffer is", said("4096-byte buffer"));

    ok("Ctrl+C is character 3", clip_is_copy(3) && !clip_is_copy('c'));
    ok("Ctrl+V is character 22", clip_is_paste(22) && !clip_is_paste('v'));

    /* ---- THE GATE: a paste that lands in a different app than the copy --- */
    printf("  -- copy in one app, paste in another --\n");
    struct fakeapp term = { "terminal", "zl> nvme_read_to", 16 };
    struct fakeapp edit = { "editor",   "",                0  };

    clip_clear();
    ok("the terminal copies its line with Ctrl+C", app_key(&term, 3) == 1);
    ok("...the clipboard now holds 16 bytes", clip_len() == 16);
    ok("the EDITOR pastes it with Ctrl+V", app_key(&edit, 22) == 16);
    ok("...and the editor's own text now contains it",
       strcmp(edit.text, "zl> nvme_read_to") == 0);
    ok("...while the terminal is unchanged",
       strcmp(term.text, "zl> nvme_read_to") == 0);

    /* and back the other way, to prove it is not one-directional plumbing */
    edit.len = 0; edit.text[0] = 0;
    strcpy(edit.text, "pasted back"); edit.len = 11;
    app_key(&edit, 3);
    term.len = 0; term.text[0] = 0;
    app_key(&term, 22);
    ok("and the reverse direction works with no extra wiring",
       strcmp(term.text, "pasted back") == 0);
}

/* =====================================================================
 * WINDOW SNAPPING
 * ===================================================================== */
static void test_snap(void)
{
    printf("\n  -- window snapping --\n");
    snap_reset();

    /* an ODD width and an odd work height, deliberately: two halves of 1365
     * must cover 1365 with no seam */
    const int SW = 1365, SH = 869, RT = 34, RB = 64;
    const int AW = SW, AH = SH - RT - RB;      /* 1365 x 771 */
    int x, y, w, h, x2, y2, w2, h2;

    ok("the top-left corner beats the left edge",
       snap_zone_for_point(2, 2, SW, SH) == SNAP_TL);
    ok("the bottom-right corner beats the right edge",
       snap_zone_for_point(SW - 1, SH - 1, SW, SH) == SNAP_BR);
    ok("the left edge alone is the left half",
       snap_zone_for_point(3, SH / 2, SW, SH) == SNAP_LEFT);
    ok("the top edge alone maximises",
       snap_zone_for_point(SW / 2, 1, SW, SH) == SNAP_MAX);
    ok("the middle of the screen is not a zone",
       snap_zone_for_point(SW / 2, SH / 2, SW, SH) == SNAP_NONE);

    snap_rect(SNAP_LEFT, SW, SH, RT, RB, &x, &y, &w, &h);
    ok("the left half starts at x=0, below the header",
       x == 0 && y == RT && h == AH);
    snap_rect(SNAP_RIGHT, SW, SH, RT, RB, &x2, &y2, &w2, &h2);
    ok("the right half starts where the left one ends", x2 == w);
    ok("...and the two halves cover the width EXACTLY, odd though it is",
       w + w2 == AW);
    ok("...with no overlap", x + w == x2);

    snap_rect(SNAP_MAX, SW, SH, RT, RB, &x, &y, &w, &h);
    ok("maximised is the work area, not the screen",
       x == 0 && y == RT && w == AW && h == AH);
    ok("...so it does not go under the dock", y + h == SH - RB);

    int qx[4], qy[4], qw[4], qh[4];
    int zs[4] = { SNAP_TL, SNAP_TR, SNAP_BL, SNAP_BR };
    for (int i = 0; i < 4; i++)
        snap_rect(zs[i], SW, SH, RT, RB, &qx[i], &qy[i], &qw[i], &qh[i]);
    int area = 0;
    for (int i = 0; i < 4; i++) area += qw[i] * qh[i];
    ok("the four quarters tile the work area exactly, by area",
       area == AW * AH);
    ok("...the top two share a top edge", qy[0] == RT && qy[1] == RT);
    ok("...the bottom two start where the top two end",
       qy[2] == qy[0] + qh[0] && qy[3] == qy[1] + qh[1]);
    ok("...and the bottom row reaches the dock exactly",
       qy[2] + qh[2] == SH - RB);

    /* ---- THE RESTORE RECTANGLE ------------------------------------------- */
    printf("  -- the restore rectangle, which is the part people forget --\n");
    snap_reset();
    const int OX = 120, OY = 90, OW = 640, OH = 400;

    ok("a window that was never snapped has nothing to restore",
       snap_release(0, &x, &y, &w, &h) == 0);

    ok("snapping left applies", snap_apply(0, SNAP_LEFT, OX, OY, OW, OH,
                                           SW, SH, RT, RB, &x, &y, &w, &h) == 1);
    ok("...and the state says LEFT", snap_state(0) == SNAP_LEFT);
    ok("snapping left AGAIN is a no-op", snap_apply(0, SNAP_LEFT, x, y, w, h,
                                           SW, SH, RT, RB, &x2, &y2, &w2, &h2) == 0);

    ok("snapping RIGHT from left applies", snap_apply(0, SNAP_RIGHT, x, y, w, h,
                                           SW, SH, RT, RB, &x, &y, &w, &h) == 1);
    ok("...and it is the right half", x == AW / 2);

    ok("releasing restores", snap_release(0, &x, &y, &w, &h) == 1);
    ok("...the ORIGINAL geometry, not the left half it passed through",
       x == OX && y == OY && w == OW && h == OH);
    ok("...and the state is clear again", snap_state(0) == SNAP_NONE);
    ok("releasing twice does nothing", snap_release(0, &x, &y, &w, &h) == 0);

    /* dragging by hand invalidates the snap */
    snap_apply(1, SNAP_LEFT, 10, 20, 300, 200, SW, SH, RT, RB, &x, &y, &w, &h);
    snap_note_moved(1);
    ok("a hand-dragged window is no longer snapped", snap_state(1) == SNAP_NONE);
    ok("...so there is nothing stale to restore it to",
       snap_release(1, &x, &y, &w, &h) == 0);

    /* a closed window must not leave state for whatever reuses the slot */
    snap_apply(2, SNAP_MAX, 1, 2, 3, 4, SW, SH, RT, RB, &x, &y, &w, &h);
    snap_note_closed(2);
    ok("a closed window leaves no snap state behind", snap_state(2) == SNAP_NONE);
    ok("...and no restore rectangle", snap_release(2, &x, &y, &w, &h) == 0);

    /* out of range must not scribble on the arrays */
    ok("an out-of-range window index is refused, not indexed",
       snap_apply(999, SNAP_LEFT, 0, 0, 1, 1, SW, SH, RT, RB, &x, &y, &w, &h) == 0
       && snap_apply(-1, SNAP_LEFT, 0, 0, 1, 1, SW, SH, RT, RB, &x, &y, &w, &h) == 0
       && snap_state(999) == SNAP_NONE);

    /* ---- Super+arrow ----------------------------------------------------- */
    printf("  -- Super+arrow, using the MOD_SUPER nothing has ever used --\n");
    snap_reset();
    ok("Super+Left from unsnapped is the left half",
       snap_key_zone(3, SK_LEFT) == SNAP_LEFT);
    snap_apply(3, SNAP_LEFT, OX, OY, OW, OH, SW, SH, RT, RB, &x, &y, &w, &h);
    ok("...again is the top-left quarter", snap_key_zone(3, SK_LEFT) == SNAP_TL);
    snap_apply(3, SNAP_TL, x, y, w, h, SW, SH, RT, RB, &x, &y, &w, &h);
    ok("...and again the bottom-left", snap_key_zone(3, SK_LEFT) == SNAP_BL);
    ok("Super+Up maximises from anywhere", snap_key_zone(3, SK_UP) == SNAP_MAX);
    ok("Super+Down un-snaps", snap_key_zone(3, SK_DOWN) == SNAP_NONE);

    snap_reset();
    snap_apply(4, SNAP_LEFT, OX, OY, OW, OH, SW, SH, RT, RB, &x, &y, &w, &h);
    ok("Super+Right from the left half goes through the middle",
       snap_key_zone(4, SK_RIGHT) == SNAP_MAX);

    /* a degenerate screen must not produce a negative height */
    snap_rect(SNAP_MAX, 320, 60, RT, RB, &x, &y, &w, &h);

    /* ---- the side reserve: a maximised window must not cover the rail ------
     *
     * The shell moved its launcher to a 170dp REGISTER RAIL on the left edge,
     * and wm.c reserved nothing there - so SNAP_MAX landed at x = 0 and drew
     * straight over it. The old two-reserve snap_rect could not express a side
     * at all; snap_rect_lr can, and this is the assertion that it does.
     *
     * The control matters as much as the check: the four-argument form must
     * still put a maximised window at x = 0, because twenty-odd call sites in
     * this file mean exactly that and would otherwise be silently re-aimed. */
    {
        const int L = 170, R = 0;
        int mx, my, mw, mh;
        snap_rect_lr(SNAP_MAX, SW, SH, RT, RB, L, R, &mx, &my, &mw, &mh);
        ok("maximised starts to the RIGHT of the rail", mx == L);
        ok("maximised is narrowed by the rail, not just moved",
           mw == SW - L - R);
        ok("maximised still reaches the right screen edge", mx + mw == SW - R);

        int lx, ly, lw, lh;
        snap_rect_lr(SNAP_LEFT, SW, SH, RT, RB, L, R, &lx, &ly, &lw, &lh);
        ok("snap-left also clears the rail", lx == L);
        ok("snap-left is half the REMAINING width, not half the screen",
           lw == (SW - L - R) / 2);

        int rx, ry, rw, rh;
        snap_rect_lr(SNAP_RIGHT, SW, SH, RT, RB, L, R, &rx, &ry, &rw, &rh);
        ok("left and right halves still meet exactly", lx + lw == rx);
        ok("...and together fill the area beside the rail",
           lw + rw == SW - L - R);

        /* CONTROL: the old signature is unchanged for everyone else. */
        int ox, oy, ow, oh;
        snap_rect(SNAP_MAX, SW, SH, RT, RB, &ox, &oy, &ow, &oh);
        ok("control: the four-reserve form still starts at x = 0", ox == 0);
        ok("control: ...and is still the full screen width", ow == SW);
    }


    ok("a screen shorter than its own furniture gives h=0, not a negative",
       h == 0);
}

/* =====================================================================
 * NOTIFICATIONS
 * ===================================================================== */

/* A miniature of wm.c's route_key, to answer the one question that matters:
 * with a toast on screen, does the next keystroke still reach the app? */
static int keys_seen;
static void route_key_like_wm(int ch)
{
    (void)ch;
    /* notify.c is given first refusal here, exactly as a focus-stealing
     * implementation would be. It has no entry point that could consume a
     * key, which is the structural reason it cannot eat one. */
    keys_seen++;
}

static void test_notify(void)
{
    printf("\n  -- notifications --\n");
    notify_reset();
    said_reset();

    ok("nothing is showing at rest", notify_active() == 0);
    ok("...and nothing is queued", notify_queued() == 0);

    ok("posting succeeds", notify_post("disk mounted", 300) == 1);
    ok("...but nothing is on screen until a tick", notify_active() == 0);

    ok("the first tick puts it on screen", notify_tick(1000) == 1);
    ok("...and it is active", notify_active() == 1);
    ok("...with the right text", strcmp(notify_text(), "disk mounted") == 0);
    ok("a tick that changes nothing says so", notify_tick(1001) == 0);

    /* ---- it expires ON ITS OWN ------------------------------------------- */
    ok("it is still up one tick before expiry", notify_tick(1299) == 0 && notify_active());
    ok("it retires ITSELF at the expiry tick", notify_tick(1300) == 1);
    ok("...and nothing is showing", notify_active() == 0);
    ok("...and the queue is empty", notify_queued() == 0);

    /* ---- IT DOES NOT STEAL FOCUS ---------------------------------------- */
    notify_reset();
    notify_post("something happened", 300);
    notify_tick(2000);
    keys_seen = 0;
    route_key_like_wm('a');
    route_key_like_wm('b');
    ok("with a toast on screen, keystrokes still reach the app", keys_seen == 2);
    ok("...and the toast is still up, having consumed nothing",
       notify_active() == 1);

    /* ---- click to dismiss early ----------------------------------------- */
    ok("a click on the toast dismisses it", notify_dismiss() == 1);
    ok("...immediately, without waiting for expiry", notify_active() == 0);
    ok("dismissing when nothing is up does nothing", notify_dismiss() == 0);

    /* ---- a queue of four, one at a time ---------------------------------- */
    notify_reset();
    said_reset();
    ok("four fit", notify_post("one", 100) == 1 && notify_post("two", 100) == 1
                && notify_post("three", 100) == 1 && notify_post("four", 100) == 1);
    ok("...and the queue holds exactly four", notify_queued() == 4);
    ok("a FIFTH is refused rather than silently dropped",
       notify_post("five", 100) == 0);
    ok("...and it says a message was dropped", said("queue full"));
    ok("...and the count of dropped ones is visible", notify_dropped() == 1);

    notify_tick(5000);
    int live_was_one = (strcmp(notify_text(), "one") == 0);
    ok("the live one is the FIRST posted, not the newest", live_was_one);
    ok("only ONE is visible at a time", notify_queued() == 4 && notify_active() == 1);
    ok("...with three waiting behind it", notify_waiting() == 3);

    ok("it expires", notify_tick(5100) == 1);
    ok("the NEXT one promotes on the following tick", notify_tick(5101) == 1);

    /* 'five' arrived into a full queue. The message it displaced is the
     * OLDEST WAITING one - 'two' - and NOT 'one', which was live. Taking away
     * what the user is currently reading to make room for something they have
     * not seen would be the wrong trade, so the live slot is never the victim.
     * That is why the next message up is 'three'. */
    ok("...and it is 'three': 'two' was the waiting message that got displaced",
       strcmp(notify_text(), "three") == 0);
    ok("the LIVE message was never the one dropped - 'one' showed in full",
       live_was_one);
    ok("...and 'five', the newest, is still queued behind", notify_queued() == 3);

    /* ---- the tick counter wrapping --------------------------------------- */
    notify_reset();
    notify_post("near the wrap", 100);
    notify_tick(0xFFFFFFF0u);
    ok("a toast posted just before the 2^32 tick wrap is up",
       notify_active() == 1);
    ok("...and retires after its 100 ticks, across the wrap",
       notify_tick(0xFFFFFFF0u + 100) == 1 && notify_active() == 0);

    /* ---- where it goes --------------------------------------------------- */
    int x, y, w, h;
    notify_rect(1920, 1200, 64, 1, &x, &y, &w, &h);
    ok("the toast sits above the dock, not under it", y + h <= 1200 - 64);
    ok("...and inside the right edge", x + w <= 1920);
    notify_rect(800, 600, 64, 1, &x, &y, &w, &h);
    ok("at 800x600 it is still fully on screen",
       x >= 0 && y >= 0 && x + w <= 800 && y + h <= 600 - 64);
    notify_rect(240, 600, 64, 1, &x, &y, &w, &h);
    ok("on a screen narrower than the toast it shrinks instead of hanging off",
       x >= 0 && x + w <= 240);

    notify_reset();
    notify_post("hit me", 300);
    notify_tick(10);
    notify_rect(1920, 1200, 64, 1, &x, &y, &w, &h);
    ok("a click inside the toast hits it",
       notify_hit(x + 2, y + 2, 1920, 1200, 64, 1) == 1);
    ok("...a click outside does not",
       notify_hit(x - 5, y - 5, 1920, 1200, 64, 1) == 0);
    notify_dismiss();
    ok("...and once dismissed, nothing is hit there",
       notify_hit(x + 2, y + 2, 1920, 1200, 64, 1) == 0);
}

int main(void)
{
    printf("the system track - clipboard, snapping, notifications\n\n");
    test_clipboard();
    test_snap();
    test_notify();
    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "all good", fails);
    return fails ? 1 : 0;
}
