#ifndef __IAI_DataPath__
#define __IAI_DataPath__

#include <stdbool.h>
#include <rte_ether.h>
#include <rte_mbuf.h>
#include "../../MBus-Shared/src/mbus_common.h"

/*******************************************************************************/

struct data_path;
struct data_path_port;

/*******************************************************************************/

typedef void (*data_path_on_start_fun_ptr)(struct data_path_port* data_path_port);
typedef void (*data_path_rings_fun_ptr)(struct data_path_port* data_path_port);
typedef void (*data_path_packet_fun_ptr)(struct data_path_port*, struct rte_mbuf*);

/*******************************************************************************/

struct data_path_handlers{
  data_path_on_start_fun_ptr    ptr_handle_on_start;
  data_path_rings_fun_ptr       ptr_handle_rings;
  data_path_packet_fun_ptr      ptr_handle_packet;
};

/*******************************************************************************/

struct data_path_selector_udp{
  uint32_t dst_ip;
  uint32_t dst_ip_mask;
  uint32_t dst_udp_port;
};

struct data_path_udp {
  struct data_path_selector_udp selector;
};

/*******************************************************************************/

struct data_path_selector_mbus{
  uint32_t dst_ip;
  uint32_t dst_ip_mask;
  uint16_t dst_udp_port;
};

struct data_path_mbus {
  struct data_path_selector_mbus selector;
  struct rte_ether_hdr src_ether_hdr;

  uint32_t  mbus_ring_size;
  struct rte_mbuf** mbus_ring;
  struct rte_mbuf** mbus_ring_current_input;
  struct rte_mbuf** mbus_ring_end_guard;
  size_t sequence;
  size_t sequence_max_who_has_it;
};

/*******************************************************************************/

struct data_path_selector_id_only{
  uint32_t dst_id;
};

struct data_path_id_only {
  struct data_path_selector_id_only selector;
};

/*******************************************************************************/

union data_path_private {
  struct data_path_udp udp;
  struct data_path_mbus mbus;
  struct data_path_id_only id_only;
};

struct data_path_stats{
  size_t num_tx;
  size_t num_rx;
  size_t num_rx_drop;
  size_t num_who_has;
};

struct data_path{
  union data_path_private _private;
  struct ring_pair ring_pair;
  struct data_path_stats stats;
};
/*******************************************************************************/

typedef enum data_path_types{
  IAI_DPT_UDP    = 0,
  IAI_DPT_MBUS   = 1,
  IAI_DPT_IDONLY = 2
} data_path_types;
/*******************************************************************************/

#define IAI_DP_MAX_PATHS   8
#define IAI_DP_MAX_PORTS   8
/*******************************************************************************/

struct data_path_port {

  struct data_path data_paths[IAI_DP_MAX_PATHS];

  struct data_path_handlers handlers;

  uint8_t  port_id;
  uint8_t  num_queues;
  uint8_t  num_data_paths;
};
/*******************************************************************************/

struct data_path_ports{

  struct data_path_port ports[IAI_DP_MAX_PORTS];
  uint8_t num_ports;

};
/*******************************************************************************/


extern void iai_initialize_datapaths(void);

extern uint8_t iai_configure_data_path_port(uint8_t port_id, data_path_types type_id);
extern uint8_t iai_configure_data_path_mbus(uint8_t idx, struct data_path_selector_mbus* selector);

extern void iai_close_data_paths(void);

#endif