/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_stack.c - Stack widget implementation
 */

#include "vaxp/widgets/vaxp_stack.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

#define INITIAL_CHILD_CAPACITY 4

/* ============================================================================
 * STACK CLASS METHODS
 * ============================================================================ */

static void stack_init(VaxpWidget* widget) {
    VaxpStack* stack = (VaxpStack*)widget;
    
    stack->children = NULL;
    stack->child_count = 0;
    stack->child_capacity = 0;
    stack->alignment = VAXP_STACK_CENTER;
}

static void stack_destroy(VaxpWidget* widget) {
    VaxpStack* stack = (VaxpStack*)widget;
    
    /* Unref all children */
    for (VaxpU32 i = 0; i < stack->child_count; i++) {
        if (stack->children[i]) {
            stack->children[i]->parent = NULL;
            vaxp_unref(stack->children[i]);
        }
    }
    
    if (stack->children) {
        vaxp_free(stack->children, stack->child_capacity * sizeof(VaxpWidget*));
        stack->children = NULL;
    }
    
    vaxp_widget_class.destroy(widget);
}

static void stack_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                          VaxpF32* out_width, VaxpF32* out_height) {
    VaxpStack* stack = (VaxpStack*)widget;
    
    VaxpF32 max_width = 0;
    VaxpF32 max_height = 0;
    
    /* Stack takes the size of its largest child */
    for (VaxpU32 i = 0; i < stack->child_count; i++) {
        VaxpWidget* child = stack->children[i];
        if (!child || !child->visible) continue;
        
        VaxpF32 child_w, child_h;
        vaxp_widget_measure(child, available_width, available_height, &child_w, &child_h);
        
        if (child_w > max_width) max_width = child_w;
        if (child_h > max_height) max_height = child_h;
    }
    
    *out_width = widget->layout.preferred_width > 0 ? widget->layout.preferred_width : max_width;
    *out_height = widget->layout.preferred_height > 0 ? widget->layout.preferred_height : max_height;
}

static void stack_layout(VaxpWidget* widget, VaxpRectF bounds) {
    VaxpStack* stack = (VaxpStack*)widget;
    widget->bounds = bounds;
    
    for (VaxpU32 i = 0; i < stack->child_count; i++) {
        VaxpWidget* child = stack->children[i];
        if (!child || !child->visible) continue;
        
        /* If child has pre-set bounds (width > 0 and height > 0), use them */
        if (child->bounds.width > 0 && child->bounds.height > 0) {
            /* Just call layout with existing bounds to let child initialize */
            vaxp_widget_layout(child, child->bounds);
            continue;
        }
        
        VaxpF32 child_w, child_h;
        vaxp_widget_measure(child, bounds.width, bounds.height, &child_w, &child_h);
        
        /* Use preferred size if set */
        if (child->layout.preferred_width > 0) child_w = child->layout.preferred_width;
        if (child->layout.preferred_height > 0) child_h = child->layout.preferred_height;
        
        /* Calculate position based on alignment */
        VaxpF32 x = 0, y = 0;
        
        switch (stack->alignment) {
            case VAXP_STACK_TOP_LEFT:
            case VAXP_STACK_CENTER_LEFT:
            case VAXP_STACK_BOTTOM_LEFT:
                x = 0;
                break;
            case VAXP_STACK_TOP_CENTER:
            case VAXP_STACK_CENTER:
            case VAXP_STACK_BOTTOM_CENTER:
                x = (bounds.width - child_w) / 2;
                break;
            case VAXP_STACK_TOP_RIGHT:
            case VAXP_STACK_CENTER_RIGHT:
            case VAXP_STACK_BOTTOM_RIGHT:
                x = bounds.width - child_w;
                break;
        }
        
        switch (stack->alignment) {
            case VAXP_STACK_TOP_LEFT:
            case VAXP_STACK_TOP_CENTER:
            case VAXP_STACK_TOP_RIGHT:
                y = 0;
                break;
            case VAXP_STACK_CENTER_LEFT:
            case VAXP_STACK_CENTER:
            case VAXP_STACK_CENTER_RIGHT:
                y = (bounds.height - child_h) / 2;
                break;
            case VAXP_STACK_BOTTOM_LEFT:
            case VAXP_STACK_BOTTOM_CENTER:
            case VAXP_STACK_BOTTOM_RIGHT:
                y = bounds.height - child_h;
                break;
        }
        
        /* Apply margins */
        x += child->layout.margin.left;
        y += child->layout.margin.top;
        
        VaxpRectF child_bounds = { x, y, child_w, child_h };
        vaxp_widget_layout(child, child_bounds);
    }
}

static void stack_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpStack* stack = (VaxpStack*)widget;
    
    printf("[Stack] Drawing %u children\n", stack->child_count);
    
    /* Draw children in order (first added = bottom, last = top) */
    for (VaxpU32 i = 0; i < stack->child_count; i++) {
        VaxpWidget* child = stack->children[i];
        if (child && child->visible) {
            printf("  [%u] %s at (%.0f, %.0f) size %.0fx%.0f\n", 
                   i, child->klass->class_name,
                   child->bounds.x, child->bounds.y,
                   child->bounds.width, child->bounds.height);
            vaxp_canvas_save(canvas);
            vaxp_canvas_translate(canvas, child->bounds.x, child->bounds.y);
            vaxp_widget_draw(child, canvas);
            vaxp_canvas_restore(canvas);
        }
    }
}

static VaxpBool stack_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpStack* stack = (VaxpStack*)widget;
    
    /* Dispatch to children in reverse order (top first) */
    for (VaxpI32 i = (VaxpI32)stack->child_count - 1; i >= 0; i--) {
        VaxpWidget* child = stack->children[i];
        if (child && child->visible) {
            if (vaxp_widget_dispatch_event(child, event)) {
                return VAXP_TRUE;
            }
        }
    }
    
    return VAXP_FALSE;
}

/* ============================================================================
 * STACK CLASS
 * ============================================================================ */

const VaxpWidgetClass vaxp_stack_class = {
    .class_name = "VaxpStack",
    .instance_size = sizeof(VaxpStack),
    .parent_class = &vaxp_widget_class,
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

VaxpResultPtr vaxp_stack_create(void) {
    return vaxp_widget_create(&vaxp_stack_class);
}

VaxpResult vaxp_stack_add_child(VaxpStack* stack, VaxpWidget* child) {
    VAXP_ENSURE_NOT_NULL(stack);
    VAXP_ENSURE_NOT_NULL(child);
    
    /* Expand capacity if needed */
    if (stack->child_count >= stack->child_capacity) {
        VaxpU32 new_cap = stack->child_capacity == 0 ? INITIAL_CHILD_CAPACITY : stack->child_capacity * 2;
        VaxpWidget** new_array = (VaxpWidget**)vaxp_alloc(new_cap * sizeof(VaxpWidget*));
        if (!new_array) return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_MEMORY);
        
        if (stack->children) {
            memcpy(new_array, stack->children, stack->child_count * sizeof(VaxpWidget*));
            vaxp_free(stack->children, stack->child_capacity * sizeof(VaxpWidget*));
        }
        
        stack->children = new_array;
        stack->child_capacity = new_cap;
    }
    
    vaxp_ref(child);
    child->parent = (VaxpWidget*)stack;
    stack->children[stack->child_count++] = child;
    
    vaxp_widget_invalidate_layout((VaxpWidget*)stack);
    return VAXP_OK_UNIT();
}

VaxpResult vaxp_stack_remove_child(VaxpStack* stack, VaxpWidget* child) {
    VAXP_ENSURE_NOT_NULL(stack);
    VAXP_ENSURE_NOT_NULL(child);
    
    for (VaxpU32 i = 0; i < stack->child_count; i++) {
        if (stack->children[i] == child) {
            child->parent = NULL;
            vaxp_unref(child);
            
            /* Shift remaining children */
            memmove(&stack->children[i], &stack->children[i + 1], 
                    (stack->child_count - i - 1) * sizeof(VaxpWidget*));
            stack->child_count--;
            
            vaxp_widget_invalidate_layout((VaxpWidget*)stack);
            return VAXP_OK_UNIT();
        }
    }
    
    return VAXP_ERR_UNIT(VAXP_ERROR_NOT_FOUND);
}

void vaxp_stack_set_alignment(VaxpStack* stack, VaxpStackAlignment alignment) {
    if (stack) {
        stack->alignment = alignment;
        vaxp_widget_invalidate_layout((VaxpWidget*)stack);
    }
}

VaxpWidget* _vaxp_stack_build(const VaxpStackConfig* config) {
    VaxpResultPtr result = vaxp_stack_create();
    if (!result.ok) return NULL;
    
    VaxpStack* stack = (VaxpStack*)result.value;
    stack->alignment = config->alignment;
    
    if (config->children && config->child_count > 0) {
        for (VaxpU32 i = 0; i < config->child_count; i++) {
            vaxp_stack_add_child(stack, config->children[i]);
        }
    }
    
    return (VaxpWidget*)stack;
}
