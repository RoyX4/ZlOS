#ifndef ZLOS_REFAPPS_H
#define ZLOS_REFAPPS_H

enum {
    APP_CATALOG = 13,
    APP_FILES = 14, APP_KLOG, APP_HEX, APP_FRAMEBUFFER, APP_TTY,
    APP_CALCULATOR, APP_FONT_ATLAS, APP_DISK_USAGE, APP_SERVICES,
    APP_ARCHIVE, APP_IMAGE_VIEWER, APP_NETWORK, APP_CLOCKS, APP_SYSINFO,
    APP_U_CLIPBOARD, APP_U_COLOUR, APP_U_REGEX, APP_U_BASE, APP_U_DIFF,
    APP_U_HASH, APP_U_UNIT, APP_U_NOTES, APP_U_KEYS, APP_U_BENCH,
    APP_G_TETRIS, APP_G_PONG, APP_G_BREAKOUT, APP_G_MINES, APP_G_2048,
    APP_G_LIFE, APP_G_ASTEROIDS, APP_G_INVADERS, APP_G_TICTACTOE,
    APP_G_FIFTEEN, APP_G_CONNECT4, APP_G_REVERSI, APP_G_LIGHTS,
    APP_G_SIMON, APP_G_MAZE, APP_G_SOKOBAN, APP_G_FLAPPY,
    APP_G_MISSILE, APP_G_HANOI, APP_G_NIM, APP_G_BLACKJACK,
    APP_G_FROGGER, APP_G_WORDLE,
    APP_REF_LAST = APP_G_WORDLE
};

enum { REF_SYSTEM = 1, REF_TOOL = 2, REF_GAME = 3 };

struct ref_app {
    int id;
    const char *name;
    const char *kind;
    int category;
    int w, h;
};

int refapps_is_managed(int app);
void refapps_draw(int app, int x, int y, int w, int h, int focused);
int refapps_event(int app, int win, int type, int code, int x, int y);
int refapps_tick(int app, int win);

int refapps_count(void);
const struct ref_app *refapps_at(int i);
const struct ref_app *refapps_by_id(int id);
int refapps_open(int id);

#endif
