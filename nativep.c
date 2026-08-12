/* nativep.c - Floor 4: emit a native .exe that PRINTS, by hand.
 *
 * Same idea as native.c but the emitted rawp.exe actually writes
 * "Hello from raw machine code!" to the console using hand-encoded
 * x86-64 that calls GetStdHandle + WriteFile + ExitProcess. No C
 * compiler touches rawp.exe's bytes.
 */
#include <stdio.h>
#include <string.h>

static unsigned char buf[0x600];

static void u16(int o, unsigned v){ buf[o]=v&0xFF; buf[o+1]=(v>>8)&0xFF; }
static void u32(int o, unsigned v){ for(int i=0;i<4;i++) buf[o+i]=(v>>(8*i))&0xFF; }
static void u64(int o, unsigned long long v){ for(int i=0;i<8;i++) buf[o+i]=(unsigned char)((v>>(8*i))&0xFF); }

int main(void)
{
    memset(buf, 0, sizeof(buf));

    /* DOS */
    buf[0]='M'; buf[1]='Z'; u32(0x3C, 0x40);
    /* PE sig */
    int pe=0x40; buf[pe]='P'; buf[pe+1]='E';
    /* COFF */
    int coff=pe+4;
    u16(coff+0,0x8664); u16(coff+2,2); u16(coff+16,0xF0); u16(coff+18,0x0022);
    /* Optional (PE32+) */
    int opt=coff+20;
    u16(opt+0,0x020B); u32(opt+4,0x200); u32(opt+8,0x200);
    u32(opt+16,0x1000); u32(opt+20,0x1000); u64(opt+24,0x140000000ULL);
    u32(opt+32,0x1000); u32(opt+36,0x200); u16(opt+40,6); u16(opt+48,6);
    u32(opt+56,0x3000); u32(opt+60,0x200); u16(opt+68,3);
    u64(opt+72,0x100000ULL); u64(opt+80,0x1000ULL); u64(opt+88,0x100000ULL); u64(opt+96,0x1000ULL);
    u32(opt+108,16);
    int dd=opt+112;
    u32(dd+1*8+0,0x2000); u32(dd+1*8+4,40);      /* import table */
    u32(dd+12*8+0,0x2048); u32(dd+12*8+4,32);    /* IAT (3 funcs + null) */
    /* sections */
    int sec=opt+0xF0;
    memcpy(buf+sec,".text",5);
    u32(sec+8,0x200); u32(sec+12,0x1000); u32(sec+16,0x200); u32(sec+20,0x200); u32(sec+36,0x60000020);
    int s2=sec+40;
    memcpy(buf+s2,".idata",6);
    u32(s2+8,0x200); u32(s2+12,0x2000); u32(s2+16,0x200); u32(s2+20,0x400); u32(s2+36,0xC0000040);

    /* ---- .text : hand-encoded machine code (RVA 0x1000, file 0x200) ----
     *   sub rsp,0x38
     *   mov ecx,-11              ; STD_OUTPUT_HANDLE
     *   call [rip+GetStdHandle]
     *   mov rcx,rax             ; handle
     *   lea rdx,[rip+msg]
     *   mov r8d,29              ; length
     *   lea r9,[rip+written]
     *   mov qword[rsp+0x20],0   ; lpOverlapped = NULL (5th arg)
     *   call [rip+WriteFile]
     *   xor ecx,ecx
     *   call [rip+ExitProcess]
     */
    unsigned char code[] = {
        0x48,0x83,0xEC,0x38,                          /* sub rsp,0x38            */
        0xB9,0xF5,0xFF,0xFF,0xFF,                      /* mov ecx,-11             */
        0xFF,0x15,0x39,0x10,0x00,0x00,                 /* call [rip+0x1039]->gsh  */
        0x48,0x89,0xC1,                                /* mov rcx,rax             */
        0x48,0x8D,0x15,0x87,0x10,0x00,0x00,            /* lea rdx,[rip+0x1087]msg */
        0x41,0xB8,0x1D,0x00,0x00,0x00,                 /* mov r8d,29              */
        0x4C,0x8D,0x0D,0x9A,0x10,0x00,0x00,            /* lea r9,[rip+0x109A]wr   */
        0x48,0xC7,0x44,0x24,0x20,0x00,0x00,0x00,0x00,  /* mov qword[rsp+0x20],0   */
        0xFF,0x15,0x1B,0x10,0x00,0x00,                 /* call [rip+0x101B]->wf   */
        0x31,0xC9,                                     /* xor ecx,ecx             */
        0xFF,0x15,0x1B,0x10,0x00,0x00                  /* call [rip+0x101B]->exit */
    };
    memcpy(buf+0x200, code, sizeof(code));

    /* ---- .idata (RVA 0x2000, file 0x400) ---- */
    /* descriptor[0] */
    u32(0x400+0,  0x2028);      /* OriginalFirstThunk (ILT) */
    u32(0x400+12, 0x2092);      /* Name (kernel32.dll)      */
    u32(0x400+16, 0x2048);      /* FirstThunk (IAT)         */
    /* ILT (RVA 0x2028, file 0x428) */
    u64(0x428, 0x2068);         /* GetStdHandle */
    u64(0x430, 0x2078);         /* WriteFile    */
    u64(0x438, 0x2084);         /* ExitProcess  */
    /* IAT (RVA 0x2048, file 0x448) */
    u64(0x448, 0x2068);
    u64(0x450, 0x2078);
    u64(0x458, 0x2084);
    /* hint/name entries */
    u16(0x468,0); memcpy(buf+0x46A,"GetStdHandle",12);   /* RVA 0x2068 */
    u16(0x478,0); memcpy(buf+0x47A,"WriteFile",9);       /* RVA 0x2078 */
    u16(0x484,0); memcpy(buf+0x486,"ExitProcess",11);    /* RVA 0x2084 */
    memcpy(buf+0x492,"kernel32.dll",12);                 /* RVA 0x2092 */
    /* message (RVA 0x20A0, file 0x4A0) */
    memcpy(buf+0x4A0, "Hello from raw machine code!\n", 29);
    /* written var (RVA 0x20C0, file 0x4C0) - 8 bytes, already zero */

    FILE *f=fopen("rawp.exe","wb");
    if(!f){ fprintf(stderr,"can't write rawp.exe\n"); return 1; }
    fwrite(buf,1,sizeof(buf),f);
    fclose(f);
    printf("wrote rawp.exe (hand-assembled PE that prints, no C compiler in the output)\n");
    return 0;
}
