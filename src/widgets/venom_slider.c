/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_slider.c - Slider widget implementation
 */

#include "venom/widgets/venom_slider.h"
#include "venom/core/venom_memory.h"

#define DEFAULT_TRACK_HEIGHT 4.0f
#define DEFAULT_THUMB_RADIUS 10.0f

static void slider_init(VenomWidget* widget) {
    VenomSlider* slider = (VenomSlider*)widget;
    
    slider->value = 0.0f;
    slider->min = 0.0f;
    slider->max = 1.0f;
    slider->step = 0;
    
    slider->enabled = VENOM_TRUE;
    slider->dragging = VENOM_FALSE;
    
    slider->track_height = DEFAULT_TRACK_HEIGHT;
    slider->thumb_radius = DEFAULT_THUMB_RADIUS;
    slider->track_color = (VenomColor){ 200, 200, 200, 255 };
    slider->fill_color = (VenomColor){ 63, 81, 181, 255 };
    slider->thumb_color = (VenomColor){ 63, 81, 181, 255 };
    
    slider->on_change = NULL;
    slider->callback_data = NULL;
    
    widget->focusable = VENOM_TRUE;
    widget->layout.preferred_width = 200;
    widget->layout.preferred_height = 30;
}

static void slider_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                           VenomF32* out_width, VenomF32* out_height) {
    VenomSlider* slider = (VenomSlider*)widget;
    (void)available_height;
    
    *out_width = widget->layout.preferred_width > 0 ? 
                 widget->layout.preferred_width : available_width;
    *out_height = slider->thumb_radius * 2 + 10;
}

static VenomF32 slider_normalize(const VenomSlider* slider) {
    if (slider->max <= slider->min) return 0;
    return (slider->value - slider->min) / (slider->max - slider->min);
}

static void slider_set_from_pos(VenomSlider* slider, VenomF32 x) {
    VenomF32 track_start = slider->thumb_radius;
    VenomF32 track_end = ((VenomWidget*)slider)->bounds.width - slider->thumb_radius;
    VenomF32 track_width = track_end - track_start;
    
    VenomF32 normalized = (x - track_start) / track_width;
    if (normalized < 0) normalized = 0;
    if (normalized > 1) normalized = 1;
    
    VenomF32 new_value = slider->min + normalized * (slider->max - slider->min);
    
    /* Apply step */
    if (slider->step > 0) {
        new_value = slider->min + 
                    (int)((new_value - slider->min) / slider->step + 0.5f) * slider->step;
    }
    
    if (new_value != slider->value) {
        slider->value = new_value;
        ((VenomWidget*)slider)->needs_redraw = VENOM_TRUE;
        
        if (slider->on_change) {
            slider->on_change(slider, slider->value, slider->callback_data);
        }
    }
}

static void slider_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomSlider* slider = (VenomSlider*)widget;
    
    VenomF32 w = widget->bounds.width;
    VenomF32 h = widget->bounds.height;
    VenomF32 cy = h / 2;
    
    VenomF32 track_start = slider->thumb_radius;
    VenomF32 track_end = w - slider->thumb_radius;
    VenomF32 track_width = track_end - track_start;
    
    VenomF32 normalized = slider_normalize(slider);
    VenomF32 thumb_x = track_start + normalized * track_width;
    
    /* Draw track background */
    VenomRectF track_bg = { 
        track_start, cy - slider->track_height / 2, 
        track_width, slider->track_height 
    };
    VenomPaint track_paint = venom_paint_fill(slider->track_color);
    venom_canvas_draw_rounded_rect(canvas, track_bg, slider->track_height / 2, &track_paint);
    
    /* Draw filled portion */
    if (normalized > 0) {
        VenomRectF track_fill = { 
            track_start, cy - slider->track_height / 2, 
            normalized * track_width, slider->track_height 
        };
        VenomPaint fill_paint = venom_paint_fill(slider->fill_color);
        venom_canvas_draw_rounded_rect(canvas, track_fill, slider->track_height / 2, &fill_paint);
    }
    
    /* Draw focus ring */
    if (widget->state & VENOM_WIDGET_STATE_FOCUSED) {
        VenomPaint focus_paint = venom_paint_stroke((VenomColor){ 63, 81, 181, 100 }, 3.0f);
        venom_canvas_draw_circle(canvas, thumb_x, cy, slider->thumb_radius + 4, &focus_paint);
    }
    
    /* Draw thumb shadow */
    VenomPaint shadow = venom_paint_fill((VenomColor){ 0, 0, 0, 30 });
    venom_canvas_draw_circle(canvas, thumb_x + 1, cy + 2, slider->thumb_radius, &shadow);
    
    /* Draw thumb */
    VenomPaint thumb_paint = venom_paint_fill(slider->thumb_color);
    venom_canvas_draw_circle(canvas, thumb_x, cy, slider->thumb_radius, &thumb_paint);
}

static VenomBool slider_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomSlider* slider = (VenomSlider*)widget;
    
    if (!slider->enabled) return VENOM_FALSE;
    
    switch (event->type) {
        case VENOM_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
                slider->dragging = VENOM_TRUE;
                slider_set_from_pos(slider, (VenomF32)event->mouse.x);
                return VENOM_TRUE;
            }
            break;
            
        case VENOM_EVENT_MOUSE_BUTTON_UP:
            if (slider->dragging) {
                slider->dragging = VENOM_FALSE;
                return VENOM_TRUE;
            }
            break;
            
        case VENOM_EVENT_MOUSE_MOVE:
            if (slider->dragging) {
                slider_set_from_pos(slider, (VenomF32)event->mouse.x);
                return VENOM_TRUE;
            }
            break;
            
        case VENOM_EVENT_KEY_DOWN:
            switch (event->key.key) {
                case VENOM_KEY_LEFT:
                case VENOM_KEY_DOWN: {
                    VenomF32 step = slider->step > 0 ? slider->step : 
                                    (slider->max - slider->min) / 20;
                    slider->value -= step;
                    if (slider->value < slider->min) slider->value = slider->min;
                    widget->needs_redraw = VENOM_TRUE;
                    if (slider->on_change) {
                        slider->on_change(slider, slider->value, slider->callback_data);
                    }
                    return VENOM_TRUE;
                }
                case VENOM_KEY_RIGHT:
                case VENOM_KEY_UP: {
                    VenomF32 step = slider->step > 0 ? slider->step : 
                                    (slider->max - slider->min) / 20;
                    slider->value += step;
                    if (slider->value > slider->max) slider->value = slider->max;
                    widget->needs_redraw = VENOM_TRUE;
                    if (slider->on_change) {
                        slider->on_change(slider, slider->value, slider->callback_data);
                    }
                    return VENOM_TRUE;
                }
                default:
                    break;
            }
            break;
            
        default:
            break;
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_slider_class = {
    .class_name = "VenomSlider",
    .instance_size = sizeof(VenomSlider),
    .parent_class = &venom_widget_class,
    .init = slider_init,
    .destroy = NULL,
    .measure = slider_measure,
    .layout = NULL,
    .draw = slider_draw,
    .on_event = slider_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_slider_create(void) {
    return venom_widget_create(&venom_slider_class);
}

void venom_slider_set_value(VenomSlider* slider, VenomF32 value) {
    if (!slider) return;
    
    if (value < slider->min) value = slider->min;
    if (value > slider->max) value = slider->max;
    
    slider->value = value;
    venom_widget_invalidate((VenomWidget*)slider);
}

VenomF32 venom_slider_get_value(const VenomSlider* slider) {
    return slider ? slider->value : 0;
}

void venom_slider_set_range(VenomSlider* slider, VenomF32 min, VenomF32 max) {
    if (slider) {
        slider->min = min;
        slider->max = max;
        if (slider->value < min) slider->value = min;
        if (slider->value > max) slider->value = max;
        venom_widget_invalidate((VenomWidget*)slider);
    }
}

void venom_slider_set_step(VenomSlider* slider, VenomF32 step) {
    if (slider) slider->step = step;
}

void venom_slider_set_on_change(VenomSlider* slider, VenomSliderCallback callback, void* data) {
    if (slider) {
        slider->on_change = callback;
        slider->callback_data = data;
    }
}

VenomWidget* _venom_slider_build(const VenomSliderConfig* config) {
    VenomResultPtr result = venom_slider_create();
    if (!result.ok) return NULL;
    
    VenomSlider* slider = (VenomSlider*)result.value;
    
    if (config->max > config->min) {
        slider->min = config->min;
        slider->max = config->max;
    }
    slider->value = config->value;
    slider->step = config->step;
    slider->on_change = config->on_change;
    slider->callback_data = config->data;
    
    return (VenomWidget*)slider;
}
