/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_drawer.h - Side Drawer Navigation Widget
 */

#ifndef VAXP_DRAWER_H
#define VAXP_DRAWER_H

#include "vaxp_widget.h"
#include "../core/vaxp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * TYPES
 * ============================================================================ */

/**
 * @brief Drawer item
 */
typedef struct VaxpDrawerItem {
    const char* icon;       /* Icon name */
    const char* label;      /* Label text */
    VaxpBool is_divider;   /* Is this a divider instead of item? */
    VaxpBool selected;     /* Is currently selected */
    void (*on_tap)(VaxpU32 index, void* user_data);
    void* user_data;
} VaxpDrawerItem;

/**
 * @brief Drawer position
 */
typedef enum {
    VAXP_DRAWER_LEFT,
    VAXP_DRAWER_RIGHT
} VaxpDrawerPosition;

/**
 * @brief Drawer structure
 */
typedef struct VaxpDrawer {
    VaxpWidget base;
    
    /* Header */
    VaxpWidget* header;
    
    /* Items */
    VaxpDrawerItem* items;
    VaxpU32 item_count;
    VaxpI32 selected_index;
    
    /* Appearance */
    VaxpColor background;
    VaxpColor selected_bg;
    VaxpColor item_color;
    VaxpColor selected_color;
    VaxpF32 width;
    VaxpDrawerPosition position;
    
    /* State */
    VaxpBool is_open;
    VaxpF32 open_progress;  /* 0.0 to 1.0 for animation */
    
    /* Callbacks */
    void (*on_item_tap)(struct VaxpDrawer* drawer, VaxpU32 index, void* user_data);
    void* user_data;
} VaxpDrawer;

/* ============================================================================
 * CREATION
 * ============================================================================ */

/**
 * @brief Create a drawer
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_drawer_create(void);

/* ============================================================================
 * CONFIGURATION
 * ============================================================================ */

/**
 * @brief Set header widget
 */
void vaxp_drawer_set_header(VaxpDrawer* drawer, VaxpWidget* header);

/**
 * @brief Set items
 */
void vaxp_drawer_set_items(VaxpDrawer* drawer, const VaxpDrawerItem* items, VaxpU32 count);

/**
 * @brief Add single item
 */
void vaxp_drawer_add_item(VaxpDrawer* drawer, const VaxpDrawerItem* item);

/**
 * @brief Add divider
 */
void vaxp_drawer_add_divider(VaxpDrawer* drawer);

/**
 * @brief Set selected index
 */
void vaxp_drawer_set_selected(VaxpDrawer* drawer, VaxpI32 index);

/**
 * @brief Open drawer
 */
void vaxp_drawer_open(VaxpDrawer* drawer);

/**
 * @brief Close drawer
 */
void vaxp_drawer_close(VaxpDrawer* drawer);

/**
 * @brief Toggle drawer
 */
void vaxp_drawer_toggle(VaxpDrawer* drawer);

/**
 * @brief Check if open
 */
VaxpBool vaxp_drawer_is_open(VaxpDrawer* drawer);

/**
 * @brief Set width
 */
void vaxp_drawer_set_width(VaxpDrawer* drawer, VaxpF32 width);

/**
 * @brief Set position
 */
void vaxp_drawer_set_position(VaxpDrawer* drawer, VaxpDrawerPosition position);

/**
 * @brief Set colors
 */
void vaxp_drawer_set_colors(VaxpDrawer* drawer,
                              VaxpColor background,
                              VaxpColor item_color,
                              VaxpColor selected_bg,
                              VaxpColor selected_color);

/**
 * @brief Set item tap callback
 */
void vaxp_drawer_set_on_item_tap(VaxpDrawer* drawer,
                                   void (*callback)(VaxpDrawer*, VaxpU32, void*),
                                   void* user_data);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VaxpWidgetClass vaxp_drawer_class;

#ifdef __cplusplus
}
#endif

#endif /* VAXP_DRAWER_H */
