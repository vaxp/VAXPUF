/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_theme.c - Theming system implementation
 */

#include "vaxp/core/vaxp_theme.h"
#include <string.h>

/* ============================================================================
 * DEFAULT THEMES
 * ============================================================================ */

static const VaxpTheme light_theme = {
    .name = "Light",
    .is_dark = VAXP_FALSE,
    
    .colors = {
        /* Primary */
        .primary = { 63, 81, 181, 255 },           /* Indigo */
        .primary_variant = { 48, 63, 159, 255 },
        .on_primary = { 255, 255, 255, 255 },
        
        /* Secondary */
        .secondary = { 0, 150, 136, 255 },          /* Teal */
        .secondary_variant = { 0, 121, 107, 255 },
        .on_secondary = { 255, 255, 255, 255 },
        
        /* Background */
        .background = { 250, 250, 250, 255 },
        .surface = { 255, 255, 255, 255 },
        .on_background = { 33, 33, 33, 255 },
        .on_surface = { 33, 33, 33, 255 },
        
        /* Status */
        .error = { 176, 0, 32, 255 },
        .on_error = { 255, 255, 255, 255 },
        .success = { 76, 175, 80, 255 },
        .warning = { 255, 152, 0, 255 },
        .info = { 33, 150, 243, 255 },
        
        /* UI */
        .divider = { 224, 224, 224, 255 },
        .disabled = { 158, 158, 158, 255 },
        .shadow = { 0, 0, 0, 40 },
    },
    
    .typography = {
        .h1 = { "Noto Sans", 96.0f, 1.1f, -1.5f, VAXP_FALSE, VAXP_FALSE },
        .h2 = { "Noto Sans", 60.0f, 1.1f, -0.5f, VAXP_FALSE, VAXP_FALSE },
        .h3 = { "Noto Sans", 48.0f, 1.2f, 0, VAXP_FALSE, VAXP_FALSE },
        .h4 = { "Noto Sans", 34.0f, 1.2f, 0.25f, VAXP_FALSE, VAXP_FALSE },
        .h5 = { "Noto Sans", 24.0f, 1.3f, 0, VAXP_FALSE, VAXP_FALSE },
        .h6 = { "Noto Sans", 20.0f, 1.3f, 0.15f, VAXP_TRUE, VAXP_FALSE },
        .body1 = { "Noto Sans", 16.0f, 1.5f, 0.5f, VAXP_FALSE, VAXP_FALSE },
        .body2 = { "Noto Sans", 14.0f, 1.5f, 0.25f, VAXP_FALSE, VAXP_FALSE },
        .caption = { "Noto Sans", 12.0f, 1.4f, 0.4f, VAXP_FALSE, VAXP_FALSE },
        .button = { "Noto Sans", 14.0f, 1.25f, 1.25f, VAXP_TRUE, VAXP_FALSE },
        .overline = { "Noto Sans", 10.0f, 1.5f, 2.0f, VAXP_TRUE, VAXP_FALSE },
    },
    
    .button = {
        .corner_radius = 8.0f,
        .padding_horizontal = 16.0f,
        .padding_vertical = 8.0f,
        .border_width = 0,
        .text_color = { 255, 255, 255, 255 },
        .background_color = { 63, 81, 181, 255 },
        .hover_color = { 48, 63, 159, 255 },
        .active_color = { 26, 35, 126, 255 },
        .border_color = { 0, 0, 0, 0 },
    },
    
    .input = {
        .corner_radius = 4.0f,
        .padding_horizontal = 12.0f,
        .padding_vertical = 8.0f,
        .border_width = 1.0f,
        .text_color = { 33, 33, 33, 255 },
        .placeholder_color = { 158, 158, 158, 255 },
        .background_color = { 255, 255, 255, 255 },
        .border_color = { 189, 189, 189, 255 },
        .focus_border_color = { 63, 81, 181, 255 },
    },
    
    .scrollbar = {
        .width = 10.0f,
        .thumb_radius = 5.0f,
        .track_color = { 238, 238, 238, 255 },
        .thumb_color = { 189, 189, 189, 255 },
        .thumb_hover_color = { 158, 158, 158, 255 },
    },
    
    /* Spacing */
    .spacing_xs = 4.0f,
    .spacing_sm = 8.0f,
    .spacing_md = 16.0f,
    .spacing_lg = 24.0f,
    .spacing_xl = 32.0f,
    
    /* Radii */
    .radius_sm = 4.0f,
    .radius_md = 8.0f,
    .radius_lg = 16.0f,
    
    /* Shadows */
    .shadow_elevation_1 = 2.0f,
    .shadow_elevation_2 = 4.0f,
    .shadow_elevation_3 = 8.0f,
};

static const VaxpTheme dark_theme = {
    .name = "Dark",
    .is_dark = VAXP_TRUE,
    
    .colors = {
        /* Primary */
        .primary = { 100, 181, 246, 255 },          /* Light Blue */
        .primary_variant = { 144, 202, 249, 255 },
        .on_primary = { 0, 0, 0, 255 },
        
        /* Secondary */
        .secondary = { 128, 203, 196, 255 },        /* Light Teal */
        .secondary_variant = { 178, 223, 219, 255 },
        .on_secondary = { 0, 0, 0, 255 },
        
        /* Background */
        .background = { 18, 18, 18, 255 },
        .surface = { 30, 30, 30, 255 },
        .on_background = { 255, 255, 255, 255 },
        .on_surface = { 255, 255, 255, 255 },
        
        /* Status */
        .error = { 239, 83, 80, 255 },
        .on_error = { 0, 0, 0, 255 },
        .success = { 129, 199, 132, 255 },
        .warning = { 255, 183, 77, 255 },
        .info = { 100, 181, 246, 255 },
        
        /* UI */
        .divider = { 66, 66, 66, 255 },
        .disabled = { 97, 97, 97, 255 },
        .shadow = { 0, 0, 0, 80 },
    },
    
    .typography = {
        .h1 = { "Noto Sans", 96.0f, 1.1f, -1.5f, VAXP_FALSE, VAXP_FALSE },
        .h2 = { "Noto Sans", 60.0f, 1.1f, -0.5f, VAXP_FALSE, VAXP_FALSE },
        .h3 = { "Noto Sans", 48.0f, 1.2f, 0, VAXP_FALSE, VAXP_FALSE },
        .h4 = { "Noto Sans", 34.0f, 1.2f, 0.25f, VAXP_FALSE, VAXP_FALSE },
        .h5 = { "Noto Sans", 24.0f, 1.3f, 0, VAXP_FALSE, VAXP_FALSE },
        .h6 = { "Noto Sans", 20.0f, 1.3f, 0.15f, VAXP_TRUE, VAXP_FALSE },
        .body1 = { "Noto Sans", 16.0f, 1.5f, 0.5f, VAXP_FALSE, VAXP_FALSE },
        .body2 = { "Noto Sans", 14.0f, 1.5f, 0.25f, VAXP_FALSE, VAXP_FALSE },
        .caption = { "Noto Sans", 12.0f, 1.4f, 0.4f, VAXP_FALSE, VAXP_FALSE },
        .button = { "Noto Sans", 14.0f, 1.25f, 1.25f, VAXP_TRUE, VAXP_FALSE },
        .overline = { "Noto Sans", 10.0f, 1.5f, 2.0f, VAXP_TRUE, VAXP_FALSE },
    },
    
    .button = {
        .corner_radius = 8.0f,
        .padding_horizontal = 16.0f,
        .padding_vertical = 8.0f,
        .border_width = 0,
        .text_color = { 0, 0, 0, 255 },
        .background_color = { 100, 181, 246, 255 },
        .hover_color = { 144, 202, 249, 255 },
        .active_color = { 66, 165, 245, 255 },
        .border_color = { 0, 0, 0, 0 },
    },
    
    .input = {
        .corner_radius = 4.0f,
        .padding_horizontal = 12.0f,
        .padding_vertical = 8.0f,
        .border_width = 1.0f,
        .text_color = { 255, 255, 255, 255 },
        .placeholder_color = { 117, 117, 117, 255 },
        .background_color = { 30, 30, 30, 255 },
        .border_color = { 66, 66, 66, 255 },
        .focus_border_color = { 100, 181, 246, 255 },
    },
    
    .scrollbar = {
        .width = 10.0f,
        .thumb_radius = 5.0f,
        .track_color = { 42, 42, 42, 255 },
        .thumb_color = { 97, 97, 97, 255 },
        .thumb_hover_color = { 117, 117, 117, 255 },
    },
    
    /* Spacing */
    .spacing_xs = 4.0f,
    .spacing_sm = 8.0f,
    .spacing_md = 16.0f,
    .spacing_lg = 24.0f,
    .spacing_xl = 32.0f,
    
    /* Radii */
    .radius_sm = 4.0f,
    .radius_md = 8.0f,
    .radius_lg = 16.0f,
    
    /* Shadows */
    .shadow_elevation_1 = 2.0f,
    .shadow_elevation_2 = 4.0f,
    .shadow_elevation_3 = 8.0f,
};

/* ============================================================================
 * GLOBAL THEME STATE
 * ============================================================================ */

static const VaxpTheme* current_theme = &light_theme;

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

const VaxpTheme* vaxp_theme_get_current(void) {
    return current_theme;
}

void vaxp_theme_set(const VaxpTheme* theme) {
    if (theme) {
        current_theme = theme;
    }
}

const VaxpTheme* vaxp_theme_light(void) {
    return &light_theme;
}

const VaxpTheme* vaxp_theme_dark(void) {
    return &dark_theme;
}

VaxpTheme vaxp_theme_create(VaxpColor primary, VaxpColor secondary, VaxpBool is_dark) {
    VaxpTheme theme = is_dark ? dark_theme : light_theme;
    
    theme.name = "Custom";
    theme.colors.primary = primary;
    theme.colors.secondary = secondary;
    
    /* Update button color to match primary */
    theme.button.background_color = primary;
    theme.input.focus_border_color = primary;
    
    return theme;
}
