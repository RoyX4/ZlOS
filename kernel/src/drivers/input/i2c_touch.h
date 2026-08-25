#ifndef ZLOS_I2C_TOUCH_H
#define ZLOS_I2C_TOUCH_H

/* Pure report decoding for the X1 Carbon Gen 8 Synaptics precision pad.
 * Transport stays in i2c_hid.c; keeping this state machine independent makes
 * the exact physical report layout host-testable. */
struct zltouch_state {
    int mode;                 /* 0 none, 1 one-finger pointer, 2 scroll */
    int contact_id;
    int last_x, last_y;
    int scroll_y, scroll_rem;
    int tap_active, tap_x, tap_y, tap_travel;
    unsigned tap_tick;
    int dx, dy, wheel;
    int buttons;
    unsigned char edges[8];
    unsigned edge_head, edge_tail, edge_drops;
    unsigned reports, malformed;
};

void zltouch_init(struct zltouch_state *s);
int  zltouch_decode(struct zltouch_state *s, const unsigned char *report,
                    int bytes, unsigned ticks);
int  zltouch_take_dx(struct zltouch_state *s);
int  zltouch_take_dy(struct zltouch_state *s);
int  zltouch_take_wheel(struct zltouch_state *s);
int  zltouch_take_button(struct zltouch_state *s);

#endif
