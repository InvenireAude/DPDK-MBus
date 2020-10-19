#ifndef __IAI_MBUS__
#define __IAI_MBUS__

#include <stdbool.h>
#include <rte_ether.h>

void mbus_extract(struct rte_mbuf *m, char** pp_data,uint16_t *p_data_len, size_t *p_sequence);
void mbus_prepare(struct rte_mbuf *created_pkt, size_t sequence);


#endif