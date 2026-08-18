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

    /* metrics, all already multiplied by the UI scale */
    int pad;              /* inside a panel, edge to content    */
    int gap;              /* between two widgets                */
    int row_h;            /* one line of controls               */
    int radius;           /* the OUTER corner. Inner is this - 1 */
    int title_h;          /* window title bar                   */
    int scale;            /* ui(): 1 at 8px cells, 2 at 16px    */
};

/* The scale everything snaps to. Use these, never a literal. */
#define UI_S1(t)  (4  * (t)->scale)
#define UI_S2(t)  (8  * (t)->scale)
#define UI_S3(t)  (12 * (t)->scale)
#define UI_S4(t)  (16 * (t)->scale)
#define UI_S6(t)  (24 * (t)->scale)

const struct ui_theme *ui_theme(void);
void ui_theme_init(int scale);          /* build the default theme at a scale */
void ui_theme_set(const struct ui_theme *t);

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

void wm_init(void);
void wm_hooks(app_draw_fn d, app_event_fn e, app_tick_fn t, desk_draw_fn desk);
void wm_desk_click(desk_click_fn f);
void wm_desk_key(desk_key_fn f);

int  wm_open(int app, const char *title, int x, int y, int w, int h);
void wm_close(int win);
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
void ui_num(const char *s, int v);
int  ui_list_row(const char *s, int selected);
void ui_scroll_begin(int h, int *off);
void ui_scroll_end(int *off);
int  ui_scroll_content(void);
void ui_row(void);              /* put the next widget beside this one       */
void ui_endrow(void);

#endif
