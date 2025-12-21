/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_number_input.c - Numeric input implementation
 */

#include "venom/widgets/venom_number_input.h"
#include "venom/core/venom_memory.h"
#include <stdio.h>

#define DEFAULT_HEIGHT 36.0f
#define BUTTON_WIDTH 32.0f

static void number_input_init(VenomWidget* widget) {
    VenomNumberInput* input = (VenomNumberInput*)widget;
    
    input->value = 0;
    input->min = 0;
    input->max = 100;
    input->step = 1;
    input->decimals = 0;
    
    input->on_change = NULL;
    input->callback_data = NULL;
    
    input->background_color = (VenomColor){ 255, 255, 255, 255 };
    input->border_color = (VenomColor){ 189, 189, 189, 255 };
    input->button_color = (VenomColor){ 245, 245, 245, 255 };
    input->text_color = (VenomColor){ 33, 33, 33, 255 };
    input->height = DEFAULT_HEIGHT;
    
    widget->focusable = VENOM_TRUE;
}

static void number_input_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                                 VenomF32* out_width, VenomF32* out_height) {
    VenomNumberInput* input = (VenomNumberInput*)widget;
    (void)available_height;
    
    *out_width = widget->layout.preferred_width > 0 ? widget->layout.preferred_width : 
                 (available_width > 120 ? 120 : available_width);
    *out_height = input->height;
}

static void number_input_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomNumberInput* input = (VenomNumberInput*)widget;
    
    VenomF32 w = widget->bounds.width;
    VenomF32 h = input->height;
    
    /* Draw background */
    VenomRectF bg = { BUTTON_WIDTH, 0, w - BUTTON_WIDTH * 2, h };
    VenomPaint bg_paint = venom_paint_fill(input->background_color);
    venom_canvas_draw_rect(canvas, bg, &bg_paint);
    
    /* Draw buttons */
    VenomPaint btn_paint = venom_paint_fill(input->button_color);
    VenomRectF minus_btn = { 0, 0, BUTTON_WIDTH, h };
    VenomRectF plus_btn = { w - BUTTON_WIDTH, 0, BUTTON_WIDTH, h };
    venom_canvas_draw_rect(canvas, minus_btn, &btn_paint);
    venom_canvas_draw_rect(canvas, plus_btn, &btn_paint);
    
    /* Draw borders */
    VenomPaint border_paint = venom_paint_stroke(input->border_color, 1.0f);
    VenomRectF full = { 0, 0, w, h };
    venom_canvas_draw_rect(canvas, full, &border_paint);
    venom_canvas_draw_line(canvas, BUTTON_WIDTH, 0, BUTTON_WIDTH, h, &border_paint);
    venom_canvas_draw_line(canvas, w - BUTTON_WIDTH, 0, w - BUTTON_WIDTH, h, &border_paint);
    
    /* Draw -/+ symbols */
    VenomPaint text_paint = venom_paint_fill(input->text_color);
    venom_canvas_draw_text(canvas, "−", BUTTON_WIDTH / 2 - 4, h / 2 + 5, NULL, &text_paint);
    venom_canvas_draw_text(canvas, "+", w - BUTTON_WIDTH / 2 - 4, h / 2 + 5, NULL, &text_paint);
    
    /* Draw value */
    char buf[32];
    if (input->decimals == 0) {
        snprintf(buf, sizeof(buf), "%d", (int)input->value);
    } else {
        snprintf(buf, sizeof(buf), "%.*f", input->decimals, input->value);
    }
    
    VenomF32 text_w = (VenomF32)strlen(buf) * 8;
    VenomF32 text_x = BUTTON_WIDTH + (w - BUTTON_WIDTH * 2 - text_w) / 2;
    venom_canvas_draw_text(canvas, buf, text_x, h / 2 + 5, NULL, &text_paint);
}

static void change_value(VenomNumberInput* input, VenomF32 delta) {
    VenomF32 new_val = input->value + delta;
    if (new_val < input->min) new_val = input->min;
    if (new_val > input->max) new_val = input->max;
    
    if (new_val != input->value) {
        input->value = new_val;
        venom_widget_invalidate((VenomWidget*)input);
        
        if (input->on_change) {
            input->on_change(input, input->value, input->callback_data);
        }
    }
}

static VenomBool number_input_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomNumberInput* input = (VenomNumberInput*)widget;
    
    if (event->type == VENOM_EVENT_MOUSE_BUTTON_DOWN && 
        event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
        VenomF32 mx = (VenomF32)event->mouse.x;
        VenomF32 w = widget->bounds.width;
        
        if (mx < BUTTON_WIDTH) {
            change_value(input, -input->step);
            return VENOM_TRUE;
        } else if (mx > w - BUTTON_WIDTH) {
            change_value(input, input->step);
            return VENOM_TRUE;
        }
    }
    
    if (event->type == VENOM_EVENT_KEY_DOWN) {
        if (event->key.key == VENOM_KEY_UP) {
            change_value(input, input->step);
            return VENOM_TRUE;
        } else if (event->key.key == VENOM_KEY_DOWN) {
            change_value(input, -input->step);
            return VENOM_TRUE;
        }
    }
    
    if (event->type == VENOM_EVENT_MOUSE_SCROLL) {
        change_value(input, event->scroll.y * input->step);
        return VENOM_TRUE;
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_number_input_class = {
    .class_name = "VenomNumberInput",
    .instance_size = sizeof(VenomNumberInput),
    .parent_class = &venom_widget_class,
    .init = number_input_init,
    .destroy = NULL,
    .measure = number_input_measure,
    .layout = NULL,
    .draw = number_input_draw,
    .on_event = number_input_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_number_input_create(void) {
    return venom_widget_create(&venom_number_input_class);
}

void venom_number_input_set_value(VenomNumberInput* input, VenomF32 value) {
    if (input) {
        if (value < input->min) value = input->min;
        if (value > input->max) value = input->max;
        input->value = value;
        venom_widget_invalidate((VenomWidget*)input);
    }
}

VenomF32 venom_number_input_get_value(const VenomNumberInput* input) {
    return input ? input->value : 0;
}

void venom_number_input_set_range(VenomNumberInput* input, VenomF32 min, VenomF32 max) {
    if (input) {
        input->min = min;
        input->max = max;
        if (input->value < min) input->value = min;
        if (input->value > max) input->value = max;
    }
}

void venom_number_input_set_step(VenomNumberInput* input, VenomF32 step) {
    if (input && step > 0) input->step = step;
}

void venom_number_input_set_on_change(VenomNumberInput* input, VenomNumberCallback callback, void* data) {
    if (input) {
        input->on_change = callback;
        input->callback_data = data;
    }
}

VenomWidget* _venom_number_input_build(const VenomNumberInputConfig* config) {
    VenomResultPtr result = venom_number_input_create();
    if (!result.ok) return NULL;
    
    VenomNumberInput* input = (VenomNumberInput*)result.value;
    
    if (config->min != 0 || config->max != 0) {
        input->min = config->min;
        input->max = config->max;
    }
    input->value = config->value;
    if (config->step > 0) input->step = config->step;
    input->on_change = config->on_change;
    input->callback_data = config->data;
    
    return (VenomWidget*)input;
}
