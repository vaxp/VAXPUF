/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_cubit.c - Cubit implementation
 */

#include "vaxp/state/vaxp_cubit.h"
#include <string.h>

/* ============================================================================
 * LIFECYCLE
 * ============================================================================ */

static void cubit_destructor(void* ptr) {
    VaxpCubitBase* base = (VaxpCubitBase*)ptr;
    
    /* Clear all listeners */
    VaxpCubitListener* listener = base->listeners;
    while (listener) {
        VaxpCubitListener* next = listener->next;
        vaxp_free(listener, sizeof(VaxpCubitListener));
        listener = next;
    }
    base->listeners = NULL;
}

void vaxp_cubit_init(void* cubit, VaxpSize state_size, VaxpSize cubit_size) {
    VaxpCubitBase* base = (VaxpCubitBase*)cubit;
    
    vaxp_ref_init(base, cubit_size, cubit_destructor, "VaxpCubit");
    base->listeners = NULL;
    base->state_size = state_size;
    base->is_closed = VAXP_FALSE;
}

void* vaxp_cubit_create_raw(VaxpSize state_size, VaxpSize cubit_size) {
    void* cubit = vaxp_alloc_zeroed(cubit_size);
    if (!cubit) return NULL;
    
    vaxp_cubit_init(cubit, state_size, cubit_size);
    return cubit;
}

void vaxp_cubit_close(void* cubit) {
    if (!cubit) return;
    VaxpCubitBase* base = (VaxpCubitBase*)cubit;
    base->is_closed = VAXP_TRUE;
}

VaxpBool vaxp_cubit_is_closed(void* cubit) {
    if (!cubit) return VAXP_TRUE;
    return ((VaxpCubitBase*)cubit)->is_closed;
}

/* ============================================================================
 * STATE MANAGEMENT
 * ============================================================================ */

void* vaxp_cubit_get_state(void* cubit) {
    if (!cubit) return NULL;
    
    /* State is right after the base structure */
    VaxpCubitBase* base = (VaxpCubitBase*)cubit;
    return (VaxpU8*)cubit + sizeof(VaxpCubitBase);
    (void)base; /* Suppress warning */
}

void vaxp_cubit_emit_raw(void* cubit, const void* state_ptr) {
    if (!cubit || !state_ptr) return;
    
    VaxpCubitBase* base = (VaxpCubitBase*)cubit;
    
    if (base->is_closed) {
        /* Cubit is closed, ignore emit */
        return;
    }
    
    /* Copy new state */
    void* current_state = vaxp_cubit_get_state(cubit);
    memcpy(current_state, state_ptr, base->state_size);
    
    /* Notify all listeners */
    VaxpCubitListener* listener = base->listeners;
    while (listener) {
        if (listener->callback) {
            listener->callback(cubit, current_state, listener->user_data);
        }
        listener = listener->next;
    }
}

/* ============================================================================
 * LISTENERS
 * ============================================================================ */

VaxpCubitListener* vaxp_cubit_listen(void* cubit, VaxpStateListener callback, void* user_data) {
    if (!cubit || !callback) return NULL;
    
    VaxpCubitBase* base = (VaxpCubitBase*)cubit;
    
    VaxpCubitListener* listener = (VaxpCubitListener*)vaxp_alloc_zeroed(sizeof(VaxpCubitListener));
    if (!listener) return NULL;
    
    listener->callback = callback;
    listener->user_data = user_data;
    listener->next = base->listeners;
    base->listeners = listener;
    
    /* Immediately notify with current state */
    void* current_state = vaxp_cubit_get_state(cubit);
    callback(cubit, current_state, user_data);
    
    return listener;
}

void vaxp_cubit_unlisten(void* cubit, VaxpCubitListener* listener) {
    if (!cubit || !listener) return;
    
    VaxpCubitBase* base = (VaxpCubitBase*)cubit;
    
    VaxpCubitListener** current = &base->listeners;
    while (*current) {
        if (*current == listener) {
            *current = listener->next;
            vaxp_free(listener, sizeof(VaxpCubitListener));
            return;
        }
        current = &(*current)->next;
    }
}

void vaxp_cubit_clear_listeners(void* cubit) {
    if (!cubit) return;
    
    VaxpCubitBase* base = (VaxpCubitBase*)cubit;
    
    VaxpCubitListener* listener = base->listeners;
    while (listener) {
        VaxpCubitListener* next = listener->next;
        vaxp_free(listener, sizeof(VaxpCubitListener));
        listener = next;
    }
    base->listeners = NULL;
}
