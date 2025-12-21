/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_drawer.h - Side Drawer Navigation Widget
 */

#ifndef VENOM_DRAWER_H
#define VENOM_DRAWER_H

#include "venom_widget.h"
#include "../core/venom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * TYPES
 * ============================================================================ */

/**
 * @brief Drawer item
 */
typedef struct VenomDrawerItem {
    const char* icon;       /* Icon name */
    const char* label;      /* Label text */
    VenomBool is_divider;   /* Is this a divider instead of item? */
    VenomBool selected;     /* Is currently selected */
    void (*on_tap)(VenomU32 index, void* user_data);
    void* user_data;
} VenomDrawerItem;

/**
 * @brief Drawer position
 */
typedef enum {
    VENOM_DRAWER_LEFT,
    VENOM_DRAWER_RIGHT
} VenomDrawerPosition;

/**
 * @brief Drawer structure
 */
typedef struct VenomDrawer {
    VenomWidget base;
    
    /* Header */
    VenomWidget* header;
    
    /* Items */
    VenomDrawerItem* items;
    VenomU32 item_count;
    VenomI32 selected_index;
    
    /* Appearance */
    VenomColor background;
    VenomColor selected_bg;
    VenomColor item_color;
    VenomColor selected_color;
    VenomF32 width;
    VenomDrawerPosition position;
    
    /* State */
    VenomBool is_open;
    VenomF32 open_progress;  /* 0.0 to 1.0 for animation */
    
    /* Callbacks */
    void (*on_item_tap)(struct VenomDrawer* drawer, VenomU32 index, void* user_data);
    void* user_data;
} VenomDrawer;

/* ============================================================================
 * CREATION
 * ============================================================================ */

/**
 * @brief Create a drawer
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_drawer_create(void);

/* ============================================================================
 * CONFIGURATION
 * ============================================================================ */

/**
 * @brief Set header widget
 */
void venom_drawer_set_header(VenomDrawer* drawer, VenomWidget* header);

/**
 * @brief Set items
 */
void venom_drawer_set_items(VenomDrawer* drawer, const VenomDrawerItem* items, VenomU32 count);

/**
 * @brief Add single item
 */
void venom_drawer_add_item(VenomDrawer* drawer, const VenomDrawerItem* item);

/**
 * @brief Add divider
 */
void venom_drawer_add_divider(VenomDrawer* drawer);

/**
 * @brief Set selected index
 */
void venom_drawer_set_selected(VenomDrawer* drawer, VenomI32 index);

/**
 * @brief Open drawer
 */
void venom_drawer_open(VenomDrawer* drawer);

/**
 * @brief Close drawer
 */
void venom_drawer_close(VenomDrawer* drawer);

/**
 * @brief Toggle drawer
 */
void venom_drawer_toggle(VenomDrawer* drawer);

/**
 * @brief Check if open
 */
VenomBool venom_drawer_is_open(VenomDrawer* drawer);

/**
 * @brief Set width
 */
void venom_drawer_set_width(VenomDrawer* drawer, VenomF32 width);

/**
 * @brief Set position
 */
void venom_drawer_set_position(VenomDrawer* drawer, VenomDrawerPosition position);

/**
 * @brief Set colors
 */
void venom_drawer_set_colors(VenomDrawer* drawer,
                              VenomColor background,
                              VenomColor item_color,
                              VenomColor selected_bg,
                              VenomColor selected_color);

/**
 * @brief Set item tap callback
 */
void venom_drawer_set_on_item_tap(VenomDrawer* drawer,
                                   void (*callback)(VenomDrawer*, VenomU32, void*),
                                   void* user_data);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VenomWidgetClass venom_drawer_class;

#ifdef __cplusplus
}
#endif

#endif /* VENOM_DRAWER_H */
