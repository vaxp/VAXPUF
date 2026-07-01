/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_text_input.h - Text input widget for user text entry
 */

#ifndef VAXP_TEXT_INPUT_H
#define VAXP_TEXT_INPUT_H

#include "vaxp/widgets/vaxp_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * TEXT INPUT TYPES
 * ============================================================================ */

/* Forward declaration */
typedef struct VaxpTextInput VaxpTextInput;

/**
 * @brief Text change callback
 */
typedef void (*VaxpTextInputCallback)(VaxpTextInput* input, const char* text, void* user_data);

/**
 * @brief Submit callback (Enter pressed)
 */
typedef void (*VaxpTextInputSubmitCallback)(VaxpTextInput* input, const char* text, void* user_data);

/**
 * @brief Text input widget
 */
struct VaxpTextInput {
    VaxpWidget base;
    
    char* text;                     /* Current text content */
    VaxpU32 text_len;              /* Length of text */
    VaxpU32 text_capacity;         /* Allocated capacity */
    
    VaxpU32 cursor_pos;            /* Cursor position (index) */
    VaxpU32 selection_start;       /* Selection start (index) */
    VaxpU32 selection_end;         /* Selection end (index) */
    
    char* placeholder;              /* Placeholder text */
    VaxpU32 max_length;            /* Max characters (0 = unlimited) */
    
    /* Styling */
    VaxpColor text_color;
    VaxpColor placeholder_color;
    VaxpColor bg_color;
    VaxpColor border_color;
    VaxpColor focus_border_color;
    VaxpF32 font_size;
    VaxpF32 corner_radius;
    VaxpF32 border_width;
    
    /* Callbacks */
    VaxpTextInputCallback on_change;
    VaxpTextInputSubmitCallback on_submit;
    void* callback_data;
    
    /* Internal state */
    VaxpBool password_mode;        /* Hide text with dots */
    VaxpBool editable;             /* Whether editing is allowed */
    VaxpF32 scroll_offset;         /* Horizontal scroll for long text */
    VaxpU64 cursor_blink_time;     /* For cursor animation */
};

/* ============================================================================
 * TEXT INPUT API
 * ============================================================================ */

/**
 * @brief Create a text input widget
 */
VaxpResultPtr vaxp_text_input_create(void);

/**
 * @brief Set text content
 */
VaxpResult vaxp_text_input_set_text(VaxpTextInput* input, const char* text);

/**
 * @brief Get text content
 */
const char* vaxp_text_input_get_text(const VaxpTextInput* input);

/**
 * @brief Set placeholder text
 */
VaxpResult vaxp_text_input_set_placeholder(VaxpTextInput* input, const char* placeholder);

/**
 * @brief Set max length (0 = unlimited)
 */
void vaxp_text_input_set_max_length(VaxpTextInput* input, VaxpU32 max_length);

/**
 * @brief Set password mode (hide characters)
 */
void vaxp_text_input_set_password(VaxpTextInput* input, VaxpBool password);

/**
 * @brief Set editable state
 */
void vaxp_text_input_set_editable(VaxpTextInput* input, VaxpBool editable);

/**
 * @brief Set callbacks
 */
void vaxp_text_input_set_on_change(VaxpTextInput* input, VaxpTextInputCallback callback, void* user_data);
void vaxp_text_input_set_on_submit(VaxpTextInput* input, VaxpTextInputSubmitCallback callback, void* user_data);

/**
 * @brief Set colors
 */
void vaxp_text_input_set_colors(VaxpTextInput* input, VaxpColor text, VaxpColor bg, VaxpColor border);

/**
 * @brief Clear text
 */
void vaxp_text_input_clear(VaxpTextInput* input);

/**
 * @brief Select all text
 */
void vaxp_text_input_select_all(VaxpTextInput* input);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VaxpWidgetClass vaxp_text_input_class;

/* ============================================================================
 * CONVENIENCE MACRO
 * ============================================================================ */

/**
 * @brief Create text input with configuration
 */
#define vaxp_input(...) \
    _vaxp_text_input_build(&(VaxpTextInputConfig){ __VA_ARGS__ })

typedef struct VaxpTextInputConfig {
    const char* placeholder;
    const char* initial_text;
    VaxpU32 max_length;
    VaxpBool password;
    VaxpTextInputCallback on_change;
    VaxpTextInputSubmitCallback on_submit;
    void* callback_data;
} VaxpTextInputConfig;

VaxpWidget* _vaxp_text_input_build(const VaxpTextInputConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_TEXT_INPUT_H */
