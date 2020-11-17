#ifndef __IAI_Datapath_MBUS__
#define __IAI_Datapath_MBUS__

#include <stdbool.h>
#include <rte_ether.h>

void mbus_extract(struct rte_mbuf *m, char** pp_data,uint16_t *p_data_len, size_t *p_sequence);
void mbus_prepare(struct rte_mbuf *created_pkt, size_t sequence, uint16_t port, struct rte_ether_hdr* src_ether_hdr);
void mbus_prepare_data(struct rte_mbuf *created_pkt, uint16_t port, struct rte_ether_hdr* src_ether_hdr);

struct mbus_who_has_it{
  size_t sequence_start;
  size_t sequence_end;
};

#endif