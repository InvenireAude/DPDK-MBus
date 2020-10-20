#include "backend.h"

#include <dlfcn.h>
#include <rte_eal.h>

static void* _dl_handle;

/*****************************************************************************/
iai_backend_core_ptr iai_backend_open(const char* lib_name, const char* fun_name){

	if( (_dl_handle=dlopen(lib_name, RTLD_NOW | RTLD_GLOBAL)) == NULL){
		rte_exit(EXIT_FAILURE, "dlopen error: %s \n", dlerror());
  }

  iai_backend_core_ptr result;

	if( (result=dlsym(_dl_handle, fun_name)) == NULL){
    	rte_exit(EXIT_FAILURE, "dlsym error: %s \n", dlerror());
  }

  return result;
}
/*****************************************************************************/