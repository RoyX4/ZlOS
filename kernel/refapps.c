#include "appkit.h"
#include "refapps.h"

void fb_rrect(int,int,int,int,int,unsigned);

/* Existing zlOS apps keep their IDs. The table order is exactly ds.html's
 * APPS.concat(UTILS).concat(GAME_APPS), which is a useful invariant: the
 * catalog is the attachment, not an approximation assembled by category. */
static const struct ref_app APPS[] = {
    {0,  "Terminal",        "System", REF_SYSTEM, 600,368},
    {APP_FILES,"Files",     "System", REF_SYSTEM, 672,352},
    {1,  "System Monitor",  "System", REF_SYSTEM, 424,376},
    {12, "Text Editor",     "System", REF_SYSTEM, 522,360},
    {APP_KLOG,"Kernel Log", "System", REF_SYSTEM, 544,320},
    {9,  "Renderer",        "System", REF_SYSTEM, 352,326},
    {APP_HEX,"Hex Viewer",  "System", REF_SYSTEM, 566,340},
    {APP_FRAMEBUFFER,"Framebuffer","System",REF_SYSTEM,600,420},
    {APP_TTY,"Console (tty1)","System",REF_SYSTEM,616,396},
    {APP_CALCULATOR,"Calculator","System",REF_SYSTEM,300,404},
    {APP_FONT_ATLAS,"Font Atlas","System",REF_SYSTEM,564,396},
    {APP_DISK_USAGE,"Disk Usage","System",REF_SYSTEM,456,344},
    {APP_SERVICES,"Services","System",REF_SYSTEM,596,392},
    {APP_ARCHIVE,"Archive Manager","System",REF_SYSTEM,590,384},
    {APP_IMAGE_VIEWER,"Image Viewer","System",REF_SYSTEM,520,410},
    {APP_NETWORK,"Network","System",REF_SYSTEM,560,380},
    {APP_CLOCKS,"Clocks & Timers","System",REF_SYSTEM,520,352},
    {APP_SYSINFO,"System Info","System",REF_SYSTEM,544,398},
    {6,  "Settings",        "System", REF_SYSTEM, 486,332},

    {APP_U_CLIPBOARD,"Clipboard","Tool",REF_TOOL,470,352},
    {APP_U_COLOUR,"Colour Picker","Tool",REF_TOOL,470,352},
    {APP_U_REGEX,"Regex Tester","Tool",REF_TOOL,470,352},
    {APP_U_BASE,"Base Converter","Tool",REF_TOOL,470,352},
    {APP_U_DIFF,"Text Diff","Tool",REF_TOOL,470,352},
    {APP_U_HASH,"Checksum","Tool",REF_TOOL,470,352},
    {APP_U_UNIT,"Unit Converter","Tool",REF_TOOL,470,352},
    {APP_U_NOTES,"Sticky Notes","Tool",REF_TOOL,470,352},
    {APP_U_KEYS,"Keyboard Tester","Tool",REF_TOOL,470,352},
    {APP_U_BENCH,"Benchmark","Tool",REF_TOOL,470,352},

    {3,"Snake","Arcade",REF_GAME,420,360},
    {APP_G_TETRIS,"Tetris","Puzzle",REF_GAME,360,440},
    {APP_G_PONG,"Pong","Arcade",REF_GAME,480,330},
    {APP_G_BREAKOUT,"Breakout","Arcade",REF_GAME,460,360},
    {APP_G_MINES,"Minesweeper","Puzzle",REF_GAME,430,380},
    {APP_G_2048,"2048","Puzzle",REF_GAME,360,400},
    {APP_G_LIFE,"Conway's Life","Sim",REF_GAME,520,400},
    {APP_G_ASTEROIDS,"Asteroids","Arcade",REF_GAME,470,380},
    {APP_G_INVADERS,"Invaders","Arcade",REF_GAME,450,390},
    {APP_G_TICTACTOE,"Tic-Tac-Toe","Board",REF_GAME,350,380},
    {APP_G_FIFTEEN,"15 Puzzle","Puzzle",REF_GAME,360,400},
    {APP_G_CONNECT4,"Connect Four","Board",REF_GAME,430,400},
    {APP_G_REVERSI,"Reversi","Board",REF_GAME,400,420},
    {APP_G_LIGHTS,"Lights Out","Puzzle",REF_GAME,360,390},
    {APP_G_SIMON,"Simon","Puzzle",REF_GAME,360,390},
    {APP_G_MAZE,"Maze","Puzzle",REF_GAME,500,390},
    {APP_G_SOKOBAN,"Sokoban","Puzzle",REF_GAME,430,340},
    {APP_G_FLAPPY,"Flappy","Arcade",REF_GAME,430,360},
    {APP_G_MISSILE,"Missile Command","Arcade",REF_GAME,470,370},
    {APP_G_HANOI,"Tower of Hanoi","Puzzle",REF_GAME,430,320},
    {APP_G_NIM,"Nim","Board",REF_GAME,400,330},
    {APP_G_BLACKJACK,"Blackjack","Board",REF_GAME,400,340},
    {APP_G_FROGGER,"Frogger","Arcade",REF_GAME,430,380},
    {APP_G_WORDLE,"Word Guess","Puzzle",REF_GAME,400,380}
};

#define N_APPS ((int)(sizeof APPS / sizeof APPS[0]))

void refsys_draw(int, int, int, int, int, int);
int refsys_event(int, int, int, int, int, int);
int refsys_tick(int, int);
void reftool_draw(int, int, int, int, int, int);
int reftool_event(int, int, int, int, int, int);
int reftool_tick(int, int);
void refgame_draw(int, int, int, int, int, int);
int refgame_event(int, int, int, int, int, int);
int refgame_tick(int, int);

static int cat;
static int page;
static char query[32];

int refapps_count(void) { return N_APPS; }

const struct ref_app *refapps_at(int i)
{
    return i >= 0 && i < N_APPS ? &APPS[i] : 0;
}

const struct ref_app *refapps_by_id(int id)
{
    for (int i = 0; i < N_APPS; i++) if (APPS[i].id == id) return &APPS[i];
    return 0;
}

int refapps_is_managed(int app)
{
    return app >= APP_CATALOG && app <= APP_REF_LAST;
}

static int lower(int c) { return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c; }

static int contains(const char *s, const char *q)
{
    if (!q || !*q) return 1;
    for (int i = 0; s && s[i]; i++) {
        int j = 0;
        while (q[j] && s[i + j] && lower(s[i + j]) == lower(q[j])) j++;
        if (!q[j]) return 1;
    }
    return 0;
}

static int visible(int i)
{
    if (i < 0 || i >= N_APPS) return 0;
    if (cat && APPS[i].category != cat) return 0;
    return contains(APPS[i].name, query) || contains(APPS[i].kind, query);
}

static int nth_visible(int n)
{
    for (int i = 0; i < N_APPS; i++) if (visible(i) && n-- == 0) return i;
    return -1;
}

static int visible_count(void)
{
    int n = 0;
    for (int i = 0; i < N_APPS; i++) if (visible(i)) n++;
    return n;
}

int refapps_open(int id)
{
    const struct ref_app *a = refapps_by_id(id);
    if (!a) return -1;
    return ak_open_once(a->id, a->name, a->w, a->h, id);
}

static void catalog_draw(int x, int y, int w, int h)
{
    int u = ui_theme()->scale;
    int pad = 12 * u, top = 42 * u, gap = 8 * u;
    int cols = w >= 620 * u ? 4 : (w >= 440 * u ? 3 : 2);
    int rows = 3, tilew = (w - pad * 2 - gap * (cols - 1)) / cols;
    int tileh = (h - top - pad - gap * (rows - 1)) / rows;
    const char *tabs[4] = {"All", "System", "Tools", "Games"};
    ak_fill((struct ak_rect){x,y,w,h}, AK_SURFACE);
    for (int i = 0; i < 4; i++)
        ak_button((struct ak_rect){x + pad + i * 76 * u, y + 8 * u, 68 * u, 25 * u},
                  tabs[i], cat == (i ? i : 0));
    ak_card((struct ak_rect){x + w - 220 * u, y + 7 * u, 208 * u, 27 * u});
    ak_text_mono(x + w - 210 * u, y + 11 * u,
                 query[0] ? query : "type to search...", query[0] ? AK_TEXT : AK_FAINT);

    for (int slot = 0; slot < cols * rows; slot++) {
        int idx = nth_visible(page + slot);
        int cx = slot % cols, cy = slot / cols;
        struct ak_rect r = {x + pad + cx * (tilew + gap), y + top + cy * (tileh + gap), tilew, tileh};
        ak_card(r);
        if (idx < 0) continue;
        unsigned mark = APPS[idx].category == REF_GAME ? AK_WARN :
                        APPS[idx].category == REF_TOOL ? AK_ACCENT : AK_OK;
        fb_rrect(r.x + 10*u, r.y + 10*u, 28*u, 28*u, 9*u, mark);
        ak_text_bold(r.x + 48*u, r.y + 10*u, APPS[idx].name, AK_BRIGHT);
        ak_text_mono(r.x + 48*u, r.y + 28*u, APPS[idx].kind, AK_DIM);
        ak_text_mono(r.x + 10*u, r.y + r.h - 20*u, "open", AK_ACCENT_2);
    }
    char count[16]; ak_u32(count, (unsigned)visible_count());
    ak_text_mono(x + pad, y + h - 15*u, count, AK_DIM);
    ak_text_mono(x + pad + 34*u, y + h - 15*u, "apps  |  wheel for more", AK_FAINT);
}

static int catalog_event(int win, int type, int code, int px, int py)
{
    int x,y,w,h,u=ui_theme()->scale;
    wm_client(win,&x,&y,&w,&h);
    if (type == AK_EV_CHAR) {
        if (code == 8 || code == 127) ak_backspace(query);
        else ak_append_char(query, (int)sizeof query, code);
        page = 0; wm_damage_win(win); return 1;
    }
    if (type == AK_EV_KEY_DOWN && code == AK_KEY_BACKSPACE) {
        ak_backspace(query); page = 0; wm_damage_win(win); return 1;
    }
    if (type == AK_EV_WHEEL) {
        int max = visible_count();
        page += code > 0 ? -4 : 4;
        page = ak_clamp(page, 0, max > 12 ? max - 12 : 0);
        wm_damage_win(win); return 1;
    }
    if (type != AK_EV_MOUSE || !(code & 1)) return type == AK_EV_MOUSE;
    for (int i=0;i<4;i++) {
        struct ak_rect r={x+12*u+i*76*u,y+8*u,68*u,25*u};
        if (ak_hit(r,px,py)) { cat=i; page=0; wm_damage_win(win); return 1; }
    }
    int pad=12*u,top=42*u,gap=8*u;
    int cols=w>=620*u?4:(w>=440*u?3:2), rows=3;
    int tw=(w-pad*2-gap*(cols-1))/cols;
    int th=(h-top-pad-gap*(rows-1))/rows;
    for (int slot=0;slot<cols*rows;slot++) {
        struct ak_rect r={x+pad+(slot%cols)*(tw+gap),y+top+(slot/cols)*(th+gap),tw,th};
        if (ak_hit(r,px,py)) { int idx=nth_visible(page+slot); if(idx>=0) refapps_open(APPS[idx].id); return 1; }
    }
    return 1;
}

void refapps_draw(int app, int x, int y, int w, int h, int focused)
{
    if (app == APP_CATALOG) { catalog_draw(x,y,w,h); return; }
    if (app >= APP_FILES && app <= APP_SYSINFO) { refsys_draw(app,x,y,w,h,focused); return; }
    if (app >= APP_U_CLIPBOARD && app <= APP_U_BENCH) { reftool_draw(app,x,y,w,h,focused); return; }
    if (app >= APP_G_TETRIS && app <= APP_G_WORDLE) refgame_draw(app,x,y,w,h,focused);
}

int refapps_event(int app, int win, int type, int code, int x, int y)
{
    if (app == APP_CATALOG) return catalog_event(win,type,code,x,y);
    if (app >= APP_FILES && app <= APP_SYSINFO) return refsys_event(app,win,type,code,x,y);
    if (app >= APP_U_CLIPBOARD && app <= APP_U_BENCH) return reftool_event(app,win,type,code,x,y);
    if (app >= APP_G_TETRIS && app <= APP_G_WORDLE) return refgame_event(app,win,type,code,x,y);
    return 0;
}

int refapps_tick(int app, int win)
{
    if (app >= APP_FILES && app <= APP_SYSINFO) return refsys_tick(app,win);
    if (app >= APP_U_CLIPBOARD && app <= APP_U_BENCH) return reftool_tick(app,win);
    if (app >= APP_G_TETRIS && app <= APP_G_WORDLE) return refgame_tick(app,win);
    return 0;
}
