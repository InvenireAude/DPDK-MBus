#ifndef __IAI_SETUP__
#define __IAI_SETUP__

#include <rte_eal.h>

extern void iai_assert_link_status(uint16_t port_id);
extern void iai_init_port(uint16_t port_id);
extern void iai_setup_mbuf(void);
extern void iai_init_shared(void);
extern void iai_init_ports(void);
extern void iai_init_flow(uint32_t ipDstAddress,    uint32_t ipDstMask,
                          uint16_t ipPortStart,     uint16_t ipPortEnd);

extern void iai_init_rings(void);

struct MainContext {
  uint16_t nr_queues;
  uint16_t port_id;
  struct rte_flow *flow;

  struct rte_ring *ring_in;
  struct rte_ring *ring_out;

  struct rte_mempool *mbuf_pool;
};

extern struct MainContext TheMainContext;

#endif