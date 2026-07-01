/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_context_menu.h - Context menu (right-click menu)
 */

#ifndef VAXP_CONTEXT_MENU_H
#define VAXP_CONTEXT_MENU_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VaxpContextMenu VaxpContextMenu;
typedef struct VaxpMenuItem VaxpMenuItem;

typedef void (*VaxpMenuItemCallback)(VaxpMenuItem* item, void* user_data);

/* ============================================================================
 * MENU ITEM
 * ============================================================================ */

struct VaxpMenuItem {
    char* label;
    char* shortcut;              /* Keyboard shortcut display (e.g., "Ctrl+C") */
    VaxpBool enabled;
    VaxpBool is_separator;
    VaxpMenuItemCallback on_click;
    void* user_data;
    
    /* Submenu (optional) */
    VaxpContextMenu* submenu;
    
    /* Internal */
    VaxpMenuItem* next;
};

/* ============================================================================
 * CONTEXT MENU
 * ============================================================================ */

struct VaxpContextMenu {
    VaxpWidget base;
    
    VaxpMenuItem* items;
    VaxpU32 item_count;
    
    VaxpBool is_open;
    VaxpF32 popup_x;
    VaxpF32 popup_y;
    
    /* State */
    VaxpI32 hover_index;
    VaxpContextMenu* open_submenu;
    
    /* Styling */
    VaxpColor background_color;
    VaxpColor hover_color;
    VaxpColor text_color;
    VaxpColor disabled_color;
    VaxpF32 item_height;
    VaxpF32 padding;
    VaxpF32 corner_radius;
    VaxpF32 min_width;
};

VaxpResultPtr vaxp_context_menu_create(void);
VaxpResult vaxp_context_menu_add_item(VaxpContextMenu* menu, const char* label, 
                                         VaxpMenuItemCallback callback, void* data);
VaxpResult vaxp_context_menu_add_separator(VaxpContextMenu* menu);
VaxpResult vaxp_context_menu_add_submenu(VaxpContextMenu* menu, const char* label,
                                            VaxpContextMenu* submenu);
void vaxp_context_menu_show(VaxpContextMenu* menu, VaxpF32 x, VaxpF32 y);
void vaxp_context_menu_hide(VaxpContextMenu* menu);
void vaxp_context_menu_clear(VaxpContextMenu* menu);

extern const VaxpWidgetClass vaxp_context_menu_class;

#ifdef __cplusplus
}
#endif

#endif /* VAXP_CONTEXT_MENU_H */
