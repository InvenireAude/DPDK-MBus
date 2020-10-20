#ifndef __IAI_FLOW__
#define __IAI_FLOW__

#include <stdbool.h>

#include <rte_eal.h>
#include <rte_common.h>
#include <rte_flow.h>

struct rte_flow *
iai_generate_flow(
    uint16_t iEthPort,        uint16_t iQueueRX,
		uint32_t ipDstAddress,    uint32_t ipDstMask,
    uint16_t ipPortStart,     uint16_t ipPortEnd,
		struct rte_flow_error *error);

bool iai_filter(struct rte_mbuf *m,
    uint32_t ipDstAddress,    uint32_t ipDstMask,
    uint16_t ipPortStart,     uint16_t ipPortEnd,
    uint16_t *pPort);

#endif