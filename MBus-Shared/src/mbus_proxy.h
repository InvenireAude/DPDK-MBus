#ifndef __MBUS_Proxy_h
#define __MBUS_Proxy_h

#include <stdint.h>
#include <stdbool.h>
typedef bool (*iai_interface_dequeue_ptr)(uint16_t ring_id, void** pp_data, uint16_t *p_data_len, void** pp_pkt);
typedef bool (*iai_interface_enqueue_ptr)(uint16_t ring_id, void* p_data, uint16_t data_len, void* p_pkt);
typedef void (*iai_interface_free_ptr)(void *p_pkt);
typedef void (*iai_interface_alloc_ptr)(void **pp_pkt, void **pp_data, uint16_t data_len);

struct proxy_interface {
  iai_interface_dequeue_ptr p_dequeue;
  iai_interface_enqueue_ptr p_enqueue;
  iai_interface_free_ptr    p_free;
  iai_interface_alloc_ptr   p_alloc;
};


#endif