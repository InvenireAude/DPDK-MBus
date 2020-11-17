
#include "utils.h"

#include <signal.h>
#include <rte_ether.h>

/*******************************************************************************/
static int _iai_force_quit = false;
/*******************************************************************************/
static void _iai_signal_handler(int signum)
{
	if (signum == SIGINT || signum == SIGTERM) {
		printf("\n\nSignal %d received, preparing to exit...\n", signum);
		_iai_force_quit = true;
	}
}
/*******************************************************************************/
void iai_setup_signals(void){
	signal(SIGINT, _iai_signal_handler);
	signal(SIGTERM, _iai_signal_handler);
}
/*******************************************************************************/
bool iai_is_quit(void){
	return _iai_force_quit;
}
/*******************************************************************************/
void println_ether_addr(const char *what, struct rte_ether_addr *eth_addr) {
	char buf[RTE_ETHER_ADDR_FMT_SIZE];
	rte_ether_format_addr(buf, RTE_ETHER_ADDR_FMT_SIZE, eth_addr);
	printf("%s%s\n", what, buf);
}
/*******************************************************************************/
void println_ip_addr(const char *what, uint32_t ip){
  printf("%s%d.%d.%d.%d\n", what,
      (ip & 0xff000000) >> 24,
      (ip & 0x00ff0000) >> 16,
      (ip & 0x0000ff00) >> 8,
      (ip & 0x000000ff));
}
/*******************************************************************************/
void println_ip_addr_port(const char *what, uint32_t ip, uint16_t port){
  printf("%s%d.%d.%d.%d(%d)\n", what,
      (ip & 0xff000000) >> 24,
      (ip & 0x00ff0000) >> 16,
      (ip & 0x0000ff00) >> 8,
      (ip & 0x000000ff),
      port);
}
/*******************************************************************************/
