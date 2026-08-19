/* exectest.c - `run`, and every way it declines, asserted.
 *
 * exec.c is compiled here UNMODIFIED. Everything it talks to outside itself is
 * a handful of functions, so stubbing those turns it into an ordinary program -
 * the same trick inputtest.c plays on input.c.
 *
 * THIS GATE EXISTS BECAUSE THE FAILURE PATHS ARE THE ONLY PATHS.
 *
 * Nothing can execute a program yet, so every route through exec_run() ends in
 * a refusal, and a refusal is exactly the kind of thing that rots unnoticed:
 * "it printed something" looks like success from a distance, and the difference
 * between "no filesystem driver in this kernel" and "no filesystem on the disk"
 * is invisible unless somebody asserts on the actual words. runtime_kernel.c's
 * own comment records that conflating those two for the USB pointer "cost a
 * long hunt once already".
 *
 * TWO BINARIES FROM ONE SOURCE, and that is the point of the weak seam:
 *
 *   exectest         a fake filesystem IS linked  -> mounted/not-mounted,
 *                    found/not-found, empty, too-big, and the load path
 *   exectest-nofs    nothing defines fs_*         -> the weak symbols are
 *                    NULL, and exec.c reports "no fs driver"
 *
 * UPDATE, and it is the point of the weak seam rather than a problem with it:
 * fs.c has since been merged from desktop/system-track, so the SHIPPING kernel
 * is now the first configuration, not the second. exec.c did not change by a
 * character - the message it prints changed by itself when the symbols found
 * something to bind to.
 *
 * exectest-nofs stays anyway. It is the only thing that proves the NULL-weak
 * branch is REACHED rather than merely written, and that branch is what any
 * future build without fs.c - the EFI target, a trimmed kernel, a bisect -
 * still lands on. A branch with no test is a branch that rots.
 *
 * Build and run:  ./build.sh && ./exectest && ./exectest-nofs
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>

typedef unsigned int u32;

/* ---- exec.c's interface --------------------------------------------------*/
int  exec_run(void);
int  exec_state(void);
int  exec_wants_window(void);
const char *exec_name(void);
const char *exec_title(void);
u32  exec_runs(void);
void exec_draw(int x, int y, int w, int h, unsigned fg, unsigned dim, unsigned accent);

#define EX_IDLE       0
#define EX_NO_NAME    1
#define EX_NO_DRIVER  2
#define EX_NO_FS      3
#define EX_NOT_FOUND  4
#define EX_EMPTY      5
#define EX_TOO_BIG    6
#define EX_LOADED     7

/* ---- the seam: term.c ----------------------------------------------------*/
static char typed[256];
static char out[1 << 14];
static size_t outn;

const char *term_argstr(void) { return typed; }

void term_say(const char *s)
{
    while (*s && outn < sizeof out - 1) out[outn++] = *s++;
    out[outn] = 0;
}

/* ---- the seam: arena.c ---------------------------------------------------
 * The real arena is 16 MiB at a fixed physical address. exec.c only ever asks
 * it two questions and tells it one thing, so the numbers are what matter, not
 * the memory - and a stub makes "did run reset the arena" checkable, which it
 * would not be against the real one. */
static unsigned long fake_cap = 16UL << 20;
static unsigned long fake_free;
static int resets;

unsigned long arena_capacity(void)  { return fake_cap; }
unsigned long arena_available(void) { return fake_free; }
unsigned long arena_base_addr(void) { return 0; }
void *arena_alloc(unsigned long n)  { (void)n; return 0; }
void arena_reset(void)              { fake_free = fake_cap; resets++; }

/* ---- the seam: fb.c ------------------------------------------------------
 * exec_draw is called for real below - a body that faults on a zero-height
 * window or walks off the end of its own buffer is a compositor crash, and
 * "the window drew" is not something a serial log can assert. */
static int drawn_lines;
static char drawn[16][256];

void fb_text_prop(int x, int y, const char *s, unsigned int rgb)
{
    (void)x; (void)y; (void)rgb;
    if (drawn_lines < 16) {
        snprintf(drawn[drawn_lines], sizeof drawn[0], "%s", s ? s : "(null)");
        drawn_lines++;
    }
}
int fb_text_prop_w(const char *s) { return s ? (int)strlen(s) * 8 : 0; }
int fb_text_prop_h(void) { return 16; }

/* ---- the seam: fs.c ------------------------------------------------------
 * Defined STRONG here, which overrides exec.c's weak references - exactly what
 * happens when the real fs.c from desktop/system-track lands. Omitted entirely
 * in the -nofs build, where the weak symbols stay NULL. */
#ifndef EXECTEST_NO_FS
static int  fs_up = 1;
static const char *fs_name = "hello.zl";
static u32  fs_bytes = 120;

int fs_mounted(void) { return fs_up; }
int fs_find(const char *n) { return (fs_name && n && !strcmp(n, fs_name)) ? 3 : -1; }
u32 fs_size(int idx) { return idx == 3 ? fs_bytes : 0; }
#endif

/* ---- assertions ----------------------------------------------------------*/
static int checks, fails;

static void ok(int cond, const char *what)
{
    checks++;
    if (!cond) { fails++; printf("  FAIL  %s\n", what); }
}

static int said(const char *s) { return strstr(out, s) != NULL; }
static int drew(const char *s)
{
    for (int i = 0; i < drawn_lines; i++) if (strstr(drawn[i], s)) return 1;
    return 0;
}

static int run(const char *line)
{
    snprintf(typed, sizeof typed, "%s", line);
    outn = 0; out[0] = 0;
    return exec_run();
}

static void draw(int w, int h)
{
    drawn_lines = 0;
    exec_draw(10, 10, w, h, 0xFFFFFF, 0x808080, 0x00AAFF);
}

int main(void)
{
#ifdef EXECTEST_NO_FS
    printf("exectest-nofs - `run` with NO filesystem driver linked\n\n");
#else
    printf("exectest - `run`, and every way it declines\n\n");
#endif
    fake_free = fake_cap;

    ok(exec_state() == EX_IDLE, "state starts as IDLE");
    ok(exec_runs() == 0, "no runs counted yet");

    /* ---- `run` with no filename ------------------------------------------*/
    int st = run("");
    ok(st == EX_NO_NAME, "empty argument -> EX_NO_NAME");
    ok(said("run:"), "it printed something prefixed run:");
    ok(said("no filename"), "it said WHICH thing was missing");
    ok(said("run hello.zl"), "it showed how to use it");
    ok(exec_runs() == 1, "the attempt was counted");
    ok(exec_wants_window() == 0, "a usage error opens NO window");

    /* whitespace is not a filename */
    ok(run("   ") == EX_NO_NAME, "spaces only -> EX_NO_NAME");
    ok(run("\t ") == EX_NO_NAME, "tabs and spaces -> EX_NO_NAME");

#ifdef EXECTEST_NO_FS
    /* ---- THE SHIPPING CASE: no fs driver at all --------------------------*/
    st = run("hello.zl");
    ok(st == EX_NO_DRIVER, "a name with no fs driver -> EX_NO_DRIVER");
    ok(said("no filesystem"), "it said there is no filesystem");
    ok(said("no fs driver"), "...and that it is the DRIVER that is missing");
    ok(said("hello.zl"), "it named the file that could not be looked up");
    ok(said("ls"), "it pointed at what storage there IS");
    ok(exec_wants_window() == 1, "a named file DOES open a window");
    ok(!strcmp(exec_name(), "hello.zl"), "the name was kept");

    /* the weak-symbol branch must be reached for EVERY name, not just one */
    ok(run("a") == EX_NO_DRIVER, "single-char name -> EX_NO_DRIVER");
    ok(run("does/not/matter.zl") == EX_NO_DRIVER, "any name -> EX_NO_DRIVER");

    /* the window body works on the state that actually ships */
    draw(400, 120);
    ok(drawn_lines > 0, "the window body drew something");
    ok(drew("does/not/matter.zl"), "the window names the program");
    ok(drew("no filesystem driver"), "the window says why it did not run");
    ok(drew("KiB free"), "the window shows the arena");
#else
    /* ---- driver present, disk not formatted ------------------------------*/
    fs_up = 0;
    st = run("hello.zl");
    ok(st == EX_NO_FS, "driver up, nothing mounted -> EX_NO_FS");
    ok(said("no filesystem on the disk"), "and it says the DISK is the problem");
    ok(!said("no fs driver"), "it does NOT blame the driver, which is present");
    fs_up = 1;

    /* ---- no such file ----------------------------------------------------*/
    st = run("missing.zl");
    ok(st == EX_NOT_FOUND, "a name that is not there -> EX_NOT_FOUND");
    ok(said("no such file"), "it said so");
    ok(said("missing.zl"), "and named it");
    ok(!said("no filesystem"), "it did NOT say no filesystem - there is one");

    /* ---- the file is empty -----------------------------------------------*/
    fs_bytes = 0;
    ok(run("hello.zl") == EX_EMPTY, "a zero-byte file -> EX_EMPTY");
    ok(said("empty"), "it said empty");

    /* ---- bigger than the arena -------------------------------------------*/
    fs_bytes = (u32)(fake_cap + 1);
    st = run("hello.zl");
    ok(st == EX_TOO_BIG, "a file larger than the arena -> EX_TOO_BIG");
    ok(said("will not fit"), "it said it will not fit");
    ok(said("16777217"), "it printed the file's actual size");
    ok(said("16777216"), "...and the ceiling it did not fit under");

    /* exactly the arena's size DOES fit - the boundary, not near it */
    fs_bytes = (u32)fake_cap;
    ok(run("hello.zl") == EX_LOADED, "a file exactly the arena's size fits");

    /* ---- found, and honest about not running it --------------------------*/
    fs_bytes = 120;
    st = run("hello.zl");
    ok(st == EX_LOADED, "a real file -> EX_LOADED");
    ok(said("120 bytes"), "it printed the size");
    ok(said("nothing can execute it yet"),
       "it does NOT claim to have run it - the honest line");
    ok(said("Item 2"), "and says what would");

    /* ---- the arena is reclaimed on the way IN ----------------------------*/
    fake_free = 1234;
    int before = resets;
    run("hello.zl");
    ok(resets == before + 1, "run reset the arena");
    ok(arena_available() == fake_cap, "...and the space came back");

    /* a REFUSED run still resets - the previous program's memory is not held
     * hostage by a typo in the next command */
    fake_free = 4321;
    before = resets;
    run("missing.zl");
    ok(resets == before + 1, "a run that finds nothing still reset the arena");

    /* ...but a usage error does NOT: nothing was asked of the arena */
    before = resets;
    run("");
    ok(resets == before, "`run` with no filename does not touch the arena");

    /* ---- the window ------------------------------------------------------*/
    run("hello.zl");
    draw(400, 120);
    ok(drawn_lines > 0, "the window body drew something");
    ok(drew("hello.zl"), "the window names the program");
    ok(drew("KiB free"), "the window shows the arena");
#endif

    /* ---- the title is a pointer zl may hold but must never compare -------*/
    run("prog.zl");
    ok(strstr(exec_title(), "prog.zl") != NULL, "the title carries the name");
    ok(exec_title() != NULL, "the title is never NULL");
    run("");
    ok(strstr(exec_title(), "nothing") != NULL,
       "with no name the title says so rather than being empty");

    /* ---- a name longer than the buffer is truncated, not overflowed ------*/
    {
        char big[400];
        memset(big, 'x', sizeof big - 1);
        big[sizeof big - 1] = 0;
        run(big);
        ok(strlen(exec_name()) < 200, "an over-long name was truncated");
        ok(exec_name()[strlen(exec_name())] == 0, "and is still terminated");
        /* and the title, which concatenates onto it, did not run off either */
        ok(strlen(exec_title()) < 300, "the title survived an over-long name");
    }

    /* ---- surrounding whitespace is not part of the filename --------------*/
    run("  spaced.zl  ");
    ok(!strcmp(exec_name(), "spaced.zl"),
       "leading and trailing spaces are trimmed - a trailing space is a lookup\n"
       "        failure that reads exactly like a missing file");

    /* ---- a zero-size window must not draw or fault -----------------------*/
    draw(0, 0);
    ok(drawn_lines == 0, "a zero-size window draws nothing and does not fault");
    draw(400, 4);
    ok(drawn_lines <= 1, "a window one line tall draws at most one line");

    printf("\n%d checks, %d failures\n", checks, fails);
    if (fails == 0) printf("ok    every way `run` declines says something different\n");
    return fails ? 1 : 0;
}
