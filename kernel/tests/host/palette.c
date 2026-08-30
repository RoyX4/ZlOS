/* palette.c - ONE palette, and the chain that proves it.
 *
 * DECISIONS.md open item E. Two palettes once shipped simultaneously:
 *
 *   kernel.zl's rgb() constants   header bar, dock, two legacy app bodies
 *   ui.c's ui_theme struct        EVERY WINDOW FRAME ON SCREEN
 *
 * and they agreed on 2 of 10 roles. That is closed: kernel.zl carries semantic
 * role numbers and calls ui_color(), so it has no RGB copy at all.
 *
 * WHAT THIS GATE NOW CHECKS, and why it changed. It used to compare ui.c
 * against docs/design/zlOS-design-northstar.html - a hand-written mockup that
 * had itself been transcribed FROM kernel.zl. So "agree with the reference"
 * and "agree with ourselves" were the same statement, and the gate could not
 * catch a colour someone invented: it only caught the two copies drifting.
 *
 * The desktop is cloned from a real external artifact and the chain has three
 * links. THE ARTIFACT IS NOW docs/design/presswork-prototype.html - PRESSWORK's
 * running, self-verifying prototype - and it was ds-reference.html, which is the
 * PREDECESSOR design and describes a palette this kernel no longer has:
 *
 *   1. presswork-prototype.html  the artifact. Not ours, not editable to suit.
 *   2. kernel/src/graphics/ui/design.h     every token, measured out of link 1.
 *   3. ui.c / settings.c   roles, each naming a token from link 2.
 *
 * That re-anchoring is not bookkeeping. Left pointed at ds-reference.html this
 * gate failed on every PRESSWORK token, and the only ways to make it green
 * again would have been to delete it or to put the lime palette back - which
 * is exactly how a gate ends up holding a design in place after the design has
 * moved. A gate anchored to the wrong document is worse than no gate: it
 * argues, with evidence, for the wrong answer.
 *
 * LINK 1->2 IS THE ONE THAT MATTERS and it is the one that did not exist
 * before: every colour token in design.h must occur LITERALLY in the
 * reference. That makes "I picked this colour by eye and wrote a plausible
 * comment above it" a build failure rather than a code review.
 *
 * Build and run:  ./build.sh && ./palette
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

#include "../../src/graphics/ui/ui.h"
#include "../../src/graphics/ui/design.h"

/* ---- ui.c's externs, stubbed. It is linked for real; none of these is
 * reached by ui_theme_init/ui_theme, they only have to resolve. ---- */
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb)
{ (void)x;(void)y;(void)w;(void)h;(void)rgb; }
void fb_fill_blend(int x, int y, int w, int h, unsigned int rgb, int a)
{ (void)x;(void)y;(void)w;(void)h;(void)rgb;(void)a; }
void fb_rrect(int x, int y, int w, int h, int r, unsigned int rgb)
{ (void)x;(void)y;(void)w;(void)h;(void)r;(void)rgb; }
void fb_rrect_blend(int x, int y, int w, int h, int r, unsigned int rgb, int a)
{ (void)x;(void)y;(void)w;(void)h;(void)r;(void)rgb;(void)a; }
void fb_box(int x, int y, int w, int h, unsigned int rgb)
{ (void)x;(void)y;(void)w;(void)h;(void)rgb; }
void fb_text_prop(int px, int py, const char *s, unsigned int fg)
{ (void)px;(void)py;(void)s;(void)fg; }
int  fb_text_prop_w(const char *s) { (void)s; return 0; }
int  fb_text_prop_h(void) { return 0; }
int  fb_cell_w(void) { return 16; }
/* ui.c's atlas table asks fb.c what pixel height a type role resolves to, so
 * that ui_atlas_for_role() can say which atlas a register is on RIGHT NOW.
 * fb.c is not linked here - this file links ui.c alone, on purpose - and the
 * atlas geometry itself needs no stub because it comes out of sizeof, which
 * emits no relocation. Only the one live query does. 8 / 12 / 16 is fb.c's own
 * role_base at scale 1, which is what makes this a stub rather than a lie:
 * uitest.c exercises the resolution across the shipped scales. */
int  fb_text_role_h(int role)
{ return role <= 0 ? 12 : (role == 1 ? 12 : 16); }
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

/* Does #rrggbb appear in the reference, in either case, as a whole token?
 * "Whole token" matters: #b8e838 must not be found inside #b8e8380a. */
static int ref_has_colour(const char *html, unsigned rgb)
{
    char want[8];
    snprintf(want, sizeof want, "#%06x", rgb);
    for (const char *p = html; (p = strchr(p, '#')) != NULL; p++) {
        int i;
        for (i = 1; i < 7; i++)
            if (!isxdigit((unsigned char)p[i])) break;
        if (i != 7) continue;                 /* not six hex digits */
        if (isxdigit((unsigned char)p[7])) continue;  /* seven or more */
        char got[8];
        for (i = 0; i < 7; i++) got[i] = (char)tolower((unsigned char)p[i]);
        got[7] = 0;
        if (!strcmp(got, want)) return 1;
    }
    return 0;
}

/* `{ "Lime",    ZD_ACCENT },` - settings.c names the token, so read the
 * token's NAME and resolve it here rather than parsing a hex literal that
 * deliberately no longer exists in that file. */
static int settings_names_token(const char *src, const char *token)
{
    const char *p = strstr(src, "ACCENTS[] = {");
    if (!p) return 0;
    const char *end = strstr(p, "\n};");
    if (!end) return 0;
    const char *q = strstr(p, token);
    return q && q < end;
}

/* `fn theme(role) { return ui_color(role) }` */
static int zl_consumes_roles(const char *zl)
{
    return strstr(zl, "fn theme(role) { return ui_color(role) }") != NULL;
}

/* Declared here because the duplicate scan below needs it; the table itself
 * is further down, next to the tokens it lists. */
struct token { const char *name; unsigned rgb; };

/* Does kernel.zl carry a SECOND COPY of a theme colour?
 *
 * The first version of this asked whether kernel.zl contained any rgb(r,g,b)
 * literal at all, and answered 18. That check was wrong, not the code: most of
 * those are app CONTENT - the Paint app's seven swatches, the starfield demo's
 * two star colours - and content is not a palette role. A Paint app with no
 * colours in it is not a cleaner design, it is a broken app.
 *
 * The property DECISIONS item E actually asks for is that no theme role has a
 * second definition. So: decode every rgb(r,g,b) in kernel.zl and fail only if
 * one of them EQUALS a design.h token. That catches the real regression - a
 * second accent, a second panel colour - and lets content through.
 *
 * Returns the number of duplicated roles, and writes the first offender's
 * value to *dup for the message.
 */
static int zl_duplicates_theme(const char *zl, const struct token *toks, int n_toks,
                               unsigned *dup, const char **which)
{
    int n = 0;
    for (const char *p = zl; (p = strstr(p, "rgb(")) != NULL; p += 4) {
        int r, g, b;
        /* SKIP COMMENTS. zl comments run from '#' to end of line, and this
         * scanner used to read straight through them - so a comment SAYING
         * "rgb(7,8,10) is a duplicate, do not write it" was itself reported as
         * the duplicate. A gate that fails on its own documentation trains
         * people to stop writing documentation. */
        int in_comment = 0;
        for (const char *q = p; q > zl && q[-1] != '\n'; q--)
            if (q[-1] == '#') { in_comment = 1; break; }
        if (in_comment) continue;
        if (sscanf(p, "rgb(%d, %d, %d)", &r, &g, &b) != 3) continue;
        if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) continue;
        unsigned v = ((unsigned)r << 16) | ((unsigned)g << 8) | (unsigned)b;
        /* COUNT OCCURRENCES, NOT (literal, token) PAIRS. This used to n++
         * inside the token loop, so one rgb() literal scored once per token
         * that happened to hold that value. Under the predecessor's palette
         * every token was a distinct colour and the two counts agreed;
         * PRESSWORK aliases deliberately - ZD_ACCENT, ZD_VERM and
         * ZD_ACCENT_ALT_3 are all #E8734F - so a single planted duplicate
         * scored 3 and the control asserting `== 1` failed while the scanner
         * was working. The question is "how many literals in kernel.zl
         * duplicate a theme colour", and that is one number per literal. */
        for (int i = 0; i < n_toks; i++)
            if (toks[i].rgb == v) {
                if (n == 0) { *dup = v; *which = toks[i].name; }
                n++;
                break;
            }
    }
    return n;
}

/* ---- the tokens, as data --------------------------------------------------
 * Every colour constant design.h defines. The gate asserts each one occurs in
 * the reference; the NAME is carried so a failure says which token is invented
 * rather than just printing a number. */
static const struct token TOKENS[] = {
    /* the eight-rung surface ladder, which is the design's one structural
     * decision - the minimum step is 1.1682:1 so a knockout has a rung */
    { "ZD_CUT",         ZD_CUT         }, { "ZD_WELL",        ZD_WELL        },
    { "ZD_VOID",        ZD_VOID        }, { "ZD_BASE",        ZD_BASE        },
    { "ZD_RAISE",       ZD_RAISE       }, { "ZD_FLOAT",       ZD_FLOAT       },
    { "ZD_LITSOFT",     ZD_LITSOFT     }, { "ZD_LIT",         ZD_LIT         },
    /* the boundary colours and the knockout */
    { "ZD_EDGE_OVER",   ZD_EDGE_OVER   }, { "ZD_KNOCK",       ZD_KNOCK       },
    { "ZD_KO_EDGE",     ZD_KO_EDGE     }, { "ZD_KNOCK_INK",   ZD_KNOCK_INK   },
    { "ZD_KNOCK_INK2",  ZD_KNOCK_INK2  }, { "ZD_GRID",        ZD_GRID        },
    /* the ink ramp - four rungs and a fifth that must never carry a glyph */
    { "ZD_TEXT_0",      ZD_TEXT_0      }, { "ZD_TEXT_1",      ZD_TEXT_1      },
    { "ZD_TEXT_2",      ZD_TEXT_2      }, { "ZD_TEXT_3",      ZD_TEXT_3      },
    { "ZD_TEXT_INERT",  ZD_TEXT_INERT  },
    /* the two inks, and ink that goes ON the overprint */
    { "ZD_STEEL",       ZD_STEEL       }, { "ZD_STEEL_BR",    ZD_STEEL_BR    },
    { "ZD_VERM",        ZD_VERM        }, { "ZD_VERM_BR",     ZD_VERM_BR     },
    { "ZD_INK_ON",      ZD_INK_ON      },
    /* state, wired to state and never to the accent */
    { "ZD_OK",          ZD_OK          }, { "ZD_WARN",        ZD_WARN        },
    { "ZD_BAD",         ZD_BAD         }, { "ZD_BAD_INK",     ZD_BAD_INK     },
    /* THE ALIASES ARE CHECKED TOO, and that is the point of listing them.
     * design.h keeps the predecessor's names pointing at PRESSWORK rungs so no
     * call site broke; if one of them is ever re-pointed at an invented colour
     * this catches it in the same breath as a new token. */
    { "ZD_SURF_0",      ZD_SURF_0      }, { "ZD_SURF_1",      ZD_SURF_1      },
    { "ZD_SURF_2",      ZD_SURF_2      }, { "ZD_SURF_3",      ZD_SURF_3      },
    { "ZD_SURF_4",      ZD_SURF_4      }, { "ZD_SURF_5",      ZD_SURF_5      },
    { "ZD_SURF_6",      ZD_SURF_6      }, { "ZD_SURF_7",      ZD_SURF_7      },
    { "ZD_SURF_TABS",   ZD_SURF_TABS   }, { "ZD_SURF_WELL",   ZD_SURF_WELL   },
    { "ZD_SURF_CARD",   ZD_SURF_CARD   }, { "ZD_SURF_HEAD",   ZD_SURF_HEAD   },
    { "ZD_SURF_GAME",   ZD_SURF_GAME   }, { "ZD_SURF_BODY",   ZD_SURF_BODY   },
    { "ZD_SURF_BAR_OFF",ZD_SURF_BAR_OFF},
    { "ZD_TEXT_4",      ZD_TEXT_4      }, { "ZD_TEXT_5",      ZD_TEXT_5      },
    { "ZD_TEXT_6",      ZD_TEXT_6      },
    { "ZD_ACCENT",      ZD_ACCENT      }, { "ZD_ACCENT_BR",   ZD_ACCENT_BR   },
    { "ZD_ACCENT_LINK", ZD_ACCENT_LINK }, { "ZD_ACCENT_PALE", ZD_ACCENT_PALE },
    { "ZD_BAD_SOFT",    ZD_BAD_SOFT    },
    { "ZD_INK_DARK",    ZD_INK_DARK    }, { "ZD_INK_LIGHT",   ZD_INK_LIGHT   },
    { "ZD_TITLE_INK",     ZD_TITLE_INK     },
    { "ZD_TITLE_INK_KO",  ZD_TITLE_INK_KO  },
    { "ZD_TITLE_INK_OFF", ZD_TITLE_INK_OFF },
    { "ZD_WINCTL_INK",    ZD_WINCTL_INK    },
    { "ZD_WINCTL_RULE",   ZD_WINCTL_RULE   },
    { "ZD_CLOSE_HOVER_BG",  ZD_CLOSE_HOVER_BG  },
    { "ZD_CLOSE_HOVER_INK", ZD_CLOSE_HOVER_INK },
    { "ZD_RING_FOCUS",    ZD_RING_FOCUS    },
    { "ZD_WALL_100",      ZD_WALL_100      },
    { "ZD_WALL_GLOW",     ZD_WALL_GLOW     },
    { "ZD_ACCENT_ALT_1", ZD_ACCENT_ALT_1 }, { "ZD_ACCENT_ALT_2", ZD_ACCENT_ALT_2 },
    { "ZD_ACCENT_ALT_3", ZD_ACCENT_ALT_3 }, { "ZD_ACCENT_ALT_4", ZD_ACCENT_ALT_4 },
    /* THE COMPARISON LADDER. The PARENT designs' tokens, kept so the settings
     * pane can COMPUTE the comparison instead of quoting it. Nothing paints
     * with them - they are arguments to ui_ratio_q4 - but they are colours in
     * design.h and so they are held to exactly the same standard as the ones
     * that do: the prototype's own `--zd-ref-*` block, or nothing. */
    { "ZD_REF_LIT_RAKING",  ZD_REF_LIT_RAKING  },
    { "ZD_REF_BASE_RAKING", ZD_REF_BASE_RAKING },
    { "ZD_REF_KNOCK_PLATE", ZD_REF_KNOCK_PLATE },
    { "ZD_REF_WASH_RAKING", ZD_REF_WASH_RAKING },
    /* the two ends of the contrast axis. Listed so they are HELD to the rule
     * rather than quietly outside it - they are exempted below, with the
     * reason, and the staleness check keeps that exemption honest. */
    { "ZD_AXIS_BLACK",      ZD_AXIS_BLACK      },
    { "ZD_AXIS_WHITE",      ZD_AXIS_WHITE      },
};

/* ---- THE THREE TOKENS THAT ARE NOT LITERALLY IN THE PROTOTYPE --------------
 * Every exemption is named, and the list is checked for STALENESS in both
 * directions: a name here that HAS become literal fails the gate, so an
 * exemption cannot outlive its reason. That is the shape gen_icons.py's
 * OFF_AXIS_OK uses and it is the shape every exemption list in this tree
 * should use - an unchecked allowlist is where invented colours go to hide.
 *
 *   ZD_LIFT      the prototype writes it `#0000008C`, RGB plus alpha in one
 *                8-digit token. The scanner deliberately refuses a 6-digit
 *                prefix of a longer hex run - that refusal is itself asserted
 *                below - so #000000 cannot be found and must not be forced.
 *                The alpha half is ZD_LIFT_A, 0x8C == 140, and it agrees.
 *   ZD_WALL_0    the desk's two gradient stops. The prototype computes them
 *   ZD_WALL_42   with color-mix() in sRGB and never writes either down; the
 *                kernel cannot use those numbers anyway, because fb.c blends
 *                in LINEAR light and the same visual stop is a different byte
 *                there. They are derived through fb.c's own gamma tables, and
 *                the derivation - not a literal - is what makes them right. */
struct exempt { const char *name; unsigned rgb; };
static const struct exempt EXEMPT[] = {
    { "ZD_LIFT",    ZD_LIFT    },
    { "ZD_WALL_0",  ZD_WALL_0  },
    { "ZD_WALL_42", ZD_WALL_42 },
    /*   ZD_FOCUS_WASH  the fourth, and it is derived for the same reason
     *                  ZD_WALL_0 is. The prototype's fallback header is
     *                  ZD_BASE with a gradient over it whose loudest stop is
     *                  `color-mix(in srgb, var(--zd-lit) 34%, transparent)` -
     *                  a colour it never writes down as a hex. The value is
     *                  ZD_LIT at alpha 0.34 composited on ZD_BASE, computed
     *                  per channel in design.h's comment, and the assertion
     *                  below checks that derivation rather than the literal. */
    { "ZD_FOCUS_WASH", ZD_FOCUS_WASH },
    /*   ZD_AXIS_BLACK  the ends of the WCAG contrast axis, and they are not
     *   ZD_AXIS_WHITE  design colours at all - nothing paints with either and
     *                  no theme role is assigned one. They are in design.h
     *                  because the "no colour literal outside this file" rule
     *                  is enforced by a scanner that counts six hex digits and
     *                  does not take intent as an argument; it caught them
     *                  written as 0x000000 inside ui_ceil_dn_q4. Neither
     *                  occurs in the prototype, which is correct: on this
     *                  ladder pure black is louder than the groove and pure
     *                  white is twice the knockout, so neither is drawable. */
    { "ZD_AXIS_BLACK", ZD_AXIS_BLACK },
    { "ZD_AXIS_WHITE", ZD_AXIS_WHITE },
};
#define N_EXEMPT ((int)(sizeof EXEMPT / sizeof EXEMPT[0]))

static int is_exempt(unsigned rgb)
{
    for (int i = 0; i < N_EXEMPT; i++) if (EXEMPT[i].rgb == rgb) return 1;
    return 0;
}

#define N_TOKENS ((int)(sizeof TOKENS / sizeof TOKENS[0]))

/* ---- the roles, as data ---------------------------------------------------
 * Which design.h token each ui_theme field must hold. This is link 2->3.
 * A role whose token is wrong is a role painted in a colour the reference
 * uses somewhere - just not there - which is the failure mode a bare
 * "is it in the reference" check cannot see. */
struct role { const char *what; const char *token; unsigned want; size_t off; };
#define F(name) offsetof(struct ui_theme, name)

static const struct role ROLES[] = {
    { "desktop background", "ZD_VOID",         ZD_VOID,         F(bg)             },
    { "window body",        "ZD_BASE",         ZD_BASE,         F(panel)          },
    { "control face",       "ZD_RAISE",        ZD_RAISE,        F(panel_hi)       },
    { "primary text",       "ZD_TEXT_1",       ZD_TEXT_1,       F(text)           },
    { "secondary text",     "ZD_TEXT_3",       ZD_TEXT_3,       F(text_dim)       },
    { "the overprint",      "ZD_VERM",         ZD_VERM,         F(accent)         },
    { "the groove",         "ZD_CUT",          ZD_CUT,          F(border)         },
    { "destructive",        "ZD_BAD",          ZD_BAD,          F(danger)         },
    /* all four title roles are the plate at rest; focus is the knockout, and
     * that is a decision rather than four copies of a missing one */
    { "focused title top",  "ZD_BASE",         ZD_BASE,         F(title)          },
    { "focused title bot",  "ZD_BASE",         ZD_BASE,         F(title_bot)      },
    { "unfocused title",    "ZD_BASE",         ZD_BASE,         F(title_off)      },
    { "unfocused title bot","ZD_BASE",         ZD_BASE,         F(title_off_bot)  },
    { "emphasis text",      "ZD_TEXT_0",       ZD_TEXT_0,       F(text_hi)        },
    { "healthy",            "ZD_OK",           ZD_OK,           F(ok)             },
    { "chrome hairline",    "ZD_CUT",          ZD_CUT,          F(chrome_line)    },
    /* PRESSWORK's own roles. These are the grammar the twenty-nine above
     * cannot express - a boundary is a colour, and focus is a value
     * inversion - so a wrong one here is a wrong DEPTH, not a wrong tint. */
    { "the struck run",     "ZD_LIT",          ZD_LIT,          F(lit)            },
    { "the grazed run",     "ZD_LITSOFT",      ZD_LITSOFT,      F(litsoft)        },
    { "boundary under overlap","ZD_EDGE_OVER", ZD_EDGE_OVER,    F(edge_over)      },
    { "THE KNOCKOUT",       "ZD_KNOCK",        ZD_KNOCK,        F(knock)          },
    { "reversed out of it", "ZD_KNOCK_INK",    ZD_KNOCK_INK,    F(knock_ink)      },
    { "secondary on it",    "ZD_KNOCK_INK2",   ZD_KNOCK_INK2,   F(knock_ink2)     },
    { "the knockout's foot","ZD_KO_EDGE",      ZD_KO_EDGE,      F(ko_edge)        },
    { "the ruled desk grid","ZD_GRID",         ZD_GRID,         F(grid)           },
    { "instruments only",   "ZD_STEEL",        ZD_STEEL,        F(steel)          },
    { "ink on the overprint","ZD_INK_ON",      ZD_INK_ON,       F(ink_on)         },
    /* STRUCTURE ONLY - the scrollbar thumb, the resize grip, the switch knob.
     * It is 2.02:1 on ZD_RAISE and must never form a letter. */
    { "structure, never ink","ZD_TEXT_INERT",  ZD_TEXT_INERT,   F(surf_7)         },
};
#define N_ROLES ((int)(sizeof ROLES / sizeof ROLES[0]))

int main(void)
{
    char *html = slurp("../../../docs/design/presswork-prototype.html");
    char *zl   = slurp("../../src/kernel.zl");
    char *set  = slurp("../../src/graphics/ui/settings.c");

    ui_theme_init(2);
    const struct ui_theme *t = ui_theme();

    printf("palette - the chain from the reference to the pixels\n\n");

    /* ---- link 1 -> 2 : every token is IN the artifact ---------------------- */
    printf("  design.h -> presswork-prototype.html\n");
    int invented = 0;
    for (int i = 0; i < N_TOKENS; i++) {
        if (ref_has_colour(html, TOKENS[i].rgb)) continue;
        if (is_exempt(TOKENS[i].rgb)) continue;
        printf("    #%06X  %-22s NOT IN THE PROTOTYPE\n",
               TOKENS[i].rgb, TOKENS[i].name);
        invented++;
    }
    printf("    %d tokens checked, %d exempt by name, %d not found\n\n",
           N_TOKENS, N_EXEMPT, invented);
    ok(invented == 0,
       "every colour in design.h occurs literally in the prototype");
    /* THE EXEMPTION LIST'S OWN STALENESS CHECK. An allowlist nobody re-reads
     * is how an invented colour survives a redesign: the entry outlives the
     * reason, and the next person reads it as permission. */
    int stale = 0;
    for (int i = 0; i < N_EXEMPT; i++)
        if (ref_has_colour(html, EXEMPT[i].rgb)) {
            printf("    #%06X  %-22s IS in the prototype - drop the exemption\n",
                   EXEMPT[i].rgb, EXEMPT[i].name);
            stale++;
        }
    ok(stale == 0, "no exemption has outlived its reason");

    /* ---- link 2 -> 3 : every role holds the token it claims ---------------- */
    printf("\n  design.h -> ui.c\n");
    printf("    %-22s %-18s %-9s %-9s\n", "role", "token", "design.h", "ui.c");
    int agree = 0;
    for (int i = 0; i < N_ROLES; i++) {
        const struct role *r = &ROLES[i];
        unsigned ui = *(const unsigned *)((const char *)t + r->off);
        printf("    %-22s %-18s #%06X   #%06X", r->what, r->token, r->want, ui);
        if (ui == r->want) agree++; else printf("   <-- ui.c differs");
        printf("\n");
    }
    printf("\n");
    ok(agree == N_ROLES, "every ui_theme role holds the design.h token it names");

    /* ---- the accent cannot come back through the panel --------------------- */
    ok(settings_names_token(set, "ZD_ACCENT"),
       "settings.c's default accent names ZD_ACCENT rather than a hex literal");
    ok(settings_names_token(set, "ZD_ACCENT_ALT_4"),
       "settings.c offers the reference's own five accents");

    /* ...and the values must actually DIFFER. The line above only establishes
     * that settings.c mentions the token; it reported green while ALT_3 was an
     * alias of ZD_ACCENT itself, so the picker drew five chips carrying four
     * colours with two byte-identical. Checking the text was strictly weaker
     * than the sentence it printed - the shape
     * docs/GUARDS-THAT-DID-NOT-GUARD.md is about. */
    {
        unsigned acc[5] = { ZD_ACCENT, ZD_ACCENT_ALT_1, ZD_ACCENT_ALT_2,
                            ZD_ACCENT_ALT_3, ZD_ACCENT_ALT_4 };
        int dup = 0;
        for (int i = 0; i < 5; i++)
            for (int j = i + 1; j < 5; j++)
                if (acc[i] == acc[j]) dup++;
        ok(dup == 0, "the five accent swatches are five DISTINCT colours");
    }

    /* ---- kernel.zl still holds no colour ----------------------------------- */
    ok(zl_consumes_roles(zl),
       "kernel.zl consumes semantic roles from ui.c instead of copying RGB");
    unsigned dup = 0; const char *dupname = NULL;
    int ndup = zl_duplicates_theme(zl, TOKENS, N_TOKENS, &dup, &dupname);
    if (ndup) printf("    kernel.zl re-defines #%06X (%s) %d time%s\n",
                     dup, dupname, ndup, ndup == 1 ? "" : "s");
    ok(ndup == 0, "kernel.zl carries no second copy of a theme colour");

    /* ---- NEGATIVE CONTROLS -------------------------------------------------
     * Every assertion above is a search that could silently succeed or
     * silently fail for the wrong reason. These prove the searcher works:
     * that it finds a colour that IS there, refuses one that is not, and
     * does not match a six-digit prefix of a longer run of hex. */
    printf("\n");
    /* THE CONTROL COLOUR HAD TO CHANGE TOO, and it is worth saying why: this
     * used to look for #b8e838, the predecessor's lime, and #b8e838 is not in
     * the PRESSWORK prototype at all. A negative control that quietly stops
     * being positive turns into a second failing assertion nobody reads. */
    ok(ref_has_colour(html, ZD_VERM),
       "control: the scanner finds #e8734f, the overprint, which is in the file");
    ok(!ref_has_colour(html, 0xB8E838),
       "control: it refuses #b8e838 - the predecessor's lime is really gone");
    ok(!ref_has_colour(html, 0x123456),
       "control: ...and refuses #123456, which was never in any of them");
    /* The scanner's job is to find #b8e838 and NOT to find it inside a longer
     * run of hex such as an 8-digit #rrggbbaa. Asserted against a synthetic
     * string, because the real reference happens to contain the bare form too
     * and so cannot distinguish the two behaviours. */
    ok(!ref_has_colour("box-shadow: 0 5px 13px #0000008C;", 0x000000),
       "control: it does not match a six-digit prefix of a longer hex run");
    ok(ref_has_colour("box-shadow: 0 5px 13px #000000;", 0x000000),
       "control: ...but it does match the same colour when it stands alone");
    ok(!settings_names_token(set, "ZD_NO_SUCH_TOKEN"),
       "control: the settings scanner reports a missing token rather than true");

    /* THE PLANTED DEFECT. GUARDS-THAT-DID-NOT-GUARD.md is a list of checks in
     * this tree that reported green while checking nothing. The duplicate scan
     * is the easiest one here to get wrong - it passes trivially if the parser
     * never decodes anything - so it is run against a string that DOES contain
     * a second accent, and must find it. If this control ever fails, the
     * duplicate check above is green for the wrong reason. */
    {
        static const char planted[] =
            "fn theme(role) { return ui_color(role) }\n"
            "MY_ACCENT = rgb(232, 115, 79)\n";    /* == ZD_ACCENT #E8734F */
        unsigned pv = 0; const char *pn = NULL;
        ok(zl_duplicates_theme(planted, TOKENS, N_TOKENS, &pv, &pn) == 1
           && pv == ZD_ACCENT,
           "control: a planted second accent IS caught by the duplicate scan");

        /* ...and the comment-skip must not have turned the scan off. The same
         * duplicate, once inside a comment and once not, must be found exactly
         * once. Without the second half of this, "skip comments" could be
         * implemented as "skip everything" and every check above would pass. */
        static const char mixed[] =
            "# MY_ACCENT = rgb(232, 115, 79) would be a duplicate\n"
            "MY_ACCENT = rgb(232, 115, 79)\n";
        unsigned mv = 0; const char *mn = NULL;
        ok(zl_duplicates_theme(mixed, TOKENS, N_TOKENS, &mv, &mn) == 1,
           "control: a commented duplicate is ignored, a real one beside it is not");
    }

    /* ---- THE CONTRAST ENGINE, AGAINST design.h's OWN PROSE -----------------
     * ui_ratio_q4 is what makes the settings pane an argument instead of a
     * caption: every figure it prints is recomputed from the loaded tokens.
     * That only means anything if the recomputation is right, and "right" has
     * an external definition here - design.h states these ratios in prose,
     * derived independently with IEEE doubles, and this is the integer engine
     * having to reproduce them to four decimals from a 256-entry sRGB table.
     *
     * IT IS THE HARDER DIRECTION ON PURPOSE. Comparing the engine to itself
     * would pass with the table full of zeroes. Comparing it to the numbers
     * somebody wrote down after doing the maths a different way catches a
     * wrong coefficient, a truncation where a rounding belongs, and an
     * off-by-one in the table - the three ways this can be subtly wrong. */
    printf("\n  the contrast engine - integer WCAG against design.h's prose\n");
    ok(ui_ratio_q4(ZD_KNOCK, ZD_BASE)        == 64796, "ZD_KNOCK on ZD_BASE is 6.4796:1");
    ok(ui_ratio_q4(ZD_KO_EDGE, ZD_KNOCK)     == 25487, "ZD_KO_EDGE on ZD_KNOCK is 2.5487:1");
    ok(ui_ratio_q4(ZD_LIT, ZD_BASE)          == 25423, "ZD_LIT on ZD_BASE is 2.5423:1");
    ok(ui_ratio_q4(ZD_VERM, ZD_BASE)         == 46319, "ZD_VERM on ZD_BASE is 4.6319:1");
    ok(ui_ratio_q4(ZD_CUT, ZD_BASE)          == 14723, "ZD_CUT on ZD_BASE is 1.4723:1");
    ok(ui_ratio_q4(ZD_KNOCK_INK, ZD_KNOCK)   == 85329, "ZD_KNOCK_INK on the knockout is 8.5329:1");
    ok(ui_ratio_q4(ZD_KNOCK_INK2, ZD_KNOCK)  == 46965, "ZD_KNOCK_INK2 on the knockout is 4.6965:1");
    ok(ui_ratio_q4(ZD_STEEL, ZD_BASE)        == 61881, "ZD_STEEL on ZD_BASE is 6.1881:1");
    ok(ui_ratio_q4(ZD_EDGE_OVER, ZD_FLOAT)   == 34322, "ZD_EDGE_OVER on ZD_FLOAT is 3.4322:1 - the worst case");
    ok(ui_ratio_q4(ZD_EDGE_OVER, ZD_BASE)    == 49991, "ZD_EDGE_OVER on ZD_BASE is 4.9991:1");
    ok(ui_ratio_q4(ZD_RAISE, ZD_FLOAT)       == 11682, "the smallest surface step is 1.1682:1");
    ok(ui_lstar_q2(0x000000) == 0 && ui_lstar_q2(0xFFFFFF) == 10000,
       "CIE L star spans exactly 0.00 through 100.00");
    ok(ui_lstar_q2(ZD_BASE) == 1815 && ui_lstar_q2(ZD_FLOAT) == 2941,
       "live ladder L star matches an independent sRGB calculation");
    ok(ui_lstar_q2(ZD_TEXT_3) == 7319 && ui_lstar_q2(ZD_TEXT_0) == 9153,
       "the text ramp exposes its live L star endpoints");
    /* ORDER INDEPENDENCE. WCAG defines the ratio lighter-over-darker, so no
     * caller has to know which of its two colours is on top - and every table
     * in the settings pane relies on that, because a cell passes a token and
     * a ground without sorting them. */
    ok(ui_ratio_q4(ZD_BASE, ZD_KNOCK) == ui_ratio_q4(ZD_KNOCK, ZD_BASE),
       "ui_ratio_q4 is order independent");
    /* THE TWO CEILINGS, which are what decide which side an edge goes on.
     * design.h's header states both for ZD_BASE and derives the whole
     * asymmetric-widening argument from them. */
    ok(ui_ceil_dn_q4(ZD_BASE) == 15105, "ZD_BASE has 1.5105:1 of room downward");
    ok(ui_ceil_up_q4(ZD_BASE) == 139030, "ZD_BASE has 13.9030:1 of room upward");
    ok(ui_ceil_dn_q4(ZD_KNOCK) > ui_ceil_up_q4(ZD_KNOCK),
       "on the knockout the room runs DOWNWARD - which is why its edge is a foot rule");
    ok(ui_ceil_dn_q4(ZD_BASE) < ui_ceil_up_q4(ZD_BASE),
       "on the plate the room runs UPWARD - which is why its edge is a top run");
    /* THE SKEW, and it is the widening's central claim in one assertion: the
     * edge run computes the SAME ratio against the knockout as against the
     * ground, so the light does not invert its loudness when the header
     * inverts. design.h says 0.25 %; 64 parts in 25423 is 0.2517 %. */
    ok(ui_ratio_q4(ZD_KO_EDGE, ZD_KNOCK) - ui_ratio_q4(ZD_LIT, ZD_BASE) == 64,
       "the edge run is within 0.25% of itself in both geometries");
    /* THE FALLBACK, derived rather than found. ZD_LIT at alpha 0.34 on
     * ZD_BASE, per channel - this is the calculation design.h's comment shows,
     * run, so the token cannot drift from the comment that justifies it. */
    {
        unsigned mix = 0;
        for (int sh = 16; sh >= 0; sh -= 8) {
            unsigned a = (ZD_LIT >> sh) & 255u, b = (ZD_BASE >> sh) & 255u;
            /* 34/100, rounded to nearest - the same rounding the comment used */
            unsigned c = (b * 100u + (a - b) * 34u + 50u) / 100u;
            mix |= c << sh;
        }
        ok(mix == ZD_FOCUS_WASH, "ZD_FOCUS_WASH is ZD_LIT at 34% on ZD_BASE, recomputed");
        ok(ui_ratio_q4(ZD_FOCUS_WASH, ZD_BASE) == 13681,
           "...and it computes 1.3681:1, NOT the prototype's quoted 1.3999 - see design.h");
    }
    /* THE COMPARISON LADDER. The sign inversion the knockout was designed
     * around, computed from the parent designs' own tokens: one value, two
     * grounds, 3.00x the loudness and the opposite sign. design.h's header
     * quotes the third of these at line 129 as the failure being replaced. */
    ok(ui_ratio_q4(ZD_REF_LIT_RAKING, ZD_REF_BASE_RAKING) == 19931,
       "the parent's struck run is 1.9931:1 on its ground - a LIGHTER term");
    ok(ui_ratio_q4(ZD_REF_LIT_RAKING, ZD_REF_KNOCK_PLATE) == 59822,
       "...and 5.9822:1 on PLATE's knockout - a DARKER one");
    ok(ui_ratio_q4(ZD_REF_KNOCK_PLATE, ZD_REF_BASE_RAKING) == 119231,
       "PLATE's knockout on the un-widened ladder is 11.9231:1, as design.h states");

    /* ---- the two live controls the settings pane owns ---------------------- */
    printf("\n  the FOCUS tab's two controls\n");
    ok(ui_knockout_get() == 1, "the knockout is ON by default");
    ok(ui_theme()->knock == ZD_KNOCK, "...and theme.knock is the knockout");
    ui_knockout_set(0);
    ok(ui_theme()->knock == ZD_FOCUS_WASH, "off swaps the fill for graphite's wash");
    ok(ui_theme()->ko_edge == ZD_CUT, "...the foot rule goes back to the groove");
    ok(ui_theme()->knock_ink == ZD_TEXT_0,
       "...and the title stops being reversed out, or it would be 1.29:1");
    /* THE REGRESSION THIS EXISTS TO CATCH. The switches live above the theme
     * struct because ui_theme_init_q8 rebuilds it wholesale on every scale
     * change; a switch stored INSIDE it is silently reverted the next time
     * anybody drags the UI-scale slider, and the setting appears to work and
     * then quietly undoes itself. */
    ui_theme_init_q8(384);
    ok(ui_theme()->knock == ZD_FOCUS_WASH, "the knockout switch survives a rescale");
    ui_knockout_set(1);
    ok(ui_theme()->knock == ZD_KNOCK, "...and back on again");

    ok(ui_focus_bar_dp() == ZD_FOCUS_BAR, "the focus bar starts at its design width");
    ok(ui_focus_bar_set(6) == 6, "the slider reaches its top value");
    ok(ui_focus_bar_set(99) == UI_FBAR_MAX, "...and clamps above it");
    ok(ui_focus_bar_set(-4) == UI_FBAR_MIN, "...and below it");
    /* THE RETURN VALUE IS THE STATE IT SETTLED ON, not the argument - a caller
     * that echoed its own argument back into the slider would draw a thumb
     * past the end of its own track. */
    ui_focus_bar_set(3);
    ui_theme_init_q8(512);
    ok(ui_theme()->focus_bar == 6, "the bar is stored in dp, so it rescales: 3dp at 2x is 6px");
    ok(ui_focus_bar_dp() == 3, "...and the dp value itself is unchanged by the rescale");
    ui_theme_init_q8(512);
    ok(ui_theme()->focus_bar == 6, "the focus-bar width survives a rescale too");
    ui_theme_init(2);

    printf("\n%s (%d failure%s)\n", fails ? "FAILED" : "all passed",
           fails, fails == 1 ? "" : "s");
    free(html); free(zl); free(set);
    return fails ? 1 : 0;
}
