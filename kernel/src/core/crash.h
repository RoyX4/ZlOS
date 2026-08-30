#ifndef ZLOS_CRASH_H
#define ZLOS_CRASH_H

typedef unsigned int crash_u32;
typedef unsigned short crash_u16;
typedef unsigned long long crash_u64;

#define CRASH_RECORD_MAGIC   0x5A4C4352u /* "ZLCR" */
#define CRASH_RECORD_VERSION 3u

#define CRASH_REG_AX  (1u << 0)
#define CRASH_REG_BX  (1u << 1)
#define CRASH_REG_CX  (1u << 2)
#define CRASH_REG_DX  (1u << 3)
#define CRASH_REG_SI  (1u << 4)
#define CRASH_REG_DI  (1u << 5)
#define CRASH_REG_BP  (1u << 6)
#define CRASH_REG_SP  (1u << 7)
#define CRASH_REG_R8  (1u << 8)
#define CRASH_REG_R9  (1u << 9)
#define CRASH_REG_R10 (1u << 10)
#define CRASH_REG_R11 (1u << 11)
#define CRASH_REG_R12 (1u << 12)
#define CRASH_REG_R13 (1u << 13)
#define CRASH_REG_R14 (1u << 14)
#define CRASH_REG_R15 (1u << 15)
#define CRASH_REGS_32_ALL 0x00FFu
#define CRASH_REGS_64_ALL 0xFFFFu

struct crash_registers {
    crash_u32 mask;
    crash_u32 reserved;
    crash_u64 ax, bx, cx, dx, si, di, bp, sp;
    crash_u64 r8, r9, r10, r11, r12, r13, r14, r15;
};

/* The first bounded machine-readable record zlOS creates after a CPU fault.
 * It is deliberately scalar and pointer-free so a recovery reader can copy it
 * without trusting the failed address space. This is RAM evidence, not yet a
 * durable dump: the machine still halts after the record is emitted. */
struct crash_record {
    crash_u32 magic;       /* committed last; zero means incomplete */
    crash_u16 version;
    crash_u16 bytes;
    crash_u32 vector;
    crash_u32 has_error;
    crash_u32 word_bits;
    crash_u32 register_mask;
    crash_u64 error_code;
    crash_u64 ip;
    crash_u64 cs;
    crash_u64 flags;
    crash_u64 sp;
    crash_u64 ss;
    crash_u64 cr2;
    crash_u64 handler_sp;
    crash_u64 emergency_stack_low;
    crash_u64 emergency_stack_high;
    crash_u64 ax, bx, cx, dx, si, di, bp, register_sp;
    crash_u64 r8, r9, r10, r11, r12, r13, r14, r15;
    crash_u64 checksum;
};

int crash_vector_has_error(crash_u32 vector);
int crash_capture(crash_u32 vector, crash_u32 has_error,
                  crash_u64 error_code, crash_u64 ip, crash_u64 cs,
                  crash_u64 flags, crash_u64 sp, crash_u64 ss,
                  crash_u64 cr2, crash_u64 handler_sp,
                  crash_u64 emergency_stack_low,
                  crash_u64 emergency_stack_high, crash_u32 word_bits,
                  const struct crash_registers *registers);
int crash_snapshot(struct crash_record *out);
int crash_validate(const struct crash_record *record);
int crash_format(char *out, crash_u32 capacity,
                 const struct crash_record *record);
void crash_report(void);

#ifdef CRASH_HOSTTEST
void crash_host_reset(void);
#endif

#endif
