#ifndef __MBUS_Common_h
#define __MBUS_Common_h

#include <rte_eal.h>

struct shared_info {
	uint16_t num_ports;
  uint16_t num_ring_pairs;

  char somedata[1024];

//	volatile struct rx_stats rx_stats;
//	volatile struct tx_stats tx_stats[MAX_CLIENTS];

};

#define MBUS_SHARED_INFO_NAME  "mbus_info"
#define MBUS_SHARED_MEM_POOL   "mbus_info"

#define MBUS_SHARED_RING_IN    "mbus_ring_in"
#define MBUS_SHARED_RING_OUT   "mbus_ring_out"

#define MBUS_MAX_IO_RINGS 16

struct ring_pair{
  struct rte_ring *ring_in;
  struct rte_ring *ring_out;
};

static inline const char *iai_get_ring_name(const char* base, unsigned int id)
{
	static char buffer[2 * sizeof(MBUS_SHARED_RING_IN) + 2];

	snprintf(buffer, sizeof(buffer) - 1, "%s_%d", base, id);
	return buffer;
}

#endif