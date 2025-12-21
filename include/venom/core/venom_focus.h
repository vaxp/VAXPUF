/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_focus.h - Input focus management system
 * 
 * Manages keyboard focus for widgets. Only one widget can have
 * focus at a time. Supports Tab navigation between focusable widgets.
 */

#ifndef VENOM_FOCUS_H
#define VENOM_FOCUS_H

#include "venom/core/venom_types.h"
#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * FOCUS MANAGEMENT API
 * ============================================================================ */

/**
 * @brief Initialize focus system (called by venom_init)
 */
void venom_focus_init(void);

/**
 * @brief Shutdown focus system
 */
void venom_focus_shutdown(void);

/**
 * @brief Set focus to a specific widget
 * 
 * The previous focused widget will lose focus.
 * If widget is NULL, clears focus.
 */
void venom_focus_set(VenomWidget* widget);

/**
 * @brief Get currently focused widget
 * 
 * @return Currently focused widget, or NULL if none
 */
VenomWidget* venom_focus_get(void);

/**
 * @brief Move focus to next focusable widget
 * 
 * Uses Tab order (tree traversal order).
 */
void venom_focus_next(void);

/**
 * @brief Move focus to previous focusable widget
 * 
 * Uses reverse Tab order (Shift+Tab).
 */
void venom_focus_prev(void);

/**
 * @brief Clear focus (no widget focused)
 */
void venom_focus_clear(void);

/**
 * @brief Check if a widget has focus
 */
VenomBool venom_focus_has(const VenomWidget* widget);

/**
 * @brief Set the root widget for focus traversal
 * 
 * Called when window root changes.
 */
void venom_focus_set_root(VenomWidget* root);

/**
 * @brief Get all focusable widgets in order
 * 
 * @param widgets Output array for widgets
 * @param max_count Maximum number to return
 * @return Number of focusable widgets found
 */
VenomU32 venom_focus_get_focusable_list(VenomWidget** widgets, VenomU32 max_count);

/* ============================================================================
 * WIDGET FOCUS HELPERS
 * ============================================================================ */

/**
 * @brief Mark a widget as focusable
 */
void venom_widget_set_focusable(VenomWidget* widget, VenomBool focusable);

/**
 * @brief Check if widget is focusable
 */
VenomBool venom_widget_is_focusable(const VenomWidget* widget);

/**
 * @brief Request focus for a widget
 */
VenomBool venom_widget_request_focus(VenomWidget* widget);

/**
 * @brief Check if widget has focus
 */
VenomBool venom_widget_has_focus(const VenomWidget* widget);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_FOCUS_H */
