#ifndef __IAI_Setup_h
#define __IAI_Setup_h

#include "../../MBus-Shared/src/mbus_common.h"

struct MainContext {
  struct ring_pair ring_pairs[MBUS_MAX_IO_RINGS];
  uint16_t nr_ring_pairs;
  struct rte_mempool *mp;
};

extern struct MainContext TheMainContext;

extern void iai_setup_client(void);

#endif