/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_appbar.h - Application Bar Widget (Material Design style)
*/

#ifndef VENOM_APPBAR_H
#define VENOM_APPBAR_H

#include "venom_widget.h"
#include "../core/venom_types.h"

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
    VENOM_APPBAR_FLAT = 0,      /* No shadow */
    VENOM_APPBAR_ELEVATED = 4,   /* Standard elevation */
    VENOM_APPBAR_PROMINENT = 8   /* Higher elevation */
} VenomAppBarElevation;

/**
 * @brief AppBar structure
 */
typedef struct VenomAppBar {
    VenomWidget base;
    
    /* Content */
    char* title;
    char* subtitle;
    VenomWidget* leading;       /* Left widget (menu/back button) */
    VenomWidget** actions;      /* Right action widgets */
    VenomU32 action_count;
    
    /* Appearance */
    VenomColor background;
    VenomColor title_color;
    VenomColor subtitle_color;
    VenomF32 elevation;
    VenomF32 height;
    VenomBool center_title;
    
    /* Callbacks */
    void (*on_leading_tap)(struct VenomAppBar* bar, void* user_data);
    void* user_data;
} VenomAppBar;

/* ============================================================================
 * CREATION
 * ============================================================================ */

/**
 * @brief Create an app bar
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_appbar_create(const char* title);

/* ============================================================================
 * CONFIGURATION
 * ============================================================================ */

/**
 * @brief Set title
 */
void venom_appbar_set_title(VenomAppBar* bar, const char* title);

/**
 * @brief Set subtitle
 */
void venom_appbar_set_subtitle(VenomAppBar* bar, const char* subtitle);

/**
 * @brief Set leading widget (left side - usually menu or back button)
 */
void venom_appbar_set_leading(VenomAppBar* bar, VenomWidget* leading);

/**
 * @brief Add action widget (right side)
 */
void venom_appbar_add_action(VenomAppBar* bar, VenomWidget* action);

/**
 * @brief Clear all actions
 */
void venom_appbar_clear_actions(VenomAppBar* bar);

/**
 * @brief Set background color
 */
void venom_appbar_set_background(VenomAppBar* bar, VenomColor color);

/**
 * @brief Set elevation (shadow depth)
 */
void venom_appbar_set_elevation(VenomAppBar* bar, VenomF32 elevation);

/**
 * @brief Set whether title is centered
 */
void venom_appbar_set_center_title(VenomAppBar* bar, VenomBool center);

/**
 * @brief Set height
 */
void venom_appbar_set_height(VenomAppBar* bar, VenomF32 height);

/**
 * @brief Set leading tap callback
 */
void venom_appbar_set_on_leading_tap(VenomAppBar* bar, 
                                      void (*callback)(VenomAppBar*, void*),
                                      void* user_data);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VenomWidgetClass venom_appbar_class;

#ifdef __cplusplus
}
#endif

#endif /* VENOM_APPBAR_H */
