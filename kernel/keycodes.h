/* Key codes, shared by every producer of key events.
 *
 * These lived only in input.c while PS/2 was the only source of keys. xhci.c
 * produces them too now, and a second copy of a numeric table is a copy that
 * eventually disagrees with the first - so they live here and both files
 * include this.
 *
 * Printable keys use their unshifted ASCII, so the common case is trivial and a
 * consumer can compare against a character literal. Everything else is at or
 * above 0x100 where it cannot collide with a character. That boundary is load
 * bearing: `code >= KEY_NONCHAR` is exactly the test for "this key has no
 * character", and input.c, xhci.c and the zl shell all rely on it. */
#ifndef ZL_KEYCODES_H
#define ZL_KEYCODES_H

/* The first code that is a key rather than a character. */
#define KEY_NONCHAR   0x100

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
#define KEY_INSERT    0x118
#define KEY_DELETE    0x119
/* THE SUPER KEY AS A KEY, not only as a modifier. Super is a modifier and
 * produces no event of its own, so "Super opens the menu" - which every
 * desktop does and desktop-TODO asks for - had nothing to hang on. A TAP is
 * the gesture: pressed and released with no other key in between. Held with
 * another key it stays a pure modifier and this code is never emitted. */
#define KEY_SUPER     0x11A
#define KEY_F1        0x120        /* F1..F12 are KEY_F1 + n */

#define MOD_SHIFT   (1 << 0)
#define MOD_CTRL    (1 << 1)
#define MOD_ALT     (1 << 2)
#define MOD_CAPS    (1 << 3)
#define MOD_NUM     (1 << 4)
#define MOD_SUPER   (1 << 5)

#endif
