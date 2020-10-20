

#include "datapath.h"
#include "setup.h"
#include "flow.h"
#include "mbus.h"
#include <rte_ether.h>
#include <rte_ethdev.h>

/*******************************************************************************/
void iai_initialize_datapaths(void){

  const struct rte_memzone *mz;

	mz = rte_memzone_reserve(MBUS_SHARED_DATA_PATHS, sizeof(struct data_path_ports),
				rte_socket_id(), NO_FLAGS);

	if (mz == NULL)
		rte_exit(EXIT_FAILURE, "Cannot rte_memzone_reserve data paths.\n");

  iai_the_context.data_path_ports = mz->addr;

  memset(mz->addr, 0, sizeof(struct data_path_ports));
}
/*******************************************************************************/
/*******************************************************************************/
// static void _handle_rings_udp(struct data_path* data_path){

// }
/*******************************************************************************/
// static void _handle_packet_udp(struct data_path_port* data_path_port, struct rte_mbuf* m){

// }
/*******************************************************************************/
/*******************************************************************************/
static void _handle_rings_mbus(struct data_path_port* data_path_port){

  //TODO add some limits here, just to be fair and not spend all the time here
  struct rte_mbuf* m;

  for(int idx=0; idx<data_path_port->num_data_paths; idx++){

    struct data_path *dp = &data_path_port->data_paths[idx];

    while( rte_ring_sc_dequeue(dp->ring_pair.ring_out, (void**)&m) == 0 ){

      printf("Data from client [%.*s]\n", m->data_len, rte_pktmbuf_mtod(m, char *));
      mbus_prepare(m, dp->_private.mbus.sequence, dp->_private.mbus.selector.dst_udp_port, data_path_port->port_id);
      dp->_private.mbus.sequence++;
      int rc = rte_eth_tx_burst(data_path_port->port_id, 0, &m, 1);
      printf("rc = %d\n",rc);

    }
  }
}
/*******************************************************************************/
static void _handle_packet_mbus(struct data_path_port* data_path_port, struct rte_mbuf* m){

  uint32_t dst_ip;
  uint32_t src_ip;
  uint16_t dst_udp_port;

  if(!iai_extract_udp(m, &dst_ip, &src_ip, &dst_udp_port))
    return;

  for(int idx=0; idx<data_path_port->num_data_paths; idx++){
      struct data_path *dp = &data_path_port->data_paths[idx];
      if(dp->_private.mbus.selector.dst_ip == dst_ip){

        if(dp->_private.mbus.selector.dst_udp_port == dst_udp_port){

            char     *data;
            uint16_t data_len;
            size_t   sequence;
            mbus_extract(m, &data, &data_len, &sequence);
            printf("Data %08ld %d %ld\n",sequence, data_len, (long)data);

            int nb_enq = rte_ring_sp_enqueue(dp->ring_pair.ring_in, m);
            printf("Result %d, ring size: %d \n", nb_enq, rte_ring_count(dp->ring_pair.ring_in));

            return;

        }else if(dp->_private.mbus.selector.dst_udp_port == dst_udp_port + 1){

            struct mbus_who_has_it * p_msg = rte_pktmbuf_mtod(m, struct mbus_who_has_it *);
            printf("Who has it ? [%d] %ld - %ld" , dst_udp_port, p_msg->sequence_start, p_msg->sequence_end);
            rte_pktmbuf_free(m);

            return;
        }
      }
  }

  rte_pktmbuf_free(m);
}
/*******************************************************************************/
/*******************************************************************************/
uint8_t iai_configure_data_path_port(uint8_t port_id, data_path_types type_id){

  if(iai_the_context.data_path_ports->num_ports >= IAI_DP_MAX_PORTS)
    rte_exit(EXIT_FAILURE, ":: out of available IAI Data Path ports.\n");

  int nr_ports = rte_eth_dev_count_avail();

	if(nr_ports <= port_id)
		  rte_exit(EXIT_FAILURE, ":: no (more) Ethernet ports found [port_id=%d]\n", port_id);

  struct data_path_port* p_new = &iai_the_context.data_path_ports->ports[iai_the_context.data_path_ports->num_ports];

  p_new->port_id = port_id;
  p_new->num_queues = 1;

  iai_init_port(port_id, p_new->num_queues);

  switch(type_id){

    case IAI_DPT_MBUS:

      p_new->handlers.ptr_handle_rings  = &_handle_rings_mbus;
      p_new->handlers.ptr_handle_packet = &_handle_packet_mbus;
    break;

    default:
    	rte_exit(EXIT_FAILURE, ":: uknown IAI Data Path port type: %d, port: %d \n", type_id, port_id);
  }

  printf("IAI Data Path Port [%d]: type = %d, eth_port = %d \n", iai_the_context.data_path_ports->num_ports, type_id, port_id);

  return iai_the_context.data_path_ports->num_ports++;
}
/*******************************************************************************/
uint8_t iai_configure_data_path_mbus(uint8_t idx, struct data_path_selector_mbus* selector){

  if(iai_the_context.data_path_ports->num_ports <= idx)
    rte_exit(EXIT_FAILURE, ":: out of IAI Data Path ports range.\n");

   struct data_path_port* port = &iai_the_context.data_path_ports->ports[idx];

  if(port->num_data_paths >= IAI_DP_MAX_PATHS)
    rte_exit(EXIT_FAILURE, ":: out of available IAI Data Paths.\n");

  struct data_path *dp = &port->data_paths[port->num_data_paths];
  dp->_private.mbus.selector = *selector;
  dp->_private.mbus.sequence = 0;

  printf("IAI Data Path [%d]: [mbus] : dst = %d.%d.%d.%d, port = %d \n", idx,
      (selector->dst_ip & 0xff000000) >> 24,
      (selector->dst_ip & 0x00ff0000) >> 16,
      (selector->dst_ip & 0x0000ff00) >> 8,
      (selector->dst_ip & 0x000000ff),
      selector->dst_udp_port);

  static int16_t _next_ring_id;
  dp->ring_pair.ring_id = _next_ring_id++;

  iai_init_ring_pair(&dp->ring_pair);

  return port->num_data_paths++;
}

/*******************************************************************************/
void iai_close_data_paths(void){
	/* closing and releasing resources */

  for(int port_idx=0; port_idx<iai_the_context.data_path_ports->num_ports; port_idx++){
    struct data_path_port* p_port = &iai_the_context.data_path_ports->ports[port_idx];

    for(int idx=0; idx<p_port->num_data_paths; idx++) {
      struct data_path *dp = &p_port->data_paths[idx];
      iai_close_ring_pair(&dp->ring_pair);
    }

    printf("IAI Data Path Port [%d], shutdown ...\n", port_idx);
    iai_close_port(p_port->port_id);
  }

}
/*******************************************************************************/