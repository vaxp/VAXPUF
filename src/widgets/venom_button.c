/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_button.c - Button widget implementation
 */

#include "venom/widgets/venom_button.h"
#include "venom/core/venom_memory.h"
#include <string.h>

/* ============================================================================
 * DEFAULT COLORS
 * ============================================================================ */

static const VenomColor DEFAULT_BG = { 60, 120, 220, 255 };         /* Blue */
static const VenomColor DEFAULT_HOVER = { 80, 140, 240, 255 };      /* Lighter blue */
static const VenomColor DEFAULT_PRESSED = { 40, 100, 200, 255 };    /* Darker blue */
static const VenomColor DEFAULT_TEXT = { 255, 255, 255, 255 };      /* White */

/* ============================================================================
 * BUTTON CLASS METHODS
 * ============================================================================ */

static void button_init(VenomWidget* widget) {
    VenomButton* btn = (VenomButton*)widget;
    
    /* Default styling */
    btn->bg_color = DEFAULT_BG;
    btn->bg_hover_color = DEFAULT_HOVER;
    btn->bg_pressed_color = DEFAULT_PRESSED;
    btn->text_color = DEFAULT_TEXT;
    btn->corner_radius = 6.0f;
    btn->font_size = 14.0f;
    
    /* Default layout */
    widget->layout.padding = (VenomInsets){ 8, 16, 8, 16 };
    widget->layout.min_width = 60.0f;
    widget->layout.min_height = 32.0f;
}

static void button_destroy(VenomWidget* widget) {
    VenomButton* btn = (VenomButton*)widget;
    
    if (btn->label) {
        venom_free(btn->label, strlen(btn->label) + 1);
        btn->label = NULL;
    }
    
    /* Call parent destroy */
    venom_widget_class.destroy(widget);
}

static void button_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                            VenomF32* out_width, VenomF32* out_height) {
    VenomButton* btn = (VenomButton*)widget;
    (void)available_width;
    (void)available_height;
    
    /* Calculate based on text */
    VenomF32 text_width = 0;
    VenomF32 text_height = btn->font_size * 1.2f;  /* Approximate line height */
    
    if (btn->label) {
        /* Simple estimation: ~7 pixels per character */
        text_width = strlen(btn->label) * (btn->font_size * 0.5f);
    }
    
    *out_width = text_width + widget->layout.padding.left + widget->layout.padding.right;
    *out_height = text_height + widget->layout.padding.top + widget->layout.padding.bottom;
    
    /* Apply minimums */
    if (*out_width < widget->layout.min_width) *out_width = widget->layout.min_width;
    if (*out_height < widget->layout.min_height) *out_height = widget->layout.min_height;
}

static void button_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomButton* btn = (VenomButton*)widget;
    
    /* Determine background color based on state */
    VenomColor bg;
    if (venom_widget_has_state(widget, VENOM_WIDGET_STATE_PRESSED)) {
        bg = btn->bg_pressed_color;
    } else if (venom_widget_has_state(widget, VENOM_WIDGET_STATE_HOVERED)) {
        bg = btn->bg_hover_color;
    } else {
        bg = btn->bg_color;
    }
    
    if (venom_widget_has_state(widget, VENOM_WIDGET_STATE_DISABLED)) {
        /* Desaturate for disabled */
        VenomU8 gray = (bg.r + bg.g + bg.b) / 3;
        bg = venom_color_rgba(gray, gray, gray, 160);
    }
    
    /* Draw rounded rectangle background */
    VenomRectF rect = { 0, 0, widget->bounds.width, widget->bounds.height };
    VenomPaint paint = venom_paint_fill(bg);
    venom_canvas_draw_rounded_rect(canvas, rect, btn->corner_radius, &paint);
    
    /* Draw text centered */
    if (btn->label) {
        VenomF32 text_width = strlen(btn->label) * (btn->font_size * 0.5f);
        VenomF32 text_height = btn->font_size;
        VenomF32 x = (widget->bounds.width - text_width) / 2;
        VenomF32 y = (widget->bounds.height + text_height * 0.35f) / 2;  /* Baseline adjust */
        
        VenomPaint text_paint = venom_paint_fill(btn->text_color);
        venom_canvas_draw_text(canvas, btn->label, x, y, NULL, &text_paint);
    }
}

static VenomBool button_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomButton* btn = (VenomButton*)widget;
    
    if (!venom_widget_is_enabled(widget)) return VENOM_FALSE;
    
    switch (event->type) {
        case VENOM_EVENT_MOUSE_ENTER:
            venom_widget_add_state(widget, VENOM_WIDGET_STATE_HOVERED);
            return VENOM_TRUE;
            
        case VENOM_EVENT_MOUSE_LEAVE:
            venom_widget_remove_state(widget, VENOM_WIDGET_STATE_HOVERED);
            venom_widget_remove_state(widget, VENOM_WIDGET_STATE_PRESSED);
            return VENOM_TRUE;
            
        case VENOM_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
                venom_widget_add_state(widget, VENOM_WIDGET_STATE_PRESSED);
                return VENOM_TRUE;
            }
            break;
            
        case VENOM_EVENT_MOUSE_BUTTON_UP:
            if (event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
                VenomBool was_pressed = venom_widget_has_state(widget, VENOM_WIDGET_STATE_PRESSED);
                venom_widget_remove_state(widget, VENOM_WIDGET_STATE_PRESSED);
                
                /* Fire click if released while still pressed */
                if (was_pressed && btn->on_click) {
                    btn->on_click(btn, btn->callback_data);
                }
                return VENOM_TRUE;
            }
            break;
            
        default:
            break;
    }
    
    return VENOM_FALSE;
}

/* ============================================================================
 * BUTTON CLASS
 * ============================================================================ */

const VenomWidgetClass venom_button_class = {
    .class_name = "VenomButton",
    .instance_size = sizeof(VenomButton),
    .parent_class = &venom_widget_class,
    .init = button_init,
    .destroy = button_destroy,
    .measure = button_measure,
    .layout = NULL,  /* Will use parent's at runtime */
    .draw = button_draw,
    .on_event = button_on_event,
    .on_state_changed = NULL,  /* Will use parent's at runtime */
};

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VenomResultPtr venom_button_create(const char* label) {
    VenomResultPtr result = venom_widget_create(&venom_button_class);
    if (!result.ok) return result;
    
    VenomButton* btn = (VenomButton*)result.value;
    
    if (label) {
        VenomResult r = venom_button_set_label(btn, label);
        if (!r.ok) {
            venom_unref(btn);
            return VENOM_ERR_PTR(r.error);
        }
    }
    
    return VENOM_OK_PTR(btn);
}

VenomResult venom_button_set_label(VenomButton* button, const char* label) {
    VENOM_ENSURE_NOT_NULL(button);
    
    /* Free old label */
    if (button->label) {
        venom_free(button->label, strlen(button->label) + 1);
        button->label = NULL;
    }
    
    if (label) {
        VenomSize len = strlen(label) + 1;
        button->label = (char*)venom_alloc(len);
        if (!button->label) {
            return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
        }
        memcpy(button->label, label, len);
    }
    
    venom_widget_invalidate_layout((VenomWidget*)button);
    return VENOM_OK_UNIT();
}

const char* venom_button_get_label(const VenomButton* button) {
    return button ? button->label : NULL;
}

void venom_button_set_on_click(VenomButton* button, VenomButtonCallback callback, void* user_data) {
    if (!button) return;
    button->on_click = callback;
    button->callback_data = user_data;
}

void venom_button_set_colors(VenomButton* button, VenomColor bg, VenomColor hover, 
                              VenomColor pressed, VenomColor text) {
    if (!button) return;
    button->bg_color = bg;
    button->bg_hover_color = hover;
    button->bg_pressed_color = pressed;
    button->text_color = text;
    venom_widget_invalidate((VenomWidget*)button);
}

void venom_button_set_corner_radius(VenomButton* button, VenomF32 radius) {
    if (!button) return;
    button->corner_radius = radius;
    venom_widget_invalidate((VenomWidget*)button);
}
