/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_bloc_builder.h - Widget that rebuilds when Cubit/Bloc state changes
 */

#ifndef VAXP_BLOC_BUILDER_H
#define VAXP_BLOC_BUILDER_H

#include "vaxp/widgets/vaxp_widget.h"
#include "vaxp/state/vaxp_cubit.h"

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
typedef VaxpWidget* (*VaxpBlocBuilderFn)(void* state, void* user_data);

/**
 * @brief BlocBuilder widget structure
 */
typedef struct VaxpBlocBuilder {
    VaxpWidget base;
    
    void* cubit;                    /* Cubit being listened to */
    VaxpBlocBuilderFn builder;     /* Builder function */
    void* builder_data;             /* User data for builder */
    VaxpCubitListener* listener;   /* Listener handle */
    VaxpWidget* current_child;     /* Currently built child widget */
} VaxpBlocBuilder;

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
VaxpResultPtr vaxp_bloc_builder_create(void* cubit, VaxpBlocBuilderFn builder, void* user_data);

/**
 * @brief Trigger a manual rebuild
 */
void vaxp_bloc_builder_rebuild(VaxpBlocBuilder* bb);

/* Class definition */
extern const VaxpWidgetClass vaxp_bloc_builder_class;

/* ============================================================================
 * CONVENIENCE MACRO
 * ============================================================================ */

/**
 * @brief Create a bloc builder with type-safe state access
 * 
 * Usage:
 *   VAXP_BLOC_BUILDER(my_cubit, CounterState, state, {
 *       return vaxp_text_fmt("Count: %d", state.count);
 *   })
 */
#define VAXP_BLOC_BUILDER(cubit, StateType, state_var, builder_body) \
    ({ \
        VaxpWidget* _builder_fn(void* _state_ptr, void* _user_data) { \
            (void)_user_data; \
            StateType state_var = *(StateType*)_state_ptr; \
            builder_body \
        } \
        vaxp_bloc_builder_create((cubit), _builder_fn, NULL).value; \
    })

#ifdef __cplusplus
}
#endif

#endif /* VAXP_BLOC_BUILDER_H */
