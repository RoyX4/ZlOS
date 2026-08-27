/* wmglue.c - the seam between the compositor (C) and the apps (zl).
 *
 * wm.c is mechanism and knows nothing about what a window contains. kernel.zl
 * is policy and says what each app draws. Something has to carry a call across
 * that line, and this is it - deliberately the ONLY place that does, so the
 * layering in ui.h stays a rule rather than a suggestion.
 *
 * IT WORKS BECAUSE ZL COMPILES TO C. A zl function `fn app_draw(id, x, y, w,
 * h, focus)` becomes a C function `Value zl_fn_app_draw(Value, Value, Value,
 * Value, Value, Value)` in the generated _gen.c. So calling zl from C is an
 * ordinary call - no interpreter, no dispatch table, no marshalling beyond
 * boxing the numbers. That fact is what made the whole layer split possible;
 * without it the app callbacks would have had to live in C.
 *
 * THE SYMBOLS ARE WEAK, AND THAT IS THE POINT. kernel.zl does not define the
 * app_* functions yet - the shell is still the top of the system - and this
 * file has to link anyway, today, in a kernel that boots. A weak reference is
 * NULL when nothing defines it, so wm_bind_zl() simply declines and the
 * machine behaves exactly as it does now. The day kernel.zl grows those three
 * functions, the same binary wiring starts working with no change here.
 *
 * That also means this file is a CONTRACT, published in advance. The exact zl
 * signatures it expects are in the comment on each shim, and getting one wrong
 * shows up as a link error rather than as a mystery at run time.
 */

#include "../../../../src/runtime/runtime.h"
#include "ui.h"

/* ---- what kernel.zl must define, eventually --------------------------------
 * Weak, so their absence is a NULL pointer rather than a link failure. Keep
 * the arities exactly in step with the zl side: the compiler generates one C
 * parameter per zl parameter, and a mismatch is undefined behaviour rather
 * than a diagnostic.
 */

/* fn app_draw(app, x, y, w, h, focused) */
extern Value zl_fn_app_draw(Value, Value, Value, Value, Value, Value)
    __attribute__((weak));

/* fn app_event(app, win, type, code, x, y) -> 1 if handled */
extern Value zl_fn_app_event(Value, Value, Value, Value, Value, Value)
    __attribute__((weak));

/* fn app_tick(app, win) -> 1 if it damaged itself */
extern Value zl_fn_app_tick(Value, Value) __attribute__((weak));

/* fn desk_draw(x, y, w, h) - wallpaper, header bar, dock */
extern Value zl_fn_desk_draw(Value, Value, Value, Value) __attribute__((weak));

/* fn desk_click(x, y, btn) - a pointer event on the dock, the start button or
 * the tray. Every event, not just presses: a dock with no hover state reads as
 * a picture of a dock. */
extern Value zl_fn_desk_click(Value, Value, Value) __attribute__((weak));

/* fn desk_key(code, mods) - a system key: Super, today */
extern Value zl_fn_desk_key(Value, Value) __attribute__((weak));
/* ---- apps that live in C --------------------------------------------------
 * THERE ARE NONE LEFT, AND THIS BLOCK IS THE RECORD OF WHY.
 *
 * Settings was written in C and dispatched from here, for a reason that was
 * not preference: settings.c's own header states it as "zl exposes NO natives
 * for ui_* at all, so a zl Settings app would need ~15 new builtins in
 * runtime_kernel.c first", and it added that moving it to zl afterwards would
 * be "a translation, not a redesign, because the widget calls map one to one".
 *
 * THE BLOCKER IS GONE. runtime_kernel.c answers for ui_color, ui_metric,
 * ui_begin and the whole ui_* widget set today, plus the contrast engine the
 * new pane needs (ui_ratio / ui_ceil_dn / ui_ceil_up) and its two live
 * controls (ui_knock / ui_fbar). kernel.zl's app_draw now carries the
 * APP_SETTINGS branch like every other app, so the two lines that used to
 * intercept it here are deleted rather than commented out - an intercept that
 * still ran would silently win over the zl branch and the zl code would be
 * dead with no diagnostic, which is this file's whole hazard class.
 *
 * IT WAS NOT A TRANSLATION IN THE END. What the pane has to SAY changed
 * completely - it is the design arguing for itself against live tokens, not a
 * preferences dialog - so settings.c is superseded rather than ported, and the
 * preference sinks it drove (accent, UI scale, pointer speed and acceleration,
 * subpixel text, window animation) have NO caller now. That is a real loss and
 * it is named here rather than left for someone to discover: those sinks still
 * exist, settings.c still compiles and hosttest/settingstest still passes, but
 * nothing on screen reaches them until they are re-homed - the PRESS tab is
 * the shaped place for them, and it is the prototype's own home for exactly
 * that kind of switch.
 *
 * The one rule this file states is unchanged and is why the deletion is small:
 * this is "deliberately the ONLY place" that crosses between the compositor
 * and an app, so removing an app from C means removing it from HERE and
 * nowhere else. */

/* ---- the shims ------------------------------------------------------------ */
static void glue_draw(int app, int x, int y, int w, int h, int focused)
{
    if (!zl_fn_app_draw) return;
    zl_fn_app_draw(zl_num(app), zl_num(x), zl_num(y),
                   zl_num(w), zl_num(h), zl_num(focused));
}

static int glue_event(int app, int win, int type, int code, int x, int y)
{
    if (!zl_fn_app_event) return 0;
    Value r = zl_fn_app_event(zl_num(app), zl_num(win), zl_num(type),
                              zl_num(code), zl_num(x), zl_num(y));
    return r.type == V_NUM && r.num != 0.0;
}

static int glue_tick(int app, int win)
{
    if (!zl_fn_app_tick) return 0;
    Value r = zl_fn_app_tick(zl_num(app), zl_num(win));
    return r.type == V_NUM && r.num != 0.0;
}

static void glue_desk_click(int x, int y, int btn)
{
    if (!zl_fn_desk_click) return;
    zl_fn_desk_click(zl_num(x), zl_num(y), zl_num(btn));
}

static void glue_desk_key(int code, int mods)
{
    if (!zl_fn_desk_key) return;
    zl_fn_desk_key(zl_num(code), zl_num(mods));
}

static void glue_desk(int x, int y, int w, int h)
{
    if (!zl_fn_desk_draw) return;
    zl_fn_desk_draw(zl_num(x), zl_num(y), zl_num(w), zl_num(h));
}

/* ---- binding ---------------------------------------------------------------
 * Returns 1 if the compositor now has apps to call, 0 if kernel.zl has not
 * grown them yet. A caller that ignores the answer and starts the frame loop
 * anyway gets a desktop of empty window frames, so it is worth checking.
 */
int wm_bind_zl(void)
{
    if (!zl_fn_app_draw) return 0;       /* nothing to composite yet */
    /* glue_event still goes in UNCONDITIONALLY, and the reason has changed
     * rather than gone away. It used to be "it dispatches Settings before it
     * looks at zl"; Settings is a zl app now and that clause is deleted with
     * it. What remains is that glue_event returns 0 for an app zl does not
     * handle, which is exactly what the old conditional produced, so gating it
     * on zl_fn_app_event buys nothing and costs one more way for the two
     * branches to disagree. */
    wm_hooks(glue_draw,
             glue_event,
             zl_fn_app_tick  ? glue_tick  : 0,
             zl_fn_desk_draw ? glue_desk  : 0);
    if (zl_fn_desk_click) wm_desk_click(glue_desk_click);
    if (zl_fn_desk_key)   wm_desk_key(glue_desk_key);
    return 1;
}

/* Is the compositor reachable at all? kernel.zl asks this before choosing
 * between the frame loop and the old shell loop. Both conditions matter and
 * for different reasons:
 *
 *   px_w() != 0   there IS a framebuffer. verify.sh boots -kernel with
 *                 -display none, where QEMU's multiboot loader supplies no
 *                 framebuffer tag at all, so this is 0 and the plain text
 *                 shell must keep working - byte-identical transcript.
 *   the apps      kernel.zl actually defines app_draw.
 */
unsigned int fb_pxw(void);

int wm_available(void)
{
    return fb_pxw() != 0 && zl_fn_app_draw != 0;
}
