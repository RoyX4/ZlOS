#ifndef ZL_NETDEV_H
#define ZL_NETDEV_H
enum { NETDEV_NONE = 0, NETDEV_VIRTIO = 1, NETDEV_E1000 = 2,
       NETDEV_CDC_ECM = 3 };
int netdev_find(void);
int netdev_init(void);
int netdev_send(const unsigned char *frame, int len);
int netdev_poll(unsigned char *out, int max);
int netdev_mac(int i);
int netdev_link_up(void);
int netdev_kind(void);
int netdev_mtu(void);
int netdev_device(void);
int netdev_tx_count(void);
int netdev_rx_count(void);
int netdev_rx_drops(void);
int netdev_tx_full(void);
int netdev_ram_ok(void);
#endif
