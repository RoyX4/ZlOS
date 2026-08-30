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

#include "telemetry.h"
#include "keycodes.h"

#define TERM_COLS 200
#define TERM_ROWS 160

static char scroll[TERM_ROWS][TERM_COLS];
static int  s_head;                 /* next row to write                  */
static int  s_live;                 /* how many rows hold anything        */
static int  s_col;                  /* chars in the row being built       */

static char input[TERM_COLS];       /* the line being typed               */
static int  in_len;
static int  in_cursor;              /* insertion point, 0..in_len          */

/* A terminal that accepts text but silently discards Left/Right/Up/Down is
 * not a line editor.  The physical 2026-08-24 trace showed exactly that: the
 * ThinkPad delivered the navigation keys cleanly while the Terminal threw
 * them away above this file.  Keep a small bounded history here, beside the
 * line it edits.  No allocation and no filesystem dependency are involved. */
#define TERM_HISTORY 16
static char history[TERM_HISTORY][TERM_COLS];
static int  history_len[TERM_HISTORY];
static int  history_head;           /* next slot to replace                */
static int  history_count;
static int  history_view = -1;      /* age: 0 newest, -1 current draft     */
static char history_draft[TERM_COLS];
static int  history_draft_len;
static int  history_draft_cursor;

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
 * and wrong for a terminal, and the split is exactly the one docs/desktop/desktop-look.md
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

/* THE SCROLLBACK, READABLE.
 *
 * The Kernel Log app drew FOURTEEN FIXED ROWS with hand-written timestamps -
 * kl_time() returned 2, 11, 17, 34, 48, 63, 95, 140 - and presented them as
 * this machine's boot log. A whole pane of fiction shaped exactly like
 * measurement, in a system whose argument is that its instruments are read.
 *
 * The real thing was already here. term_putc is tee'd from every kernel print,
 * so this ring holds what the machine actually said, in the order it said it.
 * zllog cannot serve: it is a telemetry emitter to a host - counters and events
 * - with no stored text and no read-back at all.
 *
 * Line 0 is the OLDEST live row, which is the order a log is read in, and the
 * reverse of term_draw's own walk (it counts back from s_head to bottom-anchor
 * the newest against the prompt). Both are correct for their caller; they must
 * not be confused, so the ring arithmetic lives here once. */
int term_lines(void) { return s_live; }

int term_ch(int line, int col)
{
    if (line < 0 || line >= s_live) return 0;
    if (col < 0 || col >= TERM_COLS) return 0;
    int first = s_head - s_live;
    while (first < 0) first += TERM_ROWS;
    return (unsigned char)scroll[(first + line) % TERM_ROWS][col];
}

void term_clear(void)
{
    s_head = 0; s_live = 0; s_col = 0; in_len = 0; in_cursor = 0;
    history_view = -1;
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

static void line_load(const char *src, int len, int cursor)
{
    if (len < 0) len = 0;
    if (len > TERM_COLS - 2) len = TERM_COLS - 2;
    for (int i = 0; i < len; i++) input[i] = src[i];
    input[len] = 0;
    in_len = len;
    in_cursor = cursor < 0 ? 0 : cursor > len ? len : cursor;
}

static void history_remember(void)
{
    if (in_len <= 0) return;
    if (history_count > 0) {
        int newest = (history_head + TERM_HISTORY - 1) % TERM_HISTORY;
        if (history_len[newest] == in_len) {
            int same = 1;
            for (int i = 0; i < in_len; i++)
                if (history[newest][i] != input[i]) { same = 0; break; }
            if (same) return;                 /* consecutive duplicate */
        }
    }
    for (int i = 0; i < in_len; i++) history[history_head][i] = input[i];
    history[history_head][in_len] = 0;
    history_len[history_head] = in_len;
    history_head = (history_head + 1) % TERM_HISTORY;
    if (history_count < TERM_HISTORY) history_count++;
}

static void history_up(void)
{
    if (history_count <= 0) return;
    if (history_view < 0) {
        for (int i = 0; i < in_len; i++) history_draft[i] = input[i];
        history_draft[in_len] = 0;
        history_draft_len = in_len;
        history_draft_cursor = in_cursor;
        history_view = 0;
    } else if (history_view + 1 < history_count) {
        history_view++;
    }
    int slot = (history_head + TERM_HISTORY - 1 - history_view) % TERM_HISTORY;
    line_load(history[slot], history_len[slot], history_len[slot]);
}

static void history_down(void)
{
    if (history_view < 0) return;
    if (history_view > 0) {
        history_view--;
        int slot = (history_head + TERM_HISTORY - 1 - history_view) % TERM_HISTORY;
        line_load(history[slot], history_len[slot], history_len[slot]);
    } else {
        line_load(history_draft, history_draft_len, history_draft_cursor);
        history_view = -1;
    }
}

static void history_leave(void) { history_view = -1; }

void term_submit(int command, int argument, int word_len)
{
    zlt_event(ZLLOG_SUB_KERNEL, ZLLOG_EV_COMMAND_SUBMIT, ZLLOG_INFO,
              (unsigned)command, (unsigned)argument, (unsigned)word_len);
}

void term_complete(int command, int result)
{
    zlt_event(ZLLOG_SUB_KERNEL, ZLLOG_EV_COMMAND_COMPLETE,
              result ? ZLLOG_WARN : ZLLOG_INFO,
              (unsigned)command, (unsigned)result, 0u);
}

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
        history_remember();
        int got = match_cmd();
        in_len = 0;
        in_cursor = 0;
        history_view = -1;
        input[0] = 0;
        return got;
    }
    if (code == 8 || code == 127) {          /* Backspace */
        history_leave();
        if (in_cursor > 0) {
            for (int i = in_cursor - 1; i < in_len; i++) input[i] = input[i + 1];
            in_cursor--;
            in_len--;
        }
        return 0;
    }
    if (code == KEY_DELETE) {
        history_leave();
        if (in_cursor < in_len) {
            for (int i = in_cursor; i < in_len; i++) input[i] = input[i + 1];
            in_len--;
        }
        return 0;
    }
    if (code == KEY_LEFT)  { if (in_cursor > 0) in_cursor--; return 0; }
    if (code == KEY_RIGHT) { if (in_cursor < in_len) in_cursor++; return 0; }
    if (code == KEY_HOME)  { in_cursor = 0; return 0; }
    if (code == KEY_END)   { in_cursor = in_len; return 0; }
    if (code == KEY_UP)    { history_up(); return 0; }
    if (code == KEY_DOWN)  { history_down(); return 0; }
    if (code >= 32 && code < 127 && in_len < TERM_COLS - 2) {
        history_leave();
        for (int i = in_len; i > in_cursor; i--) input[i] = input[i - 1];
        input[in_cursor++] = (char)code;
        in_len++;
        input[in_len] = 0;
    }
    return 0;
}

int term_input_len(void) { return in_len; }
int term_input_cursor(void) { return in_cursor; }

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
    { "format",   46 }, { "mkfs",     46 }, /* explicit destructive zlfs init */
    { "sched",    43 }, { "tasks",    43 },
    { "smp",      42 }, { "cores",    42 },
    { "usbstor",  47 }, { "stor",     47 },
    /* Flight-recorder controls dispatch into zl. This file never touches the
     * USB device or persistent journal itself. */
    { "diag",     200 }, { "diagsave", 201 },
    { "wifi",     202 }, { "ax201",    202 },
    { "i2c",      63 }, { "touchpad", 63 },
    { "input",    61 }, { "events",   61 },
    { "panel",    80 },                       /* lights the real panel - laptop */
    /* OUR OWN ring on the blitter, not virtio's. "gpu" was already taken by
     * virtio_gpu (121) - this is the Intel one, and it is the whole reason to
     * boot this from a USB stick. See kernel/docs/drivers/display/gpu-driver.md. */
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
    /* The prototype's command palette. It is a MODAL, not a window - see the
     * overlay layer in kernel.zl - so it has no register slot to open from. */
    { "palette", 203 }, { "cmdp",    203 },
    /* The field's context menu, at a fixed point. A right-click opens it in
     * use; this exists so a probe that can only type can still render it - the
     * same reason `windows` exists beside the compositor itself. */
    { "ctxmenu", 204 }, { "menu2",   204 },
    { "activities", 205 }, { "overview", 205 },
    { "lock",     206 }, { "locksession", 206 },
    /* THE REGISTER'S APPS, one word each. The rail opens them with a click and
     * the palette names them, but a probe can only type - and an app whose body
     * has been rebuilt to the reference cannot be checked against the reference
     * without a way to put it on screen. */
    { "klog",     210 }, { "dmesg",    210 },
    { "hexv",     211 }, { "calcapp",  212 },
    { "netapp",   213 }, { "clocks",   214 },
    { "disku",    215 }, { "sysinfo",  216 },
    { "typepane", 217 }, { "specimen", 217 },
    { "syspane",  218 },
    /* THE FONT ATLAS HAD NO WORD AT ALL - reachable only by opening the
     * catalogue and finding its tile. Every other pane in this block got one
     * for the reason the comment above gives: a pane that cannot be put on
     * screen cannot be checked against the reference. This one was missed. */
    { "font",     221 }, { "atlas",    221 },
    /* AND NEITHER DID THE RENDERER. Same gap, same reason it went unnoticed:
     * `cube` and `3d` at :352 open the CUBE APP, which is a different window,
     * so a grep for a 3D word found one and stopped. The Renderer's canvas was
     * a flat fill for as long as nothing could type its way to it. */
    { "render",   222 }, { "mesh",     222 },
    /* AND NEITHER DID THE IMAGE VIEWER. Three of this register's panes had no
     * word between them; each was found only when something needed to be
     * checked against the reference and there was no way to put it on screen.
     * That is the cost of the gap, and it is why they are being closed as they
     * are found rather than "later". */
    { "image",    223 }, { "img",      223 },
    /* The Archive Manager, the fourth. */
    { "archive",  224 }, { "tar",      224 },
    { "sysmon",   219 }, { "monitor",  219 },
    { "settings", 220 }, { "prefs",    220 },
    { "mouse",   120 }, { "snake",   103 },
    { "paint",   100 }, { "edit",    105 },
    { "anim",     97 }, { "demo",     97 },
    { "ls",      108 },
    /* `ls` is the textual zlfs listing; `files` opens the graphical manager. */
    { "files",    77 }, { "filemgr",  77 }, { "explorer", 77 },
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
    { "perf",     13 },                         /* raw percentile samples */
    { "userexec", 14 }, { "ring3", 14 },       /* /system/user.bin */
    { "reboot",  114 }, { "halt",    113 }, { "quit",  113 }, { "exit", 113 },
    /* 82 is 'R'. Lower-case 'r' (114) is already reboot, and the exec track
     * needs a code run_command dispatches on that nothing else claims. */
    { "run",      82 },
    /* Destructive by design and deliberately absent from ordinary help. The
     * exact word is the target proof route for the CPU-fault recorder; code
     * 127 cannot be produced by the printable single-key text shell. */
    { "crashtest", 127 },
    /* Persistent zlfs. The kernel documents and dispatches single-key '.' as
     * code 46, but the compositor owns input after graphical boot, so leaving
     * it out here made the documented mount path unreachable. Keep the exact
     * spelling and a discoverable word alias. */
    { ".",         46 }, { "mount",    46 },
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
            if (table[k].code == 1) {
                term_submit(1, arg, wlen);
                term_clear();
                term_complete(1, 0);
                return 0;
            }
            pending_cmd = table[k].code;
            /* `diag save` is the readable form. The raw argument already
             * lives here in C, where strings are real values; zl deliberately
             * receives only the resulting command code. `diagsave` remains a
             * no-argument recovery alias if a damaged line editor ever loses
             * the space. */
            if (pending_cmd == 200 &&
                input[astart + 0] == 's' && input[astart + 1] == 'a' &&
                input[astart + 2] == 'v' && input[astart + 3] == 'e' &&
                input[astart + 4] == 0)
                pending_cmd = 201;
            pending_arg = arg;
            term_submit(pending_cmd, pending_arg, wlen);
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
     * typed is worse than one that has no commands at all - which is why this
     * is the assertion probe-term.py cares about most, and why it goes to the
     * serial log rather than only into the scrollback. */
    last_unknown = 1;
    zlt_event(ZLLOG_SUB_KERNEL, ZLLOG_EV_DROP, ZLLOG_WARN,
              40u /* unknown shell command */, (unsigned)wlen, 0u);
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

/* THE HANGING INDENT for continuation rows.
 *
 * `help` prints a two-column table - a command, a gutter of spaces, then the
 * description - and a continuation row starting at column 0 reads as a NEW
 * command rather than as the rest of the previous sentence. That is what put
 * "back", "l" and "LPSS I2C)" at the left margin: three orphaned fragments of
 * three different descriptions, each looking like a command with no help text.
 *
 * The description column is the first non-space after the first run of two or
 * more spaces that follows a non-space character. A line with no such run -
 * ordinary prose, a banner, an echoed command - keeps its OWN leading indent
 * instead, so nothing that is not a table acquires a table's shape.
 *
 * Capped at a third of the width, because a deeply indented table in a narrow
 * window would otherwise leave almost no room for the text being indented. */
static int hang_indent(const char *s, int cols)
{
    int i = 0;
    while (s[i] == ' ') i++;
    int lead = i;
    for (; s[i]; i++) {
        if (s[i] == ' ' && s[i + 1] == ' ') {
            int j = i;
            while (s[j] == ' ') j++;
            if (s[j]) lead = j;
            break;
        }
    }
    int cap = cols / 3;
    if (lead > cap) lead = cap;
    return lead;
}

/* ONE display row of `s` starting at byte `off`, with `room` columns to fill.
 * Returns how many bytes to DRAW, and sets *next to where the following row
 * begins - which steps over the spaces the break consumed, so a wrapped table
 * never opens its continuation with the tail of the gutter.
 *
 * IT BREAKS ON A WORD. The old code chopped at exactly `cols`, mid-word,
 * because "does it fit" was the only question being asked.
 *
 * A single token longer than the row still hard-breaks: the alternative is
 * drawing past the client edge and letting the scissor cut a glyph in half,
 * which is the defect this entire path exists to prevent.
 *
 * *next is always > off while off < n, so no caller can stall on it. */
static int seg_len(const char *s, int n, int off, int room, int *next)
{
    if (n - off <= room) { *next = n; return n - off; }
    int brk = -1;
    for (int p = off + room; p > off; p--)
        if (s[p] == ' ') { brk = p; break; }
    if (brk < 0) { *next = off + room; return room; }   /* one long token */
    int nx = brk;
    while (s[nx] == ' ') nx++;
    *next = nx;
    int len = brk - off;
    while (len > 0 && s[off + len - 1] == ' ') len--;   /* no trailing gutter */
    return len;
}

/* How many display rows one stored line needs at `cols` columns. An EMPTY line
 * still needs one - it is a blank row on screen, not an absence - and that is
 * what keeps this in step with the drawing loop below, which also emits one
 * (blank) row for it.
 *
 * THIS MUST SEGMENT IDENTICALLY TO THE DRAWING LOOP, which is why both now go
 * through seg_len and hang_indent rather than each doing their own arithmetic.
 * The count here is what bottom-anchors the view; if it disagrees with what is
 * actually drawn, the newest line slides off the bottom edge by the difference. */
static int wrapped_rows(const char *s, int cols)
{
    int n = 0;
    while (s[n]) n++;
    if (n <= cols) return 1;
    int ind = hang_indent(s, cols);
    int rows = 0, off = 0;
    do {
        int room = rows ? cols - ind : cols;
        if (room < 1) room = 1;
        int next;
        seg_len(s, n, off, room, &next);
        rows++;
        if (next <= off) break;
        off = next;
    } while (off < n);
    return rows ? rows : 1;
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
        int ind = hang_indent(line, cols);
        int off = 0, si = 0;
        do {
            int room = si ? cols - ind : cols;
            if (room < 1) room = 1;
            int next;
            int len = seg_len(line, n, off, room, &next);
            if (skip > 0) {                  /* above the band - not drawn, and
                                              * ty must NOT advance for it */
                skip--;
            } else {
                if (len > 0 && ty + lh > c_top && ty < c_bot) {
                    for (int i = 0; i < len; i++) seg[i] = line[off + i];
                    seg[len] = 0;
                    fb_text_aa(x + (si ? ind * cw : 0), ty, seg, dim);
                }
                ty += lh;
            }
            si++;
            if (next <= off) break;
            off = next;
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
    if (in_cursor >= avail) from = in_cursor - avail + 1;
    input[in_len] = 0;
    if (in_len > from) {
        char shown[TERM_COLS];
        int n = in_len - from;
        if (n > avail) n = avail;
        for (int i = 0; i < n; i++) shown[i] = input[from + i];
        shown[n] = 0;
        fb_text_aa(px, py, shown, fg);
    }
    if (cursor_on) fb_fill_px(px + (in_cursor - from) * cw, py, cw, lh, accent);
}
