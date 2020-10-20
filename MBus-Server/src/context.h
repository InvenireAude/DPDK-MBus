#ifndef __IAI_Context_h
#define __IAI_Context_h

#include <rte_eal.h>
#define NO_FLAGS 0

struct data_path_ports;

struct iai_context {
  struct rte_mempool *mbuf_pool;
  struct data_path_ports *data_path_ports;
};

extern struct iai_context iai_the_context;

#endif