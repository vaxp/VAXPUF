/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_theme.h - Theming system for consistent styling
 * 
 * Features:
 * - Color palette with semantic colors
 * - Typography settings
 * - Component-specific styling
 * - Light and dark theme presets
 */

#ifndef VENOM_THEME_H
#define VENOM_THEME_H

#include "venom/core/venom_types.h"
#include "venom/graphics/venom_canvas.h"  /* For VenomColor */

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * COLOR PALETTE
 * ============================================================================ */

/**
 * @brief Semantic color palette for UI elements
 */
typedef struct VenomColorPalette {
    /* Primary colors */
    VenomColor primary;
    VenomColor primary_variant;
    VenomColor on_primary;          /* Text/icons on primary */
    
    /* Secondary colors */
    VenomColor secondary;
    VenomColor secondary_variant;
    VenomColor on_secondary;
    
    /* Background colors */
    VenomColor background;
    VenomColor surface;
    VenomColor on_background;
    VenomColor on_surface;
    
    /* Status colors */
    VenomColor error;
    VenomColor on_error;
    VenomColor success;
    VenomColor warning;
    VenomColor info;
    
    /* UI element colors */
    VenomColor divider;
    VenomColor disabled;
    VenomColor shadow;
    
} VenomColorPalette;

/* ============================================================================
 * TYPOGRAPHY
 * ============================================================================ */

/**
 * @brief Text style definition
 */
typedef struct VenomTextStyle {
    const char* font_family;
    VenomF32 font_size;
    VenomF32 line_height;
    VenomF32 letter_spacing;
    VenomBool is_bold;
    VenomBool is_italic;
} VenomTextStyle;

/**
 * @brief Typography scale for the theme
 */
typedef struct VenomTypography {
    VenomTextStyle h1;
    VenomTextStyle h2;
    VenomTextStyle h3;
    VenomTextStyle h4;
    VenomTextStyle h5;
    VenomTextStyle h6;
    VenomTextStyle body1;
    VenomTextStyle body2;
    VenomTextStyle caption;
    VenomTextStyle button;
    VenomTextStyle overline;
} VenomTypography;

/* ============================================================================
 * COMPONENT STYLES
 * ============================================================================ */

/**
 * @brief Button styling
 */
typedef struct VenomButtonStyle {
    VenomF32 corner_radius;
    VenomF32 padding_horizontal;
    VenomF32 padding_vertical;
    VenomF32 border_width;
    VenomColor text_color;
    VenomColor background_color;
    VenomColor hover_color;
    VenomColor active_color;
    VenomColor border_color;
} VenomButtonStyle;

/**
 * @brief Input field styling
 */
typedef struct VenomInputStyle {
    VenomF32 corner_radius;
    VenomF32 padding_horizontal;
    VenomF32 padding_vertical;
    VenomF32 border_width;
    VenomColor text_color;
    VenomColor placeholder_color;
    VenomColor background_color;
    VenomColor border_color;
    VenomColor focus_border_color;
} VenomInputStyle;

/**
 * @brief Scrollbar styling
 */
typedef struct VenomScrollbarStyle {
    VenomF32 width;
    VenomF32 thumb_radius;
    VenomColor track_color;
    VenomColor thumb_color;
    VenomColor thumb_hover_color;
} VenomScrollbarStyle;

/* ============================================================================
 * THEME STRUCTURE
 * ============================================================================ */

/**
 * @brief Complete theme definition
 */
typedef struct VenomTheme {
    const char* name;
    VenomBool is_dark;
    
    VenomColorPalette colors;
    VenomTypography typography;
    
    /* Component styles */
    VenomButtonStyle button;
    VenomInputStyle input;
    VenomScrollbarStyle scrollbar;
    
    /* Spacing */
    VenomF32 spacing_xs;    /* 4px */
    VenomF32 spacing_sm;    /* 8px */
    VenomF32 spacing_md;    /* 16px */
    VenomF32 spacing_lg;    /* 24px */
    VenomF32 spacing_xl;    /* 32px */
    
    /* Default corner radii */
    VenomF32 radius_sm;     /* 4px */
    VenomF32 radius_md;     /* 8px */
    VenomF32 radius_lg;     /* 16px */
    
    /* Shadows */
    VenomF32 shadow_elevation_1;
    VenomF32 shadow_elevation_2;
    VenomF32 shadow_elevation_3;
    
} VenomTheme;

/* ============================================================================
 * THEME API
 * ============================================================================ */

/**
 * @brief Get the current global theme
 */
const VenomTheme* venom_theme_get_current(void);

/**
 * @brief Set the global theme
 */
void venom_theme_set(const VenomTheme* theme);

/**
 * @brief Get the light theme preset
 */
const VenomTheme* venom_theme_light(void);

/**
 * @brief Get the dark theme preset
 */
const VenomTheme* venom_theme_dark(void);

/**
 * @brief Create a custom theme based on colors
 */
VenomTheme venom_theme_create(VenomColor primary, VenomColor secondary, VenomBool is_dark);

/* ============================================================================
 * HELPER COLORS
 * ============================================================================ */

/* Pre-defined colors for easy use */
#define VENOM_COLOR_TRANSPARENT ((VenomColor){ 0, 0, 0, 0 })
#define VENOM_COLOR_WHITE       ((VenomColor){ 255, 255, 255, 255 })
#define VENOM_COLOR_BLACK       ((VenomColor){ 0, 0, 0, 255 })
#define VENOM_COLOR_GRAY_50     ((VenomColor){ 250, 250, 250, 255 })
#define VENOM_COLOR_GRAY_100    ((VenomColor){ 245, 245, 245, 255 })
#define VENOM_COLOR_GRAY_200    ((VenomColor){ 238, 238, 238, 255 })
#define VENOM_COLOR_GRAY_300    ((VenomColor){ 224, 224, 224, 255 })
#define VENOM_COLOR_GRAY_400    ((VenomColor){ 189, 189, 189, 255 })
#define VENOM_COLOR_GRAY_500    ((VenomColor){ 158, 158, 158, 255 })
#define VENOM_COLOR_GRAY_600    ((VenomColor){ 117, 117, 117, 255 })
#define VENOM_COLOR_GRAY_700    ((VenomColor){ 97, 97, 97, 255 })
#define VENOM_COLOR_GRAY_800    ((VenomColor){ 66, 66, 66, 255 })
#define VENOM_COLOR_GRAY_900    ((VenomColor){ 33, 33, 33, 255 })

/* Material Design colors */
#define VENOM_COLOR_BLUE        ((VenomColor){ 33, 150, 243, 255 })
#define VENOM_COLOR_INDIGO      ((VenomColor){ 63, 81, 181, 255 })
#define VENOM_COLOR_PURPLE      ((VenomColor){ 156, 39, 176, 255 })
#define VENOM_COLOR_PINK        ((VenomColor){ 233, 30, 99, 255 })
#define VENOM_COLOR_RED         ((VenomColor){ 244, 67, 54, 255 })
#define VENOM_COLOR_ORANGE      ((VenomColor){ 255, 152, 0, 255 })
#define VENOM_COLOR_YELLOW      ((VenomColor){ 255, 235, 59, 255 })
#define VENOM_COLOR_GREEN       ((VenomColor){ 76, 175, 80, 255 })
#define VENOM_COLOR_TEAL        ((VenomColor){ 0, 150, 136, 255 })
#define VENOM_COLOR_CYAN        ((VenomColor){ 0, 188, 212, 255 })

#ifdef __cplusplus
}
#endif

#endif /* VENOM_THEME_H */
