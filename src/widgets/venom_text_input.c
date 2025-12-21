/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_text_input.c - Text input widget implementation
 */

#include "venom/widgets/venom_text_input.h"
#include "venom/core/venom_memory.h"
#include "venom/core/venom_focus.h"
#include <string.h>
#include <time.h>

/* ============================================================================
 * DEFAULT COLORS
 * ============================================================================ */

static const VenomColor DEFAULT_TEXT = { 30, 30, 30, 255 };
static const VenomColor DEFAULT_PLACEHOLDER = { 150, 150, 150, 255 };
static const VenomColor DEFAULT_BG = { 255, 255, 255, 255 };
static const VenomColor DEFAULT_BORDER = { 180, 180, 180, 255 };
static const VenomColor DEFAULT_FOCUS_BORDER = { 100, 150, 255, 255 };

#define INITIAL_CAPACITY 64
#define CURSOR_BLINK_MS 500

/* ============================================================================
 * TEXT INPUT CLASS METHODS
 * ============================================================================ */

static void text_input_init(VenomWidget* widget) {
    VenomTextInput* input = (VenomTextInput*)widget;
    
    /* Default styling */
    input->text_color = DEFAULT_TEXT;
    input->placeholder_color = DEFAULT_PLACEHOLDER;
    input->bg_color = DEFAULT_BG;
    input->border_color = DEFAULT_BORDER;
    input->focus_border_color = DEFAULT_FOCUS_BORDER;
    input->font_size = 14.0f;
    input->corner_radius = 4.0f;
    input->border_width = 1.0f;
    
    /* State */
    input->editable = VENOM_TRUE;
    input->password_mode = VENOM_FALSE;
    input->max_length = 0;  /* Unlimited */
    
    /* Allocate initial text buffer */
    input->text = (char*)venom_alloc(INITIAL_CAPACITY);
    if (input->text) {
        input->text[0] = '\0';
        input->text_capacity = INITIAL_CAPACITY;
    }
    input->text_len = 0;
    input->cursor_pos = 0;
    
    /* Layout defaults */
    widget->layout.min_width = 100.0f;
    widget->layout.min_height = 32.0f;
    widget->layout.padding = (VenomInsets){ 8, 12, 8, 12 };
    
    /* Focusable */
    widget->focusable = VENOM_TRUE;
}

static void text_input_destroy(VenomWidget* widget) {
    VenomTextInput* input = (VenomTextInput*)widget;
    
    if (input->text) {
        venom_free(input->text, input->text_capacity);
        input->text = NULL;
    }
    if (input->placeholder) {
        venom_free(input->placeholder, strlen(input->placeholder) + 1);
        input->placeholder = NULL;
    }
    
    /* Call parent destroy */
    venom_widget_class.destroy(widget);
}

static void text_input_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                                VenomF32* out_width, VenomF32* out_height) {
    (void)available_width;
    (void)available_height;
    
    VenomTextInput* input = (VenomTextInput*)widget;
    
    *out_height = input->font_size * 1.5f + 
                  widget->layout.padding.top + widget->layout.padding.bottom;
    *out_width = widget->layout.min_width;
    
    /* Apply constraints */
    if (*out_width < widget->layout.min_width) *out_width = widget->layout.min_width;
    if (*out_height < widget->layout.min_height) *out_height = widget->layout.min_height;
}

static void text_input_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomTextInput* input = (VenomTextInput*)widget;
    VenomBool has_focus = venom_widget_has_state(widget, VENOM_WIDGET_STATE_FOCUSED);
    
    /* Draw background */
    VenomRectF rect = { 0, 0, widget->bounds.width, widget->bounds.height };
    VenomPaint bg_paint = venom_paint_fill(input->bg_color);
    venom_canvas_draw_rounded_rect(canvas, rect, input->corner_radius, &bg_paint);
    
    /* Draw border */
    VenomColor border = has_focus ? input->focus_border_color : input->border_color;
    VenomPaint border_paint = venom_paint_stroke(border, input->border_width);
    venom_canvas_draw_rounded_rect(canvas, rect, input->corner_radius, &border_paint);
    
    /* Calculate text area */
    VenomF32 text_x = widget->layout.padding.left;
    VenomF32 text_y = (widget->bounds.height + input->font_size * 0.35f) / 2;
    
    /* Draw text or placeholder */
    const char* display_text;
    VenomColor text_color;
    
    if (input->text_len > 0) {
        if (input->password_mode) {
            /* Create dots for password */
            static char dots[256];
            VenomU32 len = input->text_len < 255 ? input->text_len : 255;
            for (VenomU32 i = 0; i < len; i++) dots[i] = '*';
            dots[len] = '\0';
            display_text = dots;
        } else {
            display_text = input->text;
        }
        text_color = input->text_color;
    } else if (input->placeholder) {
        display_text = input->placeholder;
        text_color = input->placeholder_color;
    } else {
        display_text = NULL;
    }
    
    if (display_text) {
        VenomPaint text_paint = venom_paint_fill(text_color);
        venom_canvas_draw_text(canvas, display_text, text_x, text_y, NULL, &text_paint);
    }
    
    /* Draw cursor if focused */
    if (has_focus && input->editable) {
        /* Calculate cursor position */
        VenomF32 cursor_x = text_x;
        if (input->text_len > 0 && input->cursor_pos > 0) {
            VenomU32 chars_before = input->cursor_pos < input->text_len ? 
                                    input->cursor_pos : input->text_len;
            cursor_x += chars_before * (input->font_size * 0.5f);
        }
        
        /* Blinking cursor */
        VenomU64 now_ms = (VenomU64)(clock() * 1000 / CLOCKS_PER_SEC);
        if ((now_ms / CURSOR_BLINK_MS) % 2 == 0) {
            VenomF32 cursor_y = (widget->bounds.height - input->font_size) / 2;
            VenomRectF cursor_rect = { cursor_x, cursor_y, 2, input->font_size };
            VenomPaint cursor_paint = venom_paint_fill(input->text_color);
            venom_canvas_draw_rect(canvas, cursor_rect, &cursor_paint);
        }
    }
}

static VenomBool text_input_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomTextInput* input = (VenomTextInput*)widget;
    
    if (!input->editable) return VENOM_FALSE;
    
    switch (event->type) {
        case VENOM_EVENT_MOUSE_BUTTON_DOWN:
            /* Request focus on click */
            venom_focus_set(widget);
            return VENOM_TRUE;
            
        case VENOM_EVENT_KEY_DOWN:
            if (!venom_widget_has_state(widget, VENOM_WIDGET_STATE_FOCUSED)) {
                return VENOM_FALSE;
            }
            
            switch (event->key.key) {
                case VENOM_KEY_BACKSPACE:
                    if (input->cursor_pos > 0 && input->text_len > 0) {
                        /* Find start of previous UTF-8 character */
                        VenomU32 delete_pos = input->cursor_pos - 1;
                        
                        /* UTF-8: continuation bytes have form 10xxxxxx (0x80-0xBF) */
                        /* Go back until we find a leading byte (not 10xxxxxx) */
                        while (delete_pos > 0 && 
                               (input->text[delete_pos] & 0xC0) == 0x80) {
                            delete_pos--;
                        }
                        
                        VenomU32 char_bytes = input->cursor_pos - delete_pos;
                        
                        /* Delete the full UTF-8 character */
                        memmove(&input->text[delete_pos],
                                &input->text[input->cursor_pos],
                                input->text_len - input->cursor_pos + 1);
                        input->cursor_pos = delete_pos;
                        input->text_len -= char_bytes;
                        
                        if (input->on_change) {
                            input->on_change(input, input->text, input->callback_data);
                        }
                        widget->needs_redraw = VENOM_TRUE;
                    }
                    return VENOM_TRUE;
                    
                case VENOM_KEY_DELETE:
                    if (input->cursor_pos < input->text_len) {
                        /* Delete character at cursor */
                        memmove(&input->text[input->cursor_pos],
                                &input->text[input->cursor_pos + 1],
                                input->text_len - input->cursor_pos);
                        input->text_len--;
                        if (input->on_change) {
                            input->on_change(input, input->text, input->callback_data);
                        }
                        widget->needs_redraw = VENOM_TRUE;
                    }
                    return VENOM_TRUE;
                    
                case VENOM_KEY_LEFT:
                    if (input->cursor_pos > 0) {
                        input->cursor_pos--;
                        widget->needs_redraw = VENOM_TRUE;
                    }
                    return VENOM_TRUE;
                    
                case VENOM_KEY_RIGHT:
                    if (input->cursor_pos < input->text_len) {
                        input->cursor_pos++;
                        widget->needs_redraw = VENOM_TRUE;
                    }
                    return VENOM_TRUE;
                    
                case VENOM_KEY_HOME:
                    input->cursor_pos = 0;
                    widget->needs_redraw = VENOM_TRUE;
                    return VENOM_TRUE;
                    
                case VENOM_KEY_END:
                    input->cursor_pos = input->text_len;
                    widget->needs_redraw = VENOM_TRUE;
                    return VENOM_TRUE;
                    
                case VENOM_KEY_RETURN:
                    if (input->on_submit) {
                        input->on_submit(input, input->text, input->callback_data);
                    }
                    return VENOM_TRUE;
                    
                default: {
                    /* Use UTF-8 text from event (populated by XIM/Xutf8LookupString) */
                    /* This properly handles international characters including Arabic */
                    if (event->text.text[0] != '\0' && (unsigned char)event->text.text[0] >= 32) {
                        VenomU32 char_len = strlen(event->text.text);
                        
                        /* Check max length */
                        if (input->max_length > 0 && input->text_len >= input->max_length) {
                            return VENOM_TRUE;
                        }
                        
                        /* Ensure capacity for UTF-8 (up to 4 bytes per char) */
                        if (input->text_len + char_len >= input->text_capacity) {
                            VenomSize new_cap = input->text_capacity * 2;
                            while (new_cap < input->text_len + char_len + 1) {
                                new_cap *= 2;
                            }
                            char* new_text = (char*)venom_alloc(new_cap);
                            if (!new_text) return VENOM_TRUE;
                            memcpy(new_text, input->text, input->text_len + 1);
                            venom_free(input->text, input->text_capacity);
                            input->text = new_text;
                            input->text_capacity = new_cap;
                        }
                        
                        /* Insert UTF-8 character(s) at cursor */
                        memmove(&input->text[input->cursor_pos + char_len],
                                &input->text[input->cursor_pos],
                                input->text_len - input->cursor_pos + 1);
                        memcpy(&input->text[input->cursor_pos], event->text.text, char_len);
                        input->cursor_pos += char_len;
                        input->text_len += char_len;
                        
                        if (input->on_change) {
                            input->on_change(input, input->text, input->callback_data);
                        }
                        widget->needs_redraw = VENOM_TRUE;
                        return VENOM_TRUE;
                    }
                    break;
                }
            }
            break;
            
        case VENOM_EVENT_TEXT_INPUT:
            /* Add typed character */
            if (event->text.text[0] >= 32 && event->text.text[0] < 127) {
                /* Check max length */
                if (input->max_length > 0 && input->text_len >= input->max_length) {
                    return VENOM_TRUE;
                }
                
                /* Ensure capacity */
                if (input->text_len + 1 >= input->text_capacity) {
                    VenomSize new_cap = input->text_capacity * 2;
                    char* new_text = (char*)venom_alloc(new_cap);
                    if (!new_text) return VENOM_TRUE;
                    memcpy(new_text, input->text, input->text_len + 1);
                    venom_free(input->text, input->text_capacity);
                    input->text = new_text;
                    input->text_capacity = new_cap;
                }
                
                /* Insert character at cursor */
                memmove(&input->text[input->cursor_pos + 1],
                        &input->text[input->cursor_pos],
                        input->text_len - input->cursor_pos + 1);
                input->text[input->cursor_pos] = event->text.text[0];
                input->cursor_pos++;
                input->text_len++;
                
                if (input->on_change) {
                    input->on_change(input, input->text, input->callback_data);
                }
                widget->needs_redraw = VENOM_TRUE;
            }
            return VENOM_TRUE;
            
        default:
            break;
    }
    
    return VENOM_FALSE;
}

/* ============================================================================
 * TEXT INPUT CLASS
 * ============================================================================ */

const VenomWidgetClass venom_text_input_class = {
    .class_name = "VenomTextInput",
    .instance_size = sizeof(VenomTextInput),
    .parent_class = &venom_widget_class,
    .init = text_input_init,
    .destroy = text_input_destroy,
    .measure = text_input_measure,
    .layout = NULL,
    .draw = text_input_draw,
    .on_event = text_input_on_event,
    .on_state_changed = NULL,
};

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VenomResultPtr venom_text_input_create(void) {
    return venom_widget_create(&venom_text_input_class);
}

VenomResult venom_text_input_set_text(VenomTextInput* input, const char* text) {
    VENOM_ENSURE_NOT_NULL(input);
    
    VenomU32 len = text ? (VenomU32)strlen(text) : 0;
    
    /* Ensure capacity */
    if (len + 1 > input->text_capacity) {
        VenomSize new_cap = len + 1;
        char* new_text = (char*)venom_alloc(new_cap);
        if (!new_text) return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
        venom_free(input->text, input->text_capacity);
        input->text = new_text;
        input->text_capacity = new_cap;
    }
    
    if (text) {
        memcpy(input->text, text, len + 1);
    } else {
        input->text[0] = '\0';
    }
    input->text_len = len;
    input->cursor_pos = len;
    
    venom_widget_invalidate((VenomWidget*)input);
    return VENOM_OK_UNIT();
}

const char* venom_text_input_get_text(const VenomTextInput* input) {
    return input ? input->text : "";
}

VenomResult venom_text_input_set_placeholder(VenomTextInput* input, const char* placeholder) {
    VENOM_ENSURE_NOT_NULL(input);
    
    if (input->placeholder) {
        venom_free(input->placeholder, strlen(input->placeholder) + 1);
        input->placeholder = NULL;
    }
    
    if (placeholder) {
        VenomSize len = strlen(placeholder) + 1;
        input->placeholder = (char*)venom_alloc(len);
        if (!input->placeholder) return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
        memcpy(input->placeholder, placeholder, len);
    }
    
    venom_widget_invalidate((VenomWidget*)input);
    return VENOM_OK_UNIT();
}

void venom_text_input_set_max_length(VenomTextInput* input, VenomU32 max_length) {
    if (input) input->max_length = max_length;
}

void venom_text_input_set_password(VenomTextInput* input, VenomBool password) {
    if (input) {
        input->password_mode = password;
        venom_widget_invalidate((VenomWidget*)input);
    }
}

void venom_text_input_set_editable(VenomTextInput* input, VenomBool editable) {
    if (input) input->editable = editable;
}

void venom_text_input_set_on_change(VenomTextInput* input, VenomTextInputCallback callback, void* user_data) {
    if (!input) return;
    input->on_change = callback;
    input->callback_data = user_data;
}

void venom_text_input_set_on_submit(VenomTextInput* input, VenomTextInputSubmitCallback callback, void* user_data) {
    if (!input) return;
    input->on_submit = callback;
    input->callback_data = user_data;
}

void venom_text_input_set_colors(VenomTextInput* input, VenomColor text, VenomColor bg, VenomColor border) {
    if (!input) return;
    input->text_color = text;
    input->bg_color = bg;
    input->border_color = border;
    venom_widget_invalidate((VenomWidget*)input);
}

void venom_text_input_clear(VenomTextInput* input) {
    if (!input) return;
    input->text[0] = '\0';
    input->text_len = 0;
    input->cursor_pos = 0;
    venom_widget_invalidate((VenomWidget*)input);
}

void venom_text_input_select_all(VenomTextInput* input) {
    if (!input) return;
    input->selection_start = 0;
    input->selection_end = input->text_len;
}

/* ============================================================================
 * CONVENIENCE BUILDER
 * ============================================================================ */

VenomWidget* _venom_text_input_build(const VenomTextInputConfig* config) {
    VenomResultPtr result = venom_text_input_create();
    if (!result.ok) return NULL;
    
    VenomTextInput* input = (VenomTextInput*)result.value;
    
    if (config->placeholder) {
        venom_text_input_set_placeholder(input, config->placeholder);
    }
    if (config->initial_text) {
        venom_text_input_set_text(input, config->initial_text);
    }
    if (config->max_length > 0) {
        venom_text_input_set_max_length(input, config->max_length);
    }
    if (config->password) {
        venom_text_input_set_password(input, config->password);
    }
    if (config->on_change) {
        input->on_change = config->on_change;
    }
    if (config->on_submit) {
        input->on_submit = config->on_submit;
    }
    input->callback_data = config->callback_data;
    
    return (VenomWidget*)input;
}
