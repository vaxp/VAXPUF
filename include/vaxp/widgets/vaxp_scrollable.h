/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_scrollable.h - Scrollable container widget
 * 
 * Professional implementation with:
 * - Smooth scrolling with momentum
 * - Visible scrollbars with proper styling
 * - Mouse wheel and drag support
 * - Keyboard navigation (Page Up/Down, Arrows)
 * - Proper clipping of overflow content
 */

#ifndef VAXP_SCROLLABLE_H
#define VAXP_SCROLLABLE_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * SCROLL DIRECTION
 * ============================================================================ */

typedef enum VaxpScrollDirection {
    VAXP_SCROLL_VERTICAL   = (1 << 0),
    VAXP_SCROLL_HORIZONTAL = (1 << 1),
    VAXP_SCROLL_BOTH       = VAXP_SCROLL_VERTICAL | VAXP_SCROLL_HORIZONTAL,
} VaxpScrollDirection;

/* ============================================================================
 * SCROLLBAR VISIBILITY
 * ============================================================================ */

typedef enum VaxpScrollbarVisibility {
    VAXP_SCROLLBAR_AUTO,       /* Show when content overflows */
    VAXP_SCROLLBAR_ALWAYS,     /* Always show */
    VAXP_SCROLLBAR_NEVER,      /* Never show (still scrollable) */
} VaxpScrollbarVisibility;

/* ============================================================================
 * SCROLLABLE WIDGET
 * ============================================================================ */

typedef struct VaxpScrollable {
    VaxpWidget base;
    
    /* Content */
    VaxpWidget* content;           /* Single child that can be larger than viewport */
    
    /* Scroll state */
    VaxpF32 scroll_x;              /* Current horizontal scroll position */
    VaxpF32 scroll_y;              /* Current vertical scroll position */
    VaxpF32 max_scroll_x;          /* Maximum horizontal scroll */
    VaxpF32 max_scroll_y;          /* Maximum vertical scroll */
    
    /* Content size */
    VaxpF32 content_width;         /* Measured content width */
    VaxpF32 content_height;        /* Measured content height */
    
    /* Configuration */
    VaxpScrollDirection direction;
    VaxpScrollbarVisibility scrollbar_visibility;
    VaxpF32 scrollbar_width;       /* Width of scrollbar track */
    VaxpF32 scroll_step;           /* Pixels per scroll wheel notch */
    
    /* Scrollbar colors */
    VaxpColor scrollbar_track_color;
    VaxpColor scrollbar_thumb_color;
    VaxpColor scrollbar_thumb_hover_color;
    
    /* Smooth scrolling */
    VaxpF32 target_scroll_x;       /* Target for smooth scroll */
    VaxpF32 target_scroll_y;
    VaxpF32 scroll_velocity_x;     /* For momentum scrolling */
    VaxpF32 scroll_velocity_y;
    VaxpBool smooth_scrolling;     /* Enable smooth interpolation */
    
    /* Drag scrolling */
    VaxpBool is_dragging;
    VaxpI32 drag_start_x;
    VaxpI32 drag_start_y;
    VaxpF32 drag_start_scroll_x;
    VaxpF32 drag_start_scroll_y;
    
    /* Scrollbar hover state */
    VaxpBool vertical_thumb_hovered;
    VaxpBool horizontal_thumb_hovered;
    
} VaxpScrollable;

/* ============================================================================
 * SCROLLABLE API
 * ============================================================================ */

/**
 * @brief Create a scrollable container
 */
VaxpResultPtr vaxp_scrollable_create(void);

/**
 * @brief Set the scrollable content (single child)
 */
VaxpResult vaxp_scrollable_set_content(VaxpScrollable* scroll, VaxpWidget* content);

/**
 * @brief Get current scroll position
 */
void vaxp_scrollable_get_scroll(const VaxpScrollable* scroll, VaxpF32* x, VaxpF32* y);

/**
 * @brief Set scroll position
 */
void vaxp_scrollable_set_scroll(VaxpScrollable* scroll, VaxpF32 x, VaxpF32 y);

/**
 * @brief Scroll by delta amount
 */
void vaxp_scrollable_scroll_by(VaxpScrollable* scroll, VaxpF32 dx, VaxpF32 dy);

/**
 * @brief Scroll to make a child widget visible
 */
void vaxp_scrollable_ensure_visible(VaxpScrollable* scroll, VaxpWidget* widget);

/**
 * @brief Set scroll direction
 */
void vaxp_scrollable_set_direction(VaxpScrollable* scroll, VaxpScrollDirection direction);

/**
 * @brief Set scrollbar visibility
 */
void vaxp_scrollable_set_scrollbar_visibility(VaxpScrollable* scroll, VaxpScrollbarVisibility visibility);

/**
 * @brief Enable/disable smooth scrolling
 */
void vaxp_scrollable_set_smooth(VaxpScrollable* scroll, VaxpBool smooth);

/**
 * @brief Set scrollbar colors
 */
void vaxp_scrollable_set_scrollbar_colors(VaxpScrollable* scroll, 
                                            VaxpColor track, 
                                            VaxpColor thumb);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VaxpWidgetClass vaxp_scrollable_class;

/* ============================================================================
 * CONVENIENCE MACRO
 * ============================================================================ */

/**
 * @brief Create scrollable with configuration
 */
#define vaxp_scroll(...) \
    _vaxp_scrollable_build(&(VaxpScrollableConfig){ __VA_ARGS__ })

typedef struct VaxpScrollableConfig {
    VaxpWidget* content;
    VaxpScrollDirection direction;
    VaxpScrollbarVisibility scrollbar;
    VaxpBool smooth;
    VaxpF32 width;
    VaxpF32 height;
} VaxpScrollableConfig;

VaxpWidget* _vaxp_scrollable_build(const VaxpScrollableConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_SCROLLABLE_H */
