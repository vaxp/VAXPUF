/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_button.h - Button widget
 */

#ifndef VAXP_BUTTON_H
#define VAXP_BUTTON_H

#include "vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * BUTTON TYPES
 * ============================================================================ */

typedef struct VaxpButton VaxpButton;

typedef void (*VaxpButtonCallback)(VaxpButton* button, void* user_data);

struct VaxpButton {
    VaxpWidget base;
    
    char* label;            /* Button text (owned) */
    char* font_family;
    VaxpButtonCallback on_click;
    void* callback_data;
    
    /* Style */
    VaxpColor bg_color;
    VaxpColor bg_hover_color;
    VaxpColor bg_pressed_color;
    VaxpColor text_color;
    VaxpF32 corner_radius;
    VaxpF32 font_size;
};

/* ============================================================================
 * BUTTON API
 * ============================================================================ */

/**
 * @brief Create a button with text
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_button_create(const char* label);

/**
 * @brief Set button text
 */
VaxpResult vaxp_button_set_label(VaxpButton* button, const char* label);

/**
 * @brief Set font family
 */
void vaxp_button_set_font_family(VaxpButton* button, const char* family);

/**
 * @brief Get button text
 */
const char* vaxp_button_get_label(const VaxpButton* button);

/**
 * @brief Set click callback
 */
void vaxp_button_set_on_click(VaxpButton* button, VaxpButtonCallback callback, void* user_data);

/**
 * @brief Set button colors
 */
void vaxp_button_set_colors(VaxpButton* button, VaxpColor bg, VaxpColor hover, 
                              VaxpColor pressed, VaxpColor text);

/**
 * @brief Set corner radius
 */
void vaxp_button_set_corner_radius(VaxpButton* button, VaxpF32 radius);

/* Button class */
extern const VaxpWidgetClass vaxp_button_class;

#ifdef __cplusplus
}
#endif

#endif /* VAXP_BUTTON_H */
