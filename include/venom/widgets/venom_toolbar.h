/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_toolbar.h - Toolbar Widget
 * 
 * Horizontal bar containing tool buttons, separators, and other widgets.
 */

#ifndef VENOM_TOOLBAR_H
#define VENOM_TOOLBAR_H

#include "venom_widget.h"
#include "venom_button.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

typedef struct VenomToolbar VenomToolbar;
typedef struct VenomToolbarItem VenomToolbarItem;

/* ============================================================================
 * TOOLBAR ITEM TYPES
 * ============================================================================ */

/**
 * @brief Type of toolbar item
 */
typedef enum VenomToolbarItemType {
    VENOM_TOOLBAR_ITEM_BUTTON,      /**< Button with icon/text */
    VENOM_TOOLBAR_ITEM_TOGGLE,      /**< Toggle button */
    VENOM_TOOLBAR_ITEM_SEPARATOR,   /**< Vertical separator line */
    VENOM_TOOLBAR_ITEM_SPACER,      /**< Flexible spacer */
    VENOM_TOOLBAR_ITEM_WIDGET,      /**< Custom widget */
} VenomToolbarItemType;

/**
 * @brief Toolbar button style
 */
typedef enum VenomToolbarButtonStyle {
    VENOM_TOOLBAR_STYLE_ICON_ONLY,  /**< Show icon only */
    VENOM_TOOLBAR_STYLE_TEXT_ONLY,  /**< Show text only */
    VENOM_TOOLBAR_STYLE_ICON_TEXT,  /**< Icon with text below */
    VENOM_TOOLBAR_STYLE_TEXT_ICON,  /**< Text with icon on right */
} VenomToolbarButtonStyle;

/* ============================================================================
 * TOOLBAR ITEM
 * ============================================================================ */

/**
 * @brief Callback for toolbar button clicks
 */
typedef void (*VenomToolbarCallback)(VenomToolbar* toolbar, VenomU32 item_index, 
                                      void* user_data);

/**
 * @brief A single item in the toolbar
 */
struct VenomToolbarItem {
    VenomToolbarItemType type;      /**< Item type */
    char* label;                    /**< Button label */
    char* icon;                     /**< Icon identifier */
    char* tooltip;                  /**< Tooltip text */
    VenomBool enabled;              /**< Whether item is enabled */
    VenomBool toggled;              /**< Toggle state (for TOGGLE type) */
    VenomWidget* widget;            /**< Custom widget (for WIDGET type) */
    VenomToolbarCallback on_click;  /**< Click callback */
    void* user_data;                /**< User data for callback */
    
    /* Computed layout */
    VenomF32 x;                     /**< X position */
    VenomF32 width;                 /**< Item width */
};

/* ============================================================================
 * TOOLBAR WIDGET
 * ============================================================================ */

/**
 * @brief Toolbar widget - horizontal bar with buttons and tools
 */
struct VenomToolbar {
    VenomWidget base;
    
    /* Content */
    VenomToolbarItem* items;        /**< Array of items */
    VenomU32 item_count;            /**< Number of items */
    VenomU32 item_capacity;         /**< Allocated capacity */
    
    /* State */
    VenomI32 hover_index;           /**< Currently hovered item */
    VenomI32 pressed_index;         /**< Currently pressed item */
    
    /* Appearance */
    VenomToolbarButtonStyle button_style;
    VenomColor background;          /**< Background color */
    VenomColor button_hover;        /**< Button hover color */
    VenomColor button_pressed;      /**< Button pressed color */
    VenomColor button_toggled;      /**< Toggled button color */
    VenomColor icon_color;          /**< Icon color */
    VenomColor text_color;          /**< Text color */
    VenomColor separator_color;     /**< Separator color */
    VenomF32 height;                /**< Toolbar height */
    VenomF32 button_size;           /**< Button size (square) */
    VenomF32 spacing;               /**< Spacing between items */
    VenomF32 padding_h;             /**< Horizontal padding */
    VenomF32 corner_radius;         /**< Button corner radius */
    VenomF32 icon_size;             /**< Icon size */
};

/* ============================================================================
 * CREATION
 * ============================================================================ */

/**
 * @brief Create a new toolbar
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_toolbar_create(void);

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
VenomI32 venom_toolbar_add_button(VenomToolbar* toolbar,
                                   const char* icon,
                                   const char* label,
                                   const char* tooltip,
                                   VenomToolbarCallback callback,
                                   void* user_data);

/**
 * @brief Add a toggle button to the toolbar
 */
VenomI32 venom_toolbar_add_toggle(VenomToolbar* toolbar,
                                   const char* icon,
                                   const char* label,
                                   const char* tooltip,
                                   VenomBool initial_state,
                                   VenomToolbarCallback callback,
                                   void* user_data);

/**
 * @brief Add a separator to the toolbar
 */
VenomI32 venom_toolbar_add_separator(VenomToolbar* toolbar);

/**
 * @brief Add a flexible spacer to the toolbar
 * 
 * Spacer expands to fill available space.
 */
VenomI32 venom_toolbar_add_spacer(VenomToolbar* toolbar);

/**
 * @brief Add a custom widget to the toolbar
 */
VenomI32 venom_toolbar_add_widget(VenomToolbar* toolbar, VenomWidget* widget);

/**
 * @brief Remove an item by index
 */
VenomResult venom_toolbar_remove_item(VenomToolbar* toolbar, VenomU32 index);

/**
 * @brief Clear all items
 */
void venom_toolbar_clear(VenomToolbar* toolbar);

/**
 * @brief Get item count
 */
VenomU32 venom_toolbar_get_count(const VenomToolbar* toolbar);

/* ============================================================================
 * ITEM STATE
 * ============================================================================ */

/**
 * @brief Enable/disable an item
 */
void venom_toolbar_set_enabled(VenomToolbar* toolbar, VenomU32 index, VenomBool enabled);

/**
 * @brief Get/set toggle state
 */
VenomBool venom_toolbar_get_toggled(const VenomToolbar* toolbar, VenomU32 index);
void venom_toolbar_set_toggled(VenomToolbar* toolbar, VenomU32 index, VenomBool toggled);

/**
 * @brief Update tooltip
 */
void venom_toolbar_set_tooltip(VenomToolbar* toolbar, VenomU32 index, const char* tooltip);

/* ============================================================================
 * APPEARANCE
 * ============================================================================ */

/**
 * @brief Set button display style
 */
void venom_toolbar_set_style(VenomToolbar* toolbar, VenomToolbarButtonStyle style);

/**
 * @brief Set background color
 */
void venom_toolbar_set_background(VenomToolbar* toolbar, VenomColor color);

/**
 * @brief Set height
 */
void venom_toolbar_set_height(VenomToolbar* toolbar, VenomF32 height);

/**
 * @brief Set button size
 */
void venom_toolbar_set_button_size(VenomToolbar* toolbar, VenomF32 size);

/**
 * @brief Set spacing between items
 */
void venom_toolbar_set_spacing(VenomToolbar* toolbar, VenomF32 spacing);

/**
 * @brief Set icon size
 */
void venom_toolbar_set_icon_size(VenomToolbar* toolbar, VenomF32 size);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VenomWidgetClass venom_toolbar_class;

#ifdef __cplusplus
}
#endif

#endif /* VENOM_TOOLBAR_H */
