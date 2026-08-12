/* nativeheap.c - Floor 4 runtime brick #1: a HEAP with no libc.
 *
 * Emits raw_heap.exe by hand: it calls VirtualAlloc to get real
 * memory from Windows, copies a string into that heap block, and
 * WriteFiles it to stdout - then exits. No malloc, no C runtime,
 * no libc. This proves the foundation of the "no C at all" runtime:
 * we can get heap memory straight from the OS.
 *
 * Imports (kernel32): VirtualAlloc, GetStdHandle, WriteFile, ExitProcess.
 */
#include <stdio.h>
#include <string.h>

static unsigned char buf[0x600];
static void u16(int o, unsigned v){ buf[o]=v&0xFF; buf[o+1]=(v>>8)&0xFF; }
static void u32(int o, unsigned v){ for(int i=0;i<4;i++) buf[o+i]=(v>>(8*i))&0xFF; }
static void u64(int o, unsigned long long v){ for(int i=0;i<8;i++) buf[o+i]=(unsigned char)((v>>(8*i))&0xFF); }

int main(void)
{
    memset(buf,0,sizeof(buf));
    buf[0]='M'; buf[1]='Z'; u32(0x3C,0x40);
    int pe=0x40; buf[pe]='P'; buf[pe+1]='E';
    int coff=pe+4;
    u16(coff+0,0x8664); u16(coff+2,2); u16(coff+16,0xF0); u16(coff+18,0x0022);
    int opt=coff+20;
    u16(opt+0,0x020B); u32(opt+4,0x200); u32(opt+8,0x200);
    u32(opt+16,0x1000); u32(opt+20,0x1000); u64(opt+24,0x140000000ULL);
    u32(opt+32,0x1000); u32(opt+36,0x200); u16(opt+40,6); u16(opt+48,6);
    u32(opt+56,0x3000); u32(opt+60,0x200); u16(opt+68,3);
    u64(opt+72,0x100000ULL); u64(opt+80,0x1000ULL); u64(opt+88,0x100000ULL); u64(opt+96,0x1000ULL);
    u32(opt+108,16);
    int dd=opt+112;
    u32(dd+1*8+0,0x2000); u32(dd+1*8+4,40);
    u32(dd+12*8+0,0x2050); u32(dd+12*8+4,40);
    int sec=opt+0xF0;
    memcpy(buf+sec,".text",5);
    u32(sec+8,0x200); u32(sec+12,0x1000); u32(sec+16,0x200); u32(sec+20,0x200); u32(sec+36,0x60000020);
    int s2=sec+40;
    memcpy(buf+s2,".idata",6);
    u32(s2+8,0x200); u32(s2+12,0x2000); u32(s2+16,0x200); u32(s2+20,0x400); u32(s2+36,0xC0000040);

    /* ---- .text : machine code (RVA 0x1000, file 0x200) ---- */
    unsigned char code[] = {
        0x55,                                       /* push rbp                        */
        0x48,0x89,0xE5,                             /* mov rbp,rsp                     */
        0x48,0x83,0xEC,0x30,                        /* sub rsp,0x30                    */
        0x48,0x83,0xE4,0xF0,                        /* and rsp,-16 (force 16-align)    */
        /* VirtualAlloc(NULL, 0x100000, MEM_COMMIT|RESERVE, PAGE_RW) */
        0x31,0xC9,                                  /* xor ecx,ecx  (NULL)             */
        0xBA,0x00,0x00,0x10,0x00,                   /* mov edx,0x100000                */
        0x41,0xB8,0x00,0x30,0x00,0x00,              /* mov r8d,0x3000                  */
        0x41,0xB9,0x04,0x00,0x00,0x00,              /* mov r9d,4 (PAGE_READWRITE)      */
        0xFF,0x15,0x2B,0x10,0x00,0x00,              /* call [rip+VirtualAlloc]         */
        0x48,0x89,0xC3,                             /* mov rbx,rax (heap base)         */
        /* copy 13 bytes of msg into the heap */
        0x48,0x8D,0x35,0x91,0x10,0x00,0x00,         /* lea rsi,[rip+msg]               */
        0x48,0x89,0xDF,                             /* mov rdi,rbx                     */
        0xB9,0x0D,0x00,0x00,0x00,                   /* mov ecx,13                      */
        0xF3,0xA4,                                  /* rep movsb                       */
        /* GetStdHandle(-11) */
        0xB9,0xF5,0xFF,0xFF,0xFF,                   /* mov ecx,-11                     */
        0xFF,0x15,0x14,0x10,0x00,0x00,              /* call [rip+GetStdHandle]         */
        /* WriteFile(handle, heap, 13, &written, 0) */
        0x48,0x89,0xC1,                             /* mov rcx,rax                     */
        0x48,0x89,0xDA,                             /* mov rdx,rbx                     */
        0x41,0xB8,0x0D,0x00,0x00,0x00,              /* mov r8d,13                      */
        0x4C,0x8D,0x4C,0x24,0x28,                   /* lea r9,[rsp+0x28] (&written)    */
        0x48,0xC7,0x44,0x24,0x20,0x00,0x00,0x00,0x00,/* mov qword[rsp+0x20],0          */
        0xFF,0x15,0xFC,0x0F,0x00,0x00,              /* call [rip+WriteFile]            */
        /* ExitProcess(0) */
        0x31,0xC9,                                  /* xor ecx,ecx                     */
        0xFF,0x15,0xFC,0x0F,0x00,0x00               /* call [rip+ExitProcess]          */
    };
    memcpy(buf+0x200, code, sizeof(code));

    /* ---- .idata (RVA 0x2000, file 0x400): 4 imports ---- */
    int d=0x400;
    u32(d+0,  0x2028);      /* OFT (ILT)         */
    u32(d+12, 0x20B2);      /* Name (kernel32)   */
    u32(d+16, 0x2050);      /* FT (IAT)          */
    /* ILT */
    u64(0x428,0x2078); u64(0x430,0x2088); u64(0x438,0x2098); u64(0x440,0x20A4);
    /* IAT */
    u64(0x450,0x2078); u64(0x458,0x2088); u64(0x460,0x2098); u64(0x468,0x20A4);
    /* names */
    u16(0x478,0); memcpy(buf+0x47A,"VirtualAlloc",12);
    u16(0x488,0); memcpy(buf+0x48A,"GetStdHandle",12);
    u16(0x498,0); memcpy(buf+0x49A,"WriteFile",9);
    u16(0x4A4,0); memcpy(buf+0x4A6,"ExitProcess",11);
    memcpy(buf+0x4B2,"kernel32.dll",12);
    /* the message that gets copied into the heap */
    memcpy(buf+0x4C0,"heap string!\n",13);

    FILE *f=fopen("raw_heap.exe","wb");
    if(!f){ fprintf(stderr,"can't write\n"); return 1; }
    fwrite(buf,1,sizeof(buf),f);
    fclose(f);
    printf("wrote raw_heap.exe (allocates OS heap via VirtualAlloc, no libc)\n");
    return 0;
}
