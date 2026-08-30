#include "appkit.h"
#include "refapps.h"

void fb_fill_px(int,int,int,int,unsigned);
void fb_rrect(int,int,int,int,int,unsigned);
void fb_line(int,int,int,int,unsigned);
void fb_text_scaled(int,int,const char*,int,unsigned);
unsigned idt_ticks(void);

struct game {
    int ready, board[384];
    int x,y,dx,dy,a,b,c,d,score,turn,over,paused,last;
    char word[8]; int wn;
};
static struct game G[23];

static int gi(int app){return app-APP_G_TETRIS;}
static int inside(int v,int lo,int hi){return v>=lo&&v<hi;}
static unsigned game_col(int n){static const unsigned c[]={0x57B6FF,0xB8E838,0xF5B93C,0xFF6A50,0x8F7BFF,0x4CE0B3,0xEAF3FF};return c[(unsigned)n%7];}

static void reset_game(int app)
{
    struct game *g=&G[gi(app)];for(int i=0;i<384;i++)g->board[i]=0;
    g->ready=1;g->x=3;g->y=0;g->dx=1;g->dy=1;g->a=0;g->b=0;g->c=0;g->d=0;g->score=0;g->turn=1;g->over=0;g->paused=0;g->last=(int)idt_ticks();g->wn=0;g->word[0]=0;
    if(app==APP_G_PONG){g->x=50;g->y=50;g->dx=2;g->dy=2;g->a=45;g->b=45;}
    if(app==APP_G_BREAKOUT){g->x=50;g->y=70;g->dx=2;g->dy=-2;g->a=50;for(int i=0;i<40;i++)g->board[i]=1;}
    if(app==APP_G_MINES){for(int i=0;i<64;i++)g->board[i]=1;unsigned s=0xC0DEu;for(int n=0;n<10;n++){s=s*1664525u+1013904223u;g->board[(s>>16)&63]=9;}}
    if(app==APP_G_2048){g->board[0]=2;g->board[5]=2;}
    if(app==APP_G_LIFE){for(int i=0;i<192;i++)g->board[i]=((i*17+i/7)%11)==0;g->paused=1;}
    if(app==APP_G_ASTEROIDS){g->x=50;g->y=50;for(int i=0;i<12;i++)g->board[i]=(i*29+17)%90+5;}
    if(app==APP_G_INVADERS){g->x=48;g->a=1;for(int i=0;i<32;i++)g->board[i]=1;}
    if(app==APP_G_FIFTEEN){for(int i=0;i<15;i++)g->board[i]=i+1;g->board[15]=0;}
    if(app==APP_G_CONNECT4||app==APP_G_REVERSI){if(app==APP_G_REVERSI){g->board[27]=1;g->board[28]=2;g->board[35]=2;g->board[36]=1;}}
    if(app==APP_G_LIGHTS){for(int i=0;i<25;i++)g->board[i]=((i*7+3)%5)==0;}
    if(app==APP_G_SIMON){for(int i=0;i<16;i++)g->board[i]=(i*3+1)&3;g->a=1;}
    if(app==APP_G_MAZE){for(int y=0;y<13;y++)for(int x=0;x<21;x++)g->board[y*21+x]=(x==0||y==0||x==20||y==12||((x*7+y*5)%13<3));g->board[22]=0;g->board[12*21-2]=0;g->x=1;g->y=1;}
    if(app==APP_G_SOKOBAN){for(int y=0;y<8;y++)for(int x=0;x<12;x++)g->board[y*12+x]=(x==0||y==0||x==11||y==7)?1:0;g->board[4*12+6]=2;g->board[4*12+8]=3;g->x=3;g->y=4;}
    if(app==APP_G_FLAPPY){g->x=24;g->y=45;g->dy=0;g->a=90;g->b=45;}
    if(app==APP_G_MISSILE){for(int i=0;i<6;i++)g->board[i]=15+i*14;g->a=50;g->b=80;}
    if(app==APP_G_HANOI){for(int i=0;i<5;i++)g->board[i]=5-i;g->a=5;g->b=0;g->c=0;g->turn=-1;}
    if(app==APP_G_NIM){g->board[0]=1;g->board[1]=3;g->board[2]=5;g->board[3]=7;}
    if(app==APP_G_BLACKJACK){g->a=10+(int)(ak_xorshift()%11);g->b=10+(int)(ak_xorshift()%11);g->c=10+(int)(ak_xorshift()%11);}
    if(app==APP_G_FROGGER){g->x=5;g->y=9;for(int i=0;i<8;i++)g->board[i]=(i*3)%10;}
    if(app==APP_G_WORDLE)g->turn=0;
}

static struct game *gs(int app){struct game *g=&G[gi(app)];if(!g->ready)reset_game(app);return g;}

static void status(int app,int x,int y,int w,int h,int u,const char *keys)
{
    char id[16];ak_i32(id,app);ak_fill((struct ak_rect){x,y+h-30*u,w,30*u},AK_PANEL);ak_badge(x+10*u,y+h-24*u,id,AK_ACCENT_2);ak_text_mono(x+68*u,y+h-22*u,keys,AK_FAINT);
}

static void score_text(struct game *g,int x,int y,int u)
{
    char n[16];ak_i32(n,g->score);ak_text_mono(x,y,"score",AK_DIM);ak_text_mono(x+48*u,y,n,AK_ACCENT_2);
}

static void draw_tetris(struct game*g,int x,int y,int w,int h,int u)
{
    int ch=(h-42*u)/18,cw=ch,bx=x+w/2-5*cw,by=y+6*u;
    for(int yy=0;yy<18;yy++)for(int xx=0;xx<10;xx++){int v=g->board[yy*10+xx];fb_rrect(bx+xx*cw,by+yy*ch,cw-1,ch-1,2,v?game_col(v):AK_RAISED);}
    for(int yy=0;yy<2;yy++)for(int xx=0;xx<2;xx++)if(inside(g->x+xx,0,10)&&inside(g->y+yy,0,18))fb_rrect(bx+(g->x+xx)*cw,by+(g->y+yy)*ch,cw-1,ch-1,2,AK_ACCENT);
    score_text(g,x+12*u,y+10*u,u);
}

static void draw_pong(struct game*g,int x,int y,int w,int h,int u)
{
    int bh=h-30*u;for(int yy=0;yy<bh;yy+=12*u)fb_fill_px(x+w/2, y+yy,2,6*u,AK_FAINT);
    fb_rrect(x+12*u,y+g->a*(bh-50*u)/100,8*u,50*u,4*u,AK_TEXT);fb_rrect(x+w-20*u,y+g->b*(bh-50*u)/100,8*u,50*u,4*u,AK_TEXT);fb_rrect(x+g->x*w/100,y+g->y*bh/100,10*u,10*u,5*u,AK_ACCENT);score_text(g,x+12*u,y+8*u,u);
}

static void draw_breakout(struct game*g,int x,int y,int w,int h,int u)
{
    int bh=h-30*u,bw=(w-20*u)/10;for(int i=0;i<40;i++)if(g->board[i])fb_rrect(x+10*u+(i%10)*bw,y+10*u+(i/10)*18*u,bw-2,14*u,3*u,game_col(i/10));
    fb_rrect(x+g->a*(w-80*u)/100,y+bh-20*u,80*u,8*u,4*u,AK_TEXT);fb_rrect(x+g->x*w/100,y+g->y*bh/100,9*u,9*u,5*u,AK_ACCENT);score_text(g,x+12*u,y+bh-45*u,u);
}

static int mine_adj(struct game*g,int q){int x=q%8,y=q/8,n=0;for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++){int xx=x+dx,yy=y+dy;if(inside(xx,0,8)&&inside(yy,0,8)&&g->board[yy*8+xx]%10==9)n++;}return n;}
static void draw_mines(struct game*g,int x,int y,int w,int h,int u)
{
    int c=(h-65*u)/8,bx=x+w/2-4*c,by=y+10*u;for(int i=0;i<64;i++){int rev=g->board[i]>=10;fb_rrect(bx+(i%8)*c,by+(i/8)*c,c-2,c-2,4*u,rev?AK_RAISED:AK_PANEL);if(rev){char n[3];if(g->board[i]%10==9)ak_copy(n,3,"*");else ak_u32(n,(unsigned)mine_adj(g,i));ak_text_mono(bx+(i%8)*c+8*u,by+(i/8)*c+5*u,n,g->board[i]%10==9?AK_BAD:AK_ACCENT_2);}}
    ak_text_mono(x+12*u,y+h-52*u,g->over?"mine hit - click reset":"reveal every safe cell",g->over?AK_BAD:AK_DIM);
}

static void draw_2048(struct game*g,int x,int y,int w,int h,int u)
{
    int c=(h-76*u)/4,bx=x+w/2-2*c,by=y+12*u;for(int i=0;i<16;i++){int v=g->board[i];fb_rrect(bx+(i%4)*c,by+(i/4)*c,c-5*u,c-5*u,8*u,v?game_col(v):AK_RAISED);char n[12];ak_i32(n,v);if(v)ak_text_bold(bx+(i%4)*c+12*u,by+(i/4)*c+c/2-8*u,n,AK_BG);}score_text(g,x+12*u,y+h-52*u,u);
}

static void draw_life(struct game*g,int x,int y,int w,int h,int u)
{
    int cw=(w-20*u)/16,ch=(h-65*u)/12;for(int yy=0;yy<12;yy++)for(int xx=0;xx<16;xx++)fb_rrect(x+10*u+xx*cw,y+8*u+yy*ch,cw-1,ch-1,2,g->board[yy*16+xx]?AK_ACCENT:AK_RAISED);ak_text_mono(x+12*u,y+h-52*u,g->paused?"paused | click cells | space runs":"running | space pauses",g->paused?AK_WARN:AK_OK);
}

static void draw_space(struct game*g,int app,int x,int y,int w,int h,int u)
{
    int bh=h-30*u;
    if(app==APP_G_ASTEROIDS){for(int i=0;i<6;i++){int ax=x+g->board[i*2]*w/100,ay=y+g->board[i*2+1]*bh/100;fb_line(ax-8*u,ay,ax,ay-8*u,AK_DIM);fb_line(ax,ay-8*u,ax+8*u,ay,AK_DIM);fb_line(ax+8*u,ay,ax,ay+8*u,AK_DIM);fb_line(ax,ay+8*u,ax-8*u,ay,AK_DIM);}fb_line(x+g->x*w/100,y+g->y*bh/100-10*u,x+g->x*w/100-8*u,y+g->y*bh/100+8*u,AK_ACCENT);fb_line(x+g->x*w/100-8*u,y+g->y*bh/100+8*u,x+g->x*w/100+8*u,y+g->y*bh/100+8*u,AK_ACCENT);fb_line(x+g->x*w/100+8*u,y+g->y*bh/100+8*u,x+g->x*w/100,y+g->y*bh/100-10*u,AK_ACCENT);}
    else{for(int i=0;i<32;i++)if(g->board[i])fb_rrect(x+32*u+(i%8)*40*u+g->a*u,y+25*u+(i/8)*30*u,24*u,14*u,4*u,AK_OK);fb_rrect(x+g->x*w/100-18*u,y+bh-32*u,36*u,12*u,4*u,AK_ACCENT);for(int i=0;i<g->score%8;i++)fb_rrect(x+60*u+i*32*u,y+bh-70*u,3*u,10*u,1,AK_WARN);}score_text(g,x+12*u,y+8*u,u);
}

static void draw_ttt(struct game*g,int x,int y,int w,int h,int u)
{
    int c=(h-80*u)/3,bx=x+w/2-c*3/2,by=y+12*u;for(int i=1;i<3;i++){fb_line(bx+i*c,by,bx+i*c,by+3*c,AK_LINE);fb_line(bx,by+i*c,bx+3*c,by+i*c,AK_LINE);}for(int i=0;i<9;i++){int cx=bx+(i%3)*c,cy=by+(i/3)*c;if(g->board[i]==1){fb_line(cx+12*u,cy+12*u,cx+c-12*u,cy+c-12*u,AK_ACCENT);fb_line(cx+c-12*u,cy+12*u,cx+12*u,cy+c-12*u,AK_ACCENT);}if(g->board[i]==2){fb_rrect(cx+14*u,cy+14*u,c-28*u,c-28*u,(c-28*u)/2,AK_WARN);fb_rrect(cx+20*u,cy+20*u,c-40*u,c-40*u,(c-40*u)/2,AK_BG);}}ak_text_mono(x+12*u,y+h-52*u,g->over?"game over - click to reset":(g->turn==1?"X to move":"O to move"),g->over?AK_BAD:AK_DIM);
}

static void draw_fifteen(struct game*g,int x,int y,int w,int h,int u)
{
    int c=(h-70*u)/4,bx=x+w/2-2*c,by=y+10*u;for(int i=0;i<16;i++){int v=g->board[i];if(v){fb_rrect(bx+(i%4)*c,by+(i/4)*c,c-5*u,c-5*u,8*u,game_col(v));char n[8];ak_i32(n,v);ak_text_bold(bx+(i%4)*c+c/2-8*u,by+(i/4)*c+c/2-10*u,n,AK_BG);}}ak_text_mono(x+12*u,y+h-52*u,"slide a tile into the empty square",AK_DIM);
}

static void draw_connect(struct game*g,int x,int y,int w,int h,int u)
{
    int c=(h-74*u)/6,bx=x+w/2-7*c/2,by=y+10*u;fb_rrect(bx-5*u,by-5*u,7*c+10*u,6*c+10*u,8*u,0x234B72);for(int i=0;i<42;i++){unsigned col=g->board[i]==1?AK_BAD:(g->board[i]==2?AK_WARN:AK_BG);fb_rrect(bx+(i%7)*c+4*u,by+(i/7)*c+4*u,c-8*u,c-8*u,(c-8*u)/2,col);}ak_text_mono(x+12*u,y+h-52*u,g->turn==1?"red to drop":"amber to drop",AK_DIM);
}

static void draw_reversi(struct game*g,int x,int y,int w,int h,int u)
{
    int c=(h-72*u)/8,bx=x+w/2-4*c,by=y+8*u;fb_fill_px(bx,by,8*c,8*c,0x1B6B43);for(int i=0;i<64;i++){fb_line(bx+(i%8)*c,by+(i/8)*c,bx+(i%8+1)*c,by+(i/8)*c,0x124D30);if(g->board[i])fb_rrect(bx+(i%8)*c+4*u,by+(i/8)*c+4*u,c-8*u,c-8*u,(c-8*u)/2,g->board[i]==1?AK_BRIGHT:AK_BG);}ak_text_mono(x+12*u,y+h-52*u,g->turn==1?"white to move":"black to move",AK_DIM);
}

static void draw_lights(struct game*g,int x,int y,int w,int h,int u)
{
    int c=(h-76*u)/5,bx=x+w/2-5*c/2,by=y+10*u;for(int i=0;i<25;i++)fb_rrect(bx+(i%5)*c,by+(i/5)*c,c-6*u,c-6*u,10*u,g->board[i]?AK_WARN:AK_RAISED);ak_text_mono(x+12*u,y+h-52*u,"click toggles a light and its neighbours",AK_DIM);
}

static void draw_simon(struct game*g,int x,int y,int w,int h,int u)
{
    unsigned cs[]={AK_BAD,AK_ACCENT,0x57B6FF,AK_WARN};int cx=x+w/2,cy=y+(h-30*u)/2,r=72*u;for(int i=0;i<4;i++){int ox=(i&1)?4*u:-r,oy=(i&2)?4*u:-r;fb_rrect(cx+ox,cy+oy,r-4*u,r-4*u,16*u,cs[i]);}char n[16];ak_i32(n,g->a);ak_text_mono(x+12*u,y+12*u,"sequence",AK_DIM);ak_text_mono(x+88*u,y+12*u,n,AK_ACCENT_2);ak_text_mono(x+12*u,y+h-52*u,g->over?"wrong pad - click to reset":"repeat the growing sequence",g->over?AK_BAD:AK_DIM);
}

static void draw_maze(struct game*g,int x,int y,int w,int h,int u)
{
    int cw=(w-20*u)/21,ch=(h-56*u)/13;for(int yy=0;yy<13;yy++)for(int xx=0;xx<21;xx++)if(g->board[yy*21+xx])fb_fill_px(x+10*u+xx*cw,y+8*u+yy*ch,cw,ch,AK_RAISED);fb_rrect(x+10*u+g->x*cw,y+8*u+g->y*ch,cw,ch,3*u,AK_ACCENT);fb_rrect(x+10*u+19*cw,y+8*u+11*ch,cw,ch,3*u,AK_OK);ak_text_mono(x+12*u,y+h-52*u,"arrows / WASD | reach green",AK_DIM);
}

static void draw_sokoban(struct game*g,int x,int y,int w,int h,int u)
{
    int c=(h-62*u)/8,bx=x+w/2-6*c,by=y+8*u;for(int yy=0;yy<8;yy++)for(int xx=0;xx<12;xx++){int v=g->board[yy*12+xx];if(v==1)fb_fill_px(bx+xx*c,by+yy*c,c-1,c-1,AK_RAISED);if(v==2)fb_rrect(bx+xx*c+3*u,by+yy*c+3*u,c-6*u,c-6*u,4*u,AK_WARN);if(v==3)fb_rrect(bx+xx*c+c/3,by+yy*c+c/3,c/3,c/3,c/6,AK_OK);}fb_rrect(bx+g->x*c+c/4,by+g->y*c+c/4,c/2,c/2,c/4,AK_ACCENT);ak_text_mono(x+12*u,y+h-52*u,"push the crate onto the green target",AK_DIM);
}

static void draw_flappy(struct game*g,int x,int y,int w,int h,int u)
{
    int bh=h-30*u,px=x+g->a*w/100,gap=g->b*bh/100;fb_fill_px(px,y,18*u,gap-34*u,AK_OK);fb_fill_px(px,y+gap+34*u,18*u,bh-gap-34*u,AK_OK);fb_rrect(x+g->x*w/100,y+g->y*bh/100,18*u,14*u,6*u,AK_WARN);score_text(g,x+12*u,y+10*u,u);
}

static void draw_missile(struct game*g,int x,int y,int w,int h,int u)
{
    int bh=h-30*u;for(int i=0;i<3;i++)fb_rrect(x+(20+i*30)*w/100,y+bh-12*u,26*u,10*u,4*u,AK_OK);for(int i=0;i<6;i++){int sx=x+g->board[i]*w/100;fb_line(sx,y,sx+(i&1?20:-20)*u,y+g->a*bh/100,AK_BAD);}for(int i=0;i<g->score%6;i++)fb_rrect(x+(12+i*15)*w/100,y+bh-45*u,8*u,8*u,4*u,AK_WARN);score_text(g,x+12*u,y+10*u,u);
}

static void draw_hanoi(struct game*g,int x,int y,int w,int h,int u)
{
    int base=y+h-70*u,rodw=w/3;for(int r=0;r<3;r++){fb_fill_px(x+r*rodw+rodw/2,base-130*u,3*u,130*u,AK_DIM);fb_fill_px(x+r*rodw+20*u,base,rodw-40*u,4*u,AK_DIM);}int cnt[3]={g->a,g->b,g->c};for(int r=0;r<3;r++)for(int i=0;i<cnt[r];i++){int disk=g->board[r*8+i];int ww=(18+disk*10)*u;fb_rrect(x+r*rodw+rodw/2-ww/2,base-(i+1)*18*u,ww,15*u,5*u,game_col(disk));}ak_text_mono(x+12*u,y+h-52*u,g->turn<0?"click a rod to pick a disk":"click a destination rod",AK_DIM);
}

static void draw_nim(struct game*g,int x,int y,int w,int h,int u)
{
    for(int r=0;r<4;r++){for(int i=0;i<g->board[r];i++)fb_rrect(x+60*u+i*28*u,y+35*u+r*52*u,8*u,34*u,4*u,AK_WARN);char n[8];ak_i32(n,g->board[r]);ak_text_mono(x+18*u,y+43*u+r*52*u,n,AK_DIM);}ak_text_mono(x+12*u,y+h-52*u,g->turn==1?"your turn: click a row":"kernel moved",AK_DIM);
}

static void draw_blackjack(struct game*g,int x,int y,int w,int h,int u)
{
    int vals[4]={g->a,g->b,g->c,g->d};for(int i=0;i<(g->d?4:3);i++){int yy=i<2?y+36*u:y+150*u,xx=x+50*u+(i%2)*94*u;fb_rrect(xx,yy,76*u,104*u,8*u,AK_BRIGHT);char n[8];ak_i32(n,vals[i]);ak_text_bold(xx+18*u,yy+16*u,n,i<2?AK_BAD:AK_BG);}char p[16],d[16];ak_i32(p,g->a+g->b);ak_i32(d,g->c+g->d);ak_text_mono(x+w-130*u,y+42*u,"player",AK_DIM);ak_text_bold(x+w-70*u,y+42*u,p,g->a+g->b>21?AK_BAD:AK_ACCENT_2);ak_text_mono(x+w-130*u,y+166*u,"dealer",AK_DIM);ak_text_bold(x+w-70*u,y+166*u,d,AK_TEXT);ak_button((struct ak_rect){x+w-136*u,y+218*u,54*u,27*u},"Hit",0);ak_button((struct ak_rect){x+w-76*u,y+218*u,62*u,27*u},"Stand",0);
}

static void draw_frogger(struct game*g,int x,int y,int w,int h,int u)
{
    int bh=h-30*u,row=bh/10;for(int r=0;r<10;r++)fb_fill_px(x,y+r*row,w,row,r==0?0x1B6B43:(r==9?0x1B6B43:(r%3?0x20242A:0x18304A)));for(int r=1;r<9;r++){int car=(g->board[r-1]+g->a*(r%2?1:-1)+100)%100;fb_rrect(x+car*w/100,y+r*row+4*u,36*u,row-8*u,5*u,r%3?AK_BAD:AK_WARN);}fb_rrect(x+g->x*w/10+6*u,y+g->y*row+4*u,row-8*u,row-8*u,5*u,AK_OK);score_text(g,x+12*u,y+8*u,u);
}

static void draw_wordle(struct game*g,int x,int y,int w,int h,int u)
{
    const char *target="KERNEL";int c=(h-100*u)/6,bx=x+w/2-3*c,by=y+10*u;for(int r=0;r<6;r++)for(int q=0;q<6;q++){int k=r*6+q;char ch=(char)g->board[k];unsigned col=AK_RAISED;if(ch){col=ch==target[q]?AK_OK:(ch=='K'||ch=='E'||ch=='R'||ch=='N'||ch=='L'?AK_WARN:AK_FAINT);}fb_rrect(bx+q*c,by+r*c,c-5*u,c-5*u,6*u,col);if(ch){char s[2]={ch,0};ak_text_bold(bx+q*c+c/2-5*u,by+r*c+c/2-9*u,s,AK_BG);}}ak_card((struct ak_rect){x+24*u,y+h-70*u,w-48*u,30*u});ak_text_mono(x+36*u,y+h-63*u,g->word[0]?g->word:"type a six-letter word",g->word[0]?AK_TEXT:AK_FAINT);ak_text_mono(x+12*u,y+h-26*u,g->over==1?"solved":(g->over==2?"six guesses used":"Enter submits | Backspace edits"),g->over?AK_ACCENT_2:AK_DIM);
}

void refgame_draw(int app,int x,int y,int w,int h,int focused)
{
    (void)focused;struct game*g=gs(app);int u=ui_theme()->scale;ak_fill((struct ak_rect){x,y,w,h},AK_CANVAS);
    if(app==APP_G_TETRIS)draw_tetris(g,x,y,w,h,u);else if(app==APP_G_PONG)draw_pong(g,x,y,w,h,u);else if(app==APP_G_BREAKOUT)draw_breakout(g,x,y,w,h,u);else if(app==APP_G_MINES)draw_mines(g,x,y,w,h,u);else if(app==APP_G_2048)draw_2048(g,x,y,w,h,u);else if(app==APP_G_LIFE)draw_life(g,x,y,w,h,u);else if(app==APP_G_ASTEROIDS||app==APP_G_INVADERS)draw_space(g,app,x,y,w,h,u);else if(app==APP_G_TICTACTOE)draw_ttt(g,x,y,w,h,u);else if(app==APP_G_FIFTEEN)draw_fifteen(g,x,y,w,h,u);else if(app==APP_G_CONNECT4)draw_connect(g,x,y,w,h,u);else if(app==APP_G_REVERSI)draw_reversi(g,x,y,w,h,u);else if(app==APP_G_LIGHTS)draw_lights(g,x,y,w,h,u);else if(app==APP_G_SIMON)draw_simon(g,x,y,w,h,u);else if(app==APP_G_MAZE)draw_maze(g,x,y,w,h,u);else if(app==APP_G_SOKOBAN)draw_sokoban(g,x,y,w,h,u);else if(app==APP_G_FLAPPY)draw_flappy(g,x,y,w,h,u);else if(app==APP_G_MISSILE)draw_missile(g,x,y,w,h,u);else if(app==APP_G_HANOI)draw_hanoi(g,x,y,w,h,u);else if(app==APP_G_NIM)draw_nim(g,x,y,w,h,u);else if(app==APP_G_BLACKJACK)draw_blackjack(g,x,y,w,h,u);else if(app==APP_G_FROGGER)draw_frogger(g,x,y,w,h,u);else if(app==APP_G_WORDLE)draw_wordle(g,x,y,w,h,u);
    status(app,x,y,w,h,u,app==APP_G_WORDLE?"type + enter | R reset":"arrows / WASD | space | R reset");
}

static void dir_from(int code,int *dx,int *dy)
{
    *dx=*dy=0;if(code==AK_KEY_LEFT||code=='a'||code=='A')*dx=-1;if(code==AK_KEY_RIGHT||code=='d'||code=='D')*dx=1;if(code==AK_KEY_UP||code=='w'||code=='W')*dy=-1;if(code==AK_KEY_DOWN||code=='s'||code=='S')*dy=1;
}
static void spawn_2048(struct game*g){for(int i=0;i<16;i++)if(!g->board[i]){g->board[i]=2;return;}}
static void move_2048(struct game*g,int dx,int dy){int changed=0;for(int pass=0;pass<4;pass++)for(int y=0;y<4;y++)for(int x=0;x<4;x++){int i=y*4+x,nx=x+dx,ny=y+dy;if(!g->board[i]||!inside(nx,0,4)||!inside(ny,0,4))continue;int j=ny*4+nx;if(!g->board[j]){g->board[j]=g->board[i];g->board[i]=0;changed=1;}else if(g->board[j]==g->board[i]){g->board[j]*=2;g->score+=g->board[j];g->board[i]=0;changed=1;}}if(changed)spawn_2048(g);}
static void move_grid(struct game*g,int app,int dx,int dy)
{
    if(app==APP_G_MAZE){int nx=g->x+dx,ny=g->y+dy;if(inside(nx,0,21)&&inside(ny,0,13)&&!g->board[ny*21+nx]){g->x=nx;g->y=ny;if(nx==19&&ny==11)g->over=1;}}
    if(app==APP_G_SOKOBAN){int nx=g->x+dx,ny=g->y+dy,n=ny*12+nx;if(!inside(nx,0,12)||!inside(ny,0,8)||g->board[n]==1)return;if(g->board[n]==2){int bx=nx+dx,by=ny+dy,bn=by*12+bx;if(!inside(bx,0,12)||!inside(by,0,8)||g->board[bn]==1||g->board[bn]==2)return;g->board[bn]=2;g->board[n]=0;}g->x=nx;g->y=ny;}
    if(app==APP_G_FROGGER){g->x=ak_clamp(g->x+dx,0,9);g->y=ak_clamp(g->y+dy,0,9);if(g->y==0){g->score++;g->x=5;g->y=9;}}
}

static void key_game(int app,struct game*g,int code)
{
    if(code=='r'||code=='R'){reset_game(app);return;}int dx,dy;dir_from(code,&dx,&dy);
    if(app==APP_G_TETRIS){if(dx)g->x=ak_clamp(g->x+dx,0,8);if(dy>0)g->y++;}
    else if(app==APP_G_PONG){if(dy)g->a=ak_clamp(g->a+dy*8,0,100);}
    else if(app==APP_G_BREAKOUT){if(dx)g->a=ak_clamp(g->a+dx*8,0,100);}
    else if(app==APP_G_2048&& (dx||dy))move_2048(g,dx,dy);
    else if(app==APP_G_LIFE&&code==' ')g->paused=!g->paused;
    else if(app==APP_G_ASTEROIDS){g->x=ak_clamp(g->x+dx*4,2,98);g->y=ak_clamp(g->y+dy*4,2,98);if(code==' ')g->score++;}
    else if(app==APP_G_INVADERS){g->x=ak_clamp(g->x+dx*5,4,96);if(code==' ')g->score++;}
    else if(app==APP_G_MAZE||app==APP_G_SOKOBAN||app==APP_G_FROGGER)move_grid(g,app,dx,dy);
    else if(app==APP_G_FLAPPY&&(code==' '||dy<0))g->dy=-4;
}

static void click_board(int app,struct game*g,int win,int px,int py)
{
    int x,y,w,h,u=ui_theme()->scale;wm_client(win,&x,&y,&w,&h);
    if(app==APP_G_MINES){int c=(h-65*u)/8,bx=x+w/2-4*c,by=y+10*u,cx=(px-bx)/c,cy=(py-by)/c;if(inside(cx,0,8)&&inside(cy,0,8)){int i=cy*8+cx;g->board[i]+=10;if(g->board[i]%10==9)g->over=1;}else if(g->over)reset_game(app);}
    else if(app==APP_G_LIFE){int cw=(w-20*u)/16,ch=(h-65*u)/12,cx=(px-x-10*u)/cw,cy=(py-y-8*u)/ch;if(inside(cx,0,16)&&inside(cy,0,12))g->board[cy*16+cx]^=1;}
    else if(app==APP_G_TICTACTOE){int c=(h-80*u)/3,bx=x+w/2-c*3/2,by=y+12*u,cx=(px-bx)/c,cy=(py-by)/c;if(g->over){reset_game(app);return;}if(inside(cx,0,3)&&inside(cy,0,3)&&!g->board[cy*3+cx]){g->board[cy*3+cx]=g->turn;g->turn=3-g->turn;}}
    else if(app==APP_G_FIFTEEN){int c=(h-70*u)/4,bx=x+w/2-2*c,by=y+10*u,cx=(px-bx)/c,cy=(py-by)/c;if(inside(cx,0,4)&&inside(cy,0,4)){int i=cy*4+cx,z=0;while(g->board[z])z++;if(((i%4==z%4)&&(i-z==4||z-i==4))||((i/4==z/4)&&(i-z==1||z-i==1))){g->board[z]=g->board[i];g->board[i]=0;}}}
    else if(app==APP_G_CONNECT4){int c=(h-74*u)/6,bx=x+w/2-7*c/2,col=(px-bx)/c;if(inside(col,0,7))for(int r=5;r>=0;r--)if(!g->board[r*7+col]){g->board[r*7+col]=g->turn;g->turn=3-g->turn;break;}}
    else if(app==APP_G_REVERSI){int c=(h-72*u)/8,bx=x+w/2-4*c,by=y+8*u,cx=(px-bx)/c,cy=(py-by)/c;if(inside(cx,0,8)&&inside(cy,0,8)&&!g->board[cy*8+cx]){g->board[cy*8+cx]=g->turn;for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++){int nx=cx+dx,ny=cy+dy;if(inside(nx,0,8)&&inside(ny,0,8)&&g->board[ny*8+nx])g->board[ny*8+nx]=g->turn;}g->turn=3-g->turn;}}
    else if(app==APP_G_LIGHTS){int c=(h-76*u)/5,bx=x+w/2-5*c/2,by=y+10*u,cx=(px-bx)/c,cy=(py-by)/c;int ds[5][2]={{0,0},{1,0},{-1,0},{0,1},{0,-1}};for(int k=0;k<5;k++){int nx=cx+ds[k][0],ny=cy+ds[k][1];if(inside(nx,0,5)&&inside(ny,0,5))g->board[ny*5+nx]^=1;}}
    else if(app==APP_G_SIMON){if(g->over){reset_game(app);return;}int cx=px<x+w/2?0:1,cy=py<y+(h-30*u)/2?0:2,pad=cx+cy;if(pad!=g->board[g->b]){g->over=1;}else if(++g->b>=g->a){g->a++;g->b=0;g->score++;}}
    else if(app==APP_G_HANOI){int rod=(px-x)/(w/3);if(!inside(rod,0,3))return;int *cnt=rod==0?&g->a:(rod==1?&g->b:&g->c);if(g->turn<0){if(*cnt>0){g->turn=rod;g->d=g->board[rod*8+*cnt-1];(*cnt)--;}}else{if(*cnt==0||g->board[rod*8+*cnt-1]>g->d){g->board[rod*8+(*cnt)++]=g->d;g->score++;}else{int *old=g->turn==0?&g->a:(g->turn==1?&g->b:&g->c);g->board[g->turn*8+(*old)++]=g->d;}g->turn=-1;}}
    else if(app==APP_G_NIM){int row=(py-y-28*u)/(52*u);if(inside(row,0,4)&&g->board[row]>0){g->board[row]--;g->score++;}}
    else if(app==APP_G_BLACKJACK){if(py>y+200*u&&px>x+w-150*u){if(px<x+w-78*u){g->a+=1+(int)(ak_xorshift()%10);}else{g->d=1+(int)(ak_xorshift()%10);g->over=1;}}}
    else if(app==APP_G_MISSILE){g->score++;g->a=0;}
}

int refgame_event(int app,int win,int type,int code,int px,int py)
{
    struct game*g=gs(app);
    if(app==APP_G_WORDLE&&(type==AK_EV_CHAR||type==AK_EV_KEY_DOWN)){if(type==AK_EV_KEY_DOWN&&code==AK_KEY_BACKSPACE){if(g->wn)g->word[--g->wn]=0;}else if((code==13||code==AK_KEY_ENTER)&&g->wn==6){const char*t="KERNEL";int same=1;for(int i=0;i<6;i++){g->board[g->turn*6+i]=g->word[i];if(g->word[i]!=t[i])same=0;}g->turn++;g->wn=0;g->word[0]=0;if(same)g->over=1;else if(g->turn>=6)g->over=2;}else if(type==AK_EV_CHAR&&code>='a'&&code<='z'&&g->wn<6){g->word[g->wn++]=(char)(code-32);g->word[g->wn]=0;}else if(type==AK_EV_CHAR&&code>='A'&&code<='Z'&&g->wn<6){g->word[g->wn++]=(char)code;g->word[g->wn]=0;}if(code=='r'||code=='R')reset_game(app);wm_damage_win(win);return 1;}
    if(type==AK_EV_KEY_DOWN||type==AK_EV_CHAR){key_game(app,g,code);wm_damage_win(win);return 1;}
    if(type==AK_EV_MOUSE&&(code&1)){click_board(app,g,win,px,py);wm_damage_win(win);return 1;}
    return type==AK_EV_MOUSE;
}

static int blocked_tetris(struct game*g,int nx,int ny){for(int yy=0;yy<2;yy++)for(int xx=0;xx<2;xx++){int x=nx+xx,y=ny+yy;if(x<0||x>=10||y>=18||(y>=0&&g->board[y*10+x]))return 1;}return 0;}
static void tick_tetris(struct game*g){if(!blocked_tetris(g,g->x,g->y+1)){g->y++;return;}for(int yy=0;yy<2;yy++)for(int xx=0;xx<2;xx++)if(g->y+yy>=0)g->board[(g->y+yy)*10+g->x+xx]=1+(g->score%7);for(int y=17;y>=0;y--){int full=1;for(int x=0;x<10;x++)if(!g->board[y*10+x])full=0;if(full){for(int yy=y;yy>0;yy--)for(int x=0;x<10;x++)g->board[yy*10+x]=g->board[(yy-1)*10+x];for(int x=0;x<10;x++)g->board[x]=0;g->score+=100;y++;}}g->x=3;g->y=0;if(blocked_tetris(g,g->x,g->y))reset_game(APP_G_TETRIS);}
static void tick_life(struct game*g){int n[192];for(int y=0;y<12;y++)for(int x=0;x<16;x++){int c=0;for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++)if((dx||dy)&&inside(x+dx,0,16)&&inside(y+dy,0,12))c+=g->board[(y+dy)*16+x+dx];n[y*16+x]=c==3||(c==2&&g->board[y*16+x]);}for(int i=0;i<192;i++)g->board[i]=n[i];}

int refgame_tick(int app,int win)
{
    struct game*g=gs(app);int now=(int)idt_ticks(),rate=8;if(app==APP_G_TETRIS)rate=14;if(app==APP_G_LIFE)rate=12;if(now-g->last<rate)return 0;g->last=now;if(g->paused)return 0;
    if(app==APP_G_TETRIS)tick_tetris(g);
    else if(app==APP_G_PONG){g->x+=g->dx;g->y+=g->dy;if(g->y<1||g->y>98)g->dy=-g->dy;if(g->x<4){if(g->y>g->a-12&&g->y<g->a+30)g->dx=-g->dx;else{g->score--;g->x=50;}}if(g->x>96){g->dx=-g->dx;g->score++;}g->b+=(g->y-g->b)/4;}
    else if(app==APP_G_BREAKOUT){g->x+=g->dx;g->y+=g->dy;if(g->x<1||g->x>98)g->dx=-g->dx;if(g->y<2)g->dy=-g->dy;if(g->y>88){if(g->x>g->a-12&&g->x<g->a+24)g->dy=-g->dy;else{g->x=50;g->y=70;g->score--;}}if(g->y<32){int col=g->x/10,row=g->y/8;if(inside(col,0,10)&&inside(row,0,4)&&g->board[row*10+col]){g->board[row*10+col]=0;g->dy=-g->dy;g->score+=10;}}}
    else if(app==APP_G_LIFE)tick_life(g);
    else if(app==APP_G_ASTEROIDS){for(int i=1;i<12;i+=2)g->board[i]=(g->board[i]+1+i)%95+2;}
    else if(app==APP_G_INVADERS){g->a+=g->dx;if(g->a>8||g->a<-8)g->dx=-g->dx;}
    else if(app==APP_G_FLAPPY){g->dy++;g->y+=g->dy;g->a-=3;if(g->a<0){g->a=100;g->b=25+(int)(ak_xorshift()%50);g->score++;}if(g->y<0||g->y>95||(g->a>18&&g->a<28&&(g->y<g->b-12||g->y>g->b+12)))reset_game(app);}
    else if(app==APP_G_MISSILE){g->a+=5;if(g->a>92){g->a=0;g->score--;}}
    else if(app==APP_G_FROGGER){g->a=(g->a+3)%100;}
    else return 0;
    (void)win;return 1;
}
