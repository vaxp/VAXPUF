/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_menubar.h - Menu Bar Widget (Classic desktop-style menu bar)
 * 
 * Provides a horizontal menu bar with dropdown menus (File, Edit, View, etc.)
*/

#ifndef VENOM_MENUBAR_H
#define VENOM_MENUBAR_H

#include "venom_widget.h"
#include "venom_context_menu.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

typedef struct VenomMenuBar VenomMenuBar;
typedef struct VenomMenuBarItem VenomMenuBarItem;

/* ============================================================================
 * MENU BAR ITEM (Top-level menu entry)
 * ============================================================================ */

/**
 * @brief A single top-level menu item in the menu bar
 */
struct VenomMenuBarItem {
    char* label;                    /**< Menu label (e.g., "File", "Edit") */
    char* mnemonic;                 /**< Underlined character (e.g., "F" for Alt+F) */
    VenomContextMenu* submenu;      /**< Dropdown menu when clicked */
    VenomBool enabled;              /**< Whether this menu is enabled */
    
    /* Computed layout */
    VenomF32 x;                     /**< X position in menu bar */
    VenomF32 width;                 /**< Width of this item */
};

/* ============================================================================
 * MENU BAR WIDGET
 * ============================================================================ */

/**
 * @brief Menu bar widget - horizontal bar with dropdown menus
 */
struct VenomMenuBar {
    VenomWidget base;
    
    /* Content */
    VenomMenuBarItem* items;        /**< Array of menu items */
    VenomU32 item_count;            /**< Number of items */
    VenomU32 item_capacity;         /**< Allocated capacity */
    
    /* State */
    VenomI32 hover_index;           /**< Currently hovered item (-1 = none) */
    VenomI32 active_index;          /**< Currently open menu (-1 = none) */
    VenomBool menu_open;            /**< Whether any menu is open */
    
    /* Appearance */
    VenomColor background;          /**< Background color */
    VenomColor hover_background;    /**< Hover background color */
    VenomColor active_background;   /**< Active/open menu background */
    VenomColor text_color;          /**< Normal text color */
    VenomColor text_color_disabled; /**< Disabled text color */
    VenomColor mnemonic_color;      /**< Underlined mnemonic color */
    VenomF32 height;                /**< Height of menu bar */
    VenomF32 item_padding_h;        /**< Horizontal padding for items */
    VenomF32 item_padding_v;        /**< Vertical padding for items */
    VenomF32 corner_radius;         /**< Corner radius for hover/active */
};

/* ============================================================================
 * CREATION
 * ============================================================================ */

/**
 * @brief Create a new menu bar
 * 
 * @return VenomResultPtr containing VenomMenuBar* or error
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_menubar_create(void);

/* ============================================================================
 * MENU MANAGEMENT
 * ============================================================================ */

/**
 * @brief Add a menu to the menu bar
 * 
 * @param bar Menu bar widget
 * @param label Menu label (e.g., "File")
 * @param submenu Context menu to show when clicked
 * @return VenomResult Success or error
 */
VenomResult venom_menubar_add_menu(VenomMenuBar* bar, const char* label,
                                    VenomContextMenu* submenu);

/**
 * @brief Add a menu with mnemonic (underlined character)
 * 
 * Example: "_File" -> "File" with 'F' underlined (Alt+F)
 * 
 * @param bar Menu bar widget
 * @param label Label with underscore before mnemonic character
 * @param submenu Context menu to show when clicked
 * @return VenomResult Success or error
 */
VenomResult venom_menubar_add_menu_with_mnemonic(VenomMenuBar* bar, 
                                                   const char* label,
                                                   VenomContextMenu* submenu);

/**
 * @brief Remove a menu by index
 */
VenomResult venom_menubar_remove_menu(VenomMenuBar* bar, VenomU32 index);

/**
 * @brief Clear all menus
 */
void venom_menubar_clear(VenomMenuBar* bar);

/**
 * @brief Get menu count
 */
VenomU32 venom_menubar_get_count(const VenomMenuBar* bar);

/**
 * @brief Enable/disable a menu item
 */
void venom_menubar_set_enabled(VenomMenuBar* bar, VenomU32 index, VenomBool enabled);

/* ============================================================================
 * APPEARANCE
 * ============================================================================ */

/**
 * @brief Set background color
 */
void venom_menubar_set_background(VenomMenuBar* bar, VenomColor color);

/**
 * @brief Set hover background color
 */
void venom_menubar_set_hover_background(VenomMenuBar* bar, VenomColor color);

/**
 * @brief Set text color
 */
void venom_menubar_set_text_color(VenomMenuBar* bar, VenomColor color);

/**
 * @brief Set height
 */
void venom_menubar_set_height(VenomMenuBar* bar, VenomF32 height);

/**
 * @brief Set item padding
 */
void venom_menubar_set_padding(VenomMenuBar* bar, VenomF32 horizontal, VenomF32 vertical);

/* ============================================================================
 * MENU OPEN/CLOSE
 * ============================================================================ */

/**
 * @brief Open a specific menu by index
 */
void venom_menubar_open_menu(VenomMenuBar* bar, VenomU32 index);

/**
 * @brief Close all menus
 */
void venom_menubar_close_menus(VenomMenuBar* bar);

/**
 * @brief Check if any menu is open
 */
VenomBool venom_menubar_is_open(const VenomMenuBar* bar);

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
 * @return VENOM_TRUE if a menu was opened
 */
VenomBool venom_menubar_handle_mnemonic(VenomMenuBar* bar, char key);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VenomWidgetClass venom_menubar_class;

#ifdef __cplusplus
}
#endif

#endif /* VENOM_MENUBAR_H */
