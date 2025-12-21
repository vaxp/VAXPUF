/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_bloc_builder.c - BlocBuilder implementation
 */

#include "venom/state/venom_bloc_builder.h"
#include "venom/core/venom_memory.h"

/* Forward declare rebuild trigger */
extern void venom_rebuild(void);

/* ============================================================================
 * BLOC BUILDER CLASS METHODS
 * ============================================================================ */

static void bloc_builder_init(VenomWidget* widget) {
    VenomBlocBuilder* bb = (VenomBlocBuilder*)widget;
    bb->cubit = NULL;
    bb->builder = NULL;
    bb->builder_data = NULL;
    bb->listener = NULL;
    bb->current_child = NULL;
}

static void bloc_builder_destroy(VenomWidget* widget) {
    VenomBlocBuilder* bb = (VenomBlocBuilder*)widget;
    
    /* Unsubscribe from cubit */
    if (bb->cubit && bb->listener) {
        venom_cubit_unlisten(bb->cubit, bb->listener);
        bb->listener = NULL;
    }
    
    /* Release cubit reference */
    if (bb->cubit) {
        venom_unref(bb->cubit);
        bb->cubit = NULL;
    }
    
    /* Child is managed by widget system, no need to unref here */
    bb->current_child = NULL;
    
    /* Call parent destroy */
    venom_widget_class.destroy(widget);
}

static void bloc_builder_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                                  VenomF32* out_width, VenomF32* out_height) {
    VenomBlocBuilder* bb = (VenomBlocBuilder*)widget;
    
    if (bb->current_child && bb->current_child->visible) {
        venom_widget_measure(bb->current_child, available_width, available_height, out_width, out_height);
    } else {
        *out_width = 0;
        *out_height = 0;
    }
}

static void bloc_builder_layout(VenomWidget* widget, VenomRectF bounds) {
    VenomBlocBuilder* bb = (VenomBlocBuilder*)widget;
    
    /* Set our bounds */
    widget->bounds = bounds;
    
    /* Layout child with same bounds (pass-through) */
    if (bb->current_child) {
        VenomRectF child_bounds = { 0, 0, bounds.width, bounds.height };
        venom_widget_layout(bb->current_child, child_bounds);
    }
}

static void bloc_builder_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomBlocBuilder* bb = (VenomBlocBuilder*)widget;
    
    /* Draw child */
    if (bb->current_child && bb->current_child->visible) {
        venom_widget_draw(bb->current_child, canvas);
    }
}

/* ============================================================================
 * STATE CHANGE HANDLER
 * ============================================================================ */

static void on_state_changed(void* cubit, void* state, void* user_data) {
    VenomBlocBuilder* bb = (VenomBlocBuilder*)user_data;
    (void)cubit;
    
    if (!bb || !bb->builder) return;
    
    /* Remove old child */
    if (bb->current_child) {
        venom_widget_remove_all_children((VenomWidget*)bb);
        bb->current_child = NULL;
    }
    
    /* Build new child */
    VenomWidget* new_child = bb->builder(state, bb->builder_data);
    if (new_child) {
        venom_widget_add_child((VenomWidget*)bb, new_child);
        bb->current_child = new_child;
        venom_unref(new_child);  /* Parent owns it now */
    }
    
    /* Request redraw */
    venom_widget_invalidate_layout((VenomWidget*)bb);
    
    /* Trigger app rebuild if available */
    venom_rebuild();
}

/* ============================================================================
 * BLOC BUILDER CLASS
 * ============================================================================ */

const VenomWidgetClass venom_bloc_builder_class = {
    .class_name = "VenomBlocBuilder",
    .instance_size = sizeof(VenomBlocBuilder),
    .parent_class = &venom_widget_class,
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

VenomResultPtr venom_bloc_builder_create(void* cubit, VenomBlocBuilderFn builder, void* user_data) {
    if (!cubit || !builder) {
        return VENOM_ERR_PTR(VENOM_ERROR_NULL_POINTER);
    }
    
    VenomResultPtr result = venom_widget_create(&venom_bloc_builder_class);
    if (!result.ok) return result;
    
    VenomBlocBuilder* bb = (VenomBlocBuilder*)result.value;
    
    /* Store cubit reference */
    bb->cubit = venom_ref(cubit);
    bb->builder = builder;
    bb->builder_data = user_data;
    
    /* Subscribe to state changes */
    bb->listener = venom_cubit_listen(cubit, on_state_changed, bb);
    if (!bb->listener) {
        venom_unref(bb);
        return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    /* Note: on_state_changed is called immediately by venom_cubit_listen,
       so current_child is already built */
    
    return VENOM_OK_PTR(bb);
}

void venom_bloc_builder_rebuild(VenomBlocBuilder* bb) {
    if (!bb || !bb->cubit) return;
    
    void* state = venom_cubit_get_state(bb->cubit);
    on_state_changed(bb->cubit, state, bb);
}
