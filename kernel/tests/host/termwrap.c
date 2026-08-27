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

#include "../../src/drivers/input/keycodes.h"

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
int  term_input_cursor(void);
int  term_cmd(void);
int  term_arg(void);
int  term_unknown(void);
void term_draw(int x, int y, int w, int h, unsigned int fg, unsigned int dim,
               unsigned int accent, int cursor_on);

static int fails;
static void ok(int cond, const char *what)
{
    printf("  %s %s\n", cond ? "ok  " : "FAIL", what);
    if (!cond) fails++;
}

static int submit(const char *line)
{
    term_clear();
    while (*line) term_key((unsigned char)*line++);
    return term_key('\n');
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

/* 2. the segments of the target line carry ALL of its content, in order - i.e.
 *    the wrap moved the tail down rather than dropping it.
 *
 *    THIS USED TO CONCATENATE BYTE FOR BYTE and that question stopped being the
 *    right one when the wrap started breaking on words: a word wrap CONSUMES the
 *    space it breaks on, so the exact original can no longer come back. Spaces
 *    are therefore ignored on both sides and the check is that no visible
 *    character was lost, gained or reordered. */
static int content_kept(const char *want)
{
    char got[512] = "", exp[512] = "";
    int g = 0, e = 0;
    for (int i = 0; i < n_drawn && g < 500; i++) {
        const char *t = drawn[i].s;
        /* only segments that occur in `want` belong to it - the banner rows and
         * the prompt are drawn too */
        if (!strstr(want, t) || !*t) continue;
        for (int k = 0; t[k] && g < 500; k++) if (t[k] != ' ') got[g++] = t[k];
    }
    for (int k = 0; want[k] && e < 500; k++) if (want[k] != ' ') exp[e++] = want[k];
    got[g] = 0; exp[e] = 0;
    return strcmp(got, exp) == 0;
}

/* 3. NO BREAK FALLS INSIDE A WORD. This is the defect the fix is about: the old
 *    chop chopped at exactly `cols`, so `help` came out with "back", "l" and
 *    "LPSS I2C)" stranded at the left margin - three fragments of three
 *    different descriptions, each shaped exactly like a command with no help
 *    text next to it.
 *
 *    For each consecutive pair of segments of the target, the character that
 *    FOLLOWS the first one in the original must be a space. A segment holding no
 *    space at all is exempt: it is a single token longer than the row, it has
 *    nothing to break on, and hard-breaking it is the only alternative to
 *    drawing past the client edge. */
static int no_word_split(const char *want)
{
    int cursor = 0, prev_end = -1;
    for (int i = 0; i < n_drawn; i++) {
        const char *seg = drawn[i].s;
        if (!*seg) continue;
        const char *hit = strstr(want + cursor, seg);
        if (!hit) continue;
        int st = (int)(hit - want), en = st + (int)strlen(seg);
        if (prev_end >= 0 && want[prev_end] != ' ') {
            const char *pv = drawn[i - 1].s;
            if (strchr(pv, ' ')) return 0;       /* it COULD have broken - fail */
        }
        prev_end = en; cursor = en;
    }
    return 1;
}

/* 4. a wrapped table CONTINUES UNDER ITS DESCRIPTION, not at the left margin.
 *    The expected column is read out of the data - where "I2C-HID" actually
 *    starts in the line - so this does not just restate hang_indent's own
 *    arithmetic back to itself. */
static int hangs_under_description(const char *want, int term_x, int cell_w)
{
    const char *desc = strstr(want, "I2C-HID");
    if (!desc) return 0;
    int want_x = term_x + (int)(desc - want) * cell_w;
    int seen = 0;
    for (int i = 0; i < n_drawn; i++) {
        if (!*drawn[i].s || !strstr(want, drawn[i].s)) continue;
        seen++;
        if (seen > 1 && drawn[i].x != want_x) return 0;   /* a continuation row */
    }
    return seen > 1;
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

    /* Assert the shipping matcher, not a parser-shaped copy. Codes 200/201
     * are the corresponding run_command arms in kernel.zl. */
    ok(submit("diag") == 1 && term_cmd() == 200,
       "typed `diag` reaches the flight-recorder status command");
    ok(submit("diag save") == 1 && term_cmd() == 201,
       "typed `diag save` reaches the explicit durable-flush command");
    ok(submit("diagsave") == 1 && term_cmd() == 201,
       "typed `diagsave` remains a no-space recovery alias");
    ok(submit("format") == 1 && term_cmd() == 46,
       "typed `format` reaches the explicit destructive zlfs route");
    ok(submit("ls") == 1 && term_cmd() == 108,
       "typed `ls` reaches the textual zlfs listing");
    ok(submit("files") == 1 && term_cmd() == 77,
       "typed `files` remains distinct and opens the graphical manager");
    ok(submit("fib 999999999999999999999999") == 1 &&
       term_cmd() == 102 && term_arg() == 2147483647,
       "an oversized numeric argument clamps without signed overflow");
    ok(submit("nonsense") == 0 && term_unknown() == 1,
       "an unknown typed command is reported, not silently accepted");

    /* Physical-trace regression: the ThinkPad delivered arrow keys cleanly,
     * but app_event discarded them before this file.  Exercise the shipping
     * editor itself: insertion in the middle, Delete, Home/End and history. */
    term_clear();
    term_key('d'); term_key('i'); term_key('g');
    term_key(KEY_LEFT); term_key('a');
    ok(term_input_len() == 4 && term_input_cursor() == 3,
       "Left moves the insertion point and typing inserts in the middle");
    term_key(KEY_END);
    ok(term_key('\n') == 1 && term_cmd() == 200,
       "the physically observed `dig` + Left + `a` correction submits `diag`");

    term_clear();
    for (const char *p = "dixag"; *p; p++) term_key((unsigned char)*p);
    term_key(KEY_HOME); term_key(KEY_RIGHT); term_key(KEY_RIGHT);
    term_key(KEY_DELETE);
    ok(term_input_len() == 4 && term_input_cursor() == 2,
       "Home, Right and Delete edit at the cursor instead of the line end");
    term_key(KEY_END);
    ok(term_key('\n') == 1 && term_cmd() == 200,
       "Delete-corrected `diag` reaches the command matcher");

    term_clear();
    term_key(KEY_UP);
    ok(term_input_len() == 4 && term_input_cursor() == 4,
       "Up restores the newest submitted command with the cursor at its end");
    ok(term_key('\n') == 1 && term_cmd() == 200,
       "a recalled command submits without being retyped");

    term_clear();
    term_key('x'); term_key(KEY_UP); term_key(KEY_DOWN);
    ok(term_input_len() == 1 && term_input_cursor() == 1,
       "Down returns from history to the draft that was being edited");

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
    ok(content_kept(LONGEST), "the long line keeps all its content - wrapped, "
                              "not truncated");
    ok(no_word_split(LONGEST), "the break falls between words, not inside one");
    ok(hangs_under_description(LONGEST, TERM_X, CELL_W),
       "the continuation row hangs under the description, not at the margin");

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
    int ctl_keep = content_kept(LONGEST);
    printf("\n  control A - the pre-WRAP emission, one %d-column call:\n",
           (int)strlen(LONGEST));
    printf("      width check    %s\n", ctl_wide ? "PASSED (bad)" : "rejected it");
    printf("      content check  %s\n", ctl_keep ? "PASSED" : "rejected it");
    ok(!ctl_wide, "the width check REJECTS the over-wide emission - it can fail");
    /* content deliberately still passes: one segment equal to the whole line
     * does carry all of it. That is why the width check is the one that catches
     * this, and why both are asserted rather than just content - on its own it
     * would have been green against the bug. */
    ok(ctl_keep, "...while the content check alone would NOT have - it is green "
                 "on the over-wide emission, so it is not sufficient by itself");

    /* ---- CONTROL B - the emission this commit actually replaced --------------
     * The width check was already green on the character chop: it never drew
     * past the edge, it just broke words to stay inside. So the two checks above
     * are BOTH blind to the defect being fixed here, and a checker that cannot
     * see the bug it was written for is decoration. Rebuild the chop by hand -
     * 77 columns, then the remaining 5 at the left margin - and require the two
     * new checks to reject it. */
    n_drawn = 0;
    {
        char a[128];
        int cut = COLS;
        snprintf(a, sizeof a, "%.*s", cut, LONGEST);
        fb_text_aa(TERM_X, TERM_Y, a, 0x96A5C3);
        fb_text_aa(TERM_X, TERM_Y + CELL_H, LONGEST + cut, 0x96A5C3);
    }
    int ctlb_wide  = (widest() <= COLS);
    int ctlb_split = no_word_split(LONGEST);
    int ctlb_hang  = hangs_under_description(LONGEST, TERM_X, CELL_W);
    printf("\n  control B - the CHARACTER CHOP this commit replaced, %d + %d cols:\n",
           COLS, (int)strlen(LONGEST) - COLS);
    printf("      width check    %s\n", ctlb_wide ? "PASSED - blind to it" : "rejected it");
    printf("      word check     %s\n", ctlb_split ? "PASSED (bad)" : "rejected it");
    printf("      indent check   %s\n", ctlb_hang ? "PASSED (bad)" : "rejected it");
    ok(ctlb_wide, "the chop is INSIDE the window - so width alone never saw this bug");
    ok(!ctlb_split, "the word check REJECTS the character chop - it can fail");
    ok(!ctlb_hang, "the indent check REJECTS the margin-anchored continuation");

    printf("\n%s (%d failure%s)\n", fails ? "FAILED" : "all passed",
           fails, fails == 1 ? "" : "s");
    return fails ? 1 : 0;
}
