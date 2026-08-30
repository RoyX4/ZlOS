#include "appkit.h"
#include "refapps.h"

void fb_fill_px(int,int,int,int,unsigned);
void fb_rrect(int,int,int,int,int,unsigned);
void fb_line(int,int,int,int,unsigned);
unsigned idt_ticks(void);
unsigned clip_len(void); int clip_type(void); unsigned clip_seq(void); int clip_byte(int); void clip_clear(void);

static char colour[16]="#b8e838";
static char regex_pat[40]="zl.*";
static char regex_text[96]="zl kernel | zlOS | kernel";
static char base_text[32]="1193182";
static char diff_a[80]="surface\ninput\nscheduler";
static char diff_b[80]="surface\nnetwork\nscheduler";
static char hash_text[96]="zlOS";
static char unit_text[32]="1024";
static char note_text[160]="apps live in the kernel, not in a browser";
static int unit_from=1, diff_field, regex_field;
static char key_last[32]="press any key"; static int key_count;
static unsigned bench_int,bench_hash,bench_raster; static int bench_done;

static void head(int x,int y,int w,const char *title,const char *note,int u)
{
    ak_fill((struct ak_rect){x,y,w,38*u},AK_PANEL);ak_text_bold(x+12*u,y+8*u,title,AK_BRIGHT);if(note)ak_text_mono(x+w-180*u,y+10*u,note,AK_DIM);ak_rule(x,y+38*u,w);
}

static void input(int x,int y,int w,const char *label,const char *value,int active,int u)
{
    ak_text_mono(x,y+6*u,label,AK_DIM);ak_card((struct ak_rect){x+76*u,y,w-76*u,28*u});ak_text_mono(x+86*u,y+6*u,value,active?AK_ACCENT_2:AK_TEXT);
}

static void row(int x,int y,int w,const char *k,const char *v,unsigned col,int u)
{
    ak_text_mono(x,y,k,AK_DIM);ak_text_mono(x+112*u,y,v,col);ak_rule(x,y+20*u,w);
}

static unsigned parse_num(const char *s,int *ok)
{
    unsigned v=0;int base=10,i=0,d,n=0;*ok=1;
    if(s[0]=='0'&&(s[1]=='x'||s[1]=='X')){base=16;i=2;}else if(s[0]=='0'&&(s[1]=='b'||s[1]=='B')){base=2;i=2;}else if(s[0]=='0'&&(s[1]=='o'||s[1]=='O')){base=8;i=2;}
    for(;s[i];i++){int c=s[i];if(c>='0'&&c<='9')d=c-'0';else if(c>='a'&&c<='f')d=c-'a'+10;else if(c>='A'&&c<='F')d=c-'A'+10;else{*ok=0;break;}if(d>=base){*ok=0;break;}v=v*(unsigned)base+(unsigned)d;n++;}
    if(!n)*ok=0;return v;
}

static void binary(char *b,unsigned v)
{
    int started=0,p=0;for(int i=31;i>=0;i--){int bit=(v>>i)&1;if(bit)started=1;if(started||i==0)b[p++]=(char)('0'+bit);}b[p]=0;
}

static int match_here(const char *p,const char *s)
{
    if(!*p)return 1;
    if(p[1]=='*'){do{if(match_here(p+2,s))return 1;if(!*s||(*p!='.'&&*p!=*s))break;s++;}while(1);return 0;}
    if(!*s)return 0;if(*p=='.'||*p==*s)return match_here(p+1,s+1);return 0;
}
static int regex_count(void){int n=0;for(int i=0;regex_text[i];i++)if(match_here(regex_pat,regex_text+i))n++;return n;}

static void draw_clip(int x,int y,int w,int h,int u)
{
    head(x,y,w,"Clipboard","shared kernel buffer",u);char len[16],seq[16],preview[65];ak_u32(len,clip_len());ak_u32(seq,clip_seq());int n=(int)clip_len();if(n>64)n=64;for(int i=0;i<n;i++){int c=clip_byte(i);preview[i]=c>=32&&c<127?(char)c:'.';}preview[n]=0;
    row(x+14*u,y+56*u,w-28*u,"state",clip_type()?"text":"empty",clip_type()?AK_OK:AK_FAINT,u);row(x+14*u,y+84*u,w-28*u,"bytes",len,AK_TEXT,u);row(x+14*u,y+112*u,w-28*u,"sequence",seq,AK_TEXT,u);row(x+14*u,y+140*u,w-28*u,"preview",preview[0]?preview:"--",AK_ACCENT_2,u);ak_button((struct ak_rect){x+14*u,y+184*u,82*u,26*u},"Clear",0);ak_text(x+14*u,y+h-48*u,"Ctrl+C and Ctrl+V in every app use this one buffer.",AK_DIM);
}

static int hexval(int c){if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;if(c>='A'&&c<='F')return c-'A'+10;return -1;}
static unsigned colour_rgb(int *ok){int off=colour[0]=='#';if(ak_strlen(colour+off)<6){*ok=0;return 0;}unsigned v=0;for(int i=0;i<6;i++){int d=hexval(colour[off+i]);if(d<0){*ok=0;return 0;}v=(v<<4)|(unsigned)d;}*ok=1;return v;}
static void draw_colour(int x,int y,int w,int h,int u)
{
    head(x,y,w,"Colour Picker","xrgb8888",u);input(x+14*u,y+52*u,w-28*u,"hex",colour,1,u);int ok;unsigned c=colour_rgb(&ok);fb_rrect(x+14*u,y+94*u,w-28*u,62*u,12*u,ok?c:AK_BAD);char hx[12],r[12],g[12],b[12],rgb[48];ak_hex32(hx,c);ak_u32(r,(c>>16)&255);ak_u32(g,(c>>8)&255);ak_u32(b,c&255);int p=0;const char *parts[]={r,", ",g,", ",b};for(int q=0;q<5;q++)for(int j=0;parts[q][j];j++)rgb[p++]=parts[q][j];rgb[p]=0;row(x+14*u,y+174*u,w-28*u,"hex",hx,ok?AK_ACCENT_2:AK_BAD,u);row(x+14*u,y+202*u,w-28*u,"rgb",rgb,AK_TEXT,u);char rgb565[12];unsigned v=((c>>19)<<11)|(((c>>10)&63)<<5)|((c>>3)&31);ak_hex32(rgb565,v);row(x+14*u,y+230*u,w-28*u,"16-bit",rgb565,AK_WARN,u);row(x+14*u,y+258*u,w-28*u,"contrast",((c&0xFFFFFF)>0x7F7F7F)?"dark text":"light text",AK_OK,u);
}

static void draw_regex(int x,int y,int w,int h,int u)
{
    head(x,y,w,"Regex Tester",". and *",u);input(x+14*u,y+52*u,w-28*u,"pattern",regex_pat,regex_field==0,u);input(x+14*u,y+88*u,w-28*u,"subject",regex_text,regex_field==1,u);char n[16];ak_u32(n,(unsigned)regex_count());row(x+14*u,y+136*u,w-28*u,"matches",n,regex_count()?AK_OK:AK_FAINT,u);row(x+14*u,y+164*u,w-28*u,"engine","kernel NFA subset",AK_TEXT,u);ak_text(x+14*u,y+214*u,"Click a field, then type. Supports literal characters, dot and star.",AK_DIM);
}

static void draw_base(int x,int y,int w,int h,int u)
{
    head(x,y,w,"Base Converter","integer",u);input(x+14*u,y+52*u,w-28*u,"value",base_text,1,u);int ok;unsigned v=parse_num(base_text,&ok);char dec[16],hex[12],bin[40],oct[16];ak_u32(dec,v);ak_hex32(hex,v);binary(bin,v);if(ok){unsigned t=v;char rev[16];int n=0;if(!t)rev[n++]='0';while(t){rev[n++]=(char)('0'+t%8);t/=8;}for(int i=0;i<n;i++)oct[i]=rev[n-i-1];oct[n]=0;}else ak_copy(oct,16,"error");row(x+14*u,y+98*u,w-28*u,"decimal",ok?dec:"error",ok?AK_TEXT:AK_BAD,u);row(x+14*u,y+126*u,w-28*u,"hex",ok?hex:"--",AK_ACCENT_2,u);row(x+14*u,y+154*u,w-28*u,"octal",ok?oct:"--",AK_TEXT,u);row(x+14*u,y+182*u,w-28*u,"binary",ok?bin:"--",AK_TEXT,u);char pages[20];ak_u32(pages,(v+4095)/4096);row(x+14*u,y+210*u,w-28*u,"4K pages",pages,AK_WARN,u);
}

static int line_count(const char *s){int n=1;for(int i=0;s[i];i++)if(s[i]=='\n')n++;return n;}
static int diff_same(void){int i=0,j=0,same=0;while(diff_a[i]||diff_b[j]){int eq=1;while(diff_a[i]!='\n'&&diff_a[i]&&diff_b[j]!='\n'&&diff_b[j]){if(diff_a[i++]!=diff_b[j++])eq=0;}if((diff_a[i]=='\n'||!diff_a[i])&&(diff_b[j]=='\n'||!diff_b[j])&&eq)same++;while(diff_a[i]&&diff_a[i]!='\n')i++;while(diff_b[j]&&diff_b[j]!='\n')j++;if(diff_a[i]=='\n')i++;if(diff_b[j]=='\n')j++;}return same;}
static void draw_diff(int x,int y,int w,int h,int u)
{
    head(x,y,w,"Text Diff","line by line",u);input(x+14*u,y+52*u,w-28*u,"a",diff_a,diff_field==0,u);input(x+14*u,y+88*u,w-28*u,"b",diff_b,diff_field==1,u);char same[16],total[16];ak_u32(same,(unsigned)diff_same());int n=line_count(diff_a);if(line_count(diff_b)>n)n=line_count(diff_b);ak_u32(total,(unsigned)n);row(x+14*u,y+136*u,w-28*u,"unchanged",same,AK_OK,u);row(x+14*u,y+164*u,w-28*u,"total lines",total,AK_TEXT,u);row(x+14*u,y+192*u,w-28*u,"- a","input",AK_BAD,u);row(x+14*u,y+220*u,w-28*u,"+ b","input",AK_OK,u);ak_text(x+14*u,y+260*u,"Use | as a newline while typing in either field.",AK_DIM);
}

static unsigned djb2(const char *s){unsigned h=5381;while(*s)h=((h<<5)+h)+(unsigned char)*s++;return h;}
static unsigned adler(const char *s){unsigned a=1,b=0;while(*s){a=(a+(unsigned char)*s++)%65521;b=(b+a)%65521;}return(b<<16)|a;}
static void draw_hash(int x,int y,int w,int h,int u)
{
    head(x,y,w,"Checksum","four real digests",u);input(x+14*u,y+52*u,w-28*u,"input",hash_text,1,u);char n[16],a[12],b[12],c[12],d[12];ak_u32(n,(unsigned)ak_strlen(hash_text));ak_hex32(a,ak_fnv1a(hash_text));ak_hex32(b,djb2(hash_text));ak_hex32(c,adler(hash_text));ak_hex32(d,ak_crc32(hash_text));row(x+14*u,y+98*u,w-28*u,"length",n,AK_TEXT,u);row(x+14*u,y+126*u,w-28*u,"fnv-1a",a,AK_ACCENT_2,u);row(x+14*u,y+154*u,w-28*u,"djb2",b,AK_ACCENT_2,u);row(x+14*u,y+182*u,w-28*u,"adler-32",c,AK_ACCENT_2,u);row(x+14*u,y+210*u,w-28*u,"crc-32",d,AK_ACCENT_2,u);ak_text(x+14*u,y+254*u,"Computed in zlOS from each published algorithm.",AK_DIM);
}

static void draw_unit(int x,int y,int w,int h,int u)
{
    head(x,y,w,"Unit Converter","binary units",u);input(x+14*u,y+52*u,w-28*u,"value",unit_text,1,u);const char *units[]={"B","KiB","MiB","GiB"};for(int i=0;i<4;i++)ak_button((struct ak_rect){x+14*u+i*64*u,y+90*u,58*u,25*u},units[i],unit_from==i);int ok;unsigned v=parse_num(unit_text,&ok),mul=unit_from==0?1u:(unit_from==1?1024u:(unit_from==2?1048576u:1073741824u));unsigned bytes=v*mul;char b[16],k[16],m[16],pages[16];ak_u32(b,bytes);ak_u32(k,bytes/1024);ak_u32(m,bytes/1048576);ak_u32(pages,(bytes+4095)/4096);row(x+14*u,y+132*u,w-28*u,"bytes",ok?b:"error",ok?AK_TEXT:AK_BAD,u);row(x+14*u,y+160*u,w-28*u,"KiB",k,AK_ACCENT_2,u);row(x+14*u,y+188*u,w-28*u,"MiB",m,AK_TEXT,u);row(x+14*u,y+216*u,w-28*u,"4K pages",pages,AK_WARN,u);
}

static void draw_notes(int x,int y,int w,int h,int u)
{
    head(x,y,w,"Sticky Notes","/tmp/note.txt",u);ak_card((struct ak_rect){x+14*u,y+52*u,w-28*u,130*u});ak_text(x+26*u,y+66*u,note_text,AK_TEXT);ak_button((struct ak_rect){x+14*u,y+196*u,118*u,27*u},"Save to /tmp",1);ak_button((struct ak_rect){x+140*u,y+196*u,70*u,27*u},"Clear",0);char chars[16],words[16];ak_u32(chars,(unsigned)ak_strlen(note_text));int n=0,in=0;for(int i=0;note_text[i];i++){if(note_text[i]==' '||note_text[i]=='\n'){in=0;}else if(!in){n++;in=1;}}ak_u32(words,(unsigned)n);row(x+14*u,y+244*u,w-28*u,"characters",chars,AK_TEXT,u);row(x+14*u,y+272*u,w-28*u,"words",words,AK_TEXT,u);
}

static void draw_keys(int x,int y,int w,int h,int u)
{
    head(x,y,w,"Keyboard Tester","raw events",u);ak_card((struct ak_rect){x+14*u,y+54*u,w-28*u,74*u});ak_text_bold(x+28*u,y+68*u,"Last key",AK_DIM);ak_text_bold(x+28*u,y+94*u,key_last,key_count?AK_ACCENT_2:AK_FAINT);char n[16];ak_u32(n,(unsigned)key_count);row(x+14*u,y+148*u,w-28*u,"events",n,AK_TEXT,u);row(x+14*u,y+176*u,w-28*u,"modifiers","reported by input queue",AK_TEXT,u);ak_text(x+14*u,y+224*u,"Press arrows, function keys, or type text. Every event reaches this window.",AK_DIM);
}

static void run_bench(void)
{
    volatile unsigned a=1;unsigned t=idt_ticks();for(unsigned i=0;i<300000;i++)a=a*1664525u+1013904223u;bench_int=idt_ticks()-t;t=idt_ticks();char s[24]="zl kernel benchmark";for(unsigned i=0;i<20000;i++)a^=ak_fnv1a(s);bench_hash=idt_ticks()-t;t=idt_ticks();for(unsigned i=0;i<50000;i++)a^=i*2654435761u;bench_raster=idt_ticks()-t;bench_done=(int)(a|1);
}
static void draw_bench(int x,int y,int w,int h,int u)
{
    head(x,y,w,"Benchmark","measured at 100 Hz",u);ak_button((struct ak_rect){x+14*u,y+54*u,126*u,28*u},"Run benchmark",1);char a[16],b[16],c[16];ak_u32(a,bench_int*10);ak_u32(b,bench_hash*10);ak_u32(c,bench_raster*10);row(x+14*u,y+106*u,w-28*u,"integer ms",bench_done?a:"not run",bench_done?AK_OK:AK_FAINT,u);row(x+14*u,y+134*u,w-28*u,"hash ms",bench_done?b:"not run",bench_done?AK_OK:AK_FAINT,u);row(x+14*u,y+162*u,w-28*u,"raster ms",bench_done?c:"not run",bench_done?AK_OK:AK_FAINT,u);ak_text(x+14*u,y+214*u,"Three real kernel workloads; no host timing API or library.",AK_DIM);
}

void reftool_draw(int app,int x,int y,int w,int h,int focused)
{
    (void)focused;int u=ui_theme()->scale;ak_fill((struct ak_rect){x,y,w,h},AK_SURFACE);
    if(app==APP_U_CLIPBOARD)draw_clip(x,y,w,h,u);else if(app==APP_U_COLOUR)draw_colour(x,y,w,h,u);else if(app==APP_U_REGEX)draw_regex(x,y,w,h,u);else if(app==APP_U_BASE)draw_base(x,y,w,h,u);else if(app==APP_U_DIFF)draw_diff(x,y,w,h,u);else if(app==APP_U_HASH)draw_hash(x,y,w,h,u);else if(app==APP_U_UNIT)draw_unit(x,y,w,h,u);else if(app==APP_U_NOTES)draw_notes(x,y,w,h,u);else if(app==APP_U_KEYS)draw_keys(x,y,w,h,u);else if(app==APP_U_BENCH)draw_bench(x,y,w,h,u);
}

static char *active_buf(int app,int *cap)
{
    if(app==APP_U_COLOUR){*cap=sizeof colour;return colour;}if(app==APP_U_REGEX){if(regex_field){*cap=sizeof regex_text;return regex_text;}*cap=sizeof regex_pat;return regex_pat;}if(app==APP_U_BASE){*cap=sizeof base_text;return base_text;}if(app==APP_U_DIFF){if(diff_field){*cap=sizeof diff_b;return diff_b;}*cap=sizeof diff_a;return diff_a;}if(app==APP_U_HASH){*cap=sizeof hash_text;return hash_text;}if(app==APP_U_UNIT){*cap=sizeof unit_text;return unit_text;}if(app==APP_U_NOTES){*cap=sizeof note_text;return note_text;}*cap=0;return 0;
}

int reftool_event(int app,int win,int type,int code,int px,int py)
{
    int x,y,w,h,u=ui_theme()->scale;wm_client(win,&x,&y,&w,&h);
    if(app==APP_U_KEYS&&(type==AK_EV_CHAR||type==AK_EV_KEY_DOWN)){char n[16];ak_i32(n,code);ak_copy(key_last,sizeof key_last,type==AK_EV_CHAR?"char ":"key ");for(int i=0;n[i];i++)ak_append_char(key_last,sizeof key_last,n[i]);key_count++;wm_damage_win(win);return 1;}
    if(type==AK_EV_CHAR){int cap;char *b=active_buf(app,&cap);if(b){int ch=code;if(app==APP_U_DIFF&&ch=='|')ch='\n';if(ch==8)ak_backspace(b);else ak_append_char(b,cap,ch);wm_damage_win(win);return 1;}}
    if(type==AK_EV_KEY_DOWN&&code==AK_KEY_BACKSPACE){int cap;char*b=active_buf(app,&cap);if(b){ak_backspace(b);wm_damage_win(win);return 1;}}
    if(type!=AK_EV_MOUSE||!(code&1))return type==AK_EV_MOUSE;
    if(app==APP_U_CLIPBOARD&&py>y+175*u&&py<y+220*u)clip_clear();
    else if(app==APP_U_REGEX){if(py>y+50*u&&py<y+84*u)regex_field=0;else if(py>y+84*u&&py<y+122*u)regex_field=1;}
    else if(app==APP_U_DIFF){if(py>y+50*u&&py<y+84*u)diff_field=0;else if(py>y+84*u&&py<y+122*u)diff_field=1;}
    else if(app==APP_U_UNIT&&py>y+86*u&&py<y+120*u){int i=(px-(x+14*u))/(64*u);if(i>=0&&i<4)unit_from=i;}
    else if(app==APP_U_NOTES&&py>y+188*u&&py<y+230*u&&px>x+130*u)note_text[0]=0;
    else if(app==APP_U_BENCH&&py>y+48*u&&py<y+90*u)run_bench();
    wm_damage_win(win);return 1;
}

int reftool_tick(int app,int win){(void)app;(void)win;return 0;}
