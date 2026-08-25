#ifndef ZLOS_CRASH_H
#define ZLOS_CRASH_H

typedef unsigned int crash_u32;
typedef unsigned short crash_u16;
typedef unsigned long long crash_u64;

#define CRASH_RECORD_MAGIC   0x5A4C4352u /* "ZLCR" */
#define CRASH_RECORD_VERSION 1u

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
    crash_u32 reserved;
    crash_u64 error_code;
    crash_u64 ip;
    crash_u64 cs;
    crash_u64 flags;
    crash_u64 sp;
    crash_u64 ss;
    crash_u64 cr2;
    crash_u64 checksum;
};

int crash_vector_has_error(crash_u32 vector);
int crash_capture(crash_u32 vector, crash_u32 has_error,
                  crash_u64 error_code, crash_u64 ip, crash_u64 cs,
                  crash_u64 flags, crash_u64 sp, crash_u64 ss,
                  crash_u64 cr2, crash_u32 word_bits);
int crash_snapshot(struct crash_record *out);
int crash_validate(const struct crash_record *record);
int crash_format(char *out, crash_u32 capacity,
                 const struct crash_record *record);
void crash_report(void);

#ifdef CRASH_HOSTTEST
void crash_host_reset(void);
#endif

#endif
