/* dpll_test.c - program a real DPLL and watch it lock.
 *
 * This is the first time zlOS's driver WRITES to the display engine, and it is
 * deliberately aimed at a PLL that nothing is using: DPLL2 and DPLL3 have their
 * override bits clear, so i915 has not assigned them to any port. Programming
 * one changes no pixel on any screen - it just spins up a clock generator,
 * which is exactly the thing we need to prove we can do.
 *
 * What this validates that reading never could:
 *   - the CFGCR1/CFGCR2 encoding produced by the divider search is one the
 *     hardware accepts
 *   - the enable register mapping is right (it was NOT: 0x46010 + pll*4 lands
 *     on the wrong register for DPLL2, and for DPLL0 it is the clock the whole
 *     display engine runs on)
 *   - the lock wait actually observes lock
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>

typedef unsigned int u32; typedef unsigned short u16;
typedef unsigned char u8; typedef unsigned long long uptr;
#define PCI_DEV "/sys/bus/pci/devices/0000:00:02.0"

void intel_attach(uptr, u32, uptr, u32, int, u32(*)(int,int,int,int));
void intel_link_train_arm(int);
int  intel_dpll_compute_hdmi(u32);
u32  intel_wrpll_cfgcr1(void); u32 intel_wrpll_cfgcr2(void);
u32  intel_wrpll_dco_khz(void); int intel_wrpll_divider(void);
int  intel_wrpll_p(void); int intel_wrpll_q(void); int intel_wrpll_k(void);
u32  intel_wrpll_actual_khz(void);
int  intel_dpll_program_hdmi(int, u32);
int  intel_dpll_enable(int); int intel_dpll_disable(int);
int  intel_dpll_locked(int); int intel_dpll_in_use(int);
u32  intel_dpll_ctrl1(void); u32 intel_dpll_cfgcr1(int); u32 intel_dpll_cfgcr2(int);
u32  intel_dpll_enable_reg(int); u32 intel_dpll_enable_val(int);
u32  intel_dpll_status(void); u32 intel_pwr_well_driver(void); u32 intel_dc_state(void);

static int cfg_fd = -1;
u32 host_cfg_read(int b,int d,int f,int o){(void)b;(void)d;(void)f;u32 v=0;
  if(cfg_fd>=0&&pread(cfg_fd,&v,4,o)==4)return v;return 0;}
/* intel.c needs real timing now (cpu.c provides it in the kernel; here we are
 * a Linux process, so nanosleep is both simpler and more accurate than any
 * spin). Without these three the link fails at cpu_delay_us. */
void cpu_delay_us(unsigned int us)
{
    struct timespec ts = { (long)(us / 1000000u), (long)(us % 1000000u) * 1000L };
    while (nanosleep(&ts, &ts) == -1) { }
}
void cpu_delay_ms(unsigned int ms) { cpu_delay_us(ms * 1000u); }
unsigned int cpu_now_ms(void)
{
    struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
    return (unsigned int)(t.tv_sec * 1000ull + t.tv_nsec / 1000000ull);
}

u32 idt_ticks(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);
  return (u32)(t.tv_sec*100+t.tv_nsec/10000000);}
int pci_count(void){return 0;} int pci_vendor(int i){(void)i;return 0;}
int pci_device(int i){(void)i;return 0;} int pci_class(int i){(void)i;return 0;}
void pci_scan(void){} void pci_enable(int i){(void)i;}
u32 pci_bar(int i,int w){(void)i;(void)w;return 0;}
u32 pci_bar_size(int i,int w){(void)i;(void)w;return 0;}
u32 pci_read32(int b,int d,int f,int o){return host_cfg_read(b,d,f,o);}

int main(int argc,char**argv)
{
    setvbuf(stdout,NULL,_IONBF,0);
    int pll = 2, target = 148500;          /* 1080p60, on an unused PLL */
    if (argc > 1) pll = atoi(argv[1]);
    if (argc > 2) target = atoi(argv[2]);

    char p[256];
    snprintf(p,sizeof p,"%s/config",PCI_DEV); cfg_fd = open(p,O_RDONLY);
    snprintf(p,sizeof p,"%s/resource0",PCI_DEV);
    int fd = open(p,O_RDWR);
    if (fd < 0) { perror("open (need sudo)"); return 1; }
    void *m = mmap(NULL,8u<<20,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    if (m == MAP_FAILED) { perror("mmap"); return 1; }
    intel_attach((uptr)m,8u<<20,0,0,(int)(host_cfg_read(0,2,0,0)>>16),host_cfg_read);

    printf("zlOS DPLL write test\n====================\n\n");
    printf("  target      DPLL%d at %d kHz\n", pll, target);

    if (intel_dpll_in_use(pll)) {
        printf("  REFUSING: DPLL%d has its override bit set - something is using it.\n", pll);
        return 1;
    }
    printf("  DPLL%d is unused (override clear) - safe to program\n\n", pll);

    printf("  before:  CTRL1=%08X  CFGCR1=%08X CFGCR2=%08X  ENABLE(%05X)=%08X  lock=%d\n",
           intel_dpll_ctrl1(), intel_dpll_cfgcr1(pll), intel_dpll_cfgcr2(pll),
           intel_dpll_enable_reg(pll), intel_dpll_enable_val(pll), intel_dpll_locked(pll));

    if (!intel_dpll_compute_hdmi((u32)target)) { printf("  no divider solution\n"); return 1; }
    printf("\n  computed: dco=%u kHz  divider=%d (p=%d q=%d k=%d)  actual=%u kHz\n",
           intel_wrpll_dco_khz(), intel_wrpll_divider(), intel_wrpll_p(),
           intel_wrpll_q(), intel_wrpll_k(), intel_wrpll_actual_khz());
    printf("            CFGCR1=%08X  CFGCR2=%08X\n",
           intel_wrpll_cfgcr1(), intel_wrpll_cfgcr2());

    intel_link_train_arm(1);               /* from here on, we write */

    printf("\n  programming...\n");
    if (!intel_dpll_program_hdmi(pll,(u32)target)) { printf("  program failed\n"); return 1; }
    printf("  after program: CTRL1=%08X CFGCR1=%08X CFGCR2=%08X\n",
           intel_dpll_ctrl1(), intel_dpll_cfgcr1(pll), intel_dpll_cfgcr2(pll));

    printf("\n  power wells: CTL_DRIVER=%08X  DC_STATE=%08X\n",
           intel_pwr_well_driver(), intel_dc_state());
    printf("  DPLL_STATUS before enable = %08X\n", intel_dpll_status());
    printf("\n  enabling and waiting for lock...\n");
    int locked = intel_dpll_enable(pll);
    for (int i = 0; i < 5; i++) {
        struct timespec ts = {0, 20000000};   /* 20 ms */
        nanosleep(&ts, 0);
        printf("    +%2d ms  DPLL_STATUS=%08X  ENABLE=%08X\n",
               (i+1)*20, intel_dpll_status(), intel_dpll_enable_val(pll));
    }
    printf("  ENABLE(%05X)=%08X   lock bit=%d   -> %s\n",
           intel_dpll_enable_reg(pll), intel_dpll_enable_val(pll),
           intel_dpll_locked(pll), locked ? "LOCKED" : "did not lock");

    printf("\n  cleaning up - putting it back exactly as found...\n");
    intel_dpll_disable(pll);
    printf("  CTRL1=%08X  ENABLE=%08X  lock=%d\n",
           intel_dpll_ctrl1(), intel_dpll_enable_val(pll), intel_dpll_locked(pll));

    intel_link_train_arm(0);
    munmap(m,8u<<20);
    return locked ? 0 : 2;
}
