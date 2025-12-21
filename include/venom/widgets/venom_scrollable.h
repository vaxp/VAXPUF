/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_scrollable.h - Scrollable container widget
 * 
 * Professional implementation with:
 * - Smooth scrolling with momentum
 * - Visible scrollbars with proper styling
 * - Mouse wheel and drag support
 * - Keyboard navigation (Page Up/Down, Arrows)
 * - Proper clipping of overflow content
 */

#ifndef VENOM_SCROLLABLE_H
#define VENOM_SCROLLABLE_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * SCROLL DIRECTION
 * ============================================================================ */

typedef enum VenomScrollDirection {
    VENOM_SCROLL_VERTICAL   = (1 << 0),
    VENOM_SCROLL_HORIZONTAL = (1 << 1),
    VENOM_SCROLL_BOTH       = VENOM_SCROLL_VERTICAL | VENOM_SCROLL_HORIZONTAL,
} VenomScrollDirection;

/* ============================================================================
 * SCROLLBAR VISIBILITY
 * ============================================================================ */

typedef enum VenomScrollbarVisibility {
    VENOM_SCROLLBAR_AUTO,       /* Show when content overflows */
    VENOM_SCROLLBAR_ALWAYS,     /* Always show */
    VENOM_SCROLLBAR_NEVER,      /* Never show (still scrollable) */
} VenomScrollbarVisibility;

/* ============================================================================
 * SCROLLABLE WIDGET
 * ============================================================================ */

typedef struct VenomScrollable {
    VenomWidget base;
    
    /* Content */
    VenomWidget* content;           /* Single child that can be larger than viewport */
    
    /* Scroll state */
    VenomF32 scroll_x;              /* Current horizontal scroll position */
    VenomF32 scroll_y;              /* Current vertical scroll position */
    VenomF32 max_scroll_x;          /* Maximum horizontal scroll */
    VenomF32 max_scroll_y;          /* Maximum vertical scroll */
    
    /* Content size */
    VenomF32 content_width;         /* Measured content width */
    VenomF32 content_height;        /* Measured content height */
    
    /* Configuration */
    VenomScrollDirection direction;
    VenomScrollbarVisibility scrollbar_visibility;
    VenomF32 scrollbar_width;       /* Width of scrollbar track */
    VenomF32 scroll_step;           /* Pixels per scroll wheel notch */
    
    /* Scrollbar colors */
    VenomColor scrollbar_track_color;
    VenomColor scrollbar_thumb_color;
    VenomColor scrollbar_thumb_hover_color;
    
    /* Smooth scrolling */
    VenomF32 target_scroll_x;       /* Target for smooth scroll */
    VenomF32 target_scroll_y;
    VenomF32 scroll_velocity_x;     /* For momentum scrolling */
    VenomF32 scroll_velocity_y;
    VenomBool smooth_scrolling;     /* Enable smooth interpolation */
    
    /* Drag scrolling */
    VenomBool is_dragging;
    VenomI32 drag_start_x;
    VenomI32 drag_start_y;
    VenomF32 drag_start_scroll_x;
    VenomF32 drag_start_scroll_y;
    
    /* Scrollbar hover state */
    VenomBool vertical_thumb_hovered;
    VenomBool horizontal_thumb_hovered;
    
} VenomScrollable;

/* ============================================================================
 * SCROLLABLE API
 * ============================================================================ */

/**
 * @brief Create a scrollable container
 */
VenomResultPtr venom_scrollable_create(void);

/**
 * @brief Set the scrollable content (single child)
 */
VenomResult venom_scrollable_set_content(VenomScrollable* scroll, VenomWidget* content);

/**
 * @brief Get current scroll position
 */
void venom_scrollable_get_scroll(const VenomScrollable* scroll, VenomF32* x, VenomF32* y);

/**
 * @brief Set scroll position
 */
void venom_scrollable_set_scroll(VenomScrollable* scroll, VenomF32 x, VenomF32 y);

/**
 * @brief Scroll by delta amount
 */
void venom_scrollable_scroll_by(VenomScrollable* scroll, VenomF32 dx, VenomF32 dy);

/**
 * @brief Scroll to make a child widget visible
 */
void venom_scrollable_ensure_visible(VenomScrollable* scroll, VenomWidget* widget);

/**
 * @brief Set scroll direction
 */
void venom_scrollable_set_direction(VenomScrollable* scroll, VenomScrollDirection direction);

/**
 * @brief Set scrollbar visibility
 */
void venom_scrollable_set_scrollbar_visibility(VenomScrollable* scroll, VenomScrollbarVisibility visibility);

/**
 * @brief Enable/disable smooth scrolling
 */
void venom_scrollable_set_smooth(VenomScrollable* scroll, VenomBool smooth);

/**
 * @brief Set scrollbar colors
 */
void venom_scrollable_set_scrollbar_colors(VenomScrollable* scroll, 
                                            VenomColor track, 
                                            VenomColor thumb);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VenomWidgetClass venom_scrollable_class;

/* ============================================================================
 * CONVENIENCE MACRO
 * ============================================================================ */

/**
 * @brief Create scrollable with configuration
 */
#define venom_scroll(...) \
    _venom_scrollable_build(&(VenomScrollableConfig){ __VA_ARGS__ })

typedef struct VenomScrollableConfig {
    VenomWidget* content;
    VenomScrollDirection direction;
    VenomScrollbarVisibility scrollbar;
    VenomBool smooth;
    VenomF32 width;
    VenomF32 height;
} VenomScrollableConfig;

VenomWidget* _venom_scrollable_build(const VenomScrollableConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_SCROLLABLE_H */
