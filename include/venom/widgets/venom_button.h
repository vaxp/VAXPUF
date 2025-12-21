/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_button.h - Button widget
 */

#ifndef VENOM_BUTTON_H
#define VENOM_BUTTON_H

#include "venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * BUTTON TYPES
 * ============================================================================ */

typedef struct VenomButton VenomButton;

typedef void (*VenomButtonCallback)(VenomButton* button, void* user_data);

struct VenomButton {
    VenomWidget base;
    
    char* label;            /* Button text (owned) */
    VenomButtonCallback on_click;
    void* callback_data;
    
    /* Style */
    VenomColor bg_color;
    VenomColor bg_hover_color;
    VenomColor bg_pressed_color;
    VenomColor text_color;
    VenomF32 corner_radius;
    VenomF32 font_size;
};

/* ============================================================================
 * BUTTON API
 * ============================================================================ */

/**
 * @brief Create a button with text
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_button_create(const char* label);

/**
 * @brief Set button text
 */
VenomResult venom_button_set_label(VenomButton* button, const char* label);

/**
 * @brief Get button text
 */
const char* venom_button_get_label(const VenomButton* button);

/**
 * @brief Set click callback
 */
void venom_button_set_on_click(VenomButton* button, VenomButtonCallback callback, void* user_data);

/**
 * @brief Set button colors
 */
void venom_button_set_colors(VenomButton* button, VenomColor bg, VenomColor hover, 
                              VenomColor pressed, VenomColor text);

/**
 * @brief Set corner radius
 */
void venom_button_set_corner_radius(VenomButton* button, VenomF32 radius);

/* Button class */
extern const VenomWidgetClass venom_button_class;

#ifdef __cplusplus
}
#endif

#endif /* VENOM_BUTTON_H */
