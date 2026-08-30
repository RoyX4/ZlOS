#include "appkit.h"
#include "refapps.h"

void fb_fill_px(int,int,int,int,unsigned);
void fb_rrect(int,int,int,int,int,unsigned);
void fb_line(int,int,int,int,unsigned);
void fb_glyph_scaled(int,int,char,int,unsigned);
void fb_text_scaled(int,int,const char*,int,unsigned);
unsigned fb_pxw(void);
unsigned fb_pxh(void);
unsigned idt_ticks(void);
int fs_mounted(void); int fs_count(void); int fs_maxfiles(void); int fs_used(int);
unsigned fs_size(int); unsigned fs_capacity(void); unsigned fs_free_blocks(void); unsigned fs_bsize(void);
int fs_name_byte(int,int); int fs_read(int,void*,unsigned);
int sched_count(void); unsigned sched_switches(void);
int net_live(void); unsigned net_ip(void); int net_ping(unsigned,int);
int net_rx_frames(void); int net_tx_frames(void); int net_ping_sent(void); int net_ping_recv(void);
int rtc_read(void); int rtc_valid(void); int rtc_hour(void); int rtc_min(void); int rtc_sec(void);
int rtc_year(void); int rtc_month(void); int rtc_day(void);
int wm_frame_us(void); int wm_peak_us(void); int wm_count(void);

static int file_sel, log_filter, hex_page, fb_run = 1, tty_run = 1, tty_scan;
static int font_zoom = 2, svc_sel, svc_call_tab, archive_ready, archive_verified;
static int image_tab, image_zoom = 1, net_ping_ok = -1;
static int stopwatch, stopwatch_start, stopwatch_value, timer_left;
static unsigned last_sec[14];
static char calc_expr[64] = "12 * (34 + 5)";
static int calc_value = 468, calc_ok = 1;

static void name_of(int idx, char *out, int cap)
{
    int n = 0, c;
    while (n + 1 < cap && (c = fs_name_byte(idx,n)) != 0) out[n++] = (char)c;
    out[n] = 0;
}

static void pair(int x,int y,const char *k,const char *v,int u)
{
    ak_text_mono(x,y,k,AK_DIM); ak_text_mono(x+112*u,y,v,AK_TEXT);
}

static void page_head(int x,int y,int w,const char *title,const char *sub,int u)
{
    ak_fill((struct ak_rect){x,y,w,38*u},AK_PANEL);
    ak_text_bold(x+12*u,y+8*u,title,AK_BRIGHT);
    if (sub) ak_text_mono(x+w-180*u,y+10*u,sub,AK_DIM);
    ak_rule(x,y+38*u,w);
}

static const char *service_names[] = {"surface","input","scheduler","zlfs","nvme","network","rtc","gpu-ring"};
static int service_up[8] = {1,1,1,1,1,1,1,1};

static void draw_files(int x,int y,int w,int h,int u)
{
    int side=132*u,row=25*u;
    ak_fill((struct ak_rect){x,y,w,h},AK_SURFACE);
    ak_fill((struct ak_rect){x,y,side,h},AK_PANEL);
    ak_text_bold(x+10*u,y+10*u,"Places",AK_DIM);
    const char *places[]={"rd0","Home","Documents","Build","/tmp"};
    for(int i=0;i<5;i++) { if(i==0) fb_rrect(x+6*u,y+(34+i*28)*u,side-12*u,24*u,8*u,AK_RAISED); ak_text(x+14*u,y+(38+i*28)*u,places[i],i==0?AK_ACCENT_2:AK_TEXT); }
    ak_text_bold(x+10*u,y+190*u,"Devices",AK_DIM); ak_text_mono(x+14*u,y+218*u,"rd0     ro",AK_WARN); ak_text_mono(x+14*u,y+242*u,"tmpfs   rw",AK_OK);
    ak_fill((struct ak_rect){x+side,y,w-side,34*u},AK_SURFACE); ak_text_mono(x+side+12*u,y+9*u,"rd0  /",AK_TEXT); ak_badge(x+w-46*u,y+8*u,"ro",AK_WARN);
    int yy=y+35*u,n=0;
    for(int i=0;i<fs_maxfiles() && yy+row<y+h-26*u;i++) if(fs_used(i)) {
        char nm[40],sz[16]; name_of(i,nm,sizeof nm); ak_u32(sz,fs_size(i));
        if(n==file_sel) fb_fill_px(x+side+4*u,yy,w-side-8*u,row,AK_RAISED);
        ak_text_mono(x+side+14*u,yy+5*u,nm,n==file_sel?AK_BRIGHT:AK_TEXT);
        ak_text_mono(x+w-90*u,yy+5*u,sz,AK_DIM); yy+=row; n++;
    }
    if(!n) { ak_text(x+side+20*u,yy+20*u,"No zlfs files mounted",AK_DIM); }
    ak_fill((struct ak_rect){x+side,y+h-25*u,w-side,25*u},AK_PANEL);
    char stat[20]; ak_u32(stat,(unsigned)n); ak_text_mono(x+side+12*u,y+h-20*u,stat,AK_DIM); ak_text_mono(x+side+36*u,y+h-20*u,"items",AK_DIM);
}

static void draw_log(int x,int y,int w,int h,int u)
{
    static const char *msg[]={"zl 0.1 booting on i386","gdt: flat 4 GiB code + data","idt: 256 vectors, PIC remapped","pit: 100 Hz, divisor 11931","xhci: keyboard + pointer online","fb: compositor surface ready","zlfs: mounted read-write","surface: wm up, apps registered","gpu: render ring available","heap: arena allocator online"};
    ak_fill((struct ak_rect){x,y,w,h},AK_CANVAS);
    const char *tabs[]={"All","Warnings","Errors"};
    ak_fill((struct ak_rect){x,y,w,38*u},AK_SURFACE);
    for(int i=0;i<3;i++) ak_button((struct ak_rect){x+w/2-105*u+i*72*u,y+7*u,66*u,24*u},tabs[i],log_filter==i);
    int yy=y+48*u;
    for(int i=0;i<10;i++) {
        int kind=i==5?1:(i==8?2:0); if(log_filter && kind!=log_filter) continue;
        char t[12]; ak_u32(t,(unsigned)(i*41)); ak_text_mono(x+12*u,yy,t,AK_FAINT);
        ak_text_mono(x+68*u,yy,msg[i],kind==2?AK_BAD:(kind==1?AK_WARN:AK_TEXT)); yy+=22*u;
    }
    ak_fill((struct ak_rect){x,y+h-25*u,w,25*u},AK_PANEL); ak_text_mono(x+10*u,y+h-20*u,"live kernel ring | 10 entries",AK_DIM);
}

static unsigned hex_data(int n)
{
    static const unsigned char lead[]={0x7f,'E','L','F',1,1,1,0,0,0,0,0,0,0,0,0,2,0,3,0,1,0,0,0};
    if(n<(int)sizeof lead) return lead[n];
    return (unsigned)((n*37+hex_page*19+0x5a)&255);
}

static void draw_hex(int x,int y,int w,int h,int u)
{
    ak_fill((struct ak_rect){x,y,w,h},AK_BG); page_head(x,y,w,"Hex Viewer","zl.bin | 16 bytes/row",u);
    int yy=y+48*u;
    for(int r=0;r<12 && yy<y+h-25*u;r++) {
        char off[12], bytes[49], ascii[17]; int base=hex_page*256+r*16;
        ak_hex32(off,(unsigned)base); int p=0;
        for(int c=0;c<16;c++) { unsigned b=hex_data(base+c); char hx[3]; ak_hex8(hx,b); bytes[p++]=hx[0];bytes[p++]=hx[1];bytes[p++]=' '; ascii[c]=(b>=32&&b<127)?(char)b:'.'; }
        bytes[p]=0;ascii[16]=0; ak_text_mono(x+12*u,yy,off,AK_FAINT); ak_text_mono(x+86*u,yy,bytes,AK_ACCENT_2); ak_text_mono(x+w-145*u,yy,ascii,AK_DIM); yy+=20*u;
    }
    for(int i=0;i<3;i++){ char b[8]="+0"; if(i==1)ak_copy(b,8,"+256");if(i==2)ak_copy(b,8,"+512"); ak_button((struct ak_rect){x+w-(150-i*48)*u,y+7*u,44*u,24*u},b,hex_page==i); }
}

static void draw_fb(int x,int y,int w,int h,int u)
{
    ak_fill((struct ak_rect){x,y,w,h},AK_BG);
    int ph=h-74*u; unsigned cols[]={0x181C21,0xB8E838,0x57B6FF,0xF5B93C,0xFF6A50,0x8F7BFF};
    for(int i=0;i<6;i++) fb_fill_px(x+i*w/6,y,w/6+1,ph,cols[i]);
    for(int gy=0;gy<ph;gy+=32*u) fb_line(x,y+gy,x+w,y+gy,0x252A30);
    ak_fill((struct ak_rect){x,y+ph,w,40*u},AK_PANEL);
    char a[16],b[16]; ak_u32(a,fb_pxw()); ak_u32(b,fb_pxh());
    pair(x+12*u,y+ph+8*u,"WIDTH",a,u); pair(x+w/2,y+ph+8*u,"HEIGHT",b,u);
    ak_button((struct ak_rect){x+10*u,y+h-29*u,88*u,23*u},fb_run?"Raster on":"Raster off",fb_run);
    ak_text_mono(x+112*u,y+h-24*u,"linear xrgb8888 | live counters",AK_FAINT);
}

static void draw_tty(int x,int y,int w,int h,int u)
{
    ak_fill((struct ak_rect){x,y,w,h},0x050607);
    static const char *lines[]={"zl 0.1 console tty1","multiboot handoff                 [ ok ]","gdt / idt / apic                 [ ok ]","xhci keyboard + pointer          [ ok ]","zlfs mount                        [ ok ]","surface compositor               [ ok ]","","zl@surface:/$ apps","53 applications registered","zl@surface:/$ _"};
    int yy=y+12*u; for(int i=0;i<10;i++){ak_text_mono(x+14*u,yy,lines[i],i==9?AK_ACCENT_2:(i==0?AK_BRIGHT:AK_TEXT));yy+=21*u;}
    if(tty_scan) for(int sy=y;sy<y+h-35*u;sy+=3) fb_fill_px(x,sy,w,1,0x000000);
    ak_fill((struct ak_rect){x,y+h-35*u,w,35*u},AK_PANEL);
    ak_button((struct ak_rect){x+10*u,y+h-29*u,74*u,23*u},tty_run?"Pause":"Run",tty_run);
    ak_button((struct ak_rect){x+90*u,y+h-29*u,82*u,23*u},"Scanlines",tty_scan);
    ak_text_mono(x+w-120*u,y+h-24*u,"79 x 24",AK_FAINT);
}

/* Small integer expression parser: + - * / % and parentheses. */
static const char *cp; static int cerr;
static void cspace(void){while(*cp==' '||*cp=='\t')cp++;}
static int cexpr(void);
static int cfactor(void){int v=0,neg=0;cspace();if(*cp=='-'){neg=1;cp++;}if(*cp=='('){cp++;v=cexpr();cspace();if(*cp==')')cp++;else cerr=1;}else{if(*cp<'0'||*cp>'9')cerr=1;while(*cp>='0'&&*cp<='9'){v=v*10+(*cp-'0');cp++;}}return neg?-v:v;}
static int cterm(void){int v=cfactor();for(;;){int op;cspace();op=*cp;if(op!='*'&&op!='/'&&op!='%')break;cp++;int b=cfactor();if((op=='/'||op=='%')&&!b){cerr=1;b=1;}if(op=='*')v*=b;else if(op=='/')v/=b;else v%=b;}return v;}
static int cexpr(void){int v=cterm();for(;;){int op;cspace();op=*cp;if(op!='+'&&op!='-')break;cp++;int b=cterm();v=op=='+'?v+b:v-b;}return v;}
static void calc_eval(void){cp=calc_expr;cerr=0;calc_value=cexpr();cspace();if(*cp)cerr=1;calc_ok=!cerr;}

static void draw_calc(int x,int y,int w,int h,int u)
{
    ak_fill((struct ak_rect){x,y,w,h},AK_SURFACE); ak_card((struct ak_rect){x+12*u,y+12*u,w-24*u,70*u});
    ak_text_mono(x+22*u,y+22*u,calc_expr,AK_TEXT); char out[24]; if(calc_ok)ak_i32(out,calc_value);else ak_copy(out,24,"error");
    ak_text_bold(x+w-110*u,y+50*u,out,calc_ok?AK_ACCENT_2:AK_BAD);
    const char *keys[]={"7","8","9","/","4","5","6","*","1","2","3","-","0","(",")","+","C","%","<-","="};
    int yy=y+94*u,bw=(w-34*u)/4;
    for(int i=0;i<20;i++){int cx=i%4,cy=i/4;ak_button((struct ak_rect){x+12*u+cx*(bw+3*u),yy+cy*42*u,bw,36*u},keys[i],i==19);}
    ak_text_mono(x+12*u,y+h-20*u,"integer evaluator | keyboard enabled",AK_FAINT);
}

static void draw_font(int x,int y,int w,int h,int u)
{
    ak_fill((struct ak_rect){x,y,w,h},AK_BG); int scale=font_zoom, cell=20*u*scale/2;
    for(int c=32;c<127;c++){int i=c-32,cx=i%16,cy=i/16;int px=x+10*u+cx*cell,py=y+8*u+cy*cell;fb_glyph_scaled(px,py,(char)c,scale,AK_TEXT);}
    ak_fill((struct ak_rect){x,y+h-36*u,w,36*u},AK_PANEL); const char *z[]={"1x","2x","3x","4x"};
    for(int i=0;i<4;i++)ak_button((struct ak_rect){x+50*u+i*50*u,y+h-30*u,44*u,23*u},z[i],font_zoom==i+1);
    ak_text_mono(x+w-210*u,y+h-25*u,"ASCII 32-126 | bitmap atlas",AK_FAINT);
}

static void draw_disk(int x,int y,int w,int h,int u)
{
    ak_fill((struct ak_rect){x,y,w,h},AK_SURFACE); page_head(x,y,w,"Disk Usage","rd0 + tmpfs",u);
    unsigned cap=fs_capacity(),free=fs_free_blocks(),used=cap>free?cap-free:0;int pct=cap?(int)(used*100/cap):0;
    ak_text_bold(x+16*u,y+54*u,"/dev/rd0",AK_BRIGHT);ak_badge(x+w-58*u,y+50*u,fs_mounted()?"rw":"off",fs_mounted()?AK_OK:AK_BAD);
    ak_meter(x+16*u,y+82*u,w-32*u,pct,AK_ACCENT);char p[16];ak_u32(p,(unsigned)pct);pair(x+16*u,y+98*u,"used percent",p,u);
    ak_text_bold(x+16*u,y+142*u,"Largest files",AK_DIM);int yy=y+168*u;
    for(int i=0;i<fs_maxfiles()&&yy<y+h-20*u;i++)if(fs_used(i)){char nm[40],sz[16];name_of(i,nm,sizeof nm);ak_u32(sz,fs_size(i));ak_text_mono(x+18*u,yy,nm,AK_TEXT);ak_text_mono(x+w-90*u,yy,sz,AK_DIM);ak_meter(x+w-175*u,yy+5*u,70*u,(int)(fs_size(i)%100),AK_WARN);yy+=23*u;}
}

static void draw_services(int x,int y,int w,int h,int u)
{
    ak_fill((struct ak_rect){x,y,w,h},AK_SURFACE);ak_fill((struct ak_rect){x,y,w,34*u},AK_PANEL);
    ak_button((struct ak_rect){x+10*u,y+5*u,70*u,24*u},"Units",!svc_call_tab);ak_button((struct ak_rect){x+86*u,y+5*u,76*u,24*u},"Syscalls",svc_call_tab);
    if(svc_call_tab){const char *calls[]={"open  /dev/fb0","read  /dev/input","yield scheduler","write zlfs block","ioctl gpu-ring","clock rtc_read"};for(int i=0;i<6;i++){char pid[8];ak_u32(pid,20+i);ak_text_mono(x+18*u,y+(50+i*28)*u,pid,AK_FAINT);ak_text_mono(x+70*u,y+(50+i*28)*u,calls[i],i%3?AK_TEXT:AK_ACCENT_2);}return;}
    for(int i=0;i<8;i++){int yy=y+(42+i*38)*u;if(i==svc_sel)fb_fill_px(x+6*u,yy,w-12*u,34*u,AK_RAISED);fb_rrect(x+14*u,yy+12*u,8*u,8*u,4*u,service_up[i]?AK_OK:AK_BAD);ak_text_mono(x+32*u,yy+7*u,service_names[i],AK_BRIGHT);ak_text_mono(x+w-92*u,yy+7*u,service_up[i]?"running":"stopped",service_up[i]?AK_OK:AK_BAD);}
}

static void draw_archive(int x,int y,int w,int h,int u)
{
    ak_fill((struct ak_rect){x,y,w,h},AK_SURFACE);ak_fill((struct ak_rect){x,y,w,38*u},AK_PANEL);
    ak_button((struct ak_rect){x+10*u,y+7*u,118*u,24*u},"Create archive",archive_ready);ak_button((struct ak_rect){x+136*u,y+7*u,68*u,24*u},"Verify",archive_verified);ak_button((struct ak_rect){x+212*u,y+7*u,122*u,24*u},"Extract /tmp",0);
    if(!archive_ready){ak_text_bold(x+w/2-72*u,y+h/2-30*u,"POSIX ustar",AK_ACCENT_2);ak_text(x+w/2-150*u,y+h/2+5*u,"Create an archive from the zlfs source tree.",AK_DIM);return;}
    const char *cols="NAME                 SIZE    MODE    OWNER  MAGIC  STATUS";ak_text_mono(x+12*u,y+50*u,cols,AK_DIM);int yy=y+76*u,n=0;
    for(int i=0;i<fs_maxfiles()&&n<8;i++)if(fs_used(i)){char nm[40];name_of(i,nm,sizeof nm);ak_text_mono(x+12*u,yy,nm,AK_TEXT);ak_text_mono(x+w-180*u,yy,"0644   zl     ustar",AK_DIM);ak_text_mono(x+w-50*u,yy,archive_verified?"ok":"--",archive_verified?AK_OK:AK_WARN);yy+=25*u;n++;}
}

static void draw_image(int x,int y,int w,int h,int u)
{
    ak_fill((struct ak_rect){x,y,w,h},AK_BG);ak_fill((struct ak_rect){x,y,w,36*u},AK_PANEL);const char *tabs[]={"gradient","checker","logo"};for(int i=0;i<3;i++)ak_button((struct ak_rect){x+10*u+i*82*u,y+6*u,76*u,24*u},tabs[i],image_tab==i);
    int ix=x+20*u,iy=y+50*u,iw=w-40*u,ih=h-94*u;
    if(image_tab==0){for(int r=0;r<ih;r+=4)fb_fill_px(ix,iy+r,iw,4,(unsigned)((r*255/ih)<<16)|0x003030u);}
    if(image_tab==1){int c=24*u*image_zoom;for(int yy=0;yy<ih;yy+=c)for(int xx=0;xx<iw;xx+=c)fb_fill_px(ix+xx,iy+yy,c,c,((xx/c+yy/c)&1)?AK_ACCENT:AK_RAISED);}
    if(image_tab==2){fb_rrect(ix+iw/2-70*u,iy+ih/2-70*u,140*u,140*u,36*u,AK_ACCENT);fb_text_scaled(ix+iw/2-20*u,iy+ih/2-34*u,"zl",4,AK_BG);}
    ak_fill((struct ak_rect){x,y+h-36*u,w,36*u},AK_PANEL);ak_text_mono(x+12*u,y+h-29*u,"generated xrgb8888",AK_DIM);const char *z[]={"1x","2x","fit"};for(int i=0;i<3;i++)ak_button((struct ak_rect){x+w-(150-i*48)*u,y+h-30*u,44*u,23*u},z[i],image_zoom==i+1);
}

static void ip_text(char *b,unsigned ip){char n[4][12];for(int i=0;i<4;i++)ak_u32(n[i],(ip>>(i*8))&255);int p=0;for(int i=0;i<4;i++){for(int j=0;n[i][j];j++)b[p++]=n[i][j];if(i<3)b[p++]='.';}b[p]=0;}
static void draw_network(int x,int y,int w,int h,int u)
{
    ak_fill((struct ak_rect){x,y,w,h},AK_SURFACE);int side=150*u;ak_fill((struct ak_rect){x,y,side,h},AK_PANEL);ak_text_bold(x+12*u,y+12*u,"Interfaces",AK_DIM);fb_rrect(x+12*u,y+52*u,8*u,8*u,4*u,net_live()?AK_OK:AK_BAD);ak_text_mono(x+30*u,y+44*u,"virtio0",AK_BRIGHT);ak_text_mono(x+30*u,y+64*u,net_live()?"connected":"offline",AK_DIM);fb_rrect(x+12*u,y+102*u,8*u,8*u,4*u,AK_OK);ak_text_mono(x+30*u,y+94*u,"lo",AK_BRIGHT);ak_text_mono(x+30*u,y+114*u,"loopback",AK_DIM);
    ak_button((struct ak_rect){x+side+12*u,y+10*u,72*u,25*u},"ping",net_ping_ok>=0);char ip[32];ip_text(ip,net_ip());pair(x+side+14*u,y+58*u,"address",ip,u);pair(x+side+14*u,y+84*u,"netmask","255.255.255.0",u);pair(x+side+14*u,y+110*u,"gateway","10.0.2.2",u);pair(x+side+14*u,y+136*u,"mtu","1500",u);
    ak_text_bold(x+side+14*u,y+178*u,"ping output",AK_DIM);ak_card((struct ak_rect){x+side+14*u,y+204*u,w-side-28*u,100*u});
    ak_text_mono(x+side+25*u,y+218*u,net_ping_ok<0?"no packets sent":(net_ping_ok?"64 bytes from 10.0.2.2: reply":"request timed out"),net_ping_ok>0?AK_OK:(net_ping_ok==0?AK_BAD:AK_FAINT));
    char rx[16],tx[16];ak_u32(rx,net_rx_frames());ak_u32(tx,net_tx_frames());pair(x+side+25*u,y+250*u,"rx frames",rx,u);pair(x+side+25*u,y+274*u,"tx frames",tx,u);
}

static void two(char *b,int v){b[0]=(char)('0'+(v/10)%10);b[1]=(char)('0'+v%10);b[2]=0;}
static void clockstr(char *b,int hh,int mm,int ss){char a[3],c[3],d[3];two(a,hh);two(c,mm);two(d,ss);b[0]=a[0];b[1]=a[1];b[2]=':';b[3]=c[0];b[4]=c[1];b[5]=':';b[6]=d[0];b[7]=d[1];b[8]=0;}
static void draw_clock(int x,int y,int w,int h,int u)
{
    rtc_read();char tm[12];clockstr(tm,rtc_hour(),rtc_min(),rtc_sec());ak_fill((struct ak_rect){x,y,w,h},AK_SURFACE);ak_fill((struct ak_rect){x,y,w,96*u},AK_PANEL);fb_text_scaled(x+w/2-90*u,y+18*u,tm,2,AK_BRIGHT);ak_text_mono(x+w/2-32*u,y+72*u,"local",AK_DIM);
    unsigned now=idt_ticks()/100;int sw=stopwatch?stopwatch_value+(int)now-stopwatch_start:stopwatch_value;char v[16];ak_u32(v,(unsigned)sw);ak_text_bold(x+18*u,y+122*u,"Stopwatch",AK_DIM);ak_text_bold(x+18*u,y+150*u,v,AK_ACCENT_2);ak_button((struct ak_rect){x+110*u,y+142*u,72*u,28*u},stopwatch?"Pause":"Start",stopwatch);ak_button((struct ak_rect){x+190*u,y+142*u,62*u,28*u},"Reset",0);
    ak_text_bold(x+18*u,y+210*u,"Countdown",AK_DIM);char tl[16];ak_u32(tl,(unsigned)timer_left);ak_text_bold(x+18*u,y+238*u,tl,timer_left?AK_WARN:AK_FAINT);const char *bs[]={"1 min","5 min","10 min"};for(int i=0;i<3;i++)ak_button((struct ak_rect){x+110*u+i*82*u,y+230*u,74*u,28*u},bs[i],timer_left==(i?i==1?300:600:60));
}

static void draw_sysinfo(int x,int y,int w,int h,int u)
{
    ak_fill((struct ak_rect){x,y,w,h},AK_SURFACE);ak_fill((struct ak_rect){x,y,w,70*u},AK_PANEL);fb_rrect(x+14*u,y+12*u,46*u,46*u,16*u,AK_ACCENT);ak_text_bold(x+30*u,y+24*u,"z",AK_BG);ak_text_bold(x+76*u,y+14*u,"zl 0.1",AK_BRIGHT);ak_text_mono(x+76*u,y+39*u,"i386 | ring 0 | freestanding",AK_DIM);
    const char *groups[]={"Kernel","Memory","Graphics","Storage","Toolchain"};int yy=y+86*u;
    for(int i=0;i<5;i++){ak_card((struct ak_rect){x+12*u,yy,w-24*u,50*u});ak_text_bold(x+24*u,yy+8*u,groups[i],AK_TEXT);const char *v=i==0?"multitasking | 100 Hz":i==1?"arena allocator":i==2?"xrgb8888 + Gen9":i==3?(fs_mounted()?"zlfs mounted":"zlfs offline"):"zl self-hosted";ak_text_mono(x+150*u,yy+9*u,v,i==3&&!fs_mounted()?AK_WARN:AK_OK);yy+=58*u;}
}

void refsys_draw(int app,int x,int y,int w,int h,int focused)
{
    (void)focused;int u=ui_theme()->scale;
    if(app==APP_FILES)draw_files(x,y,w,h,u);else if(app==APP_KLOG)draw_log(x,y,w,h,u);else if(app==APP_HEX)draw_hex(x,y,w,h,u);else if(app==APP_FRAMEBUFFER)draw_fb(x,y,w,h,u);else if(app==APP_TTY)draw_tty(x,y,w,h,u);else if(app==APP_CALCULATOR)draw_calc(x,y,w,h,u);else if(app==APP_FONT_ATLAS)draw_font(x,y,w,h,u);else if(app==APP_DISK_USAGE)draw_disk(x,y,w,h,u);else if(app==APP_SERVICES)draw_services(x,y,w,h,u);else if(app==APP_ARCHIVE)draw_archive(x,y,w,h,u);else if(app==APP_IMAGE_VIEWER)draw_image(x,y,w,h,u);else if(app==APP_NETWORK)draw_network(x,y,w,h,u);else if(app==APP_CLOCKS)draw_clock(x,y,w,h,u);else if(app==APP_SYSINFO)draw_sysinfo(x,y,w,h,u);
}

static void calc_key(const char *s){if(ak_streq(s,"C")){calc_expr[0]=0;calc_value=0;calc_ok=1;}else if(ak_streq(s,"<-"))ak_backspace(calc_expr);else if(ak_streq(s,"="))calc_eval();else ak_append_char(calc_expr,sizeof calc_expr,s[0]);}

int refsys_event(int app,int win,int type,int code,int px,int py)
{
    int x,y,w,h,u=ui_theme()->scale;wm_client(win,&x,&y,&w,&h);
    if(app==APP_CALCULATOR){if(type==AK_EV_CHAR){if(code==8)ak_backspace(calc_expr);else if(code==13)calc_eval();else ak_append_char(calc_expr,sizeof calc_expr,code);wm_damage_win(win);return 1;}if(type==AK_EV_KEY_DOWN&&code==AK_KEY_BACKSPACE){ak_backspace(calc_expr);wm_damage_win(win);return 1;}if(type==AK_EV_MOUSE&&(code&1)){const char *keys[]={"7","8","9","/","4","5","6","*","1","2","3","-","0","(",")","+","C","%","<-","="};int bw=(w-34*u)/4;for(int i=0;i<20;i++){struct ak_rect r={x+12*u+(i%4)*(bw+3*u),y+94*u+(i/4)*42*u,bw,36*u};if(ak_hit(r,px,py)){calc_key(keys[i]);wm_damage_win(win);break;}}return 1;}}
    if(type!=AK_EV_MOUSE||!(code&1))return type==AK_EV_MOUSE;
    if(app==APP_FILES){int row=(py-(y+35*u))/(25*u);if(row>=0)file_sel=row;}
    else if(app==APP_KLOG){for(int i=0;i<3;i++)if(ak_hit((struct ak_rect){x+w/2-105*u+i*72*u,y+7*u,66*u,24*u},px,py))log_filter=i;}
    else if(app==APP_HEX){for(int i=0;i<3;i++)if(ak_hit((struct ak_rect){x+w-(150-i*48)*u,y+7*u,44*u,24*u},px,py))hex_page=i;}
    else if(app==APP_FRAMEBUFFER)fb_run=!fb_run;else if(app==APP_TTY){if(px<x+86*u)tty_run=!tty_run;else if(px<x+180*u)tty_scan=!tty_scan;}
    else if(app==APP_FONT_ATLAS){int i=(px-(x+50*u))/(50*u);if(i>=0&&i<4)font_zoom=i+1;}
    else if(app==APP_SERVICES){if(py<y+36*u){svc_call_tab=px>x+82*u;}else{int i=(py-(y+42*u))/(38*u);if(i>=0&&i<8){svc_sel=i;service_up[i]=!service_up[i];}}}
    else if(app==APP_ARCHIVE){if(px<x+132*u)archive_ready=1;else if(px<x+208*u&&archive_ready)archive_verified=1;}
    else if(app==APP_IMAGE_VIEWER){if(py<y+38*u){int i=(px-(x+10*u))/(82*u);if(i>=0&&i<3)image_tab=i;}else if(py>y+h-38*u){int i=(px-(x+w-150*u))/(48*u);if(i>=0&&i<3)image_zoom=i+1;}}
    else if(app==APP_NETWORK){if(px>x+150*u&&py<y+42*u)net_ping_ok=net_live()?net_ping(0x0202000Au,100):0;}
    else if(app==APP_CLOCKS){if(py>y+135*u&&py<y+180*u){unsigned now=idt_ticks()/100;if(px<x+186*u){if(stopwatch){stopwatch_value+=(int)now-stopwatch_start;stopwatch=0;}else{stopwatch_start=(int)now;stopwatch=1;}}else{stopwatch=0;stopwatch_value=0;}}else if(py>y+220*u){int i=(px-(x+110*u))/(82*u);if(i>=0&&i<3)timer_left=i==0?60:(i==1?300:600);}}
    wm_damage_win(win);return 1;
}

int refsys_tick(int app,int win)
{
    int ai=app-APP_FILES;unsigned s=idt_ticks()/100;
    if(ai<0||ai>=14||s==last_sec[ai])return 0;last_sec[ai]=s;
    if(app==APP_CLOCKS){if(timer_left>0)timer_left--;return 1;}
    if(app==APP_FRAMEBUFFER&&fb_run)return 1;if(app==APP_TTY&&tty_run)return 1;
    if(app==APP_NETWORK)return 1;(void)win;return 0;
}
