#ifndef __IAI_Backend_h
#define __IAI_Backend_h

#include "../../MBus-Shared/src/mbus_proxy.h"

void iai_backend_open(const char* lib_name);
iai_backend_setup_ptr iai_backend_get_setup_fun(const char* fun_name);
iai_backend_work_ptr  iai_backend_get_work_fun(const char* fun_name);

#endif