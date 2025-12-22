/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_titlebar.h - Custom Window Title Bar Widget
 * 
 * Provides a customizable title bar for windows with:
 * - Title and icon
 * - Window control buttons (minimize, maximize, close)
 * - Custom widgets support
 * - Draggable for window movement
 */

#ifndef VENOM_TITLEBAR_H
#define VENOM_TITLEBAR_H

#include "venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

typedef struct VenomTitleBar VenomTitleBar;

/* ============================================================================
 * TITLE BAR BUTTON STATE
 * ============================================================================ */

typedef enum VenomTitleBarButton {
    VENOM_TITLEBAR_BUTTON_NONE     = 0,
    VENOM_TITLEBAR_BUTTON_MINIMIZE = (1 << 0),
    VENOM_TITLEBAR_BUTTON_MAXIMIZE = (1 << 1),
    VENOM_TITLEBAR_BUTTON_CLOSE    = (1 << 2),
    VENOM_TITLEBAR_BUTTON_ALL      = 0x7,
} VenomTitleBarButton;

/* ============================================================================
 * CALLBACKS
 * ============================================================================ */

typedef void (*VenomTitleBarCallback)(VenomTitleBar* titlebar, void* user_data);
typedef void (*VenomTitleBarDragCallback)(VenomTitleBar* titlebar, 
                                           VenomI32 delta_x, VenomI32 delta_y,
                                           void* user_data);

/* ============================================================================
 * TITLE BAR WIDGET
 * ============================================================================ */

/**
 * @brief Custom window title bar
 */
struct VenomTitleBar {
    VenomWidget base;
    
    /* Content */
    char* title;                    /**< Window title */
    VenomWidget* icon;              /**< Window icon widget (optional) */
    VenomWidget** left_widgets;     /**< Custom left widgets */
    VenomU32 left_widget_count;
    VenomWidget** right_widgets;    /**< Custom right widgets (before buttons) */
    VenomU32 right_widget_count;
    
    /* Control buttons */
    VenomTitleBarButton visible_buttons; /**< Which buttons to show */
    VenomI32 hover_button;          /**< Currently hovered button (-1=none, 0=min, 1=max, 2=close) */
    VenomI32 pressed_button;        /**< Currently pressed button */
    VenomBool is_maximized;         /**< Current maximize state */
    
    /* Callbacks */
    VenomTitleBarCallback on_minimize;
    VenomTitleBarCallback on_maximize;
    VenomTitleBarCallback on_restore;
    VenomTitleBarCallback on_close;
    VenomTitleBarDragCallback on_drag;
    void* user_data;
    
    /* Dragging state */
    VenomBool is_dragging;
    VenomI32 drag_start_x;
    VenomI32 drag_start_y;
    
    /* Appearance */
    VenomColor background;          /**< Background color */
    VenomColor title_color;         /**< Title text color */
    VenomColor button_hover;        /**< Button hover color */
    VenomColor button_pressed;      /**< Button pressed color */
    VenomColor close_hover;         /**< Close button hover (red) */
    VenomColor close_pressed;       /**< Close button pressed */
    VenomColor icon_color;          /**< Button icon color */
    VenomF32 height;                /**< Title bar height */
    VenomF32 button_width;          /**< Control button width */
    VenomF32 padding_h;             /**< Horizontal padding */
    VenomF32 icon_padding;          /**< Padding after icon */
    VenomF32 title_font_size;       /**< Title font size */
    VenomBool center_title;         /**< Center the title */
};

/* ============================================================================
 * CREATION
 * ============================================================================ */

/**
 * @brief Create a new title bar
 * 
 * @param title Window title
 * @return VenomResultPtr containing VenomTitleBar* or error
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_titlebar_create(const char* title);

/* ============================================================================
 * CONFIGURATION
 * ============================================================================ */

/**
 * @brief Set title text
 */
void venom_titlebar_set_title(VenomTitleBar* bar, const char* title);

/**
 * @brief Get title text
 */
const char* venom_titlebar_get_title(const VenomTitleBar* bar);

/**
 * @brief Set icon widget
 */
void venom_titlebar_set_icon(VenomTitleBar* bar, VenomWidget* icon);

/**
 * @brief Set which control buttons to show
 */
void venom_titlebar_set_buttons(VenomTitleBar* bar, VenomTitleBarButton buttons);

/**
 * @brief Set maximize state (changes maximize button icon)
 */
void venom_titlebar_set_maximized(VenomTitleBar* bar, VenomBool maximized);

/**
 * @brief Get maximize state
 */
VenomBool venom_titlebar_is_maximized(const VenomTitleBar* bar);

/* ============================================================================
 * CUSTOM WIDGETS
 * ============================================================================ */

/**
 * @brief Add a widget to the left side (after icon)
 */
VenomResult venom_titlebar_add_left_widget(VenomTitleBar* bar, VenomWidget* widget);

/**
 * @brief Add a widget to the right side (before control buttons)
 */
VenomResult venom_titlebar_add_right_widget(VenomTitleBar* bar, VenomWidget* widget);

/**
 * @brief Clear all custom widgets
 */
void venom_titlebar_clear_widgets(VenomTitleBar* bar);

/* ============================================================================
 * CALLBACKS
 * ============================================================================ */

/**
 * @brief Set callback for minimize button
 */
void venom_titlebar_on_minimize(VenomTitleBar* bar, VenomTitleBarCallback callback, void* data);

/**
 * @brief Set callback for maximize button
 */
void venom_titlebar_on_maximize(VenomTitleBar* bar, VenomTitleBarCallback callback, void* data);

/**
 * @brief Set callback for restore (when clicking maximize while maximized)
 */
void venom_titlebar_on_restore(VenomTitleBar* bar, VenomTitleBarCallback callback, void* data);

/**
 * @brief Set callback for close button
 */
void venom_titlebar_on_close(VenomTitleBar* bar, VenomTitleBarCallback callback, void* data);

/**
 * @brief Set callback for dragging (for window movement)
 */
void venom_titlebar_on_drag(VenomTitleBar* bar, VenomTitleBarDragCallback callback, void* data);

/* ============================================================================
 * APPEARANCE
 * ============================================================================ */

/**
 * @brief Set background color
 */
void venom_titlebar_set_background(VenomTitleBar* bar, VenomColor color);

/**
 * @brief Set title color
 */
void venom_titlebar_set_title_color(VenomTitleBar* bar, VenomColor color);

/**
 * @brief Set height
 */
void venom_titlebar_set_height(VenomTitleBar* bar, VenomF32 height);

/**
 * @brief Set whether title is centered
 */
void venom_titlebar_set_center_title(VenomTitleBar* bar, VenomBool centered);

/**
 * @brief Set button width
 */
void venom_titlebar_set_button_width(VenomTitleBar* bar, VenomF32 width);

/* ============================================================================
 * THEME PRESETS
 * ============================================================================ */

/**
 * @brief Apply light theme
 */
void venom_titlebar_apply_light_theme(VenomTitleBar* bar);

/**
 * @brief Apply dark theme
 */
void venom_titlebar_apply_dark_theme(VenomTitleBar* bar);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VenomWidgetClass venom_titlebar_class;

#ifdef __cplusplus
}
#endif

#endif /* VENOM_TITLEBAR_H */
