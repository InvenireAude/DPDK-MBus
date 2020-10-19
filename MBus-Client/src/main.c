
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>

#include "../../MBus-Shared/src/mbus_common.h"

struct MainContext {
  struct rte_ring *ring_in;
  struct rte_ring *ring_out;
};

struct MainContext TheMainContext;

int main(int argc, char *argv[]) {

  const struct rte_memzone *mz;
	struct rte_mempool *mp;
	int retval;

	if ((retval = rte_eal_init(argc, argv)) < 0)
		return -1;

	mp = rte_mempool_lookup(MBUS_SHARED_MEM_POOL);
	if (mp == NULL)
		rte_exit(EXIT_FAILURE, "Cannot get mempool for mbufs\n");

	mz = rte_memzone_lookup(MBUS_SHARED_INFO_NAME);
	if (mz == NULL)
		rte_exit(EXIT_FAILURE, "Cannot get port info structure\n");


  TheMainContext.ring_in = rte_ring_lookup(MBUS_SHARED_RING_IN);
  TheMainContext.ring_out = rte_ring_lookup(MBUS_SHARED_RING_OUT);

  printf("Rings: %lx %lx \n",(long)TheMainContext.ring_in, (long)TheMainContext.ring_out);

  for(;;){
    struct rte_mbuf *m;
    int rc = rte_ring_sc_dequeue(TheMainContext.ring_in, (void**)&m);
    if(rc == 0){
      printf("Data [%.*s]\n", m->data_len, rte_pktmbuf_mtod(m, char *));
      rte_pktmbuf_free(m);

      struct rte_mbuf * created_pkt = rte_pktmbuf_alloc(mp);
      char *data = rte_pktmbuf_mtod(created_pkt, char *);
      strcpy(data,"Message from client !!");
      created_pkt->data_len = strlen(data);

      int nb_enq = rte_ring_sp_enqueue(TheMainContext.ring_out, created_pkt);
      printf("Result %d, size: %d \n", nb_enq, rte_ring_count(TheMainContext.ring_out));

    }
  }

  struct shared_info *shared    = NULL;
  printf("Address: %lx \n",(long)mz->addr);
	shared = mz->addr;

	printf("Message: %s \n",shared->somedata);

}
