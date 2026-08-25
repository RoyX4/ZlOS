/* divmod.c - the 64-bit division a 32-bit CPU cannot do in one instruction.
 *
 * On i386 there is no "divide a 64-bit number" machine instruction, so gcc
 * emits calls to __divdi3 (signed /) and __moddi3 (signed %) and expects
 * libgcc to supply them. These two are the ONLY things the kernel borrowed
 * from libgcc - kernel.zl's `100 / 7` and `100 % 7` are what reach here.
 *
 * Providing them ourselves drops -lgcc entirely: nothing GNU is linked into
 * the kernel any more, only gcc-the-tool that compiled the C.
 *
 * The method is long division in binary - the same thing you do on paper in
 * base 10, in base 2. Walk the dividend from its top bit down; at each step
 * shift that bit into a running remainder, and whenever the remainder has
 * grown big enough to hold the divisor, subtract it and record a 1 in the
 * quotient. 64 steps, no hardware divide, no table.
 */

typedef unsigned long long u64;
typedef long long          s64;

/* unsigned 64/64: returns the quotient, writes the remainder through rem */
static u64 udivmod(u64 num, u64 den, u64 *rem)
{
    /* Divide by zero is undefined in C; on real division the CPU would
     * fault. There is no fault handler yet, so return all-ones - a value
     * wrong enough to be obvious rather than a silent 0. */
    if (den == 0) { if (rem) *rem = 0; return ~0ULL; }

    u64 quot = 0, r = 0;
    for (int i = 63; i >= 0; i--) {
        r = (r << 1) | ((num >> i) & 1ULL);   /* pull the next bit down */
        if (r >= den) {                        /* does the divisor fit?  */
            r -= den;
            quot |= (1ULL << i);               /* yes - record it        */
        }
    }
    if (rem) *rem = r;
    return quot;
}

/* Take the sign off, divide the magnitudes, put the sign back. Negating a
 * u64 is modular and well-defined even for the most-negative s64, which is
 * why the magnitude is taken as -(u64)x rather than (u64)(-x). */
s64 __divdi3(s64 a, s64 b)
{
    int neg = 0;
    u64 ua = (a < 0) ? -(u64)a : (u64)a;
    u64 ub = (b < 0) ? -(u64)b : (u64)b;
    if (a < 0) neg ^= 1;
    if (b < 0) neg ^= 1;
    u64 q = udivmod(ua, ub, 0);
    return neg ? -(s64)q : (s64)q;
}

/* The remainder takes the sign of the DIVIDEND, matching C's % and the
 * hardware idiv it stands in for: -7 % 3 == -1, 7 % -3 == 1. */
s64 __moddi3(s64 a, s64 b)
{
    int neg = (a < 0);
    u64 ua = (a < 0) ? -(u64)a : (u64)a;
    u64 ub = (b < 0) ? -(u64)b : (u64)b;
    u64 r;
    udivmod(ua, ub, &r);
    return neg ? -(s64)r : (s64)r;
}

/* The unsigned forms. gcc emits these for any 64-bit / or % on unsigned
 * operands - the TSC calibration in cpu.c divides a cycle count, and a cycle
 * count is emphatically unsigned. Same shift-and-subtract loop, without the
 * sign dance. */
u64 __udivdi3(u64 a, u64 b) { return udivmod(a, b, 0); }

u64 __umoddi3(u64 a, u64 b) { u64 r = 0; udivmod(a, b, &r); return r; }

/* Combined unsigned divide: quotient in the return, remainder through rem.
 * gcc emits this when a function does both / and % on the same unsigned
 * long long pair (interp_kernel.c kf_u64). */ 
u64 __udivmoddi4(u64 a, u64 b, u64 *rem) { return udivmod(a, b, rem); }
