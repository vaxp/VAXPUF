/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_menubar.h - Menu Bar Widget (Classic desktop-style menu bar)
 * 
 * Provides a horizontal menu bar with dropdown menus (File, Edit, View, etc.)
*/

#ifndef VAXP_MENUBAR_H
#define VAXP_MENUBAR_H

#include "vaxp_widget.h"
#include "vaxp_context_menu.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

typedef struct VaxpMenuBar VaxpMenuBar;
typedef struct VaxpMenuBarItem VaxpMenuBarItem;

/* ============================================================================
 * MENU BAR ITEM (Top-level menu entry)
 * ============================================================================ */

/**
 * @brief A single top-level menu item in the menu bar
 */
struct VaxpMenuBarItem {
    char* label;                    /**< Menu label (e.g., "File", "Edit") */
    char* mnemonic;                 /**< Underlined character (e.g., "F" for Alt+F) */
    VaxpContextMenu* submenu;      /**< Dropdown menu when clicked */
    VaxpBool enabled;              /**< Whether this menu is enabled */
    
    /* Computed layout */
    VaxpF32 x;                     /**< X position in menu bar */
    VaxpF32 width;                 /**< Width of this item */
};

/* ============================================================================
 * MENU BAR WIDGET
 * ============================================================================ */

/**
 * @brief Menu bar widget - horizontal bar with dropdown menus
 */
struct VaxpMenuBar {
    VaxpWidget base;
    
    /* Content */
    VaxpMenuBarItem* items;        /**< Array of menu items */
    VaxpU32 item_count;            /**< Number of items */
    VaxpU32 item_capacity;         /**< Allocated capacity */
    
    /* State */
    VaxpI32 hover_index;           /**< Currently hovered item (-1 = none) */
    VaxpI32 active_index;          /**< Currently open menu (-1 = none) */
    VaxpBool menu_open;            /**< Whether any menu is open */
    
    /* Appearance */
    VaxpColor background;          /**< Background color */
    VaxpColor hover_background;    /**< Hover background color */
    VaxpColor active_background;   /**< Active/open menu background */
    VaxpColor text_color;          /**< Normal text color */
    VaxpColor text_color_disabled; /**< Disabled text color */
    VaxpColor mnemonic_color;      /**< Underlined mnemonic color */
    VaxpF32 height;                /**< Height of menu bar */
    VaxpF32 item_padding_h;        /**< Horizontal padding for items */
    VaxpF32 item_padding_v;        /**< Vertical padding for items */
    VaxpF32 corner_radius;         /**< Corner radius for hover/active */
};

/* ============================================================================
 * CREATION
 * ============================================================================ */

/**
 * @brief Create a new menu bar
 * 
 * @return VaxpResultPtr containing VaxpMenuBar* or error
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_menubar_create(void);

/* ============================================================================
 * MENU MANAGEMENT
 * ============================================================================ */

/**
 * @brief Add a menu to the menu bar
 * 
 * @param bar Menu bar widget
 * @param label Menu label (e.g., "File")
 * @param submenu Context menu to show when clicked
 * @return VaxpResult Success or error
 */
VaxpResult vaxp_menubar_add_menu(VaxpMenuBar* bar, const char* label,
                                    VaxpContextMenu* submenu);

/**
 * @brief Add a menu with mnemonic (underlined character)
 * 
 * Example: "_File" -> "File" with 'F' underlined (Alt+F)
 * 
 * @param bar Menu bar widget
 * @param label Label with underscore before mnemonic character
 * @param submenu Context menu to show when clicked
 * @return VaxpResult Success or error
 */
VaxpResult vaxp_menubar_add_menu_with_mnemonic(VaxpMenuBar* bar, 
                                                   const char* label,
                                                   VaxpContextMenu* submenu);

/**
 * @brief Remove a menu by index
 */
VaxpResult vaxp_menubar_remove_menu(VaxpMenuBar* bar, VaxpU32 index);

/**
 * @brief Clear all menus
 */
void vaxp_menubar_clear(VaxpMenuBar* bar);

/**
 * @brief Get menu count
 */
VaxpU32 vaxp_menubar_get_count(const VaxpMenuBar* bar);

/**
 * @brief Enable/disable a menu item
 */
void vaxp_menubar_set_enabled(VaxpMenuBar* bar, VaxpU32 index, VaxpBool enabled);

/* ============================================================================
 * APPEARANCE
 * ============================================================================ */

/**
 * @brief Set background color
 */
void vaxp_menubar_set_background(VaxpMenuBar* bar, VaxpColor color);

/**
 * @brief Set hover background color
 */
void vaxp_menubar_set_hover_background(VaxpMenuBar* bar, VaxpColor color);

/**
 * @brief Set text color
 */
void vaxp_menubar_set_text_color(VaxpMenuBar* bar, VaxpColor color);

/**
 * @brief Set height
 */
void vaxp_menubar_set_height(VaxpMenuBar* bar, VaxpF32 height);

/**
 * @brief Set item padding
 */
void vaxp_menubar_set_padding(VaxpMenuBar* bar, VaxpF32 horizontal, VaxpF32 vertical);

/* ============================================================================
 * MENU OPEN/CLOSE
 * ============================================================================ */

/**
 * @brief Open a specific menu by index
 */
void vaxp_menubar_open_menu(VaxpMenuBar* bar, VaxpU32 index);

/**
 * @brief Close all menus
 */
void vaxp_menubar_close_menus(VaxpMenuBar* bar);

/**
 * @brief Check if any menu is open
 */
VaxpBool vaxp_menubar_is_open(const VaxpMenuBar* bar);

/* ============================================================================
 * KEYBOARD NAVIGATION
 * ============================================================================ */

/**
 * @brief Handle Alt+key mnemonic
 * 
 * Call this when Alt+key is pressed to open corresponding menu.
 * 
 * @param bar Menu bar widget
 * @param key The key that was pressed
 * @return VAXP_TRUE if a menu was opened
 */
VaxpBool vaxp_menubar_handle_mnemonic(VaxpMenuBar* bar, char key);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VaxpWidgetClass vaxp_menubar_class;

#ifdef __cplusplus
}
#endif

#endif /* VAXP_MENUBAR_H */
