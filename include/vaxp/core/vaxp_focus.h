/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_focus.h - Input focus management system
 * 
 * Manages keyboard focus for widgets. Only one widget can have
 * focus at a time. Supports Tab navigation between focusable widgets.
 */

#ifndef VAXP_FOCUS_H
#define VAXP_FOCUS_H

#include "vaxp/core/vaxp_types.h"
#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * FOCUS MANAGEMENT API
 * ============================================================================ */

/**
 * @brief Initialize focus system (called by vaxp_init)
 */
void vaxp_focus_init(void);

/**
 * @brief Shutdown focus system
 */
void vaxp_focus_shutdown(void);

/**
 * @brief Set focus to a specific widget
 * 
 * The previous focused widget will lose focus.
 * If widget is NULL, clears focus.
 */
void vaxp_focus_set(VaxpWidget* widget);

/**
 * @brief Get currently focused widget
 * 
 * @return Currently focused widget, or NULL if none
 */
VaxpWidget* vaxp_focus_get(void);

/**
 * @brief Move focus to next focusable widget
 * 
 * Uses Tab order (tree traversal order).
 */
void vaxp_focus_next(void);

/**
 * @brief Move focus to previous focusable widget
 * 
 * Uses reverse Tab order (Shift+Tab).
 */
void vaxp_focus_prev(void);

/**
 * @brief Clear focus (no widget focused)
 */
void vaxp_focus_clear(void);

/**
 * @brief Check if a widget has focus
 */
VaxpBool vaxp_focus_has(const VaxpWidget* widget);

/**
 * @brief Set the root widget for focus traversal
 * 
 * Called when window root changes.
 */
void vaxp_focus_set_root(VaxpWidget* root);

/**
 * @brief Get all focusable widgets in order
 * 
 * @param widgets Output array for widgets
 * @param max_count Maximum number to return
 * @return Number of focusable widgets found
 */
VaxpU32 vaxp_focus_get_focusable_list(VaxpWidget** widgets, VaxpU32 max_count);

/* ============================================================================
 * WIDGET FOCUS HELPERS
 * ============================================================================ */

/**
 * @brief Mark a widget as focusable
 */
void vaxp_widget_set_focusable(VaxpWidget* widget, VaxpBool focusable);

/**
 * @brief Check if widget is focusable
 */
VaxpBool vaxp_widget_is_focusable(const VaxpWidget* widget);

/**
 * @brief Request focus for a widget
 */
VaxpBool vaxp_widget_request_focus(VaxpWidget* widget);

/**
 * @brief Check if widget has focus
 */
VaxpBool vaxp_widget_has_focus(const VaxpWidget* widget);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_FOCUS_H */
