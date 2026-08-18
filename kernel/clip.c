/* clip.c - the clipboard. One buffer, and it is what makes this one system.
 *
 * Before this, zlOS was several programs that happened to share a screen. You
 * could read a line in the terminal and there was no way to get it into the
 * editor except to retype it. Copy and paste is the smallest feature that
 * turns "several programs" into "a machine": it is the only thing in a desktop
 * whose whole purpose is to be shared BETWEEN apps.
 *
 * So the entire design is that there is exactly ONE of it. A fixed buffer, a
 * length, and a type tag. No history, no multiple selections, no ownership
 * protocol - X11 spent thirty years on that and the result is that pasting
 * from a closed window loses the data.
 *
 * TWO WAYS IN, BECAUSE THERE ARE TWO KINDS OF CALLER
 * --------------------------------------------------
 * C callers hand over a pointer and a length. zl cannot: the zl kernel subset
 * has string literals and no string VALUES, so a zl app has no pointer to give
 * and no way to build one. It pushes bytes instead - the same seam term.c uses
 * for typed commands and fs.c uses for filenames.
 *
 * Overflow REFUSES AND SAYS SO. A clipboard that silently keeps the first
 * 4096 bytes of your selection is worse than one that tells you it could not
 * take it, because you find out at the paste.
 */

typedef unsigned int   u32;
typedef unsigned char  u8;

void zl_putc_pub(char c);

#define CLIP_MAX   4096
#define CLIP_EMPTY 0
#define CLIP_TEXT  1

static u8  buf[CLIP_MAX];
static u32 blen;
static int btype = CLIP_EMPTY;

/* Bumped on every successful put. An app that caches a rendering of the
 * clipboard needs to know it changed, and comparing lengths is not enough -
 * two different 12-byte selections have the same length. */
static u32 bseq;

/* the staging buffer zl fills one character at a time */
static u8  stage[CLIP_MAX];
static u32 stagelen;
static int overflowed;

static void p_str(const char *s) { while (*s) zl_putc_pub(*s++); }
static void p_u32(u32 v)
{
    char b[12]; int i = 0;
    if (!v) { zl_putc_pub('0'); return; }
    while (v) { b[i++] = (char)('0' + (v % 10)); v /= 10; }
    while (i) zl_putc_pub(b[--i]);
}

/* ---- the C side ---------------------------------------------------------- */
int clip_put(const void *p, u32 n, int type)
{
    if (n > CLIP_MAX) {
        p_str("  clipboard: "); p_u32(n);
        p_str(" bytes is more than the "); p_u32(CLIP_MAX);
        p_str("-byte buffer - nothing was copied\n");
        return 0;
    }
    const u8 *s = (const u8 *)p;
    for (u32 i = 0; i < n; i++) buf[i] = s[i];
    blen  = n;
    btype = (n == 0) ? CLIP_EMPTY : type;
    bseq++;
    return 1;
}

int clip_get(void *p, u32 max)
{
    u8 *d = (u8 *)p;
    u32 n = (blen < max) ? blen : max;
    for (u32 i = 0; i < n; i++) d[i] = buf[i];
    return (int)n;
}

u32 clip_len(void)  { return blen; }
int clip_type(void) { return btype; }
u32 clip_seq(void)  { return bseq; }

void clip_clear(void)
{
    blen = 0; btype = CLIP_EMPTY; bseq++;
}

/* one byte out, for a zl app painting a paste preview */
int clip_byte(int i)
{
    if (i < 0 || (u32)i >= blen) return 0;
    return (int)buf[i];
}

/* ---- the zl side: push bytes, then commit -------------------------------- */
void clip_begin(void) { stagelen = 0; overflowed = 0; }

int clip_push(int ch)
{
    if (ch < 0 || ch > 255) return 0;
    if (stagelen >= CLIP_MAX) { overflowed = 1; return 0; }
    stage[stagelen++] = (u8)ch;
    return 1;
}

/* Publishing a truncated selection would be the silent drop this project has
 * been bitten by twice. If anything was pushed past the end, the whole put is
 * refused and the OLD clipboard contents survive - which is the behaviour you
 * want, because the alternative is losing what you had AND not getting what
 * you asked for. */
int clip_commit(int type)
{
    if (overflowed) {
        p_str("  clipboard: selection is larger than the "); p_u32(CLIP_MAX);
        p_str("-byte buffer - nothing was copied, the old contents are intact\n");
        return 0;
    }
    return clip_put(stage, stagelen, type);
}

u32 clip_staged(void) { return stagelen; }

/* ---- the two keys -------------------------------------------------------
 * input.c already produces control codes: Ctrl+C is 3, Ctrl+V is 22. So the
 * plumbing exists and nothing in the routing layer has to change - these
 * arrive at the focused app as an ordinary EV_CHAR, which is exactly what
 * makes a copy in one app and a paste in another work with no coordination
 * between them beyond this file.
 */
#define CLIP_KEY_COPY  3
#define CLIP_KEY_PASTE 22

int clip_is_copy(int ch)  { return ch == CLIP_KEY_COPY; }
int clip_is_paste(int ch) { return ch == CLIP_KEY_PASTE; }
