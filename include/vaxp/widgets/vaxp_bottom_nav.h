/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_bottom_nav.h - Bottom Navigation Widget (Material Design style)
 */

#ifndef VAXP_BOTTOM_NAV_H
#define VAXP_BOTTOM_NAV_H

#include "vaxp_widget.h"
#include "../core/vaxp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * TYPES
 * ============================================================================ */

/**
 * @brief Navigation item
 */
typedef struct VaxpNavItem {
    const char* icon;       /* Icon name or path */
    const char* label;      /* Label text */
    VaxpBool enabled;      /* Is item enabled */
} VaxpNavItem;

/**
 * @brief Navigation style
 */
typedef enum {
    VAXP_NAV_FIXED,        /* Fixed items, always show labels */
    VAXP_NAV_SHIFTING,     /* Shifting, only show selected label */
    VAXP_NAV_LABELED       /* Always show labels, animate selected */
} VaxpNavStyle;

/**
 * @brief Bottom Navigation callback
 */
typedef void (*VaxpNavCallback)(VaxpI32 index, void* user_data);

/**
 * @brief Bottom Navigation structure
 */
typedef struct VaxpBottomNav {
    VaxpWidget base;
    
    /* Items */
    VaxpNavItem* items;
    VaxpU32 item_count;
    VaxpI32 selected;
    
    /* Appearance */
    VaxpColor background;
    VaxpColor active_color;
    VaxpColor inactive_color;
    VaxpNavStyle style;
    VaxpF32 height;
    VaxpBool show_labels;
    VaxpF32 elevation;
    
    /* Callback */
    VaxpNavCallback on_change;
    void* user_data;
    
    /* Animation */
    VaxpF32 indicator_x;
    VaxpF32 indicator_target;
} VaxpBottomNav;

/* ============================================================================
 * CREATION
 * ============================================================================ */

/**
 * @brief Create bottom navigation with items
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_bottom_nav_create(const VaxpNavItem* items, VaxpU32 count);

/* ============================================================================
 * CONFIGURATION
 * ============================================================================ */

/**
 * @brief Set selected index
 */
void vaxp_bottom_nav_set_selected(VaxpBottomNav* nav, VaxpI32 index);

/**
 * @brief Get selected index
 */
VaxpI32 vaxp_bottom_nav_get_selected(VaxpBottomNav* nav);

/**
 * @brief Set change callback
 */
void vaxp_bottom_nav_set_on_change(VaxpBottomNav* nav, 
                                     VaxpNavCallback callback, 
                                     void* user_data);

/**
 * @brief Set navigation style
 */
void vaxp_bottom_nav_set_style(VaxpBottomNav* nav, VaxpNavStyle style);

/**
 * @brief Set colors
 */
void vaxp_bottom_nav_set_colors(VaxpBottomNav* nav,
                                  VaxpColor background,
                                  VaxpColor active,
                                  VaxpColor inactive);

/**
 * @brief Set whether to show labels
 */
void vaxp_bottom_nav_set_show_labels(VaxpBottomNav* nav, VaxpBool show);

/**
 * @brief Set elevation (shadow)
 */
void vaxp_bottom_nav_set_elevation(VaxpBottomNav* nav, VaxpF32 elevation);

/**
 * @brief Enable/disable item
 */
void vaxp_bottom_nav_set_item_enabled(VaxpBottomNav* nav, VaxpU32 index, VaxpBool enabled);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VaxpWidgetClass vaxp_bottom_nav_class;

#ifdef __cplusplus
}
#endif

#endif /* VAXP_BOTTOM_NAV_H */
