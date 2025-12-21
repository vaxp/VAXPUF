/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_switch.c - Switch widget implementation
 */

#include "venom/widgets/venom_switch.h"
#include "venom/core/venom_memory.h"

#define DEFAULT_WIDTH 50.0f
#define DEFAULT_HEIGHT 28.0f

static void switch_init(VenomWidget* widget) {
    VenomSwitch* sw = (VenomSwitch*)widget;
    
    sw->on = VENOM_FALSE;
    sw->enabled = VENOM_TRUE;
    
    sw->width = DEFAULT_WIDTH;
    sw->height = DEFAULT_HEIGHT;
    sw->on_color = (VenomColor){ 76, 175, 80, 255 };    /* Green */
    sw->off_color = (VenomColor){ 189, 189, 189, 255 }; /* Gray */
    sw->thumb_color = (VenomColor){ 255, 255, 255, 255 };
    
    sw->on_change = NULL;
    sw->callback_data = NULL;
    
    widget->focusable = VENOM_TRUE;
}

static void switch_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                           VenomF32* out_width, VenomF32* out_height) {
    VenomSwitch* sw = (VenomSwitch*)widget;
    (void)available_width; (void)available_height;
    
    *out_width = sw->width;
    *out_height = sw->height;
}

static void switch_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomSwitch* sw = (VenomSwitch*)widget;
    
    VenomF32 radius = sw->height / 2;
    VenomColor track_color = sw->on ? sw->on_color : sw->off_color;
    
    if (!sw->enabled) {
        track_color.a = 128;
    }
    
    /* Draw track (rounded rectangle) */
    VenomRectF track = { 0, 0, sw->width, sw->height };
    VenomPaint track_paint = venom_paint_fill(track_color);
    venom_canvas_draw_rounded_rect(canvas, track, radius, &track_paint);
    
    /* Draw focus ring */
    if (widget->state & VENOM_WIDGET_STATE_FOCUSED) {
        VenomPaint focus_paint = venom_paint_stroke((VenomColor){ 63, 81, 181, 200 }, 2.0f);
        VenomRectF focus_rect = { -2, -2, sw->width + 4, sw->height + 4 };
        venom_canvas_draw_rounded_rect(canvas, focus_rect, radius + 2, &focus_paint);
    }
    
    /* Draw thumb (circle) */
    VenomF32 thumb_radius = radius - 4;
    VenomF32 thumb_x = sw->on ? (sw->width - radius) : radius;
    VenomF32 thumb_y = sw->height / 2;
    
    VenomColor tc = sw->thumb_color;
    if (!sw->enabled) tc.a = 200;
    
    VenomPaint thumb_paint = venom_paint_fill(tc);
    venom_canvas_draw_circle(canvas, thumb_x, thumb_y, thumb_radius, &thumb_paint);
    
    /* Drop shadow on thumb */
    VenomPaint shadow = venom_paint_fill((VenomColor){ 0, 0, 0, 30 });
    venom_canvas_draw_circle(canvas, thumb_x + 1, thumb_y + 2, thumb_radius, &shadow);
}

static VenomBool switch_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomSwitch* sw = (VenomSwitch*)widget;
    
    if (!sw->enabled) return VENOM_FALSE;
    
    switch (event->type) {
        case VENOM_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
                sw->on = !sw->on;
                widget->needs_redraw = VENOM_TRUE;
                
                if (sw->on_change) {
                    sw->on_change(sw, sw->on, sw->callback_data);
                }
                return VENOM_TRUE;
            }
            break;
            
        case VENOM_EVENT_KEY_DOWN:
            if (event->key.key == VENOM_KEY_SPACE || event->key.key == VENOM_KEY_RETURN) {
                sw->on = !sw->on;
                widget->needs_redraw = VENOM_TRUE;
                
                if (sw->on_change) {
                    sw->on_change(sw, sw->on, sw->callback_data);
                }
                return VENOM_TRUE;
            }
            break;
            
        default:
            break;
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_switch_class = {
    .class_name = "VenomSwitch",
    .instance_size = sizeof(VenomSwitch),
    .parent_class = &venom_widget_class,
    .init = switch_init,
    .destroy = NULL,
    .measure = switch_measure,
    .layout = NULL,
    .draw = switch_draw,
    .on_event = switch_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_switch_create(void) {
    return venom_widget_create(&venom_switch_class);
}

void venom_switch_set_on(VenomSwitch* sw, VenomBool on) {
    if (sw) {
        sw->on = on;
        venom_widget_invalidate((VenomWidget*)sw);
    }
}

VenomBool venom_switch_is_on(const VenomSwitch* sw) {
    return sw ? sw->on : VENOM_FALSE;
}

void venom_switch_set_enabled(VenomSwitch* sw, VenomBool enabled) {
    if (sw) {
        sw->enabled = enabled;
        venom_widget_invalidate((VenomWidget*)sw);
    }
}

void venom_switch_set_on_change(VenomSwitch* sw, VenomSwitchCallback callback, void* data) {
    if (sw) {
        sw->on_change = callback;
        sw->callback_data = data;
    }
}

VenomWidget* _venom_switch_build(const VenomSwitchConfig* config) {
    VenomResultPtr result = venom_switch_create();
    if (!result.ok) return NULL;
    
    VenomSwitch* sw = (VenomSwitch*)result.value;
    sw->on = config->on;
    sw->on_change = config->on_change;
    sw->callback_data = config->data;
    
    return (VenomWidget*)sw;
}
