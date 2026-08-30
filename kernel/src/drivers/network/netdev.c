/* netdev.c - the one NIC selection point above concrete link drivers. */
#include "netdev.h"
#include "e1000.h"

int virtio_net_find(void);
int virtio_net_init(void);
int virtio_net_send(const unsigned char *, int);
int virtio_net_poll(unsigned char *, int);
int virtio_net_mac(int);
int virtio_net_mtu(void);
int e1000_mtu(void);
int virtio_net_link_up(void);
int virtio_net_tx_count(void);
int virtio_net_rx_count(void);
int virtio_net_rx_drops(void);
int virtio_net_tx_full(void);
int virtio_net_ram_ok(void);

/* xHCI is in every kernel build, but host-only netdev harnesses deliberately
 * omit it. Weak seams keep those narrow tests narrow while the shipping image
 * binds the real CDC-ECM class driver. */
int xhci_ecm_init(void) __attribute__((weak));
int xhci_ecm_send(const unsigned char *, int) __attribute__((weak));
int xhci_ecm_poll(unsigned char *, int) __attribute__((weak));
int xhci_ecm_mac(int) __attribute__((weak));
int xhci_ecm_link_up(void) __attribute__((weak));
int xhci_ecm_tx_count(void) __attribute__((weak));
int xhci_ecm_rx_count(void) __attribute__((weak));
int xhci_ecm_rx_drops(void) __attribute__((weak));
int xhci_ecm_tx_full(void) __attribute__((weak));

static int selected;

int netdev_find(void)
{
    selected = NETDEV_NONE;
    /* Prefer an attached standards-based USB Ethernet/tethering device. The
     * ThinkPad's onboard I219 has no carrier without its proprietary dongle;
     * CDC-ECM is therefore the shortest physical path when both enumerate. */
    if (xhci_ecm_init && xhci_ecm_init()) {
        selected = NETDEV_CDC_ECM; return selected;
    }
    if (e1000_find() >= 0) { selected = NETDEV_E1000; return selected; }
    if (virtio_net_find() >= 0) { selected = NETDEV_VIRTIO; return selected; }
    return 0;
}

int netdev_init(void)
{
    if (!selected && !netdev_find()) return 0;
    return selected == NETDEV_CDC_ECM ? 1 :
           selected == NETDEV_E1000 ? e1000_init() : virtio_net_init();
}
int netdev_send(const unsigned char *p, int n)
{ return selected == NETDEV_CDC_ECM && xhci_ecm_send ? xhci_ecm_send(p, n) : selected == NETDEV_E1000 ? e1000_send(p, n) : selected == NETDEV_VIRTIO ? virtio_net_send(p, n) : 0; }
int netdev_poll(unsigned char *p, int n)
{ return selected == NETDEV_CDC_ECM && xhci_ecm_poll ? xhci_ecm_poll(p, n) : selected == NETDEV_E1000 ? e1000_poll(p, n) : selected == NETDEV_VIRTIO ? virtio_net_poll(p, n) : 0; }
int netdev_mac(int i)
{ return selected == NETDEV_CDC_ECM && xhci_ecm_mac ? xhci_ecm_mac(i) : selected == NETDEV_E1000 ? e1000_mac(i) : selected == NETDEV_VIRTIO ? virtio_net_mac(i) : 0; }
int netdev_link_up(void)
{ return selected == NETDEV_CDC_ECM && xhci_ecm_link_up ? xhci_ecm_link_up() : selected == NETDEV_E1000 ? e1000_link_up() : selected == NETDEV_VIRTIO ? virtio_net_link_up() : 0; }
int netdev_kind(void) { return selected; }
/* THE SELECTED DRIVER'S FRAME CEILING. CDC-ECM's is the USB stack's and is not
 * exported, so it answers 0 - which the pane must print as "not reported"
 * rather than as a number, because 0 is not a ceiling. */
int netdev_mtu(void)
{ return selected == NETDEV_E1000 ? e1000_mtu() : selected == NETDEV_VIRTIO ? virtio_net_mtu() : 0; }
int netdev_device(void) { return selected == NETDEV_E1000 ? e1000_device() : 0; }
int netdev_tx_count(void)
{ return selected == NETDEV_CDC_ECM && xhci_ecm_tx_count ? xhci_ecm_tx_count() : selected == NETDEV_E1000 ? e1000_tx_count() : selected == NETDEV_VIRTIO ? virtio_net_tx_count() : 0; }
int netdev_rx_count(void)
{ return selected == NETDEV_CDC_ECM && xhci_ecm_rx_count ? xhci_ecm_rx_count() : selected == NETDEV_E1000 ? e1000_rx_count() : selected == NETDEV_VIRTIO ? virtio_net_rx_count() : 0; }
int netdev_rx_drops(void)
{ return selected == NETDEV_CDC_ECM && xhci_ecm_rx_drops ? xhci_ecm_rx_drops() : selected == NETDEV_E1000 ? e1000_rx_drops() : selected == NETDEV_VIRTIO ? virtio_net_rx_drops() : 0; }
int netdev_tx_full(void)
{ return selected == NETDEV_CDC_ECM && xhci_ecm_tx_full ? xhci_ecm_tx_full() : selected == NETDEV_E1000 ? e1000_tx_full() : selected == NETDEV_VIRTIO ? virtio_net_tx_full() : 0; }
int netdev_ram_ok(void)
{ return selected == NETDEV_CDC_ECM ? 1 : selected == NETDEV_E1000 ? 1 : selected == NETDEV_VIRTIO ? virtio_net_ram_ok() : 0; }
