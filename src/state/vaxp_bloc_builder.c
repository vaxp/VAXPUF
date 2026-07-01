/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_bloc_builder.c - BlocBuilder implementation
 */

#include "vaxp/state/vaxp_bloc_builder.h"
#include "vaxp/core/vaxp_memory.h"

/* Forward declare rebuild trigger */
extern void vaxp_rebuild(void);

/* ============================================================================
 * BLOC BUILDER CLASS METHODS
 * ============================================================================ */

static void bloc_builder_init(VaxpWidget* widget) {
    VaxpBlocBuilder* bb = (VaxpBlocBuilder*)widget;
    bb->cubit = NULL;
    bb->builder = NULL;
    bb->builder_data = NULL;
    bb->listener = NULL;
    bb->current_child = NULL;
}

static void bloc_builder_destroy(VaxpWidget* widget) {
    VaxpBlocBuilder* bb = (VaxpBlocBuilder*)widget;
    
    /* Unsubscribe from cubit */
    if (bb->cubit && bb->listener) {
        vaxp_cubit_unlisten(bb->cubit, bb->listener);
        bb->listener = NULL;
    }
    
    /* Release cubit reference */
    if (bb->cubit) {
        vaxp_unref(bb->cubit);
        bb->cubit = NULL;
    }
    
    /* Child is managed by widget system, no need to unref here */
    bb->current_child = NULL;
    
    /* Call parent destroy */
    vaxp_widget_class.destroy(widget);
}

static void bloc_builder_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                                  VaxpF32* out_width, VaxpF32* out_height) {
    VaxpBlocBuilder* bb = (VaxpBlocBuilder*)widget;
    
    if (bb->current_child && bb->current_child->visible) {
        vaxp_widget_measure(bb->current_child, available_width, available_height, out_width, out_height);
    } else {
        *out_width = 0;
        *out_height = 0;
    }
}

static void bloc_builder_layout(VaxpWidget* widget, VaxpRectF bounds) {
    VaxpBlocBuilder* bb = (VaxpBlocBuilder*)widget;
    
    /* Set our bounds */
    widget->bounds = bounds;
    
    /* Layout child with same bounds (pass-through) */
    if (bb->current_child) {
        VaxpRectF child_bounds = { 0, 0, bounds.width, bounds.height };
        vaxp_widget_layout(bb->current_child, child_bounds);
    }
}

static void bloc_builder_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpBlocBuilder* bb = (VaxpBlocBuilder*)widget;
    
    /* Draw child */
    if (bb->current_child && bb->current_child->visible) {
        vaxp_widget_draw(bb->current_child, canvas);
    }
}

/* ============================================================================
 * STATE CHANGE HANDLER
 * ============================================================================ */

static void on_state_changed(void* cubit, void* state, void* user_data) {
    VaxpBlocBuilder* bb = (VaxpBlocBuilder*)user_data;
    (void)cubit;
    
    if (!bb || !bb->builder) return;
    
    /* Remove old child */
    if (bb->current_child) {
        vaxp_widget_remove_all_children((VaxpWidget*)bb);
        bb->current_child = NULL;
    }
    
    /* Build new child */
    VaxpWidget* new_child = bb->builder(state, bb->builder_data);
    if (new_child) {
        vaxp_widget_add_child((VaxpWidget*)bb, new_child);
        bb->current_child = new_child;
        vaxp_unref(new_child);  /* Parent owns it now */
    }
    
    /* Request redraw */
    vaxp_widget_invalidate_layout((VaxpWidget*)bb);
    
    /* Trigger app rebuild if available */
    vaxp_rebuild();
}

/* ============================================================================
 * BLOC BUILDER CLASS
 * ============================================================================ */

const VaxpWidgetClass vaxp_bloc_builder_class = {
    .class_name = "VaxpBlocBuilder",
    .instance_size = sizeof(VaxpBlocBuilder),
    .parent_class = &vaxp_widget_class,
    .init = bloc_builder_init,
    .destroy = bloc_builder_destroy,
    .measure = bloc_builder_measure,
    .layout = bloc_builder_layout,
    .draw = bloc_builder_draw,
    .on_event = NULL,  /* Events handled by children */
    .on_state_changed = NULL,
};

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VaxpResultPtr vaxp_bloc_builder_create(void* cubit, VaxpBlocBuilderFn builder, void* user_data) {
    if (!cubit || !builder) {
        return VAXP_ERR_PTR(VAXP_ERROR_NULL_POINTER);
    }
    
    VaxpResultPtr result = vaxp_widget_create(&vaxp_bloc_builder_class);
    if (!result.ok) return result;
    
    VaxpBlocBuilder* bb = (VaxpBlocBuilder*)result.value;
    
    /* Store cubit reference */
    bb->cubit = vaxp_ref(cubit);
    bb->builder = builder;
    bb->builder_data = user_data;
    
    /* Subscribe to state changes */
    bb->listener = vaxp_cubit_listen(cubit, on_state_changed, bb);
    if (!bb->listener) {
        vaxp_unref(bb);
        return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
    }
    
    /* Note: on_state_changed is called immediately by vaxp_cubit_listen,
       so current_child is already built */
    
    return VAXP_OK_PTR(bb);
}

void vaxp_bloc_builder_rebuild(VaxpBlocBuilder* bb) {
    if (!bb || !bb->cubit) return;
    
    void* state = vaxp_cubit_get_state(bb->cubit);
    on_state_changed(bb->cubit, state, bb);
}
