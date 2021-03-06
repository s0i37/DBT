#ifndef __TRACE_EXT_H__
#define __TRACE_EXT_H__

/*
 * DO NOT MODIFY. This file is automatically generated by scripts/apigen.py,
 * based on the <plugin>_int.h file in your plugin directory.
 */

#include <dlfcn.h>
#include "panda_plugin.h"

#define API_PLUGIN_NAME "trace"
#define IMPORT_PPP(module, func_name) { \
 __##func_name = (func_name##_t) dlsym(module, #func_name); \
 char *err = dlerror(); \
 if (err) { \
    printf("Couldn't find %s function in library %s.\n", #func_name, API_PLUGIN_NAME); \
    printf("Error: %s\n", err); \
    return false; \
 } \
}
static inline bool init_trace_api(void);static inline bool init_trace_api(void){
    void *module = panda_get_plugin_by_name("panda_" API_PLUGIN_NAME ".so");
    if (!module) {
        printf("In trying to add plugin, couldn't load %s plugin\n", API_PLUGIN_NAME);
        return false;
    }
    dlerror();
return true;
}

#undef API_PLUGIN_NAME
#undef IMPORT_PPP

#endif
