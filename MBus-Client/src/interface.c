
#include "interface.h"

#include "setup.h"

#include <stdbool.h>

#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>

static bool iai_interface_dequeue(uint16_t ring_id, void** pp_data, uint16_t *p_data_len, void** pp_pkt){

  struct rte_mbuf *m;
  int rc = rte_ring_sc_dequeue(TheMainContext.ring_pairs[ring_id].ring_in, (void**)&m);

  if(rc != 0)
    return false;

   *pp_data    = rte_pktmbuf_mtod(m, char *);
   *p_data_len =  m->data_len;
   *pp_pkt = m;

   printf("Data [%.*s]\n", m->data_len, rte_pktmbuf_mtod(m, char *));

   return true;
}

static bool iai_interface_enqueue(uint16_t ring_id, void* p_data, uint16_t data_len, void* p_pkt){

  struct rte_mbuf *m = (struct rte_mbuf *)p_pkt;
  printf("ENQUEUE %d %lx\n", ring_id, (long)p_pkt);

  if(p_data != rte_pktmbuf_mtod(m, char *)){
    printf("p_data != rte_pktmbuf_mtod(m, char *)), ring_id: %d", ring_id);
    return false;
  }

  m->data_len = data_len;
  m->pkt_len  = data_len;

  int rc = rte_ring_mp_enqueue(TheMainContext.ring_pairs[ring_id].ring_out, (void*)m);
  printf("Ring size: %d, bytes %d \n", rte_ring_count(TheMainContext.ring_pairs[ring_id].ring_out), data_len);

  if(rc != 0)
    return false;

   return true;
}

static void iai_interface_free(void *p_pkt){
  struct rte_mbuf *m = (struct rte_mbuf *)p_pkt;
  rte_pktmbuf_free(m);
}

static void iai_interface_alloc(void **pp_pkt, void **pp_data, __attribute__((unused)) uint16_t data_len){
  //TODO check if data_len is valid
  struct rte_mbuf * created_pkt = rte_pktmbuf_alloc(TheMainContext.mp);
  *pp_pkt = created_pkt;
  *pp_data = rte_pktmbuf_mtod(created_pkt, char *);
}

struct proxy_interface proxy_interface = {
  .p_enqueue = &iai_interface_enqueue,
  .p_dequeue = &iai_interface_dequeue,
  .p_free = &iai_interface_free,
  .p_alloc = &iai_interface_alloc
};