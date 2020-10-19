
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
void print_ether_addr(const char *what, struct ether_addr *eth_addr) {
	char buf[ETHER_ADDR_FMT_SIZE];
	ether_format_addr(buf, ETHER_ADDR_FMT_SIZE, eth_addr);
	printf("%s%s", what, buf);
}
