/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_toggle_button.c - Toggle button implementation
 */

#include "venom/widgets/venom_toggle_button.h"
#include "venom/core/venom_memory.h"
#include <string.h>

#define DEFAULT_HEIGHT 36.0f

static void toggle_button_init(VenomWidget* widget) {
    VenomToggleButton* btn = (VenomToggleButton*)widget;
    
    btn->label = NULL;
    btn->toggled = VENOM_FALSE;
    btn->on_toggle = NULL;
    btn->callback_data = NULL;
    
    btn->normal_color = (VenomColor){ 224, 224, 224, 255 };
    btn->toggled_color = (VenomColor){ 63, 81, 181, 255 };
    btn->text_color = (VenomColor){ 33, 33, 33, 255 };
    btn->toggled_text_color = (VenomColor){ 255, 255, 255, 255 };
    btn->corner_radius = 4.0f;
    btn->height = DEFAULT_HEIGHT;
    
    widget->focusable = VENOM_TRUE;
}

static void toggle_button_destroy(VenomWidget* widget) {
    VenomToggleButton* btn = (VenomToggleButton*)widget;
    if (btn->label) venom_free(btn->label, strlen(btn->label) + 1);
    venom_widget_class.destroy(widget);
}

static void toggle_button_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                                  VenomF32* out_width, VenomF32* out_height) {
    VenomToggleButton* btn = (VenomToggleButton*)widget;
    (void)available_width; (void)available_height;
    
    VenomF32 text_w = btn->label ? (VenomF32)strlen(btn->label) * 9 : 0;
    *out_width = text_w + 32;
    *out_height = btn->height;
}

static void toggle_button_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomToggleButton* btn = (VenomToggleButton*)widget;
    
    VenomF32 w = widget->bounds.width;
    VenomF32 h = btn->height;
    
    /* Draw background */
    VenomColor bg_color = btn->toggled ? btn->toggled_color : btn->normal_color;
    VenomRectF bg = { 0, 0, w, h };
    VenomPaint bg_paint = venom_paint_fill(bg_color);
    venom_canvas_draw_rounded_rect(canvas, bg, btn->corner_radius, &bg_paint);
    
    /* Draw border for normal state */
    if (!btn->toggled) {
        VenomPaint border_paint = venom_paint_stroke((VenomColor){ 189, 189, 189, 255 }, 1.0f);
        venom_canvas_draw_rounded_rect(canvas, bg, btn->corner_radius, &border_paint);
    }
    
    /* Draw label */
    if (btn->label) {
        VenomColor text_color = btn->toggled ? btn->toggled_text_color : btn->text_color;
        VenomPaint text_paint = venom_paint_fill(text_color);
        VenomF32 text_w = (VenomF32)strlen(btn->label) * 9;
        VenomF32 text_x = (w - text_w) / 2;
        venom_canvas_draw_text(canvas, btn->label, text_x, h / 2 + 5, NULL, &text_paint);
    }
}

static VenomBool toggle_button_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomToggleButton* btn = (VenomToggleButton*)widget;
    
    if (event->type == VENOM_EVENT_MOUSE_BUTTON_DOWN && 
        event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
        btn->toggled = !btn->toggled;
        widget->needs_redraw = VENOM_TRUE;
        
        if (btn->on_toggle) {
            btn->on_toggle(btn, btn->toggled, btn->callback_data);
        }
        return VENOM_TRUE;
    }
    
    if (event->type == VENOM_EVENT_KEY_DOWN && 
        (event->key.key == VENOM_KEY_RETURN || event->key.key == VENOM_KEY_SPACE)) {
        btn->toggled = !btn->toggled;
        widget->needs_redraw = VENOM_TRUE;
        
        if (btn->on_toggle) {
            btn->on_toggle(btn, btn->toggled, btn->callback_data);
        }
        return VENOM_TRUE;
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_toggle_button_class = {
    .class_name = "VenomToggleButton",
    .instance_size = sizeof(VenomToggleButton),
    .parent_class = &venom_widget_class,
    .init = toggle_button_init,
    .destroy = toggle_button_destroy,
    .measure = toggle_button_measure,
    .layout = NULL,
    .draw = toggle_button_draw,
    .on_event = toggle_button_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_toggle_button_create(void) {
    return venom_widget_create(&venom_toggle_button_class);
}

void venom_toggle_button_set_label(VenomToggleButton* btn, const char* label) {
    if (!btn) return;
    if (btn->label) venom_free(btn->label, strlen(btn->label) + 1);
    btn->label = NULL;
    
    if (label) {
        VenomSize len = strlen(label) + 1;
        btn->label = (char*)venom_alloc(len);
        if (btn->label) memcpy(btn->label, label, len);
    }
}

void venom_toggle_button_set_toggled(VenomToggleButton* btn, VenomBool toggled) {
    if (btn) {
        btn->toggled = toggled;
        venom_widget_invalidate((VenomWidget*)btn);
    }
}

VenomBool venom_toggle_button_get_toggled(const VenomToggleButton* btn) {
    return btn ? btn->toggled : VENOM_FALSE;
}

void venom_toggle_button_set_on_toggle(VenomToggleButton* btn, VenomToggleButtonCallback callback, void* data) {
    if (btn) {
        btn->on_toggle = callback;
        btn->callback_data = data;
    }
}

VenomWidget* _venom_toggle_button_build(const VenomToggleButtonConfig* config) {
    VenomResultPtr result = venom_toggle_button_create();
    if (!result.ok) return NULL;
    
    VenomToggleButton* btn = (VenomToggleButton*)result.value;
    
    if (config->label) venom_toggle_button_set_label(btn, config->label);
    btn->toggled = config->toggled;
    btn->on_toggle = config->on_toggle;
    btn->callback_data = config->data;
    
    return (VenomWidget*)btn;
}
