/* support.c - the two port-I/O intrinsics and the COM1 bring-up.
 *
 * design_kernel.md §6.3 lists outb/inb among the intrinsics the kernel
 * backend must provide. Until kernelgen.c exists they live here as ordinary
 * C, which is exactly the point of the C-backend route: gcc supplies the
 * privileged-instruction escape (__asm__) that W5 would otherwise have to
 * build into zl itself.
 */
#define COM1 0x3F8

void zl_outb(unsigned short port, unsigned char val)
{ __asm__ volatile("outb %0, %1" :: "a"(val), "Nd"(port)); }

unsigned char zl_inb(unsigned short port)
{ unsigned char r; __asm__ volatile("inb %1, %0" : "=a"(r) : "Nd"(port)); return r; }

/* ---- CPUID: ask the processor its own name ------------------------------
 * Leaves 0x80000002..4 return the 48-character brand string ("Intel(R)
 * Core(TM) i7-...") directly in EAX/EBX/ECX/EDX. This is a real query to
 * the silicon - the same string Linux shows in /proc/cpuinfo. Cached on
 * first use; zl reads it a byte at a time through cpu_brand_byte(). */
static int  brand_ready = 0;
static char brand[49];

/* CPUID clobbers ebx, and at -O2 with ebx as a direct output constraint gcc
 * can mismanage it - the result came back all-zero in the kernel while the
 * same code worked in user mode. The bulletproof pattern saves ebx into esi
 * around the instruction and reads the result from esi. */
static void do_cpuid(unsigned leaf, unsigned *a, unsigned *b, unsigned *c, unsigned *d)
{
    unsigned ra, rb, rc, rd;
    __asm__ volatile(
        "movl %%ebx, %%esi\n\t"
        "cpuid\n\t"
        "xchgl %%ebx, %%esi\n\t"
        : "=a"(ra), "=S"(rb), "=c"(rc), "=d"(rd)
        : "0"(leaf)
        : "memory");
    *a = ra; *b = rb; *c = rc; *d = rd;
}

int cpu_brand_byte(int i)
{
    if (!brand_ready) {
        unsigned regs[12];
        unsigned *p = regs;
        for (unsigned leaf = 0x80000002u; leaf <= 0x80000004u; leaf++) {
            do_cpuid(leaf, &p[0], &p[1], &p[2], &p[3]);
            p += 4;
        }
        char *src = (char *)regs;
        for (int k = 0; k < 48; k++) brand[k] = src[k];
        brand[48] = 0;
        brand_ready = 1;
    }
    if (i < 0 || i >= 48) return 0;
    return (unsigned char)brand[i];
}

/* ---- PC speaker: real sound, via PIT channel 2 and the gate at port 0x61 */
void speaker_on(unsigned freq)
{
    if (freq == 0) return;
    unsigned div = 1193182u / freq;
    zl_outb(0x43, 0xB6);                        /* channel 2, mode 3, square wave */
    zl_outb(0x42, (unsigned char)(div & 0xFF));
    zl_outb(0x42, (unsigned char)((div >> 8) & 0xFF));
    unsigned char t = zl_inb(0x61);
    zl_outb(0x61, t | 0x03);                    /* connect the speaker to channel 2 */
}
void speaker_off(void)
{
    unsigned char t = zl_inb(0x61);
    zl_outb(0x61, t & 0xFC);
}

/* ---- reboot: pulse the 8042 keyboard controller's CPU reset line -------- */
void kreboot(void)
{
    unsigned char s = 0x02;
    while (s & 0x02) s = zl_inb(0x64);          /* wait for the input buffer to clear */
    zl_outb(0x64, 0xFE);                        /* command 0xFE: pulse reset */
    for (;;) __asm__ volatile("hlt");
}

/* 115200 8N1, FIFOs on, interrupts off - v1 polls (design_kernel.md §7.2) */
void serial_init(void)
{
    zl_outb(COM1 + 1, 0x00);   /* no interrupts - we poll               */
    zl_outb(COM1 + 3, 0x80);   /* DLAB on: next two writes are divisor  */
    zl_outb(COM1 + 0, 0x01);   /* divisor lo = 1  -> 115200 baud        */
    zl_outb(COM1 + 1, 0x00);   /* divisor hi = 0                        */
    zl_outb(COM1 + 3, 0x03);   /* DLAB off, 8 bits, no parity, 1 stop   */
    /* Enable FIFOs at a 14-byte trigger WITHOUT clearing them (0xC1, not
       0xC7): clearing would discard a byte that arrived before init.
       Note this does not fully solve piped-at-boot input - QEMU can deliver
       the very first byte before the guest executes at all, and that one is
       unrecoverable from inside the kernel. It costs nothing for a human at
       a terminal, who cannot type before the machine boots. The automated
       test sends a padding byte first for this reason. */
    zl_outb(COM1 + 2, 0xC1);
    zl_outb(COM1 + 4, 0x0B);   /* DTR + RTS + OUT2                      */
}

/* Reached when the zl program's main() returns. A kernel has nowhere to
 * return TO, so say so and stop - and tell QEMU to exit so a headless test
 * can assert on it (isa-debug-exit maps a port write to an exit code). */
void kernel_done(void)
{
    const char *m = "\n[kernel] main() returned - halting\n";
    while (*m) {
        while ((zl_inb(COM1 + 5) & 0x20) == 0) { }
        zl_outb(COM1, (unsigned char)*m++);
    }
    zl_outb(0xF4, 0x00);       /* QEMU isa-debug-exit -> exit code 1     */
    for (;;) __asm__ volatile("hlt");
}
