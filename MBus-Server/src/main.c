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
static uint16_t start_port      = 34001;
static uint16_t num_port_pairs  = 2;

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

          uint16_t port;

          bool bMatch = TheMainContext.flow ? true : iai_filter(m, DEST_IP, FULL_MASK,
                                                                start_port, start_port + 2*num_port_pairs,
                                                                &port);

          if(bMatch){

            char     *data;
            uint16_t data_len;
            size_t   sequence;
            mbus_extract(m, &data, &data_len, &sequence);
            printf("Data %08ld %d %ld\n",sequence, data_len, (long)data);

            if( (port - start_port) & 0x1 ){
              printf("Who has it ? [%d] %ld - %ld" ,port, *(size_t*)data, sequence);
              rte_pktmbuf_free(m);
            }else{
              uint16_t ring_id = (port - start_port) / 2;
              nb_enq = rte_ring_sp_enqueue(TheMainContext.ring_pairs[ring_id].ring_in, m);
              printf("Result %d, size: %d \n", nb_enq, rte_ring_count(TheMainContext.ring_pairs[ring_id].ring_in));
            }

          }else{
            rte_pktmbuf_free(m);
          }
				}
			}

    struct rte_mbuf *m;

    for(int ring_id=0; ring_id<TheMainContext.nr_ring_pairs; ring_id++){

      int rc = rte_ring_sc_dequeue(TheMainContext.ring_pairs[ring_id].ring_out, (void**)&m);

      if(rc == 0){
        printf("Data from client [%.*s]\n", m->data_len, rte_pktmbuf_mtod(m, char *));
        mbus_prepare(m, sequence, start_port + 2*ring_id);
        sequence = (sequence + 1) % 5;
        int rc = rte_eth_tx_burst(TheMainContext.port_id, 0, &m, 1);
        printf("rc = %d\n",rc);
      }

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

  iai_init_rings(num_port_pairs);

  iai_init_flow(DEST_IP, FULL_MASK, 34000, 34003);

	main_loop();

	return 0;
}
