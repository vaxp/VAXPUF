/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_toolbar.h - Toolbar Widget
 * 
 * Horizontal bar containing tool buttons, separators, and other widgets.
 */

#ifndef VAXP_TOOLBAR_H
#define VAXP_TOOLBAR_H

#include "vaxp_widget.h"
#include "vaxp_button.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

typedef struct VaxpToolbar VaxpToolbar;
typedef struct VaxpToolbarItem VaxpToolbarItem;

/* ============================================================================
 * TOOLBAR ITEM TYPES
 * ============================================================================ */

/**
 * @brief Type of toolbar item
 */
typedef enum VaxpToolbarItemType {
    VAXP_TOOLBAR_ITEM_BUTTON,      /**< Button with icon/text */
    VAXP_TOOLBAR_ITEM_TOGGLE,      /**< Toggle button */
    VAXP_TOOLBAR_ITEM_SEPARATOR,   /**< Vertical separator line */
    VAXP_TOOLBAR_ITEM_SPACER,      /**< Flexible spacer */
    VAXP_TOOLBAR_ITEM_WIDGET,      /**< Custom widget */
} VaxpToolbarItemType;

/**
 * @brief Toolbar button style
 */
typedef enum VaxpToolbarButtonStyle {
    VAXP_TOOLBAR_STYLE_ICON_ONLY,  /**< Show icon only */
    VAXP_TOOLBAR_STYLE_TEXT_ONLY,  /**< Show text only */
    VAXP_TOOLBAR_STYLE_ICON_TEXT,  /**< Icon with text below */
    VAXP_TOOLBAR_STYLE_TEXT_ICON,  /**< Text with icon on right */
} VaxpToolbarButtonStyle;

/* ============================================================================
 * TOOLBAR ITEM
 * ============================================================================ */

/**
 * @brief Callback for toolbar button clicks
 */
typedef void (*VaxpToolbarCallback)(VaxpToolbar* toolbar, VaxpU32 item_index, 
                                      void* user_data);

/**
 * @brief A single item in the toolbar
 */
struct VaxpToolbarItem {
    VaxpToolbarItemType type;      /**< Item type */
    char* label;                    /**< Button label */
    char* icon;                     /**< Icon identifier */
    char* tooltip;                  /**< Tooltip text */
    VaxpBool enabled;              /**< Whether item is enabled */
    VaxpBool toggled;              /**< Toggle state (for TOGGLE type) */
    VaxpWidget* widget;            /**< Custom widget (for WIDGET type) */
    VaxpToolbarCallback on_click;  /**< Click callback */
    void* user_data;                /**< User data for callback */
    
    /* Computed layout */
    VaxpF32 x;                     /**< X position */
    VaxpF32 width;                 /**< Item width */
};

/* ============================================================================
 * TOOLBAR WIDGET
 * ============================================================================ */

/**
 * @brief Toolbar widget - horizontal bar with buttons and tools
 */
struct VaxpToolbar {
    VaxpWidget base;
    
    /* Content */
    VaxpToolbarItem* items;        /**< Array of items */
    VaxpU32 item_count;            /**< Number of items */
    VaxpU32 item_capacity;         /**< Allocated capacity */
    
    /* State */
    VaxpI32 hover_index;           /**< Currently hovered item */
    VaxpI32 pressed_index;         /**< Currently pressed item */
    
    /* Appearance */
    VaxpToolbarButtonStyle button_style;
    VaxpColor background;          /**< Background color */
    VaxpColor button_hover;        /**< Button hover color */
    VaxpColor button_pressed;      /**< Button pressed color */
    VaxpColor button_toggled;      /**< Toggled button color */
    VaxpColor icon_color;          /**< Icon color */
    VaxpColor text_color;          /**< Text color */
    VaxpColor separator_color;     /**< Separator color */
    VaxpF32 height;                /**< Toolbar height */
    VaxpF32 button_size;           /**< Button size (square) */
    VaxpF32 spacing;               /**< Spacing between items */
    VaxpF32 padding_h;             /**< Horizontal padding */
    VaxpF32 corner_radius;         /**< Button corner radius */
    VaxpF32 icon_size;             /**< Icon size */
};

/* ============================================================================
 * CREATION
 * ============================================================================ */

/**
 * @brief Create a new toolbar
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_toolbar_create(void);

/* ============================================================================
 * ITEM MANAGEMENT
 * ============================================================================ */

/**
 * @brief Add a button to the toolbar
 * 
 * @param toolbar Toolbar widget
 * @param icon Icon identifier (can be NULL)
 * @param label Button label (can be NULL)
 * @param tooltip Tooltip text (can be NULL)
 * @param callback Click callback
 * @param user_data User data passed to callback
 * @return Index of added item, or -1 on error
 */
VaxpI32 vaxp_toolbar_add_button(VaxpToolbar* toolbar,
                                   const char* icon,
                                   const char* label,
                                   const char* tooltip,
                                   VaxpToolbarCallback callback,
                                   void* user_data);

/**
 * @brief Add a toggle button to the toolbar
 */
VaxpI32 vaxp_toolbar_add_toggle(VaxpToolbar* toolbar,
                                   const char* icon,
                                   const char* label,
                                   const char* tooltip,
                                   VaxpBool initial_state,
                                   VaxpToolbarCallback callback,
                                   void* user_data);

/**
 * @brief Add a separator to the toolbar
 */
VaxpI32 vaxp_toolbar_add_separator(VaxpToolbar* toolbar);

/**
 * @brief Add a flexible spacer to the toolbar
 * 
 * Spacer expands to fill available space.
 */
VaxpI32 vaxp_toolbar_add_spacer(VaxpToolbar* toolbar);

/**
 * @brief Add a custom widget to the toolbar
 */
VaxpI32 vaxp_toolbar_add_widget(VaxpToolbar* toolbar, VaxpWidget* widget);

/**
 * @brief Remove an item by index
 */
VaxpResult vaxp_toolbar_remove_item(VaxpToolbar* toolbar, VaxpU32 index);

/**
 * @brief Clear all items
 */
void vaxp_toolbar_clear(VaxpToolbar* toolbar);

/**
 * @brief Get item count
 */
VaxpU32 vaxp_toolbar_get_count(const VaxpToolbar* toolbar);

/* ============================================================================
 * ITEM STATE
 * ============================================================================ */

/**
 * @brief Enable/disable an item
 */
void vaxp_toolbar_set_enabled(VaxpToolbar* toolbar, VaxpU32 index, VaxpBool enabled);

/**
 * @brief Get/set toggle state
 */
VaxpBool vaxp_toolbar_get_toggled(const VaxpToolbar* toolbar, VaxpU32 index);
void vaxp_toolbar_set_toggled(VaxpToolbar* toolbar, VaxpU32 index, VaxpBool toggled);

/**
 * @brief Update tooltip
 */
void vaxp_toolbar_set_tooltip(VaxpToolbar* toolbar, VaxpU32 index, const char* tooltip);

/* ============================================================================
 * APPEARANCE
 * ============================================================================ */

/**
 * @brief Set button display style
 */
void vaxp_toolbar_set_style(VaxpToolbar* toolbar, VaxpToolbarButtonStyle style);

/**
 * @brief Set background color
 */
void vaxp_toolbar_set_background(VaxpToolbar* toolbar, VaxpColor color);

/**
 * @brief Set height
 */
void vaxp_toolbar_set_height(VaxpToolbar* toolbar, VaxpF32 height);

/**
 * @brief Set button size
 */
void vaxp_toolbar_set_button_size(VaxpToolbar* toolbar, VaxpF32 size);

/**
 * @brief Set spacing between items
 */
void vaxp_toolbar_set_spacing(VaxpToolbar* toolbar, VaxpF32 spacing);

/**
 * @brief Set icon size
 */
void vaxp_toolbar_set_icon_size(VaxpToolbar* toolbar, VaxpF32 size);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VaxpWidgetClass vaxp_toolbar_class;

#ifdef __cplusplus
}
#endif

#endif /* VAXP_TOOLBAR_H */
