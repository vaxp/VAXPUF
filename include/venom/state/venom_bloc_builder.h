/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_bloc_builder.h - Widget that rebuilds when Cubit/Bloc state changes
 */

#ifndef VENOM_BLOC_BUILDER_H
#define VENOM_BLOC_BUILDER_H

#include "venom/widgets/venom_widget.h"
#include "venom/state/venom_cubit.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * BLOC BUILDER TYPES
 * ============================================================================ */

/**
 * @brief Builder function that creates widget based on state
 * 
 * @param state Current state from cubit
 * @param user_data User data passed to builder
 * @return New widget tree (ownership transferred)
 */
typedef VenomWidget* (*VenomBlocBuilderFn)(void* state, void* user_data);

/**
 * @brief BlocBuilder widget structure
 */
typedef struct VenomBlocBuilder {
    VenomWidget base;
    
    void* cubit;                    /* Cubit being listened to */
    VenomBlocBuilderFn builder;     /* Builder function */
    void* builder_data;             /* User data for builder */
    VenomCubitListener* listener;   /* Listener handle */
    VenomWidget* current_child;     /* Currently built child widget */
} VenomBlocBuilder;

/* ============================================================================
 * BLOC BUILDER API
 * ============================================================================ */

/**
 * @brief Create a BlocBuilder widget
 * 
 * The BlocBuilder will call the builder function whenever the cubit's state changes,
 * replacing the current child widget with the new one.
 * 
 * @param cubit The cubit to listen to
 * @param builder Function that builds widget from state
 * @param user_data User data passed to builder
 */
VenomResultPtr venom_bloc_builder_create(void* cubit, VenomBlocBuilderFn builder, void* user_data);

/**
 * @brief Trigger a manual rebuild
 */
void venom_bloc_builder_rebuild(VenomBlocBuilder* bb);

/* Class definition */
extern const VenomWidgetClass venom_bloc_builder_class;

/* ============================================================================
 * CONVENIENCE MACRO
 * ============================================================================ */

/**
 * @brief Create a bloc builder with type-safe state access
 * 
 * Usage:
 *   VENOM_BLOC_BUILDER(my_cubit, CounterState, state, {
 *       return venom_text_fmt("Count: %d", state.count);
 *   })
 */
#define VENOM_BLOC_BUILDER(cubit, StateType, state_var, builder_body) \
    ({ \
        VenomWidget* _builder_fn(void* _state_ptr, void* _user_data) { \
            (void)_user_data; \
            StateType state_var = *(StateType*)_state_ptr; \
            builder_body \
        } \
        venom_bloc_builder_create((cubit), _builder_fn, NULL).value; \
    })

#ifdef __cplusplus
}
#endif

#endif /* VENOM_BLOC_BUILDER_H */
