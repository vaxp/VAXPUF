/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_slider.c - Slider widget implementation
 */

#include "vaxp/widgets/vaxp_slider.h"
#include "vaxp/core/vaxp_memory.h"

#define DEFAULT_TRACK_HEIGHT 4.0f
#define DEFAULT_THUMB_RADIUS 10.0f

static void slider_init(VaxpWidget* widget) {
    VaxpSlider* slider = (VaxpSlider*)widget;
    
    slider->value = 0.0f;
    slider->min = 0.0f;
    slider->max = 1.0f;
    slider->step = 0;
    
    slider->enabled = VAXP_TRUE;
    slider->dragging = VAXP_FALSE;
    
    slider->track_height = DEFAULT_TRACK_HEIGHT;
    slider->thumb_radius = DEFAULT_THUMB_RADIUS;
    slider->track_color = (VaxpColor){ 200, 200, 200, 255 };
    slider->fill_color = (VaxpColor){ 63, 81, 181, 255 };
    slider->thumb_color = (VaxpColor){ 63, 81, 181, 255 };
    
    slider->on_change = NULL;
    slider->callback_data = NULL;
    
    widget->focusable = VAXP_TRUE;
    widget->layout.preferred_width = 200;
    widget->layout.preferred_height = 30;
}

static void slider_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                           VaxpF32* out_width, VaxpF32* out_height) {
    VaxpSlider* slider = (VaxpSlider*)widget;
    (void)available_height;
    
    *out_width = widget->layout.preferred_width > 0 ? 
                 widget->layout.preferred_width : available_width;
    *out_height = slider->thumb_radius * 2 + 10;
}

static VaxpF32 slider_normalize(const VaxpSlider* slider) {
    if (slider->max <= slider->min) return 0;
    return (slider->value - slider->min) / (slider->max - slider->min);
}

static void slider_set_from_pos(VaxpSlider* slider, VaxpF32 x) {
    VaxpF32 track_start = slider->thumb_radius;
    VaxpF32 track_end = ((VaxpWidget*)slider)->bounds.width - slider->thumb_radius;
    VaxpF32 track_width = track_end - track_start;
    
    VaxpF32 normalized = (x - track_start) / track_width;
    if (normalized < 0) normalized = 0;
    if (normalized > 1) normalized = 1;
    
    VaxpF32 new_value = slider->min + normalized * (slider->max - slider->min);
    
    /* Apply step */
    if (slider->step > 0) {
        new_value = slider->min + 
                    (int)((new_value - slider->min) / slider->step + 0.5f) * slider->step;
    }
    
    if (new_value != slider->value) {
        slider->value = new_value;
        ((VaxpWidget*)slider)->needs_redraw = VAXP_TRUE;
        
        if (slider->on_change) {
            slider->on_change(slider, slider->value, slider->callback_data);
        }
    }
}

static void slider_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpSlider* slider = (VaxpSlider*)widget;
    
    VaxpF32 w = widget->bounds.width;
    VaxpF32 h = widget->bounds.height;
    VaxpF32 cy = h / 2;
    
    VaxpF32 track_start = slider->thumb_radius;
    VaxpF32 track_end = w - slider->thumb_radius;
    VaxpF32 track_width = track_end - track_start;
    
    VaxpF32 normalized = slider_normalize(slider);
    VaxpF32 thumb_x = track_start + normalized * track_width;
    
    /* Draw track background */
    VaxpRectF track_bg = { 
        track_start, cy - slider->track_height / 2, 
        track_width, slider->track_height 
    };
    VaxpPaint track_paint = vaxp_paint_fill(slider->track_color);
    vaxp_canvas_draw_rounded_rect(canvas, track_bg, slider->track_height / 2, &track_paint);
    
    /* Draw filled portion */
    if (normalized > 0) {
        VaxpRectF track_fill = { 
            track_start, cy - slider->track_height / 2, 
            normalized * track_width, slider->track_height 
        };
        VaxpPaint fill_paint = vaxp_paint_fill(slider->fill_color);
        vaxp_canvas_draw_rounded_rect(canvas, track_fill, slider->track_height / 2, &fill_paint);
    }
    
    /* Draw focus ring */
    if (widget->state & VAXP_WIDGET_STATE_FOCUSED) {
        VaxpPaint focus_paint = vaxp_paint_stroke((VaxpColor){ 63, 81, 181, 100 }, 3.0f);
        vaxp_canvas_draw_circle(canvas, thumb_x, cy, slider->thumb_radius + 4, &focus_paint);
    }
    
    /* Draw thumb shadow */
    VaxpPaint shadow = vaxp_paint_fill((VaxpColor){ 0, 0, 0, 30 });
    vaxp_canvas_draw_circle(canvas, thumb_x + 1, cy + 2, slider->thumb_radius, &shadow);
    
    /* Draw thumb */
    VaxpPaint thumb_paint = vaxp_paint_fill(slider->thumb_color);
    vaxp_canvas_draw_circle(canvas, thumb_x, cy, slider->thumb_radius, &thumb_paint);
}

static VaxpBool slider_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpSlider* slider = (VaxpSlider*)widget;
    
    if (!slider->enabled) return VAXP_FALSE;
    
    switch (event->type) {
        case VAXP_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
                slider->dragging = VAXP_TRUE;
                slider_set_from_pos(slider, (VaxpF32)event->mouse.x);
                return VAXP_TRUE;
            }
            break;
            
        case VAXP_EVENT_MOUSE_BUTTON_UP:
            if (slider->dragging) {
                slider->dragging = VAXP_FALSE;
                return VAXP_TRUE;
            }
            break;
            
        case VAXP_EVENT_MOUSE_MOVE:
            if (slider->dragging) {
                slider_set_from_pos(slider, (VaxpF32)event->mouse.x);
                return VAXP_TRUE;
            }
            break;
            
        case VAXP_EVENT_KEY_DOWN:
            switch (event->key.key) {
                case VAXP_KEY_LEFT:
                case VAXP_KEY_DOWN: {
                    VaxpF32 step = slider->step > 0 ? slider->step : 
                                    (slider->max - slider->min) / 20;
                    slider->value -= step;
                    if (slider->value < slider->min) slider->value = slider->min;
                    widget->needs_redraw = VAXP_TRUE;
                    if (slider->on_change) {
                        slider->on_change(slider, slider->value, slider->callback_data);
                    }
                    return VAXP_TRUE;
                }
                case VAXP_KEY_RIGHT:
                case VAXP_KEY_UP: {
                    VaxpF32 step = slider->step > 0 ? slider->step : 
                                    (slider->max - slider->min) / 20;
                    slider->value += step;
                    if (slider->value > slider->max) slider->value = slider->max;
                    widget->needs_redraw = VAXP_TRUE;
                    if (slider->on_change) {
                        slider->on_change(slider, slider->value, slider->callback_data);
                    }
                    return VAXP_TRUE;
                }
                default:
                    break;
            }
            break;
            
        default:
            break;
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_slider_class = {
    .class_name = "VaxpSlider",
    .instance_size = sizeof(VaxpSlider),
    .parent_class = &vaxp_widget_class,
    .init = slider_init,
    .destroy = NULL,
    .measure = slider_measure,
    .layout = NULL,
    .draw = slider_draw,
    .on_event = slider_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_slider_create(void) {
    return vaxp_widget_create(&vaxp_slider_class);
}

void vaxp_slider_set_value(VaxpSlider* slider, VaxpF32 value) {
    if (!slider) return;
    
    if (value < slider->min) value = slider->min;
    if (value > slider->max) value = slider->max;
    
    slider->value = value;
    vaxp_widget_invalidate((VaxpWidget*)slider);
}

VaxpF32 vaxp_slider_get_value(const VaxpSlider* slider) {
    return slider ? slider->value : 0;
}

void vaxp_slider_set_range(VaxpSlider* slider, VaxpF32 min, VaxpF32 max) {
    if (slider) {
        slider->min = min;
        slider->max = max;
        if (slider->value < min) slider->value = min;
        if (slider->value > max) slider->value = max;
        vaxp_widget_invalidate((VaxpWidget*)slider);
    }
}

void vaxp_slider_set_step(VaxpSlider* slider, VaxpF32 step) {
    if (slider) slider->step = step;
}

void vaxp_slider_set_on_change(VaxpSlider* slider, VaxpSliderCallback callback, void* data) {
    if (slider) {
        slider->on_change = callback;
        slider->callback_data = data;
    }
}

VaxpWidget* _vaxp_slider_build(const VaxpSliderConfig* config) {
    VaxpResultPtr result = vaxp_slider_create();
    if (!result.ok) return NULL;
    
    VaxpSlider* slider = (VaxpSlider*)result.value;
    
    if (config->max > config->min) {
        slider->min = config->min;
        slider->max = config->max;
    }
    slider->value = config->value;
    slider->step = config->step;
    slider->on_change = config->on_change;
    slider->callback_data = config->data;
    
    return (VaxpWidget*)slider;
}
