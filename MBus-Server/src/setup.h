#ifndef __IAI_SETUP__
#define __IAI_SETUP__

#include <rte_eal.h>
#include "../../MBus-Shared/src/mbus_common.h"

extern void iai_assert_link_status(uint16_t port_id);
extern void iai_init_port(uint16_t port_id);
extern void iai_setup_mbuf(void);
extern void iai_init_shared(void);
extern void iai_init_ports(void);
extern void iai_init_flow(uint32_t ipDstAddress,    uint32_t ipDstMask,
                          uint16_t ipPortStart,     uint16_t ipPortEnd);


extern void iai_init_ring_pair(struct ring_pair* rings, uint16_t id);
extern void iai_init_rings(uint16_t num_ring_pairs);


struct MainContext {
  uint16_t nr_queues;
  uint16_t port_id;
  struct rte_flow *flow;
  uint16_t nr_ring_pairs;
  struct ring_pair ring_pairs[MBUS_MAX_IO_RINGS];
  struct rte_mempool *mbuf_pool;
};

extern struct MainContext TheMainContext;

#endif