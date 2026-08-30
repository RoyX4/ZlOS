#ifndef ZLOS_APPKIT_H
#define ZLOS_APPKIT_H

#include "ui.h"

/* The ds.html application palette. Kept separate from the desktop theme so
 * the imported apps remain visually exact even when Settings changes zlOS. */
#define AK_BG       0x070809u
#define AK_CANVAS   0x0B0D0Fu
#define AK_SURFACE  0x101215u
#define AK_PANEL    0x14171Au
#define AK_RAISED   0x1C2024u
#define AK_LINE     0x22262Bu
#define AK_TEXT     0xDFE2E5u
#define AK_BRIGHT   0xEEF0F2u
#define AK_DIM      0x74797Fu
#define AK_FAINT    0x474B50u
#define AK_ACCENT   0xB8E838u
#define AK_ACCENT_2 0xCDF25Au
#define AK_OK       0xA9E34Bu
#define AK_WARN     0xF5B93Cu
#define AK_BAD      0xFF6A50u

#define AK_EV_KEY_DOWN 1
#define AK_EV_KEY_UP   2
#define AK_EV_CHAR     3
#define AK_EV_MOUSE    4
#define AK_EV_WHEEL    5
#define AK_KEY_BACKSPACE 0x102
#define AK_KEY_ENTER     0x104
#define AK_KEY_LEFT      0x110
#define AK_KEY_RIGHT     0x111
#define AK_KEY_UP        0x112
#define AK_KEY_DOWN      0x113

struct ak_rect { int x, y, w, h; };

void ak_fill(struct ak_rect r, unsigned rgb);
void ak_panel(struct ak_rect r);
void ak_card(struct ak_rect r);
void ak_text(int x, int y, const char *s, unsigned rgb);
void ak_text_bold(int x, int y, const char *s, unsigned rgb);
void ak_text_mono(int x, int y, const char *s, unsigned rgb);
void ak_rule(int x, int y, int w);
void ak_meter(int x, int y, int w, int pct, unsigned rgb);
void ak_badge(int x, int y, const char *s, unsigned rgb);
void ak_button(struct ak_rect r, const char *s, int active);
int  ak_hit(struct ak_rect r, int x, int y);
int  ak_clamp(int v, int lo, int hi);
int  ak_strlen(const char *s);
int  ak_streq(const char *a, const char *b);
void ak_copy(char *dst, int cap, const char *src);
void ak_append_char(char *dst, int cap, int ch);
void ak_backspace(char *dst);
void ak_u32(char *dst, unsigned v);
void ak_i32(char *dst, int v);
void ak_hex32(char *dst, unsigned v);
void ak_hex8(char *dst, unsigned v);
unsigned ak_fnv1a(const char *s);
unsigned ak_crc32(const char *s);
unsigned ak_xorshift(void);
void ak_seed(unsigned seed);
int ak_window_find(int app);
int ak_open_once(int app, const char *title, int w, int h, int cascade);

#endif
