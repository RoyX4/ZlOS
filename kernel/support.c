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
