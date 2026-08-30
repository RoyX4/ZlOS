/* exec.c - `run`, and every way it declines.
 *
 * This file exists BEFORE anything can execute, and that is deliberate. The
 * brief's reasoning: get the failure modes right while they are the only
 * modes. A `run` that grows an execution path first and error paths afterwards
 * ends up with one good path and four that print nothing, and "it did nothing"
 * is the single hardest symptom to debug in this project - it has cost two
 * silent fallbacks already.
 *
 * So today every single path through this file is a refusal, each one says
 * something different, and each one is gated.
 *
 * THE FILENAME NEVER BECOMES A ZL VALUE, AND THAT IS NOT A STYLE CHOICE
 * --------------------------------------------------------------------
 * runtime_kernel.c:562 hard-faults on ANY operand of type V_STR:
 *
 *     if (a.type == V_STR || b.type == V_STR)
 *         kfatal("string operators are not available in the kernel subset");
 *
 * and that test sits BEFORE the `==` arm at :573. So this, in kernel.zl:
 *
 *     if name == "hello.zl" { ... }
 *
 * compiles clean, links clean, boots, and halts the machine the instant it
 * runs. Compile-clean and boot-fatal is the worst failure shape there is, and
 * it is why the name is read here, in C, from term.c's own buffer, and why
 * nothing above this file ever holds it. The only string that crosses into zl
 * is a pointer for a window title, which is passed straight to wm_open and
 * never compared.
 *
 * WHY THE FILESYSTEM IS A WEAK SYMBOL
 * -----------------------------------
 * There is no fs.c on this branch. There is one on desktop/system-track
 * (4f2d7ee, 11fa28d) and it will land here. Writing a second one would be the
 * wrong repair; guessing when it arrives and hard-linking against it breaks the
 * build until it does. Weak references are NULL when nothing defines them, so
 * this file links today, says "no filesystem" honestly, and BINDS TO THE REAL
 * DRIVER the moment that merge happens, with no change here. Same trick
 * runtime_kernel.c:115-128 uses for the USB pointer.
 *
 * And it lets the two cases be told apart, which they must be:
 *
 *     fs_mounted == 0          no filesystem DRIVER in this kernel at all
 *     fs_mounted() == 0        the driver is here, the disk has no filesystem
 *
 * Those are different problems with different fixes, and they look identical
 * from any message that says only "no filesystem". runtime_kernel.c:1016 makes
 * exactly this distinction for the USB pointer, and the comment there records
 * that conflating them "cost a long hunt once already".
 */

typedef unsigned int u32;

#define ZL_WEAK __attribute__((weak))

/* fs.c, desktop/system-track. Signatures read off that branch, not guessed:
 *   git show desktop/system-track:kernel/fs.c | grep -E '^(int|u32) fs_' */
extern int fs_mounted(void) ZL_WEAK;
extern int fs_find(const char *name) ZL_WEAK;
extern u32 fs_size(int idx) ZL_WEAK;

/* term.c owns the typed line; this reads the argument out of it rather than
 * keeping a second copy of the parser. */
extern const char *term_argstr(void);
extern void term_say(const char *s);

/* arena.c - the ceiling a program's memory has to fit under. Not weak: it is
 * in all four source lists and a missing arena has no safe fallback. */
extern unsigned long arena_capacity(void);
extern unsigned long arena_available(void);
extern void arena_reset(void);

/* ---- what happened ------------------------------------------------------ */
#define EX_IDLE       0    /* nothing has been run yet                       */
#define EX_NO_NAME    1    /* `run` with no filename                         */
#define EX_NO_DRIVER  2    /* no filesystem driver linked into this kernel   */
#define EX_NO_FS      3    /* driver present, nothing mounted                */
#define EX_NOT_FOUND  4    /* filesystem present, no such file               */
#define EX_EMPTY      5    /* the file exists and is zero bytes              */
#define EX_TOO_BIG    6    /* larger than the arena's ceiling                */
#define EX_LOADED     7    /* in the arena - nothing executes it yet (Item 2)*/

#define EX_NAME_MAX   64

static char name[EX_NAME_MAX];
static char title[EX_NAME_MAX + 8];
static int  state = EX_IDLE;
static u32  runs;                 /* how many times `run` has been asked     */

/* ---- the smallest string helpers, because there is no libc --------------- */
static int slen(const char *s) { int n = 0; while (s && s[n]) n++; return n; }

static void scopy(char *dst, const char *src, int cap)
{
    int i = 0;
    if (cap <= 0) return;
    while (src && src[i] && i < cap - 1) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}

static void scat(char *dst, const char *src, int cap)
{
    int i = slen(dst), j = 0;
    while (src && src[j] && i < cap - 1) dst[i++] = src[j++];
    dst[i] = 0;
}

/* trim leading and trailing spaces in place - `run   hello.zl  ` is the same
 * request as `run hello.zl`, and a trailing space in a filename is a lookup
 * failure that reads like a missing file. */
static void strim(char *s)
{
    int a = 0, b, i = 0;
    while (s[a] == ' ' || s[a] == '\t') a++;
    b = slen(s);
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t')) b--;
    while (a < b) s[i++] = s[a++];
    s[i] = 0;
}

static void say_u(unsigned long v)
{
    char b[24];
    int i = 0;
    if (!v) { term_say("0"); return; }
    while (v) { b[i++] = (char)('0' + (int)(v % 10UL)); v /= 10UL; }
    b[i] = 0;
    /* reverse in place */
    for (int a = 0, z = i - 1; a < z; a++, z--) { char t = b[a]; b[a] = b[z]; b[z] = t; }
    term_say(b);
}

/* ---- the one place a refusal is worded ----------------------------------- */
static int decline(int st, const char *what)
{
    state = st;
    term_say("  run: ");
    term_say(what);
    term_say("\n");
    return st;
}

const char *exec_name(void)  { return name; }
int         exec_state(void) { return state; }
u32         exec_runs(void)  { return runs; }

/* The window title. A pointer into this file's own buffer, handed to wm_open
 * and never compared - see the header comment on why that distinction is the
 * difference between working and halting the machine. */
const char *exec_title(void)
{
    scopy(title, "run - ", sizeof title);
    scat(title, name[0] ? name : "(nothing)", sizeof title);
    return title;
}

/* Should `run` put a window on screen? A usage error - `run` with no filename -
 * gets a line and nothing else; opening a window to say "you forgot the
 * argument" is noise. Anything that names a file gets a window, because from
 * Item 2 onward that window is where the program lives, and a window that only
 * ever appears on the success path is a window nobody has tested. */
int exec_wants_window(void)
{
    return state != EX_IDLE && state != EX_NO_NAME;
}

/* ---- `run` --------------------------------------------------------------- */
int exec_run(void)
{
    runs++;

    scopy(name, term_argstr(), sizeof name);
    strim(name);

    if (!name[0])
        return decline(EX_NO_NAME, "no filename.  try:  run hello.zl");

    /* Reclaim on the way IN, not on the way out. A program that faulted still
     * has its memory intact to be looked at afterwards, and nothing is holding
     * a pointer into the arena at the moment it is reclaimed except code that
     * is about to be handed a new one. */
    arena_reset();

    if (fs_mounted == 0) {
        /* Not "no filesystem". THIS kernel has no filesystem driver compiled
         * into it - a different fact with a different fix, and the message has
         * to carry that or the next person goes looking at the disk. */
        state = EX_NO_DRIVER;
        term_say("  run: no filesystem - this kernel has no fs driver, so there is\n");
        term_say("       nothing to look '");
        term_say(name);
        term_say("' up in.  Storage today is ten numbered\n");
        term_say("       RAM slots addressed by number; try 'ls'.\n");
        return state;
    }

    if (!fs_mounted())
        return decline(EX_NO_FS,
                       "no filesystem on the disk.  it has a driver but nothing mounted");

    if (fs_find == 0)         /* driver linked without a lookup - cannot happen
                               * today, and if it ever does it must not be
                               * silent. */
        return decline(EX_NO_DRIVER, "the filesystem driver has no lookup");

    int idx = fs_find(name);
    if (idx < 0) {
        state = EX_NOT_FOUND;
        term_say("  run: no such file: ");
        term_say(name);
        term_say("\n");
        return state;
    }

    u32 bytes = (fs_size != 0) ? fs_size(idx) : 0;
    if (bytes == 0)
        return decline(EX_EMPTY, "that file is empty");

    if ((unsigned long)bytes > arena_available()) {
        state = EX_TOO_BIG;
        term_say("  run: ");
        say_u((unsigned long)bytes);
        term_say(" bytes will not fit - the program arena holds ");
        say_u(arena_capacity());
        term_say("\n");
        return state;
    }

    /* Everything above this line is reachable and gated today. Below it is not:
     * loading needs a filesystem this branch does not have, and RUNNING needs
     * the interpreter that is Item 2. Saying so is better than a stub that
     * returns success. */
    state = EX_LOADED;
    term_say("  run: '");
    term_say(name);
    term_say("' found, ");
    say_u((unsigned long)bytes);
    term_say(" bytes - but nothing can execute it yet (Item 2).\n");
    return state;
}

/* ---- the window ----------------------------------------------------------
 * Same shape as term_draw (term.c:214): the app body is written in C and
 * kernel.zl's app_draw does nothing but call it with the client rectangle.
 * POSITION-PURE, as ui.h requires - every coordinate below is derived from the
 * x,y,w,h passed in and nothing is baked in.
 */
extern void fb_text_prop(int x, int y, const char *s, unsigned int rgb);
extern int  fb_text_prop_w(const char *s);
extern int  fb_text_prop_h(void);

static const char *state_line(void)
{
    switch (state) {
    case EX_IDLE:      return "nothing has been run yet.";
    case EX_NO_NAME:   return "no filename was given.";
    case EX_NO_DRIVER: return "there is no filesystem driver in this kernel,";
    case EX_NO_FS:     return "the disk has no filesystem on it,";
    case EX_NOT_FOUND: return "no file by that name.";
    case EX_EMPTY:     return "that file is empty.";
    case EX_TOO_BIG:   return "that file is larger than the program arena.";
    case EX_LOADED:    return "found it, but nothing can execute it yet.";
    }
    return "unknown state.";
}

static const char *state_line2(void)
{
    switch (state) {
    case EX_NO_DRIVER: return "so there is nothing to look a name up in.";
    case EX_NO_FS:     return "so there is nothing to look a name up in.";
    case EX_LOADED:    return "the interpreter is Item 2 of the exec track.";
    }
    return 0;
}

void exec_draw(int x, int y, int w, int h,
               unsigned int fg, unsigned int dim, unsigned int accent)
{
    int lh = fb_text_prop_h();
    if (lh <= 0 || w <= 0 || h <= 0) return;

    int ty = y;
    fb_text_prop(x, ty, name[0] ? name : "(no program)", accent);
    ty += lh + lh / 2;

    if (ty + lh <= y + h) { fb_text_prop(x, ty, state_line(), fg); ty += lh; }

    const char *l2 = state_line2();
    if (l2 && ty + lh <= y + h) { fb_text_prop(x, ty, l2, fg); ty += lh; }

    /* The arena is the thing this window will be about once programs run, so
     * it is on screen from the first version rather than added later - a
     * number that has always been there is one somebody notices going wrong. */
    ty += lh / 2;
    if (ty + lh <= y + h) {
        char b[64];
        scopy(b, "arena: ", sizeof b);
        {
            unsigned long free_b = arena_available() >> 10;
            char n[24]; int i = 0;
            if (!free_b) { n[i++] = '0'; }
            while (free_b) { n[i++] = (char)('0' + (int)(free_b % 10UL)); free_b /= 10UL; }
            n[i] = 0;
            for (int a = 0, z = i - 1; a < z; a++, z--) { char t = n[a]; n[a] = n[z]; n[z] = t; }
            scat(b, n, sizeof b);
        }
        scat(b, " KiB free", sizeof b);
        fb_text_prop(x, ty, b, dim);
    }
}
