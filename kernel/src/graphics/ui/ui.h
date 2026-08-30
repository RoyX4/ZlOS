/* ui.h - the contract between the compositor, the toolkit and the apps.
 *
 * Three layers meet here and each may only call DOWN:
 *
 *   kernel.zl   POLICY    theme values, which apps exist, what each draws
 *       |                 calls ui_* only - no coordinates, no fb_* calls
 *   ui.c        WIDGETS   layout cursor, buttons, labels, the theme
 *       |                 calls fb_* - never touches windows or z-order
 *   wm.c        WINDOWS   window table, z-order, focus, damage, routing
 *       |                 calls fb_clip + app_draw - never draws a widget
 *   fb.c        PIXELS    primitives, the clip rect, the damage list
 *
 * If something here needs to call UP, the boundary is in the wrong place.
 * Adding a back-channel instead of moving it is how these three become one.
 */
#ifndef UI_H
#define UI_H

/* ---- the theme, as DATA ---------------------------------------------------
 * Stolen from SerenityOS, where window frame colours and metrics live in a
 * theme object rather than being spelled out in the painting code. zlOS
 * hardcoded both, in two different files, with numbers picked by eye per
 * window - `sx + 14 * u` here, `sy + 58 * u` there, `214 * u` for a bar.
 *
 * Every colour and every distance the desktop draws comes from here now.
 * SPACING IS A SCALE: 4 / 8 / 12 / 16 / 24, times ui(). Nothing may use a
 * value outside it. That single rule is most of what "designed" means, and it
 * is the difference between a layout and a pile of numbers that happened to
 * look right on one screen.
 */
struct ui_theme {
    /* colour roles - not a palette. Every colour on screen is one of these,
     * surfaces are low saturation, and saturation is spent on the accent. */
    unsigned bg;          /* the desktop behind everything      */
    unsigned panel;       /* a raised surface: window interior  */
    unsigned panel_hi;    /* a surface raised above THAT        */
    unsigned text;        /* primary text                       */
    unsigned text_dim;    /* secondary text                     */
    unsigned accent;      /* the one saturated colour           */
    unsigned border;      /* hairlines and outlines             */
    unsigned danger;      /* destructive only - the close box   */
    unsigned title;       /* focused title bar, top of gradient */
    unsigned title_bot;   /* focused title bar, bottom          */
    unsigned title_off;   /* unfocused title bar, flat          */
    unsigned wallpaper_top;
    unsigned wallpaper_bot;
    unsigned bar_top;
    unsigned bar_bot;
    unsigned bar_hi;
    unsigned chrome;
    unsigned chrome_line;
    unsigned text_hi;
    unsigned ok;
    /* NINE MORE, added because the twenty above could not say what the
     * design says. An app slice reported collapsing three text rungs onto two
     * roles, "warning" onto the accent, the sunken well onto the background
     * and the dimmest mark onto the border - six tokens rendering as three
     * colours. A palette with no name for "warning" spends the accent on it,
     * and then the accent no longer means "this is the one thing to act on".
     * These are appended, so no existing ui_color() index moves. */
    unsigned text_2;      /* ZD_TEXT_2   secondary               */
    unsigned text_5;      /* labels, column heads                */
    unsigned text_6;      /* hints, units - see ui.c on the ramp */
    unsigned warn;        /* ZD_WARN     NOT the accent          */
    unsigned surf_1;      /* ZD_WELL     sunken wells            */
    unsigned surf_5;      /* ZD_FLOAT    menus, modals, toasts   */
    unsigned surf_7;      /* ZD_LIT      the dimmest mark        */
    unsigned surf_well;   /* ZD_WELL                             */
    unsigned accent_br;   /* ZD_VERM_BR  the overprint, full     */
    /* This used to be a C-only role parked after the zl-visible colour array.
     * PRESSWORK appends thirteen roles behind it, so it is now inside the
     * array and has an enum name of its own; the index it already occupied
     * (29) did not move. */
    unsigned title_off_bot;

    /* ---- PRESSWORK's own roles, appended at indices 30..41 -----------------
     * FIELD ORDER IS THE ABI. ui_color() indexes this struct as a flat array
     * of unsigned, so enum ui_color_role below and these declarations are the
     * same list written twice and must stay in the same order. Everything new
     * goes AFTER accent_br/title_off_bot, never between.
     *
     * These exist because PRESSWORK's depth grammar is not a set of fills: the
     * boundary between two surfaces is a colour in its own right, and the
     * focused header is a full value inversion rather than a tint. Neither can
     * be expressed by "panel, panel_hi, border". */
    unsigned cut;         /* 30  the 1px groove, and the pit ground          */
    unsigned lit;         /* 31  the struck TOP run, from the one lamp       */
    unsigned litsoft;     /* 32  the grazed LEFT run                         */
    unsigned edge_over;   /* 33  the boundary UNDER OVERLAP. the occluder    */
                          /*     draws it; it never appears unoccluded.      */
    unsigned knock;       /* 34  THE KNOCKOUT - the focused header plate     */
    unsigned knock_ink;   /* 35  the title, reversed out of the knockout     */
    unsigned knock_ink2;  /* 36  secondary ink on the knockout               */
    unsigned ko_edge;     /* 37  the knockout's own edge run, on its FOOT    */
    unsigned grid;        /* 38  the ruled module grid on the desk           */
    unsigned steel;       /* 39  INSTRUMENTS ONLY. never a control.          */
    unsigned steel_br;    /* 40  the instrument's own highlight              */
    unsigned ink_on;      /* 41  ink that goes ON the overprint              */
    unsigned bad_ink;     /* 42  failure TEXT on a dark row. design.h says so
                           *     and nothing carried it: theme.danger is ZD_BAD,
                           *     a fill at 4.2591:1, which is a different colour
                           *     and a different job.                          */

    /* metrics, all already multiplied by the UI scale. These are NOT in the
     * flat colour array - ui_metric() is a switch, so appending here is free. */
    int pad;              /* inside a panel, edge to content    */
    int gap;              /* between two widgets                */
    int row_h;            /* one line of controls               */
    int radius;           /* the OUTER corner. Inner is this - 1 */
    int title_h;          /* window title bar                   */
    int scale;            /* ui(): 1 at 8px cells, 2 at 16px    */
    int scale_q8;         /* continuous scale, 256 == 1 design unit */
    /* PRESSWORK's fixed shell bands. The shell is printed furniture: it is
     * ruled once and does not reflow, so each band is a metric rather than a
     * fraction of the screen. */
    int focus_bar;        /* the vermilion focus bar, 3dp        */
    int rail_w;           /* the left tool column                */
    int strip_h;          /* the raster readout across the top   */
    int foot_h;           /* the status / ruler band at the foot */
    int band_h;           /* a section band inside a plate       */
};

/* The scale everything snaps to. Use these, never a literal. */
#define UI_DP(t,n) ((((n) * (t)->scale_q8) + 128) >> 8)
#define UI_S1(t)  UI_DP((t), 4)
#define UI_S2(t)  UI_DP((t), 8)
#define UI_S3(t)  UI_DP((t), 12)
#define UI_S4(t)  UI_DP((t), 16)
#define UI_S6(t)  UI_DP((t), 24)

enum ui_color_role {
    UI_COLOR_BG = 0, UI_COLOR_PANEL, UI_COLOR_PANEL_HI, UI_COLOR_TEXT,
    UI_COLOR_TEXT_DIM, UI_COLOR_ACCENT, UI_COLOR_BORDER, UI_COLOR_DANGER,
    UI_COLOR_TITLE, UI_COLOR_TITLE_BOT, UI_COLOR_TITLE_OFF,
    UI_COLOR_WALL_TOP, UI_COLOR_WALL_BOT,
    UI_COLOR_BAR_TOP, UI_COLOR_BAR_BOT, UI_COLOR_BAR_HI,
    UI_COLOR_CHROME, UI_COLOR_CHROME_LINE, UI_COLOR_TEXT_HI, UI_COLOR_OK,
    UI_COLOR_TEXT_2, UI_COLOR_TEXT_5, UI_COLOR_TEXT_6, UI_COLOR_WARN,
    UI_COLOR_SURF_1, UI_COLOR_SURF_5, UI_COLOR_SURF_7, UI_COLOR_SURF_WELL,
    UI_COLOR_ACCENT_BR,
    /* 29. Was a C-only field; named now because PRESSWORK's roles sit behind
     * it and the flat index has to stay contiguous to reach them. */
    UI_COLOR_TITLE_OFF_BOT,
    /* 30..41 - PRESSWORK. Same order as struct ui_theme, because ui_color()
     * casts the struct to unsigned[] and the two lists ARE one list.
     * kernel.zl's TH_* numbering must match these exactly. */
    UI_COLOR_CUT, UI_COLOR_LIT, UI_COLOR_LITSOFT, UI_COLOR_EDGE_OVER,
    UI_COLOR_KNOCK, UI_COLOR_KNOCK_INK, UI_COLOR_KNOCK_INK2, UI_COLOR_KO_EDGE,
    UI_COLOR_GRID, UI_COLOR_STEEL, UI_COLOR_STEEL_BR, UI_COLOR_INK_ON,
    UI_COLOR_BAD_INK,
    UI_COLOR_COUNT
};
enum ui_metric_role {
    UI_METRIC_PAD = 0, UI_METRIC_GAP, UI_METRIC_ROW_H, UI_METRIC_RADIUS,
    UI_METRIC_TITLE_H, UI_METRIC_SCALE_Q8,
    /* PRESSWORK's shell bands. ui_metric() is a switch, not a flat index, so
     * these are appended without an ABI consequence. */
    UI_METRIC_FOCUS_BAR, UI_METRIC_RAIL_W, UI_METRIC_STRIP_H,
    UI_METRIC_FOOT_H, UI_METRIC_BAND_H,
    UI_METRIC_COUNT
};

/* THE GUARD FOR THAT ABI, and it is a real one rather than a comment asking to
 * be careful. ui_color() does `((const unsigned *)&theme.bg)[role]`, so a field
 * inserted in the middle of the colour block, or an enumerator added out of
 * order, silently re-points every role after it - the failure mode is a screen
 * that looks nearly right, which is the hardest kind to notice. These two
 * anchor the far end and the start of the appended block, so either mistake is
 * a compile error instead. Move a field, and this stops building. */
_Static_assert(__builtin_offsetof(struct ui_theme, cut)
                   == (unsigned)UI_COLOR_CUT * sizeof(unsigned),
               "struct ui_theme and enum ui_color_role disagree at UI_COLOR_CUT");
_Static_assert(__builtin_offsetof(struct ui_theme, bad_ink)
                   == (unsigned)UI_COLOR_BAD_INK * sizeof(unsigned),
               "struct ui_theme and enum ui_color_role disagree at the end of "
               "the colour array; UI_COLOR_COUNT no longer describes it");

/* ...and this third one, which is the only one that catches the OTHER
 * direction. The two above anchor NAMED fields to NAMED enumerators, so they
 * both stay green if you add an enumerator and no field: UI_COLOR_COUNT simply
 * grows, every existing pair still agrees, and ui_color(COUNT-1) walks one
 * slot PAST the colour block into theme.pad - an int metric read as a colour.
 * Demonstrated rather than feared: inserting one enumerator before
 * UI_COLOR_COUNT compiled clean and made ui_color(42) return 0x00000014, which
 * is 20, which is theme.pad.
 *
 * Anchoring the first field AFTER the block closes it, because that offset is
 * the block's true length and it cannot be satisfied by an enum that has grown
 * without the struct. pad is the first metric and must stay first for this to
 * mean anything. */
_Static_assert(__builtin_offsetof(struct ui_theme, pad)
                   == (unsigned)UI_COLOR_COUNT * sizeof(unsigned),
               "enum ui_color_role has grown without struct ui_theme; "
               "ui_color(UI_COLOR_COUNT-1) would read a metric as a colour");

const struct ui_theme *ui_theme(void);
void ui_theme_init(int scale);          /* build the default theme at a scale */
void ui_theme_init_q8(int scale_q8);    /* continuous scale; 256 == 1x */
void ui_theme_set(const struct ui_theme *t);
unsigned ui_color(int role);            /* shared C/zl source of colour truth */
int ui_metric(int role);

/* ---- WHAT TYPE IS IN THIS IMAGE --------------------------------------------
 * What the TYPE app is built on. Exposed rather than kept private because it
 * must be callable from kernel.zl, and because it is a MEASUREMENT of the
 * linked font arrays - not a table of numbers a comment claims are true. See
 * ui.c for the derivation, and for the one thing the prototype asserted that
 * this image does not have.
 *
 * Contrast itself is NOT here - ui_ratio_q4() above already is it, at the same
 * x10^4 fixed point, and a second copy of a WCAG table is exactly the kind of
 * drift design.h's one-file rule exists to stop. */
int ui_atlas_n(void);              /* atlases the TYPE pane knows about      */
int ui_atlas_w(int i);             /* cell width, px - sizeof, not typed in  */
int ui_atlas_h(int i);             /* cell height, px                        */
int ui_atlas_glyphs(int i);        /* coverage                               */
int ui_atlas_face(int i);          /* 0 mono, 1 sans, 2 sans bold            */
int ui_atlas_in_image(int i);      /* 0 == declared by the design, not linked */
int ui_atlas_for_role(int role, int weight); /* which one, at this scale, now */

/* ---- what the settings pane needs, and nothing else needs ------------------
 * ui_ratio_q4 and the two ceilings are WCAG contrast in integer fixed point,
 * x10^4, computed from a 256-entry sRGB table rather than from a pow() that
 * does not exist here. See ui.c for the accuracy measurement.
 *
 * ui_knockout_* and ui_focus_bar_* are the two live controls the settings
 * pane's FOCUS tab owns. Both rebuild the theme and NEITHER repaints - the
 * caller damages afterwards, because surfaces belong to wm.c and this layer
 * must not know about them. */
unsigned ui_ratio_q4(unsigned a, unsigned b);
unsigned ui_ceil_dn_q4(unsigned rgb);   /* room downward: ratio to black    */
unsigned ui_ceil_up_q4(unsigned rgb);   /* room upward:   ratio to white    */
int ui_us_get(void);                    /* 0 measured, 1 repaint, 2 off     */
int ui_us_set(int m);
int ui_knockout_get(void);              /* 1 == the knockout is on          */
int ui_knockout_set(int on);            /* returns the state it settled on  */
int ui_over_get(void);                  /* the occlusion edge               */
int ui_over_set(int on);
int ui_motion_get(void);                /* animation timings                */
int ui_motion_set(int on);
int ui_track_get(void);                 /* letter-spacing on tracked faces  */
int ui_track_set(int on);
/* the focus bar's width in DESIGN px, and the prototype's own slider range. */
#define UI_FBAR_MIN 1
#define UI_FBAR_MAX 6
int ui_focus_bar_dp(void);
int ui_focus_bar_set(int n);

/* THE COMPARISON LADDER, by index. design.h's ZD_REF_* block - the PARENT
 * designs' tokens, kept so the settings pane can COMPUTE the comparison rather
 * than quote it. Nothing paints with them; they are arguments to ui_ratio_q4.
 * Reached through a function so that the colour literals stay in design.h,
 * which is the rule with no exceptions. */
enum ui_ref_color_id {
    UI_REF_LIT_RAKING = 0, UI_REF_BASE_RAKING,
    UI_REF_KNOCK_PLATE, UI_REF_WASH_RAKING
};
unsigned ui_ref_color(int which);
/* ...and the figures that are not colours: the reference plate, the header
 * height in DESIGN px, and the three quantities no live token can produce. */
enum ui_ref_num_id {
    UI_REFN_PLATE_W = 0, UI_REFN_PLATE_H, UI_REFN_TITLE_H,
    UI_REFN_RAKING_PX, UI_REFN_GRAPHITE_PX,
    UI_REFN_WASH_Q4, UI_REFN_RAKING_Q4
};
int ui_ref_num(int which);

/* ---- the app contract -----------------------------------------------------
 * Three functions, NO LOOP, EVER. If you are writing `while (...)` inside an
 * app you have made a mistake - that `while` is exactly why every demo in this
 * kernel ends with "press any key to exit".
 *
 * app_draw  repaints the CLIENT AREA and must be POSITION-PURE: every
 *           coordinate derived from the x,y,w,h passed in, nothing baked in.
 *           Called only when the window is damaged, not every frame. Also
 *           called with drawing switched off, to hit-test (see ui_begin).
 * app_event returns 1 if it handled the event.
 * app_tick  runs every frame, must be cheap, MUST NOT DRAW. It returns 1 to
 *           say "my state changed, repaint me" - that is how a clock ticks or
 *           a snake moves without owning the frame.
 */
typedef void (*app_draw_fn)(int app, int x, int y, int w, int h, int focused);
typedef int  (*app_event_fn)(int app, int win, int type, int code, int x, int y);
typedef int  (*app_tick_fn)(int app, int win);

/* What wm_repaint draws BEFORE any window: wallpaper, header bar, dock.
 * Desktop furniture is not a window - it is always at the bottom, never
 * overlapped, and never in the z-order. Only the start menu becomes a real
 * window, because it has to appear on top of things. */
typedef void (*desk_draw_fn)(int x, int y, int w, int h);
/* THE OVERLAY LAYER - drawn after every window and after the toast, clipped to
 * the damage rectangle like everything else. It is what a MODAL is made of: the
 * prototype has five surfaces that live above the whole desktop rather than in
 * a window - activities, the command palette, the context and window menus, and
 * the lock sheet - and none of them is a window, because a window would appear
 * in the register, take focus, and be listed by the thing it is drawn over.
 *
 * The rectangle passed is the DAMAGE rect, not the screen: an overlay that
 * ignores it repaints the whole screen for a one-row menu highlight. */
typedef void (*overlay_draw_fn)(int x0, int y0, int x1, int y1);

/* ...and what happens when the pointer is pressed on it. The dock is drawn by
 * desk_draw and is not in the z-order, so wm_at() finds nothing there and the
 * click had nowhere to go. */
typedef void (*desk_click_fn)(int x, int y, int btn);

/* A SYSTEM KEY - one that belongs to the desktop rather than to whichever
 * window happens to have focus. Super is the only one today. Routing it to the
 * focused app instead would mean every app had to know about the start menu. */
/* RETURNS 1 IF THE DESKTOP CONSUMED THE KEY. It was void, and that is half
 * of why Escape never dismissed an overlay: with no way to say "mine", the
 * only safe thing wm.c could do with a desk key was route the one key that
 * is unambiguously the desktop's (Super) and give the rest to the focused
 * window. Escape belongs to whichever is true at the time - the overlay if
 * one is up, the app otherwise - and that is a question only the desktop
 * can answer, so it has to be able to answer it. */
typedef int (*desk_key_fn)(int code, int mods);
/* An app may REFUSE to be closed. Returns 0 to keep the window open. The only
 * caller that matters today is the editor with an unsaved buffer: before this
 * existed, the close box tore the window down and the edits went with it,
 * silently, one frame after the footer had drawn the word UNSAVED. */
typedef int (*can_close_fn)(int win);

/* AN OVERLAY IS MODAL TO THE POINTER, and until this existed it was modal to
 * nothing at all. The only overlay hook wm.c had was a DRAW hook, so every menu
 * row, palette row and activities tile was unclickable and clicks fell through
 * to whatever was painted underneath. Returns non-zero when the overlay
 * consumed the event. */
typedef int (*overlay_click_fn)(int x, int y, int down);

/* ---- wm.c ---------------------------------------------------------------- */
#define WM_MAX 12
#define WM_TABS 4        /* apps sharing one window frame */

#define WF_OPEN     (1 << 0)
#define WF_MODAL    (1 << 1)   /* takes everything; a click outside dismisses */
#define WF_NOCHROME (1 << 2)   /* draws its own frame, e.g. the start menu    */
#define WF_MINIMIZED (1 << 3)  /* remains in taskbar/z-order, paints nothing */

void wm_init(void);
void wm_hooks(app_draw_fn d, app_event_fn e, app_tick_fn t, desk_draw_fn desk);
/* Register the modal layer. Separate from wm_hooks for the reason snap_rect_lr
 * is separate from snap_rect: the existing callers mean "the four layers this
 * desktop had", and widening the signature would edit each of them to pass 0
 * and prove nothing. */
void wm_overlay(overlay_draw_fn f);
/* A RIGHT-PRESS OVER A WINDOW. The left button already has three destinations
 * inside route_mouse - grab, control, app - and none of them is the shell. The
 * window's own menu is a shell surface (it names the register row, and it is
 * drawn on the overlay layer), so it needs its own way out, exactly as the
 * desk's click does. Edge-triggered: holding the button must not reopen the
 * menu on every motion event. */
typedef void (*win_menu_fn)(int win, int x, int y);
void wm_win_menu(win_menu_fn f);
/* A window's retained content, box-filtered into dx,dy,dw,dh. 0 if it has
 * never painted - the caller must not fake a preview for a window with none. */
int  wm_thumb(int win, int dx, int dy, int dw, int dh);
void wm_desk_click(desk_click_fn f);
void wm_desk_key(desk_key_fn f);
void wm_can_close(can_close_fn f);
void wm_overlay_click(overlay_click_fn f);

int  wm_open(int app, const char *title, int x, int y, int w, int h);
void wm_close(int win);
void wm_minimize(int win);
void wm_max_toggle(int win);   /* maximise/restore, the same test the title control uses */
int  wm_over_below(int win);   /* does this plate occlude another - the chrome's own test */
int  wm_is_minimized(int win);
void wm_raise(int win);
void wm_focus(int win);
void wm_set_modal(int win, int on);
int  wm_add_tab(int win, int app, const char *title);
void wm_set_tab(int win, int tab);
/* ---- what the title bar and the foot band read out -------------------------
 * PRESSWORK's header is "01 TERMINAL  zlsh" and its foot band is
 * "01  tty1 - 80x24   APP US 995 us                            ws 01". wm.c
 * derives the module code, the app cost and the workspace itself; the register
 * slot, the mono qualifier and the status line are POLICY and arrive here.
 *
 * None of the three has a default. A window never told stays blank in those
 * cells rather than being given the app id or the title as a stand-in - the
 * readout's whole value is that every figure in it is true.
 *
 *   reg    1..99, the shell's REGISTER rail slot; anything else clears it
 *   sub    up to 15 chars, mono, after the title
 *   status up to 23 chars, mono, the band's left readout
 */
void wm_set_label(int win, int reg, const char *sub);
void wm_set_status(int win, const char *status);
/* WHERE THE DESK'S FIELD IS, so the module code is read off the same grid the
 * shell rules onto it. The grid's SHAPE is design.h's (ZD_GRID_COLS/ROWS/
 * MARGIN/GUTTER); only its origin and extent are shell furniture. Until this
 * is called wm.c derives the field from theme rail_w/strip_h/foot_h, which is
 * the same grid to within the shell's own 1dp hairlines. w or h <= 0 returns
 * to that derivation. */
void wm_set_field(int x, int y, int w, int h);
int  wm_tab(int win);
int  wm_ntabs(int win);
void wm_move(int win, int x, int y);
/* wm_resize had no caller at all until the resize grip; see wm.c. */
void wm_resize(int win, int w, int h);
/* App content changed. This is deliberately distinct from screen damage:
 * moving/raising a valid retained client must not redraw the app. */
void wm_invalidate_client(int win);
unsigned long wm_client_surface_bytes(void);
unsigned int wm_client_surface_refusals(void);
unsigned int wm_retained_shell_builds(void);
unsigned int wm_region_fallbacks(void);
unsigned long long wm_region_occluded_pixels(void);
int wm_region_fragmentation_probe(void);

/* ---- the animation timeline -----------------------------------------------
 * A fixed array of running animations, ticked once per frame by wm_frame().
 * Kinds are integer step tables, not easing curves - see wm.c for why that is
 * a decision rather than a shortcut. An animation NEVER changes what exists:
 * it draws, and the caller decides lifetime. */
#define ANIM_NONE   0
#define ANIM_OPEN   1   /* scale 82 -> 100, the window open        */
#define ANIM_CLOSE  2   /* scale 100 -> 70, its mirror             */
#define ANIM_PRESS  3   /* scale 100 -> 96 -> 100, zpress          */
#define ANIM_PULSE  4   /* opacity 0 -> 40 -> 0, zpulse            */
#define ANIM_FADE   5   /* opacity up, zov / zpop / ztoast         */

int  wm_anim(int win, int kind);      /* 0 = refused, every slot busy */
int  wm_anim_running(int win);        /* the kind, or 0               */
int  wm_anim_alpha(int win);          /* 0..255, 255 when settled     */

/* ---- animating something that is NOT a window -----------------------------
 * A dock tile, the dot under it, a toast, the wallpaper. Same slots, same
 * curves, same durations; the caller supplies the rectangle to repaint,
 * because wm.c does not know where the dock is. The id must be negative or it
 * collides with a window index and wm_anim_at refuses. */
#define WM_FX_TOAST  (-1)     /* wm.c's own: ztoast                          */
#define WM_FX_GHOST  (-2)     /* wm.c's own: ANIM_CLOSE, the closing window  */
#define WM_FX_USER   (-16)    /* the policy layer's, -16 and downward        */

int  wm_anim_at(int id, int kind, int x, int y, int w, int h);
int  wm_anim_scale(int id);           /* thousandths; 1000 when settled      */
int  wm_anim_progress(int id, int kind);  /* eased 0..1000, or -1 if not run */

/* zpulse and zsweep are `infinite` in the reference and so hold no slot at
 * all - an entry that never ends would take a quarter of the timeline for the
 * life of the boot. This is a pure function of the clock: opacity 0..255,
 * never below the reference's .55 floor, and 255 when animations are off. */
int  wm_pulse(int period_ms);

/* zsweep, the wallpaper band. OFF until asked - the reference keeps it behind
 * its CRT toggle too - and gated by wm_set_anim on top of that. */
void wm_set_sweep(int on);
int  wm_sweep_enabled(void);
int  wm_sweep_y(void);            /* the band's quantised top edge, or 0 off */

/* Close it AND shrink a ghost of it away - the gesture form of wm_close.
 * wm_close() stays instantaneous, because teardown loops and policy use it. */
void wm_close_fx(int win);

void wm_damage(int x, int y, int w, int h);   /* mark a screen region dirty  */
void wm_damage_win(int win);                  /* ...or a whole window        */
void wm_frame(void);                          /* one pass of the frame loop  */
void wm_repaint(void);

int  wm_focused(void);
int  wm_count(void);
int  wm_is_open(int win);
void wm_geometry(int win, int *x, int *y, int *w, int *h);
void wm_client(int win, int *x, int *y, int *w, int *h);
int  wm_at(int x, int y);                     /* topmost window containing    */
int  wm_zorder_at(int i);                     /* i-th from the BACK           */
void wm_stop(void);                           /* 'q': ask the loop to end     */
int  wm_running(void);
/* what the last REPAINTING frame cost, in microseconds, and the worst so far */
int  wm_frame_us(void);
int  wm_win_us(int win);
int  wm_peak_us(void);
void wm_peak_reset(void);
int  wm_win_app(int win);                      /* the ACTIVE tab.s app        */
/* ---- workspaces -------------------------------------------------------------
 * A window is on exactly one, numbered from 1. The z-order is GLOBAL and a
 * switch never touches it, so a workspace you come back to looks exactly as
 * you left it. wm_set_ws / wm_set_win_ws return 0 when they refuse (a number
 * below 1, a window that is not open, or a move that changes nothing).
 * HOW MANY workspaces exist is policy and lives in kernel.zl. */
int  wm_ws(void);                              /* the one being looked at     */
int  wm_set_ws(int n);
int  wm_win_ws(int win);                       /* 0 if the window is not open */
int  wm_set_win_ws(int win, int n);
int  wm_ws_count(void);                        /* defaults to 1 until told    */
int  wm_set_ws_n(int n);
void wm_set_anim(int on);                     /* window open animation on/off */
int  wm_anim_enabled(void);

/* ---- the pointer -----------------------------------------------------------
 * The cursor SHAPES, published so the compositor can ask for one. fb.c static-
 * asserts these against cursor.inc's own CUR_* so the two cannot drift; a
 * mismatch would silently show the wrong picture, which is the kind of thing
 * nothing tests. */
/* A double-click arrives as an ordinary EV_MOUSE press with this bit set in the
 * button mask, rather than as a new event type. The PS/2 protocol uses bits
 * 0..2 for left/right/middle, so bit 8 is free, and an app that does not care
 * about double-clicks keeps working unchanged - it masks for button 1 and never
 * sees this. A new event type would have made every existing app_event handler
 * wrong by omission instead. */
#define MOUSE_DOUBLE  (1 << 8)

#define CURSOR_ARROW   0
#define CURSOR_IBEAM   1
#define CURSOR_RESIZE  2
#define CURSOR_BUSY    3
void fb_cursor_set(int kind);
int  fb_cursor_get(void);

/* ---- settings.c -------------------------------------------------------------
 * The Settings app. Signatures match app_draw_fn / app_event_fn above, so it
 * can be handed to wm_hooks or dispatched to from wmglue.c. */
void settings_draw(int app, int x, int y, int w, int h, int focused);
int  settings_event(int app, int win, int type, int code, int x, int y);
void settings_apply(void);       /* push every setting to its sink */
int  settings_accent(void);
int  settings_scale(void);
int  settings_speed(void);
int  settings_accel(void);
int  settings_subpixel(void);
int  settings_anim(void);

/* ---- ui.c ---------------------------------------------------------------- */
#define UI_DRAW     0
#define UI_HITTEST  1

void ui_begin(int x, int y, int w, int h, int mode, int px, int py, int click);
int  ui_fired(void);            /* did any widget fire this pass, and which  */
void ui_label(const char *s);
void ui_label_dim(const char *s);
void ui_bar(int pct);
int  ui_button(const char *s);
void ui_sep(void);
void ui_space(int n);
int  ui_toggle(const char *s, int *on);
int  ui_slider(int *v, int lo, int hi);
int  ui_toggle_value(const char *s, int on);       /* scalar bridge for zl */
int  ui_slider_value(int v, int lo, int hi);       /* scalar bridge for zl */
void ui_num(const char *s, int v);
int  ui_list_row(const char *s, int selected);
void ui_scroll_begin(int h, int *off);
void ui_scroll_end(int *off);
int  ui_scroll_content(void);
void ui_scroll_begin_value(int h, int off);        /* one sequential zl pass */
int  ui_scroll_end_value(void);
/* ---- keyboard focus --------------------------------------------------------
 * Which control the keyboard is on. The INDEX lives in ui.c because it has to
 * survive between the hit-test pass and the draw pass; choosing it is still the
 * app's job, like every other piece of widget state. -1 is "nothing focused",
 * and it is the default - a desktop that boots with a ring on some arbitrary
 * control looks broken. */
void ui_set_focus(int idx);
int  ui_focus_get(void);
int  ui_widget_count(void);     /* fired-capable widgets in the last pass    */
void ui_activate_focus(void);   /* one-shot: the focused widget fires        */
void ui_end_activate(void);     /* ...consumed after the app re-runs its UI  */

void ui_row(void);              /* put the next widget beside this one       */
void ui_endrow(void);

/* The NAVIGATION row's selection treatment - the register mark. See ui.c for
 * why there are two row idioms and which rows get which. `zebra` is accepted
 * and ignored; PRESSWORK rules rows with a groove, not a stripe. */
void ui_row_select(int x, int y, int w, int h, int selected, int zebra);

/* ---- PRESSWORK's depth recipe, shared -------------------------------------
 * ONE raking light from the upper left that never moves, written once. Every
 * raised or sunken object in the system is these calls and nothing else, so
 * the direction the light comes from is a fact of the toolkit rather than a
 * convention each widget re-implements. See ui.c for the full derivation.
 *
 *   ui_seat_face    the ring plus the object
 *   ui_run_top      the 1px STRUCK run, inside the ring, stopping at the arcs
 *   ui_run_bottom   the 1px cut/grazed run on the far edge
 *   ui_seat_raised  face + ring + lit top run; `lit` 0 == disabled
 *   ui_seat_sunken  the same recipe with the sign flipped - the `.well` pit
 *
 * `ring` is ZD_CUT at rest and ZD_EDGE_OVER under the pointer. Radius follows
 * the design's rule that it encodes how much the object can move: ZD_R_BOLT
 * for anything bolted down, ZD_R_CHIP for a control, ZD_R_INSET for a pane,
 * ZD_R_PLATE for a window. */
void ui_seat_face(int x, int y, int w, int h, int r, unsigned face, unsigned ring);
void ui_run_top(int x, int y, int w, int r, unsigned edge);
void ui_run_bottom(int x, int y, int w, int h, int r, unsigned edge);
void ui_seat_raised(int x, int y, int w, int h, int r, unsigned face,
                    unsigned ring, int lit);
void ui_seat_sunken(int x, int y, int w, int h, int r, unsigned face);

/* INK ON THE ACCENT, computed the way the reference computes it - a WCAG
 * relative luminance and one threshold, no floating point. Any widget painting
 * text on a saturated fill must route through this instead of naming a colour;
 * three widgets in the reference itself hard-code #fff and fail contrast. */
unsigned ui_ink_on(unsigned bg);
unsigned ui_luminance_q16(unsigned rgb);   /* 0..65535; exposed for the gate */

/* ---- uikit.c - the shared widget catalogue ---------------------------------
 * Everything the 53 apps draw with. Two conventions run through all of it:
 *
 * 1. RECT IN, FIRED OUT. These take explicit device-pixel geometry rather than
 *    flowing from ui.c's layout cursor, because an app draws into a client
 *    rectangle it was handed and every reference widget is positioned against
 *    an edge of it. A `*_w()` / `*_h()` measuring function exists wherever the
 *    natural size is not obvious, so a row can be laid out before it is drawn.
 *    Nothing here allocates, and nothing here holds widget state.
 *
 * 2. NO POINTERS IN THE zl-FACING FORM. zl has no address-of, so anything
 *    stateful comes in two shapes, exactly as ui_toggle/ui_toggle_value do:
 *    the C form returns "which index was clicked, or -1", and the `_value`
 *    form takes the current selection by value and returns the new one.
 *
 * Multi-item widgets (segmented control, tab strips, menus) take their items
 * as ONE '|'-separated string - "CPU|Memory|Disk" - because an array of char*
 * cannot cross the zl boundary and a callback cannot either. Empty string is
 * zero items; a trailing '|' is an empty item, not a terminator.
 */

/* THE TYPE SCALE. These used to BE fb.c's text roles and are now design.h's
 * ZD_T_SM / ZD_T_MD / ZD_T_LG - 11 / 13 / 21 design px. The numbering is
 * unchanged so no call site moves, but the resolution is not fb.c's any more:
 * asking fb.c for a role gets 12 / 12 / 16 on a 1920-wide panel, because its
 * role ladder is floored at 12 and the caption's base is 8. CAPTION and BODY
 * came out the same size. See the long note over ui_text_h in uikit.c for the
 * measurement and for what the change costs. */
#define UI_SM 0
#define UI_MD 1
#define UI_LG 2

/* flags, OR-ed */
#define UI_F_MONO (1 << 0)   /* Roboto Mono in the reference: numbers, paths */
#define UI_F_BOLD (1 << 1)   /* the reference's fontWeight:700              */
#define UI_F_CAPS (1 << 2)   /* text-transform: uppercase. ASCII only, which */
                             /* is the whole of what the three atlases carry */

/* button kinds */
#define UI_BTN_NEUTRAL 0     /* rgba(255,255,255,.07) / body text           */
#define UI_BTN_PRIMARY 1     /* accent fill + COMPUTED ink                  */
#define UI_BTN_DANGER  2     /* BAD at 16% + 40% border + soft red text     */
#define UI_BTN_GHOST   3     /* no fill until active                        */

/* cell alignment */
#define UI_ALIGN_L 0
#define UI_ALIGN_C 1
#define UI_ALIGN_R 2

/* monospace panel grounds - docs/reference/ui/widgets.md S14 */
#define UI_PANEL_TERM 0      /* #0b0d0f, 12px/1.5   terminal + editor       */
#define UI_PANEL_LOG  1      /* #0b0d0f, 11px/1.55  kernel log              */
#define UI_PANEL_HEX  2      /* #090a0c, 11.5px/1.5 hex + syscall trace     */

/* ---- text ---------------------------------------------------------------- */
int  ui_text_w(const char *s, int size, int flags);
int  ui_text_h(int size);
void ui_text(int x, int y, const char *s, unsigned rgb, int size, int flags);

/* ---- TRACKED TEXT, which is PRESSWORK's fourth type style ------------------
 * Not a fourth SIZE - there are three baked atlases and no rasteriser, so a
 * fourth size does not exist to be asked for. It is SM, uppercase, bold and
 * LETTER-SPACED, and design.h has carried ZD_TR_LAB (1.4 design px) and
 * ZD_TR_BIG (2.6) since PRESSWORK landed. ZD_TR_BIG had NO reader in the tree
 * at all; ZD_TR_LAB had one, inside uikit.c, reachable by three widgets.
 *
 * fb_text_role draws a whole string in one call and has no track argument -
 * and no flag word to put one in, which is the shape design.h predicts. It is
 * not fb.c's job: tracking is one draw per glyph with the pen advanced by the
 * glyph plus the track, and that is this layer. fb.c is not edited.
 *
 * THE MEASURE AND THE DRAW SHARE ONE LOOP, which design.h states in the same
 * breath as the token: a label measured without the track and drawn with it
 * clips at its right edge. `_w` and the draw are the same function with the
 * ink switched off. The trailing track is cancelled - the prototype's own
 * `margin-right: calc(-1 * var(--tr-lab))` - so a width is the glyphs plus
 * (n-1) tracks, never n.
 *
 * `track_x10` is a design.h tracking token, i.e. design px times ten. 0 means
 * untracked, which makes this pair the single text engine: the zl shell's
 * `label` builtin is this with a track of 0.
 *
 * ui_caps and ui_display bake in one token each, so no call site spells a
 * track and design.h stays the only place the number lives. */
int  ui_text_tracked_w(const char *s, int size, int flags, int track_x10);
void ui_text_tracked(int x, int y, const char *s, unsigned rgb,
                     int size, int flags, int track_x10);
int  ui_caps_w(const char *s, int size);      /* th / .t-lab / .sect / .kv .k */
void ui_caps(int x, int y, const char *s, unsigned rgb, int size);
int  ui_display_w(const char *s, int size);   /* .t-big - one reading a view  */
void ui_display(int x, int y, const char *s, unsigned rgb, int size);

/* ---- buttons - docs/reference/ui/widgets.md S13 -------------------------- */
int  ui_pill_w(const char *s, int size, int flags);
int  ui_pill_h(int size);
int  ui_pill(int x, int y, int w, int h, const char *s,
             int size, int kind, int flags);
int  ui_button_sz(const char *s, int size, int kind, int flags);  /* cursor  */
int  ui_icon_button(int x, int y, int px, const char *glyph, int active);

/* ---- segmented control - S3 ----------------------------------------------- */
int  ui_seg_h(int size);
int  ui_seg_w(const char *items, int size);
int  ui_segmented(int x, int y, int w, int h, const char *items,
                  int sel, int size);                /* clicked index, or -1 */
int  ui_segmented_value(int x, int y, int w, int h, const char *items,
                        int sel, int size);          /* the new selection    */

/* ---- tab strips - S4.1 closeable, S4.2 underline -------------------------- */
int  ui_tabs_h(void);
int  ui_tabstrip(int x, int y, int w, const char *items, int sel);
int  ui_tabstrip_closed(void);      /* whose x was hit in the last strip, -1 */
int  ui_tabstrip_value(int x, int y, int w, const char *items, int sel);
int  ui_utabs_h(void);
int  ui_utabs(int x, int y, int w, const char *items, int sel);
int  ui_utabs_value(int x, int y, int w, const char *items, int sel);

/* ---- toolbar and status bar - S5, S6 -------------------------------------- */
int  ui_toolbar_h(void);
void ui_toolbar(int x, int y, int w, int h, int at_bottom);
int  ui_status_h(void);
void ui_statusbar(int x, int y, int w, int h);

/* ---- column header + list row - S7 ----------------------------------------
 * Tracks are the reference's own grid strings, transliterated: a number is a
 * fixed track in design px, '*' is the `minmax(0,1fr)` that takes the rest.
 * The three verbatim ones are below so an app does not retype them. */
#define UI_GRID_PROC  "48,52,28,48,52,*"      /* PGRID,  ds-reference 3055   */
#define UI_GRID_FILES "*,58,84,78,62"         /* FGRID,  3057                */
#define UI_GRID_ARCH  "*,66,62,62,54,58"      /* archive, 4160               */

void ui_grid(const char *tracks);
int  ui_grid_cols(void);
void ui_grid_span(int x, int w, int col, int *cx, int *cw);
int  ui_colhead_h(void);
int  ui_colhead(int x, int y, int w, const char *labels, int sortcol, int sortdir);
int  ui_grid_row_h(void);
int  ui_grid_row(int x, int y, int w, int idx, int selected);
void ui_grid_cell(int x, int w, int y, int h, int col, const char *s,
                  int align, unsigned rgb, int size, int flags);

/* ---- stat card strip - S8 -------------------------------------------------
 * `repeat(auto-fit, minmax(minw, 1fr))`: as many columns as fit at >= minw,
 * cells wrapping into as many rows as they need. Pass 0 for the reference's
 * own 88px. ui_stat_end returns the height the strip turned out to be. */
void ui_stat_begin(int x, int y, int w, int minw);
void ui_stat_cell(const char *key, const char *val, unsigned val_rgb);
int  ui_stat_end(void);

/* ---- sidebar - S9 --------------------------------------------------------- */
int  ui_sidebar_w(void);
void ui_sidebar(int x, int y, int w, int h);
int  ui_heading_h(void);
void ui_heading(int x, int y, int w, const char *s);
int  ui_nav_h(void);
int  ui_nav_row(int x, int y, int w, const char *s, int selected);

/* ---- bars - S10 ----------------------------------------------------------- */
int  ui_meter_h(void);
void ui_meter(int x, int y, int w, int pct, unsigned rgb);
int  ui_progress_h(void);
void ui_progress(int x, int y, int w, int pct, unsigned rgb);
int  ui_minibar_h(void);
void ui_minibar(int x, int y, int w, int pct, unsigned rgb);
int  ui_segbar_h(void);
void ui_segbar_begin(int x, int y, int w, int total);
void ui_segbar_item(int amount, unsigned rgb);
void ui_segbar_end(void);

/* ---- monospace panel - S14 ------------------------------------------------ */
void ui_mono_panel(int x, int y, int w, int h, int kind);
int  ui_mono_line_h(int kind);
void ui_mono_line(int x, int y, int w, const char *s, unsigned rgb, int kind,
                  int highlight);

/* ---- cards and key/value - S15 -------------------------------------------- */
void ui_card(int x, int y, int w, int h);
int  ui_card_head_h(void);
void ui_card_head(int x, int y, int w, const char *title,
                  const char *badge, unsigned badge_rgb);
int  ui_kv_h(void);
void ui_kv(int x, int y, int w, const char *k, const char *v,
           unsigned v_rgb, int first);

/* ---- overlays - S16 -------------------------------------------------------
 * ONE menu widget. The reference's window menu and context menu differ only in
 * width (224/218), shadow, animation and item gap (16/14) - S20.8 - so they
 * are collapsed here at the midpoint. */
void ui_popover(int x, int y, int w, int h);
int  ui_menu_w(const char *items);
int  ui_menu_h(const char *items);
int  ui_menu(int x, int y, const char *items, int sel);
int  ui_modal_head_h(void);
void ui_modal(int x, int y, int w, int h, const char *title);
int  ui_toast_h(void);
void ui_toast_draw(int x, int y, int w, const char *title, const char *body,
                   unsigned kind_rgb);

/* ---- chart - S17 ----------------------------------------------------------
 * The reference's CPU sparkline multiplies by 1.9 into a viewBox that cannot
 * hold it, so everything above 52.63% renders flat against the top (S20.18).
 * This does NOT reproduce that: the gain is 1 and the value is clamped. */
void ui_spark_begin(int x, int y, int w, int h);
void ui_spark_point(int pct);
void ui_spark_end(void);

/* ---- indicators and inputs - S18 ------------------------------------------ */
int  ui_dot_size(void);
void ui_dot(int x, int y, unsigned rgb, int glow);
int  ui_badge_w(const char *s);
int  ui_badge_h(void);
void ui_badge(int x, int y, const char *s, unsigned rgb);
int  ui_input_h(void);
int  ui_input(int x, int y, int w, const char *text, const char *placeholder,
              int focused);
int  ui_search_h(void);
int  ui_search(int x, int y, int w, const char *text, const char *placeholder);
int  ui_chip_w(const char *s);
int  ui_chip_h(void);
int  ui_chip(int x, int y, const char *s, int active);

/* ---- item lists ----------------------------------------------------------- */
int  ui_items_count(const char *items);

/* ---- ui.c internals, published for uikit.c only ----------------------------
 * "Did it fire" is decided in exactly ONE place (ui.c's fire()), and the only
 * way for a second file to hold widgets is for that funnel to be reachable.
 * These are that funnel and nothing more: no access to the layout cursor, no
 * way to set L.fired directly, no second focus index. */
int  ui_mode_get(void);
int  ui_click_get(void);
int  ui_ptr_x(void);
int  ui_ptr_y(void);
int  ui_hit(int x, int y, int w, int h);
int  ui_fire(int x, int y, int w, int h);
void ui_place(int w, int h, int *x, int *y);  /* the layout cursor, one step */
void ui_ring(int x, int y, int w, int h);

#endif
