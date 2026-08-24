/* smp.c - waking the other CPU cores.
 *
 * The machine has had four cores the whole time and zlOS has been using one.
 * Every other core is sitting in a wait-for-startup state, halted, since the
 * moment the firmware handed over.
 *
 * Waking one is not like starting a thread. There is no operating system on
 * that core to ask. You send it two interrupts by hand - an INIT, then a
 * STARTUP carrying a page number - and it begins executing 16-bit real mode
 * code at that page with nothing configured. Everything from there is
 * smp_trampoline.S's job; this file's job is to place that code, send the
 * signals, and count who answers.
 *
 * THE SEQUENCE, AND WHY IT IS SHAPED LIKE THIS
 * -------------------------------------------
 *   1. copy the trampoline below 1 MiB (a startup IPI can only address a page
 *      number in 0x00-0xFF, so the target must be under 1 MiB and page aligned)
 *   2. INIT IPI            - resets the target core into a known state
 *   3. wait 10 ms          - the spec's required settling time
 *   4. STARTUP IPI         - "begin executing at page N"
 *   5. wait, and if it did not answer, STARTUP again
 *
 * The second STARTUP is not superstition: Intel's own multiprocessor
 * initialisation algorithm sends two, because on some parts the first is lost
 * if it arrives while the core is still settling from INIT. Sending only one
 * works on most hardware and mysteriously fails on some, which is the worst
 * kind of bug to leave in.
 */

typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;

/* Pointer-sized. `unsigned long` is 4 bytes under buildefi.sh's LLP64 target,
 * so putting smp_ap_main's address through it truncated the entry point every
 * AP jumps to - see the ENTRY_PTR store below. */
#if defined(ZL_64) || defined(__x86_64__)
typedef unsigned long long uptr;
#else
typedef unsigned int       uptr;
#endif

extern u32  idt_ticks(void);
extern int  apic_active(void);
extern int  apic_init(void);
extern int  apic_cpus(void);
extern int  apic_cpu_id(int i);
extern void apic_send_ipi(int dest_id, u32 icr_low);
extern void apic_enable_local(void);

/* Two trampolines, because an AP has to arrive in whichever mode the kernel is
 * already running in. The 32-bit one stops at protected mode; the 64-bit one
 * goes all the way into long mode, reusing the boot processor's page tables. */
#if defined(ZL_64)
extern const u8 smp_tramp64_start[];
extern const u8 smp_tramp64_end[];
#define TRAMP_BEGIN smp_tramp64_start
#define TRAMP_END   smp_tramp64_end
#else
extern const u8 smp_tramp_start[];
extern const u8 smp_tramp_end[];
#define TRAMP_BEGIN smp_tramp_start
#define TRAMP_END   smp_tramp_end
#endif

#define TRAMP_ADDR   0x9000u        /* must match smp_trampoline.S */
#define ENTRY_PTR    0x8FF0u
#define CR3_PTR      0x8FE0u        /* the BSP leaves its page tables here */
#define IDT_PTR      0x8FC0u        /* descriptor APs load before sti/hlt  */
#define TRAMP_PAGE   0x09           /* 0x9000 >> 12 - what the SIPI carries */

/* ICR delivery modes. Bit 14 is "assert", bits 10:8 the mode. */
#define IPI_INIT     0x00004500u
#define IPI_SIPI     0x00004600u    /* low byte is the page number */

static volatile int ap_online   = 0;
static volatile int ap_last_id  = -1;
static volatile u32 ap_mask     = 0;   /* which APIC ids answered */
static int smp_started = 0;
static int smp_attempted = 0;

/* Each woken core lands here, on its own stack, in 32-bit protected mode.
 *
 * It deliberately does very little. Handing an application processor real work
 * needs per-core scheduler state and locking around everything it touches, and
 * neither exists yet - a core that started running the shell's data structures
 * concurrently would corrupt them within milliseconds. So it registers itself,
 * proves it is alive, and halts. That is a real milestone and an honest stopping
 * point: the cores are awake and reachable, and what they are allowed to touch
 * is a separate problem. */
/* ---- band rendering: the one job an AP is allowed to do --------------------
 *
 * An AP still may not touch the console, the scheduler or any driver - none of
 * them are protected by a lock. What it CAN do is write a rectangle of the
 * back buffer, because fb.c hands out bands that are disjoint by construction:
 * band i owns rows [y0,y1) and no other band can name those rows. There is
 * nothing shared to lock, which is why this is safe and why nothing else is.
 *
 * APs park in `sti; hlt` and are woken by a fixed 0xF1 IPI after their slot is
 * published. The check is made with interrupts disabled and `sti; hlt` is one
 * instruction pair, closing the lost-wakeup race between observing an empty
 * slot and halting. Idle APs therefore consume no core.
 *
 * ONE CACHE LINE PER SLOT. Two cores writing `done` flags in the same 64-byte
 * line ping-pong that line between them on every store, and the loop ends up
 * slower than serial with nothing in the code to show why. This is the single
 * most common way band rendering fails to pay.
 */
#define SMP_LINE  64
#define SMP_SLOTS 8

typedef void (*fb_band_fn)(void *ctx, int y0, int y1);
extern void fb_par_hook(void (*d)(fb_band_fn, void *, const int *, int), int n);

struct ap_slot {
    volatile u32 seq;          /* bumped by the BSP: here is work            */
    volatile u32 done;         /* bumped by the AP: finished it              */
    volatile u32 wakes;
    int apic_id;
    int monitor_wait;
    fb_band_fn   fn;
    void        *ctx;
    volatile int y0, y1;
    char pad[SMP_LINE * 2];    /* keep the next slot off this line           */
} __attribute__((aligned(SMP_LINE)));

static struct ap_slot ap_slots[SMP_SLOTS];
static volatile int   ap_slots_live = 0;   /* how many APs are in the spin   */

struct idtr_image {
    u16 limit;
    uptr base;
} __attribute__((packed));

static void smp_pause(void) { __asm__ volatile("pause" ::: "memory"); }
static void smp_monitor(volatile u32 *address)
{
    __asm__ volatile("monitor" :: "a"(address), "c"(0), "d"(0) : "memory");
}
static void smp_mwait(void)
{
    __asm__ volatile("mwait" :: "a"(0), "c"(0) : "memory");
}
void vmm_sync_framebuffer_pat(void);
void apic_send_ipi(int dest_id, u32 icr_low);

void smp_ap_main(void)
{
    /* IA32_PAT is per logical CPU.  The BSP may have created one WC selector
     * before AP startup; no AP may publish itself or run a present band until
     * it has the identical table. */
    vmm_sync_framebuffer_pat();
    apic_enable_local();
    u32 a, b, c, d;
    __asm__ volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(1), "c"(0));
    int id = (int)((b >> 24) & 0xFF);

    ap_last_id = id;
    if (id < 32) ap_mask |= (1u << id);
    ap_online++;

    /* Claim a slot by arrival order, not by APIC id - ids are not dense and a
     * sparse array would leave the dispatcher spinning on a slot nobody owns. */
    int slot = ap_online;              /* 1..n-1; the BSP is band 0          */
    if (slot < 1 || slot >= SMP_SLOTS) {
        for (;;) __asm__ volatile("cli; hlt");   /* more cores than slots    */
    }
    ap_slots[slot].seq = ap_slots[slot].done = 0;
    ap_slots[slot].wakes = 0;
    ap_slots[slot].apic_id = id;
    /* MONITOR/MWAIT is a store-driven sleep and avoids depending on a fixed
     * IPI for the normal worker wake. Comet Lake and QEMU -cpu host expose it;
     * the IDT/LAPIC path remains the bounded fallback for older x86 CPUs. */
    ap_slots[slot].monitor_wait = (c & (1u << 3)) ? 1 : 0;
    __sync_synchronize();
    ap_slots_live = slot;

    u32 seen = 0;
    for (;;) {
        if (ap_slots[slot].monitor_wait) {
            __asm__ volatile("sti" ::: "memory");
            while (ap_slots[slot].seq == seen) {
                smp_monitor(&ap_slots[slot].seq);
                if (ap_slots[slot].seq == seen) smp_mwait();
            }
        } else {
            __asm__ volatile("cli" ::: "memory");
            if (ap_slots[slot].seq == seen) {
                __asm__ volatile("sti; hlt" ::: "memory");
                continue;
            }
            __asm__ volatile("sti" ::: "memory");
        }
        seen = ap_slots[slot].seq;
        ap_slots[slot].wakes++;
        ap_slots[slot].fn(ap_slots[slot].ctx, ap_slots[slot].y0, ap_slots[slot].y1);
        /* x86 is store-ordered, so the band's writes are visible before this
         * one - the barrier is against the COMPILER reordering them. */
        __sync_synchronize();
        ap_slots[slot].done = seen;
    }
}

/* Hand one banded job to the parked cores and wait for all of them.
 *
 * Band 0 runs HERE, on the calling core, rather than the BSP sitting in the
 * barrier watching three cores work. */
static void smp_band_dispatch(fb_band_fn fn, void *ctx, const int *edges, int n)
{
    for (int i = 1; i < n; i++) {
        ap_slots[i].fn = fn;
        ap_slots[i].ctx = ctx;
        ap_slots[i].y0 = edges[i];
        ap_slots[i].y1 = edges[i + 1];
        __sync_synchronize();
        ap_slots[i].seq++;
        /* KVM may expose MONITOR/MWAIT but implement MWAIT as an interrupt
         * sleep rather than a cache-store wake.  Always send the fixed IPI as
         * well: real silicon normally wakes from the monitored store, while
         * the interrupt makes the same contract reliable under that VMM
         * implementation and remains required by the HLT fallback. */
        apic_send_ipi(ap_slots[i].apic_id, 0xF1u);
    }
    fn(ctx, edges[0], edges[1]);
    /* THE BARRIER, and it is not optional: fb_present must not blit a row a
     * core is still writing into. */
    for (int i = 1; i < n; i++)
        while (ap_slots[i].done != ap_slots[i].seq) smp_pause();
    __sync_synchronize();
}

static void wait_ticks(u32 n)
{
    u32 t0 = idt_ticks();
    long spins = 100000000L;
    while (idt_ticks() - t0 < n && spins-- > 0) { }
}

int smp_cpu_count(void) { return apic_cpus(); }
int smp_online(void)    { return ap_online + 1; }   /* +1 for the boot core */
int smp_bands(void)     { return ap_slots_live + 1; }
u32 smp_band_wakes(void)
{
    u32 n = 0;
    for (int i = 1; i <= ap_slots_live; i++) n += ap_slots[i].wakes;
    return n;
}
int smp_last_id(void)   { return ap_last_id; }
u32 smp_mask(void)      { return ap_mask | 1u; }
int smp_ready(void)     { return smp_started; }
int smp_tramp_size(void){ return (int)(TRAMP_END - TRAMP_BEGIN); }
int smp_supported(void) { return 1; }

/* Wake every application processor the MADT listed. Returns how many cores are
 * online afterwards, including this one. */
int smp_start(void)
{
    if (smp_attempted) return smp_online();
    smp_attempted = 1;

    if (!apic_active() && !apic_init()) return 1;   /* no APIC, no SMP */

    int total = apic_cpus();
    if (total <= 1) { smp_started = 1; return 1; }

    /* 1. put the trampoline where a startup IPI can point at it */
    u32 size = (u32)(TRAMP_END - TRAMP_BEGIN);
    if (size == 0 || size > 0x0FF0) return 1;       /* would not fit below ENTRY_PTR */
    for (u32 i = 0; i < size; i++)
        *(volatile u8 *)(uptr)(TRAMP_ADDR + i) = TRAMP_BEGIN[i];

    /* INIT resets an AP's interrupt-table register.  The workers deliberately
     * sleep with interrupts enabled and wake on vector 0xF1, so publishing the
     * job and sending the IPI is not sufficient: without a valid IDTR an AP
     * can service the first job only if it races the halt, then faults or
     * triple-faults on the first real wake while the BSP spins at the barrier.
     * The IDT is read-only after boot and identity-mapped on both builds, so
     * every core may safely load the BSP's descriptor before enabling IRQs. */
    __asm__ volatile("sidt %0" : "=m"(*(struct idtr_image *)(uptr)IDT_PTR));

    /* Values the trampoline reads out of memory rather than having patched into
     * it. The 64-bit path also needs our page tables: sharing CR3 means there
     * is only ever one address space to keep correct. */
#if defined(ZL_64)
    unsigned long long cr3;
    __asm__ volatile("movq %%cr3, %0" : "=r"(cr3));
    *(volatile unsigned long long *)(uptr)CR3_PTR = cr3;
    /* (uptr), NOT (unsigned long). This is the exact defect CLAUDE.md records
     * under "As a cast, which the struct fix did NOT cover": the destination
     * was already 64 bits wide, and it was being handed a value truncated to
     * 32 on the way in. Every application processor jumps to whatever address
     * lands here, so a kernel image the firmware placed above 4 GiB sent all
     * three APs into nothing. Below 4 GiB it is harmless, which is why QEMU
     * never showed it - the same reason the other five sites hid. */
    *(volatile unsigned long long *)(uptr)ENTRY_PTR =
        (unsigned long long)(uptr)smp_ap_main;
#else
    *(volatile u32 *)(uptr)ENTRY_PTR = (u32)(uptr)smp_ap_main;
#endif

    /* our own APIC id - never send ourselves a startup IPI */
    u32 a, b, c, d;
    __asm__ volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(1), "c"(0));
    int self = (int)((b >> 24) & 0xFF);

    for (int i = 0; i < total && i < 16; i++) {
        int id = apic_cpu_id(i);
        if (id < 0 || id == self) continue;

        int before = ap_online;

        apic_send_ipi(id, IPI_INIT);
        wait_ticks(2);                       /* ~20 ms, spec asks for 10 */

        apic_send_ipi(id, IPI_SIPI | TRAMP_PAGE);
        wait_ticks(1);
        if (ap_online == before) {           /* Intel's algorithm: try twice */
            apic_send_ipi(id, IPI_SIPI | TRAMP_PAGE);
            wait_ticks(3);
        }
    }

    smp_started = 1;

    /* THE CORES ARE AWAKE, so let fb.c use them. Not before: fb_par_hook with
     * a NULL dispatcher is the default and means "run every band on the
     * calling core", which is exactly the behaviour of every build that never
     * calls this. So nothing about the normal boot changes - verify.sh boots
     * -smp 1 and never gets here at all. */
    int bands = ap_slots_live + 1;
    if (bands > 1) fb_par_hook(smp_band_dispatch, bands);
    return smp_online();
}
