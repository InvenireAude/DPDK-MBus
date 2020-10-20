
#include "setup.h"

#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <rte_flow.h>

#include "flow.h"


/*******************************************************************************/
struct MainContext TheMainContext  = {
  .nr_queues = 1,
  .port_id = 0,
  .nr_ring_pairs = 0,
  .mbuf_pool = NULL
};

/*******************************************************************************/
#define CHECK_INTERVAL 1000  /* 100ms */
#define MAX_REPEAT_TIMES 90  /* 9s (90 * 100ms) in total */
#define NO_FLAGS 0

/*******************************************************************************/
struct shared_info *shared    = NULL;
/*******************************************************************************/
void iai_setup_mbuf(void){
	TheMainContext.mbuf_pool = rte_pktmbuf_pool_create(MBUS_SHARED_MEM_POOL, 4095, 128, 0,
					    RTE_MBUF_DEFAULT_BUF_SIZE,
					    rte_socket_id());
	if (TheMainContext.mbuf_pool == NULL)
		rte_exit(EXIT_FAILURE, "Cannot init mbuf pool\n");

}
/*******************************************************************************/
/* set up array for port data */
void iai_init_shared(void){

  const struct rte_memzone *mz;

	mz = rte_memzone_reserve(MBUS_SHARED_INFO_NAME, sizeof(*shared),
				rte_socket_id(), NO_FLAGS);

	if (mz == NULL)
		rte_exit(EXIT_FAILURE, "Cannot rte_memzone_reserve \n");

  memset(mz->addr, 0, sizeof(*shared));

  shared = mz->addr;

  sprintf(shared->somedata,"Hello World !!!");
}
/*******************************************************************************/
void iai_assert_link_status(uint16_t port_id){

  struct rte_eth_link link;
	uint8_t iCounter = MAX_REPEAT_TIMES;

	memset(&link, 0, sizeof(link));
	do {
		rte_eth_link_get(port_id, &link);
		if (link.link_status == ETH_LINK_UP)
			break;
		rte_delay_ms(CHECK_INTERVAL);
	} while (--iCounter);

	if (link.link_status == ETH_LINK_DOWN)
		rte_exit(EXIT_FAILURE, ":: error: link is still down\n");
}
/*******************************************************************************/

/*******************************************************************************/
void iai_init_flow(uint32_t ipDstAddress,    uint32_t ipDstMask,
                  uint16_t ipPortStart,     uint16_t ipPortEnd){

	struct rte_flow_error error;

	TheMainContext.flow = iai_generate_flow(TheMainContext.port_id, 1, ipDstAddress, ipDstMask, ipPortStart, ipPortEnd, &error);

	if (!TheMainContext.flow) {
		printf("Flow can't be created %d message: %s\n",
			error.type,
			error.message ? error.message : "(no stated reason)");
		//rte_exit(EXIT_FAILURE, "error in creating flow");
	}

}
/*******************************************************************************/
#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024

/*******************************************************************************/
void iai_init_port(uint16_t port_id){

	int ret;
	uint16_t i;
  int nr_queues = 1;

	uint16_t nb_rxd = RX_RING_SIZE;
	uint16_t nb_txd = TX_RING_SIZE;

	struct rte_eth_conf port_conf = {
		.rxmode = {
			.split_hdr_size = 0,
		},
		.txmode = {
			.offloads =
				DEV_TX_OFFLOAD_VLAN_INSERT |
				DEV_TX_OFFLOAD_IPV4_CKSUM  |
				DEV_TX_OFFLOAD_UDP_CKSUM   |
				DEV_TX_OFFLOAD_TCP_CKSUM   |
				DEV_TX_OFFLOAD_SCTP_CKSUM  |
				DEV_TX_OFFLOAD_TCP_TSO,
		},
	};
	struct rte_eth_txconf txq_conf;
	struct rte_eth_rxconf rxq_conf;
	struct rte_eth_dev_info dev_info;

	rte_eth_dev_info_get(port_id, &dev_info);
	port_conf.txmode.offloads &= dev_info.tx_offload_capa;

  printf(":: initializing port: %d\n", port_id);
	ret = rte_eth_dev_configure(port_id, nr_queues, nr_queues, &port_conf);
	if (ret < 0) {
		rte_exit(EXIT_FAILURE,
			":: cannot configure device: err=%d, port=%u\n",
			ret, port_id);
	}

	ret = rte_eth_dev_adjust_nb_rx_tx_desc(port_id, &nb_rxd, &nb_txd);
	if(ret != 0){
		rte_exit(EXIT_FAILURE,
			":: rte_eth_dev_adjust_nb_rx_tx_desc: err=%d, port=%u\n",
			ret, port_id);
	}

	rxq_conf = dev_info.default_rxconf;
	rxq_conf.offloads = port_conf.rxmode.offloads;

	for (i = 0; i < nr_queues; i++) {
		ret = rte_eth_rx_queue_setup(port_id, i, nb_rxd, rte_eth_dev_socket_id(port_id), &rxq_conf, TheMainContext.mbuf_pool);
		if (ret < 0) {
			rte_exit(EXIT_FAILURE,
				":: Rx queue setup failed: err=%d, port=%u\n",
				ret, port_id);
		}
	}

	txq_conf = dev_info.default_txconf;
	txq_conf.offloads = port_conf.txmode.offloads;

	for (i = 0; i < nr_queues; i++) {
		ret = rte_eth_tx_queue_setup(port_id, i, nb_txd, rte_eth_dev_socket_id(port_id), &txq_conf);
		if (ret < 0) {
			rte_exit(EXIT_FAILURE,
				":: Tx queue setup failed: err=%d, port=%u\n",
				ret, port_id);
		}
	}

	rte_eth_promiscuous_enable(port_id);

	ret = rte_eth_dev_start(port_id);
	if (ret < 0) {
		rte_exit(EXIT_FAILURE,
			"rte_eth_dev_start:err=%d, port=%u\n",
			ret, port_id);
	}

	iai_assert_link_status(port_id);

  /* Display the port MAC address. */
	struct ether_addr addr;
	rte_eth_macaddr_get(port_id, &addr);
	printf("Port %u MAC: %02" PRIx8 " %02" PRIx8 " %02" PRIx8
			   " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
			port_id,
			addr.addr_bytes[0], addr.addr_bytes[1],
			addr.addr_bytes[2], addr.addr_bytes[3],
			addr.addr_bytes[4], addr.addr_bytes[5]);

	printf(":: initializing port: %d done\n", port_id);

}
/*******************************************************************************/
void iai_init_ports(void){

	uint16_t nr_ports;

  nr_ports = rte_eth_dev_count_avail();

	if (nr_ports == 0)
		rte_exit(EXIT_FAILURE, ":: no Ethernet ports found\n");

  TheMainContext.port_id = 0;

	if (nr_ports != 1) {
		printf(":: warn: %d ports detected, but we use only one: port %u\n",
			nr_ports, TheMainContext.port_id);
	}

  iai_init_port(TheMainContext.port_id);
}
/*******************************************************************************/
#define RING_IN_SIZE 1024
#define RING_OUT_SIZE 1024

void iai_init_ring_pair(struct ring_pair* rings, uint16_t id){

  printf("Initializing rings [ %d ]... \n", id);
	rings->ring_in= rte_ring_create(iai_get_ring_name(MBUS_SHARED_RING_IN, id), RING_IN_SIZE,  rte_socket_id(), RING_F_SP_ENQ | RING_F_SC_DEQ );
  if (rings->ring_in == NULL)
			rte_exit(EXIT_FAILURE, "Cannot create ring input queue ");

	rings->ring_out= rte_ring_create(iai_get_ring_name(MBUS_SHARED_RING_OUT, id), RING_OUT_SIZE,  rte_socket_id(), RING_F_SP_ENQ | RING_F_SC_DEQ );
  if (rings->ring_out == NULL)
			rte_exit(EXIT_FAILURE, "Cannot create ring output queue ");

  printf("Done! \n");
}
/*******************************************************************************/
void iai_init_rings(uint16_t num_ring_pairs){
  int i;
  for(i=0; i<num_ring_pairs; i++){
    iai_init_ring_pair(&TheMainContext.ring_pairs[i], i);
    TheMainContext.nr_ring_pairs++;
  }
  shared->num_ring_pairs = num_ring_pairs;
}
/*******************************************************************************/