#include <stdint.h>
#include <inttypes.h>

#include <rte_eal.h>
#include <rte_common.h>
#include <rte_malloc.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_net.h>

#include <rte_cycles.h>

#include <rte_ip.h>
#include <rte_udp.h>

#include "setup.h"
#include "flow.h"
#include "mbus.h"
#include "utils.h"
#include "datapath.h"

/************************************************************************************/

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

/************************************************************************************/

#define DEST_IP ((226<<24) + (1<<16) + (1<<8) + 1) /* dest ip = 192.168.1.1 */
#define FULL_MASK 0xffffffff /* full mask */
#define EMPTY_MASK 0x0 /* empty mask */

/************************************************************************************/

static void main_loop(void)
{
	struct rte_mbuf *mbufs[BURST_SIZE];
	uint16_t nb_rx;
	uint16_t i,j;

  // single thread for now, for the simplicity ...

	while (!iai_is_quit()) {
	  for(int port_idx=0; port_idx < the_data_path_ports.num_ports; port_idx++){

      struct data_path_port* p_port = &the_data_path_ports.ports[port_idx];

	    for (i = 0; i < p_port->num_queues; i++) {

        nb_rx = rte_eth_rx_burst(p_port->port_id, i, mbufs, 32);

        for (j = 0; j < nb_rx; j++){
          p_port->handlers.ptr_handle_packet(p_port, mbufs[j]);
        }
      }

      p_port->handlers.ptr_handle_rings(p_port);
    }
  }

}

/************************************************************************************/

int main(int argc, char **argv){

  int ret;

	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, ":: invalid EAL arguments\n");

	iai_setup_signals();

  iai_setup_mbuf();
  iai_init_shared();

  uint8_t port_idx = iai_configure_data_path_port(0, IAI_DPT_MBUS);

  struct data_path_selector_mbus selectorInput = {
    .dst_ip = DEST_IP,
    .dst_ip_mask = FULL_MASK,
    .dst_udp_port = 34001
  };

  struct data_path_selector_mbus selectorOutput = {
    .dst_ip = DEST_IP,
    .dst_ip_mask = FULL_MASK,
    .dst_udp_port = 34003
  };

  iai_configure_data_path_mbus(port_idx, &selectorInput);
  iai_configure_data_path_mbus(port_idx, &selectorOutput);

	main_loop();

	//rte_flow_flush(TheMainContext.port_id, &error);

  iai_close_ports();
	return 0;
}
