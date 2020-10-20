
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>

#include <stdbool.h>

#include "../../MBus-Shared/src/mbus_common.h"

#include <signal.h>

#include "setup.h"
#include "interface.h"
#include "backend.h"



int main(int argc, char *argv[]) {

  iai_backend_open("libFEX-ExchangeLib.so");

  iai_backend_setup_ptr fun_setup = iai_backend_get_setup_fun("fex_proxy_setup");
  iai_backend_work_ptr fun_work  = iai_backend_get_work_fun("fex_proxy_work");

  (*fun_setup)(&proxy_interface);

	int retval;
	if ((retval = rte_eal_init(argc, argv)) < 0)
		return -1;

  iai_setup_client();

  (*fun_work)();

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