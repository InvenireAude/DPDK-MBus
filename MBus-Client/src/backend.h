#ifndef __IAI_Backend_h
#define __IAI_Backend_h

#include "../../MBus-Shared/src/mbus_proxy.h"

iai_backend_core_ptr iai_backend_open(const char* lib_name, const char* fun_name);

#endif