#ifndef __IAI_SETUP__
#define __IAI_SETUP__

#include <rte_eal.h>
#include "../../MBus-Shared/src/mbus_common.h"

extern void iai_assert_link_status(uint16_t port_id);

extern void iai_init_port(uint16_t port_id, uint16_t nr_queues);
extern void iai_close_port(uint16_t port_id);

extern void iai_setup_mbuf(void);
extern void iai_init_shared(void);

extern void iai_init_ring_pair(struct ring_pair* rings);
extern void iai_close_ring_pair(struct ring_pair* rings);

struct MainContext {
  struct rte_mempool *mbuf_pool;
};

extern struct MainContext TheMainContext;

#endif