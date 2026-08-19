/* cpu.c - reading the processor itself.
 *
 * "A CPU driver" is not quite the same thing as a device driver: nothing here
 * is on a bus and nothing needs a doorbell. What the CPU has instead is a
 * self-description protocol - CPUID - and a large set of model-specific
 * registers, and a kernel that does not read them is guessing about the
 * machine it is running on.
 *
 * What zlOS actually needs from this:
 *
 *   - the feature bits, because whether SSE/AVX exist decides what the
 *     compositor may emit, and whether the APIC exists decides how interrupts
 *     are delivered at all
 *   - the topology: how many cores are really there, which is the prerequisite
 *     for ever starting a second one
 *   - the TSC, a cycle counter that ticks at a constant rate independent of
 *     the current clock speed on anything modern. That gives us a time source
 *     with nanosecond resolution instead of the PIT's 10 ms, which is what
 *     frame pacing and any real profiling need
 *   - the cache sizes, because a compositor that knows its L2 is 256 KiB can
 *     make sensible decisions about tile sizes
 *
 * Everything here is read-only. Writing MSRs to change frequency or voltage is
 * how you damage hardware, and it is not attempted.
 */

typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;

extern u32 idt_ticks(void);

static void do_cpuid(u32 leaf, u32 sub, u32 *a, u32 *b, u32 *c, u32 *d)
{
    __asm__ volatile("cpuid"
                     : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
                     : "a"(leaf), "c"(sub));
}

static u64 read_tsc(void)
{
    u32 lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((u64)hi << 32) | (u64)lo;
}

static u64 read_msr(u32 msr)
{
    u32 lo, hi;
    __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
    return ((u64)hi << 32) | (u64)lo;
}

/* The write half, which did not exist anywhere in the tree - read_msr above has
 * been here since the beginning with no counterpart, so every MSR-based feature
 * (RAPL power reporting, C-state limits, frequency control, turbo) was blocked
 * on three lines of inline asm.
 *
 * Deliberately not static and deliberately blunt: writing an MSR is a genuinely
 * privileged act with no validation possible from here. A wrong MSR number is a
 * general-protection fault at best and silently different CPU behaviour at
 * worst, so the checking belongs in the caller that knows what it is writing,
 * not in a wrapper pretending to make it safe. */
void write_msr(u32 msr, u64 val)
{
    __asm__ volatile("wrmsr" : : "c"(msr), "a"((u32)val), "d"((u32)(val >> 32)));
}

/* And a reader that is reachable from outside this file, for the same reason. */
u64 cpu_read_msr(u32 msr) { return read_msr(msr); }

/* ---- what the CPU says it is ------------------------------------------- */
u32 cpu_max_leaf(void)
{
    u32 a, b, c, d;
    do_cpuid(0, 0, &a, &b, &c, &d);
    return a;
}

/* vendor string is EBX:EDX:ECX from leaf 0 - a famously odd register order */
int cpu_vendor_byte(int i)
{
    if (i < 0 || i >= 12) return 0;
    u32 a, b, c, d;
    do_cpuid(0, 0, &a, &b, &c, &d);
    u32 w = (i < 4) ? b : (i < 8 ? d : c);
    return (int)((w >> ((i % 4) * 8)) & 0xFF);
}

/* family/model/stepping, with the extended fields folded in the way Intel
 * specifies - the base fields saturate and the extended ones carry the rest */
u32 cpu_signature(void)
{
    u32 a, b, c, d;
    do_cpuid(1, 0, &a, &b, &c, &d);
    return a;
}

int cpu_family(void)
{
    u32 s = cpu_signature();
    int base = (int)((s >> 8) & 0xF);
    int ext  = (int)((s >> 20) & 0xFF);
    return (base == 0xF) ? (base + ext) : base;
}

int cpu_model(void)
{
    u32 s = cpu_signature();
    int base = (int)((s >> 4) & 0xF);
    int ext  = (int)((s >> 16) & 0xF);
    int fam  = (int)((s >> 8) & 0xF);
    return (fam == 0x6 || fam == 0xF) ? ((ext << 4) | base) : base;
}

int cpu_stepping(void) { return (int)(cpu_signature() & 0xF); }

/* ---- features ----------------------------------------------------------
 * Named rather than returned as a raw bitmap, because a caller asking "is
 * there AVX2" should not have to know it is leaf 7, EBX, bit 5. */
u32 cpu_feat_edx(void) { u32 a,b,c,d; do_cpuid(1,0,&a,&b,&c,&d); return d; }
u32 cpu_feat_ecx(void) { u32 a,b,c,d; do_cpuid(1,0,&a,&b,&c,&d); return c; }
u32 cpu_feat7_ebx(void)
{
    if (cpu_max_leaf() < 7) return 0;
    u32 a,b,c,d; do_cpuid(7,0,&a,&b,&c,&d); return b;
}

int cpu_has_fpu(void)   { return (cpu_feat_edx() & (1u << 0))  ? 1 : 0; }
int cpu_has_tsc(void)   { return (cpu_feat_edx() & (1u << 4))  ? 1 : 0; }
int cpu_has_msr(void)   { return (cpu_feat_edx() & (1u << 5))  ? 1 : 0; }
int cpu_has_apic(void)  { return (cpu_feat_edx() & (1u << 9))  ? 1 : 0; }
int cpu_has_mmx(void)   { return (cpu_feat_edx() & (1u << 23)) ? 1 : 0; }
int cpu_has_sse(void)   { return (cpu_feat_edx() & (1u << 25)) ? 1 : 0; }
int cpu_has_sse2(void)  { return (cpu_feat_edx() & (1u << 26)) ? 1 : 0; }
int cpu_has_htt(void)   { return (cpu_feat_edx() & (1u << 28)) ? 1 : 0; }
int cpu_has_sse3(void)  { return (cpu_feat_ecx() & (1u << 0))  ? 1 : 0; }
int cpu_has_ssse3(void) { return (cpu_feat_ecx() & (1u << 9))  ? 1 : 0; }
int cpu_has_sse41(void) { return (cpu_feat_ecx() & (1u << 19)) ? 1 : 0; }
int cpu_has_sse42(void) { return (cpu_feat_ecx() & (1u << 20)) ? 1 : 0; }
int cpu_has_aes(void)   { return (cpu_feat_ecx() & (1u << 25)) ? 1 : 0; }
int cpu_has_avx(void)   { return (cpu_feat_ecx() & (1u << 28)) ? 1 : 0; }
int cpu_has_rdrand(void){ return (cpu_feat_ecx() & (1u << 30)) ? 1 : 0; }

/* RDRAND, with the retry the spec requires.
 *
 * The instruction sets CF when the value is good and CLEARS it when the
 * hardware pool is momentarily empty - and the register is UNDEFINED in that
 * case, not zero. Reading it without checking CF is a bug that produces
 * plausible-looking rubbish under load and perfect output on an idle machine,
 * which is the worst possible failure mode for a key. Intel's guidance is ten
 * retries; after that the caller must be told, not quietly handed a zero. */
int cpu_rdrand32(u32 *out)
{
    for (int i = 0; i < 10; i++) {
        u32 v = 0;
        unsigned char ok = 0;
        __asm__ volatile("rdrand %0; setc %1" : "=r"(v), "=qm"(ok) :: "cc");
        if (ok) { *out = v; return 1; }
    }
    return 0;
}
int cpu_has_hypervisor(void) { return (cpu_feat_ecx() & (1u << 31)) ? 1 : 0; }
int cpu_has_avx2(void)  { return (cpu_feat7_ebx() & (1u << 5))  ? 1 : 0; }
int cpu_has_bmi1(void)  { return (cpu_feat7_ebx() & (1u << 3))  ? 1 : 0; }
int cpu_has_bmi2(void)  { return (cpu_feat7_ebx() & (1u << 8))  ? 1 : 0; }

/* ---- topology ----------------------------------------------------------
 * Leaf 0x0B is the modern, authoritative answer: it enumerates levels, and
 * level type 1 is SMT (threads per core) while type 2 is Core. The old
 * leaf-1 EBX[23:16] field is a maximum, not a count, and lies on most parts. */
int cpu_threads(void)
{
    if (cpu_max_leaf() >= 0x0B) {
        u32 a, b, c, d;
        for (u32 lvl = 0; lvl < 4; lvl++) {
            do_cpuid(0x0B, lvl, &a, &b, &c, &d);
            if (((c >> 8) & 0xFF) == 2) return (int)(b & 0xFFFF);  /* core level */
        }
    }
    u32 a, b, c, d;
    do_cpuid(1, 0, &a, &b, &c, &d);
    int n = (int)((b >> 16) & 0xFF);
    return n ? n : 1;
}

int cpu_threads_per_core(void)
{
    if (cpu_max_leaf() >= 0x0B) {
        u32 a, b, c, d;
        do_cpuid(0x0B, 0, &a, &b, &c, &d);
        if (((c >> 8) & 0xFF) == 1) { int n = (int)(b & 0xFFFF); return n ? n : 1; }
    }
    return cpu_has_htt() ? 2 : 1;
}

int cpu_cores(void)
{
    int t = cpu_threads(), tpc = cpu_threads_per_core();
    return (tpc > 0) ? (t / tpc) : t;
}

/* the local APIC id of the core we are running on */
int cpu_apic_id(void)
{
    u32 a, b, c, d;
    do_cpuid(1, 0, &a, &b, &c, &d);
    return (int)((b >> 24) & 0xFF);
}

/* ---- caches ------------------------------------------------------------
 * Leaf 4 enumerates each cache: type in EAX[4:0] (1=data, 2=instruction,
 * 3=unified, 0=no more), level in EAX[7:5], and the geometry spread across
 * EBX and ECX. Size is (ways+1)*(partitions+1)*(line+1)*(sets+1). */
static int cache_field(int idx, int which)
{
    u32 a, b, c, d;
    do_cpuid(4, (u32)idx, &a, &b, &c, &d);
    int type = (int)(a & 0x1F);
    if (!type) return 0;
    if (which == 0) return type;
    if (which == 1) return (int)((a >> 5) & 0x7);          /* level */
    u32 ways  = ((b >> 22) & 0x3FF) + 1;
    u32 parts = ((b >> 12) & 0x3FF) + 1;
    u32 line  = (b & 0xFFF) + 1;
    u32 sets  = c + 1;
    return (int)((ways * parts * line * sets) / 1024);     /* KiB */
}

int cpu_cache_type(int i)  { return cache_field(i, 0); }
int cpu_cache_level(int i) { return cache_field(i, 1); }
int cpu_cache_kb(int i)    { return cache_field(i, 2); }

/* ---- the time stamp counter -------------------------------------------
 * An invariant TSC (leaf 0x80000007, EDX bit 8) runs at a constant rate no
 * matter what the core clock is doing, which is what makes it usable as a
 * clock at all. Calibrating it against the PIT gives us a real frequency, and
 * therefore sub-microsecond timing - the PIT alone only resolves 10 ms. */
int cpu_tsc_invariant(void)
{
    u32 a, b, c, d;
    do_cpuid(0x80000000, 0, &a, &b, &c, &d);
    if (a < 0x80000007) return 0;
    do_cpuid(0x80000007, 0, &a, &b, &c, &d);
    return (d & (1u << 8)) ? 1 : 0;
}

u64 cpu_tsc(void) { return read_tsc(); }
u32 cpu_tsc_lo(void) { return (u32)(read_tsc() & 0xFFFFFFFFu); }

static u32 tsc_khz_cached = 0;

/* Measure the TSC against the 100 Hz PIT. Ten ticks is 100 ms - long enough
 * that the tick quantisation is a 1% error rather than a 10% one, short enough
 * that nobody notices it at boot. Returns kHz. */
u32 cpu_tsc_khz(void)
{
    if (tsc_khz_cached) return tsc_khz_cached;
    if (!cpu_has_tsc()) return 0;

    u32 t0 = idt_ticks();
    /* wait for a tick edge so we start aligned */
    u32 guard = 0;
    while (idt_ticks() == t0 && ++guard < 200000000u) { }
    if (guard >= 200000000u) return 0;          /* the PIT is not running */

    u32 start_tick = idt_ticks();
    u64 start_tsc  = read_tsc();
    guard = 0;
    while (idt_ticks() - start_tick < 10 && ++guard < 2000000000u) { }
    u64 end_tsc = read_tsc();
    u32 elapsed = idt_ticks() - start_tick;
    if (!elapsed) return 0;

    u64 cycles = end_tsc - start_tsc;
    /* cycles over (elapsed * 10 ms) -> kHz is cycles / (elapsed * 10) */
    tsc_khz_cached = (u32)(cycles / ((u64)elapsed * 10u));
    return tsc_khz_cached;
}

u32 cpu_mhz(void) { u32 k = cpu_tsc_khz(); return k / 1000u; }

/* ---- a real microsecond delay ------------------------------------------
 *
 * The display driver needs waits from 100 us (link training) to 500 ms (the
 * panel power cycle), and it needs them to be RIGHT: under-waiting a panel
 * power cycle is the one hazard in that driver that damages hardware rather
 * than merely failing. Two things were being used before this existed and
 * neither works:
 *
 *   idt_ticks()  resolves 10 ms, so it cannot express 100 us at all, and it
 *                stops advancing entirely when interrupts are masked - which
 *                is exactly the state a modeset runs in.
 *   for (volatile int d = 0; d < N; d++)
 *                measured 0.576 ms for N=400000 on this box at -O2, where the
 *                caller wanted 200 ms. Wrong by 350x, and wrong by a different
 *                factor on every machine and optimisation level.
 *
 * The TSC is the answer: cpu_tsc_khz() calibrates it against the PIT once and
 * caches, and read_tsc() keeps working with interrupts off.
 *
 * No 64-bit division here on purpose. Dividing by 1000 to get MHz first turns
 * the cycle count into a 32x32->64 multiply; the alternative calls __udivdi3
 * from divmod.c, in software, inside a timing loop. Truncating 2304548 kHz to
 * 2304 MHz costs 0.02%, which is nothing against delays specified to 100 us.
 */
void cpu_delay_us(u32 us)
{
    if (!us) return;

    u32 khz = cpu_tsc_khz();
    if (khz >= 1000u) {
        u64 target = read_tsc() + (u64)us * (u64)(khz / 1000u);
        /* pause is a hint to the core that this is a spin-wait: it drops the
         * pipeline out of the memory-order speculation that makes a tight loop
         * expensive, and on a hyperthread it yields to the sibling. */
        while (read_tsc() < target) __asm__ volatile("pause");
        return;
    }

    /* No usable TSC. Never under-wait - over-waiting costs milliseconds,
     * under-waiting a T12 costs a panel. Anything the PIT can resolve goes to
     * the PIT, rounded up by two ticks. */
    if (us >= 20000u) {
        u32 t0 = idt_ticks();
        u32 ticks = (us / 10000u) + 2u;
        while (idt_ticks() - t0 < ticks) { }
        return;
    }
    /* Below 20 ms with no TSC, all that is left is a spin with no time base.
     * 20000 iterations per us is deliberately far more than any real CPU
     * needs - it is a pessimistic bound, not a measurement, and it only ever
     * runs on hardware with no invariant TSC. */
    for (volatile u32 d = 0; d < us * 20000u; d++) { }
}

void cpu_delay_ms(u32 ms)
{
    /* split so the us->cycles multiply cannot overflow 64 bits on a long wait */
    while (ms--) cpu_delay_us(1000u);
}

/* Monotonic milliseconds since boot. The display driver needs this to enforce
 * the panel's T12 power-cycle delay, which is measured from the last power-off
 * and is the one timing in that driver that damages hardware when violated.
 *
 * Wraps after ~49 days, which is fine: every consumer takes a difference, and
 * unsigned subtraction is correct across the wrap. The 64-bit divide here does
 * call into divmod.c, but this runs once per power transition, not in a loop. */
u32 cpu_now_ms(void)
{
    u32 khz = cpu_tsc_khz();
    if (khz >= 1000u) return (u32)(read_tsc() / (u64)khz);
    return idt_ticks() * 10u;         /* PIT fallback: 100 Hz, 10 ms a tick */
}

/* ---- are we in a VM? ---------------------------------------------------
 * Bit 31 of leaf 1 ECX is the hypervisor-present bit, and leaf 0x40000000
 * returns the hypervisor's own signature - "TCGTCGTCGTCG" for QEMU without
 * KVM, "KVMKVMKVM" with it. Worth knowing, because it tells the driver code
 * whether it is talking to emulated or real hardware. */
int cpu_hypervisor_byte(int i)
{
    if (i < 0 || i >= 12) return 0;
    if (!cpu_has_hypervisor()) return 0;
    u32 a, b, c, d;
    do_cpuid(0x40000000, 0, &a, &b, &c, &d);
    u32 w = (i < 4) ? b : (i < 8 ? c : d);
    return (int)((w >> ((i % 4) * 8)) & 0xFF);
}

/* ---- thermals, read-only ----------------------------------------------
 * IA32_THERM_STATUS bits [22:16] hold how many degrees BELOW the throttle
 * point the core currently is, so the actual temperature needs TjMax from
 * MSR 0x1A2. Both are Intel-specific and only meaningful when leaf 6 says
 * a digital thermal sensor exists. */
int cpu_has_therm(void)
{
    if (cpu_max_leaf() < 6) return 0;
    u32 a, b, c, d;
    do_cpuid(6, 0, &a, &b, &c, &d);
    return (a & 1u) ? 1 : 0;
}

int cpu_temp_c(void)
{
    if (!cpu_has_therm() || !cpu_has_msr()) return -1;
    u64 tj = read_msr(0x1A2);
    int tjmax = (int)((tj >> 16) & 0xFF);
    if (tjmax < 50 || tjmax > 130) tjmax = 100;      /* implausible - default */
    u64 st = read_msr(0x19C);
    if (!(st & (1u << 31))) return -1;               /* reading not valid */
    int delta = (int)((st >> 16) & 0x7F);
    return tjmax - delta;
}
