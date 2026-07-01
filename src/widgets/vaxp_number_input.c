/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_number_input.c - Numeric input implementation
 */

#include "vaxp/widgets/vaxp_number_input.h"
#include "vaxp/core/vaxp_memory.h"
#include <stdio.h>

#define DEFAULT_HEIGHT 36.0f
#define BUTTON_WIDTH 32.0f

static void number_input_init(VaxpWidget* widget) {
    VaxpNumberInput* input = (VaxpNumberInput*)widget;
    
    input->value = 0;
    input->min = 0;
    input->max = 100;
    input->step = 1;
    input->decimals = 0;
    
    input->on_change = NULL;
    input->callback_data = NULL;
    
    input->background_color = (VaxpColor){ 255, 255, 255, 255 };
    input->border_color = (VaxpColor){ 189, 189, 189, 255 };
    input->button_color = (VaxpColor){ 245, 245, 245, 255 };
    input->text_color = (VaxpColor){ 33, 33, 33, 255 };
    input->height = DEFAULT_HEIGHT;
    
    widget->focusable = VAXP_TRUE;
}

static void number_input_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                                 VaxpF32* out_width, VaxpF32* out_height) {
    VaxpNumberInput* input = (VaxpNumberInput*)widget;
    (void)available_height;
    
    *out_width = widget->layout.preferred_width > 0 ? widget->layout.preferred_width : 
                 (available_width > 120 ? 120 : available_width);
    *out_height = input->height;
}

static void number_input_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpNumberInput* input = (VaxpNumberInput*)widget;
    
    VaxpF32 w = widget->bounds.width;
    VaxpF32 h = input->height;
    
    /* Draw background */
    VaxpRectF bg = { BUTTON_WIDTH, 0, w - BUTTON_WIDTH * 2, h };
    VaxpPaint bg_paint = vaxp_paint_fill(input->background_color);
    vaxp_canvas_draw_rect(canvas, bg, &bg_paint);
    
    /* Draw buttons */
    VaxpPaint btn_paint = vaxp_paint_fill(input->button_color);
    VaxpRectF minus_btn = { 0, 0, BUTTON_WIDTH, h };
    VaxpRectF plus_btn = { w - BUTTON_WIDTH, 0, BUTTON_WIDTH, h };
    vaxp_canvas_draw_rect(canvas, minus_btn, &btn_paint);
    vaxp_canvas_draw_rect(canvas, plus_btn, &btn_paint);
    
    /* Draw borders */
    VaxpPaint border_paint = vaxp_paint_stroke(input->border_color, 1.0f);
    VaxpRectF full = { 0, 0, w, h };
    vaxp_canvas_draw_rect(canvas, full, &border_paint);
    vaxp_canvas_draw_line(canvas, BUTTON_WIDTH, 0, BUTTON_WIDTH, h, &border_paint);
    vaxp_canvas_draw_line(canvas, w - BUTTON_WIDTH, 0, w - BUTTON_WIDTH, h, &border_paint);
    
    /* Draw -/+ symbols */
    VaxpPaint text_paint = vaxp_paint_fill(input->text_color);
    vaxp_canvas_draw_text(canvas, "−", BUTTON_WIDTH / 2 - 4, h / 2 + 5, NULL, &text_paint);
    vaxp_canvas_draw_text(canvas, "+", w - BUTTON_WIDTH / 2 - 4, h / 2 + 5, NULL, &text_paint);
    
    /* Draw value */
    char buf[32];
    if (input->decimals == 0) {
        snprintf(buf, sizeof(buf), "%d", (int)input->value);
    } else {
        snprintf(buf, sizeof(buf), "%.*f", input->decimals, input->value);
    }
    
    VaxpF32 text_w = (VaxpF32)strlen(buf) * 8;
    VaxpF32 text_x = BUTTON_WIDTH + (w - BUTTON_WIDTH * 2 - text_w) / 2;
    vaxp_canvas_draw_text(canvas, buf, text_x, h / 2 + 5, NULL, &text_paint);
}

static void change_value(VaxpNumberInput* input, VaxpF32 delta) {
    VaxpF32 new_val = input->value + delta;
    if (new_val < input->min) new_val = input->min;
    if (new_val > input->max) new_val = input->max;
    
    if (new_val != input->value) {
        input->value = new_val;
        vaxp_widget_invalidate((VaxpWidget*)input);
        
        if (input->on_change) {
            input->on_change(input, input->value, input->callback_data);
        }
    }
}

static VaxpBool number_input_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpNumberInput* input = (VaxpNumberInput*)widget;
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN && 
        event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
        VaxpF32 mx = (VaxpF32)event->mouse.x;
        VaxpF32 w = widget->bounds.width;
        
        if (mx < BUTTON_WIDTH) {
            change_value(input, -input->step);
            return VAXP_TRUE;
        } else if (mx > w - BUTTON_WIDTH) {
            change_value(input, input->step);
            return VAXP_TRUE;
        }
    }
    
    if (event->type == VAXP_EVENT_KEY_DOWN) {
        if (event->key.key == VAXP_KEY_UP) {
            change_value(input, input->step);
            return VAXP_TRUE;
        } else if (event->key.key == VAXP_KEY_DOWN) {
            change_value(input, -input->step);
            return VAXP_TRUE;
        }
    }
    
    if (event->type == VAXP_EVENT_MOUSE_SCROLL) {
        change_value(input, event->scroll.y * input->step);
        return VAXP_TRUE;
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_number_input_class = {
    .class_name = "VaxpNumberInput",
    .instance_size = sizeof(VaxpNumberInput),
    .parent_class = &vaxp_widget_class,
    .init = number_input_init,
    .destroy = NULL,
    .measure = number_input_measure,
    .layout = NULL,
    .draw = number_input_draw,
    .on_event = number_input_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_number_input_create(void) {
    return vaxp_widget_create(&vaxp_number_input_class);
}

void vaxp_number_input_set_value(VaxpNumberInput* input, VaxpF32 value) {
    if (input) {
        if (value < input->min) value = input->min;
        if (value > input->max) value = input->max;
        input->value = value;
        vaxp_widget_invalidate((VaxpWidget*)input);
    }
}

VaxpF32 vaxp_number_input_get_value(const VaxpNumberInput* input) {
    return input ? input->value : 0;
}

void vaxp_number_input_set_range(VaxpNumberInput* input, VaxpF32 min, VaxpF32 max) {
    if (input) {
        input->min = min;
        input->max = max;
        if (input->value < min) input->value = min;
        if (input->value > max) input->value = max;
    }
}

void vaxp_number_input_set_step(VaxpNumberInput* input, VaxpF32 step) {
    if (input && step > 0) input->step = step;
}

void vaxp_number_input_set_on_change(VaxpNumberInput* input, VaxpNumberCallback callback, void* data) {
    if (input) {
        input->on_change = callback;
        input->callback_data = data;
    }
}

VaxpWidget* _vaxp_number_input_build(const VaxpNumberInputConfig* config) {
    VaxpResultPtr result = vaxp_number_input_create();
    if (!result.ok) return NULL;
    
    VaxpNumberInput* input = (VaxpNumberInput*)result.value;
    
    if (config->min != 0 || config->max != 0) {
        input->min = config->min;
        input->max = config->max;
    }
    input->value = config->value;
    if (config->step > 0) input->step = config->step;
    input->on_change = config->on_change;
    input->callback_data = config->data;
    
    return (VaxpWidget*)input;
}
