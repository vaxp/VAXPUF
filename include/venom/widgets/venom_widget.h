/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_widget.h - Base widget with inheritance via vtable
 */

#ifndef VENOM_WIDGET_H
#define VENOM_WIDGET_H

#include "venom/core/venom_types.h"
#include "venom/core/venom_result.h"
#include "venom/core/venom_ref.h"
#include "venom/graphics/venom_canvas.h"
#include "venom/backend/venom_event.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
typedef struct VenomWidget VenomWidget;
typedef struct VenomWidgetClass VenomWidgetClass;

/* ============================================================================
 * WIDGET STATE FLAGS
 * ============================================================================ */

typedef enum VenomWidgetState {
    VENOM_WIDGET_STATE_NORMAL   = 0,
    VENOM_WIDGET_STATE_HOVERED  = (1 << 0),
    VENOM_WIDGET_STATE_PRESSED  = (1 << 1),
    VENOM_WIDGET_STATE_FOCUSED  = (1 << 2),
    VENOM_WIDGET_STATE_DISABLED = (1 << 3),
    VENOM_WIDGET_STATE_SELECTED = (1 << 4),
} VenomWidgetState;

/* ============================================================================
 * LAYOUT PROPERTIES
 * ============================================================================ */

typedef enum VenomAlign {
    VENOM_ALIGN_START,
    VENOM_ALIGN_CENTER,
    VENOM_ALIGN_END,
    VENOM_ALIGN_STRETCH,
} VenomAlign;

typedef enum VenomFlexDirection {
    VENOM_FLEX_ROW,
    VENOM_FLEX_COLUMN,
    VENOM_FLEX_ROW_REVERSE,
    VENOM_FLEX_COLUMN_REVERSE,
} VenomFlexDirection;

typedef struct VenomLayoutProps {
    /* Size constraints */
    VenomF32 min_width;
    VenomF32 min_height;
    VenomF32 max_width;
    VenomF32 max_height;
    VenomF32 preferred_width;   /* 0 = auto */
    VenomF32 preferred_height;  /* 0 = auto */
    
    /* Padding (inside) */
    VenomInsets padding;
    
    /* Margin (outside) */
    VenomInsets margin;
    
    /* Flex layout */
    VenomF32 flex_grow;
    VenomF32 flex_shrink;
    VenomAlign align_self;
    
} VenomLayoutProps;

/* ============================================================================
 * WIDGET CLASS (VTABLE)
 * ============================================================================ */

/**
 * @brief Virtual method table for widgets
 * 
 * Each widget type has one static instance of this.
 * Inheritance is achieved by copying parent vtable and overriding methods.
 */
struct VenomWidgetClass {
    const char* class_name;
    VenomSize   instance_size;
    const VenomWidgetClass* parent_class;  /* For chain of responsibility */
    
    /* Lifecycle */
    void (*init)(VenomWidget* self);
    void (*destroy)(VenomWidget* self);
    
    /* Layout */
    void (*measure)(VenomWidget* self, VenomF32 available_width, VenomF32 available_height,
                    VenomF32* out_width, VenomF32* out_height);
    void (*layout)(VenomWidget* self, VenomRectF bounds);
    
    /* Rendering */
    void (*draw)(VenomWidget* self, VenomCanvas* canvas);
    
    /* Events (return true if handled) */
    VenomBool (*on_event)(VenomWidget* self, const VenomEvent* event);
    
    /* State changes */
    void (*on_state_changed)(VenomWidget* self, VenomWidgetState old_state);
};

/* ============================================================================
 * BASE WIDGET STRUCTURE
 * ============================================================================ */

struct VenomWidget {
    VENOM_REF_HEADER;
    
    const VenomWidgetClass* klass;  /* Widget class (vtable) */
    
    /* Hierarchy */
    VenomWidget* VENOM_BORROWED parent;
    VenomWidget** VENOM_OWNED children;
    VenomU32 children_count;
    VenomU32 children_capacity;
    
    /* Geometry */
    VenomRectF bounds;      /* Position relative to parent */
    VenomRectF content_rect; /* Bounds minus padding */
    
    /* State */
    VenomWidgetState state;
    VenomBool visible;
    VenomBool needs_layout;
    VenomBool needs_redraw;
    
    /* Layout */
    VenomLayoutProps layout;
    
    /* Focus */
    VenomBool focusable;    /* Can receive keyboard focus */
    
    /* User data */
    void* user_data;
};

/* ============================================================================
 * WIDGET LIFECYCLE
 * ============================================================================ */

/**
 * @brief Create a widget of the given class
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_widget_create(const VenomWidgetClass* klass);

/**
 * @brief Initialize base widget (call from subclass init)
 */
void venom_widget_init_base(VenomWidget* widget, const VenomWidgetClass* klass);

/**
 * @brief Destroy widget (decrements ref count)
 */
VENOM_INLINE void venom_widget_destroy(VenomWidget* widget) {
    venom_unref(widget);
}

/* ============================================================================
 * HIERARCHY
 * ============================================================================ */

/**
 * @brief Add a child widget
 * 
 * Takes ownership of child (refs it).
 */
VenomResult venom_widget_add_child(VenomWidget* parent, VenomWidget* child);

/**
 * @brief Remove a child widget
 * 
 * Releases ownership (unrefs it).
 */
VenomResult venom_widget_remove_child(VenomWidget* parent, VenomWidget* child);

/**
 * @brief Remove all children
 */
void venom_widget_remove_all_children(VenomWidget* parent);

/**
 * @brief Get child count
 */
VENOM_INLINE VenomU32 venom_widget_child_count(const VenomWidget* widget) {
    return widget ? widget->children_count : 0;
}

/**
 * @brief Get child at index
 */
VENOM_INLINE VenomWidget* venom_widget_child_at(const VenomWidget* widget, VenomU32 index) {
    if (!widget || index >= widget->children_count) return NULL;
    return widget->children[index];
}

/**
 * @brief Get parent widget
 */
VENOM_INLINE VenomWidget* venom_widget_parent(const VenomWidget* widget) {
    return widget ? widget->parent : NULL;
}

/* ============================================================================
 * LAYOUT & GEOMETRY
 * ============================================================================ */

/**
 * @brief Request re-layout
 */
void venom_widget_invalidate_layout(VenomWidget* widget);

/**
 * @brief Request redraw
 */
void venom_widget_invalidate(VenomWidget* widget);

/**
 * @brief Measure widget
 */
void venom_widget_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                          VenomF32* out_width, VenomF32* out_height);

/**
 * @brief Layout widget and children
 */
void venom_widget_layout(VenomWidget* widget, VenomRectF bounds);

/**
 * @brief Set position and size
 */
void venom_widget_set_bounds(VenomWidget* widget, VenomRectF bounds);

/**
 * @brief Get widget bounds
 */
VENOM_INLINE VenomRectF venom_widget_get_bounds(const VenomWidget* widget) {
    return widget ? widget->bounds : (VenomRectF){0};
}

/**
 * @brief Convert point from widget coords to screen coords
 */
VenomPointF venom_widget_to_screen(const VenomWidget* widget, VenomF32 x, VenomF32 y);

/**
 * @brief Convert point from screen coords to widget coords
 */
VenomPointF venom_widget_from_screen(const VenomWidget* widget, VenomF32 x, VenomF32 y);

/* ============================================================================
 * RENDERING
 * ============================================================================ */

/**
 * @brief Draw widget and children
 */
void venom_widget_draw(VenomWidget* widget, VenomCanvas* canvas);

/* ============================================================================
 * EVENT HANDLING
 * ============================================================================ */

/**
 * @brief Dispatch event to widget
 * 
 * @return true if event was handled
 */
VenomBool venom_widget_dispatch_event(VenomWidget* widget, const VenomEvent* event);

/**
 * @brief Find widget at position
 */
VenomWidget* venom_widget_hit_test(VenomWidget* widget, VenomF32 x, VenomF32 y);

/* ============================================================================
 * STATE
 * ============================================================================ */

/**
 * @brief Set widget state flags
 */
void venom_widget_set_state(VenomWidget* widget, VenomWidgetState state);

/**
 * @brief Add state flag
 */
void venom_widget_add_state(VenomWidget* widget, VenomWidgetState flag);

/**
 * @brief Remove state flag
 */
void venom_widget_remove_state(VenomWidget* widget, VenomWidgetState flag);

/**
 * @brief Check if widget has state flag
 */
VENOM_INLINE VenomBool venom_widget_has_state(const VenomWidget* widget, VenomWidgetState flag) {
    return widget && (widget->state & flag) != 0;
}

/**
 * @brief Check if widget is enabled
 */
VENOM_INLINE VenomBool venom_widget_is_enabled(const VenomWidget* widget) {
    return !venom_widget_has_state(widget, VENOM_WIDGET_STATE_DISABLED);
}

/**
 * @brief Set enabled/disabled
 */
void venom_widget_set_enabled(VenomWidget* widget, VenomBool enabled);

/**
 * @brief Set visibility
 */
void venom_widget_set_visible(VenomWidget* widget, VenomBool visible);

/**
 * @brief Check if visible
 */
VENOM_INLINE VenomBool venom_widget_is_visible(const VenomWidget* widget) {
    return widget && widget->visible;
}

/* ============================================================================
 * BASE WIDGET CLASS
 * ============================================================================ */

extern const VenomWidgetClass venom_widget_class;

#ifdef __cplusplus
}
#endif

#endif /* VENOM_WIDGET_H */
