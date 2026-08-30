/* hostmachine.c - the parts of the machine a host harness cannot have.
 *
 * cpu.c reaches CPUID and RDTSC and rtc.c reaches the CMOS through port I/O.
 * CPUID would work here; port I/O would fault. Rather than let each harness
 * grow its own copy of these - which is how two harnesses end up disagreeing
 * about what the machine does - they live once, here.
 *
 * RDRAND IS REPORTED ABSENT ON PURPOSE. That forces entropy.c down its WEAK
 * path, which is the one worth exercising: the hardware path is three
 * instructions and the fallback is where a bug would hide. And there is no
 * clock, so certificate dates go unchecked in these harnesses - x509test
 * covers expiry directly with dates it controls.
 */
int cpu_has_rdrand(void) { return 0; }
int cpu_rdrand32(unsigned int *out) { (void)out; return 0; }
int rtc_present(void) { return 0; }
int rtc_read(void) { return 0; }
int rtc_year(void) { return 0; }
int rtc_month(void) { return 0; }
int rtc_day(void) { return 0; }
int rtc_hour(void) { return 0; }
int rtc_min(void) { return 0; }
int rtc_sec(void) { return 0; }
