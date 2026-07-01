/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_theme.h - Theming system for consistent styling
 * 
 * Features:
 * - Color palette with semantic colors
 * - Typography settings
 * - Component-specific styling
 * - Light and dark theme presets
 */

#ifndef VAXP_THEME_H
#define VAXP_THEME_H

#include "vaxp/core/vaxp_types.h"
#include "vaxp/graphics/vaxp_canvas.h"  /* For VaxpColor */

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * COLOR PALETTE
 * ============================================================================ */

/**
 * @brief Semantic color palette for UI elements
 */
typedef struct VaxpColorPalette {
    /* Primary colors */
    VaxpColor primary;
    VaxpColor primary_variant;
    VaxpColor on_primary;          /* Text/icons on primary */
    
    /* Secondary colors */
    VaxpColor secondary;
    VaxpColor secondary_variant;
    VaxpColor on_secondary;
    
    /* Background colors */
    VaxpColor background;
    VaxpColor surface;
    VaxpColor on_background;
    VaxpColor on_surface;
    
    /* Status colors */
    VaxpColor error;
    VaxpColor on_error;
    VaxpColor success;
    VaxpColor warning;
    VaxpColor info;
    
    /* UI element colors */
    VaxpColor divider;
    VaxpColor disabled;
    VaxpColor shadow;
    
} VaxpColorPalette;

/* ============================================================================
 * TYPOGRAPHY
 * ============================================================================ */

/**
 * @brief Text style definition
 */
typedef struct VaxpTextStyle {
    const char* font_family;
    VaxpF32 font_size;
    VaxpF32 line_height;
    VaxpF32 letter_spacing;
    VaxpBool is_bold;
    VaxpBool is_italic;
} VaxpTextStyle;

/**
 * @brief Typography scale for the theme
 */
typedef struct VaxpTypography {
    VaxpTextStyle h1;
    VaxpTextStyle h2;
    VaxpTextStyle h3;
    VaxpTextStyle h4;
    VaxpTextStyle h5;
    VaxpTextStyle h6;
    VaxpTextStyle body1;
    VaxpTextStyle body2;
    VaxpTextStyle caption;
    VaxpTextStyle button;
    VaxpTextStyle overline;
} VaxpTypography;

/* ============================================================================
 * COMPONENT STYLES
 * ============================================================================ */

/**
 * @brief Button styling
 */
typedef struct VaxpButtonStyle {
    VaxpF32 corner_radius;
    VaxpF32 padding_horizontal;
    VaxpF32 padding_vertical;
    VaxpF32 border_width;
    VaxpColor text_color;
    VaxpColor background_color;
    VaxpColor hover_color;
    VaxpColor active_color;
    VaxpColor border_color;
} VaxpButtonStyle;

/**
 * @brief Input field styling
 */
typedef struct VaxpInputStyle {
    VaxpF32 corner_radius;
    VaxpF32 padding_horizontal;
    VaxpF32 padding_vertical;
    VaxpF32 border_width;
    VaxpColor text_color;
    VaxpColor placeholder_color;
    VaxpColor background_color;
    VaxpColor border_color;
    VaxpColor focus_border_color;
} VaxpInputStyle;

/**
 * @brief Scrollbar styling
 */
typedef struct VaxpScrollbarStyle {
    VaxpF32 width;
    VaxpF32 thumb_radius;
    VaxpColor track_color;
    VaxpColor thumb_color;
    VaxpColor thumb_hover_color;
} VaxpScrollbarStyle;

/* ============================================================================
 * THEME STRUCTURE
 * ============================================================================ */

/**
 * @brief Complete theme definition
 */
typedef struct VaxpTheme {
    const char* name;
    VaxpBool is_dark;
    
    VaxpColorPalette colors;
    VaxpTypography typography;
    
    /* Component styles */
    VaxpButtonStyle button;
    VaxpInputStyle input;
    VaxpScrollbarStyle scrollbar;
    
    /* Spacing */
    VaxpF32 spacing_xs;    /* 4px */
    VaxpF32 spacing_sm;    /* 8px */
    VaxpF32 spacing_md;    /* 16px */
    VaxpF32 spacing_lg;    /* 24px */
    VaxpF32 spacing_xl;    /* 32px */
    
    /* Default corner radii */
    VaxpF32 radius_sm;     /* 4px */
    VaxpF32 radius_md;     /* 8px */
    VaxpF32 radius_lg;     /* 16px */
    
    /* Shadows */
    VaxpF32 shadow_elevation_1;
    VaxpF32 shadow_elevation_2;
    VaxpF32 shadow_elevation_3;
    
} VaxpTheme;

/* ============================================================================
 * THEME API
 * ============================================================================ */

/**
 * @brief Get the current global theme
 */
const VaxpTheme* vaxp_theme_get_current(void);

/**
 * @brief Set the global theme
 */
void vaxp_theme_set(const VaxpTheme* theme);

/**
 * @brief Get the light theme preset
 */
const VaxpTheme* vaxp_theme_light(void);

/**
 * @brief Get the dark theme preset
 */
const VaxpTheme* vaxp_theme_dark(void);

/**
 * @brief Create a custom theme based on colors
 */
VaxpTheme vaxp_theme_create(VaxpColor primary, VaxpColor secondary, VaxpBool is_dark);

/* ============================================================================
 * HELPER COLORS
 * ============================================================================ */

/* Pre-defined colors for easy use */
#define VAXP_COLOR_TRANSPARENT ((VaxpColor){ 0, 0, 0, 0 })
#define VAXP_COLOR_WHITE       ((VaxpColor){ 255, 255, 255, 255 })
#define VAXP_COLOR_BLACK       ((VaxpColor){ 0, 0, 0, 255 })
#define VAXP_COLOR_GRAY_50     ((VaxpColor){ 250, 250, 250, 255 })
#define VAXP_COLOR_GRAY_100    ((VaxpColor){ 245, 245, 245, 255 })
#define VAXP_COLOR_GRAY_200    ((VaxpColor){ 238, 238, 238, 255 })
#define VAXP_COLOR_GRAY_300    ((VaxpColor){ 224, 224, 224, 255 })
#define VAXP_COLOR_GRAY_400    ((VaxpColor){ 189, 189, 189, 255 })
#define VAXP_COLOR_GRAY_500    ((VaxpColor){ 158, 158, 158, 255 })
#define VAXP_COLOR_GRAY_600    ((VaxpColor){ 117, 117, 117, 255 })
#define VAXP_COLOR_GRAY_700    ((VaxpColor){ 97, 97, 97, 255 })
#define VAXP_COLOR_GRAY_800    ((VaxpColor){ 66, 66, 66, 255 })
#define VAXP_COLOR_GRAY_900    ((VaxpColor){ 33, 33, 33, 255 })

/* Material Design colors */
#define VAXP_COLOR_BLUE        ((VaxpColor){ 33, 150, 243, 255 })
#define VAXP_COLOR_INDIGO      ((VaxpColor){ 63, 81, 181, 255 })
#define VAXP_COLOR_PURPLE      ((VaxpColor){ 156, 39, 176, 255 })
#define VAXP_COLOR_PINK        ((VaxpColor){ 233, 30, 99, 255 })
#define VAXP_COLOR_RED         ((VaxpColor){ 244, 67, 54, 255 })
#define VAXP_COLOR_ORANGE      ((VaxpColor){ 255, 152, 0, 255 })
#define VAXP_COLOR_YELLOW      ((VaxpColor){ 255, 235, 59, 255 })
#define VAXP_COLOR_GREEN       ((VaxpColor){ 76, 175, 80, 255 })
#define VAXP_COLOR_TEAL        ((VaxpColor){ 0, 150, 136, 255 })
#define VAXP_COLOR_CYAN        ((VaxpColor){ 0, 188, 212, 255 })

#ifdef __cplusplus
}
#endif

#endif /* VAXP_THEME_H */
