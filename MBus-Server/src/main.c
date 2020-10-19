#include <stdint.h>
#include <inttypes.h>

#include <rte_eal.h>
#include <rte_common.h>
#include <rte_malloc.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_net.h>

#include <rte_cycles.h>

#include <rte_ip.h>
#include <rte_udp.h>

#include "setup.h"
#include "flow.h"
#include "mbus.h"
#include "utils.h"

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32


#define DEST_IP ((226<<24) + (1<<16) + (1<<8) + 1) /* dest ip = 192.168.1.1 */
#define FULL_MASK 0xffffffff /* full mask */
#define EMPTY_MASK 0x0 /* empty mask */

static size_t sequence = 0L;

static void main_loop(void)
{
	struct rte_mbuf *mbufs[32];
	struct rte_flow_error error;
	uint16_t nb_rx, nb_enq;
	uint16_t i;
	uint16_t j;

	while (!iai_is_quit()) {
		for (i = 0; i < TheMainContext.nr_queues; i++) {

      nb_rx = rte_eth_rx_burst(TheMainContext.port_id, i, mbufs, 32);

      if (nb_rx) {

      	for (j = 0; j < nb_rx; j++) {
					struct rte_mbuf *m = mbufs[j];

          bool bMatch = TheMainContext.flow ? true : iai_filter(m, DEST_IP, FULL_MASK, 34001, 34001);

          if(bMatch){

            char     *data;
            uint16_t data_len;
            size_t   sequence;

            mbus_extract(m, &data, &data_len, &sequence);
            printf("Data %08ld %d %ld\n",sequence, data_len, (long)data);
            //printf("Data %08ld [%.*s]\n",sequence, data_len, data);

            nb_enq = rte_ring_sp_enqueue(TheMainContext.ring_in, m);
            printf("Result %d, size: %d \n", nb_enq, rte_ring_count(TheMainContext.ring_in));

          }else{
            rte_pktmbuf_free(m);
          }
				}
			}

    struct rte_mbuf *m;
    int rc = rte_ring_sc_dequeue(TheMainContext.ring_out, (void**)&m);
    if(rc == 0){
      printf("Data from client [%.*s]\n", m->data_len, rte_pktmbuf_mtod(m, char *));
      mbus_prepare(m, sequence);
      sequence = (sequence + 1) % 5;
      int rc = rte_eth_tx_burst(TheMainContext.port_id, 0, &m, 1);
      printf("rc = %d\n",rc);
    }

		}
	}

	/* closing and releasing resources */
	rte_flow_flush(TheMainContext.port_id, &error);
	rte_eth_dev_stop(TheMainContext.port_id);
	rte_eth_dev_close(TheMainContext.port_id);
}

int main(int argc, char **argv){


  int ret;


	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, ":: invalid EAL arguments\n");

	iai_setup_signals();


  iai_setup_mbuf();
  iai_init_shared();
	iai_init_ports();
  iai_init_rings();
  iai_init_flow(DEST_IP, FULL_MASK, 34000, 34003);

	main_loop();

	return 0;
}
