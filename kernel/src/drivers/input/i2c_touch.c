/* i2c_touch.c - Synaptics SYNA8006 report decoder and touchpad policy.
 *
 * The descriptor was read from the physical X1 Carbon Gen 8 through Linux:
 * 06CB:CD8B, report 2 is a relative mouse and report 3 is five precision-pad
 * contacts.  I2C-HID prepends a little-endian byte count to both.
 *
 * Report 3, after the two-byte I2C length and one-byte report ID:
 *   five x { confidence:1, tip:1, contact-id:3, pad:3, x:u16, y:u16 }
 *   scan-time:u16, contact-count:u8, physical-button:1, pad:7
 * Logical ranges are X 0..1162 and Y 0..634.  We retain deltas rather than
 * mapping a finger to one absolute screen point: that is touchpad behaviour,
 * while input.c applies the user's existing speed/acceleration policy.
 */
#include "i2c_touch.h"

#define TP_REPORT_MOUSE       2
#define TP_REPORT_PRECISION   3
#define TP_MOUSE_BYTES        6
#define TP_PRECISION_BYTES   32
#define TP_CONTACTS           5
#define TP_SCROLL_STEP       24
#define TP_TAP_TICKS         20u       /* idt_ticks is 100 Hz: 200 ms */
#define TP_TAP_TRAVEL        24

static int abs_i(int v) { return v < 0 ? -v : v; }

void zltouch_init(struct zltouch_state *s)
{
    unsigned char *p = (unsigned char *)s;
    for (unsigned i = 0; i < sizeof(*s); i++) p[i] = 0;
    s->contact_id = -1;
}

static void edge_push(struct zltouch_state *s, int buttons)
{
    buttons &= 3;
    if (buttons == s->buttons) return;
    s->buttons = buttons;
    unsigned next = (s->edge_head + 1u) & 7u;
    if (next == s->edge_tail) {
        s->edge_drops++;
        return;
    }
    s->edges[s->edge_head] = (unsigned char)buttons;
    s->edge_head = next;
}

static int report_size(const unsigned char *r, int bytes)
{
    if (!r || bytes < 3) return 0;
    int n = (int)r[0] | ((int)r[1] << 8);
    return n >= 3 && n <= bytes ? n : 0;
}

static int signed_byte(unsigned char v)
{
    return v & 0x80u ? (int)v - 256 : (int)v;
}

static int u16le(const unsigned char *p)
{
    return (int)p[0] | ((int)p[1] << 8);
}

static void end_touch(struct zltouch_state *s, unsigned ticks)
{
    if (s->tap_active && ticks - s->tap_tick <= TP_TAP_TICKS &&
        s->tap_travel <= TP_TAP_TRAVEL && s->buttons == 0) {
        /* A tap is history, not state. Queue both edges so a slow frame cannot
         * collapse the click into a final up state. */
        edge_push(s, 1);
        edge_push(s, 0);
    }
    s->mode = 0;
    s->contact_id = -1;
    s->tap_active = 0;
    s->scroll_rem = 0;
}

static int decode_mouse(struct zltouch_state *s, const unsigned char *r, int n)
{
    if (n < TP_MOUSE_BYTES) return 0;
    s->mode = 0;
    s->contact_id = -1;
    s->tap_active = 0;
    s->dx += signed_byte(r[4]);
    s->dy += signed_byte(r[5]);
    edge_push(s, r[3] & 3);
    return 1;
}

static int decode_precision(struct zltouch_state *s,
                            const unsigned char *r, int n, unsigned ticks)
{
    if (n < TP_PRECISION_BYTES) return 0;

    int ids[2] = {-1, -1};
    int xs[2] = {0, 0};
    int ys[2] = {0, 0};
    int active = 0;
    for (int i = 0; i < TP_CONTACTS; i++) {
        int at = 3 + i * 5;
        int flags = r[at];
        if (!(flags & 2)) continue;             /* tip switch */
        if (active < 2) {
            ids[active] = (flags >> 2) & 7;
            xs[active] = u16le(r + at + 1);
            ys[active] = u16le(r + at + 3);
        }
        active++;
    }

    /* The descriptor's count byte is advisory; active tip bits are the data
     * that actually carry coordinates. Still reject impossible corruption. */
    int declared = r[30] & 0x7f;
    if (declared > TP_CONTACTS || active > TP_CONTACTS) return 0;
    int physical = r[31] & 1;
    edge_push(s, physical);
    /* A clickpad press is already a click. Its eventual finger lift must not
     * add a second synthetic tap click. */
    if (physical) s->tap_active = 0;

    if (active == 0) {
        end_touch(s, ticks);
        return 1;
    }

    if (active >= 2) {
        int cy = (ys[0] + ys[1]) / 2;
        s->tap_active = 0;
        if (s->mode == 2) {
            s->scroll_rem += cy - s->scroll_y;
            while (s->scroll_rem >= TP_SCROLL_STEP) {
                s->wheel--;
                s->scroll_rem -= TP_SCROLL_STEP;
            }
            while (s->scroll_rem <= -TP_SCROLL_STEP) {
                s->wheel++;
                s->scroll_rem += TP_SCROLL_STEP;
            }
        } else {
            s->scroll_rem = 0;
        }
        s->mode = 2;
        s->scroll_y = cy;
        s->contact_id = -1;
        return 1;
    }

    int id = ids[0], x = xs[0], y = ys[0];
    if (s->mode == 1 && s->contact_id == id) {
        int dx = x - s->last_x;
        int dy = y - s->last_y;
        s->dx += dx;
        s->dy += dy;
        s->tap_travel += abs_i(dx) + abs_i(dy);
    } else {
        s->tap_active = physical ? 0 : 1;
        s->tap_tick = ticks;
        s->tap_x = x;
        s->tap_y = y;
        s->tap_travel = 0;
    }
    s->mode = 1;
    s->contact_id = id;
    s->last_x = x;
    s->last_y = y;
    return 1;
}

int zltouch_decode(struct zltouch_state *s, const unsigned char *r,
                   int bytes, unsigned ticks)
{
    int n = report_size(r, bytes);
    if (!s || !n) {
        if (s) s->malformed++;
        return 0;
    }

    int ok = 0;
    if (r[2] == TP_REPORT_MOUSE) ok = decode_mouse(s, r, n);
    else if (r[2] == TP_REPORT_PRECISION) ok = decode_precision(s, r, n, ticks);
    if (!ok) {
        s->malformed++;
        return 0;
    }
    s->reports++;
    return 1;
}

int zltouch_take_dx(struct zltouch_state *s)
{
    int v = s->dx; s->dx = 0; return v;
}
int zltouch_take_dy(struct zltouch_state *s)
{
    int v = s->dy; s->dy = 0; return v;
}
int zltouch_take_wheel(struct zltouch_state *s)
{
    int v = s->wheel; s->wheel = 0; return v;
}
int zltouch_take_button(struct zltouch_state *s)
{
    if (s->edge_tail == s->edge_head) return -1;
    int v = s->edges[s->edge_tail];
    s->edge_tail = (s->edge_tail + 1u) & 7u;
    return v;
}
