/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_bottom_nav.h - Bottom Navigation Widget (Material Design style)
 */

#ifndef VENOM_BOTTOM_NAV_H
#define VENOM_BOTTOM_NAV_H

#include "venom_widget.h"
#include "../core/venom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * TYPES
 * ============================================================================ */

/**
 * @brief Navigation item
 */
typedef struct VenomNavItem {
    const char* icon;       /* Icon name or path */
    const char* label;      /* Label text */
    VenomBool enabled;      /* Is item enabled */
} VenomNavItem;

/**
 * @brief Navigation style
 */
typedef enum {
    VENOM_NAV_FIXED,        /* Fixed items, always show labels */
    VENOM_NAV_SHIFTING,     /* Shifting, only show selected label */
    VENOM_NAV_LABELED       /* Always show labels, animate selected */
} VenomNavStyle;

/**
 * @brief Bottom Navigation callback
 */
typedef void (*VenomNavCallback)(VenomI32 index, void* user_data);

/**
 * @brief Bottom Navigation structure
 */
typedef struct VenomBottomNav {
    VenomWidget base;
    
    /* Items */
    VenomNavItem* items;
    VenomU32 item_count;
    VenomI32 selected;
    
    /* Appearance */
    VenomColor background;
    VenomColor active_color;
    VenomColor inactive_color;
    VenomNavStyle style;
    VenomF32 height;
    VenomBool show_labels;
    VenomF32 elevation;
    
    /* Callback */
    VenomNavCallback on_change;
    void* user_data;
    
    /* Animation */
    VenomF32 indicator_x;
    VenomF32 indicator_target;
} VenomBottomNav;

/* ============================================================================
 * CREATION
 * ============================================================================ */

/**
 * @brief Create bottom navigation with items
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_bottom_nav_create(const VenomNavItem* items, VenomU32 count);

/* ============================================================================
 * CONFIGURATION
 * ============================================================================ */

/**
 * @brief Set selected index
 */
void venom_bottom_nav_set_selected(VenomBottomNav* nav, VenomI32 index);

/**
 * @brief Get selected index
 */
VenomI32 venom_bottom_nav_get_selected(VenomBottomNav* nav);

/**
 * @brief Set change callback
 */
void venom_bottom_nav_set_on_change(VenomBottomNav* nav, 
                                     VenomNavCallback callback, 
                                     void* user_data);

/**
 * @brief Set navigation style
 */
void venom_bottom_nav_set_style(VenomBottomNav* nav, VenomNavStyle style);

/**
 * @brief Set colors
 */
void venom_bottom_nav_set_colors(VenomBottomNav* nav,
                                  VenomColor background,
                                  VenomColor active,
                                  VenomColor inactive);

/**
 * @brief Set whether to show labels
 */
void venom_bottom_nav_set_show_labels(VenomBottomNav* nav, VenomBool show);

/**
 * @brief Set elevation (shadow)
 */
void venom_bottom_nav_set_elevation(VenomBottomNav* nav, VenomF32 elevation);

/**
 * @brief Enable/disable item
 */
void venom_bottom_nav_set_item_enabled(VenomBottomNav* nav, VenomU32 index, VenomBool enabled);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VenomWidgetClass venom_bottom_nav_class;

#ifdef __cplusplus
}
#endif

#endif /* VENOM_BOTTOM_NAV_H */
