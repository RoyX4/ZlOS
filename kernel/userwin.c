/* userwin.c - the bounded Ring-3 window and input mailbox.
 *
 * User code never receives a framebuffer pointer or a wm window index. It owns
 * one of two opaque handles, replaces a bounded UTF-8/text payload, and polls
 * fixed-size input records. wm.c recognizes the reserved app IDs and calls the
 * draw/event functions below. Ownership checks happen again here even though
 * the syscall layer already selected the current PID. */
typedef unsigned int u32;

#include "telemetry.h"

#define USERWIN_MAX 2
#define USERWIN_TEXT 256
#define USERWIN_EVENTS 8
#define USERWIN_APP_BASE 30000

struct userwin_event { u32 type, code, x, y; };
struct userwin {
    int used, owner, wm_win;
    char title[24];
    char text[USERWIN_TEXT];
    struct userwin_event events[USERWIN_EVENTS];
    u32 head, tail, count, dropped;
};

static struct userwin windows[USERWIN_MAX];

extern int wm_running(void);
extern int wm_open(int app, const char *title, int x, int y, int w, int h);
extern int wm_is_open(int win);
extern void wm_close(int win);
extern void wm_invalidate_client(int win);
extern void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);
extern void fb_text_prop(int x, int y, const char *s, unsigned int rgb);

static struct userwin *owned(int owner, int handle)
{
    if (handle <= 0 || handle > USERWIN_MAX) return 0;
    struct userwin *w = &windows[handle - 1];
    return w->used && w->owner == owner ? w : 0;
}

int userwin_open(int owner, const char *title)
{
    unsigned operation_id = zlt_operation_begin(
        ZLLOG_SUB_DISPLAY, ZLLOG_OBJ_PROCESS, (unsigned)owner,
        ZLLOG_OP_WINDOW_OPEN, 0u);
    if (owner <= 0 || !title || !title[0]) {
        zlt_operation_result(ZLLOG_SUB_DISPLAY, operation_id,
                             ZLLOG_OP_WINDOW_OPEN, -1, 22u, 0u);
        return -1;
    }
    for (int i = 0; i < USERWIN_MAX; i++) {
        if (windows[i].used) continue;
        struct userwin *w = &windows[i];
        w->used = 1; w->owner = owner; w->wm_win = -1;
        w->head = w->tail = w->count = w->dropped = 0;
        int n = 0;
        while (n < 23 && title[n]) { w->title[n] = title[n]; n++; }
        w->title[n] = 0; w->text[0] = 0;
        if (wm_running()) {
            w->wm_win = wm_open(USERWIN_APP_BASE + i, w->title,
                                160 + i * 36, 140 + i * 32, 480, 300);
            if (w->wm_win < 0) {
                w->used = 0;
                zlt_operation_result(ZLLOG_SUB_DISPLAY, operation_id,
                                     ZLLOG_OP_WINDOW_OPEN, -1, 28u, 0u);
                return -1;
            }
        }
        zlt_operation_result(ZLLOG_SUB_DISPLAY, operation_id,
                             ZLLOG_OP_WINDOW_OPEN, i + 1, 0u,
                             (unsigned)(w->wm_win + 1));
        zlt_lifecycle(ZLLOG_SUB_DISPLAY, ZLLOG_OBJ_WINDOW, (unsigned)(i + 1),
                      ZLLOG_LIFE_START, (unsigned)owner,
                      (unsigned)(w->wm_win + 1));
        return i + 1;
    }
    zlt_operation_result(ZLLOG_SUB_DISPLAY, operation_id,
                         ZLLOG_OP_WINDOW_OPEN, -1, 28u, 0u);
    return -1;
}

int userwin_present(int owner, int handle, const char *text, u32 bytes)
{
    unsigned operation_id = zlt_operation_begin(
        ZLLOG_SUB_DISPLAY, ZLLOG_OBJ_PROCESS, (unsigned)owner,
        ZLLOG_OP_WINDOW_PRESENT, (unsigned)handle);
    struct userwin *w = owned(owner, handle);
    if (!w || !text || bytes >= USERWIN_TEXT) {
        zlt_operation_result(ZLLOG_SUB_DISPLAY, operation_id,
                             ZLLOG_OP_WINDOW_PRESENT, -1,
                             w ? 22u : 9u, bytes);
        return 0;
    }
    for (u32 i = 0; i < bytes; i++) w->text[i] = text[i];
    w->text[bytes] = 0;
    if (w->wm_win >= 0 && wm_is_open(w->wm_win)) wm_invalidate_client(w->wm_win);
    zlt_operation_result(ZLLOG_SUB_DISPLAY, operation_id,
                         ZLLOG_OP_WINDOW_PRESENT, (int)bytes, 0u, bytes);
    return 1;
}

int userwin_poll(int owner, int handle, struct userwin_event *out)
{
    unsigned operation_id = zlt_operation_begin(
        ZLLOG_SUB_DISPLAY, ZLLOG_OBJ_PROCESS, (unsigned)owner,
        ZLLOG_OP_WINDOW_POLL, (unsigned)handle);
    struct userwin *w = owned(owner, handle);
    if (!w || !out || !w->count) {
        zlt_operation_result(ZLLOG_SUB_DISPLAY, operation_id,
                             ZLLOG_OP_WINDOW_POLL, -1,
                             !w ? 9u : (!out ? 22u : 11u),
                             w ? w->count : 0u);
        return 0;
    }
    *out = w->events[w->head];
    w->head = (w->head + 1u) % USERWIN_EVENTS;
    w->count--;
    zlt_operation_result(ZLLOG_SUB_DISPLAY, operation_id,
                         ZLLOG_OP_WINDOW_POLL, 1, 0u, w->count);
    return 1;
}

int userwin_close(int owner, int handle)
{
    unsigned operation_id = zlt_operation_begin(
        ZLLOG_SUB_DISPLAY, ZLLOG_OBJ_PROCESS, (unsigned)owner,
        ZLLOG_OP_WINDOW_CLOSE, (unsigned)handle);
    struct userwin *w = owned(owner, handle);
    if (!w) {
        zlt_operation_result(ZLLOG_SUB_DISPLAY, operation_id,
                             ZLLOG_OP_WINDOW_CLOSE, -1, 9u,
                             (unsigned)handle);
        return 0;
    }
    if (w->wm_win >= 0 && wm_is_open(w->wm_win)) wm_close(w->wm_win);
    w->used = 0; w->owner = 0; w->wm_win = -1;
    w->head = w->tail = w->count = 0;
    zlt_lifecycle(ZLLOG_SUB_DISPLAY, ZLLOG_OBJ_WINDOW, (unsigned)handle,
                  ZLLOG_LIFE_EXIT, (unsigned)owner, 0u);
    zlt_operation_result(ZLLOG_SUB_DISPLAY, operation_id,
                         ZLLOG_OP_WINDOW_CLOSE, 0, 0u,
                         (unsigned)handle);
    return 1;
}

void userwin_close_owner(int owner)
{
    for (int i = 0; i < USERWIN_MAX; i++)
        if (windows[i].used && windows[i].owner == owner) userwin_close(owner, i + 1);
}

int userwin_is_app(int app)
{
    int i = app - USERWIN_APP_BASE;
    return i >= 0 && i < USERWIN_MAX && windows[i].used;
}

void userwin_draw_app(int app, int x, int y, int w, int h, int focused)
{
    int i = app - USERWIN_APP_BASE;
    if (i < 0 || i >= USERWIN_MAX || !windows[i].used) return;
    fb_fill_px(x, y, w, h, focused ? 0x101820u : 0x0d1218u);
    fb_text_prop(x + 18, y + 18, windows[i].text, 0xe8edf2u);
}

int userwin_event_app(int app, int win, int type, int code, int x, int y)
{
    int i = app - USERWIN_APP_BASE;
    if (i < 0 || i >= USERWIN_MAX || !windows[i].used ||
        windows[i].wm_win != win) return 0;
    struct userwin *w = &windows[i];
    if (w->count >= USERWIN_EVENTS) { w->dropped++; return 0; }
    struct userwin_event *e = &w->events[w->tail];
    e->type = (u32)type; e->code = (u32)code; e->x = (u32)x; e->y = (u32)y;
    w->tail = (w->tail + 1u) % USERWIN_EVENTS;
    w->count++;
    return 0;
}

/* Deterministic injection is used only by the native EFI ABI gate. It takes
 * the same queue path as wm input and still enforces owner/handle bounds. */
int userwin_test_inject(int owner, int handle, u32 type, u32 code, u32 x, u32 y)
{
    struct userwin *w = owned(owner, handle);
    if (!w || w->count >= USERWIN_EVENTS) return 0;
    struct userwin_event *e = &w->events[w->tail];
    e->type = type; e->code = code; e->x = x; e->y = y;
    w->tail = (w->tail + 1u) % USERWIN_EVENTS;
    w->count++;
    return 1;
}

int userwin_count(void)
{
    int n = 0;
    for (int i = 0; i < USERWIN_MAX; i++) if (windows[i].used) n++;
    return n;
}

int userwin_text_byte(int owner, int handle, int index)
{
    struct userwin *w = owned(owner, handle);
    if (!w || index < 0 || index >= USERWIN_TEXT) return 0;
    return (unsigned char)w->text[index];
}

int userwin_has_wm_window(int owner, int handle)
{
    struct userwin *w = owned(owner, handle);
    return w && w->wm_win >= 0 && wm_is_open(w->wm_win);
}
