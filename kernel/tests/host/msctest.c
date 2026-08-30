/* msctest.c - bounded USB mass-storage command construction and status decode.
 *
 * This does not pretend to be a USB controller. xhcitest owns that model. It
 * tests the pure safety boundary around the fixed 4 KiB MSC staging area: LBA
 * range arithmetic, READ/WRITE(10) encoding, CBW tags, CSW validation, sense
 * decoding, and copies that may later carry journal sectors. */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>

void pci_scan(void) { }
int pci_count(void) { return 0; }
int pci_class(int i) { (void)i; return 0; }
int pci_subclass(int i) { (void)i; return 0; }
int pci_prog_if(int i) { (void)i; return 0; }
void pci_enable(int i) { (void)i; }
unsigned int pci_bar(int i, int w) { (void)i; (void)w; return 0; }
unsigned int pci_bar_hi(int i, int w) { (void)i; (void)w; return 0; }
unsigned int idt_ticks(void) { return 0; }
int console_pxw(void) { return 4096; }
int console_pxh(void) { return 4096; }
unsigned long long vmm_phys(unsigned long long v) { return v; }
unsigned long long vmm_virt(unsigned long long p) { return p; }

#include "../../src/drivers/input/xhci.c"

static int fails;

static void ok(const char *name, int pass)
{
    printf("  %-61s %s\n", name, pass ? "ok" : "FAIL");
    if (!pass) fails++;
}

static void eq(const char *name, unsigned got, unsigned want)
{
    int pass = got == want;
    printf("  %-61s %s", name, pass ? "ok" : "FAIL");
    if (!pass) printf(" (got 0x%X, want 0x%X)", got, want);
    printf("\n");
    if (!pass) fails++;
}

static void map_arena(void)
{
    void *p = mmap((void *)(uintptr_t)HI_XHCI, (size_t)(HI_VGPU - HI_XHCI),
                   PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    if (p == MAP_FAILED || (uintptr_t)p != (uintptr_t)HI_XHCI) {
        printf("FATAL: cannot map xHCI arena at 0x%08lX\n",
               (unsigned long)HI_XHCI);
        exit(2);
    }
}

static void test_ranges(void)
{
    printf("\nBOUNDS - no command may escape capacity or staging memory\n\n");
    msc_ready = 0; msc_slot = 0; msc_blocks = 0; msc_blocksize = 512;
    ok("uninitialised storage rejects I/O", !msc_range_ok(0, 1));

    msc_ready = 1; msc_slot = 1; msc_blocks = 100; msc_blocksize = 512;
    ok("eight 512-byte blocks exactly fill the 4 KiB window",
       msc_range_ok(0, 8));
    ok("nine 512-byte blocks exceed the fixed window", !msc_range_ok(0, 9));
    ok("the final logical block is addressable", msc_range_ok(99, 1));
    ok("a request crossing the final block is refused", !msc_range_ok(99, 2));
    ok("zero blocks is refused", !msc_range_ok(0, 0));
    ok("overflow-shaped LBA is refused", !msc_range_ok(0xFFFFFFFFu, 2));
    eq("max-blocks accessor agrees with the range bound",
       (unsigned)xhci_msc_max_blocks_per_io(), 8);

    msc_blocksize = 4096;
    ok("one 4 KiB logical block fits", msc_range_ok(0, 1));
    ok("two 4 KiB logical blocks do not fit", !msc_range_ok(0, 2));
}

static void test_command_encoding(void)
{
    printf("\nCOMMANDS - SCSI fields and Bulk-Only wrapper are exact\n\n");
    u8 cdb[10];
    msc_build_rw10(cdb, 0x2A, 0x12345678u, 0x0203u);
    eq("WRITE(10) opcode", cdb[0], 0x2A);
    eq("LBA byte 3", cdb[2], 0x12);
    eq("LBA byte 2", cdb[3], 0x34);
    eq("LBA byte 1", cdb[4], 0x56);
    eq("LBA byte 0", cdb[5], 0x78);
    eq("transfer count high byte", cdb[7], 0x02);
    eq("transfer count low byte", cdb[8], 0x03);

    msc_tag = 0xAABBCCDDu;
    u32 tag = build_cbw(4096, 0, cdb, 10);
    volatile u32 *dw = (volatile u32 *)MSC_CBW;
    volatile u8 *b = (volatile u8 *)MSC_CBW;
    eq("CBW returns the tag placed on the wire", tag, 0xAABBCCDDu);
    eq("CBW signature", dw[0], 0x43425355u);
    eq("CBW transfer length", dw[2], 4096);
    eq("WRITE direction is bulk OUT", b[12], 0);
    eq("CBW CDB length", b[14], 10);
    eq("CBW carries WRITE(10)", b[15], 0x2A);
}

static void put_csw(u32 signature, u32 tag, u32 residue, int status)
{
    zero_mem(MSC_CSW, 16);
    volatile u32 *dw = (volatile u32 *)MSC_CSW;
    dw[0] = signature; dw[1] = tag; dw[2] = residue;
    *(volatile u8 *)(MSC_CSW + 12) = (u8)status;
}

static void test_status(void)
{
    printf("\nSTATUS - stale, failed and malformed replies stay distinguishable\n\n");
    put_csw(0x53425355u, 7, 0, 0);
    ok("matching successful CSW passes", msc_parse_csw(7));
    eq("successful CSW result", (unsigned)msc_last_result, MSC_RESULT_OK);

    put_csw(0x53425355u, 8, 0, 0);
    ok("a stale tag is rejected", !msc_parse_csw(7));
    eq("stale tag has a distinct result", (unsigned)msc_last_result,
       MSC_RESULT_CSW_TAG);

    put_csw(0, 7, 0, 0);
    ok("a malformed signature is rejected", !msc_parse_csw(7));
    eq("bad signature has a distinct result", (unsigned)msc_last_result,
       MSC_RESULT_CSW_SIGNATURE);

    put_csw(0x53425355u, 7, 512, 1);
    ok("target command failure is rejected", !msc_parse_csw(7));
    eq("CSW failure status is retained", (unsigned)msc_last_csw_status, 1);
    eq("CSW residue is retained", msc_last_residue, 512);
    eq("command failure has a distinct result", (unsigned)msc_last_result,
       MSC_RESULT_CSW_FAILED);
}

static void test_sense(void)
{
    printf("\nSENSE - fixed and descriptor error reports both decode\n\n");
    zero_mem(MSC_DATA, 18);
    volatile u8 *d = (volatile u8 *)MSC_DATA;
    d[0] = 0x70; d[2] = 0x05; d[12] = 0x21; d[13] = 0x00;
    msc_clear_sense();
    ok("fixed-format sense decodes", msc_parse_sense(18));
    eq("sense key: illegal request", (unsigned)msc_sense_key, 0x05);
    eq("ASC: LBA out of range", (unsigned)msc_sense_asc, 0x21);
    eq("ASCQ", (unsigned)msc_sense_ascq, 0x00);

    zero_mem(MSC_DATA, 18);
    d[0] = 0x72; d[1] = 0x07; d[2] = 0x27; d[3] = 0x00;
    msc_clear_sense();
    ok("descriptor-format sense decodes", msc_parse_sense(18));
    eq("sense key: data protect", (unsigned)msc_sense_key, 0x07);
    eq("ASC: write protected", (unsigned)msc_sense_asc, 0x27);

    zero_mem(MSC_DATA, 18);
    msc_clear_sense();
    ok("unknown sense format is refused", !msc_parse_sense(18));
    ok("refused sense is not marked valid", !msc_sense_valid);
}

static void test_staging_copy(void)
{
    printf("\nSTAGING - caller buffers round-trip without escaping 4 KiB\n\n");
    u8 src[MSC_DATA_MAX], dst[MSC_DATA_MAX];
    for (u32 i = 0; i < MSC_DATA_MAX; i++) src[i] = (u8)(i * 37u + 11u);
    memset(dst, 0, sizeof dst);
    msc_copy_to_data(src, MSC_DATA_MAX);
    msc_copy_from_data(dst, MSC_DATA_MAX);
    ok("all 4096 staged bytes round-trip", memcmp(src, dst, sizeof src) == 0);
}

int main(void)
{
    printf("msctest - USB storage safety boundary\n");
    map_arena();
    test_ranges();
    test_command_encoding();
    test_status();
    test_sense();
    test_staging_copy();
    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "all good", fails);
    return fails ? 1 : 0;
}
