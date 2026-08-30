/* Host proof for the real bounded crash recorder. The interrupt entry itself
 * needs QEMU, but every admission, commit, checksum, no-overwrite and bounded
 * formatting rule lives in crash.c and is exercised here under ASan/UBSan. */
#include <stdio.h>
#include <string.h>

#include "../../src/core/crash.h"

static int checks;
static int failures;
static char serial_line[896];

void ser_puts(const char *s)
{
    size_t have = strlen(serial_line);
    size_t left = sizeof(serial_line) - have - 1;
    strncat(serial_line, s, left);
}

static void check(int condition, const char *message)
{
    checks++;
    if (condition) printf("  ok   %s\n", message);
    else { printf("  FAIL %s\n", message); failures++; }
}

static int has(const char *text, const char *part)
{
    return strstr(text, part) != NULL;
}

int main(void)
{
    struct crash_record r;
    char full[896];
    struct crash_registers regs64 = {
        CRASH_REGS_64_ALL, 0,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10
    };
    struct crash_registers regs32 = {
        CRASH_REGS_32_ALL, 0,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    puts("crash record\n");
    crash_host_reset();
    check(!crash_snapshot(&r), "an empty recorder is not a valid crash");
    check(!crash_capture(32, 0, 0, 1, 2, 3, 4, 5, 0,
                         0x7000, 0, 0, 64, 0),
          "a vector outside the architectural table is refused");
    check(!crash_capture(14, 0, 0, 1, 2, 3, 4, 5, 6,
                         0x7000, 0, 0, 64, 0),
          "a page fault without its required error code is refused");
    check(!crash_capture(6, 0, 1, 1, 2, 3, 4, 5, 0,
                         0x7000, 0, 0, 64, 0),
          "a no-error exception cannot smuggle an error code");
    check(!crash_capture(6, 0, 0, 1, 2, 3, 4, 5, 0,
                         0x7000, 0, 0, 16, 0),
          "an unknown machine word size is refused");
    check(!crash_capture(6, 0, 0, 1, 2, 3, 4, 5, 0,
                         0, 0, 0, 64, 0),
          "a missing handler-stack identity is refused");
    {
        struct crash_registers partial = regs64;
        partial.mask ^= CRASH_REG_R15;
        check(!crash_capture(6, 0, 0, 1, 2, 3, 4, 5, 0,
                             0x7000, 0, 0, 64, &partial),
              "a partial register set cannot masquerade as a full frame");
    }
    check(!crash_snapshot(&r), "refused inputs leave no half-record behind");

    check(crash_capture(6, 0, 0,
                        0x1122334455667788ULL, 0x8, 0x202,
                        0x8877665544332211ULL, 0, 0xDEADBEEFULL,
                        0x7000, 0, 0, 64, &regs64),
          "invalid opcode commits one bounded record");
    check(crash_snapshot(&r), "the committed record validates");
    check(r.magic == CRASH_RECORD_MAGIC && r.version == CRASH_RECORD_VERSION,
          "the record carries its schema identity");
    check(r.vector == 6 && r.has_error == 0 && r.error_code == 0,
          "vector 6 records the no-error shape exactly");
    check(r.ip == 0x1122334455667788ULL &&
          r.sp == 0x8877665544332211ULL && r.word_bits == 64,
          "instruction, stack and architecture fields survive byte-for-byte");
    check(r.cr2 == 0, "CR2 is zero outside a page fault, never stale evidence");
    check(r.register_mask == CRASH_REGS_64_ALL &&
          r.ax == 0x01 && r.register_sp == 0x08 && r.r15 == 0x10,
          "all sixteen supplied general registers survive byte-for-byte");

    check(!crash_capture(13, 1, 7, 9, 8, 7, 6, 5, 0,
                         0x7000, 0, 0, 64, &regs64),
          "a second fault cannot overwrite the first crash");
    {
        struct crash_record again;
        check(crash_snapshot(&again) && again.vector == 6 && again.ip == r.ip,
              "the first committed record remains intact after recurrence");
    }

    check(crash_format(full, sizeof(full), &r),
          "the machine-readable line fits its fixed buffer");
    check(has(full, "ZLCRASH v=3 bytes=240 bits=64 regmask=0x000000000000ffff vec=6 haserr=0"),
          "the line identifies schema, architecture and vector");
    check(has(full, "ip=0x1122334455667788") &&
          has(full, "sp=0x8877665544332211"),
          "the line preserves fixed-width addresses");
    check(has(full, "cr2=0x0000000000000000") && has(full, " END\n"),
          "the line has an explicit CR2 and completion marker");
    check(has(full, "handlersp=0x0000000000007000") &&
          has(full, "stacklo=0x0000000000000000 stackhi=0x0000000000000000"),
          "the line identifies the active handler stack without inventing IST bounds");
    check(has(full, "ax=0x0000000000000001") &&
          has(full, "r15=0x0000000000000010"),
          "the line includes every validated general register");

    serial_line[0] = 0;
    crash_report();
    check(strcmp(serial_line, full) == 0,
          "the fault reporter emits exactly the validated record");

    {
        struct crash_record corrupt = r;
        corrupt.ip ^= 1;
        check(!crash_validate(&corrupt), "a changed field fails the checksum");
        corrupt = r;
        corrupt.checksum ^= 1;
        check(!crash_validate(&corrupt), "a changed checksum is refused");
        corrupt = r;
        corrupt.magic = 0;
        check(!crash_validate(&corrupt), "an uncommitted record is refused");
        corrupt = r;
        corrupt.register_mask ^= CRASH_REG_R15;
        check(!crash_validate(&corrupt), "a partial register mask is refused");
        corrupt = r;
        corrupt.register_mask = 0;
        corrupt.checksum = 0;
        check(!crash_validate(&corrupt),
              "register values cannot survive after their validity mask is cleared");
    }

    {
        struct { char text[24]; unsigned char guard[8]; } bounded;
        memset(&bounded, 0xA5, sizeof(bounded));
        check(!crash_format(bounded.text, sizeof(bounded.text), &r),
              "a short output buffer reports truncation");
        check(bounded.text[sizeof(bounded.text) - 1] == 0,
              "truncated output is still terminated");
        check(bounded.guard[0] == 0xA5 && bounded.guard[7] == 0xA5,
              "bounded formatting never crosses the caller's capacity");
    }

    crash_host_reset();
    check(crash_capture(14, 1, 5, 0x1000, 0x8, 0x246,
                        0x2000, 0, 0xCAFEB000, 0x6000, 0, 0, 32, &regs32),
          "a page fault accepts its architectural error and CR2");
    check(crash_snapshot(&r) && r.vector == 14 && r.error_code == 5 &&
          r.cr2 == 0xCAFEB000 && r.word_bits == 32,
          "page-fault evidence survives with 32-bit identity");
    check(r.register_mask == CRASH_REGS_32_ALL && r.r8 == 0 && r.r15 == 0,
          "a 32-bit frame admits exactly eight registers and zeros absent lanes");

    crash_host_reset();
    check(!crash_capture(8, 1, 1, 0, 8, 2, 0, 0, 0,
                         0x8800, 0x8000, 0x9000, 64, &regs64),
          "double fault refuses a nonzero architectural error code");
    check(!crash_capture(8, 1, 0, 0, 8, 2, 0, 0, 0,
                         0x7800, 0x8000, 0x9000, 64, &regs64),
          "double fault refuses a handler outside its emergency stack");
    check(crash_capture(8, 1, 0, 0, 8, 2, 0, 0, 0,
                        0x8800, 0x8000, 0x9000, 64, &regs64),
          "double fault commits only with a bounded emergency handler stack");
    check(crash_snapshot(&r) && r.vector == 8 && r.handler_sp == 0x8800 &&
          r.emergency_stack_low == 0x8000 && r.emergency_stack_high == 0x9000,
          "double-fault emergency-stack bounds survive byte-for-byte");

    printf("\n%d checks, %d failed\n", checks, failures);
    return failures ? 1 : 0;
}
