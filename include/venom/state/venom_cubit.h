/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_cubit.h - Cubit state management (like Flutter Cubit)
 * 
 * Cubit is a lightweight state management solution where
 * state changes are triggered directly via methods.
 */

#ifndef VENOM_CUBIT_H
#define VENOM_CUBIT_H

#include "venom/core/venom_types.h"
#include "venom/core/venom_ref.h"
#include "venom/core/venom_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * LISTENER TYPES
 * ============================================================================ */

/**
 * @brief Callback when state changes
 */
typedef void (*VenomStateListener)(void* cubit, void* state, void* user_data);

/**
 * @brief Listener node in linked list
 */
typedef struct VenomCubitListener {
    VenomStateListener callback;
    void* user_data;
    struct VenomCubitListener* next;
} VenomCubitListener;

/* ============================================================================
 * CUBIT BASE STRUCTURE
 * ============================================================================ */

/**
 * @brief Base cubit header - must be first in any cubit struct
 * 
 * Usage:
 *   typedef struct MyCubit {
 *       VENOM_CUBIT_BASE;
 *       MyState state;
 *   } MyCubit;
 */
typedef struct VenomCubitBase {
    VENOM_REF_HEADER;
    VenomCubitListener* listeners;
    VenomSize state_size;
    VenomBool is_closed;
} VenomCubitBase;

/**
 * @brief Macro to embed cubit base in custom cubit
 */
#define VENOM_CUBIT_BASE VenomCubitBase _cubit_base

/**
 * @brief Helper macro to define a complete Cubit type
 * 
 * Usage:
 *   VENOM_DEFINE_CUBIT(Counter, CounterState);
 *   // Creates: typedef struct CounterCubit { ... } CounterCubit;
 */
#define VENOM_DEFINE_CUBIT(Name, StateType) \
    typedef struct Name##Cubit { \
        VENOM_CUBIT_BASE; \
        StateType state; \
    } Name##Cubit

/* ============================================================================
 * CUBIT LIFECYCLE
 * ============================================================================ */

/**
 * @brief Initialize cubit base (call from cubit create function)
 */
void venom_cubit_init(void* cubit, VenomSize state_size, VenomSize cubit_size);

/**
 * @brief Create a cubit with given state size
 * 
 * For typed cubits, use the generated create functions.
 */
void* venom_cubit_create_raw(VenomSize state_size, VenomSize cubit_size);

/**
 * @brief Destroy cubit (decrements ref count)
 */
#define venom_cubit_destroy(cubit) venom_unref(cubit)

/**
 * @brief Close cubit (no more state changes allowed)
 */
void venom_cubit_close(void* cubit);

/**
 * @brief Check if cubit is closed
 */
VenomBool venom_cubit_is_closed(void* cubit);

/* ============================================================================
 * STATE MANAGEMENT
 * ============================================================================ */

/**
 * @brief Emit new state (notifies all listeners)
 * 
 * @param cubit The cubit instance
 * @param state_ptr Pointer to new state data
 * 
 * Usage:
 *   CounterState new_state = { .count = old_count + 1 };
 *   venom_cubit_emit(my_cubit, &new_state);
 */
void venom_cubit_emit_raw(void* cubit, const void* state_ptr);

/**
 * @brief Type-safe emit macro
 * 
 * Usage:
 *   VENOM_CUBIT_EMIT(counter_cubit, CounterState, { .count = 5 });
 */
#define VENOM_CUBIT_EMIT(cubit, StateType, ...) \
    do { \
        StateType _new_state = __VA_ARGS__; \
        venom_cubit_emit_raw((cubit), &_new_state); \
    } while(0)

/**
 * @brief Get current state (read-only)
 */
void* venom_cubit_get_state(void* cubit);

/**
 * @brief Type-safe state access macro
 */
#define VENOM_CUBIT_STATE(cubit, StateType) \
    (*(StateType*)venom_cubit_get_state(cubit))

/* ============================================================================
 * LISTENERS
 * ============================================================================ */

/**
 * @brief Add listener for state changes
 * 
 * @return Listener handle (for removal)
 */
VenomCubitListener* venom_cubit_listen(void* cubit, VenomStateListener callback, void* user_data);

/**
 * @brief Remove listener
 */
void venom_cubit_unlisten(void* cubit, VenomCubitListener* listener);

/**
 * @brief Remove all listeners
 */
void venom_cubit_clear_listeners(void* cubit);

/* ============================================================================
 * UTILITY MACROS
 * ============================================================================ */

/**
 * @brief Create a typed cubit with initial state
 * 
 * Usage:
 *   CounterCubit* c = VENOM_CUBIT_CREATE(Counter, CounterState, { .count = 0 });
 */
#define VENOM_CUBIT_CREATE(Name, StateType, ...) \
    ({ \
        Name##Cubit* _c = (Name##Cubit*)venom_cubit_create_raw(sizeof(StateType), sizeof(Name##Cubit)); \
        if (_c) { \
            _c->state = (StateType)__VA_ARGS__; \
        } \
        _c; \
    })

/**
 * @brief Simple cubit creation without initial state
 */
#define VENOM_CUBIT_NEW(Name, StateType) \
    ((Name##Cubit*)venom_cubit_create_raw(sizeof(StateType), sizeof(Name##Cubit)))

#ifdef __cplusplus
}
#endif

#endif /* VENOM_CUBIT_H */
