/* notify.c - a toast. It appears, it sits there, it goes away by itself.
 *
 * DELIBERATELY NOT A NOTIFICATION CENTRE. There is no history, no persistence,
 * no per-app grouping, no do-not-disturb and no list you can open later. A
 * queue of four, one visible at a time, and when the fourth arrives while
 * three are waiting the OLDEST WAITING ONE IS DROPPED AND SAID SO. Every one
 * of those absent features is a week of work and the reason desktops have
 * notification bugs.
 *
 * THE ONE THAT MATTERS: A TOAST MUST NOT TAKE FOCUS
 * -------------------------------------------------
 * A notification that steals focus eats the next keystroke. You are typing,
 * something completes, and the character you were in the middle of goes to a
 * window that is about to close itself. That is why this file has no concept
 * of focus at all and hands the compositor a rectangle rather than a window:
 * there is nothing here that COULD take focus, which is a stronger guarantee
 * than remembering not to.
 *
 * TIME IS A TICK COUNT PASSED IN
 * ------------------------------
 * notify_tick(now) rather than notify.c reading the timer. It makes expiry
 * testable without waiting - the harness advances `now` by 300 and asserts the
 * toast retired - and it means this file has no dependency on idt.c at all.
 *
 * ZTOAST IS NOT IN THIS FILE, AND THAT IS DELIBERATE
 * --------------------------------------------------
 * The reference's `ztoast` - `.16s ease-out`, opacity 0 to 1 with
 * translateY(10px) to 0 - is the toast's ENTRY, and for a while this file was
 * the obvious place to look for it. It is not here and should not be: this
 * file owns the QUEUE and nothing else, which is why notify_rect() is the
 * only geometry in it and why even that is computed from a screen size handed
 * in rather than read.
 *
 * The animation lives in wm.c beside toast_draw(), on the same timeline as
 * every other one, started by the same wm_frame() branch that already damages
 * the toast's rectangle when notify_tick() reports a change. Look for
 * WM_FX_TOAST there. Putting it here would have given this file an opinion
 * about where a toast is drawn and a dependency on the compositor's clock -
 * the two things the paragraphs above exist to keep out.
 */

typedef unsigned int u32;

void zl_putc_pub(char c);

#define NOTE_SLOTS  4
#define NOTE_TEXT   64
#define NOTE_TICKS  300          /* three seconds at 100 Hz */

struct note {
    char text[NOTE_TEXT];
    u32  ticks;                  /* how long it should be visible */
};

static struct note q[NOTE_SLOTS];
static int  qn;                  /* how many are queued, live one included  */
static int  showing;             /* is q[0] currently on screen?            */
static u32  expires_at;          /* tick count at which the live one retires */
static u32  dropped;             /* how many were pushed out of a full queue */
static u32  posted;

static void p_str(const char *s) { while (*s) zl_putc_pub(*s++); }

static void copy_text(char *d, const char *s)
{
    int i = 0;
    for (; i < NOTE_TEXT - 1 && s[i]; i++) d[i] = s[i];
    d[i] = 0;
}

void notify_reset(void)
{
    qn = 0; showing = 0; expires_at = 0; dropped = 0; posted = 0;
    for (int i = 0; i < NOTE_SLOTS; i++) q[i].text[0] = 0;
}

/* Post a message. Returns 1 if it was queued, 0 if the queue was full and the
 * oldest WAITING one had to go - never 0 silently. The live toast is never the
 * one dropped: taking away something the user is currently reading to make
 * room for something they have not seen yet is the wrong trade. */
int notify_post(const char *text, u32 ticks)
{
    if (ticks == 0) ticks = NOTE_TICKS;
    posted++;

    if (qn < NOTE_SLOTS) {
        copy_text(q[qn].text, text);
        q[qn].ticks = ticks;
        qn++;
        return 1;
    }

    /* Full. Drop the oldest WAITING entry - index 1, because index 0 is either
     * on screen or about to be. */
    dropped++;
    p_str("  notify: queue full, dropped an older message\n");
    for (int i = 1; i < NOTE_SLOTS - 1; i++) q[i] = q[i + 1];
    copy_text(q[NOTE_SLOTS - 1].text, text);
    q[NOTE_SLOTS - 1].ticks = ticks;
    return 0;
}

static void retire(void)
{
    for (int i = 0; i < NOTE_SLOTS - 1; i++) q[i] = q[i + 1];
    q[NOTE_SLOTS - 1].text[0] = 0;
    if (qn > 0) qn--;
    showing = 0;
}

/* Called once a frame with the current tick count. Promotes the next message
 * when the live one expires, and starts the clock on a message that has just
 * become the live one. Returns 1 if what is on screen CHANGED, which is the
 * compositor's cue to repaint - the same contract app_tick uses. */
int notify_tick(u32 now)
{
    if (!showing) {
        if (qn == 0) return 0;
        showing    = 1;
        expires_at = now + q[0].ticks;
        return 1;                       /* one just appeared */
    }
    /* Unsigned subtraction rather than `now >= expires_at`, so a tick counter
     * that wraps at 2^32 retires the toast instead of pinning it on screen for
     * the next 497 days. */
    if ((u32)(now - expires_at) < 0x80000000u) {
        retire();
        return 1;                       /* one just went away */
    }
    return 0;
}

/* A click on the toast. Dismissing early is the whole of the interaction -
 * there is nothing else to click. */
int notify_dismiss(void)
{
    if (!showing) return 0;
    retire();
    return 1;
}

int         notify_active(void)  { return showing; }
const char *notify_text(void)    { return showing ? q[0].text : 0; }
int         notify_queued(void)  { return qn; }
int         notify_waiting(void) { return qn > 0 ? qn - (showing ? 1 : 0) : 0; }
u32         notify_dropped(void) { return dropped; }
u32         notify_posted(void)  { return posted; }
u32         notify_expires(void) { return expires_at; }

/* one character out, for a zl caller with no strings */
int notify_byte(int i)
{
    if (!showing || i < 0 || i >= NOTE_TEXT) return 0;
    return (int)(unsigned char)q[0].text[i];
}

/* ---- where it goes on screen --------------------------------------------
 * Bottom right, above the dock, inset by a margin. Position-pure like every
 * app_draw in this system: derived entirely from the screen size handed in,
 * nothing baked in, so it lands correctly at 800x600 and at 2560x1440.
 */
#define NOTE_W      280
#define NOTE_H      56
#define NOTE_MARGIN 16

void notify_rect(int sw, int sh, int reserve_bot, int scale,
                 int *x, int *y, int *w, int *h)
{
    if (scale < 1) scale = 1;
    int nw = NOTE_W * scale;
    int nh = NOTE_H * scale;
    int m  = NOTE_MARGIN * scale;
    if (nw > sw - 2 * m) nw = sw - 2 * m;     /* a narrow screen still fits  */
    if (nw < 1) nw = 1;
    *w = nw;
    *h = nh;
    *x = sw - nw - m;
    *y = sh - reserve_bot - nh - m;
    if (*x < 0) *x = 0;
    if (*y < 0) *y = 0;
}

/* Did a click at (px,py) land on the toast? The compositor asks; this file
 * does not know what a mouse is. */
int notify_hit(int px, int py, int sw, int sh, int reserve_bot, int scale)
{
    if (!showing) return 0;
    int x, y, w, h;
    notify_rect(sw, sh, reserve_bot, scale, &x, &y, &w, &h);
    return px >= x && px < x + w && py >= y && py < y + h;
}
