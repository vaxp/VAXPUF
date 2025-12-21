/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_label.h - Label widget for text display
 */

#ifndef VENOM_LABEL_H
#define VENOM_LABEL_H

#include "venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * LABEL TYPES
 * ============================================================================ */

typedef enum VenomTextAlign {
    VENOM_TEXT_ALIGN_LEFT,
    VENOM_TEXT_ALIGN_CENTER,
    VENOM_TEXT_ALIGN_RIGHT,
} VenomTextAlign;

typedef struct VenomLabel {
    VenomWidget base;
    
    char* text;             /* Label text (owned) */
    VenomColor text_color;
    VenomF32 font_size;
    VenomTextAlign align;
    VenomBool wrap;         /* Wrap text to multiple lines */
} VenomLabel;

/* ============================================================================
 * LABEL API
 * ============================================================================ */

/**
 * @brief Create a label with text
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_label_create(const char* text);

/**
 * @brief Set label text
 */
VenomResult venom_label_set_text(VenomLabel* label, const char* text);

/**
 * @brief Get label text
 */
const char* venom_label_get_text(const VenomLabel* label);

/**
 * @brief Set text color
 */
void venom_label_set_color(VenomLabel* label, VenomColor color);

/**
 * @brief Set font size
 */
void venom_label_set_font_size(VenomLabel* label, VenomF32 size);

/**
 * @brief Set text alignment
 */
void venom_label_set_align(VenomLabel* label, VenomTextAlign align);

/* Label class */
extern const VenomWidgetClass venom_label_class;

#ifdef __cplusplus
}
#endif

#endif /* VENOM_LABEL_H */
