/* native.c - Floor 4 seed: emit a real Windows .exe BY HAND.
 *
 * This writes a valid 64-bit PE executable byte-by-byte - no C
 * compiler touches the OUTPUT. The program it emits calls
 * ExitProcess(42), so if you run raw.exe and %ERRORLEVEL% is 42,
 * we produced working native machine code with nothing borrowed
 * from cl/gcc for the output path. That's the "second swap".
 *
 * (native.c itself is compiled by cl - that's the bootstrap, same
 * as every other .c here. The point is what raw.exe's bytes owe to
 * a C compiler: nothing.)
 *
 * Layout (file / RVA):
 *   headers          0x000
 *   .text  code      0x200 / 0x1000   (the machine code)
 *   .idata imports   0x400 / 0x2000   (tells Windows: need ExitProcess)
 */
#include <stdio.h>
#include <string.h>

static unsigned char buf[0x600];      /* 3 file-aligned chunks */

static void u16(int off, unsigned v) { buf[off]=v&0xFF; buf[off+1]=(v>>8)&0xFF; }
static void u32(int off, unsigned v) { for (int i=0;i<4;i++) buf[off+i]=(v>>(8*i))&0xFF; }
static void u64(int off, unsigned long long v) { for (int i=0;i<8;i++) buf[off+i]=(unsigned char)((v>>(8*i))&0xFF); }

int main(void)
{
    memset(buf, 0, sizeof(buf));

    /* ---- DOS header ---- */
    buf[0]='M'; buf[1]='Z';
    u32(0x3C, 0x40);                  /* e_lfanew -> PE header at 0x40 */

    /* ---- PE signature ---- */
    int pe = 0x40;
    buf[pe]='P'; buf[pe+1]='E';       /* "PE\0\0" */

    /* ---- COFF header (20 bytes) ---- */
    int coff = pe + 4;                /* 0x44 */
    u16(coff+0, 0x8664);              /* Machine = x86-64 */
    u16(coff+2, 2);                   /* NumberOfSections */
    u16(coff+16, 0xF0);               /* SizeOfOptionalHeader (PE32+) */
    u16(coff+18, 0x0022);            /* EXECUTABLE | LARGE_ADDRESS_AWARE */

    /* ---- Optional header (PE32+) ---- */
    int opt = coff + 20;              /* 0x58 */
    u16(opt+0, 0x020B);               /* Magic = PE32+ */
    u32(opt+4, 0x200);                /* SizeOfCode */
    u32(opt+8, 0x200);                /* SizeOfInitializedData */
    u32(opt+16, 0x1000);              /* AddressOfEntryPoint (RVA) */
    u32(opt+20, 0x1000);              /* BaseOfCode */
    u64(opt+24, 0x140000000ULL);      /* ImageBase */
    u32(opt+32, 0x1000);              /* SectionAlignment */
    u32(opt+36, 0x200);               /* FileAlignment */
    u16(opt+40, 6);                   /* MajorOSVersion */
    u16(opt+48, 6);                   /* MajorSubsystemVersion */
    u32(opt+56, 0x3000);              /* SizeOfImage (3 pages) */
    u32(opt+60, 0x200);               /* SizeOfHeaders */
    u16(opt+68, 3);                   /* Subsystem = console */
    u64(opt+72, 0x100000ULL);         /* SizeOfStackReserve */
    u64(opt+80, 0x1000ULL);           /* SizeOfStackCommit */
    u64(opt+88, 0x100000ULL);         /* SizeOfHeapReserve */
    u64(opt+96, 0x1000ULL);           /* SizeOfHeapCommit */
    u32(opt+108, 16);                 /* NumberOfRvaAndSizes */
    /* data directories start at opt+112 */
    int dd = opt + 112;
    u32(dd + 1*8 + 0, 0x2000);        /* [1] Import Table RVA */
    u32(dd + 1*8 + 4, 40);            /* [1] Import Table size (2 descriptors) */
    u32(dd + 12*8 + 0, 0x2038);       /* [12] IAT RVA */
    u32(dd + 12*8 + 4, 16);           /* [12] IAT size */

    /* ---- Section headers (start right after optional header) ---- */
    int sec = opt + 0xF0;             /* 0x148 */

    memcpy(buf+sec, ".text", 5);
    u32(sec+8, 0x200);                /* VirtualSize */
    u32(sec+12, 0x1000);              /* VirtualAddress */
    u32(sec+16, 0x200);               /* SizeOfRawData */
    u32(sec+20, 0x200);               /* PointerToRawData */
    u32(sec+36, 0x60000020);          /* CODE | EXECUTE | READ */

    int sec2 = sec + 40;
    memcpy(buf+sec2, ".idata", 6);
    u32(sec2+8, 0x200);               /* VirtualSize */
    u32(sec2+12, 0x2000);             /* VirtualAddress */
    u32(sec2+16, 0x200);              /* SizeOfRawData */
    u32(sec2+20, 0x400);              /* PointerToRawData */
    u32(sec2+36, 0xC0000040);         /* INITIALIZED_DATA | READ | WRITE */

    /* ---- .text : the actual machine code (file 0x200, RVA 0x1000) ----
     *   sub  rsp, 0x28        48 83 EC 28
     *   mov  ecx, 42          B9 2A 00 00 00
     *   call [rip+disp32]     FF 15 <disp32>   -> ExitProcess via IAT
     * disp32 = IAT_RVA(0x2038) - RVA_of_next_instr(0x100F) = 0x1029
     */
    int t = 0x200;
    unsigned char code[] = {
        0x48,0x83,0xEC,0x28,
        0xB9,0x2A,0x00,0x00,0x00,
        0xFF,0x15,0x29,0x10,0x00,0x00
    };
    memcpy(buf+t, code, sizeof(code));

    /* ---- .idata : imports (file 0x400, RVA 0x2000) ---- */
    int d = 0x400;                    /* RVA 0x2000 */
    /* import descriptor[0] for kernel32.dll */
    u32(d+0,  0x2028);                /* OriginalFirstThunk (ILT RVA) */
    u32(d+12, 0x2058);                /* Name (dll name RVA) */
    u32(d+16, 0x2038);                /* FirstThunk (IAT RVA) */
    /* descriptor[1] is the null terminator (already zero) */

    u64(0x428, 0x2048);               /* ILT[0] -> hint/name RVA */
    u64(0x438, 0x2048);               /* IAT[0] -> hint/name RVA (loader patches) */

    /* IMAGE_IMPORT_BY_NAME at RVA 0x2048 (file 0x448): hint(2)=0 + name */
    u16(0x448, 0);
    memcpy(buf+0x44A, "ExitProcess", 11);

    /* dll name at RVA 0x2058 (file 0x458) */
    memcpy(buf+0x458, "kernel32.dll", 12);

    FILE *f = fopen("raw.exe", "wb");
    if (!f) { fprintf(stderr, "can't write raw.exe\n"); return 1; }
    fwrite(buf, 1, sizeof(buf), f);
    fclose(f);
    printf("wrote raw.exe (%d bytes, hand-assembled PE, no C compiler in the output)\n", (int)sizeof(buf));
    return 0;
}
