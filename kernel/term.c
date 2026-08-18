/* term.c - the terminal as an APP, not as the machine.
 *
 * Two problems have to be solved together, and neither is solvable alone.
 *
 * 1. THE SHELL HAD NO MEMORY OF WHAT IT SAID. Every character the kernel
 *    printed went straight to pixels and was then unrecoverable. That is fine
 *    while the shell owns the whole screen and nothing ever repaints - and it
 *    is fatal the moment a window can be dragged across it, because the
 *    compositor repaints from damage and there is nothing to repaint FROM.
 *    So this keeps a scrollback ring, and app_draw redraws from it. The
 *    terminal becomes position-pure like every other app.
 *
 * 2. COMMANDS WERE SINGLE KEYPRESSES. `run_command` dispatches on a character
 *    code - 104 is help, 103 is snake. Typing "help" needs the typed word
 *    compared against a table, and the zl kernel subset has no string VALUES
 *    at all: string literals exist, runtime strings do not. So the matching
 *    lives here, in C, and hands zl back the character code it already knows
 *    how to dispatch. zl never has to compare two strings.
 *
 * The buffer is a plain static array, not one of the fixed high-RAM arenas.
 * That rule is about MULTI-MEGABYTE buffers colliding with the DMA arena; this
 * is 32 KB and belongs in BSS like any ordinary array.
 */

#define TERM_COLS 200
#define TERM_ROWS 160

static char scroll[TERM_ROWS][TERM_COLS];
static int  s_head;                 /* next row to write                  */
static int  s_live;                 /* how many rows hold anything        */
static int  s_col;                  /* chars in the row being built       */

static char input[TERM_COLS];       /* the line being typed               */
static int  in_len;

static int  pending_cmd = -1;       /* set by Enter, taken by zl          */
static int  pending_arg;

void fb_text_prop(int px, int py, const char *s, unsigned int fg);
int  fb_text_prop_w(const char *s);
int  fb_text_prop_h(void);
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);

/* One byte to COM1. See the comment on term_say below for why this file needs
 * the serial port directly rather than going through zl's print. */
void zl_serial_putc(char c);

/* ---- capture ---------------------------------------------------------------
 * Called for EVERY character the kernel prints, from zl_putc. The console
 * still gets it too - this is a tee, not a redirect, so the serial log and the
 * text-mode path are completely unaffected. */
static void commit_row(void)
{
    scroll[s_head][s_col] = 0;
    s_head = (s_head + 1) % TERM_ROWS;
    if (s_live < TERM_ROWS) s_live++;
    s_col = 0;
}

void term_putc(char c)
{
    if (c == '\r') return;
    if (c == '\n') { commit_row(); return; }
    if (c == '\b') { if (s_col > 0) s_col--; return; }
    if (s_col < TERM_COLS - 1) scroll[s_head][s_col++] = c;
    else commit_row();              /* wrap rather than truncate */
}

void term_clear(void)
{
    s_head = 0; s_live = 0; s_col = 0; in_len = 0;
    for (int i = 0; i < TERM_ROWS; i++) scroll[i][0] = 0;
}

/* ---- what THIS file says, as opposed to what it captures ------------------
 * term_putc is a capture sink: it is fed by zl_putc, so everything zl prints
 * is already on the serial log by the time it arrives here. The two messages
 * term.c generates ITSELF are not - the unknown-command line and the echo of
 * the line you typed - and calling term_putc for them put them in the
 * scrollback and NOWHERE else. That made the single most important behaviour
 * in this shell untestable: "an unknown command must SAY SO" could only be
 * checked by looking at a photograph of the screen.
 *
 * So they go to the scrollback AND to the serial log, and deliberately NOT to
 * the console. console_putc draws glyphs into the back buffer at the console's
 * own cursor, which during a compositor session is the OLD static desktop's
 * text region - a rectangle that has nothing to do with the shell window. A
 * message printed that way lands on the wallpaper.
 *
 * Bare LF, no CR, because that is what zl_putc emits and the serial log should
 * not have two conventions in it. */
static void term_say(const char *s)
{
    for (; *s; s++) { term_putc(*s); zl_serial_putc(*s); }
}

/* ---- typing ----------------------------------------------------------------
 * One character per call, from app_event. NO LOOP - that is the whole point:
 * read_line used to block, which is why the shell had to be the top of the
 * system. Returns 1 if this key completed a command. */
static int match_cmd(void);

int term_key(int code)
{
    if (code == 13 || code == 10) {          /* Enter */
        input[in_len] = 0;
        /* echo the typed line into the scrollback, so it reads like a session -
         * and onto the serial log, so a gate can see that the keystroke was
         * received at all. Without the echo there is no evidence a key landed
         * except the command's own output, and a command that produces none
         * (clear) then looks identical to a key that never arrived. */
        term_say("zl> ");
        term_say(input);
        term_say("\n");
        int got = match_cmd();
        in_len = 0;
        input[0] = 0;
        return got;
    }
    if (code == 8 || code == 127) {          /* Backspace */
        if (in_len > 0) in_len--;
        return 0;
    }
    if (code >= 32 && code < 127 && in_len < TERM_COLS - 2) {
        input[in_len++] = (char)code;
    }
    return 0;
}

int term_input_len(void) { return in_len; }

/* ---- the word table --------------------------------------------------------
 * Typed word -> the character code run_command already dispatches on. This is
 * the ONLY place the two vocabularies meet, and it is in C because comparing
 * two runtime strings is the one thing the zl kernel subset cannot do.
 *
 * Every code here is one that run_command in kernel.zl already handles; this
 * adds no behaviour, it only gives the existing commands names. */
struct cmd { const char *word; int code; };

static const struct cmd table[] = {
    { "help",    104 }, { "?",       104 },
    { "fib",     102 }, { "sum",     115 },
    { "uptime",  116 }, { "time",    116 },
    { "beep",    101 },
    { "pci",     107 }, { "hw",      107 },
    { "mode",    110 }, { "res",     110 },
    { "usb",     117 }, { "cpu",     122 },
    { "gpu",     121 }, { "virtio",  121 },
    { "cube",    118 }, { "3d",      118 },
    /* `anim` had no word at all - it was reachable only as the single key 'a'
     * in the old text shell, so under the compositor there was no way to start
     * it. A command in the help list with no way to type it is worse than one
     * that does not exist. */
    { "anim",     97 }, { "logo",     97 },
    /* '*' is not a word, so under the compositor there was no way to wake the
     * other cores at all - and band rendering is off until something does. */
    { "smp",      42 }, { "cores",    42 },
    { "frame",    70 }, { "fps",      70 },
    { "windows", 119 }, { "wm",      119 },
    { "mouse",   120 }, { "snake",   103 },
    { "paint",   100 }, { "edit",    105 },
    { "ls",      108 }, { "files",   108 },
    { "reboot",  114 }, { "halt",    113 }, { "quit",  113 }, { "exit", 113 },
    { "clear",     1 },                       /* handled here, not by zl */
    { 0, 0 }
};

static int streq_n(const char *a, const char *b, int n)
{
    for (int i = 0; i < n; i++) if (a[i] != b[i]) return 0;
    return b[n] == 0;
}

/* Split the typed line into a word and an optional number, then look the word
 * up. Sets pending_cmd/pending_arg for zl to collect. */
static int match_cmd(void)
{
    int i = 0;
    while (input[i] == ' ') i++;
    int start = i;
    while (input[i] && input[i] != ' ') i++;
    int wlen = i - start;
    if (wlen <= 0) return 0;                  /* empty line: no command */

    int arg = 0;
    while (input[i] == ' ') i++;
    while (input[i] >= '0' && input[i] <= '9') arg = arg * 10 + (input[i++] - '0');

    for (int k = 0; table[k].word; k++) {
        if (streq_n(input + start, table[k].word, wlen)) {
            if (table[k].code == 1) { term_clear(); return 0; }
            pending_cmd = table[k].code;
            pending_arg = arg;
            return 1;
        }
    }

    /* An unknown command must SAY SO. A shell that silently ignores what you
     * typed is worse than one that has no commands at all - which is why this
     * is the assertion probe-term.py cares about most, and why it goes to the
     * serial log rather than only into the scrollback. */
    term_say("  unknown command: ");
    {
        char word[TERM_COLS];
        int n = 0;
        for (int k = start; k < start + wlen && n < TERM_COLS - 1; k++)
            word[n++] = input[k];
        word[n] = 0;
        term_say(word);
    }
    term_say("\n  type 'help'\n");
    return 0;
}

int term_cmd(void) { int c = pending_cmd; pending_cmd = -1; return c; }
int term_arg(void) { return pending_arg; }

/* ---- drawing ---------------------------------------------------------------
 * POSITION-PURE, as the app contract requires: every coordinate is derived
 * from the x,y,w,h passed in. Draws the last rows that fit, bottom-anchored,
 * then the prompt and the line being typed. */
void term_draw(int x, int y, int w, int h, unsigned int fg, unsigned int dim,
               unsigned int accent, int cursor_on)
{
    int lh = fb_text_prop_h();
    if (lh <= 0) return;
    int rows = h / lh;
    if (rows < 1) return;

    /* one row is reserved for the prompt at the bottom */
    int show = rows - 1;
    if (show > s_live) show = s_live;

    /* walk back `show` committed rows from the newest */
    int first = s_head - show;
    while (first < 0) first += TERM_ROWS;

    int ty = y;
    for (int r = 0; r < show; r++) {
        const char *line = scroll[(first + r) % TERM_ROWS];
        if (line[0]) fb_text_prop(x, ty, line, dim);
        ty += lh;
    }

    /* the prompt line, always at the bottom of the client area */
    int py = y + h - lh;
    fb_text_prop(x, py, "zl>", accent);
    int px = x + fb_text_prop_w("zl> ");
    input[in_len] = 0;
    if (in_len) fb_text_prop(px, py, input, fg);
    if (cursor_on) {
        int cw = fb_text_prop_w("n");
        fb_fill_px(px + fb_text_prop_w(input), py, cw, lh, accent);
    }
}
