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
     * reference says. An app slice reported collapsing ZD_TEXT_5, ZD_TEXT_6
     * and ZD_TEXT_2 onto two roles, ZD_WARN onto the accent, ZD_SURF_WELL onto
     * the background and ZD_SURF_7 onto the border - six tokens rendering as
     * three colours. A palette with no name for "warning" spends the accent on
     * it, and then the accent no longer means "this is the one thing to look
     * at". These are appended, so no existing ui_color() index moves. */
    unsigned text_2;      /* ZD_TEXT_2   #c4c9cf                 */
    unsigned text_5;      /* ZD_TEXT_5   #74797f  labels, heads  */
    unsigned text_6;      /* ZD_TEXT_6   #5c6167  hints, units   */
    unsigned warn;        /* ZD_WARN     #f5b93c  NOT the accent */
    unsigned surf_1;      /* ZD_SURF_1   #090a0c  sunken wells   */
    unsigned surf_5;      /* ZD_SURF_5   #1c2024  menus, modals  */
    unsigned surf_7;      /* ZD_SURF_7   #474b50  dimmest ink    */
    unsigned surf_well;   /* ZD_SURF_WELL #0d0f12               */
    unsigned accent_br;   /* ZD_ACCENT_BR #cdf25a live values    */
    /* Direct C-only role kept after the zl-visible contiguous colour array,
     * so adding a second gradient stop cannot shift every ui_color() index. */
    unsigned title_off_bot;

    /* metrics, all already multiplied by the UI scale */
    int pad;              /* inside a panel, edge to content    */
    int gap;              /* between two widgets                */
    int row_h;            /* one line of controls               */
    int radius;           /* the OUTER corner. Inner is this - 1 */
    int title_h;          /* window title bar                   */
    int scale;            /* ui(): 1 at 8px cells, 2 at 16px    */
    int scale_q8;         /* continuous scale, 256 == 1 design unit */
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
    UI_COLOR_COUNT
};
enum ui_metric_role {
    UI_METRIC_PAD = 0, UI_METRIC_GAP, UI_METRIC_ROW_H, UI_METRIC_RADIUS,
    UI_METRIC_TITLE_H, UI_METRIC_SCALE_Q8, UI_METRIC_COUNT
};

const struct ui_theme *ui_theme(void);
void ui_theme_init(int scale);          /* build the default theme at a scale */
void ui_theme_init_q8(int scale_q8);    /* continuous scale; 256 == 1x */
void ui_theme_set(const struct ui_theme *t);
unsigned ui_color(int role);            /* shared C/zl source of colour truth */
int ui_metric(int role);

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

/* ...and what happens when the pointer is pressed on it. The dock is drawn by
 * desk_draw and is not in the z-order, so wm_at() finds nothing there and the
 * click had nowhere to go. */
typedef void (*desk_click_fn)(int x, int y, int btn);

/* A SYSTEM KEY - one that belongs to the desktop rather than to whichever
 * window happens to have focus. Super is the only one today. Routing it to the
 * focused app instead would mean every app had to know about the start menu. */
typedef void (*desk_key_fn)(int code, int mods);

/* ---- wm.c ---------------------------------------------------------------- */
#define WM_MAX 12
#define WM_TABS 4        /* apps sharing one window frame */

#define WF_OPEN     (1 << 0)
#define WF_MODAL    (1 << 1)   /* takes everything; a click outside dismisses */
#define WF_NOCHROME (1 << 2)   /* draws its own frame, e.g. the start menu    */
#define WF_MINIMIZED (1 << 3)  /* remains in taskbar/z-order, paints nothing */

void wm_init(void);
void wm_hooks(app_draw_fn d, app_event_fn e, app_tick_fn t, desk_draw_fn desk);
void wm_desk_click(desk_click_fn f);
void wm_desk_key(desk_key_fn f);

int  wm_open(int app, const char *title, int x, int y, int w, int h);
void wm_close(int win);
void wm_minimize(int win);
int  wm_is_minimized(int win);
void wm_raise(int win);
void wm_focus(int win);
void wm_set_modal(int win, int on);
int  wm_add_tab(int win, int app, const char *title);
void wm_set_tab(int win, int tab);
int  wm_tab(int win);
int  wm_ntabs(int win);
void wm_move(int win, int x, int y);
/* wm_resize had no caller at all until the resize grip; see wm.c. */
void wm_resize(int win, int w, int h);

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

/* The selection treatment, picked once for the whole toolkit. See ui.c for
 * WHICH of the reference's three it is and why. `zebra` stripes odd rows. */
void ui_row_select(int x, int y, int w, int h, int selected, int zebra);

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

/* sizes - these ARE fb.c's text roles, so a widget's size and its type size
 * cannot drift apart */
#define UI_SM 0
#define UI_MD 1
#define UI_LG 2

/* flags, OR-ed */
#define UI_F_MONO (1 << 0)   /* Roboto Mono in the reference: numbers, paths */
#define UI_F_BOLD (1 << 1)   /* the reference's fontWeight:700              */

/* button kinds */
#define UI_BTN_NEUTRAL 0     /* rgba(255,255,255,.07) / body text           */
#define UI_BTN_PRIMARY 1     /* accent fill + COMPUTED ink                  */
#define UI_BTN_DANGER  2     /* BAD at 16% + 40% border + soft red text     */
#define UI_BTN_GHOST   3     /* no fill until active                        */

/* cell alignment */
#define UI_ALIGN_L 0
#define UI_ALIGN_C 1
#define UI_ALIGN_R 2

/* monospace panel grounds - reference-widgets.md S14 */
#define UI_PANEL_TERM 0      /* #0b0d0f, 12px/1.5   terminal + editor       */
#define UI_PANEL_LOG  1      /* #0b0d0f, 11px/1.55  kernel log              */
#define UI_PANEL_HEX  2      /* #090a0c, 11.5px/1.5 hex + syscall trace     */

/* ---- text ---------------------------------------------------------------- */
int  ui_text_w(const char *s, int size, int flags);
int  ui_text_h(int size);
void ui_text(int x, int y, const char *s, unsigned rgb, int size, int flags);

/* ---- buttons - reference-widgets.md S13 ---------------------------------- */
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
