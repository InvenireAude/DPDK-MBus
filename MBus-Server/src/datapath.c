

#include "datapath.h"
#include "setup.h"
#include "flow.h"
#include "mbus.h"
#include "utils.h"
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_malloc.h>

/*******************************************************************************/
void iai_initialize_datapaths(void){

  printf("IAI: ******************************\n");
  printf("IAI: *** Data path confguration ***\n");
  printf("IAI: ******************************\n");

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

      printf("Data from client [%d] [%.*s]\n", m->data_len, m->data_len, rte_pktmbuf_mtod(m, char *));

      uint32_t slot_id = dp->_private.mbus.sequence % dp->_private.mbus.mbus_ring_size;
      struct rte_mbuf** slot = dp->_private.mbus.mbus_ring + slot_id;

      if(*slot){
        rte_pktmbuf_free(*slot);
      }


      *slot = rte_pktmbuf_clone(m, iai_the_context.mbuf_pool);
      if(*slot){
        (*slot)->seqn = dp->_private.mbus.sequence; // note 64 -> 32 bits ?!
      }


      mbus_prepare(m, dp->_private.mbus.sequence, dp->_private.mbus.selector.dst_udp_port, &dp->_private.mbus.src_ether_hdr);


      int rc = rte_eth_tx_burst(data_path_port->port_id, 0, &m, 1);
      printf("rc = %d\n",rc);
      dp->_private.mbus.sequence++;
      dp->stats.num_tx++;
    }
  }
}
/*******************************************************************************/
static void _handle_packet_mbus_who_has_it(struct mbus_who_has_it * p_msg, struct data_path *dp, struct data_path_port* data_path_port){

 size_t sequence = dp->_private.mbus.sequence < dp->_private.mbus.mbus_ring_size ?
                            0 : dp->_private.mbus.sequence - dp->_private.mbus.mbus_ring_size;

 if(sequence < p_msg->sequence_start)
    sequence = p_msg->sequence_start; // or return - I can help you any way ...

 while(sequence <= p_msg->sequence_end && sequence < dp->_private.mbus.sequence){
   printf("Who has it [%ld]\n", sequence);
   struct rte_mbuf** slot = dp->_private.mbus.mbus_ring + (sequence % dp->_private.mbus.mbus_ring_size);
   printf("Slot[%lx]:%d\n", (long)*slot, *slot ? (*slot)->seqn : 9999);
   if( (*slot) && (uint32_t)sequence == (*slot)->seqn){

      struct rte_mbuf* m = rte_pktmbuf_clone(*slot, iai_the_context.mbuf_pool);
      if(m){
        mbus_prepare(m, sequence, dp->_private.mbus.selector.dst_udp_port, &dp->_private.mbus.src_ether_hdr);
        int rc = rte_eth_tx_burst(data_path_port->port_id, 0, &m, 1);
        printf("Who has it [%ld] rc = %d\n", sequence, rc);
      }
   }

   sequence++;
 };

}
/*******************************************************************************/
static void _handle_packet_mbus_send_who_has_it(const struct mbus_who_has_it * p_msg, struct data_path *dp, struct data_path_port* data_path_port){

  struct rte_mbuf* m = rte_pktmbuf_alloc(iai_the_context.mbuf_pool);
  m->data_len = sizeof(struct mbus_who_has_it);
  struct mbus_who_has_it *data = rte_pktmbuf_mtod(m, struct mbus_who_has_it*);

  *data = *p_msg;

  mbus_prepare_data(m, dp->_private.mbus.selector.dst_udp_port + 1, &dp->_private.mbus.src_ether_hdr);
  int rc = rte_eth_tx_burst(data_path_port->port_id, 0, &m, 1);
  printf("Who has it request (%ld,%ld) rc = %d\n", p_msg->sequence_start, p_msg->sequence_end, rc);

  dp->stats.num_who_has++;
}
/*******************************************************************************/
static void _handle_on_start_mbus(struct data_path_port* data_path_port){
  for(int idx=0; idx<data_path_port->num_data_paths; idx++){
    struct data_path *dp = &data_path_port->data_paths[idx];
    const struct mbus_who_has_it msg = {0, 0xffff };
    _handle_packet_mbus_send_who_has_it(&msg, dp, data_path_port);
  }
}
/*******************************************************************************/
static void _handle_packet_mbus(struct data_path_port* data_path_port, struct rte_mbuf* m){

  uint32_t dst_ip;
  uint32_t src_ip;
  uint16_t dst_udp_port;

  char     *data;
  uint16_t data_len;
  size_t   sequence;

  if(!iai_extract_udp(m, &dst_ip, &src_ip, &dst_udp_port))
    return;

  printf("IP: %08x %08x %d\n",dst_ip, src_ip, dst_udp_port);

  for(int idx=0; idx<data_path_port->num_data_paths; idx++){
      struct data_path *dp = &data_path_port->data_paths[idx];
      if(dp->_private.mbus.selector.dst_ip == dst_ip){

        if(dp->_private.mbus.selector.dst_udp_port == dst_udp_port){

            mbus_extract(m, &data, &data_len, &sequence);
            printf("Data expected: %08ld, got: %08ld %d %ld\n", dp->_private.mbus.sequence, sequence, data_len, (long)data);

            if(sequence < dp->_private.mbus.sequence ||
               sequence > dp->_private.mbus.sequence + dp->_private.mbus.mbus_ring_size){
              printf("Drop it !\n");
              rte_pktmbuf_free(m);
              dp->stats.num_rx_drop++;
              return;
            }else{

              struct rte_mbuf** slot = dp->_private.mbus.mbus_ring + (sequence % dp->_private.mbus.mbus_ring_size);

              if(*slot){
                rte_pktmbuf_free(*slot);
              }
              m->seqn = sequence;
              *slot = m;

              if(dp->_private.mbus.sequence_max_who_has_it < dp->_private.mbus.sequence)
                  dp->_private.mbus.sequence_max_who_has_it = dp->_private.mbus.sequence;

              if(sequence > dp->_private.mbus.sequence_max_who_has_it + 1){
                struct mbus_who_has_it msg;
                msg.sequence_start = dp->_private.mbus.sequence_max_who_has_it;
                msg.sequence_end   = sequence - 1;
                dp->_private.mbus.sequence_max_who_has_it = msg.sequence_end;
                _handle_packet_mbus_send_who_has_it(&msg, dp, data_path_port);
              }

              slot = dp->_private.mbus.mbus_ring + (dp->_private.mbus.sequence % dp->_private.mbus.mbus_ring_size);
              printf("  Slot [%ld] = %lx, %d \n", dp->_private.mbus.sequence, (long)*slot, *slot ? (*slot)->seqn : 0xffff);
              while( (*slot) && (uint32_t)dp->_private.mbus.sequence == (*slot)->seqn){
                //TODO do not copy here, client does not need to free.

                struct rte_mbuf* m = rte_pktmbuf_clone(*slot, iai_the_context.mbuf_pool);
                if(m){
                  int rc = rte_ring_sp_enqueue(dp->ring_pair.ring_in, m);
                  printf("Deliver to client [%ld], rc=%d \n", sequence, rc);
                }

                dp->_private.mbus.sequence++;
                slot = dp->_private.mbus.mbus_ring + (dp->_private.mbus.sequence % dp->_private.mbus.mbus_ring_size);
                printf("  Slot [%ld] = %lx, %d \n", dp->_private.mbus.sequence, (long)*slot, *slot ? (*slot)->seqn : 0xffff);
              }

              dp->stats.num_rx++;
              return;
            }

        }else if(dp->_private.mbus.selector.dst_udp_port + 1 == dst_udp_port ){

            mbus_extract(m, &data, &data_len, &sequence);
            struct mbus_who_has_it * p_msg = (struct mbus_who_has_it *)data;

            printf("Who has it ? [%d] %ld - %ld" , dst_udp_port, p_msg->sequence_start, p_msg->sequence_end);

            _handle_packet_mbus_who_has_it(p_msg, dp, data_path_port);

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

  printf("IAI: Data Path Port [%d]: type = %d, eth_port = %d \n", iai_the_context.data_path_ports->num_ports, type_id, port_id);

  iai_init_port(port_id, p_new->num_queues);

  switch(type_id){

    case IAI_DPT_MBUS:
      p_new->handlers.ptr_handle_on_start      = &_handle_on_start_mbus;
      p_new->handlers.ptr_handle_rings  = &_handle_rings_mbus;
      p_new->handlers.ptr_handle_packet = &_handle_packet_mbus;
    break;

    default:
    	rte_exit(EXIT_FAILURE, ":: uknown IAI Data Path port type: %d, port: %d \n", type_id, port_id);
  }

  printf("IAI: Data Path Port [%d]: is ready ! \n", iai_the_context.data_path_ports->num_ports);

  return iai_the_context.data_path_ports->num_ports++;
}
/*******************************************************************************/
static int16_t _next_ring_id = 0;
/*******************************************************************************/
uint8_t iai_configure_data_path_mbus(uint8_t idx, struct data_path_selector_mbus* selector){

  if(iai_the_context.data_path_ports->num_ports <= idx)
    rte_exit(EXIT_FAILURE, ":: out of IAI Data Path ports range.\n");

   struct data_path_port* port = &iai_the_context.data_path_ports->ports[idx];

  if(port->num_data_paths >= IAI_DP_MAX_PATHS)
    rte_exit(EXIT_FAILURE, ":: out of available IAI Data Paths.\n");

  struct data_path *dp = &port->data_paths[port->num_data_paths];
  bzero(dp, sizeof(*dp));

  dp->_private.mbus.selector = *selector;
  dp->_private.mbus.sequence = 0;

  printf("IAI: Data Path Port [%d, %d]: [mbus] :",idx,port->num_data_paths);
  println_ip_addr_port(" dst = ", selector->dst_ip, selector->dst_udp_port);

  rte_eth_macaddr_get(port->port_id, &dp->_private.mbus.src_ether_hdr.s_addr);
  rte_ether_addr_copy(&ether_multicast, &dp->_private.mbus.src_ether_hdr.d_addr);
  dp->_private.mbus.src_ether_hdr.ether_type = htons(0x0800);

  dp->ring_pair.ring_id = _next_ring_id++;
  iai_init_ring_pair(&dp->ring_pair);

  dp->_private.mbus.mbus_ring_size = 1024;
  size_t _ring_mem_sz = dp->_private.mbus.mbus_ring_size * sizeof(struct rte_mbuf*);
  dp->_private.mbus.mbus_ring = rte_malloc("MBUS_RING", _ring_mem_sz, 0);

  bzero(dp->_private.mbus.mbus_ring, _ring_mem_sz);

  dp->_private.mbus.sequence = 0;
  dp->_private.mbus.sequence_max_who_has_it = 0;

  dp->_private.mbus.mbus_ring_end_guard     = dp->_private.mbus.mbus_ring + dp->_private.mbus.mbus_ring_size;
  dp->_private.mbus.mbus_ring_current_input = dp->_private.mbus.mbus_ring + (dp->_private.mbus.sequence % dp->_private.mbus.mbus_ring_size);


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

      printf("IAI Data Path Port [%d], stats (rx: %10ld, rx_drop: %10ld, tx: %10ld, whi: %10ld) \n", port_idx,
                                                  dp->stats.num_rx,
                                                  dp->stats.num_rx_drop,
                                                  dp->stats.num_tx,
                                                  dp->stats.num_who_has);

    }

    printf("IAI Data Path Port [%d], shutdown ...\n", port_idx);
    iai_close_port(p_port->port_id);
  }

}
/*******************************************************************************/