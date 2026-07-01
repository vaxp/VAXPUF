/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_widget.c - Base widget implementation
 */

#include "vaxp/widgets/vaxp_widget.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

/* ============================================================================
 * BASE CLASS METHODS (Default implementations)
 * ============================================================================ */

static void base_widget_init(VaxpWidget* self) {
    /* Default initialization - already zeroed by alloc */
    self->visible = VAXP_TRUE;
    self->needs_layout = VAXP_TRUE;
    self->needs_redraw = VAXP_TRUE;
    
    /* Default layout */
    self->layout.flex_shrink = 1.0f;
    self->layout.max_width = 1e10f;  /* Effectively unlimited */
    self->layout.max_height = 1e10f;
}

static void base_widget_destroy(VaxpWidget* self) {
    /* Remove all children */
    vaxp_widget_remove_all_children(self);
    
    /* Free children array */
    if (self->children) {
        vaxp_free(self->children, sizeof(VaxpWidget*) * self->children_capacity);
        self->children = NULL;
        self->children_count = 0;
        self->children_capacity = 0;
    }
}

static void base_widget_measure(VaxpWidget* self, VaxpF32 available_width, VaxpF32 available_height,
                                 VaxpF32* out_width, VaxpF32* out_height) {
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

static void base_widget_layout(VaxpWidget* self, VaxpRectF bounds) {
    self->bounds = bounds;
    
    /* Calculate content rect (bounds minus padding) */
    self->content_rect.x = bounds.x + self->layout.padding.left;
    self->content_rect.y = bounds.y + self->layout.padding.top;
    self->content_rect.width = bounds.width - self->layout.padding.left - self->layout.padding.right;
    self->content_rect.height = bounds.height - self->layout.padding.top - self->layout.padding.bottom;
    
    if (self->content_rect.width < 0) self->content_rect.width = 0;
    if (self->content_rect.height < 0) self->content_rect.height = 0;
    
    self->needs_layout = VAXP_FALSE;
}

static void base_widget_draw(VaxpWidget* self, VaxpCanvas* canvas) {
    (void)self;
    (void)canvas;
    /* Base widget draws nothing */
}

static VaxpBool base_widget_on_event(VaxpWidget* self, const VaxpEvent* event) {
    (void)self;
    (void)event;
    return VAXP_FALSE;  /* Not handled */
}

static void base_widget_on_state_changed(VaxpWidget* self, VaxpWidgetState old_state) {
    (void)old_state;
    vaxp_widget_invalidate(self);
}

/* ============================================================================
 * BASE WIDGET CLASS DEFINITION
 * ============================================================================ */

const VaxpWidgetClass vaxp_widget_class = {
    .class_name = "VaxpWidget",
    .instance_size = sizeof(VaxpWidget),
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
    VaxpWidget* widget = (VaxpWidget*)self;
    /* Walk up class hierarchy to find destroy method */
    const VaxpWidgetClass* klass = widget->klass;
    while (klass) {
        if (klass->destroy) {
            klass->destroy(widget);
            return;
        }
        klass = klass->parent_class;
    }
}

VaxpResultPtr vaxp_widget_create(const VaxpWidgetClass* klass) {
    VAXP_RETURN_IF_NULL(klass, VAXP_ERR_PTR(VAXP_ERROR_NULL_POINTER));
    
    /* Allocate */
    VaxpWidget* widget = (VaxpWidget*)vaxp_alloc_zeroed(klass->instance_size);
    if (!widget) {
        return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
    }
    
    /* Initialize ref header */
    vaxp_ref_init(widget, klass->instance_size, widget_destructor, klass->class_name);
    
    /* Initialize base */
    vaxp_widget_init_base(widget, klass);
    
    /* Call class init */
    if (klass->init) {
        klass->init(widget);
    }
    
    return VAXP_OK_PTR(widget);
}

void vaxp_widget_init_base(VaxpWidget* widget, const VaxpWidgetClass* klass) {
    widget->klass = klass;
    widget->visible = VAXP_TRUE;
    widget->needs_layout = VAXP_TRUE;
    widget->needs_redraw = VAXP_TRUE;
    widget->layout.flex_shrink = 1.0f;
    widget->layout.max_width = 1e10f;
    widget->layout.max_height = 1e10f;
}

/* ============================================================================
 * HIERARCHY
 * ============================================================================ */

#define INITIAL_CHILDREN_CAPACITY 8

VaxpResult vaxp_widget_add_child(VaxpWidget* parent, VaxpWidget* child) {
    VAXP_ENSURE_NOT_NULL(parent);
    VAXP_ENSURE_NOT_NULL(child);
    
    /* Check if child already has parent */
    if (child->parent != NULL) {
        return VAXP_ERR_UNIT(VAXP_ERROR_INVALID_STATE);
    }
    
    /* Grow array if needed */
    if (parent->children_count >= parent->children_capacity) {
        VaxpU32 new_cap = parent->children_capacity == 0 ? 
                           INITIAL_CHILDREN_CAPACITY : parent->children_capacity * 2;
        VaxpWidget** new_children = VAXP_REALLOC_ARRAY(VaxpWidget*, parent->children,
                                                          parent->children_capacity, new_cap);
        if (!new_children) {
            return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_MEMORY);
        }
        parent->children = new_children;
        parent->children_capacity = new_cap;
    }
    
    /* Add child */
    parent->children[parent->children_count++] = (VaxpWidget*)vaxp_ref(child);
    child->parent = parent;
    
    vaxp_widget_invalidate_layout(parent);
    
    return VAXP_OK_UNIT();
}

VaxpResult vaxp_widget_remove_child(VaxpWidget* parent, VaxpWidget* child) {
    VAXP_ENSURE_NOT_NULL(parent);
    VAXP_ENSURE_NOT_NULL(child);
    
    /* Find child */
    for (VaxpU32 i = 0; i < parent->children_count; i++) {
        if (parent->children[i] == child) {
            /* Remove from array (shift remaining) */
            for (VaxpU32 j = i; j < parent->children_count - 1; j++) {
                parent->children[j] = parent->children[j + 1];
            }
            parent->children_count--;
            
            child->parent = NULL;
            vaxp_unref(child);
            
            vaxp_widget_invalidate_layout(parent);
            return VAXP_OK_UNIT();
        }
    }
    
    return VAXP_ERR_UNIT(VAXP_ERROR_WIDGET_NOT_CHILD);
}

void vaxp_widget_remove_all_children(VaxpWidget* parent) {
    if (!parent) return;
    
    for (VaxpU32 i = 0; i < parent->children_count; i++) {
        VaxpWidget* child = parent->children[i];
        child->parent = NULL;
        vaxp_unref(child);
    }
    parent->children_count = 0;
    
    vaxp_widget_invalidate_layout(parent);
}

/* ============================================================================
 * LAYOUT & GEOMETRY
 * ============================================================================ */

void vaxp_widget_invalidate_layout(VaxpWidget* widget) {
    if (!widget) return;
    
    widget->needs_layout = VAXP_TRUE;
    widget->needs_redraw = VAXP_TRUE;
    
    /* Propagate to parent */
    if (widget->parent) {
        vaxp_widget_invalidate_layout(widget->parent);
    }
}

void vaxp_widget_invalidate(VaxpWidget* widget) {
    if (!widget) return;
    
    widget->needs_redraw = VAXP_TRUE;
    
    /* Propagate to parent */
    if (widget->parent) {
        vaxp_widget_invalidate(widget->parent);
    }
}

void vaxp_widget_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                          VaxpF32* out_width, VaxpF32* out_height) {
    if (!widget || !widget->klass || !widget->klass->measure) {
        *out_width = 0;
        *out_height = 0;
        return;
    }
    
    widget->klass->measure(widget, available_width, available_height, out_width, out_height);
    
    /* Apply constraints */
    const VaxpLayoutProps* l = &widget->layout;
    *out_width = VAXP_CLAMP(*out_width, l->min_width, l->max_width);
    *out_height = VAXP_CLAMP(*out_height, l->min_height, l->max_height);
}

void vaxp_widget_layout(VaxpWidget* widget, VaxpRectF bounds) {
    if (!widget || !widget->klass) return;
    /* Walk up class hierarchy to find layout method */
    const VaxpWidgetClass* klass = widget->klass;
    while (klass) {
        if (klass->layout) {
            klass->layout(widget, bounds);
            return;
        }
        klass = klass->parent_class;
    }
}

void vaxp_widget_set_bounds(VaxpWidget* widget, VaxpRectF bounds) {
    if (!widget) return;
    
    if (memcmp(&widget->bounds, &bounds, sizeof(VaxpRectF)) != 0) {
        widget->bounds = bounds;
        vaxp_widget_invalidate_layout(widget);
    }
}

VaxpPointF vaxp_widget_to_screen(const VaxpWidget* widget, VaxpF32 x, VaxpF32 y) {
    VaxpPointF p = { x, y };
    
    while (widget) {
        p.x += widget->bounds.x;
        p.y += widget->bounds.y;
        widget = widget->parent;
    }
    
    return p;
}

VaxpPointF vaxp_widget_from_screen(const VaxpWidget* widget, VaxpF32 x, VaxpF32 y) {
    VaxpPointF p = { x, y };
    
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

void vaxp_widget_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    if (!widget || !widget->visible) return;
    
    /* Save canvas state */
    vaxp_canvas_save(canvas);
    
    /* Translate to widget position */
    vaxp_canvas_translate(canvas, widget->bounds.x, widget->bounds.y);
    
    /* Clip to widget bounds */
    VaxpRectF clip = { 0, 0, widget->bounds.width, widget->bounds.height };
    vaxp_canvas_clip_rect(canvas, clip);
    
    /* Draw this widget */
    if (widget->klass && widget->klass->draw) {
        widget->klass->draw(widget, canvas);
    }
    
    /* Draw children */
    for (VaxpU32 i = 0; i < widget->children_count; i++) {
        vaxp_widget_draw(widget->children[i], canvas);
    }
    
    /* Restore canvas state */
    vaxp_canvas_restore(canvas);
    
    widget->needs_redraw = VAXP_FALSE;
}

/* ============================================================================
 * EVENT HANDLING
 * ============================================================================ */

VaxpBool vaxp_widget_dispatch_event(VaxpWidget* widget, const VaxpEvent* event) {
    if (!widget || !widget->visible || !vaxp_widget_is_enabled(widget)) {
        return VAXP_FALSE;
    }
    
    /* For mouse events, check if the mouse is within this widget's bounds */
    VaxpBool is_mouse_event = (event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN ||
                                 event->type == VAXP_EVENT_MOUSE_BUTTON_UP ||
                                 event->type == VAXP_EVENT_MOUSE_MOVE);
    
    if (is_mouse_event) {
        /* Get mouse position (in parent coordinates) */
        VaxpF32 mx = event->mouse.x;
        VaxpF32 my = event->mouse.y;
        
        /* Check if mouse is within this widget's bounds */
        if (mx < widget->bounds.x || mx >= widget->bounds.x + widget->bounds.width ||
            my < widget->bounds.y || my >= widget->bounds.y + widget->bounds.height) {
            return VAXP_FALSE;  /* Mouse is outside this widget */
        }
        
        /* Create adjusted event with local coordinates for children */
        VaxpEvent local_event = *event;
        local_event.mouse.x = mx - widget->bounds.x;
        local_event.mouse.y = my - widget->bounds.y;
        
        /* Try to dispatch to children (in reverse order for proper z-order) */
        for (VaxpI32 i = (VaxpI32)widget->children_count - 1; i >= 0; i--) {
            if (vaxp_widget_dispatch_event(widget->children[i], &local_event)) {
                return VAXP_TRUE;
            }
        }
        
        /* Then handle in this widget */
        const VaxpWidgetClass* klass = widget->klass;
        while (klass) {
            if (klass->on_event) {
                return klass->on_event(widget, &local_event);
            }
            klass = klass->parent_class;
        }
    } else {
        /* Non-mouse events: dispatch to all children */
        for (VaxpI32 i = (VaxpI32)widget->children_count - 1; i >= 0; i--) {
            if (vaxp_widget_dispatch_event(widget->children[i], event)) {
                return VAXP_TRUE;
            }
        }
        
        /* Then handle in this widget - walk up class hierarchy */
        const VaxpWidgetClass* klass = widget->klass;
        while (klass) {
            if (klass->on_event) {
                return klass->on_event(widget, event);
            }
            klass = klass->parent_class;
        }
    }
    
    return VAXP_FALSE;
}

VaxpWidget* vaxp_widget_hit_test(VaxpWidget* widget, VaxpF32 x, VaxpF32 y) {
    if (!widget || !widget->visible) return NULL;
    
    /* Check if point is in bounds */
    if (x < widget->bounds.x || x >= widget->bounds.x + widget->bounds.width ||
        y < widget->bounds.y || y >= widget->bounds.y + widget->bounds.height) {
        return NULL;
    }
    
    /* Convert to local coords */
    VaxpF32 local_x = x - widget->bounds.x;
    VaxpF32 local_y = y - widget->bounds.y;
    
    /* Check children (in reverse order for proper z-order) */
    for (VaxpI32 i = (VaxpI32)widget->children_count - 1; i >= 0; i--) {
        VaxpWidget* hit = vaxp_widget_hit_test(widget->children[i], local_x, local_y);
        if (hit) return hit;
    }
    
    return widget;
}

/* ============================================================================
 * STATE
 * ============================================================================ */

void vaxp_widget_set_state(VaxpWidget* widget, VaxpWidgetState state) {
    if (!widget || widget->state == state) return;
    
    VaxpWidgetState old = widget->state;
    widget->state = state;
    
    /* Walk up class hierarchy for on_state_changed */
    const VaxpWidgetClass* klass = widget->klass;
    while (klass) {
        if (klass->on_state_changed) {
            klass->on_state_changed(widget, old);
            return;
        }
        klass = klass->parent_class;
    }
}

void vaxp_widget_add_state(VaxpWidget* widget, VaxpWidgetState flag) {
    if (!widget) return;
    vaxp_widget_set_state(widget, widget->state | flag);
}

void vaxp_widget_remove_state(VaxpWidget* widget, VaxpWidgetState flag) {
    if (!widget) return;
    vaxp_widget_set_state(widget, widget->state & ~flag);
}

void vaxp_widget_set_enabled(VaxpWidget* widget, VaxpBool enabled) {
    if (!widget) return;
    
    if (enabled) {
        vaxp_widget_remove_state(widget, VAXP_WIDGET_STATE_DISABLED);
    } else {
        vaxp_widget_add_state(widget, VAXP_WIDGET_STATE_DISABLED);
    }
}

void vaxp_widget_set_visible(VaxpWidget* widget, VaxpBool visible) {
    if (!widget || widget->visible == visible) return;
    
    widget->visible = visible;
    vaxp_widget_invalidate_layout(widget->parent);
}

/* ============================================================================
 * CONST WIDGETS
 * ============================================================================ */

void vaxp_widget_set_const(VaxpWidget* widget, VaxpBool is_const) {
    if (!widget) return;
    widget->is_const = is_const;
}

void vaxp_widget_set_const_key(VaxpWidget* widget, const char* key) {
    if (!widget) return;
    widget->const_key = key;  /* Note: key is borrowed, not copied */
}

