
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>

#include <stdbool.h>

#include "../../MBus-Shared/src/mbus_common.h"

#include "setup.h"
#include "interface.h"

int main(int argc, char *argv[]) {

	int retval;

	if ((retval = rte_eal_init(argc, argv)) < 0)
		return -1;

  iai_setup_client();

  for(;;){

    //for(int ring_id=0; ring_id<TheMainContext.nr_ring_pairs; ring_id++){
      int ring_id = 0;
        char *data;
        uint16_t data_len;
        void *ctx;
        if(proxy_interface.p_dequeue(ring_id, (void**)&data, &data_len, &ctx)){
          printf("Data %d %lx\n", data_len, (long)data);
          printf("Data 1[%.*s]\n", data_len, data);
          proxy_interface.p_free(ctx);

          proxy_interface.p_alloc(&ctx,(void**)&data,123);
          strcpy(data,"Message from client !!");

          proxy_interface.p_enqueue(ring_id ^ 1, data, strlen(data), ctx);

        //}
      }
    }

}

/*
 int rc = rte_ring_sc_dequeue(TheMainContext.ring_pairs[ring_id].ring_in, (void**)&m);
      if(rc == 0){
        printf("Data [%.*s]\n", m->data_len, rte_pktmbuf_mtod(m, char *));
        rte_pktmbuf_free(m);

        struct rte_mbuf * created_pkt = rte_pktmbuf_alloc(TheMainContext.mp);
        char *data = rte_pktmbuf_mtod(created_pkt, char *);
        strcpy(data,"Message from client !!");
        created_pkt->data_len = strlen(data);

        int nb_enq = rte_ring_sp_enqueue(TheMainContext.ring_pairs[ring_id ^ 1].ring_out, created_pkt);
        printf("Result %d, size: %d \n", nb_enq, rte_ring_count(TheMainContext.ring_pairs[ring_id ^ 1].ring_out));

*/