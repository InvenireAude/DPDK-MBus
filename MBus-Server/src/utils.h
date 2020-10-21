#ifndef __IAI_UTILS__
#define __IAI_UTILS__

#include <stdbool.h>
#include <rte_ether.h>

void iai_setup_signals(void);
bool iai_is_quit(void);

void println_ether_addr(const char *what, struct ether_addr *eth_addr);
void println_ip_addr(const char *what,uint32_t ip);
void println_ip_addr_port(const char *what, uint32_t ip, uint16_t port);

static const struct ether_addr ether_multicast = {
	.addr_bytes = {0x01, 0x1b, 0x19, 0x0, 0x0, 0x0}
};


#endif