/* termwrap.c - does a line longer than the shell window survive being drawn?
 *
 * DECISIONS.md open item G. At 1920x1200 the compositor's boot line reads
 *
 *     compositor: 4 windows, shell client 82,160 1236x834
 *
 * and kernel.zl:2934 insets that by the toolkit's own padding before handing it
 * to term_draw:
 *
 *     term_draw(ax + 8*u, ay + 6*u, aw - 16*u, ah - 12*u, ...)   u = ui() = 2
 *
 * so the terminal actually gets 1204 px of width. The monospace cell at that
 * mode is 16 px (fb.c: cell_w = GLYPH_W * 2 when width >= 1400), which is
 *
 *     1204 / 16 = 75 columns
 *
 * against the longest line `help` prints - kernel.zl:627, the i2c row - at 82
 * characters. Seven characters past the edge, cut mid-glyph by the scissor.
 *
 * NOTE THE 75. POINTER-PROMPT §1c and DECISIONS item G both say 77, computed
 * from the client rect (1236/16 = 77) without the 16*u inset kernel.zl applies
 * on the way in. The defect is the same and slightly worse than recorded.
 *
 * WHY THIS IS NOT A PIXEL TEST. The scissor guarantees no ink escapes the
 * client rect, so "nothing drew outside the window" is true both before and
 * after the fix and proves nothing - the defect is that CHARACTERS ARE LOST,
 * not that pixels escaped. So this stubs fb_text_aa and asserts on what
 * term_draw asked to be drawn: no segment wider than the window, and the
 * segments of a line concatenating back to the line itself.
 *
 * Build and run:  ./build.sh && ./termwrap
 */

#include <stdio.h>
#include <string.h>

#define CELL_W 16       /* fb.c: GLYPH_W * 2 at width >= 1400 */
#define CELL_H 32       /* fb.c: GLYPH_H * 2                   */

/* the shell's client rect off the boot log, then kernel.zl:2934's inset */
#define CLIENT_X 82
#define CLIENT_Y 160
#define CLIENT_W 1236
#define CLIENT_H 834
#define UI       2
#define TERM_X   (CLIENT_X + 8 * UI)
#define TERM_Y   (CLIENT_Y + 6 * UI)
#define TERM_W   (CLIENT_W - 16 * UI)
#define TERM_H   (CLIENT_H - 12 * UI)
#define COLS     (TERM_W / CELL_W)

/* ---- the recording stubs --------------------------------------------------
 * term.c calls exactly these eight things and nothing else, so it links alone.
 * fb_text_aa records instead of drawing. */
#define MAX_DRAW 512
static struct { int x, y; char s[256]; unsigned fg; } drawn[MAX_DRAW];
static int n_drawn;
static int cur_x = -1, cur_y = -1, cur_w;

void fb_text_aa(int px, int py, const char *s, unsigned int fg)
{
    if (n_drawn >= MAX_DRAW) return;
    drawn[n_drawn].x = px; drawn[n_drawn].y = py; drawn[n_drawn].fg = fg;
    snprintf(drawn[n_drawn].s, sizeof drawn[0].s, "%s", s);
    n_drawn++;
}
void fb_text_aa2x(int px, int py, const char *s, unsigned int fg)
{ (void)px; (void)py; (void)s; (void)fg; }
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb)
{ (void)h; (void)rgb; cur_x = x; cur_y = y; cur_w = w; }   /* the cursor block */
int  fb_cell_w(void) { return CELL_W; }
int  fb_cell_h(void) { return CELL_H; }
/* the scissor wide open: this test is about what term_draw ASKS to draw, and a
 * clip that rejected rows would hide exactly the rows under test */
int  fb_clip_top(void) { return -1000000; }
int  fb_clip_bot(void) { return  1000000; }
void zl_serial_putc(char c) { (void)c; }
void zl_putc_pub(char c)    { (void)c; }   /* term_key echoes through this */

void term_clear(void);
void term_say(const char *s);
int  term_key(int code);
int  term_input_len(void);
void term_draw(int x, int y, int w, int h, unsigned int fg, unsigned int dim,
               unsigned int accent, int cursor_on);

static int fails;
static void ok(int cond, const char *what)
{
    printf("  %s %s\n", cond ? "ok  " : "FAIL", what);
    if (!cond) fails++;
}

/* kernel.zl:627 verbatim - the longest line the shell can print */
static const char *LONGEST =
    "    i2c               I2C-HID touchpad (real hardware only - QEMU has no LPSS I2C)";

/* ---- the two checks, as functions, so the negative control can reuse them --
 * A checker that cannot reject the OLD behaviour is decoration
 * (docs/GUARDS-THAT-DID-NOT-GUARD.md), so both are run a second time at the
 * bottom against a hand-built record of what term_draw used to emit. */

/* 1. nothing is asked to draw wider than the window */
static int widest(void)
{
    int max = 0;
    for (int i = 0; i < n_drawn; i++) {
        int n = (int)strlen(drawn[i].s);
        if (n > max) max = n;
    }
    return max;
}

/* 2. the segments of the target line, in the order drawn, concatenate back to
 *    it - i.e. the wrap moved the tail down rather than dropping it */
static int reassembles(const char *want)
{
    char got[512] = "";
    for (int i = 0; i < n_drawn; i++) {
        /* a segment belongs to `want` if want starts with it at the offset we
         * have accumulated so far */
        size_t at = strlen(got);
        size_t n  = strlen(drawn[i].s);
        if (n && at + n <= strlen(want) && strncmp(want + at, drawn[i].s, n) == 0)
            strcat(got, drawn[i].s);
    }
    return strcmp(got, want) == 0;
}

int main(void)
{
    printf("termwrap - a line longer than the shell window (DECISIONS item G)\n\n");
    printf("  shell client   %dx%d at %d,%d   (boot log)\n",
           CLIENT_W, CLIENT_H, CLIENT_X, CLIENT_Y);
    printf("  term_draw gets %dx%d at %d,%d   (kernel.zl:2934 inset, ui %dx)\n",
           TERM_W, TERM_H, TERM_X, TERM_Y, UI);
    printf("  cell %dx%d  ->  %d columns\n", CELL_W, CELL_H, COLS);
    printf("  longest help line: %d characters\n\n", (int)strlen(LONGEST));

    /* term_say does not append one - it is a character sink, and zl_putc's own
     * convention is a bare LF (see the comment on term_say) */
    term_clear();
    term_say("  zl shell\n");
    term_say("  commands:\n");
    term_say(LONGEST); term_say("\n");
    term_say("  ready.\n");

    n_drawn = 0;
    term_draw(TERM_X, TERM_Y, TERM_W, TERM_H, 0xD2E4FF, 0x96A5C3, 0x60D2EB, 0);

    printf("  what term_draw asked to draw, %d calls:\n", n_drawn);
    for (int i = 0; i < n_drawn; i++)
        printf("      y=%4d  %2d cols  |%s|\n",
               drawn[i].y, (int)strlen(drawn[i].s), drawn[i].s);
    printf("\n");

    ok(widest() <= COLS, "no segment is wider than the window");
    ok(reassembles(LONGEST), "the long line reassembles from its segments - "
                             "wrapped, not truncated");

    int prompt_y = TERM_Y + TERM_H - CELL_H;

    /* ---- a FULL band, which is the only case the wrap arithmetic can break --
     * With five lines in a twenty-five row band the scrollback is top-anchored
     * against `y` and there is a gap above the prompt. That is the shipping
     * behaviour and it is unchanged. The interesting case is a band that
     * OVERFLOWS, because that is where counting stored lines instead of display
     * rows pushes the newest content off the bottom by however many lines
     * happened to wrap - the single most likely way to get this wrong. */
    int band = TERM_H / CELL_H - 1;             /* rows, less the prompt */
    term_clear();
    for (int i = 0; i < band + 12; i++) {
        char buf[128];
        /* every third line is long enough to wrap, so display rows and stored
         * rows are deliberately different numbers */
        if (i % 3 == 0) { term_say(LONGEST); term_say("\n"); }
        else { snprintf(buf, sizeof buf, "  line %d\n", i); term_say(buf); }
    }
    term_say("  THE NEWEST LINE\n");
    n_drawn = 0;
    term_draw(TERM_X, TERM_Y, TERM_W, TERM_H, 0xD2E4FF, 0x96A5C3, 0x60D2EB, 0);

    int rows_drawn = 0, last_y = -1, last_i = -1;
    for (int i = 0; i < n_drawn; i++)
        if (drawn[i].y < prompt_y) {
            rows_drawn++;
            if (drawn[i].y > last_y) { last_y = drawn[i].y; last_i = i; }
        }
    printf("  band overflowed: %d display rows drawn into %d, last row |%s|\n\n",
           rows_drawn, band, last_i >= 0 ? drawn[last_i].s : "(none)");

    /* The LAST of the `band` rows, not prompt_y - CELL_H: term_draw lays the
     * scrollback out from the top of the client and pins the prompt to the
     * bottom, and 810 px is not a whole number of 32 px rows, so 10 px of slack
     * sits between the two. Pre-existing geometry, unchanged here - asserting
     * against prompt_y would be asserting that the client height happens to
     * divide by the cell. */
    ok(last_y == TERM_Y + (band - 1) * CELL_H,
       "full band: the newest scrollback row is the last row before the prompt");
    ok(prompt_y - last_y < 2 * CELL_H,
       "full band: ...with less than a whole row of slack under it");
    ok(last_i >= 0 && strcmp(drawn[last_i].s, "  THE NEWEST LINE") == 0,
       "full band: and it is the NEWEST line, not one scrolled past by wrapping");
    ok(rows_drawn == band,
       "full band: exactly as many display rows as the band holds");

    /* ---- the typed line ----------------------------------------------------
     * Same defect, one row down, and reachable by typing: `input` holds up to
     * 198 characters against 75 columns. */
    for (int i = 0; i < 120; i++) term_key('a' + (i % 26));
    n_drawn = 0; cur_x = -1;
    term_draw(TERM_X, TERM_Y, TERM_W, TERM_H, 0xD2E4FF, 0x96A5C3, 0x60D2EB, 1);

    int right = TERM_X + COLS * CELL_W;
    printf("\n  after typing 120 characters: cursor at x=%d, right edge x=%d\n\n",
           cur_x, right);
    ok(cur_x >= TERM_X && cur_x + cur_w <= right,
       "the cursor stays inside the window after 120 typed characters");
    int typed_ok = 1;
    for (int i = 0; i < n_drawn; i++)
        if (drawn[i].y == prompt_y &&
            drawn[i].x + (int)strlen(drawn[i].s) * CELL_W > right) typed_ok = 0;
    ok(typed_ok, "the typed line stays inside the window - it scrolls sideways");

    /* ---- A WINDOW WIDER THAN THE STORED LINE -------------------------------
     * Found by an adversarial review from a different model family, not by this
     * harness. `seg[]` is one stored line wide (TERM_COLS), so the wrap column
     * count is clamped to 199 - and the first version used that same clamped
     * number for the PROMPT, which is not a stored line. `input` holds 198
     * characters and does NOT wrap, it scrolls, so on a window with 203+ cells
     * a full-length line that fits was scrolled anyway and lost its first four
     * characters. The two counts are separate now; this is the case that tells
     * them apart, and it is green only if the prompt uses the unclamped one. */
    {
        int wide_w = 210 * CELL_W;           /* 210 columns, > TERM_COLS */
        for (int i = 0; i < 400; i++) term_key('a' + (i % 26));   /* fills to 198 */
        n_drawn = 0; cur_x = -1;
        term_draw(TERM_X, TERM_Y, wide_w, TERM_H, 0xD2E4FF, 0x96A5C3, 0x60D2EB, 1);
        int shown = 0;
        for (int i = 0; i < n_drawn; i++)
            if (drawn[i].y == prompt_y && drawn[i].x > TERM_X)
                shown = (int)strlen(drawn[i].s);
        printf("\n  210-column window, %d characters typed: %d shown\n\n",
               term_input_len(), shown);
        ok(shown == term_input_len(),
           "a 210-column window shows the whole typed line - the seg[] clamp "
           "does not shrink the prompt");
    }

    /* ---- NEGATIVE CONTROL --------------------------------------------------
     * Both checkers, run against what term_draw emitted BEFORE this fix: one
     * call, the whole 82-character line, drawn at the left edge. If either
     * comes back green here, it is not checking anything. */
    n_drawn = 0;
    fb_text_aa(TERM_X, TERM_Y, LONGEST, 0x96A5C3);
    int ctl_wide = (widest() <= COLS);
    int ctl_reas = reassembles(LONGEST);
    printf("\n  negative control - the pre-fix emission, one %d-column call:\n",
           (int)strlen(LONGEST));
    printf("      width check  %s\n", ctl_wide ? "PASSED (bad)" : "rejected it");
    printf("      reassembly   %s\n", ctl_reas ? "PASSED" : "rejected it");
    ok(!ctl_wide, "the width check REJECTS the pre-fix emission - it can fail");
    /* reassembly deliberately still passes: one segment equal to the whole line
     * does concatenate back to it. That is why the width check is the one that
     * catches this, and why both are asserted rather than just the reassembly -
     * on its own it would have been green against the bug. */
    ok(ctl_reas, "...while reassembly alone would NOT have - it is green on the "
                 "pre-fix emission, so it is not sufficient by itself");

    printf("\n%s (%d failure%s)\n", fails ? "FAILED" : "all passed",
           fails, fails == 1 ? "" : "s");
    return fails ? 1 : 0;
}
