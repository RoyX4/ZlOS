/* zllogtest.c - run the shipping recorder against a file-backed fake USB disk.
 *
 * The fake derives the one writable range from the image's GPT, independently
 * checks every WRITE, and can cut power halfway through either final metadata
 * write.  ../../src/core/zllog.c is included rather than copied: this is the exact kernel
 * implementation with only the block device, TSC and tick sources replaced.
 */
#define _FILE_OFFSET_BITS 64
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../../src/core/zllog.c"

enum tear_mode {
    TEAR_NONE,
    TEAR_FINAL_SUPER,
    TEAR_COMPLETE_SLOT,
    EXPECT_MOUNT_REFUSAL,
    EXPECT_AUTO_RETRY,
    EXPECT_ROTATION,
    EXPECT_SELFTEST
};

static FILE *fake_disk;
static u32 fake_blocks;
static u32 writable_first;
static u32 writable_blocks;
static unsigned fake_writes;
static unsigned fake_syncs;
static unsigned outside_writes;
static int fake_dead;
static int fake_init_failures;
static unsigned ready_persists;
static int tear_triggered;
static enum tear_mode requested_tear;
static u64 fake_tsc = 1000;
static u32 fake_ticks;

static const u8 expected_type_guid[16] = {
    0xd4,0x51,0xbc,0xa2, 0x5d,0x22, 0xd4,0x4a,
    0x8d,0xb5,0xb0,0x09,0x59,0x53,0xaa,0x19
};

static int raw_read(u64 byte_offset, void *dst, size_t bytes)
{
    if (fseeko(fake_disk, (off_t)byte_offset, SEEK_SET) != 0) return 0;
    return fread(dst, 1, bytes, fake_disk) == bytes;
}

static int raw_write(u64 byte_offset, const void *src, size_t bytes)
{
    if (fseeko(fake_disk, (off_t)byte_offset, SEEK_SET) != 0) return 0;
    return fwrite(src, 1, bytes, fake_disk) == bytes;
}

static int discover_writable_partition(void)
{
    u8 header[512], entries[16384];
    if (!raw_read(512, header, sizeof header) || memcmp(header, "EFI PART", 8))
        return 0;
    u64 entries_lba = get64(header + 72);
    u32 entry_count = get32(header + 80);
    u32 entry_bytes = get32(header + 84);
    if (!entries_lba || entry_count != 128 || entry_bytes != 128)
        return 0;
    if (!raw_read(entries_lba * 512u, entries, sizeof entries)) return 0;

    unsigned matches = 0;
    for (u32 i = 0; i < entry_count; i++) {
        const u8 *entry = entries + i * entry_bytes;
        if (!memcmp(entry, expected_type_guid, 16)) {
            u64 first = get64(entry + 32);
            u64 last = get64(entry + 40);
            if (!first || last < first || last >= fake_blocks ||
                first > 0xffffffffu || last > 0xffffffffu)
                return 0;
            writable_first = (u32)first;
            writable_blocks = (u32)(last - first + 1);
            matches++;
        }
    }
    u64 bytes = (u64)writable_blocks * 512u;
    return matches == 1 && bytes >= ZLLOG_PART_MIN_BYTES &&
           bytes <= ZLLOG_PART_MAX_BYTES;
}

u64 cpu_tsc(void)
{
    fake_tsc += 101;
    return fake_tsc;
}

u32 cpu_tsc_khz(void) { return 1000000u; }

u32 idt_ticks(void)
{
    fake_ticks += 100;
    return fake_ticks;
}

int xhci_msc_init(void)
{
    if (fake_init_failures > 0) { fake_init_failures--; return 0; }
    return !fake_dead;
}
int xhci_msc_read_capacity(void) { return !fake_dead; }
u32 xhci_msc_blocks(void) { return fake_blocks; }
u32 xhci_msc_blocksize(void) { return 512; }

int xhci_msc_read_blocks(u32 lba, void *dst, u32 count)
{
    if (fake_dead || !count || lba >= fake_blocks || count > fake_blocks - lba)
        return 0;
    return raw_read((u64)lba * 512u, dst, (size_t)count * 512u);
}

static int is_final_super(const u8 *src, u32 bytes)
{
    return bytes == ZLLOG_SUPER_BYTES && !memcmp(src, super_magic, 8) &&
           get32(src + 24) == ZLLOG_NONE_SLOT;
}

static int is_complete_slot(const u8 *src, u32 bytes)
{
    return bytes == ZLLOG_SLOT_HEADER && !memcmp(src, slot_magic, 8) &&
           get32(src + 16) == 2;
}

int xhci_msc_write_blocks(u32 lba, const void *src, u32 count)
{
    const u32 bytes = count * 512u;
    fake_writes++;
    if (fake_dead || !count || lba < writable_first ||
        count > writable_blocks || lba - writable_first > writable_blocks - count) {
        outside_writes++;
        return 0;
    }

    const int tear_here =
        (requested_tear == TEAR_FINAL_SUPER && is_final_super((const u8 *)src, bytes)) ||
        (requested_tear == TEAR_COMPLETE_SLOT && is_complete_slot((const u8 *)src, bytes));
    if (tear_here) {
        /* Cutting the 4 KiB transfer at 2 KiB is not adversarial: all v1
         * metadata lives in its first 164/132 bytes, so that would publish a
         * completely valid new header. Cut inside the used header instead. */
        size_t torn_bytes = requested_tear == TEAR_FINAL_SUPER ? 82u : 66u;
        if (!raw_write((u64)lba * 512u, src, torn_bytes)) return 0;
        if (fflush(fake_disk) != 0 || fsync(fileno(fake_disk)) != 0) return 0;
        tear_triggered = 1;
        fake_dead = 1;
        return 0;
    }

    return raw_write((u64)lba * 512u, src, bytes);
}

int xhci_msc_sync_cache(void)
{
    fake_syncs++;
    if (fake_dead) return 0;
    return fflush(fake_disk) == 0 && fsync(fileno(fake_disk)) == 0;
}

int xhci_msc_last_result(void) { return fake_dead ? 1 : 0; }
int xhci_msc_sense_key(void) { return fake_dead ? 3 : 0; }
int xhci_msc_sense_asc(void) { return fake_dead ? 0x0c : 0; }
int xhci_msc_sense_ascq(void) { return fake_dead ? 2 : 0; }
int xhci_msc_init_stage(void) { return fake_dead || fake_init_failures ? 3 : 9; }
int xhci_msc_init_port(void) { return 4; }
int xhci_msc_init_slot(void) { return 1; }
int xhci_msc_init_cc(void) { return fake_dead ? 0 : 1; }
int xhci_msc_init_vid(void) { return 0x0718; }
int xhci_msc_init_pid(void) { return 0x067d; }
u32 xhci_portsc(int port) { return port == 4 ? 0x00000c03u : 0u; }
u32 xhci_usbsts(void) { return fake_dead ? 1u : 0u; }
u32 xhci_usbcmd(void) { return 5u; }
void efi_persist_storage_ready(u32 ticks)
{
    (void)ticks;
    ready_persists++;
}

static enum tear_mode parse_tear(const char *name)
{
    if (!strcmp(name, "none")) return TEAR_NONE;
    if (!strcmp(name, "final-super")) return TEAR_FINAL_SUPER;
    if (!strcmp(name, "complete-slot")) return TEAR_COMPLETE_SLOT;
    if (!strcmp(name, "refuse")) return EXPECT_MOUNT_REFUSAL;
    if (!strcmp(name, "retry")) return EXPECT_AUTO_RETRY;
    if (!strcmp(name, "rotate")) return EXPECT_ROTATION;
    if (!strcmp(name, "selftest")) return EXPECT_SELFTEST;
    fprintf(stderr, "unknown tear mode: %s\n", name);
    exit(64);
}

static int recorder_core_selftest(void)
{
    /* Hot totals carry past 32 bits without libatomic/libgcc helpers. */
    counter_total[ZLLOG_C_PRESENT_BYTES] = 0xfffffff0u;
    counter_total_high[ZLLOG_C_PRESENT_BYTES] = 7u;
    zllog_counter_add(ZLLOG_C_PRESENT_BYTES, 32u);
    if (atomic_counter_read(&counter_total[ZLLOG_C_PRESENT_BYTES],
                            &counter_total_high[ZLLOG_C_PRESENT_BYTES]) !=
        (((u64)8u << 32) | 16u)) return 0;

    /* Printable identity is redacted while event timing/type survives. */
    zllog_input_event(3u, (unsigned)'s', 1u);
    u32 last = (ram_head + ram_count - 1u) % ZLLOG_RAM_RECORDS;
    if (get64(ram_records[last] + 32) != 0u) return 0;

    /* Runtime command text stops at the boot-ready boundary. */
    const char *ready = "system ready\n";
    while (*ready) zllog_putc(*ready++);
    u32 before_secret = ram_count;
    const char *secret = "never-persist-this\n";
    while (*secret) zllog_putc(*secret++);
    if (ram_count != before_secret || text_capture) return 0;

    /* Pointer records retain pixel position and the hardware-boundary stamp,
     * while ordinary motion remains sampled by the shipping recorder. */
    zllog_pointer_event(123u, 456u, 1u, 2u, 0x89abcdefu, 77u);
    u32 pointer = (ram_head + ram_count - 1u) % ZLLOG_RAM_RECORDS;
    if (get16(ram_records[pointer] + 20) != ZLLOG_EV_POINTER ||
        get64(ram_records[pointer] + 24) != ((u64)123u | ((u64)456u << 32)) ||
        get64(ram_records[pointer] + 32) != ((u64)1u | ((u64)2u << 32)) ||
        get64(ram_records[pointer] + 40) !=
            ((u64)0x89abcdefu | ((u64)77u << 32))) return 0;

    /* Audit records preserve typed identity, correlation, signed results and
     * errors without storing names, buffers or other user content. */
    zllog_lifecycle(ZLLOG_SUB_SCHED, ZLLOG_OBJ_PROCESS, 7u,
                    ZLLOG_LIFE_START, 0u, 4096u);
    u32 life = (ram_head + ram_count - 1u) % ZLLOG_RAM_RECORDS;
    if (get16(ram_records[life] + 20) != ZLLOG_EV_LIFECYCLE ||
        get64(ram_records[life] + 24) != ((u64)ZLLOG_OBJ_PROCESS << 32 | 7u) ||
        get64(ram_records[life] + 32) != ((u64)ZLLOG_LIFE_START << 32)) return 0;
    u32 operation = zllog_operation_begin(ZLLOG_SUB_FS, ZLLOG_OBJ_PROCESS,
                                           7u, ZLLOG_OP_FILE_READ, 3u);
    zllog_operation_result(ZLLOG_SUB_FS, operation, ZLLOG_OP_FILE_READ,
                           -5, 5u, 128u);
    u32 result = (ram_head + ram_count - 1u) % ZLLOG_RAM_RECORDS;
    if (!operation || get16(ram_records[result] + 20) != ZLLOG_EV_OPERATION_RESULT ||
        get64(ram_records[result] + 24) != operation ||
        (long long)get64(ram_records[result] + 32) != -5 ||
        get32(ram_records[result] + 48) != 128u) return 0;

    /* The emergency lane is reusable, not a 64-event one-shot buffer. */
    for (u32 round = 0; round < 3u; round++) {
        for (u32 i = 0; i < ZLLOG_IRQ_RECORDS; i++)
            zllog_event_irq(ZLLOG_SUB_IRQ, ZLLOG_EV_FAULT, ZLLOG_ERROR,
                            round, i, 0u);
        drain_irq_records();
        if (__atomic_load_n(&irq_write, __ATOMIC_ACQUIRE) !=
            __atomic_load_n(&irq_read, __ATOMIC_ACQUIRE)) return 0;
    }

    /* Sampling pressure cannot consume the reserved fatal/error capacity. */
    for (u32 i = 0; i < 2200u; i++)
        zllog_frame(1u, 1u, 1u, 0u, 1u, 4u, 0u, 1u, 1u);
    if (ram_count != 2048u || !ram_dropped) return 0;
    zllog_event(ZLLOG_SUB_CPU, ZLLOG_EV_FAULT, ZLLOG_FATAL, 14u, 0u, 0u);
    if (ram_count != 2049u) return 0;
    return 1;
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr, "usage: %s DISK-IMAGE BOOT-TAG none|final-super|complete-slot|refuse|retry|rotate|selftest\n", argv[0]);
        return 64;
    }
    char *end = 0;
    unsigned long tag = strtoul(argv[2], &end, 0);
    if (!end || *end || tag > 0xffffffffu) {
        fprintf(stderr, "invalid boot tag: %s\n", argv[2]);
        return 64;
    }
    requested_tear = parse_tear(argv[3]);
    if (requested_tear == EXPECT_SELFTEST) {
        int ok = recorder_core_selftest();
        printf("recorder_core_selftest=%d buffered=%u\n", ok, zllog_buffered());
        return ok ? 0 : 1;
    }
    if (requested_tear == EXPECT_AUTO_RETRY) fake_init_failures = 1;

    fake_disk = fopen(argv[1], "r+b");
    if (!fake_disk) {
        fprintf(stderr, "open %s: %s\n", argv[1], strerror(errno));
        return 1;
    }
    struct stat st;
    if (fstat(fileno(fake_disk), &st) != 0 || st.st_size <= 0 || st.st_size % 512) {
        fprintf(stderr, "image size is not whole 512-byte blocks\n");
        return 1;
    }
    fake_blocks = (u32)((u64)st.st_size / 512u);
    if (!discover_writable_partition()) {
        fprintf(stderr, "could not derive the exact 64 MiB ZLLOG GPT range\n");
        return 1;
    }

    zllog_milestone(0x100u, (unsigned)tag);
    zllog_lifecycle(ZLLOG_SUB_SCHED, ZLLOG_OBJ_PROCESS, tag,
                    ZLLOG_LIFE_START, 0u, 4096u);
    u32 audit = zllog_operation_begin(
        ZLLOG_SUB_SYSCALL, ZLLOG_OBJ_PROCESS, tag,
        ZLLOG_OP_SYSCALL_BASE + 2u, 2u);
    zllog_operation_result(ZLLOG_SUB_SYSCALL, audit,
                           ZLLOG_OP_SYSCALL_BASE + 2u, (int)tag, 0u, 0u);
    zllog_lifecycle(ZLLOG_SUB_SCHED, ZLLOG_OBJ_PROCESS, tag,
                    ZLLOG_LIFE_EXIT, 0u, 1u);
    zllog_counter_add(ZLLOG_C_IRQ_TIMER, 3u);
    zllog_counter_observe(ZLLOG_C_MMIO_POLL, 10u);
    zllog_counter_observe(ZLLOG_C_MMIO_POLL, 20u);
    zllog_putc('b'); zllog_putc('o'); zllog_putc('o'); zllog_putc('t');
    zllog_putc('\n');
    int mount_ok = zllog_mount();
    if (requested_tear == EXPECT_AUTO_RETRY) {
        if (mount_ok || zllog_last_error() != 1 || zllog_ready()) {
            fprintf(stderr, "transient MSC failure was not retained safely\n");
            return 1;
        }
        /* idt_ticks advances 100 ticks per call in this harness. The first
         * normal-context retry is due after one second and must mount+drain
         * without a shell command. */
        zllog_flush_if_due();
        int ok = zllog_ready() && zllog_last_error() == 0 &&
                 zllog_buffered() == 0 && outside_writes == 0 &&
                 ready_persists == 1;
        printf("auto_retry=%d writes=%u outside=%u buffered=%u error=%u ready_persists=%u\n",
               ok, fake_writes, outside_writes, zllog_buffered(),
               zllog_last_error(), ready_persists);
        fclose(fake_disk);
        return ok ? 0 : 1;
    }
    if (requested_tear == EXPECT_MOUNT_REFUSAL) {
        zllog_flush_if_due();
        zllog_flush_if_due();
        int ok = !mount_ok && fake_writes == 0 && outside_writes == 0 &&
                 next_mount_retry_tick == 0 && mount_permanent_refusal;
        printf("writes=%u syncs=%u outside=%u range=%u+%u refused=%d error=%u buffered=%u\n",
               fake_writes, fake_syncs, outside_writes, writable_first, writable_blocks,
               !mount_ok, zllog_last_error(), zllog_buffered());
        fclose(fake_disk);
        return ok ? 0 : 1;
    }
    if (!mount_ok) {
        fprintf(stderr, "mount failed: zllog error %u\n", zllog_last_error());
        return 1;
    }
    if (requested_tear == EXPECT_ROTATION) {
        const unsigned capacity =
            (ZLLOG_SLOT_BYTES - ZLLOG_SLOT_HEADER) / ZLLOG_RECORD_BYTES;
        for (unsigned i = 0; i < capacity + 97u; i++) {
            zllog_event(ZLLOG_SUB_LAB, ZLLOG_EV_BURST, ZLLOG_INFO,
                        (unsigned)tag, i, capacity);
            if (zllog_buffered() >= 2048u && !zllog_flush()) {
                fprintf(stderr, "rotation workload flush failed at %u\n", i);
                return 1;
            }
        }
        int completed = zllog_complete();
        int ok = completed && outside_writes == 0 && zllog_buffered() == 0;
        printf("writes=%u syncs=%u outside=%u range=%u+%u completed=%d rotated=1 buffered=%u\n",
               fake_writes, fake_syncs, outside_writes, writable_first,
               writable_blocks, completed, zllog_buffered());
        fclose(fake_disk);
        return ok ? 0 : 1;
    }
    zllog_event(ZLLOG_SUB_INPUT, ZLLOG_EV_INPUT_EVENT, ZLLOG_INFO,
                (unsigned)tag, 0x55u, 3u);
    zllog_frame_observe(10u, 20u, 30u, 40u, 50u, 150u, 0u,
                        2u, 33000u, 90u, 44u, 0u, 3u, 132000u,
                        4u, 5u, 6u, 7u, 2u, 32000u, 3u, 2u);
    if (!zllog_flush()) {
        fprintf(stderr, "record flush failed before requested metadata tear\n");
        return 1;
    }

    int completed = zllog_complete();
    int expected_tear = requested_tear != TEAR_NONE;
    int ok = outside_writes == 0 &&
             (expected_tear ? (!completed && tear_triggered) : (completed && !tear_triggered));
    printf("writes=%u syncs=%u outside=%u range=%u+%u completed=%d torn=%d buffered=%u\n",
           fake_writes, fake_syncs, outside_writes, writable_first, writable_blocks,
           completed, tear_triggered, zllog_buffered());
    fclose(fake_disk);
    return ok ? 0 : 1;
}
