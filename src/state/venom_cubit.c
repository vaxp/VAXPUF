/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_cubit.c - Cubit implementation
 */

#include "venom/state/venom_cubit.h"
#include <string.h>

/* ============================================================================
 * LIFECYCLE
 * ============================================================================ */

static void cubit_destructor(void* ptr) {
    VenomCubitBase* base = (VenomCubitBase*)ptr;
    
    /* Clear all listeners */
    VenomCubitListener* listener = base->listeners;
    while (listener) {
        VenomCubitListener* next = listener->next;
        venom_free(listener, sizeof(VenomCubitListener));
        listener = next;
    }
    base->listeners = NULL;
}

void venom_cubit_init(void* cubit, VenomSize state_size, VenomSize cubit_size) {
    VenomCubitBase* base = (VenomCubitBase*)cubit;
    
    venom_ref_init(base, cubit_size, cubit_destructor, "VenomCubit");
    base->listeners = NULL;
    base->state_size = state_size;
    base->is_closed = VENOM_FALSE;
}

void* venom_cubit_create_raw(VenomSize state_size, VenomSize cubit_size) {
    void* cubit = venom_alloc_zeroed(cubit_size);
    if (!cubit) return NULL;
    
    venom_cubit_init(cubit, state_size, cubit_size);
    return cubit;
}

void venom_cubit_close(void* cubit) {
    if (!cubit) return;
    VenomCubitBase* base = (VenomCubitBase*)cubit;
    base->is_closed = VENOM_TRUE;
}

VenomBool venom_cubit_is_closed(void* cubit) {
    if (!cubit) return VENOM_TRUE;
    return ((VenomCubitBase*)cubit)->is_closed;
}

/* ============================================================================
 * STATE MANAGEMENT
 * ============================================================================ */

void* venom_cubit_get_state(void* cubit) {
    if (!cubit) return NULL;
    
    /* State is right after the base structure */
    VenomCubitBase* base = (VenomCubitBase*)cubit;
    return (VenomU8*)cubit + sizeof(VenomCubitBase);
    (void)base; /* Suppress warning */
}

void venom_cubit_emit_raw(void* cubit, const void* state_ptr) {
    if (!cubit || !state_ptr) return;
    
    VenomCubitBase* base = (VenomCubitBase*)cubit;
    
    if (base->is_closed) {
        /* Cubit is closed, ignore emit */
        return;
    }
    
    /* Copy new state */
    void* current_state = venom_cubit_get_state(cubit);
    memcpy(current_state, state_ptr, base->state_size);
    
    /* Notify all listeners */
    VenomCubitListener* listener = base->listeners;
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

VenomCubitListener* venom_cubit_listen(void* cubit, VenomStateListener callback, void* user_data) {
    if (!cubit || !callback) return NULL;
    
    VenomCubitBase* base = (VenomCubitBase*)cubit;
    
    VenomCubitListener* listener = (VenomCubitListener*)venom_alloc_zeroed(sizeof(VenomCubitListener));
    if (!listener) return NULL;
    
    listener->callback = callback;
    listener->user_data = user_data;
    listener->next = base->listeners;
    base->listeners = listener;
    
    /* Immediately notify with current state */
    void* current_state = venom_cubit_get_state(cubit);
    callback(cubit, current_state, user_data);
    
    return listener;
}

void venom_cubit_unlisten(void* cubit, VenomCubitListener* listener) {
    if (!cubit || !listener) return;
    
    VenomCubitBase* base = (VenomCubitBase*)cubit;
    
    VenomCubitListener** current = &base->listeners;
    while (*current) {
        if (*current == listener) {
            *current = listener->next;
            venom_free(listener, sizeof(VenomCubitListener));
            return;
        }
        current = &(*current)->next;
    }
}

void venom_cubit_clear_listeners(void* cubit) {
    if (!cubit) return;
    
    VenomCubitBase* base = (VenomCubitBase*)cubit;
    
    VenomCubitListener* listener = base->listeners;
    while (listener) {
        VenomCubitListener* next = listener->next;
        venom_free(listener, sizeof(VenomCubitListener));
        listener = next;
    }
    base->listeners = NULL;
}
