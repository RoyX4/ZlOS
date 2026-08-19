/* palette.c - ONE palette, asserted across the three files that carry it.
 *
 * DECISIONS.md open item E. Two palettes shipped simultaneously:
 *
 *   kernel.zl's rgb() constants   header bar, dock, two legacy app bodies
 *   ui.c's ui_theme struct        EVERY WINDOW FRAME ON SCREEN
 *
 * and they agreed on 2 of 10 roles. Two cyans and two panel colours were up at
 * once. visual-speed-northstar.md names "duplicated palette roles" as a thing
 * that must not ship.
 *
 * WHICH IS THE SOURCE OF TRUTH IS SETTLED BY THE REFERENCE'S OWN HEADER, and
 * this test is the reason that stays settled - docs/design/
 * zlOS-design-northstar.html:13 says:
 *
 *     -- the zlOS palette, straight from kernel.zl's rgb() theme --
 *
 * So the reference was transcribed FROM kernel.zl, and ui.c was the only one of
 * the three that had drifted.
 *
 * THIS READS ALL THREE FILES rather than restating their values. A test that
 * hardcoded the numbers would be a fourth copy of the palette, and the fourth
 * copy is how you get a fifth. The reference HTML and kernel.zl are PARSED; the
 * ui.c values come from ui_theme() with ui.c linked for real.
 *
 * Build and run:  ./build.sh && ./palette
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

#include "../ui.h"

/* ---- ui.c's externs, stubbed. It is linked for real; none of these is
 * reached by ui_theme_init/ui_theme, they only have to resolve. ---- */
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb)
{ (void)x;(void)y;(void)w;(void)h;(void)rgb; }
void fb_rrect(int x, int y, int w, int h, int r, unsigned int rgb)
{ (void)x;(void)y;(void)w;(void)h;(void)r;(void)rgb; }
void fb_text_prop(int px, int py, const char *s, unsigned int fg)
{ (void)px;(void)py;(void)s;(void)fg; }
int  fb_text_prop_w(const char *s) { (void)s; return 0; }
int  fb_text_prop_h(void) { return 0; }
int  fb_cell_w(void) { return 16; }
int  fb_cell_h(void) { return 32; }
void fb_clip(int x, int y, int w, int h) { (void)x;(void)y;(void)w;(void)h; }
void fb_clip_get(int *x0, int *y0, int *x1, int *y1)
{ *x0 = *y0 = 0; *x1 = *y1 = 0; }

static int fails;
static void ok(int cond, const char *what)
{
    printf("  %s %s\n", cond ? "ok  " : "FAIL", what);
    if (!cond) fails++;
}

/* ---- reading the three sources -------------------------------------------- */

static char *slurp(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) { printf("  cannot open %s\n", path); exit(2); }
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    char *b = malloc((size_t)n + 1);
    if (fread(b, 1, (size_t)n, f) != (size_t)n) { fclose(f); exit(2); }
    b[n] = 0; fclose(f);
    return b;
}

/* `--accent:#60d2eb` -> 0x60D2EB. -1 if the token is not there. */
static long css_token(const char *html, const char *name)
{
    char pat[64];
    snprintf(pat, sizeof pat, "%s:#", name);
    const char *p = strstr(html, pat);
    if (!p) return -1;
    p += strlen(pat);
    char hex[7];
    for (int i = 0; i < 6; i++) {
        if (!isxdigit((unsigned char)p[i])) return -1;
        hex[i] = p[i];
    }
    hex[6] = 0;
    return strtol(hex, NULL, 16);
}

/* `ACCENT   = rgb(96, 210, 235)` -> 0x60D2EB. -1 if absent. */
static long zl_rgb(const char *src, const char *name)
{
    const char *p = src;
    size_t nl = strlen(name);
    while ((p = strstr(p, name)) != NULL) {
        /* must be at the start of a line and followed by space/= */
        if (p != src && p[-1] != '\n') { p += nl; continue; }
        const char *q = p + nl;
        while (*q == ' ') q++;
        if (*q != '=') { p += nl; continue; }
        q = strstr(q, "rgb(");
        if (!q) return -1;
        int r, g, b;
        if (sscanf(q, "rgb(%d, %d, %d)", &r, &g, &b) != 3) return -1;
        return ((long)r << 16) | ((long)g << 8) | b;
    }
    return -1;
}

/* `{ "Ice",     0x60D2EB },` -> 0x60D2EB */
static long settings_ice(const char *src)
{
    const char *p = strstr(src, "{ \"Ice\",");
    if (!p) return -1;
    p = strstr(p, "0x");
    if (!p) return -1;
    return strtol(p + 2, NULL, 16);
}

/* ---- the roles, as data --------------------------------------------------- */
struct role {
    const char *what;         /* what it paints                        */
    const char *css;          /* reference token, or 0 if it has none  */
    const char *zl;           /* kernel.zl constant, or 0              */
    size_t      off;          /* offset of the field in struct ui_theme */
};
#define F(name) offsetof(struct ui_theme, name)

static const struct role ROLES[] = {
    { "desktop background", "--wall-top", "WALL_TOP",  F(bg)        },
    { "window body",        "--panel",    "PANEL_NS",  F(panel)     },
    { "primary text",       "--txt-hi",   "TXT_HI",    F(text)      },
    { "secondary text",     "--txt-dim",  "TXT_DIM",   F(text_dim)  },
    { "the accent",         "--accent",   "ACCENT",    F(accent)    },
    { "hairline border",    "--line",     "LINE_SOFT", F(border)    },
    { "destructive",        "--crit",     "CRIT_RED",  F(danger)    },
    { "focused title top",  "--hdr-top",  "HDR_TOP",   F(title)     },
    { "focused title bot",  "--hdr-bot",  "HDR_BOT",   F(title_bot) },
    /* the reference gives this one no VARIABLE - it is --line-soft, and ui.c
     * takes it because the mockup has no button face to copy (its own step list
     * has the widget toolkit as "queued"). kernel.zl has no counterpart. */
    { "control face",       "--line-soft", NULL,       F(panel_hi)  },
};
#define N_ROLES ((int)(sizeof ROLES / sizeof ROLES[0]))

int main(void)
{
    char *html = slurp("../../docs/design/zlOS-design-northstar.html");
    char *zl   = slurp("../kernel.zl");
    char *set  = slurp("../settings.c");

    ui_theme_init(2);
    const struct ui_theme *t = ui_theme();

    printf("palette - one source of truth across three files (DECISIONS item E)\n\n");
    printf("  %-20s %-12s %-9s %-9s %-9s\n",
           "role", "token", "reference", "kernel.zl", "ui.c");

    int agree_ref = 0, agree_zl = 0, zl_have = 0;
    for (int i = 0; i < N_ROLES; i++) {
        const struct role *r = &ROLES[i];
        long ref = css_token(html, r->css);
        long kzl = r->zl ? zl_rgb(zl, r->zl) : -1;
        unsigned ui = *(const unsigned *)((const char *)t + r->off);
        printf("  %-20s %-12s ", r->what, r->css);
        if (ref >= 0) printf("#%06lX  ", ref); else printf("   --     ");
        if (kzl >= 0) printf("#%06lX  ", kzl); else printf("   --     ");
        printf("#%06X", ui);
        if (ref >= 0 && (long)ui == ref) agree_ref++; else printf("   <-- ui.c differs");
        if (r->zl) { zl_have++; if (kzl >= 0 && kzl == (long)ui) agree_zl++; }
        printf("\n");
    }
    printf("\n");

    ok(agree_ref == N_ROLES, "ui.c agrees with the reference on every role it has");
    ok(agree_zl == zl_have,
       "ui.c agrees with kernel.zl's rgb() constants on every shared role");

    /* the unfocused title bar has no CSS VARIABLE - it is a literal inside
     * `.win:not(.focus) .titlebar`. Asserted against the reference text so the
     * gate still breaks if the reference moves. */
    ok(strstr(html, "#2a3550") != NULL && t->title_off == 0x2A3550,
       "unfocused title top is the reference's #2a3550");
    ok(strstr(html, "#182238") != NULL && t->title_off_bot == 0x182238,
       "unfocused title bottom is the reference's #182238 - a GRADIENT, "
       "not one colour twice");

    /* THE WAY THE SECOND CYAN COMES BACK. settings_apply() rebuilds the theme
     * and then writes ACCENTS[S.accent] over the accent, so if the entry marked
     * "the default" is not ui_theme_init's accent, opening Settings and picking
     * it repaints the desktop in a different cyan - the divergence returning
     * through the panel instead of through the palette. */
    long ice = settings_ice(set);
    printf("\n  settings.c ACCENTS[0] \"Ice\" = #%06lX,  ui_theme accent = #%06X\n\n",
           ice, t->accent);
    ok(ice == (long)t->accent,
       "settings.c's default accent is ui_theme_init's accent");

    /* ---- NEGATIVE CONTROL --------------------------------------------------
     * Every assertion above is an equality between two numbers this program
     * read from somewhere, so the way it fails silently is a PARSER that
     * returns the same thing for everything - both sides -1, and -1 == -1.
     * These prove the two parsers actually found values, and that they
     * distinguish. */
    ok(css_token(html, "--accent") == 0x60D2EB,
       "control: the CSS parser reads --accent as #60D2EB");
    ok(zl_rgb(zl, "ACCENT") == 0x60D2EB,
       "control: the zl parser reads ACCENT as rgb(96,210,235)");
    ok(css_token(html, "--no-such-token") == -1,
       "control: the CSS parser reports a missing token rather than 0");
    ok(zl_rgb(zl, "NO_SUCH_CONST") == -1,
       "control: the zl parser reports a missing constant rather than 0");
    ok(css_token(html, "--panel") != css_token(html, "--panel-2"),
       "control: the CSS parser does not confuse --panel with --panel-2");

    printf("\n%s (%d failure%s)\n", fails ? "FAILED" : "all passed",
           fails, fails == 1 ? "" : "s");
    free(html); free(zl); free(set);
    return fails ? 1 : 0;
}
