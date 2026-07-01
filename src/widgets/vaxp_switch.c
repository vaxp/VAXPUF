/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_switch.c - Switch widget implementation
 */

#include "vaxp/widgets/vaxp_switch.h"
#include "vaxp/core/vaxp_memory.h"

#define DEFAULT_WIDTH 50.0f
#define DEFAULT_HEIGHT 28.0f

static void switch_init(VaxpWidget* widget) {
    VaxpSwitch* sw = (VaxpSwitch*)widget;
    
    sw->on = VAXP_FALSE;
    sw->enabled = VAXP_TRUE;
    
    sw->width = DEFAULT_WIDTH;
    sw->height = DEFAULT_HEIGHT;
    sw->on_color = (VaxpColor){ 76, 175, 80, 255 };    /* Green */
    sw->off_color = (VaxpColor){ 189, 189, 189, 255 }; /* Gray */
    sw->thumb_color = (VaxpColor){ 255, 255, 255, 255 };
    
    sw->on_change = NULL;
    sw->callback_data = NULL;
    
    widget->focusable = VAXP_TRUE;
}

static void switch_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                           VaxpF32* out_width, VaxpF32* out_height) {
    VaxpSwitch* sw = (VaxpSwitch*)widget;
    (void)available_width; (void)available_height;
    
    *out_width = sw->width;
    *out_height = sw->height;
}

static void switch_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpSwitch* sw = (VaxpSwitch*)widget;
    
    VaxpF32 radius = sw->height / 2;
    VaxpColor track_color = sw->on ? sw->on_color : sw->off_color;
    
    if (!sw->enabled) {
        track_color.a = 128;
    }
    
    /* Draw track (rounded rectangle) */
    VaxpRectF track = { 0, 0, sw->width, sw->height };
    VaxpPaint track_paint = vaxp_paint_fill(track_color);
    vaxp_canvas_draw_rounded_rect(canvas, track, radius, &track_paint);
    
    /* Draw focus ring */
    if (widget->state & VAXP_WIDGET_STATE_FOCUSED) {
        VaxpPaint focus_paint = vaxp_paint_stroke((VaxpColor){ 63, 81, 181, 200 }, 2.0f);
        VaxpRectF focus_rect = { -2, -2, sw->width + 4, sw->height + 4 };
        vaxp_canvas_draw_rounded_rect(canvas, focus_rect, radius + 2, &focus_paint);
    }
    
    /* Draw thumb (circle) */
    VaxpF32 thumb_radius = radius - 4;
    VaxpF32 thumb_x = sw->on ? (sw->width - radius) : radius;
    VaxpF32 thumb_y = sw->height / 2;
    
    VaxpColor tc = sw->thumb_color;
    if (!sw->enabled) tc.a = 200;
    
    VaxpPaint thumb_paint = vaxp_paint_fill(tc);
    vaxp_canvas_draw_circle(canvas, thumb_x, thumb_y, thumb_radius, &thumb_paint);
    
    /* Drop shadow on thumb */
    VaxpPaint shadow = vaxp_paint_fill((VaxpColor){ 0, 0, 0, 30 });
    vaxp_canvas_draw_circle(canvas, thumb_x + 1, thumb_y + 2, thumb_radius, &shadow);
}

static VaxpBool switch_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpSwitch* sw = (VaxpSwitch*)widget;
    
    if (!sw->enabled) return VAXP_FALSE;
    
    switch (event->type) {
        case VAXP_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
                sw->on = !sw->on;
                widget->needs_redraw = VAXP_TRUE;
                
                if (sw->on_change) {
                    sw->on_change(sw, sw->on, sw->callback_data);
                }
                return VAXP_TRUE;
            }
            break;
            
        case VAXP_EVENT_KEY_DOWN:
            if (event->key.key == VAXP_KEY_SPACE || event->key.key == VAXP_KEY_RETURN) {
                sw->on = !sw->on;
                widget->needs_redraw = VAXP_TRUE;
                
                if (sw->on_change) {
                    sw->on_change(sw, sw->on, sw->callback_data);
                }
                return VAXP_TRUE;
            }
            break;
            
        default:
            break;
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_switch_class = {
    .class_name = "VaxpSwitch",
    .instance_size = sizeof(VaxpSwitch),
    .parent_class = &vaxp_widget_class,
    .init = switch_init,
    .destroy = NULL,
    .measure = switch_measure,
    .layout = NULL,
    .draw = switch_draw,
    .on_event = switch_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_switch_create(void) {
    return vaxp_widget_create(&vaxp_switch_class);
}

void vaxp_switch_set_on(VaxpSwitch* sw, VaxpBool on) {
    if (sw) {
        sw->on = on;
        vaxp_widget_invalidate((VaxpWidget*)sw);
    }
}

VaxpBool vaxp_switch_is_on(const VaxpSwitch* sw) {
    return sw ? sw->on : VAXP_FALSE;
}

void vaxp_switch_set_enabled(VaxpSwitch* sw, VaxpBool enabled) {
    if (sw) {
        sw->enabled = enabled;
        vaxp_widget_invalidate((VaxpWidget*)sw);
    }
}

void vaxp_switch_set_on_change(VaxpSwitch* sw, VaxpSwitchCallback callback, void* data) {
    if (sw) {
        sw->on_change = callback;
        sw->callback_data = data;
    }
}

VaxpWidget* _vaxp_switch_build(const VaxpSwitchConfig* config) {
    VaxpResultPtr result = vaxp_switch_create();
    if (!result.ok) return NULL;
    
    VaxpSwitch* sw = (VaxpSwitch*)result.value;
    sw->on = config->on;
    sw->on_change = config->on_change;
    sw->callback_data = config->data;
    
    return (VaxpWidget*)sw;
}
