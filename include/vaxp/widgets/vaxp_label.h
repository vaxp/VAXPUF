/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_label.h - Label widget for text display
 */

#ifndef VAXP_LABEL_H
#define VAXP_LABEL_H

#include "vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * LABEL TYPES
 * ============================================================================ */

typedef enum VaxpTextAlign {
    VAXP_TEXT_ALIGN_LEFT,
    VAXP_TEXT_ALIGN_CENTER,
    VAXP_TEXT_ALIGN_RIGHT,
} VaxpTextAlign;

typedef struct VaxpLabel {
    VaxpWidget base;
    
    char* text;             /* Label text (owned) */
    char* font_family;
    VaxpColor text_color;
    VaxpF32 font_size;
    VaxpTextAlign align;
    VaxpBool wrap;         /* Wrap text to multiple lines */
} VaxpLabel;

/* ============================================================================
 * LABEL API
 * ============================================================================ */

/**
 * @brief Create a label with text
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_label_create(const char* text);

/**
 * @brief Set label text
 */
VaxpResult vaxp_label_set_text(VaxpLabel* label, const char* text);

/**
 * @brief Get label text
 */
const char* vaxp_label_get_text(const VaxpLabel* label);

/**
 * @brief Set text color
 */
void vaxp_label_set_color(VaxpLabel* label, VaxpColor color);

/**
 * @brief Set font family
 */
void vaxp_label_set_font_family(VaxpLabel* label, const char* family);

/**
 * @brief Set font size
 */
void vaxp_label_set_font_size(VaxpLabel* label, VaxpF32 size);

/**
 * @brief Set text alignment
 */
void vaxp_label_set_align(VaxpLabel* label, VaxpTextAlign align);

/* Label class */
extern const VaxpWidgetClass vaxp_label_class;

#ifdef __cplusplus
}
#endif

#endif /* VAXP_LABEL_H */
