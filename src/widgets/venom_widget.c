/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_widget.c - Base widget implementation
 */

#include "venom/widgets/venom_widget.h"
#include "venom/core/venom_memory.h"
#include <string.h>

/* ============================================================================
 * BASE CLASS METHODS (Default implementations)
 * ============================================================================ */

static void base_widget_init(VenomWidget* self) {
    /* Default initialization - already zeroed by alloc */
    self->visible = VENOM_TRUE;
    self->needs_layout = VENOM_TRUE;
    self->needs_redraw = VENOM_TRUE;
    
    /* Default layout */
    self->layout.flex_shrink = 1.0f;
    self->layout.max_width = 1e10f;  /* Effectively unlimited */
    self->layout.max_height = 1e10f;
}

static void base_widget_destroy(VenomWidget* self) {
    /* Remove all children */
    venom_widget_remove_all_children(self);
    
    /* Free children array */
    if (self->children) {
        venom_free(self->children, sizeof(VenomWidget*) * self->children_capacity);
        self->children = NULL;
        self->children_count = 0;
        self->children_capacity = 0;
    }
}

static void base_widget_measure(VenomWidget* self, VenomF32 available_width, VenomF32 available_height,
                                 VenomF32* out_width, VenomF32* out_height) {
    (void)available_width;
    (void)available_height;
    
    /* Default: use preferred size or min size */
    *out_width = self->layout.preferred_width > 0 ? 
                 self->layout.preferred_width : self->layout.min_width;
    *out_height = self->layout.preferred_height > 0 ? 
                  self->layout.preferred_height : self->layout.min_height;
    
    /* Add padding */
    *out_width += self->layout.padding.left + self->layout.padding.right;
    *out_height += self->layout.padding.top + self->layout.padding.bottom;
}

static void base_widget_layout(VenomWidget* self, VenomRectF bounds) {
    self->bounds = bounds;
    
    /* Calculate content rect (bounds minus padding) */
    self->content_rect.x = bounds.x + self->layout.padding.left;
    self->content_rect.y = bounds.y + self->layout.padding.top;
    self->content_rect.width = bounds.width - self->layout.padding.left - self->layout.padding.right;
    self->content_rect.height = bounds.height - self->layout.padding.top - self->layout.padding.bottom;
    
    if (self->content_rect.width < 0) self->content_rect.width = 0;
    if (self->content_rect.height < 0) self->content_rect.height = 0;
    
    self->needs_layout = VENOM_FALSE;
}

static void base_widget_draw(VenomWidget* self, VenomCanvas* canvas) {
    (void)self;
    (void)canvas;
    /* Base widget draws nothing */
}

static VenomBool base_widget_on_event(VenomWidget* self, const VenomEvent* event) {
    (void)self;
    (void)event;
    return VENOM_FALSE;  /* Not handled */
}

static void base_widget_on_state_changed(VenomWidget* self, VenomWidgetState old_state) {
    (void)old_state;
    venom_widget_invalidate(self);
}

/* ============================================================================
 * BASE WIDGET CLASS DEFINITION
 * ============================================================================ */

const VenomWidgetClass venom_widget_class = {
    .class_name = "VenomWidget",
    .instance_size = sizeof(VenomWidget),
    .parent_class = NULL,
    .init = base_widget_init,
    .destroy = base_widget_destroy,
    .measure = base_widget_measure,
    .layout = base_widget_layout,
    .draw = base_widget_draw,
    .on_event = base_widget_on_event,
    .on_state_changed = base_widget_on_state_changed,
};

/* ============================================================================
 * WIDGET CREATION
 * ============================================================================ */

static void widget_destructor(void* self) {
    VenomWidget* widget = (VenomWidget*)self;
    /* Walk up class hierarchy to find destroy method */
    const VenomWidgetClass* klass = widget->klass;
    while (klass) {
        if (klass->destroy) {
            klass->destroy(widget);
            return;
        }
        klass = klass->parent_class;
    }
}

VenomResultPtr venom_widget_create(const VenomWidgetClass* klass) {
    VENOM_RETURN_IF_NULL(klass, VENOM_ERR_PTR(VENOM_ERROR_NULL_POINTER));
    
    /* Allocate */
    VenomWidget* widget = (VenomWidget*)venom_alloc_zeroed(klass->instance_size);
    if (!widget) {
        return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    /* Initialize ref header */
    venom_ref_init(widget, klass->instance_size, widget_destructor, klass->class_name);
    
    /* Initialize base */
    venom_widget_init_base(widget, klass);
    
    /* Call class init */
    if (klass->init) {
        klass->init(widget);
    }
    
    return VENOM_OK_PTR(widget);
}

void venom_widget_init_base(VenomWidget* widget, const VenomWidgetClass* klass) {
    widget->klass = klass;
    widget->visible = VENOM_TRUE;
    widget->needs_layout = VENOM_TRUE;
    widget->needs_redraw = VENOM_TRUE;
    widget->layout.flex_shrink = 1.0f;
    widget->layout.max_width = 1e10f;
    widget->layout.max_height = 1e10f;
}

/* ============================================================================
 * HIERARCHY
 * ============================================================================ */

#define INITIAL_CHILDREN_CAPACITY 8

VenomResult venom_widget_add_child(VenomWidget* parent, VenomWidget* child) {
    VENOM_ENSURE_NOT_NULL(parent);
    VENOM_ENSURE_NOT_NULL(child);
    
    /* Check if child already has parent */
    if (child->parent != NULL) {
        return VENOM_ERR_UNIT(VENOM_ERROR_INVALID_STATE);
    }
    
    /* Grow array if needed */
    if (parent->children_count >= parent->children_capacity) {
        VenomU32 new_cap = parent->children_capacity == 0 ? 
                           INITIAL_CHILDREN_CAPACITY : parent->children_capacity * 2;
        VenomWidget** new_children = VENOM_REALLOC_ARRAY(VenomWidget*, parent->children,
                                                          parent->children_capacity, new_cap);
        if (!new_children) {
            return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
        }
        parent->children = new_children;
        parent->children_capacity = new_cap;
    }
    
    /* Add child */
    parent->children[parent->children_count++] = (VenomWidget*)venom_ref(child);
    child->parent = parent;
    
    venom_widget_invalidate_layout(parent);
    
    return VENOM_OK_UNIT();
}

VenomResult venom_widget_remove_child(VenomWidget* parent, VenomWidget* child) {
    VENOM_ENSURE_NOT_NULL(parent);
    VENOM_ENSURE_NOT_NULL(child);
    
    /* Find child */
    for (VenomU32 i = 0; i < parent->children_count; i++) {
        if (parent->children[i] == child) {
            /* Remove from array (shift remaining) */
            for (VenomU32 j = i; j < parent->children_count - 1; j++) {
                parent->children[j] = parent->children[j + 1];
            }
            parent->children_count--;
            
            child->parent = NULL;
            venom_unref(child);
            
            venom_widget_invalidate_layout(parent);
            return VENOM_OK_UNIT();
        }
    }
    
    return VENOM_ERR_UNIT(VENOM_ERROR_WIDGET_NOT_CHILD);
}

void venom_widget_remove_all_children(VenomWidget* parent) {
    if (!parent) return;
    
    for (VenomU32 i = 0; i < parent->children_count; i++) {
        VenomWidget* child = parent->children[i];
        child->parent = NULL;
        venom_unref(child);
    }
    parent->children_count = 0;
    
    venom_widget_invalidate_layout(parent);
}

/* ============================================================================
 * LAYOUT & GEOMETRY
 * ============================================================================ */

void venom_widget_invalidate_layout(VenomWidget* widget) {
    if (!widget) return;
    
    widget->needs_layout = VENOM_TRUE;
    widget->needs_redraw = VENOM_TRUE;
    
    /* Propagate to parent */
    if (widget->parent) {
        venom_widget_invalidate_layout(widget->parent);
    }
}

void venom_widget_invalidate(VenomWidget* widget) {
    if (!widget) return;
    
    widget->needs_redraw = VENOM_TRUE;
    
    /* Propagate to parent */
    if (widget->parent) {
        venom_widget_invalidate(widget->parent);
    }
}

void venom_widget_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                          VenomF32* out_width, VenomF32* out_height) {
    if (!widget || !widget->klass || !widget->klass->measure) {
        *out_width = 0;
        *out_height = 0;
        return;
    }
    
    widget->klass->measure(widget, available_width, available_height, out_width, out_height);
    
    /* Apply constraints */
    const VenomLayoutProps* l = &widget->layout;
    *out_width = VENOM_CLAMP(*out_width, l->min_width, l->max_width);
    *out_height = VENOM_CLAMP(*out_height, l->min_height, l->max_height);
}

void venom_widget_layout(VenomWidget* widget, VenomRectF bounds) {
    if (!widget || !widget->klass) return;
    /* Walk up class hierarchy to find layout method */
    const VenomWidgetClass* klass = widget->klass;
    while (klass) {
        if (klass->layout) {
            klass->layout(widget, bounds);
            return;
        }
        klass = klass->parent_class;
    }
}

void venom_widget_set_bounds(VenomWidget* widget, VenomRectF bounds) {
    if (!widget) return;
    
    if (memcmp(&widget->bounds, &bounds, sizeof(VenomRectF)) != 0) {
        widget->bounds = bounds;
        venom_widget_invalidate_layout(widget);
    }
}

VenomPointF venom_widget_to_screen(const VenomWidget* widget, VenomF32 x, VenomF32 y) {
    VenomPointF p = { x, y };
    
    while (widget) {
        p.x += widget->bounds.x;
        p.y += widget->bounds.y;
        widget = widget->parent;
    }
    
    return p;
}

VenomPointF venom_widget_from_screen(const VenomWidget* widget, VenomF32 x, VenomF32 y) {
    VenomPointF p = { x, y };
    
    while (widget) {
        p.x -= widget->bounds.x;
        p.y -= widget->bounds.y;
        widget = widget->parent;
    }
    
    return p;
}

/* ============================================================================
 * RENDERING
 * ============================================================================ */

void venom_widget_draw(VenomWidget* widget, VenomCanvas* canvas) {
    if (!widget || !widget->visible) return;
    
    /* Save canvas state */
    venom_canvas_save(canvas);
    
    /* Translate to widget position */
    venom_canvas_translate(canvas, widget->bounds.x, widget->bounds.y);
    
    /* Clip to widget bounds */
    VenomRectF clip = { 0, 0, widget->bounds.width, widget->bounds.height };
    venom_canvas_clip_rect(canvas, clip);
    
    /* Draw this widget */
    if (widget->klass && widget->klass->draw) {
        widget->klass->draw(widget, canvas);
    }
    
    /* Draw children */
    for (VenomU32 i = 0; i < widget->children_count; i++) {
        venom_widget_draw(widget->children[i], canvas);
    }
    
    /* Restore canvas state */
    venom_canvas_restore(canvas);
    
    widget->needs_redraw = VENOM_FALSE;
}

/* ============================================================================
 * EVENT HANDLING
 * ============================================================================ */

VenomBool venom_widget_dispatch_event(VenomWidget* widget, const VenomEvent* event) {
    if (!widget || !widget->visible || !venom_widget_is_enabled(widget)) {
        return VENOM_FALSE;
    }
    
    /* For mouse events, check if the mouse is within this widget's bounds */
    VenomBool is_mouse_event = (event->type == VENOM_EVENT_MOUSE_BUTTON_DOWN ||
                                 event->type == VENOM_EVENT_MOUSE_BUTTON_UP ||
                                 event->type == VENOM_EVENT_MOUSE_MOVE);
    
    if (is_mouse_event) {
        /* Get mouse position (in parent coordinates) */
        VenomF32 mx = event->mouse.x;
        VenomF32 my = event->mouse.y;
        
        /* Check if mouse is within this widget's bounds */
        if (mx < widget->bounds.x || mx >= widget->bounds.x + widget->bounds.width ||
            my < widget->bounds.y || my >= widget->bounds.y + widget->bounds.height) {
            return VENOM_FALSE;  /* Mouse is outside this widget */
        }
        
        /* Create adjusted event with local coordinates for children */
        VenomEvent local_event = *event;
        local_event.mouse.x = mx - widget->bounds.x;
        local_event.mouse.y = my - widget->bounds.y;
        
        /* Try to dispatch to children (in reverse order for proper z-order) */
        for (VenomI32 i = (VenomI32)widget->children_count - 1; i >= 0; i--) {
            if (venom_widget_dispatch_event(widget->children[i], &local_event)) {
                return VENOM_TRUE;
            }
        }
        
        /* Then handle in this widget */
        const VenomWidgetClass* klass = widget->klass;
        while (klass) {
            if (klass->on_event) {
                return klass->on_event(widget, &local_event);
            }
            klass = klass->parent_class;
        }
    } else {
        /* Non-mouse events: dispatch to all children */
        for (VenomI32 i = (VenomI32)widget->children_count - 1; i >= 0; i--) {
            if (venom_widget_dispatch_event(widget->children[i], event)) {
                return VENOM_TRUE;
            }
        }
        
        /* Then handle in this widget - walk up class hierarchy */
        const VenomWidgetClass* klass = widget->klass;
        while (klass) {
            if (klass->on_event) {
                return klass->on_event(widget, event);
            }
            klass = klass->parent_class;
        }
    }
    
    return VENOM_FALSE;
}

VenomWidget* venom_widget_hit_test(VenomWidget* widget, VenomF32 x, VenomF32 y) {
    if (!widget || !widget->visible) return NULL;
    
    /* Check if point is in bounds */
    if (x < widget->bounds.x || x >= widget->bounds.x + widget->bounds.width ||
        y < widget->bounds.y || y >= widget->bounds.y + widget->bounds.height) {
        return NULL;
    }
    
    /* Convert to local coords */
    VenomF32 local_x = x - widget->bounds.x;
    VenomF32 local_y = y - widget->bounds.y;
    
    /* Check children (in reverse order for proper z-order) */
    for (VenomI32 i = (VenomI32)widget->children_count - 1; i >= 0; i--) {
        VenomWidget* hit = venom_widget_hit_test(widget->children[i], local_x, local_y);
        if (hit) return hit;
    }
    
    return widget;
}

/* ============================================================================
 * STATE
 * ============================================================================ */

void venom_widget_set_state(VenomWidget* widget, VenomWidgetState state) {
    if (!widget || widget->state == state) return;
    
    VenomWidgetState old = widget->state;
    widget->state = state;
    
    /* Walk up class hierarchy for on_state_changed */
    const VenomWidgetClass* klass = widget->klass;
    while (klass) {
        if (klass->on_state_changed) {
            klass->on_state_changed(widget, old);
            return;
        }
        klass = klass->parent_class;
    }
}

void venom_widget_add_state(VenomWidget* widget, VenomWidgetState flag) {
    if (!widget) return;
    venom_widget_set_state(widget, widget->state | flag);
}

void venom_widget_remove_state(VenomWidget* widget, VenomWidgetState flag) {
    if (!widget) return;
    venom_widget_set_state(widget, widget->state & ~flag);
}

void venom_widget_set_enabled(VenomWidget* widget, VenomBool enabled) {
    if (!widget) return;
    
    if (enabled) {
        venom_widget_remove_state(widget, VENOM_WIDGET_STATE_DISABLED);
    } else {
        venom_widget_add_state(widget, VENOM_WIDGET_STATE_DISABLED);
    }
}

void venom_widget_set_visible(VenomWidget* widget, VenomBool visible) {
    if (!widget || widget->visible == visible) return;
    
    widget->visible = visible;
    venom_widget_invalidate_layout(widget->parent);
}
