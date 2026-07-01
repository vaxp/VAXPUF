/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_button.c - Button widget implementation
 */

#include "vaxp/widgets/vaxp_button.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

/* ============================================================================
 * DEFAULT COLORS
 * ============================================================================ */

static const VaxpColor DEFAULT_BG = { 60, 120, 220, 255 };         /* Blue */
static const VaxpColor DEFAULT_HOVER = { 80, 140, 240, 255 };      /* Lighter blue */
static const VaxpColor DEFAULT_PRESSED = { 40, 100, 200, 255 };    /* Darker blue */
static const VaxpColor DEFAULT_TEXT = { 255, 255, 255, 255 };      /* White */

/* ============================================================================
 * BUTTON CLASS METHODS
 * ============================================================================ */

static void button_init(VaxpWidget* widget) {
    VaxpButton* btn = (VaxpButton*)widget;
    
    /* Default styling */
    btn->bg_color = DEFAULT_BG;
    btn->bg_hover_color = DEFAULT_HOVER;
    btn->bg_pressed_color = DEFAULT_PRESSED;
    btn->text_color = DEFAULT_TEXT;
    btn->font_size = 16.0f;
    btn->font_family = NULL;
    btn->corner_radius = 4.0f;
    
    /* Default layout */
    widget->layout.padding = (VaxpInsets){ 8, 16, 8, 16 };
    widget->layout.min_width = 60.0f;
    widget->layout.min_height = 32.0f;
    
    /* Buttons are focusable by default */
    widget->focusable = VAXP_TRUE;
}

static void button_destroy(VaxpWidget* widget) {
    VaxpButton* btn = (VaxpButton*)widget;
    
    if (btn->label) {
        vaxp_free(btn->label, strlen(btn->label) + 1);
        btn->label = NULL;
    }
    
    if (btn->font_family) {
        vaxp_free(btn->font_family, strlen(btn->font_family) + 1);
        btn->font_family = NULL;
    }
    
    /* Call parent destroy */
    vaxp_widget_class.destroy(widget);
}

static void button_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                            VaxpF32* out_width, VaxpF32* out_height) {
    VaxpButton* btn = (VaxpButton*)widget;
    (void)available_width;
    (void)available_height;
    
    /* Calculate based on text */
    VaxpF32 text_width = 0;
    VaxpF32 text_height = btn->font_size * 1.2f;  /* Approximate line height */
    
    if (btn->label) {
        /* Simple estimation: ~0.6 ratio for monospace */
        text_width = strlen(btn->label) * (btn->font_size * 0.60f);
    }
    
    *out_width = text_width + widget->layout.padding.left + widget->layout.padding.right;
    *out_height = text_height + widget->layout.padding.top + widget->layout.padding.bottom;
    
    /* Apply minimums */
    if (*out_width < widget->layout.min_width) *out_width = widget->layout.min_width;
    if (*out_height < widget->layout.min_height) *out_height = widget->layout.min_height;
}

static void button_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpButton* btn = (VaxpButton*)widget;
    
    /* Determine background color based on state */
    VaxpColor bg;
    if (vaxp_widget_has_state(widget, VAXP_WIDGET_STATE_PRESSED)) {
        bg = btn->bg_pressed_color;
    } else if (vaxp_widget_has_state(widget, VAXP_WIDGET_STATE_HOVERED)) {
        bg = btn->bg_hover_color;
    } else {
        bg = btn->bg_color;
    }
    
    if (vaxp_widget_has_state(widget, VAXP_WIDGET_STATE_DISABLED)) {
        /* Desaturate for disabled */
        VaxpU8 gray = (bg.r + bg.g + bg.b) / 3;
        bg = vaxp_color_rgba(gray, gray, gray, 160);
    }
    
    /* Draw focus ring if focused */
    if (vaxp_widget_has_state(widget, VAXP_WIDGET_STATE_FOCUSED)) {
        VaxpRectF focus_rect = { -2, -2, widget->bounds.width + 4, widget->bounds.height + 4 };
        VaxpColor focus_color = vaxp_color_rgba(100, 150, 255, 200);
        VaxpPaint focus_paint = vaxp_paint_stroke(focus_color, 2.0f);
        vaxp_canvas_draw_rounded_rect(canvas, focus_rect, btn->corner_radius + 2, &focus_paint);
    }
    
    /* Draw rounded rectangle background */
    VaxpRectF rect = { 0, 0, widget->bounds.width, widget->bounds.height };
    VaxpPaint paint = vaxp_paint_fill(bg);
    vaxp_canvas_draw_rounded_rect(canvas, rect, btn->corner_radius, &paint);
    
    /* Draw text centered */
    if (btn->label) {
        VaxpF32 text_width = strlen(btn->label) * (btn->font_size * 0.60f);
        VaxpF32 text_height = btn->font_size;
        VaxpF32 x = (widget->bounds.width - text_width) / 2;
        VaxpF32 y = (widget->bounds.height - text_height) / 2;  /* Top-Left adjust */
        
        VaxpPaint text_paint = vaxp_paint_fill(btn->text_color);
        VaxpFont font = {0};
        font.family = btn->font_family;
        font.size = btn->font_size;
        vaxp_canvas_draw_text(canvas, btn->label, x, y, &font, &text_paint);
    }
}

static VaxpBool button_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpButton* btn = (VaxpButton*)widget;
    
    if (!vaxp_widget_is_enabled(widget)) return VAXP_FALSE;
    
    switch (event->type) {
        case VAXP_EVENT_MOUSE_ENTER:
            vaxp_widget_add_state(widget, VAXP_WIDGET_STATE_HOVERED);
            return VAXP_TRUE;
            
        case VAXP_EVENT_MOUSE_LEAVE:
            vaxp_widget_remove_state(widget, VAXP_WIDGET_STATE_HOVERED);
            vaxp_widget_remove_state(widget, VAXP_WIDGET_STATE_PRESSED);
            return VAXP_TRUE;
            
        case VAXP_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
                vaxp_widget_add_state(widget, VAXP_WIDGET_STATE_PRESSED);
                return VAXP_TRUE;
            }
            break;
            
        case VAXP_EVENT_MOUSE_BUTTON_UP:
            if (event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
                VaxpBool was_pressed = vaxp_widget_has_state(widget, VAXP_WIDGET_STATE_PRESSED);
                vaxp_widget_remove_state(widget, VAXP_WIDGET_STATE_PRESSED);
                
                /* Fire click if released while still pressed */
                if (was_pressed && btn->on_click) {
                    btn->on_click(btn, btn->callback_data);
                }
                return VAXP_TRUE;
            }
            break;
            
        default:
            break;
    }
    
    return VAXP_FALSE;
}

/* ============================================================================
 * BUTTON CLASS
 * ============================================================================ */

const VaxpWidgetClass vaxp_button_class = {
    .class_name = "VaxpButton",
    .instance_size = sizeof(VaxpButton),
    .parent_class = &vaxp_widget_class,
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

VaxpResultPtr vaxp_button_create(const char* label) {
    VaxpResultPtr result = vaxp_widget_create(&vaxp_button_class);
    if (!result.ok) return result;
    
    VaxpButton* btn = (VaxpButton*)result.value;
    
    if (label) {
        VaxpResult r = vaxp_button_set_label(btn, label);
        if (!r.ok) {
            vaxp_unref(btn);
            return VAXP_ERR_PTR(r.error);
        }
    }
    
    return VAXP_OK_PTR(btn);
}

VaxpResult vaxp_button_set_label(VaxpButton* button, const char* label) {
    VAXP_ENSURE_NOT_NULL(button);
    
    /* Free old label */
    if (button->label) {
        vaxp_free(button->label, strlen(button->label) + 1);
        button->label = NULL;
    }
    
    if (label) {
        VaxpSize len = strlen(label) + 1;
        button->label = (char*)vaxp_alloc(len);
        if (!button->label) {
            return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_MEMORY);
        }
        memcpy(button->label, label, len);
    }
    
    vaxp_widget_invalidate_layout((VaxpWidget*)button);
    return VAXP_OK_UNIT();
}

const char* vaxp_button_get_label(const VaxpButton* btn) {
    if (!btn) return NULL;
    return btn->label;
}

void vaxp_button_set_font_family(VaxpButton* btn, const char* family) {
    if (!btn) return;
    if (btn->font_family) {
        vaxp_free(btn->font_family, strlen(btn->font_family) + 1);
        btn->font_family = NULL;
    }
    if (family) {
        size_t len = strlen(family);
        btn->font_family = vaxp_alloc(len + 1);
        if (btn->font_family) {
            memcpy(btn->font_family, family, len + 1);
        }
    }
    vaxp_widget_invalidate_layout((VaxpWidget*)btn);
}

void vaxp_button_set_on_click(VaxpButton* button, VaxpButtonCallback callback, void* user_data) {
    if (!button) return;
    button->on_click = callback;
    button->callback_data = user_data;
}

void vaxp_button_set_colors(VaxpButton* button, VaxpColor bg, VaxpColor hover, 
                              VaxpColor pressed, VaxpColor text) {
    if (!button) return;
    button->bg_color = bg;
    button->bg_hover_color = hover;
    button->bg_pressed_color = pressed;
    button->text_color = text;
    vaxp_widget_invalidate((VaxpWidget*)button);
}

void vaxp_button_set_corner_radius(VaxpButton* button, VaxpF32 radius) {
    if (!button) return;
    button->corner_radius = radius;
    vaxp_widget_invalidate((VaxpWidget*)button);
}
