/* dhcp.h - bounded DHCPv4 client over net.c's link-independent IPv4 seam. */
#ifndef ZL_DHCP_H
#define ZL_DHCP_H

typedef unsigned int dhcp_u32;

enum {
    DHCP_IDLE = 0,
    DHCP_SELECTING = 1,
    DHCP_REQUESTING = 2,
    DHCP_BOUND = 3,
    DHCP_FAILED = 4
};

int dhcp_start(void);
int dhcp_poll(void);          /* one receive frame and bounded timer work */
int dhcp_state(void);
dhcp_u32 dhcp_address(void);
dhcp_u32 dhcp_mask(void);
dhcp_u32 dhcp_gateway(void);
dhcp_u32 dhcp_dns(void);
dhcp_u32 dhcp_server(void);
dhcp_u32 dhcp_lease_seconds(void);
int dhcp_retries(void);
int dhcp_bad_packets(void);

#endif
