/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_titlebar.h - Custom Window Title Bar Widget
 * 
 * Provides a customizable title bar for windows with:
 * - Title and icon
 * - Window control buttons (minimize, maximize, close)
 * - Custom widgets support
 * - Draggable for window movement
 */

#ifndef VAXP_TITLEBAR_H
#define VAXP_TITLEBAR_H

#include "vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

typedef struct VaxpTitleBar VaxpTitleBar;

/* ============================================================================
 * TITLE BAR BUTTON STATE
 * ============================================================================ */

typedef enum VaxpTitleBarButton {
    VAXP_TITLEBAR_BUTTON_NONE     = 0,
    VAXP_TITLEBAR_BUTTON_MINIMIZE = (1 << 0),
    VAXP_TITLEBAR_BUTTON_MAXIMIZE = (1 << 1),
    VAXP_TITLEBAR_BUTTON_CLOSE    = (1 << 2),
    VAXP_TITLEBAR_BUTTON_ALL      = 0x7,
} VaxpTitleBarButton;

/* ============================================================================
 * CALLBACKS
 * ============================================================================ */

typedef void (*VaxpTitleBarCallback)(VaxpTitleBar* titlebar, void* user_data);
typedef void (*VaxpTitleBarDragCallback)(VaxpTitleBar* titlebar, 
                                           VaxpI32 delta_x, VaxpI32 delta_y,
                                           void* user_data);

/* ============================================================================
 * TITLE BAR WIDGET
 * ============================================================================ */

/**
 * @brief Custom window title bar
 */
struct VaxpTitleBar {
    VaxpWidget base;
    
    /* Content */
    char* title;                    /**< Window title */
    VaxpWidget* icon;              /**< Window icon widget (optional) */
    VaxpWidget** left_widgets;     /**< Custom left widgets */
    VaxpU32 left_widget_count;
    VaxpWidget** right_widgets;    /**< Custom right widgets (before buttons) */
    VaxpU32 right_widget_count;
    
    /* Control buttons */
    VaxpTitleBarButton visible_buttons; /**< Which buttons to show */
    VaxpI32 hover_button;          /**< Currently hovered button (-1=none, 0=min, 1=max, 2=close) */
    VaxpI32 pressed_button;        /**< Currently pressed button */
    VaxpBool is_maximized;         /**< Current maximize state */
    
    /* Callbacks */
    VaxpTitleBarCallback on_minimize;
    VaxpTitleBarCallback on_maximize;
    VaxpTitleBarCallback on_restore;
    VaxpTitleBarCallback on_close;
    VaxpTitleBarDragCallback on_drag;
    void* user_data;
    
    /* Dragging state */
    VaxpBool is_dragging;
    VaxpI32 drag_start_x;
    VaxpI32 drag_start_y;
    
    /* Appearance */
    VaxpColor background;          /**< Background color */
    VaxpColor title_color;         /**< Title text color */
    VaxpColor button_hover;        /**< Button hover color */
    VaxpColor button_pressed;      /**< Button pressed color */
    VaxpColor close_hover;         /**< Close button hover (red) */
    VaxpColor close_pressed;       /**< Close button pressed */
    VaxpColor icon_color;          /**< Button icon color */
    VaxpF32 height;                /**< Title bar height */
    VaxpF32 button_width;          /**< Control button width */
    VaxpF32 padding_h;             /**< Horizontal padding */
    VaxpF32 icon_padding;          /**< Padding after icon */
    VaxpF32 title_font_size;       /**< Title font size */
    VaxpBool center_title;         /**< Center the title */
};

/* ============================================================================
 * CREATION
 * ============================================================================ */

/**
 * @brief Create a new title bar
 * 
 * @param title Window title
 * @return VaxpResultPtr containing VaxpTitleBar* or error
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_titlebar_create(const char* title);

/* ============================================================================
 * CONFIGURATION
 * ============================================================================ */

/**
 * @brief Set title text
 */
void vaxp_titlebar_set_title(VaxpTitleBar* bar, const char* title);

/**
 * @brief Get title text
 */
const char* vaxp_titlebar_get_title(const VaxpTitleBar* bar);

/**
 * @brief Set icon widget
 */
void vaxp_titlebar_set_icon(VaxpTitleBar* bar, VaxpWidget* icon);

/**
 * @brief Set which control buttons to show
 */
void vaxp_titlebar_set_buttons(VaxpTitleBar* bar, VaxpTitleBarButton buttons);

/**
 * @brief Set maximize state (changes maximize button icon)
 */
void vaxp_titlebar_set_maximized(VaxpTitleBar* bar, VaxpBool maximized);

/**
 * @brief Get maximize state
 */
VaxpBool vaxp_titlebar_is_maximized(const VaxpTitleBar* bar);

/* ============================================================================
 * CUSTOM WIDGETS
 * ============================================================================ */

/**
 * @brief Add a widget to the left side (after icon)
 */
VaxpResult vaxp_titlebar_add_left_widget(VaxpTitleBar* bar, VaxpWidget* widget);

/**
 * @brief Add a widget to the right side (before control buttons)
 */
VaxpResult vaxp_titlebar_add_right_widget(VaxpTitleBar* bar, VaxpWidget* widget);

/**
 * @brief Clear all custom widgets
 */
void vaxp_titlebar_clear_widgets(VaxpTitleBar* bar);

/* ============================================================================
 * CALLBACKS
 * ============================================================================ */

/**
 * @brief Set callback for minimize button
 */
void vaxp_titlebar_on_minimize(VaxpTitleBar* bar, VaxpTitleBarCallback callback, void* data);

/**
 * @brief Set callback for maximize button
 */
void vaxp_titlebar_on_maximize(VaxpTitleBar* bar, VaxpTitleBarCallback callback, void* data);

/**
 * @brief Set callback for restore (when clicking maximize while maximized)
 */
void vaxp_titlebar_on_restore(VaxpTitleBar* bar, VaxpTitleBarCallback callback, void* data);

/**
 * @brief Set callback for close button
 */
void vaxp_titlebar_on_close(VaxpTitleBar* bar, VaxpTitleBarCallback callback, void* data);

/**
 * @brief Set callback for dragging (for window movement)
 */
void vaxp_titlebar_on_drag(VaxpTitleBar* bar, VaxpTitleBarDragCallback callback, void* data);

/* ============================================================================
 * APPEARANCE
 * ============================================================================ */

/**
 * @brief Set background color
 */
void vaxp_titlebar_set_background(VaxpTitleBar* bar, VaxpColor color);

/**
 * @brief Set title color
 */
void vaxp_titlebar_set_title_color(VaxpTitleBar* bar, VaxpColor color);

/**
 * @brief Set height
 */
void vaxp_titlebar_set_height(VaxpTitleBar* bar, VaxpF32 height);

/**
 * @brief Set whether title is centered
 */
void vaxp_titlebar_set_center_title(VaxpTitleBar* bar, VaxpBool centered);

/**
 * @brief Set button width
 */
void vaxp_titlebar_set_button_width(VaxpTitleBar* bar, VaxpF32 width);

/* ============================================================================
 * THEME PRESETS
 * ============================================================================ */

/**
 * @brief Apply light theme
 */
void vaxp_titlebar_apply_light_theme(VaxpTitleBar* bar);

/**
 * @brief Apply dark theme
 */
void vaxp_titlebar_apply_dark_theme(VaxpTitleBar* bar);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VaxpWidgetClass vaxp_titlebar_class;

#ifdef __cplusplus
}
#endif

#endif /* VAXP_TITLEBAR_H */
