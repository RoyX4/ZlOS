/* e1000.h - Intel 8254x link driver plus safe I219 inventory/quarantine. */
#ifndef ZL_E1000_H
#define ZL_E1000_H
int e1000_find(void);
int e1000_init(void);
int e1000_send(const unsigned char *frame, int len);
int e1000_poll(unsigned char *out, int max);
int e1000_mac(int i);
int e1000_link_up(void);
int e1000_device(void);
unsigned e1000_mmio_lo(void);
unsigned e1000_mmio_hi(void);
int e1000_tx_count(void);
int e1000_rx_count(void);
int e1000_rx_drops(void);
int e1000_tx_full(void);
int e1000_reset_timeouts(void);
#endif
