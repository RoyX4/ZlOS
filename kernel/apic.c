/* apic.c - the modern interrupt controller, and the ACPI tables that find it.
 *
 * WHY THIS EXISTS
 * ---------------
 * zlOS boots on the real laptop and the keyboard does not work, even though
 * the i8042 driver is correct and initialises the controller properly. The
 * keyboard on that machine really is a PS/2 AT keyboard on the i8042 - that
 * part was checked, not assumed - so the device is there and it is talking.
 * What is missing is the wire between it and the CPU.
 *
 * The 8259 PIC we program in idt.c is a 1981 part. On a modern UEFI machine it
 * may be electrically absent, disconnected by the chipset, or left disabled by
 * firmware that has spent the whole boot in APIC mode. When that happens the
 * keyboard raises IRQ1, the PIC never delivers it, and the symptom is exactly
 * what a dead keyboard looks like - which is why this was worth chasing before
 * writing more device drivers.
 *
 * The replacement has two halves:
 *
 *   LOCAL APIC   one per CPU core, on the CPU die. Receives interrupts and is
 *                where the end-of-interrupt acknowledgement goes. Always at a
 *                physical address the IA32_APIC_BASE MSR tells us.
 *   I/O APIC     on the chipset. Takes the 24 hardware IRQ lines and routes
 *                each one to a CPU and a vector of our choosing.
 *
 * Finding the I/O APIC is where ACPI comes in: its address is not fixed by any
 * standard, it is written in a firmware table. So this file also contains a
 * small ACPI table walker - RSDP, then RSDT or XSDT, then the MADT.
 *
 * Interrupt Source Overrides matter more than they look. The ISA IRQ number a
 * device uses is not necessarily the I/O APIC input it lands on: the timer is
 * almost always IRQ0 re-routed to GSI 2. Ignoring the override table means
 * programming the wrong pin and getting silence.
 */

typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;

#if defined(ZL_64)
typedef unsigned long long uptr;
#else
typedef unsigned int       uptr;
#endif

extern void zl_outb(unsigned short port, unsigned char val);
extern unsigned char zl_inb(unsigned short port);

static u32 mmio_r(uptr a)          { return *(volatile u32 *)a; }
static void mmio_w(uptr a, u32 v)  { *(volatile u32 *)a = v; }

/* Keep legacy physical-memory reads behind an address-taking seam. GCC 15
 * otherwise applies hosted null-object bounds reasoning to the constant BIOS
 * Data Area address 0x40e, even though this is a freestanding kernel mapping. */
static u16 phys_r16(uptr a)
{
#if defined(__GNUC__) && !defined(__clang__)
    /* An empty value barrier prevents GCC's hosted-object constant
     * propagation without emitting an instruction or changing the address. */
    __asm__ volatile("" : "+r"(a));
#endif
    return *(volatile u16 *)a;
}

/* ---- CPU instructions we need ------------------------------------------ */
static void do_cpuid(u32 leaf, u32 *a, u32 *b, u32 *c, u32 *d)
{
    __asm__ volatile("cpuid"
                     : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
                     : "a"(leaf), "c"(0));
}

static u64 rdmsr(u32 msr)
{
    u32 lo, hi;
    __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
    return ((u64)hi << 32) | (u64)lo;
}

static void wrmsr(u32 msr, u64 v)
{
    __asm__ volatile("wrmsr" :: "c"(msr), "a"((u32)v), "d"((u32)(v >> 32)));
}

#define IA32_APIC_BASE  0x1B
#define APIC_BASE_ENABLE (1u << 11)      /* global enable, bit 11 */

/* ---- local APIC registers, offsets from its base ----------------------- */
#define LAPIC_ID        0x020
#define LAPIC_VERSION   0x030
#define LAPIC_TPR       0x080   /* task priority: 0 = accept everything */
#define LAPIC_EOI       0x0B0
#define LAPIC_SVR       0x0F0   /* spurious vector, bit 8 = software enable */
#define LAPIC_LVT_LINT0 0x350
#define LAPIC_LVT_LINT1 0x360

/* ---- I/O APIC: two registers, an index and a window -------------------- */
#define IOAPIC_REGSEL   0x00
#define IOAPIC_IOWIN    0x10
#define IOAPIC_REG_ID   0x00
#define IOAPIC_REG_VER  0x01
#define IOAPIC_REDTBL(n) (0x10 + 2 * (n))

/* ---- state ------------------------------------------------------------- */
static uptr lapic_base  = 0;
static uptr ioapic_base = 0;
static u32  ioapic_gsi_base = 0;
static int  ioapic_pins = 0;
static int  apic_on     = 0;
static u64  rsdp_addr   = 0;      /* handed to us by the UEFI loader */
static int  madt_found  = 0;
static int  cpu_count   = 0;
static u8   cpu_apic_ids[16];      /* the APIC id of every usable core */

/* IRQ -> GSI mapping, plus the polarity/trigger the firmware declared.
 * Identity by default, per ACPI: "if no override exists, ISA IRQ n is GSI n". */
static u32 gsi_of_irq[16];
static u16 flags_of_irq[16];

static u32 ioapic_read(u32 reg)
{
    mmio_w(ioapic_base + IOAPIC_REGSEL, reg);
    return mmio_r(ioapic_base + IOAPIC_IOWIN);
}

static void ioapic_write(u32 reg, u32 val)
{
    mmio_w(ioapic_base + IOAPIC_REGSEL, reg);
    mmio_w(ioapic_base + IOAPIC_IOWIN, val);
}

/* ---- ACPI ---------------------------------------------------------------
 * The Root System Description Pointer is the entry point to every other table.
 * On a BIOS machine it is found by scanning two well-known regions for the
 * signature; under UEFI there is no such guarantee and the firmware hands it
 * to us in the configuration table instead, which efi.c captures before
 * ExitBootServices and passes in here. */
void acpi_set_rsdp(u64 addr) { rsdp_addr = addr; }

static int sig_is(uptr p, const char *s, int n)
{
    for (int i = 0; i < n; i++)
        if (*(volatile u8 *)(p + (uptr)i) != (u8)s[i]) return 0;
    return 1;
}

static int checksum_ok(uptr p, u32 len)
{
    u8 sum = 0;
    for (u32 i = 0; i < len; i++) sum = (u8)(sum + *(volatile u8 *)(p + (uptr)i));
    return sum == 0;
}

static u64 scan_for_rsdp(uptr from, uptr to)
{
    /* the signature is always on a 16-byte boundary */
    for (uptr p = from; p < to; p += 16) {
        if (!sig_is(p, "RSD PTR ", 8)) continue;
        if (!checksum_ok(p, 20)) continue;      /* the ACPI 1.0 part */
        return (u64)p;
    }
    return 0;
}

u64 acpi_find_rsdp(void)
{
    if (rsdp_addr) return rsdp_addr;            /* UEFI already told us */

    /* the Extended BIOS Data Area, whose segment is parked at 0x40E */
    u32 ebda = (u32)phys_r16((uptr)0x40E) << 4;
    if (ebda >= 0x400 && ebda < 0xA0000) {
        u64 r = scan_for_rsdp((uptr)ebda, (uptr)(ebda + 1024));
        if (r) { rsdp_addr = r; return r; }
    }
    /* then the BIOS read-only region */
    u64 r = scan_for_rsdp((uptr)0xE0000, (uptr)0x100000);
    if (r) rsdp_addr = r;
    return rsdp_addr;
}

/* Walk the RSDT (32-bit entries) or XSDT (64-bit) looking for one signature.
 * Revision 0 means ACPI 1.0 and only the RSDT exists. */
/* NOT static. Six unwritten subsystems are gated on reaching this one function:
 * TPM2, HPET, FADT (reboot and power-off), ECDT (battery), MCFG (ECAM) and
 * LPIT all need to find their own table, and every one of them would otherwise
 * carry a copy of this RSDP walk. The driver survey called removing this
 * keyword the highest leverage per line in the whole thirty-four driver review,
 * which is a strange thing to be true and is true anyway. */
uptr acpi_find_table(const char *sig)
{
    u64 rsdp = acpi_find_rsdp();
    if (!rsdp) return 0;

    u8  rev  = *(volatile u8  *)((uptr)rsdp + 15);
    u32 rsdt = *(volatile u32 *)((uptr)rsdp + 16);
    u64 xsdt = 0;
    if (rev >= 2) xsdt = *(volatile u64 *)((uptr)rsdp + 24);

    uptr table = 0;
    int  entry_bytes = 4;
    if (xsdt && (sizeof(uptr) == 8 || (xsdt >> 32) == 0)) {
        table = (uptr)xsdt;
        entry_bytes = 8;
    } else if (rsdt) {
        table = (uptr)rsdt;
        entry_bytes = 4;
    }
    if (!table) return 0;

    u32 len = *(volatile u32 *)(table + 4);
    if (len < 36 || len > 0x10000) return 0;

    int n = (int)((len - 36) / (u32)entry_bytes);
    for (int i = 0; i < n; i++) {
        u64 e;
        if (entry_bytes == 8) e = *(volatile u64 *)(table + 36 + (uptr)i * 8);
        else                  e = *(volatile u32 *)(table + 36 + (uptr)i * 4);
        if (!e) continue;
        if (sizeof(uptr) < 8 && (e >> 32)) continue;   /* unreachable up here */
        uptr t = (uptr)e;
        if (sig_is(t, sig, 4)) return t;
    }
    return 0;
}

/* ---- the MADT: where the APICs are and how the IRQs are wired ---------- */
static void parse_madt(void)
{
    for (int i = 0; i < 16; i++) { gsi_of_irq[i] = (u32)i; flags_of_irq[i] = 0; }

    uptr madt = acpi_find_table("APIC");
    if (!madt) return;

    u32 len = *(volatile u32 *)(madt + 4);
    if (len < 44 || len > 0x10000) return;

    lapic_base = (uptr)*(volatile u32 *)(madt + 36);
    madt_found = 1;

    uptr p   = madt + 44;
    uptr end = madt + len;
    while (p + 2 <= end) {
        u8 type = *(volatile u8 *)p;
        u8 elen = *(volatile u8 *)(p + 1);
        if (elen < 2) break;                    /* malformed - do not spin */
        if (p + elen > end) break;

        if (type == 0) {                        /* processor local APIC */
            u32 fl = *(volatile u32 *)(p + 4);
            u8  id = *(volatile u8 *)(p + 3);   /* the APIC id, not the ACPI id */
            if (fl & 1) {                       /* bit 0: this CPU is usable */
                if (cpu_count < 16) cpu_apic_ids[cpu_count] = id;
                cpu_count++;
            }
        } else if (type == 1) {                 /* I/O APIC */
            if (!ioapic_base) {
                ioapic_base     = (uptr)*(volatile u32 *)(p + 4);
                ioapic_gsi_base = *(volatile u32 *)(p + 8);
            }
        } else if (type == 2) {                 /* interrupt source override */
            u8  src = *(volatile u8  *)(p + 3);
            u32 gsi = *(volatile u32 *)(p + 4);
            u16 fl  = *(volatile u16 *)(p + 8);
            if (src < 16) { gsi_of_irq[src] = gsi; flags_of_irq[src] = fl; }
        } else if (type == 5) {                 /* 64-bit LAPIC address override */
            u64 a = *(volatile u64 *)(p + 4);
            if (a && (sizeof(uptr) == 8 || (a >> 32) == 0)) lapic_base = (uptr)a;
        }
        p += elen;
    }
}

/* ---- bringing it up ---------------------------------------------------- */
int apic_supported(void)
{
    u32 a, b, c, d;
    do_cpuid(1, &a, &b, &c, &d);
    return (d & (1u << 9)) ? 1 : 0;             /* EDX bit 9: APIC on chip */
}

/* The PIC must be silenced before the I/O APIC starts delivering, or both
 * controllers raise the same line and the handler runs twice. Masking every
 * input is enough; the remap idt.c already did stays valid for spurious
 * interrupts. */
static void pic_disable(void)
{
    zl_outb(0x21, 0xFF);
    zl_outb(0xA1, 0xFF);
}

/* Route one ISA IRQ to a CPU vector.
 *
 * The redirection entry is 64 bits across two 32-bit registers. The low half
 * carries the vector, delivery mode, polarity, trigger mode and the mask bit;
 * the high half carries the destination APIC ID in its top byte. Polarity and
 * trigger come from the MADT override when there is one - an ISA IRQ defaults
 * to edge-triggered and active-high, but a firmware that says otherwise is
 * telling us something we cannot guess. */
int apic_route_irq(int irq, int vector)
{
    if (!ioapic_base || irq < 0 || irq > 15) return 0;

    u32 gsi = gsi_of_irq[irq];
    if (gsi < ioapic_gsi_base) return 0;
    u32 pin = gsi - ioapic_gsi_base;
    if ((int)pin >= ioapic_pins) return 0;

    u16 fl = flags_of_irq[irq];
    u32 low = (u32)vector & 0xFF;               /* fixed delivery, physical */

    if ((fl & 0x3) == 3) low |= (1u << 13);     /* active low               */
    if (((fl >> 2) & 0x3) == 3) low |= (1u << 15); /* level triggered       */

    u32 dest = (mmio_r(lapic_base + LAPIC_ID) >> 24) & 0xFF;

    /* mask while we change it, then unmask - a half-written entry can fire */
    ioapic_write(IOAPIC_REDTBL(pin) + 0, low | (1u << 16));
    ioapic_write(IOAPIC_REDTBL(pin) + 1, dest << 24);
    ioapic_write(IOAPIC_REDTBL(pin) + 0, low);
    return 1;
}

int apic_mask_irq(int irq, int masked)
{
    if (!ioapic_base || irq < 0 || irq > 15) return 0;
    u32 pin = gsi_of_irq[irq] - ioapic_gsi_base;
    if ((int)pin >= ioapic_pins) return 0;
    u32 low = ioapic_read(IOAPIC_REDTBL(pin));
    if (masked) low |= (1u << 16); else low &= ~(1u << 16);
    ioapic_write(IOAPIC_REDTBL(pin), low);
    return 1;
}

/* End of interrupt. With the APIC in charge this replaces the PIC's 0x20 to
 * port 0x20 - and it must go to the LOCAL APIC, not the I/O APIC. */
void apic_eoi(void)
{
    if (apic_on) mmio_w(lapic_base + LAPIC_EOI, 0);
}

int apic_init(void)
{
    if (apic_on) return 1;
    if (!apic_supported()) return 0;

    parse_madt();

    /* The MSR is authoritative for the local APIC even when ACPI disagrees,
     * and it also carries the enable bit. */
    u64 base_msr = rdmsr(IA32_APIC_BASE);
    uptr msr_base = (uptr)(base_msr & 0xFFFFF000ull);
    if (msr_base) lapic_base = msr_base;
    if (!lapic_base) lapic_base = 0xFEE00000u;   /* the architectural default */

    wrmsr(IA32_APIC_BASE, base_msr | APIC_BASE_ENABLE);

    /* Software-enable the local APIC and give spurious interrupts a vector of
     * their own. Without bit 8 the APIC is powered but deaf. */
    mmio_w(lapic_base + LAPIC_SVR, 0x1FF);       /* vector 0xFF, enable */
    mmio_w(lapic_base + LAPIC_TPR, 0);           /* accept every priority */

    /* NO MADT MEANS NO INTERRUPT SOURCE OVERRIDES, AND THAT IS DISQUALIFYING.
     *
     * parse_madt() leaves gsi_of_irq[] as the identity when it cannot find the
     * table, so IRQ0 gets routed to I/O APIC pin 0. On a great many machines
     * ISA IRQ0 is overridden to GSI 2, and the MADT is the only thing that
     * says so. Route it to pin 0 there and the timer interrupt is delivered
     * nowhere - while pic_disable() below has already silenced the 8259 that
     * WAS delivering it. The result is a machine with no timer at all.
     *
     * MEASURED, and it is not hypothetical: booting the ISO under OVMF, the
     * RSDP is not in the legacy 0xE0000..0x100000 window that acpi_find_rsdp
     * scans - UEFI passes it through the EFI configuration table instead, and
     * the GRUB multiboot path never calls acpi_set_rsdp(). So madt_found is 0,
     * cpu_count is 0, and the boot log said "APIC: IRQs via I/O APIC at
     * 0xFEC00000, 0 CPU(s)" - a line that should have been read as a
     * contradiction, since an I/O APIC with zero CPUs to deliver to is not a
     * working configuration. zlOS then hung forever in the two-note boot chime,
     * because beep() waits on a tick counter that had stopped advancing.
     *
     * It went unnoticed because verify-iso.sh waited for "ready." and killed
     * QEMU one second later, and "ready." is printed BEFORE the chime.
     *
     * The 8259 is already remapped and working at this point, so declining is
     * free: apic_init returns 0, kernel.zl prints "no APIC - staying on the
     * legacy 8259 PIC", and everything downstream keeps its interrupts. A
     * guess at the routing would be the alternative, and a guess that is wrong
     * costs the whole machine. */
    if (!madt_found) return 0;

    if (!ioapic_base) ioapic_base = 0xFEC00000u; /* the usual place */
    ioapic_pins = (int)((ioapic_read(IOAPIC_REG_VER) >> 16) & 0xFF) + 1;
    if (ioapic_pins < 1 || ioapic_pins > 240) return 0;   /* nothing sane there */

    /* Mask every input first: whatever the firmware left routed is not ours,
     * and an unmasked line with a stale vector fires into a handler that does
     * not exist. */
    for (int pin = 0; pin < ioapic_pins; pin++)
        ioapic_write(IOAPIC_REDTBL(pin), 1u << 16);

    pic_disable();
    apic_on = 1;

    /* the three lines zlOS actually uses, on the vectors idt.c already set */
    apic_route_irq(0,  0x20);      /* PIT timer    */
    apic_route_irq(1,  0x21);      /* keyboard     */
    apic_route_irq(12, 0x2C);      /* PS/2 mouse   */
    return 1;
}

/* ---- diagnostics, so the shell can show what was found ------------------ */
int apic_active(void)      { return apic_on; }
u32 apic_lapic_base(void)  { return (u32)lapic_base; }
u32 apic_ioapic_base(void) { return (u32)ioapic_base; }
int apic_ioapic_pins(void) { return ioapic_pins; }
int apic_madt_ok(void)     { return madt_found; }
int apic_cpus(void)        { return cpu_count; }
u32 apic_rsdp(void)        { return (u32)acpi_find_rsdp(); }
int apic_cpu_id(int i)     { return (i >= 0 && i < cpu_count && i < 16) ? (int)cpu_apic_ids[i] : -1; }

/* Send an interprocessor interrupt. The ICR is two registers: the high half
 * carries the destination APIC id, and WRITING THE LOW HALF is what sends it,
 * so the order matters. Delivery status (bit 12) stays set until the message
 * has actually gone out. */
#define LAPIC_ICR_LO 0x300
#define LAPIC_ICR_HI 0x310

void apic_send_ipi(int dest_id, u32 icr_low)
{
    if (!apic_on) return;
    mmio_w(lapic_base + LAPIC_ICR_HI, (u32)dest_id << 24);
    mmio_w(lapic_base + LAPIC_ICR_LO, icr_low);
    for (int spin = 0; spin < 1000000; spin++)
        if (!(mmio_r(lapic_base + LAPIC_ICR_LO) & (1u << 12))) break;
}
int apic_gsi(int irq)      { return (irq >= 0 && irq < 16) ? (int)gsi_of_irq[irq] : -1; }
int apic_id(void)          { return apic_on ? (int)((mmio_r(lapic_base + LAPIC_ID) >> 24) & 0xFF) : -1; }
u32 apic_redtbl(int irq)
{
    if (!apic_on || irq < 0 || irq > 15) return 0;
    u32 pin = gsi_of_irq[irq] - ioapic_gsi_base;
    if ((int)pin >= ioapic_pins) return 0;
    return ioapic_read(IOAPIC_REDTBL(pin));
}
