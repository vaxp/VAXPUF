/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_widget.h - Base widget with inheritance via vtable
 */

#ifndef VAXP_WIDGET_H
#define VAXP_WIDGET_H

#include "vaxp/core/vaxp_types.h"
#include "vaxp/core/vaxp_result.h"
#include "vaxp/core/vaxp_ref.h"
#include "vaxp/graphics/vaxp_canvas.h"
#include "vaxp/backend/vaxp_event.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
typedef struct VaxpWidget VaxpWidget;
typedef struct VaxpWidgetClass VaxpWidgetClass;

/* ============================================================================
 * WIDGET STATE FLAGS
 * ============================================================================ */

typedef enum VaxpWidgetState {
    VAXP_WIDGET_STATE_NORMAL   = 0,
    VAXP_WIDGET_STATE_HOVERED  = (1 << 0),
    VAXP_WIDGET_STATE_PRESSED  = (1 << 1),
    VAXP_WIDGET_STATE_FOCUSED  = (1 << 2),
    VAXP_WIDGET_STATE_DISABLED = (1 << 3),
    VAXP_WIDGET_STATE_SELECTED = (1 << 4),
} VaxpWidgetState;

/* ============================================================================
 * LAYOUT PROPERTIES
 * ============================================================================ */

typedef enum VaxpAlign {
    VAXP_ALIGN_START,
    VAXP_ALIGN_CENTER,
    VAXP_ALIGN_END,
    VAXP_ALIGN_STRETCH,
} VaxpAlign;

typedef enum VaxpFlexDirection {
    VAXP_FLEX_ROW,
    VAXP_FLEX_COLUMN,
    VAXP_FLEX_ROW_REVERSE,
    VAXP_FLEX_COLUMN_REVERSE,
} VaxpFlexDirection;

typedef struct VaxpLayoutProps {
    /* Size constraints */
    VaxpF32 min_width;
    VaxpF32 min_height;
    VaxpF32 max_width;
    VaxpF32 max_height;
    VaxpF32 preferred_width;   /* 0 = auto */
    VaxpF32 preferred_height;  /* 0 = auto */
    
    /* Padding (inside) */
    VaxpInsets padding;
    
    /* Margin (outside) */
    VaxpInsets margin;
    
    /* Flex layout */
    VaxpF32 flex_grow;
    VaxpF32 flex_shrink;
    VaxpAlign align_self;
    
} VaxpLayoutProps;

/* ============================================================================
 * WIDGET CLASS (VTABLE)
 * ============================================================================ */

/**
 * @brief Virtual method table for widgets
 * 
 * Each widget type has one static instance of this.
 * Inheritance is achieved by copying parent vtable and overriding methods.
 */
struct VaxpWidgetClass {
    const char* class_name;
    VaxpSize   instance_size;
    const VaxpWidgetClass* parent_class;  /* For chain of responsibility */
    
    /* Lifecycle */
    void (*init)(VaxpWidget* self);
    void (*destroy)(VaxpWidget* self);
    
    /* Layout */
    void (*measure)(VaxpWidget* self, VaxpF32 available_width, VaxpF32 available_height,
                    VaxpF32* out_width, VaxpF32* out_height);
    void (*layout)(VaxpWidget* self, VaxpRectF bounds);
    
    /* Rendering */
    void (*draw)(VaxpWidget* self, VaxpCanvas* canvas);
    
    /* Events (return true if handled) */
    VaxpBool (*on_event)(VaxpWidget* self, const VaxpEvent* event);
    
    /* State changes */
    void (*on_state_changed)(VaxpWidget* self, VaxpWidgetState old_state);
};

/* ============================================================================
 * BASE WIDGET STRUCTURE
 * ============================================================================ */

struct VaxpWidget {
    VAXP_REF_HEADER;
    
    const VaxpWidgetClass* klass;  /* Widget class (vtable) */
    
    /* Hierarchy */
    VaxpWidget* VAXP_BORROWED parent;
    VaxpWidget** VAXP_OWNED children;
    VaxpU32 children_count;
    VaxpU32 children_capacity;
    
    /* Geometry */
    VaxpRectF bounds;      /* Position relative to parent */
    VaxpRectF content_rect; /* Bounds minus padding */
    
    /* State */
    VaxpWidgetState state;
    VaxpBool visible;
    VaxpBool needs_layout;
    VaxpBool needs_redraw;
    
    /* Layout */
    VaxpLayoutProps layout;
    
    /* Focus */
    VaxpBool focusable;    /* Can receive keyboard focus */
    
    /* Const Widget System */
    VaxpBool is_const;         /* Widget won't be rebuilt */
    const char* const_key;      /* Unique key for const lookup */
    
    /* User data */
    void* user_data;
};

/* ============================================================================
 * WIDGET LIFECYCLE
 * ============================================================================ */

/**
 * @brief Create a widget of the given class
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_widget_create(const VaxpWidgetClass* klass);

/**
 * @brief Initialize base widget (call from subclass init)
 */
void vaxp_widget_init_base(VaxpWidget* widget, const VaxpWidgetClass* klass);

/**
 * @brief Destroy widget (decrements ref count)
 */
VAXP_INLINE void vaxp_widget_destroy(VaxpWidget* widget) {
    vaxp_unref(widget);
}

/* ============================================================================
 * HIERARCHY
 * ============================================================================ */

/**
 * @brief Add a child widget
 * 
 * Takes ownership of child (refs it).
 */
VaxpResult vaxp_widget_add_child(VaxpWidget* parent, VaxpWidget* child);

/**
 * @brief Remove a child widget
 * 
 * Releases ownership (unrefs it).
 */
VaxpResult vaxp_widget_remove_child(VaxpWidget* parent, VaxpWidget* child);

/**
 * @brief Remove all children
 */
void vaxp_widget_remove_all_children(VaxpWidget* parent);

/**
 * @brief Get child count
 */
VAXP_INLINE VaxpU32 vaxp_widget_child_count(const VaxpWidget* widget) {
    return widget ? widget->children_count : 0;
}

/**
 * @brief Get child at index
 */
VAXP_INLINE VaxpWidget* vaxp_widget_child_at(const VaxpWidget* widget, VaxpU32 index) {
    if (!widget || index >= widget->children_count) return NULL;
    return widget->children[index];
}

/**
 * @brief Get parent widget
 */
VAXP_INLINE VaxpWidget* vaxp_widget_parent(const VaxpWidget* widget) {
    return widget ? widget->parent : NULL;
}

/* ============================================================================
 * LAYOUT & GEOMETRY
 * ============================================================================ */

/**
 * @brief Request re-layout
 */
void vaxp_widget_invalidate_layout(VaxpWidget* widget);

/**
 * @brief Request redraw
 */
void vaxp_widget_invalidate(VaxpWidget* widget);

/**
 * @brief Measure widget
 */
void vaxp_widget_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                          VaxpF32* out_width, VaxpF32* out_height);

/**
 * @brief Layout widget and children
 */
void vaxp_widget_layout(VaxpWidget* widget, VaxpRectF bounds);

/**
 * @brief Set position and size
 */
void vaxp_widget_set_bounds(VaxpWidget* widget, VaxpRectF bounds);

/**
 * @brief Get widget bounds
 */
VAXP_INLINE VaxpRectF vaxp_widget_get_bounds(const VaxpWidget* widget) {
    return widget ? widget->bounds : (VaxpRectF){0};
}

/**
 * @brief Convert point from widget coords to screen coords
 */
VaxpPointF vaxp_widget_to_screen(const VaxpWidget* widget, VaxpF32 x, VaxpF32 y);

/**
 * @brief Convert point from screen coords to widget coords
 */
VaxpPointF vaxp_widget_from_screen(const VaxpWidget* widget, VaxpF32 x, VaxpF32 y);

/* ============================================================================
 * RENDERING
 * ============================================================================ */

/**
 * @brief Draw widget and children
 */
void vaxp_widget_draw(VaxpWidget* widget, VaxpCanvas* canvas);

/* ============================================================================
 * EVENT HANDLING
 * ============================================================================ */

/**
 * @brief Dispatch event to widget
 * 
 * @return true if event was handled
 */
VaxpBool vaxp_widget_dispatch_event(VaxpWidget* widget, const VaxpEvent* event);

/**
 * @brief Find widget at position
 */
VaxpWidget* vaxp_widget_hit_test(VaxpWidget* widget, VaxpF32 x, VaxpF32 y);

/* ============================================================================
 * STATE
 * ============================================================================ */

/**
 * @brief Set widget state flags
 */
void vaxp_widget_set_state(VaxpWidget* widget, VaxpWidgetState state);

/**
 * @brief Add state flag
 */
void vaxp_widget_add_state(VaxpWidget* widget, VaxpWidgetState flag);

/**
 * @brief Remove state flag
 */
void vaxp_widget_remove_state(VaxpWidget* widget, VaxpWidgetState flag);

/**
 * @brief Check if widget has state flag
 */
VAXP_INLINE VaxpBool vaxp_widget_has_state(const VaxpWidget* widget, VaxpWidgetState flag) {
    return widget && (widget->state & flag) != 0;
}

/**
 * @brief Check if widget is enabled
 */
VAXP_INLINE VaxpBool vaxp_widget_is_enabled(const VaxpWidget* widget) {
    return !vaxp_widget_has_state(widget, VAXP_WIDGET_STATE_DISABLED);
}

/**
 * @brief Set enabled/disabled
 */
void vaxp_widget_set_enabled(VaxpWidget* widget, VaxpBool enabled);

/**
 * @brief Set visibility
 */
void vaxp_widget_set_visible(VaxpWidget* widget, VaxpBool visible);

/**
 * @brief Check if visible
 */
VAXP_INLINE VaxpBool vaxp_widget_is_visible(const VaxpWidget* widget) {
    return widget && widget->visible;
}

/* ============================================================================
 * CONST WIDGETS
 * ============================================================================ */

/**
 * @brief Mark widget as const (won't be rebuilt during vaxp_rebuild)
 */
void vaxp_widget_set_const(VaxpWidget* widget, VaxpBool is_const);

/**
 * @brief Set const key for lookup during rebuild
 */
void vaxp_widget_set_const_key(VaxpWidget* widget, const char* key);

/**
 * @brief Check if widget is const
 */
VAXP_INLINE VaxpBool vaxp_widget_is_const(const VaxpWidget* widget) {
    return widget && widget->is_const;
}

/**
 * @brief Get const key
 */
VAXP_INLINE const char* vaxp_widget_get_const_key(const VaxpWidget* widget) {
    return widget ? widget->const_key : NULL;
}

/* ============================================================================
 * BASE WIDGET CLASS
 * ============================================================================ */

extern const VaxpWidgetClass vaxp_widget_class;

#ifdef __cplusplus
}
#endif

#endif /* VAXP_WIDGET_H */
