/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_cubit.h - Cubit state management (like Flutter Cubit)
 * 
 * Cubit is a lightweight state management solution where
 * state changes are triggered directly via methods.
 */

#ifndef VAXP_CUBIT_H
#define VAXP_CUBIT_H

#include "vaxp/core/vaxp_types.h"
#include "vaxp/core/vaxp_ref.h"
#include "vaxp/core/vaxp_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * LISTENER TYPES
 * ============================================================================ */

/**
 * @brief Callback when state changes
 */
typedef void (*VaxpStateListener)(void* cubit, void* state, void* user_data);

/**
 * @brief Listener node in linked list
 */
typedef struct VaxpCubitListener {
    VaxpStateListener callback;
    void* user_data;
    struct VaxpCubitListener* next;
} VaxpCubitListener;

/* ============================================================================
 * CUBIT BASE STRUCTURE
 * ============================================================================ */

/**
 * @brief Base cubit header - must be first in any cubit struct
 * 
 * Usage:
 *   typedef struct MyCubit {
 *       VAXP_CUBIT_BASE;
 *       MyState state;
 *   } MyCubit;
 */
typedef struct VaxpCubitBase {
    VAXP_REF_HEADER;
    VaxpCubitListener* listeners;
    VaxpSize state_size;
    VaxpBool is_closed;
} VaxpCubitBase;

/**
 * @brief Macro to embed cubit base in custom cubit
 */
#define VAXP_CUBIT_BASE VaxpCubitBase _cubit_base

/**
 * @brief Helper macro to define a complete Cubit type
 * 
 * Usage:
 *   VAXP_DEFINE_CUBIT(Counter, CounterState);
 *   // Creates: typedef struct CounterCubit { ... } CounterCubit;
 */
#define VAXP_DEFINE_CUBIT(Name, StateType) \
    typedef struct Name##Cubit { \
        VAXP_CUBIT_BASE; \
        StateType state; \
    } Name##Cubit

/* ============================================================================
 * CUBIT LIFECYCLE
 * ============================================================================ */

/**
 * @brief Initialize cubit base (call from cubit create function)
 */
void vaxp_cubit_init(void* cubit, VaxpSize state_size, VaxpSize cubit_size);

/**
 * @brief Create a cubit with given state size
 * 
 * For typed cubits, use the generated create functions.
 */
void* vaxp_cubit_create_raw(VaxpSize state_size, VaxpSize cubit_size);

/**
 * @brief Destroy cubit (decrements ref count)
 */
#define vaxp_cubit_destroy(cubit) vaxp_unref(cubit)

/**
 * @brief Close cubit (no more state changes allowed)
 */
void vaxp_cubit_close(void* cubit);

/**
 * @brief Check if cubit is closed
 */
VaxpBool vaxp_cubit_is_closed(void* cubit);

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
 *   vaxp_cubit_emit(my_cubit, &new_state);
 */
void vaxp_cubit_emit_raw(void* cubit, const void* state_ptr);

/**
 * @brief Type-safe emit macro
 * 
 * Usage:
 *   VAXP_CUBIT_EMIT(counter_cubit, CounterState, { .count = 5 });
 */
#define VAXP_CUBIT_EMIT(cubit, StateType, ...) \
    do { \
        StateType _new_state = __VA_ARGS__; \
        vaxp_cubit_emit_raw((cubit), &_new_state); \
    } while(0)

/**
 * @brief Get current state (read-only)
 */
void* vaxp_cubit_get_state(void* cubit);

/**
 * @brief Type-safe state access macro
 */
#define VAXP_CUBIT_STATE(cubit, StateType) \
    (*(StateType*)vaxp_cubit_get_state(cubit))

/* ============================================================================
 * LISTENERS
 * ============================================================================ */

/**
 * @brief Add listener for state changes
 * 
 * @return Listener handle (for removal)
 */
VaxpCubitListener* vaxp_cubit_listen(void* cubit, VaxpStateListener callback, void* user_data);

/**
 * @brief Remove listener
 */
void vaxp_cubit_unlisten(void* cubit, VaxpCubitListener* listener);

/**
 * @brief Remove all listeners
 */
void vaxp_cubit_clear_listeners(void* cubit);

/* ============================================================================
 * UTILITY MACROS
 * ============================================================================ */

/**
 * @brief Create a typed cubit with initial state
 * 
 * Usage:
 *   CounterCubit* c = VAXP_CUBIT_CREATE(Counter, CounterState, { .count = 0 });
 */
#define VAXP_CUBIT_CREATE(Name, StateType, ...) \
    ({ \
        Name##Cubit* _c = (Name##Cubit*)vaxp_cubit_create_raw(sizeof(StateType), sizeof(Name##Cubit)); \
        if (_c) { \
            _c->state = (StateType)__VA_ARGS__; \
        } \
        _c; \
    })

/**
 * @brief Simple cubit creation without initial state
 */
#define VAXP_CUBIT_NEW(Name, StateType) \
    ((Name##Cubit*)vaxp_cubit_create_raw(sizeof(StateType), sizeof(Name##Cubit)))

#ifdef __cplusplus
}
#endif

#endif /* VAXP_CUBIT_H */
