/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_context_menu.h - Context menu (right-click menu)
 */

#ifndef VENOM_CONTEXT_MENU_H
#define VENOM_CONTEXT_MENU_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VenomContextMenu VenomContextMenu;
typedef struct VenomMenuItem VenomMenuItem;

typedef void (*VenomMenuItemCallback)(VenomMenuItem* item, void* user_data);

/* ============================================================================
 * MENU ITEM
 * ============================================================================ */

struct VenomMenuItem {
    char* label;
    char* shortcut;              /* Keyboard shortcut display (e.g., "Ctrl+C") */
    VenomBool enabled;
    VenomBool is_separator;
    VenomMenuItemCallback on_click;
    void* user_data;
    
    /* Submenu (optional) */
    VenomContextMenu* submenu;
    
    /* Internal */
    VenomMenuItem* next;
};

/* ============================================================================
 * CONTEXT MENU
 * ============================================================================ */

struct VenomContextMenu {
    VenomWidget base;
    
    VenomMenuItem* items;
    VenomU32 item_count;
    
    VenomBool is_open;
    VenomF32 popup_x;
    VenomF32 popup_y;
    
    /* State */
    VenomI32 hover_index;
    VenomContextMenu* open_submenu;
    
    /* Styling */
    VenomColor background_color;
    VenomColor hover_color;
    VenomColor text_color;
    VenomColor disabled_color;
    VenomF32 item_height;
    VenomF32 padding;
    VenomF32 corner_radius;
    VenomF32 min_width;
};

VenomResultPtr venom_context_menu_create(void);
VenomResult venom_context_menu_add_item(VenomContextMenu* menu, const char* label, 
                                         VenomMenuItemCallback callback, void* data);
VenomResult venom_context_menu_add_separator(VenomContextMenu* menu);
VenomResult venom_context_menu_add_submenu(VenomContextMenu* menu, const char* label,
                                            VenomContextMenu* submenu);
void venom_context_menu_show(VenomContextMenu* menu, VenomF32 x, VenomF32 y);
void venom_context_menu_hide(VenomContextMenu* menu);
void venom_context_menu_clear(VenomContextMenu* menu);

extern const VenomWidgetClass venom_context_menu_class;

#ifdef __cplusplus
}
#endif

#endif /* VENOM_CONTEXT_MENU_H */
