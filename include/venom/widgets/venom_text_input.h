/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_text_input.h - Text input widget for user text entry
 */

#ifndef VENOM_TEXT_INPUT_H
#define VENOM_TEXT_INPUT_H

#include "venom/widgets/venom_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * TEXT INPUT TYPES
 * ============================================================================ */

/* Forward declaration */
typedef struct VenomTextInput VenomTextInput;

/**
 * @brief Text change callback
 */
typedef void (*VenomTextInputCallback)(VenomTextInput* input, const char* text, void* user_data);

/**
 * @brief Submit callback (Enter pressed)
 */
typedef void (*VenomTextInputSubmitCallback)(VenomTextInput* input, const char* text, void* user_data);

/**
 * @brief Text input widget
 */
struct VenomTextInput {
    VenomWidget base;
    
    char* text;                     /* Current text content */
    VenomU32 text_len;              /* Length of text */
    VenomU32 text_capacity;         /* Allocated capacity */
    
    VenomU32 cursor_pos;            /* Cursor position (index) */
    VenomU32 selection_start;       /* Selection start (index) */
    VenomU32 selection_end;         /* Selection end (index) */
    
    char* placeholder;              /* Placeholder text */
    VenomU32 max_length;            /* Max characters (0 = unlimited) */
    
    /* Styling */
    VenomColor text_color;
    VenomColor placeholder_color;
    VenomColor bg_color;
    VenomColor border_color;
    VenomColor focus_border_color;
    VenomF32 font_size;
    VenomF32 corner_radius;
    VenomF32 border_width;
    
    /* Callbacks */
    VenomTextInputCallback on_change;
    VenomTextInputSubmitCallback on_submit;
    void* callback_data;
    
    /* Internal state */
    VenomBool password_mode;        /* Hide text with dots */
    VenomBool editable;             /* Whether editing is allowed */
    VenomF32 scroll_offset;         /* Horizontal scroll for long text */
    VenomU64 cursor_blink_time;     /* For cursor animation */
};

/* ============================================================================
 * TEXT INPUT API
 * ============================================================================ */

/**
 * @brief Create a text input widget
 */
VenomResultPtr venom_text_input_create(void);

/**
 * @brief Set text content
 */
VenomResult venom_text_input_set_text(VenomTextInput* input, const char* text);

/**
 * @brief Get text content
 */
const char* venom_text_input_get_text(const VenomTextInput* input);

/**
 * @brief Set placeholder text
 */
VenomResult venom_text_input_set_placeholder(VenomTextInput* input, const char* placeholder);

/**
 * @brief Set max length (0 = unlimited)
 */
void venom_text_input_set_max_length(VenomTextInput* input, VenomU32 max_length);

/**
 * @brief Set password mode (hide characters)
 */
void venom_text_input_set_password(VenomTextInput* input, VenomBool password);

/**
 * @brief Set editable state
 */
void venom_text_input_set_editable(VenomTextInput* input, VenomBool editable);

/**
 * @brief Set callbacks
 */
void venom_text_input_set_on_change(VenomTextInput* input, VenomTextInputCallback callback, void* user_data);
void venom_text_input_set_on_submit(VenomTextInput* input, VenomTextInputSubmitCallback callback, void* user_data);

/**
 * @brief Set colors
 */
void venom_text_input_set_colors(VenomTextInput* input, VenomColor text, VenomColor bg, VenomColor border);

/**
 * @brief Clear text
 */
void venom_text_input_clear(VenomTextInput* input);

/**
 * @brief Select all text
 */
void venom_text_input_select_all(VenomTextInput* input);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VenomWidgetClass venom_text_input_class;

/* ============================================================================
 * CONVENIENCE MACRO
 * ============================================================================ */

/**
 * @brief Create text input with configuration
 */
#define venom_input(...) \
    _venom_text_input_build(&(VenomTextInputConfig){ __VA_ARGS__ })

typedef struct VenomTextInputConfig {
    const char* placeholder;
    const char* initial_text;
    VenomU32 max_length;
    VenomBool password;
    VenomTextInputCallback on_change;
    VenomTextInputSubmitCallback on_submit;
    void* callback_data;
} VenomTextInputConfig;

VenomWidget* _venom_text_input_build(const VenomTextInputConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_TEXT_INPUT_H */
