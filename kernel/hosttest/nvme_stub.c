/* nvme_stub.c - "there is no disk", for the harnesses that draw.
 *
 * settings.c persists to NVMe, so anything that links it needs the driver's
 * symbols. wmtest and wmshot are about pixels and routing and have no business
 * pulling in a PCI/MMIO driver, and settingstest supplies its own fake disk -
 * so this reports NO DEVICE, which is a state settings.c already has to handle
 * correctly and prints a line about.
 *
 * It lives in its own file rather than inside wmtest.c so that wmshot.c - the
 * look track's - does not have to be edited to gain eight stubs.
 *
 * These deliberately do NOT pretend a disk exists. A stub that silently
 * succeeded would let a save/load bug pass every drawing test.
 */
int  nvme_ready(void)          { return 0; }
unsigned nvme_blocksize(void)  { return 0; }
unsigned nvme_blocks_lo(void)  { return 0; }
unsigned nvme_blocks_hi(void)  { return 0; }
int  nvme_read_block(unsigned lo, unsigned hi)  { (void)lo; (void)hi; return 0; }
int  nvme_write_block(unsigned lo, unsigned hi) { (void)lo; (void)hi; return 0; }
int  nvme_data_byte(int i)     { (void)i; return 0; }
void nvme_data_set(int i, int v) { (void)i; (void)v; }
