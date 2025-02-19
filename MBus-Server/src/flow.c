
#include "flow.h"

#include <rte_udp.h>
#include <rte_ip.h>

/*******************************************************************************/

#define MAX_PATTERN_NUM		3
#define MAX_ACTION_NUM		2

/*******************************************************************************/

struct rte_flow *
iai_generate_flow(
    uint16_t iEthPort,        uint16_t iQueueRX,
		uint32_t ipDstAddress,    uint32_t ipDstMask,
    __attribute__((unused))  uint16_t ipPortStart,
    __attribute__((unused))  uint16_t ipPortEnd,
		struct rte_flow_error *error)
{
	struct rte_flow_attr attr;
	struct rte_flow_item pattern[MAX_PATTERN_NUM];
	struct rte_flow_action action[MAX_ACTION_NUM];
	struct rte_flow *flow = NULL;
	struct rte_flow_action_queue queue = { .index = iQueueRX };
	struct rte_flow_item_ipv4 ip_spec;
	struct rte_flow_item_ipv4 ip_mask;
	int res;

	memset(pattern, 0, sizeof(pattern));
	memset(action, 0, sizeof(action));

	memset(&attr, 0, sizeof(struct rte_flow_attr));
	attr.ingress = 1;

  /* Action O: enqueue */
	action[0].type = RTE_FLOW_ACTION_TYPE_QUEUE;
	action[0].conf = &queue;

  /* Action 1: end */
	action[1].type = RTE_FLOW_ACTION_TYPE_END;

  /* Pattern 0: eth */
	pattern[0].type = RTE_FLOW_ITEM_TYPE_ETH;

  /* Pattern 1: ip destination address */

	memset(&ip_spec, 0, sizeof(struct rte_flow_item_ipv4));
	memset(&ip_mask, 0, sizeof(struct rte_flow_item_ipv4));
	ip_spec.hdr.dst_addr = htonl(ipDstAddress);
	ip_mask.hdr.dst_addr = htonl(ipDstMask);
	ip_spec.hdr.src_addr = htonl(0);
	ip_mask.hdr.src_addr = 0;

	pattern[1].type = RTE_FLOW_ITEM_TYPE_IPV4;
	pattern[1].spec = &ip_spec;
	pattern[1].mask = &ip_mask;

  /* Pattern 2: end */
	pattern[1].type = RTE_FLOW_ITEM_TYPE_END;

	res = rte_flow_validate(iEthPort, &attr, pattern, action, error);

  if(!res)
		flow = rte_flow_create(iEthPort, &attr, pattern, action, error);

	return flow;
}

/*******************************************************************************/
/* poor man's filtering :)
*/
bool iai_extract_udp(struct rte_mbuf *m, uint32_t *pDstAddress, uint32_t *pSrcAddress, uint16_t *pPort){

    struct rte_ether_hdr *eth_hdr;
    struct rte_ipv4_hdr  *ip_hdr;
    struct rte_udp_hdr  *udp_hdr;

	  eth_hdr = rte_pktmbuf_mtod(m,struct rte_ether_hdr *);
		ip_hdr = (struct rte_ipv4_hdr *)(eth_hdr + 1);

    // printf("Has packet %d bytes, type: %d %d .\n", m->data_len, ip_hdr->next_proto_id, ntohs(eth_hdr->ether_type));

    if (ip_hdr->next_proto_id != 17){
    //    printf("Not IP4/UDP.\n");
      return false;
    };

   //  printf("Dest = %x ? %x \n",(ntohl(ip_hdr->dst_addr) & ipDstMask) , ipDstAddress);

    *pSrcAddress = ntohl(ip_hdr->src_addr);
    *pDstAddress = ntohl(ip_hdr->dst_addr);
	   udp_hdr     = (struct rte_udp_hdr *)(ip_hdr + 1);
    *pPort       = ntohs(udp_hdr->dst_port);
  return true;
}