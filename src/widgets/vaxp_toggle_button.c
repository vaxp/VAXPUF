/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_toggle_button.c - Toggle button implementation
 */

#include "vaxp/widgets/vaxp_toggle_button.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

#define DEFAULT_HEIGHT 36.0f

static void toggle_button_init(VaxpWidget* widget) {
    VaxpToggleButton* btn = (VaxpToggleButton*)widget;
    
    btn->label = NULL;
    btn->toggled = VAXP_FALSE;
    btn->on_toggle = NULL;
    btn->callback_data = NULL;
    
    btn->normal_color = (VaxpColor){ 224, 224, 224, 255 };
    btn->toggled_color = (VaxpColor){ 63, 81, 181, 255 };
    btn->text_color = (VaxpColor){ 33, 33, 33, 255 };
    btn->toggled_text_color = (VaxpColor){ 255, 255, 255, 255 };
    btn->corner_radius = 4.0f;
    btn->height = DEFAULT_HEIGHT;
    
    widget->focusable = VAXP_TRUE;
}

static void toggle_button_destroy(VaxpWidget* widget) {
    VaxpToggleButton* btn = (VaxpToggleButton*)widget;
    if (btn->label) vaxp_free(btn->label, strlen(btn->label) + 1);
    vaxp_widget_class.destroy(widget);
}

static void toggle_button_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                                  VaxpF32* out_width, VaxpF32* out_height) {
    VaxpToggleButton* btn = (VaxpToggleButton*)widget;
    (void)available_width; (void)available_height;
    
    VaxpF32 text_w = btn->label ? (VaxpF32)strlen(btn->label) * 9 : 0;
    *out_width = text_w + 32;
    *out_height = btn->height;
}

static void toggle_button_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpToggleButton* btn = (VaxpToggleButton*)widget;
    
    VaxpF32 w = widget->bounds.width;
    VaxpF32 h = btn->height;
    
    /* Draw background */
    VaxpColor bg_color = btn->toggled ? btn->toggled_color : btn->normal_color;
    VaxpRectF bg = { 0, 0, w, h };
    VaxpPaint bg_paint = vaxp_paint_fill(bg_color);
    vaxp_canvas_draw_rounded_rect(canvas, bg, btn->corner_radius, &bg_paint);
    
    /* Draw border for normal state */
    if (!btn->toggled) {
        VaxpPaint border_paint = vaxp_paint_stroke((VaxpColor){ 189, 189, 189, 255 }, 1.0f);
        vaxp_canvas_draw_rounded_rect(canvas, bg, btn->corner_radius, &border_paint);
    }
    
    /* Draw label */
    if (btn->label) {
        VaxpColor text_color = btn->toggled ? btn->toggled_text_color : btn->text_color;
        VaxpPaint text_paint = vaxp_paint_fill(text_color);
        VaxpF32 text_w = (VaxpF32)strlen(btn->label) * 9;
        VaxpF32 text_x = (w - text_w) / 2;
        vaxp_canvas_draw_text(canvas, btn->label, text_x, h / 2 + 5, NULL, &text_paint);
    }
}

static VaxpBool toggle_button_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpToggleButton* btn = (VaxpToggleButton*)widget;
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN && 
        event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
        btn->toggled = !btn->toggled;
        widget->needs_redraw = VAXP_TRUE;
        
        if (btn->on_toggle) {
            btn->on_toggle(btn, btn->toggled, btn->callback_data);
        }
        return VAXP_TRUE;
    }
    
    if (event->type == VAXP_EVENT_KEY_DOWN && 
        (event->key.key == VAXP_KEY_RETURN || event->key.key == VAXP_KEY_SPACE)) {
        btn->toggled = !btn->toggled;
        widget->needs_redraw = VAXP_TRUE;
        
        if (btn->on_toggle) {
            btn->on_toggle(btn, btn->toggled, btn->callback_data);
        }
        return VAXP_TRUE;
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_toggle_button_class = {
    .class_name = "VaxpToggleButton",
    .instance_size = sizeof(VaxpToggleButton),
    .parent_class = &vaxp_widget_class,
    .init = toggle_button_init,
    .destroy = toggle_button_destroy,
    .measure = toggle_button_measure,
    .layout = NULL,
    .draw = toggle_button_draw,
    .on_event = toggle_button_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_toggle_button_create(void) {
    return vaxp_widget_create(&vaxp_toggle_button_class);
}

void vaxp_toggle_button_set_label(VaxpToggleButton* btn, const char* label) {
    if (!btn) return;
    if (btn->label) vaxp_free(btn->label, strlen(btn->label) + 1);
    btn->label = NULL;
    
    if (label) {
        VaxpSize len = strlen(label) + 1;
        btn->label = (char*)vaxp_alloc(len);
        if (btn->label) memcpy(btn->label, label, len);
    }
}

void vaxp_toggle_button_set_toggled(VaxpToggleButton* btn, VaxpBool toggled) {
    if (btn) {
        btn->toggled = toggled;
        vaxp_widget_invalidate((VaxpWidget*)btn);
    }
}

VaxpBool vaxp_toggle_button_get_toggled(const VaxpToggleButton* btn) {
    return btn ? btn->toggled : VAXP_FALSE;
}

void vaxp_toggle_button_set_on_toggle(VaxpToggleButton* btn, VaxpToggleButtonCallback callback, void* data) {
    if (btn) {
        btn->on_toggle = callback;
        btn->callback_data = data;
    }
}

VaxpWidget* _vaxp_toggle_button_build(const VaxpToggleButtonConfig* config) {
    VaxpResultPtr result = vaxp_toggle_button_create();
    if (!result.ok) return NULL;
    
    VaxpToggleButton* btn = (VaxpToggleButton*)result.value;
    
    if (config->label) vaxp_toggle_button_set_label(btn, config->label);
    btn->toggled = config->toggled;
    btn->on_toggle = config->on_toggle;
    btn->callback_data = config->data;
    
    return (VaxpWidget*)btn;
}
