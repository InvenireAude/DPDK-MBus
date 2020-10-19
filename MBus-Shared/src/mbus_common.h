#ifndef __MBUS_Common_h
#define __MBUS_Common_h

#include <rte_eal.h>

struct shared_info {
	uint16_t num_ports;
	uint16_t id[RTE_MAX_ETHPORTS];

  char somedata[1024];

//	volatile struct rx_stats rx_stats;
//	volatile struct tx_stats tx_stats[MAX_CLIENTS];

};

#define MBUS_SHARED_INFO_NAME  "mbus_info"
#define MBUS_SHARED_MEM_POOL   "mbus_info"

#define MBUS_SHARED_RING_IN    "mbus_ring_in"
#define MBUS_SHARED_RING_OUT   "mbus_ring_out"

#endif