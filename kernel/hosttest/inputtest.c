/* inputtest.c - drive the REAL input.c from Linux, with fake hardware.
 *
 * Same trick hosttest uses for intel.c: input.c talks to exactly three things
 * outside itself - idt_ticks, idt_scan and the two USB HID readers - so
 * stubbing those turns the whole event stack into an ordinary program that can
 * be asserted against in milliseconds instead of a boot.
 *
 * This compiles the EXACT input.c that ships in the kernel. If it drifts from
 * what boots, that is a bug in this file, not a licence to patch a copy.
 *
 * It exists because of a bug that was invisible from a screenshot: arrow keys
 * never reached an application. Every ordinary character worked, which made it
 * look like a routing problem in the compositor, and it was not - it was the
 * USB path decoding keys to CHARACTERS. There is no character for Up, so it
 * decoded to 0, and 0 already means "nothing was typed". The key vanished with
 * no error anywhere. A test that asserts on event TYPE and CODE, rather than on
 * whether something happened, is the only kind that catches that.
 *
 * Build and run:  ./build.sh && ./inputtest
 */
#include <stdio.h>

/* ---- input.c's public surface ------------------------------------------ */
void input_poll(void);
int  input_next(void);
int  input_type(void);
int  input_code(void);
int  input_mods(void);
int  input_char(void);
int  input_key(void);
int  input_shift(void);
int  input_ctrl(void);
int  input_key_held(int code);
int  input_queued(void);

#define EV_KEY_DOWN 1
#define EV_KEY_UP   2
#define EV_CHAR     3

#define KEY_ESC       0x101
#define KEY_BACKSPACE 0x102
#define KEY_TAB       0x103
#define KEY_ENTER     0x104
#define KEY_LEFT      0x110
#define KEY_RIGHT     0x111
#define KEY_UP        0x112
#define KEY_DOWN      0x113
#define KEY_HOME      0x114
#define KEY_END       0x115
#define KEY_PGUP      0x116
#define KEY_PGDN      0x117
#define KEY_DELETE    0x119
#define KEY_F1        0x120

/* ---- the fake hardware -------------------------------------------------- */
static unsigned fake_ticks = 0;
unsigned int idt_ticks(void) { return fake_ticks; }

static int scan_q[128], scan_head, scan_tail;
int idt_scan(void)
{
    if (scan_head == scan_tail) return 0;
    int v = scan_q[scan_head];
    scan_head = (scan_head + 1) % 128;
    return v;
}
static void send_scan(int sc) { scan_q[scan_tail] = sc; scan_tail = (scan_tail + 1) % 128; }

/* The USB side speaks packed HID events, exactly as xhci.c hands them over:
 * bit 16 press, bits 15:8 the HID modifier bitmap, bits 7:0 the usage ID. */
static int hid_q[128], hid_head, hid_tail;
static int hid_mods_live = 0;
int xhci_key_event(void)
{
    if (hid_head == hid_tail) return 0;
    int v = hid_q[hid_head];
    hid_head = (hid_head + 1) % 128;
    return v;
}
int xhci_kbd_mods(void) { return hid_mods_live; }

static void send_hid(int press, int mods, int usage)
{
    hid_mods_live = mods;
    hid_q[hid_tail] = (press << 16) | (mods << 8) | usage;
    hid_tail = (hid_tail + 1) % 128;
}

/* ---- assertions --------------------------------------------------------- */
static int fails = 0;

static void ok(const char *what, int cond)
{
    printf("  %s  %s\n", cond ? "ok  " : "FAIL", what);
    if (!cond) fails++;
}

/* Drain the queue looking for one specific event. Returns 1 if it was there.
 * Draining fully between cases is what keeps the cases independent. */
static int saw(int type, int code)
{
    int found = 0;
    for (int g = 0; g < 64; g++) {
        int t = input_next();
        if (!t) break;
        if (t == type && input_code() == code) found = 1;
    }
    return found;
}

static void drain(void) { for (int g = 0; g < 64; g++) if (!input_next()) break; }

/* ---- PS/2: the arrows were never broken here, and this pins that down ---- */
static void test_ps2_arrows(void)
{
    printf("PS/2 extended keys (E0-prefixed)\n");

    struct { int sc; int key; const char *name; } t[] = {
        { 0x48, KEY_UP,     "up"     }, { 0x50, KEY_DOWN,  "down"   },
        { 0x4B, KEY_LEFT,   "left"   }, { 0x4D, KEY_RIGHT, "right"  },
        { 0x47, KEY_HOME,   "home"   }, { 0x4F, KEY_END,   "end"    },
        { 0x49, KEY_PGUP,   "pgup"   }, { 0x51, KEY_PGDN,  "pgdn"   },
        { 0x53, KEY_DELETE, "delete" },
    };

    for (unsigned i = 0; i < sizeof t / sizeof t[0]; i++) {
        char msg[64];
        send_scan(0xE0); send_scan(t[i].sc);
        snprintf(msg, sizeof msg, "PS/2 %s -> EV_KEY_DOWN 0x%03X", t[i].name, t[i].key);
        ok(msg, saw(EV_KEY_DOWN, t[i].key));

        send_scan(0xE0); send_scan(t[i].sc | 0x80);
        snprintf(msg, sizeof msg, "PS/2 %s release -> EV_KEY_UP", t[i].name);
        ok(msg, saw(EV_KEY_UP, t[i].key));
    }

    /* the ordinary path must keep working - it is the control in the experiment */
    send_scan(0x39); send_scan(0xB9);
    ok("PS/2 space still emits EV_CHAR 0x20", saw(EV_CHAR, ' '));
    drain();
}

/* ---- USB HID: this is where the reported bug lived ---------------------- */
static void test_usb_arrows(void)
{
    printf("USB HID extended keys\n");

    struct { int usage; int key; const char *name; } t[] = {
        { 0x52, KEY_UP,     "up"     }, { 0x51, KEY_DOWN,  "down"   },
        { 0x50, KEY_LEFT,   "left"   }, { 0x4F, KEY_RIGHT, "right"  },
        { 0x4A, KEY_HOME,   "home"   }, { 0x4D, KEY_END,   "end"    },
        { 0x4B, KEY_PGUP,   "pgup"   }, { 0x4E, KEY_PGDN,  "pgdn"   },
        { 0x4C, KEY_DELETE, "delete" },
    };

    for (unsigned i = 0; i < sizeof t / sizeof t[0]; i++) {
        char msg[64];
        send_hid(1, 0, t[i].usage);
        snprintf(msg, sizeof msg, "USB %s -> EV_KEY_DOWN 0x%03X", t[i].name, t[i].key);
        ok(msg, saw(EV_KEY_DOWN, t[i].key));

        send_hid(0, 0, t[i].usage);
        snprintf(msg, sizeof msg, "USB %s release -> EV_KEY_UP", t[i].name);
        ok(msg, saw(EV_KEY_UP, t[i].key));
    }
    drain();

    /* input_key() is the call a scrolling application actually makes */
    send_hid(1, 0, 0x51);
    ok("USB down through input_key() is 0x113", input_key() == KEY_DOWN);
    drain();
}

/* ---- the two keyboards must agree ---------------------------------------
 * Same key, same event, whichever one you reached for. This is the assertion
 * that would have failed before the fix while every "does a key work at all"
 * test passed. */
static void test_parity(void)
{
    printf("PS/2 and USB agree\n");

    struct { int sc; int usage; int key; const char *name; } t[] = {
        { 0x48, 0x52, KEY_UP,        "up"        },
        { 0x50, 0x51, KEY_DOWN,      "down"      },
        { 0x4B, 0x50, KEY_LEFT,      "left"      },
        { 0x4D, 0x4F, KEY_RIGHT,     "right"     },
        { 0x49, 0x4B, KEY_PGUP,      "pgup"      },
        { 0x51, 0x4E, KEY_PGDN,      "pgdn"      },
    };

    for (unsigned i = 0; i < sizeof t / sizeof t[0]; i++) {
        char msg[64];
        send_scan(0xE0); send_scan(t[i].sc);
        int a = input_key();
        send_scan(0xE0); send_scan(t[i].sc | 0x80);      /* release, or the key
                                                            stays held into the
                                                            next test */
        drain();
        send_hid(1, 0, t[i].usage);
        int b = input_key();
        send_hid(0, 0, t[i].usage);
        drain();
        snprintf(msg, sizeof msg, "%s: PS/2 0x%03X == USB 0x%03X", t[i].name, a, b);
        ok(msg, a == b && a == t[i].key);
    }

    /* and the named keys, which have a KEY_* code on both paths and no char */
    struct { int sc; int usage; int key; const char *name; } n[] = {
        { 0x1C, 0x28, KEY_ENTER,     "enter"     },
        { 0x01, 0x29, KEY_ESC,       "escape"    },
        { 0x0E, 0x2A, KEY_BACKSPACE, "backspace" },
        { 0x0F, 0x2B, KEY_TAB,       "tab"       },
        { 0x3B, 0x3A, KEY_F1,        "f1"        },
        { 0x44, 0x43, KEY_F1 + 9,    "f10"       },
    };
    for (unsigned i = 0; i < sizeof n / sizeof n[0]; i++) {
        char msg[64];
        send_scan(n[i].sc);
        int a = input_key();
        send_scan(n[i].sc | 0x80);
        drain();
        send_hid(1, 0, n[i].usage);
        int b = input_key();
        send_hid(0, 0, n[i].usage);
        drain();
        snprintf(msg, sizeof msg, "%s: PS/2 0x%03X == USB 0x%03X", n[i].name, a, b);
        ok(msg, a == b && a == n[i].key);
    }
}

/* ---- USB characters, which is what regressing would look like ----------- */
static void test_usb_chars(void)
{
    printf("USB HID characters and modifiers\n");

    send_hid(1, 0x00, 0x04);
    ok("USB 'a' -> EV_CHAR 'a'", saw(EV_CHAR, 'a'));
    drain();

    send_hid(1, 0x02, 0x04);                 /* left shift held */
    ok("USB shift+a -> EV_CHAR 'A'", saw(EV_CHAR, 'A'));
    drain();

    send_hid(1, 0x20, 0x1E);                 /* right shift + '1' */
    ok("USB rshift+1 -> EV_CHAR '!'", saw(EV_CHAR, '!'));
    drain();

    /* Ctrl over USB never worked: the old decoder handled shift and nothing
     * else, so Ctrl+W arrived as 'w' and the compositor's close key was dead
     * on an external keyboard. */
    send_hid(1, 0x01, 0x1A);                 /* left ctrl + 'w' */
    ok("USB ctrl+w -> EV_CHAR 23", saw(EV_CHAR, 23));
    drain();

    send_hid(1, 0x00, 0x2C);
    ok("USB space -> EV_CHAR 0x20", saw(EV_CHAR, ' '));
    drain();

    send_hid(1, 0x00, 0x37);
    ok("USB '.' -> EV_CHAR '.'", saw(EV_CHAR, '.'));
    drain();

    /* caps lock is a latch shared with the PS/2 keyboard, not a held modifier */
    send_hid(1, 0x00, 0x39);                 /* caps lock down */
    send_hid(0, 0x00, 0x39);
    input_poll(); drain();
    send_hid(1, 0x00, 0x04);
    ok("USB caps lock then 'a' -> 'A'", saw(EV_CHAR, 'A'));
    drain();
    send_hid(1, 0x00, 0x39); send_hid(0, 0x00, 0x39);
    input_poll(); drain();                   /* back off */

    send_hid(1, 0x02, 0x04);
    input_poll();
    ok("input_shift() sees a USB shift", input_shift() == 1);
    drain();
}

/* ---- held state and repeat --------------------------------------------- */
static void test_held_and_repeat(void)
{
    printf("held keys and auto-repeat\n");

    send_hid(1, 0, 0x51);                    /* USB down arrow, held */
    input_poll(); drain();
    ok("USB down is held", input_key_held(KEY_DOWN) == 1);

    send_hid(0, 0, 0x51);                    /* released */
    input_poll(); drain();
    ok("USB down is released", input_key_held(KEY_DOWN) == 0);

    /* A USB key with no release would repeat forever. That is what the release
     * events added to hid_decode() are for. */
    send_hid(1, 0, 0x51);
    input_poll(); drain();
    fake_ticks += 100;                       /* well past REPEAT_DELAY */
    input_poll();
    ok("held USB down auto-repeats", saw(EV_KEY_DOWN, KEY_DOWN));
    drain();

    send_hid(0, 0, 0x51);
    input_poll(); drain();
    fake_ticks += 100;
    input_poll();
    ok("released USB down stops repeating", saw(EV_KEY_DOWN, KEY_DOWN) == 0);
    drain();

    /* A USB 'a' is usage 0x04 and a PS/2 F9 is scancode 0x04. If they shared a
     * held-key slot, releasing one would un-hold the other. */
    send_scan(0x04);                         /* PS/2 '3' */
    send_hid(1, 0, 0x04);                    /* USB 'a'  */
    input_poll(); drain();
    ok("PS/2 '3' and USB 'a' are both held", input_key_held('3') && input_key_held('a'));
    send_hid(0, 0, 0x04);
    input_poll(); drain();
    ok("releasing USB 'a' leaves PS/2 '3' held", input_key_held('3') == 1);
    send_scan(0x84);
    input_poll(); drain();
}

int main(void)
{
    test_ps2_arrows();
    test_usb_arrows();
    test_parity();
    test_usb_chars();
    test_held_and_repeat();

    printf("\n%s\n", fails ? "FAILED" : "all passed");
    return fails ? 1 : 0;
}
