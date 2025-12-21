/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_init.c - Library initialization
 */

#include "venom/venomui.h"

/* Helper macro for stringification */
#define VENOM_STR_IMPL(x) #x
#define VENOM_STR(x) VENOM_STR_IMPL(x)

static VenomBool g_initialized = VENOM_FALSE;

VenomResult venom_init(void) {
    if (g_initialized) {
        return VENOM_OK_UNIT();
    }
    
    /* Initialize subsystems here */
    g_initialized = VENOM_TRUE;
    
    return VENOM_OK_UNIT();
}

void venom_shutdown(void) {
    if (!g_initialized) return;
    
    /* Cleanup subsystems here */
    
#ifdef VENOM_DEBUG
    venom_memory_report();
#endif
    
    g_initialized = VENOM_FALSE;
}

const char* venom_version_string(void) {
    return "VENOMUI " VENOM_STR(VENOM_VERSION_MAJOR) "." 
           VENOM_STR(VENOM_VERSION_MINOR) "." 
           VENOM_STR(VENOM_VERSION_PATCH);
}
