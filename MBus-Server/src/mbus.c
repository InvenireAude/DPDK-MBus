
#include "mbus.h"

#include <rte_ether.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include <rte_ethdev.h>
#include "setup.h"
#include "utils.h"

static const uint16_t _hdr_len = sizeof(struct ether_hdr) + sizeof(struct ipv4_hdr) + sizeof(struct udp_hdr);

/*******************************************************************************/
void mbus_extract(struct rte_mbuf *m, char** pp_data, uint16_t *p_data_len, size_t *p_sequence){

  m->data_off += _hdr_len;
  m->data_len -= _hdr_len + sizeof(size_t);

  *pp_data = rte_pktmbuf_mtod(m, char *);

  *p_data_len = m->data_len;

  *p_sequence = *((size_t*)(*pp_data + *p_data_len));

  m->seqn = *p_sequence; // this is 32 bit integer mbus sequence is 64 bit !!!

}
/*******************************************************************************/
#define DEST_IP ((226<<24) + (1<<16) + (1<<8) + 1) /* dest ip = 192.168.1.1 */
void mbus_prepare(struct rte_mbuf *created_pkt, size_t sequence, size_t port){


  char *pkt_data = rte_pktmbuf_mtod(created_pkt, char *);
  uint16_t orig_data_len = created_pkt->data_len;

  size_t *p_sequence = (size_t*)(pkt_data + orig_data_len);
  (*p_sequence) = sequence;

  orig_data_len += sizeof(size_t);

  struct udp_hdr *udp_hdr = (struct udp_hdr *)pkt_data;
  udp_hdr--;
  created_pkt->data_off -= sizeof(struct udp_hdr);

  udp_hdr->src_port = htons(55555);
  udp_hdr->dst_port = htons(port);
  udp_hdr->dgram_len = htons(sizeof(struct udp_hdr) + orig_data_len);

  struct ipv4_hdr *ipv4_hdr = (struct ipv4_hdr *)udp_hdr;
  ipv4_hdr--;
  created_pkt->data_off -= sizeof(struct ipv4_hdr);
  ipv4_hdr->version_ihl=0x45;
  ipv4_hdr->type_of_service = 0;
  ipv4_hdr->time_to_live=1;
  // ipv4_hdr->hdr_checksum

  ipv4_hdr->total_length=htons(sizeof(struct udp_hdr) + sizeof(struct ipv4_hdr) + orig_data_len);
  ipv4_hdr->packet_id=htons(0x1234);
  ipv4_hdr->fragment_offset=htons(0x4000);
  ipv4_hdr->dst_addr=htonl(DEST_IP);
  ipv4_hdr->src_addr=htonl(0x99999999);
  ipv4_hdr->hdr_checksum = 0;
  ipv4_hdr->next_proto_id = 17;

  struct ether_hdr *ether_hdr = (struct ether_hdr *)ipv4_hdr;
  ether_hdr--;
  created_pkt->data_off -= sizeof(struct ether_hdr);

  rte_eth_macaddr_get(TheMainContext.port_id, &ether_hdr->s_addr);
  /* Set multicast address 01-1B-19-00-00-00. */
  ether_addr_copy(&ether_multicast, &ether_hdr->d_addr);
  ether_hdr->ether_type = htons(0x0800);

  created_pkt->data_len = _hdr_len + orig_data_len;
  created_pkt->pkt_len = _hdr_len + orig_data_len;

  created_pkt->l2_len = sizeof(*ether_hdr);
  created_pkt->l3_len = sizeof(*ipv4_hdr);
  created_pkt->ol_flags |= PKT_TX_IPV4 | PKT_TX_UDP_CKSUM;

  ipv4_hdr->hdr_checksum = rte_ipv4_cksum(ipv4_hdr);

  /* Transmit the packet. */
}
/*******************************************************************************/