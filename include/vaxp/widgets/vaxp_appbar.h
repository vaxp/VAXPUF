/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_appbar.h - Application Bar Widget (Material Design style)
*/

#ifndef VAXP_APPBAR_H
#define VAXP_APPBAR_H

#include "vaxp_widget.h"
#include "../core/vaxp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * APPBAR TYPES
 * ============================================================================ */

/**
 * @brief AppBar elevation presets
 */
typedef enum {
    VAXP_APPBAR_FLAT = 0,      /* No shadow */
    VAXP_APPBAR_ELEVATED = 4,   /* Standard elevation */
    VAXP_APPBAR_PROMINENT = 8   /* Higher elevation */
} VaxpAppBarElevation;

/**
 * @brief AppBar structure
 */
typedef struct VaxpAppBar {
    VaxpWidget base;
    
    /* Content */
    char* title;
    char* subtitle;
    VaxpWidget* leading;       /* Left widget (menu/back button) */
    VaxpWidget** actions;      /* Right action widgets */
    VaxpU32 action_count;
    
    /* Appearance */
    VaxpColor background;
    VaxpColor title_color;
    VaxpColor subtitle_color;
    VaxpF32 elevation;
    VaxpF32 height;
    VaxpBool center_title;
    
    /* Callbacks */
    void (*on_leading_tap)(struct VaxpAppBar* bar, void* user_data);
    void* user_data;
} VaxpAppBar;

/* ============================================================================
 * CREATION
 * ============================================================================ */

/**
 * @brief Create an app bar
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_appbar_create(const char* title);

/* ============================================================================
 * CONFIGURATION
 * ============================================================================ */

/**
 * @brief Set title
 */
void vaxp_appbar_set_title(VaxpAppBar* bar, const char* title);

/**
 * @brief Set subtitle
 */
void vaxp_appbar_set_subtitle(VaxpAppBar* bar, const char* subtitle);

/**
 * @brief Set leading widget (left side - usually menu or back button)
 */
void vaxp_appbar_set_leading(VaxpAppBar* bar, VaxpWidget* leading);

/**
 * @brief Add action widget (right side)
 */
void vaxp_appbar_add_action(VaxpAppBar* bar, VaxpWidget* action);

/**
 * @brief Clear all actions
 */
void vaxp_appbar_clear_actions(VaxpAppBar* bar);

/**
 * @brief Set background color
 */
void vaxp_appbar_set_background(VaxpAppBar* bar, VaxpColor color);

/**
 * @brief Set elevation (shadow depth)
 */
void vaxp_appbar_set_elevation(VaxpAppBar* bar, VaxpF32 elevation);

/**
 * @brief Set whether title is centered
 */
void vaxp_appbar_set_center_title(VaxpAppBar* bar, VaxpBool center);

/**
 * @brief Set height
 */
void vaxp_appbar_set_height(VaxpAppBar* bar, VaxpF32 height);

/**
 * @brief Set leading tap callback
 */
void vaxp_appbar_set_on_leading_tap(VaxpAppBar* bar, 
                                      void (*callback)(VaxpAppBar*, void*),
                                      void* user_data);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VaxpWidgetClass vaxp_appbar_class;

#ifdef __cplusplus
}
#endif

#endif /* VAXP_APPBAR_H */
