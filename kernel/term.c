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
static char argstr[TERM_COLS];      /* ...and the argument as TEXT, for C  */

/* A TERMINAL IS A GRID, and this drew with the PROPORTIONAL font.
 *
 * Every column-aligned thing the shell prints - the help table, the PCI dump,
 * the CPUID report - is aligned with SPACES, which only lines up when every
 * character is the same width. In a proportional face the columns come out
 * ragged, and it reads as "the formatting is broken" rather than as "the font
 * is wrong", which is why it survived.
 *
 * fb_text_prop is right for a LABEL - a dock tile, a window title, a menu row -
 * and wrong for a terminal, and the split is exactly the one desktop-look.md
 * item 4 draws: uniform advance is the "this is a terminal" signal, so use it
 * on the one thing that IS a terminal. */
void fb_text_aa(int px, int py, const char *s, unsigned int fg);
/* the scissor, so a row that cannot be seen is not drawn - see below */
int  fb_clip_top(void);
int  fb_clip_bot(void);
void fb_text_aa2x(int px, int py, const char *s, unsigned int fg);
int  fb_cell_w(void);
int  fb_cell_h(void);
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
void term_say(const char *s)
{
    for (; *s; s++) { term_putc(*s); zl_serial_putc(*s); }
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
    /* OUR OWN ring on the blitter, not virtio's. "gpu" was already taken by
     * virtio_gpu (121) - this is the Intel one, and it is the whole reason to
     * boot this from a USB stick. See kernel/docs/gpu-driver.md. */
    { "blit",     71 }, { "ring",     71 },
    { "gpu",     121 }, { "virtio",  121 },
    { "cube",    118 }, { "3d",      118 },
    /* ALL APPLICATIONS. The catalog is the only route to 47 of the 53 apps and
     * had no typed word at all - the same defect the `anim` comment below
     * describes, with forty-seven times the consequence. It also had no working
     * pointer route after the top bar became the reference's floating island,
     * so for a while it had none of either. */
    { "apps",     65 }, { "catalog",  65 },
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
    { "anim",     97 }, { "demo",     97 },
    { "ls",      108 }, { "files",   108 },
    /* NOT "files" - that word already means "ls" above, the ten numbered RAM
     * slots. This is the OTHER one: the zlfs Files window, by name. */
    { "filemgr",  77 }, { "explorer", 77 },
    /* THE SAME REGRESSION THIS TABLE'S HEADER DESCRIBES, HAPPENING AGAIN.
     * 78/87/69 are 'N', 'W' and 'E' - the network card + ARP probe, TCP +
     * HTTP/1.0 against a real server, and a real website by name off the
     * internet. run_command dispatches all three (kernel.zl:2124-2126) and
     * none of them had a word, so from the moment the compositor became the
     * boot state there was NO WAY TO RUN ANY OF THEM: the text shell's loop is
     * never entered when there is a framebuffer (kernel.zl:3901), and
     * wm_frame() reads PS/2 and USB HID only - nothing in the compositor path
     * looks at COM1. Measured, not reasoned: with a NIC attached, sending 'N'
     * down the serial socket at the "zl> " prompt produces no output at all,
     * and neither does 'h'.
     * This is why the header says completeness is the point. */
    { "net",      78 }, { "arp",      78 },
    { "web",      87 }, { "http",     87 },
    { "fetch",    69 }, { "site",     69 },
    { "https",    83 }, { "tls",      83 },   /* the same fetch, verified */
    { "redraw",   99 },
    { "peak",     11 }, { "peakreset", 12 },   /* the frame timer */
    { "reboot",  114 }, { "halt",    113 }, { "quit",  113 }, { "exit", 113 },
    /* 82 is 'R'. Lower-case 'r' (114) is already reboot, and the exec track
     * needs a code run_command dispatches on that nothing else claims. */
    { "run",      82 },
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

    /* THE ARGUMENT, BOTH WAYS, AND FROM THE SAME START POINT.
     *
     * `arg` has always been a decimal number, which is all `fib 20` and
     * `edit 3` ever needed. A filename is not a number, so the raw text is
     * captured too - and both are read from the SAME offset rather than the
     * text picking up where the digit scan stopped. That ordering is the whole
     * correctness of it: `run 2048.zl` would otherwise let the digit loop eat
     * "2048" and leave the text as ".zl", which is a file-not-found that reads
     * like a typo. Taken from the same start, arg is 2048 and argstr is
     * "2048.zl", each right for whoever asked.
     *
     * No existing command's behaviour changes - `arg` is parsed from exactly
     * the offset it always was. */
    int arg = 0;
    while (input[i] == ' ') i++;
    int astart = i;
    while (input[i] >= '0' && input[i] <= '9') arg = arg * 10 + (input[i++] - '0');

    for (int k = 0; table[k].word; k++) {
        if (streq_n(input + start, table[k].word, wlen)) {
            if (table[k].code == 1) { term_clear(); return 0; }
            pending_cmd = table[k].code;
            pending_arg = arg;
            /* ...and the same argument as TEXT, read from the SAME offset the
             * digit scan started at, not from where it stopped. That ordering
             * is the whole correctness of it: `run 2048.zl` would otherwise
             * leave the text as ".zl" - a file-not-found that reads like a
             * typo - while arg quietly took the 2048. From one start point,
             * arg is 2048 and argstr is "2048.zl", each right for whoever
             * asked, and no existing command's `arg` changes by a digit. */
            {
                int n = 0;
                while (input[astart + n] && n < TERM_COLS - 1) {
                    argstr[n] = input[astart + n];
                    n++;
                }
                argstr[n] = 0;
            }
            return 1;
        }
    }

    /* An unknown command must SAY SO. A shell that silently ignores what you
    last_unknown = 1;
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
int term_unknown(void) { int u = last_unknown; last_unknown = 0; return u; }
int term_arg(void) { return pending_arg; }

/* The argument as TEXT. Deliberately not routed through zl: runtime_kernel.c
 * hard-faults on any string operand (:562, before the `==` arm at :573), so a
 * filename that reaches zl compiles clean and halts the machine. C callers
 * read it here; zl only ever sees the int. */
const char *term_argstr(void) { return argstr; }

/* How many display rows one stored line needs at `cols` columns. An EMPTY line
 * still needs one - it is a blank row on screen, not an absence - and that is
 * what keeps this in step with the drawing loop below, which also emits one
 * (blank) row for it. */
static int wrapped_rows(const char *s, int cols)
{
    int n = 0;
    while (s[n]) n++;
    if (n <= cols) return 1;
    return (n + cols - 1) / cols;
}

/* ---- drawing ---------------------------------------------------------------
 * POSITION-PURE, as the app contract requires: every coordinate is derived
 * from the x,y,w,h passed in. Draws the last rows that fit, bottom-anchored,
 * then the prompt and the line being typed.
 *
 * IT WRAPS, and that is DECISIONS.md open item G. At 1920x1200 the shell client
 * is 1236 px wide, the monospace cell is 16 px, so 77 columns fit - and the
 * longest line `help` prints is 82 (kernel.zl:627, the i2c row). Five
 * characters were being drawn past the client edge and cut mid-glyph by the
 * scissor, which the northstar names outright as a thing that must not ship.
 *
 * WIDTH WOULD NOT HAVE FIXED IT. A wider boot window fixes 1920x1200 and
 * nothing else: the window has a resize grip, `mode` changes the screen under
 * it, and `cols` is w/cell_w at whatever size the user just dragged it to. The
 * defect is "a line longer than the window", and only wrapping answers that at
 * every width. The font stays monospace - that half was settled and measured
 * (POINTER-PROMPT §1c): three space-aligned tables depend on a uniform advance,
 * and a terminal is monospace. */
void term_draw(int x, int y, int w, int h, unsigned int fg, unsigned int dim,
               unsigned int accent, int cursor_on)
{
    int lh = fb_cell_h();
    int cw = fb_cell_w();
    if (lh <= 0 || cw <= 0) return;
    int rows = h / lh;
    if (rows < 1) return;

    int view_cols = w / cw;                  /* what the WINDOW can show */
    if (view_cols < 1) return;               /* narrower than one glyph */
    /* CLAMP, because `seg` below is one stored line and no wider. A 3840-px
     * client at a 16-px cell is 240 columns and would run 40 bytes off the end
     * of it. Nothing is lost for the SCROLLBACK by clamping: term_putc (:85)
     * refuses to store past TERM_COLS - 1, so no stored line can be longer.
     *
     * The two are kept apart because the PROMPT is not a stored line - `input`
     * is 198 characters and does not wrap, it scrolls, so on a window wider
     * than 203 cells clamping would scroll a line that fits. */
    int cols = view_cols;
    if (cols > TERM_COLS - 1) cols = TERM_COLS - 1;

    /* one row is reserved for the prompt at the bottom */
    int show = rows - 1;

    /* WALK BACK FROM THE NEWEST LINE, counting DISPLAY rows rather than stored
     * ones, until the visible band is full. With wrapping the two are no longer
     * the same number - one `help` row can be two rows on screen - so the old
     * `first = s_head - show` arithmetic would scroll the newest line off the
     * bottom by however many lines happened to wrap. */
    int nlines = 0, drows = 0;
    while (nlines < s_live && drows < show) {
        int idx = s_head - 1 - nlines;
        while (idx < 0) idx += TERM_ROWS;
        drows += wrapped_rows(scroll[idx], cols);
        nlines++;
    }
    /* the oldest line reached may only be PARTLY visible: drop that many of its
     * leading segments so the newest row still lands against the prompt */
    int skip = drows - show;
    if (skip < 0) skip = 0;

    int first = s_head - nlines;
    while (first < 0) first += TERM_ROWS;

    /* SKIP THE ROWS THAT CANNOT BE SEEN.
     *
     * app_draw is called once per damage rectangle, and a window being dragged
     * across this one damages a band of it - typically two or three rows out
     * of thirty-seven. Every other row was being walked, laid out and blitted
     * glyph by glyph, and then rejected a pixel at a time by the scissor.
     * fbbench measures forty lines of AA text at 4.588 ms, so a drag across
     * the shell was paying most of that per frame to draw nothing.
     *
     * The scissor is still what GUARANTEES nothing escapes; this is about not
     * doing the work first. Same reason ui_scroll rejects rows outside its
     * viewport rather than drawing them and clipping. */
    int c_top = fb_clip_top(), c_bot = fb_clip_bot();
    int ty = y;
    char seg[TERM_COLS];
    for (int r = 0; r < nlines; r++) {
        const char *line = scroll[(first + r) % TERM_ROWS];
        int n = 0;
        while (line[n]) n++;
        int off = 0;
        do {
            int len = n - off;
            if (len > cols) len = cols;
            if (skip > 0) {                  /* above the band - not drawn, and
                                              * ty must NOT advance for it */
                skip--;
            } else {
                if (len > 0 && ty + lh > c_top && ty < c_bot) {
                    for (int i = 0; i < len; i++) seg[i] = line[off + i];
                    seg[len] = 0;
                    fb_text_aa(x, ty, seg, dim);
                }
                ty += lh;
            }
            off += cols;
        } while (off < n);
    }

    /* the prompt line, always at the bottom of the client area */
    int py = y + h - lh;
    if (py + lh <= c_top || py >= c_bot) return;      /* not in this band */
    fb_text_aa(x, py, "zl>", accent);
    int px = x + 4 * cw;

    /* THE TYPED LINE SCROLLS SIDEWAYS instead of running off the edge - the
     * same defect as the scrollback and the same fix, but wrapping is wrong
     * here: the prompt owns exactly one row, and a line that grew downward
     * would walk up over the scrollback. `input` holds up to TERM_COLS - 2 =
     * 198 characters and the client is 77 columns wide, so this is reachable by
     * typing, not a theoretical case. Anchored on the CURSOR, which is the only
     * part you have to be able to see while typing. */
    int avail = view_cols - 4;               /* "zl>" and a space. view_cols,
                                              * not cols - see the note above */
    if (avail < 1) avail = 1;
    int from = 0;
    if (in_len >= avail) from = in_len - avail + 1;
    input[in_len] = 0;
    if (in_len > from) fb_text_aa(px, py, input + from, fg);
    if (cursor_on) fb_fill_px(px + (in_len - from) * cw, py, cw, lh, accent);
}
