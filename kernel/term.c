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
/* Set when a typed word matched nothing. zl reads it to pulse the window - a
 * refusal you can SEE beats one you have to read, and the message scrolls. */
static int  last_unknown;

void fb_text_prop(int px, int py, const char *s, unsigned int fg);
int  fb_text_prop_w(const char *s);
int  fb_text_prop_h(void);
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);

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

/* ---- typing ----------------------------------------------------------------
 * One character per call, from app_event. NO LOOP - that is the whole point:
 * read_line used to block, which is why the shell had to be the top of the
 * system. Returns 1 if this key completed a command. */
static int match_cmd(void);

/* The kernel's one character sink: console (muted while the compositor owns
 * the screen), this file's scrollback, and COM1. The echo below goes through
 * it rather than through term_putc so that the SERIAL LOG still reads like a
 * session - every gate in this repo greps that log for "zl> ", and with the
 * console muted a scrollback-only echo would make the prompt invisible to all
 * of them the moment the desktop became the boot state. */
void zl_putc_pub(char c);

int term_key(int code)
{
    if (code == 13 || code == 10) {          /* Enter */
        input[in_len] = 0;
        /* The prefix goes to the SCROLLBACK only and the typed characters go
         * to both. That asymmetry is deliberate and it is what makes the two
         * transcripts agree: the serial log was already given a bare "zl> "
         * when this line was invited, so echoing the prefix there too would
         * read "zl> zl> help". The window was not, because its live prompt is
         * drawn by term_draw at the bottom rather than printed. */
        term_putc('z'); term_putc('l'); term_putc('>'); term_putc(' ');
        for (int i = 0; i < in_len; i++) zl_putc_pub(input[i]);
        zl_putc_pub('\n');
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
 * adds no behaviour, it only gives the existing commands names.
 *
 * COMPLETENESS IS THE POINT, not convenience. Ten of run_command's commands
 * had no name here - cpuid, poke, usbkbd, nvme, sched, smp, usbstor, i2c,
 * input and bars - which was invisible while single keypresses still worked
 * and became a straight capability regression the moment the compositor
 * became the boot state: those ten could be reached from the text shell and
 * from nothing else. Anything run_command dispatches should be typeable. */
struct cmd { const char *word; int code; };

static const struct cmd table[] = {
    { "help",    104 }, { "?",       104 },
    { "fib",     102 }, { "sum",     115 },
    { "uptime",  116 }, { "time",    116 },
    { "beep",    101 }, { "bars",     98 }, { "colours", 98 }, { "colors", 98 },
    { "pci",     107 }, { "hw",      107 },
    { "mode",    110 }, { "res",     110 },
    { "usb",     117 }, { "cpu",     122 },
    { "cpuid",   112 }, { "brand",   112 },
    { "poke",    109 }, { "peek",    109 },
    { "usbkbd",  106 }, { "kbd",     106 },
    { "nvme",    111 }, { "disk",    111 },
    { "sched",    43 }, { "tasks",    43 },
    { "smp",      42 }, { "cores",    42 },
    { "usbstor",  47 }, { "stor",     47 },
    { "i2c",      63 }, { "touchpad", 63 },
    { "input",    61 }, { "events",   61 },
    { "panel",    80 },                       /* lights the real panel - laptop */
    { "gpu",     121 }, { "virtio",  121 },
    { "cube",    118 }, { "3d",      118 },
    { "windows", 119 }, { "wm",      119 },
    { "mouse",   120 }, { "snake",   103 },
    { "paint",   100 }, { "edit",    105 },
    { "anim",     97 }, { "demo",     97 },
    { "ls",      108 }, { "files",   108 },
    { "redraw",   99 },
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
     * typed is worse than one that has no commands at all. */
    last_unknown = 1;
    const char *msg = "  unknown command: ";
    while (*msg) term_putc(*msg++);
    for (int k = start; k < start + wlen; k++) term_putc(input[k]);
    term_putc('\n');
    msg = "  type 'help'\n";
    while (*msg) term_putc(*msg++);
    return 0;
}

int term_cmd(void) { int c = pending_cmd; pending_cmd = -1; return c; }
int term_unknown(void) { int u = last_unknown; last_unknown = 0; return u; }
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
