#include "appkit.h"

void fb_fill_px(int, int, int, int, unsigned);
void fb_rrect(int, int, int, int, int, unsigned);
void fb_text_prop(int, int, const char *, unsigned);
void fb_text_role(int, int, const char *, unsigned, int, int);
void fb_line(int, int, int, int, unsigned);

void ak_fill(struct ak_rect r, unsigned rgb)
{
    if (r.w > 0 && r.h > 0) fb_fill_px(r.x, r.y, r.w, r.h, rgb);
}

void ak_panel(struct ak_rect r)
{
    int rad = ui_theme()->radius;
    fb_rrect(r.x, r.y, r.w, r.h, rad, AK_PANEL);
}

void ak_card(struct ak_rect r)
{
    int rad = ui_theme()->radius;
    fb_rrect(r.x, r.y, r.w, r.h, rad, AK_LINE);
    if (r.w > 2 && r.h > 2)
        fb_rrect(r.x + 1, r.y + 1, r.w - 2, r.h - 2, rad - 1, AK_PANEL);
}

void ak_text(int x, int y, const char *s, unsigned rgb)
{
    fb_text_role(x, y, s, rgb, 1, 0);
}

void ak_text_bold(int x, int y, const char *s, unsigned rgb)
{
    fb_text_role(x, y, s, rgb, 1, 1);
}

void ak_text_mono(int x, int y, const char *s, unsigned rgb)
{
    fb_text_prop(x, y, s, rgb);
}

void ak_rule(int x, int y, int w)
{
    fb_fill_px(x, y, w, 1, AK_LINE);
}

void ak_meter(int x, int y, int w, int pct, unsigned rgb)
{
    int s = ui_theme()->scale;
    int h = 5 * s;
    pct = ak_clamp(pct, 0, 100);
    fb_rrect(x, y, w, h, h / 2, AK_BG);
    if (pct) fb_rrect(x, y, w * pct / 100, h, h / 2, rgb);
}

void ak_badge(int x, int y, const char *s, unsigned rgb)
{
    int u = ui_theme()->scale;
    int w = (ak_strlen(s) * 7 + 14) * u;
    fb_rrect(x, y, w, 18 * u, 9 * u, AK_RAISED);
    ak_text_mono(x + 7 * u, y + 2 * u, s, rgb);
}

void ak_button(struct ak_rect r, const char *s, int active)
{
    int u = ui_theme()->scale;
    fb_rrect(r.x, r.y, r.w, r.h, r.h / 3, active ? AK_ACCENT : AK_RAISED);
    ak_text_mono(r.x + 8 * u, r.y + (r.h - 12 * u) / 2, s,
                 active ? AK_BG : AK_TEXT);
}

int ak_hit(struct ak_rect r, int x, int y)
{
    return x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h;
}

int ak_clamp(int v, int lo, int hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

int ak_strlen(const char *s)
{
    int n = 0;
    if (s) while (s[n]) n++;
    return n;
}

int ak_streq(const char *a, const char *b)
{
    if (!a || !b) return 0;
    while (*a && *b && *a == *b) { a++; b++; }
    return *a == *b;
}

void ak_copy(char *dst, int cap, const char *src)
{
    int i = 0;
    if (!dst || cap <= 0) return;
    if (src) while (src[i] && i + 1 < cap) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}

void ak_append_char(char *dst, int cap, int ch)
{
    int n = ak_strlen(dst);
    if (n + 1 >= cap || ch < 32 || ch > 126) return;
    dst[n] = (char)ch;
    dst[n + 1] = 0;
}

void ak_backspace(char *dst)
{
    int n = ak_strlen(dst);
    if (n) dst[n - 1] = 0;
}

void ak_u32(char *dst, unsigned v)
{
    char rev[12]; int n = 0, i;
    if (!v) rev[n++] = '0';
    while (v && n < 11) { rev[n++] = (char)('0' + v % 10u); v /= 10u; }
    for (i = 0; i < n; i++) dst[i] = rev[n - i - 1];
    dst[n] = 0;
}

void ak_i32(char *dst, int v)
{
    unsigned n;
    if (v >= 0) { ak_u32(dst, (unsigned)v); return; }
    dst[0] = '-'; n = (unsigned)(-(v + 1)) + 1u; ak_u32(dst + 1, n);
}

static char hd(unsigned v) { v &= 15u; return (char)(v < 10 ? '0' + v : 'a' + v - 10); }

void ak_hex32(char *dst, unsigned v)
{
    dst[0] = '0'; dst[1] = 'x';
    for (int i = 0; i < 8; i++) dst[2 + i] = hd(v >> (28 - 4 * i));
    dst[10] = 0;
}

void ak_hex8(char *dst, unsigned v)
{
    dst[0] = hd(v >> 4); dst[1] = hd(v); dst[2] = 0;
}

unsigned ak_fnv1a(const char *s)
{
    unsigned h = 2166136261u;
    while (s && *s) { h ^= (unsigned char)*s++; h *= 16777619u; }
    return h;
}

unsigned ak_crc32(const char *s)
{
    unsigned c = 0xFFFFFFFFu;
    while (s && *s) {
        c ^= (unsigned char)*s++;
        for (int i = 0; i < 8; i++) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1u)));
    }
    return c ^ 0xFFFFFFFFu;
}

static unsigned rng = 0x5A17C0DEu;
void ak_seed(unsigned seed) { rng = seed ? seed : 0x5A17C0DEu; }
unsigned ak_xorshift(void)
{
    rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5; return rng;
}

int ak_window_find(int app)
{
    for (int i = 0; i < wm_count(); i++) {
        int w = wm_zorder_at(i);
        if (w >= 0 && wm_is_open(w) && wm_win_app(w) == app) return w;
    }
    return -1;
}

int ak_open_once(int app, const char *title, int w, int h, int cascade)
{
    int old = ak_window_find(app);
    if (old >= 0) { wm_focus(old); wm_raise(old); return old; }
    int u = ui_theme()->scale;
    int x = (40 + (cascade % 7) * 24) * u;
    int y = (48 + (cascade % 6) * 20) * u;
    int win = wm_open(app, title, x, y, w * u, h * u);
    if (win >= 0) { wm_focus(win); wm_raise(win); }
    return win;
}
