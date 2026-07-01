/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_init.c - Library initialization
 */

#include "vaxp/vaxpui.h"

/* Helper macro for stringification */
#define VAXP_STR_IMPL(x) #x
#define VAXP_STR(x) VAXP_STR_IMPL(x)

static VaxpBool g_initialized = VAXP_FALSE;

VaxpResult vaxp_init(void) {
    if (g_initialized) {
        return VAXP_OK_UNIT();
    }
    
    /* Initialize subsystems here */
    g_initialized = VAXP_TRUE;
    
    return VAXP_OK_UNIT();
}

void vaxp_shutdown(void) {
    if (!g_initialized) return;
    
    /* Cleanup subsystems here */
    
#ifdef VAXP_DEBUG
    vaxp_memory_report();
#endif
    
    g_initialized = VAXP_FALSE;
}

const char* vaxp_version_string(void) {
    return "VAXPUI " VAXP_STR(VAXP_VERSION_MAJOR) "." 
           VAXP_STR(VAXP_VERSION_MINOR) "." 
           VAXP_STR(VAXP_VERSION_PATCH);
}
