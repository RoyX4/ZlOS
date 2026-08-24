/* Exact SYNA8006 report/policy regression without I2C hardware. */
#include <stdio.h>
#include <string.h>
#include "../i2c_touch.h"

static int failures;
static void ok(const char *name, int pass)
{
    printf("  %-68s %s\n", name, pass ? "ok" : "FAIL");
    if (!pass) failures++;
}

static void precision(unsigned char r[32], int count,
                      int id0, int x0, int y0, int id1, int x1, int y1,
                      int button)
{
    memset(r, 0, 32);
    r[0] = 32; r[2] = 3;
    if (count > 0) {
        r[3] = (unsigned char)(3 | ((id0 & 7) << 2));
        r[4] = (unsigned char)x0; r[5] = (unsigned char)(x0 >> 8);
        r[6] = (unsigned char)y0; r[7] = (unsigned char)(y0 >> 8);
    }
    if (count > 1) {
        r[8] = (unsigned char)(3 | ((id1 & 7) << 2));
        r[9] = (unsigned char)x1; r[10] = (unsigned char)(x1 >> 8);
        r[11] = (unsigned char)y1; r[12] = (unsigned char)(y1 >> 8);
    }
    r[30] = (unsigned char)count;
    r[31] = (unsigned char)button;
}

int main(void)
{
    struct zltouch_state s;
    unsigned char r[32];
    zltouch_init(&s);

    unsigned char mouse[6] = {6, 0, 2, 1, 5, 0xfd};
    ok("report 2 relative X is decoded", zltouch_decode(&s, mouse, 6, 1) &&
       zltouch_take_dx(&s) == 5);
    ok("report 2 signed Y is decoded", zltouch_take_dy(&s) == -3);
    ok("physical press is retained as an ordered edge",
       zltouch_take_button(&s) == 1 && zltouch_take_button(&s) == -1);
    mouse[3] = 0; mouse[4] = 0; mouse[5] = 0;
    zltouch_decode(&s, mouse, 6, 2);
    ok("report 2 physical release is retained",
       zltouch_take_button(&s) == 0 && zltouch_take_button(&s) == -1);

    precision(r, 1, 4, 300, 200, 0, 0, 0, 0);
    ok("first precision contact seeds rather than jumps",
       zltouch_decode(&s, r, 32, 10) && zltouch_take_dx(&s) == 0);
    precision(r, 1, 4, 314, 193, 0, 0, 0, 0);
    zltouch_decode(&s, r, 32, 11);
    ok("same contact becomes relative pointer motion",
       zltouch_take_dx(&s) == 14 && zltouch_take_dy(&s) == -7);

    precision(r, 0, 0, 0, 0, 0, 0, 0, 0);
    zltouch_decode(&s, r, 32, 12);
    ok("short low-travel contact becomes tap press+release",
       zltouch_take_button(&s) == 1 && zltouch_take_button(&s) == 0 &&
       zltouch_take_button(&s) == -1);

    precision(r, 2, 1, 200, 300, 2, 500, 320, 0);
    zltouch_decode(&s, r, 32, 20);
    precision(r, 2, 1, 200, 268, 2, 500, 288, 0);
    zltouch_decode(&s, r, 32, 21);
    ok("two fingers moving up emit a positive scroll notch",
       zltouch_take_wheel(&s) == 1);
    ok("two-finger scroll does not also move the pointer",
       zltouch_take_dx(&s) == 0 && zltouch_take_dy(&s) == 0);

    precision(r, 1, 2, 500, 300, 0, 0, 0, 1);
    zltouch_decode(&s, r, 32, 30);
    ok("precision report physical click is decoded",
       zltouch_take_button(&s) == 1);
    precision(r, 0, 0, 0, 0, 0, 0, 0, 0);
    zltouch_decode(&s, r, 32, 31);
    ok("physical release is decoded without an accidental tap",
       zltouch_take_button(&s) == 0 && zltouch_take_button(&s) == -1);

    r[0] = 40;
    ok("oversized/corrupt I2C length is rejected",
       !zltouch_decode(&s, r, 32, 40) && s.malformed == 1);
    ok("all accepted reports are counted", s.reports == 9);

    printf("\n%d failure(s)\n", failures);
    return failures ? 1 : 0;
}
