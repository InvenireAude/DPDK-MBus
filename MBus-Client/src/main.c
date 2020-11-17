
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>

#include <stdbool.h>

#include "../../MBus-Shared/src/mbus_common.h"

#include <signal.h>
#include <stdlib.h>
#include "setup.h"
#include "interface.h"
#include "backend.h"


int main(int argc, char *argv[]) {


	int retval;
	if ((retval = rte_eal_init(argc, argv)) < 0)
		return -1;

  iai_setup_client();

  const char *lib_name   = getenv("IAI_LIB") ? getenv("IAI_LIB") : "libFEX-ExchangeLib.so";
  const char *proxy_name = getenv("IAI_PROXY") ? getenv("IAI_PROXY") : "fex_proxy_core";

  iai_backend_core_ptr fun_core = iai_backend_open(lib_name, proxy_name);

  (*fun_core)(&proxy_interface);

}

