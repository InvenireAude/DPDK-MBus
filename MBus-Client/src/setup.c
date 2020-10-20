
#include "setup.h"

#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>

struct MainContext TheMainContext;

void iai_setup_client(void){

  const struct rte_memzone *mz;

	TheMainContext.mp = rte_mempool_lookup(MBUS_SHARED_MEM_POOL);
	if (TheMainContext.mp == NULL)
		rte_exit(EXIT_FAILURE, "Cannot get mempool for mbufs\n");

	mz = rte_memzone_lookup(MBUS_SHARED_INFO_NAME);
	if (mz == NULL)
		rte_exit(EXIT_FAILURE, "Cannot get port info structure\n");

  struct shared_info *shared    = NULL;
  printf("Address: %lx \n",(long)mz->addr);
	shared = mz->addr;
	printf("Message: %s \n",shared->somedata);

  TheMainContext.nr_ring_pairs = shared->num_ring_pairs;

  printf("Number of shared rings: %d \n", TheMainContext.nr_ring_pairs);

  for(int ring_id=0; ring_id<TheMainContext.nr_ring_pairs; ring_id++){
    TheMainContext.ring_pairs[ring_id].ring_in = rte_ring_lookup(iai_get_ring_name(MBUS_SHARED_RING_IN,ring_id));
    TheMainContext.ring_pairs[ring_id].ring_out = rte_ring_lookup(iai_get_ring_name(MBUS_SHARED_RING_OUT,ring_id));

    printf("Rings: %lx %lx \n",(long)TheMainContext.ring_pairs[ring_id].ring_in, (long)TheMainContext.ring_pairs[ring_id].ring_out);
  }



}