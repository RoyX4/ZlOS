/* netdev.c - the one NIC selection point above concrete link drivers. */
#include "netdev.h"
#include "e1000.h"

int virtio_net_find(void);
int virtio_net_init(void);
int virtio_net_send(const unsigned char *, int);
int virtio_net_poll(unsigned char *, int);
int virtio_net_mac(int);
int virtio_net_link_up(void);
int virtio_net_tx_count(void);
int virtio_net_rx_count(void);
int virtio_net_rx_drops(void);
int virtio_net_tx_full(void);
int virtio_net_ram_ok(void);

static int selected;

int netdev_find(void)
{
    selected = NETDEV_NONE;
    /* Prefer physical Intel wired Ethernet if both it and a virtual adapter
     * exist.  On QEMU only virtio is present and remains the reproducible path. */
    if (e1000_find() >= 0) { selected = NETDEV_E1000; return selected; }
    if (virtio_net_find() >= 0) { selected = NETDEV_VIRTIO; return selected; }
    return 0;
}

int netdev_init(void)
{
    if (!selected && !netdev_find()) return 0;
    return selected == NETDEV_E1000 ? e1000_init() : virtio_net_init();
}
int netdev_send(const unsigned char *p, int n)
{ return selected == NETDEV_E1000 ? e1000_send(p, n) : selected == NETDEV_VIRTIO ? virtio_net_send(p, n) : 0; }
int netdev_poll(unsigned char *p, int n)
{ return selected == NETDEV_E1000 ? e1000_poll(p, n) : selected == NETDEV_VIRTIO ? virtio_net_poll(p, n) : 0; }
int netdev_mac(int i)
{ return selected == NETDEV_E1000 ? e1000_mac(i) : selected == NETDEV_VIRTIO ? virtio_net_mac(i) : 0; }
int netdev_link_up(void)
{ return selected == NETDEV_E1000 ? e1000_link_up() : selected == NETDEV_VIRTIO ? virtio_net_link_up() : 0; }
int netdev_kind(void) { return selected; }
int netdev_device(void) { return selected == NETDEV_E1000 ? e1000_device() : 0; }
int netdev_tx_count(void)
{ return selected == NETDEV_E1000 ? e1000_tx_count() : selected == NETDEV_VIRTIO ? virtio_net_tx_count() : 0; }
int netdev_rx_count(void)
{ return selected == NETDEV_E1000 ? e1000_rx_count() : selected == NETDEV_VIRTIO ? virtio_net_rx_count() : 0; }
int netdev_rx_drops(void)
{ return selected == NETDEV_E1000 ? e1000_rx_drops() : selected == NETDEV_VIRTIO ? virtio_net_rx_drops() : 0; }
int netdev_tx_full(void)
{ return selected == NETDEV_E1000 ? e1000_tx_full() : selected == NETDEV_VIRTIO ? virtio_net_tx_full() : 0; }
int netdev_ram_ok(void)
{ return selected == NETDEV_E1000 ? 1 : selected == NETDEV_VIRTIO ? virtio_net_ram_ok() : 0; }
