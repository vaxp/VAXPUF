/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_stack.c - Stack widget implementation
 */

#include "venom/widgets/venom_stack.h"
#include "venom/core/venom_memory.h"
#include <string.h>

#define INITIAL_CHILD_CAPACITY 4

/* ============================================================================
 * STACK CLASS METHODS
 * ============================================================================ */

static void stack_init(VenomWidget* widget) {
    VenomStack* stack = (VenomStack*)widget;
    
    stack->children = NULL;
    stack->child_count = 0;
    stack->child_capacity = 0;
    stack->alignment = VENOM_STACK_CENTER;
}

static void stack_destroy(VenomWidget* widget) {
    VenomStack* stack = (VenomStack*)widget;
    
    /* Unref all children */
    for (VenomU32 i = 0; i < stack->child_count; i++) {
        if (stack->children[i]) {
            stack->children[i]->parent = NULL;
            venom_unref(stack->children[i]);
        }
    }
    
    if (stack->children) {
        venom_free(stack->children, stack->child_capacity * sizeof(VenomWidget*));
        stack->children = NULL;
    }
    
    venom_widget_class.destroy(widget);
}

static void stack_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                          VenomF32* out_width, VenomF32* out_height) {
    VenomStack* stack = (VenomStack*)widget;
    
    VenomF32 max_width = 0;
    VenomF32 max_height = 0;
    
    /* Stack takes the size of its largest child */
    for (VenomU32 i = 0; i < stack->child_count; i++) {
        VenomWidget* child = stack->children[i];
        if (!child || !child->visible) continue;
        
        VenomF32 child_w, child_h;
        venom_widget_measure(child, available_width, available_height, &child_w, &child_h);
        
        if (child_w > max_width) max_width = child_w;
        if (child_h > max_height) max_height = child_h;
    }
    
    *out_width = widget->layout.preferred_width > 0 ? widget->layout.preferred_width : max_width;
    *out_height = widget->layout.preferred_height > 0 ? widget->layout.preferred_height : max_height;
}

static void stack_layout(VenomWidget* widget, VenomRectF bounds) {
    VenomStack* stack = (VenomStack*)widget;
    widget->bounds = bounds;
    
    for (VenomU32 i = 0; i < stack->child_count; i++) {
        VenomWidget* child = stack->children[i];
        if (!child || !child->visible) continue;
        
        VenomF32 child_w, child_h;
        venom_widget_measure(child, bounds.width, bounds.height, &child_w, &child_h);
        
        /* Calculate position based on alignment */
        VenomF32 x = 0, y = 0;
        
        switch (stack->alignment) {
            case VENOM_STACK_TOP_LEFT:
            case VENOM_STACK_CENTER_LEFT:
            case VENOM_STACK_BOTTOM_LEFT:
                x = 0;
                break;
            case VENOM_STACK_TOP_CENTER:
            case VENOM_STACK_CENTER:
            case VENOM_STACK_BOTTOM_CENTER:
                x = (bounds.width - child_w) / 2;
                break;
            case VENOM_STACK_TOP_RIGHT:
            case VENOM_STACK_CENTER_RIGHT:
            case VENOM_STACK_BOTTOM_RIGHT:
                x = bounds.width - child_w;
                break;
        }
        
        switch (stack->alignment) {
            case VENOM_STACK_TOP_LEFT:
            case VENOM_STACK_TOP_CENTER:
            case VENOM_STACK_TOP_RIGHT:
                y = 0;
                break;
            case VENOM_STACK_CENTER_LEFT:
            case VENOM_STACK_CENTER:
            case VENOM_STACK_CENTER_RIGHT:
                y = (bounds.height - child_h) / 2;
                break;
            case VENOM_STACK_BOTTOM_LEFT:
            case VENOM_STACK_BOTTOM_CENTER:
            case VENOM_STACK_BOTTOM_RIGHT:
                y = bounds.height - child_h;
                break;
        }
        
        VenomRectF child_bounds = { x, y, child_w, child_h };
        venom_widget_layout(child, child_bounds);
    }
}

static void stack_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomStack* stack = (VenomStack*)widget;
    
    /* Draw children in order (first added = bottom, last = top) */
    for (VenomU32 i = 0; i < stack->child_count; i++) {
        VenomWidget* child = stack->children[i];
        if (child && child->visible) {
            venom_canvas_save(canvas);
            venom_canvas_translate(canvas, child->bounds.x, child->bounds.y);
            venom_widget_draw(child, canvas);
            venom_canvas_restore(canvas);
        }
    }
}

static VenomBool stack_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomStack* stack = (VenomStack*)widget;
    
    /* Dispatch to children in reverse order (top first) */
    for (VenomI32 i = (VenomI32)stack->child_count - 1; i >= 0; i--) {
        VenomWidget* child = stack->children[i];
        if (child && child->visible) {
            if (venom_widget_dispatch_event(child, event)) {
                return VENOM_TRUE;
            }
        }
    }
    
    return VENOM_FALSE;
}

/* ============================================================================
 * STACK CLASS
 * ============================================================================ */

const VenomWidgetClass venom_stack_class = {
    .class_name = "VenomStack",
    .instance_size = sizeof(VenomStack),
    .parent_class = &venom_widget_class,
    .init = stack_init,
    .destroy = stack_destroy,
    .measure = stack_measure,
    .layout = stack_layout,
    .draw = stack_draw,
    .on_event = stack_on_event,
    .on_state_changed = NULL,
};

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VenomResultPtr venom_stack_create(void) {
    return venom_widget_create(&venom_stack_class);
}

VenomResult venom_stack_add_child(VenomStack* stack, VenomWidget* child) {
    VENOM_ENSURE_NOT_NULL(stack);
    VENOM_ENSURE_NOT_NULL(child);
    
    /* Expand capacity if needed */
    if (stack->child_count >= stack->child_capacity) {
        VenomU32 new_cap = stack->child_capacity == 0 ? INITIAL_CHILD_CAPACITY : stack->child_capacity * 2;
        VenomWidget** new_array = (VenomWidget**)venom_alloc(new_cap * sizeof(VenomWidget*));
        if (!new_array) return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
        
        if (stack->children) {
            memcpy(new_array, stack->children, stack->child_count * sizeof(VenomWidget*));
            venom_free(stack->children, stack->child_capacity * sizeof(VenomWidget*));
        }
        
        stack->children = new_array;
        stack->child_capacity = new_cap;
    }
    
    venom_ref(child);
    child->parent = (VenomWidget*)stack;
    stack->children[stack->child_count++] = child;
    
    venom_widget_invalidate_layout((VenomWidget*)stack);
    return VENOM_OK_UNIT();
}

VenomResult venom_stack_remove_child(VenomStack* stack, VenomWidget* child) {
    VENOM_ENSURE_NOT_NULL(stack);
    VENOM_ENSURE_NOT_NULL(child);
    
    for (VenomU32 i = 0; i < stack->child_count; i++) {
        if (stack->children[i] == child) {
            child->parent = NULL;
            venom_unref(child);
            
            /* Shift remaining children */
            memmove(&stack->children[i], &stack->children[i + 1], 
                    (stack->child_count - i - 1) * sizeof(VenomWidget*));
            stack->child_count--;
            
            venom_widget_invalidate_layout((VenomWidget*)stack);
            return VENOM_OK_UNIT();
        }
    }
    
    return VENOM_ERR_UNIT(VENOM_ERROR_NOT_FOUND);
}

void venom_stack_set_alignment(VenomStack* stack, VenomStackAlignment alignment) {
    if (stack) {
        stack->alignment = alignment;
        venom_widget_invalidate_layout((VenomWidget*)stack);
    }
}

VenomWidget* _venom_stack_build(const VenomStackConfig* config) {
    VenomResultPtr result = venom_stack_create();
    if (!result.ok) return NULL;
    
    VenomStack* stack = (VenomStack*)result.value;
    stack->alignment = config->alignment;
    
    if (config->children && config->child_count > 0) {
        for (VenomU32 i = 0; i < config->child_count; i++) {
            venom_stack_add_child(stack, config->children[i]);
        }
    }
    
    return (VenomWidget*)stack;
}
