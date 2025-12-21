/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_color_swatch.c - Color swatch implementation
 */

#include "venom/widgets/venom_color_swatch.h"
#include "venom/core/venom_memory.h"

#define DEFAULT_SIZE 32.0f

static void color_swatch_init(VenomWidget* widget) {
    VenomColorSwatch* swatch = (VenomColorSwatch*)widget;
    
    swatch->color = (VenomColor){ 63, 81, 181, 255 };
    swatch->size = DEFAULT_SIZE;
    swatch->corner_radius = 4.0f;
    swatch->show_border = VENOM_TRUE;
    swatch->selectable = VENOM_FALSE;
    swatch->selected = VENOM_FALSE;
    
    swatch->on_click = NULL;
    swatch->callback_data = NULL;
    
    widget->focusable = VENOM_TRUE;
}

static void color_swatch_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                                 VenomF32* out_width, VenomF32* out_height) {
    VenomColorSwatch* swatch = (VenomColorSwatch*)widget;
    (void)available_width; (void)available_height;
    
    *out_width = swatch->size;
    *out_height = swatch->size;
}

static void color_swatch_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomColorSwatch* swatch = (VenomColorSwatch*)widget;
    
    VenomF32 s = swatch->size;
    
    /* Draw checkerboard for transparency */
    if (swatch->color.a < 255) {
        VenomPaint light = venom_paint_fill((VenomColor){ 255, 255, 255, 255 });
        VenomPaint dark = venom_paint_fill((VenomColor){ 200, 200, 200, 255 });
        
        VenomRectF bg = { 0, 0, s, s };
        venom_canvas_draw_rounded_rect(canvas, bg, swatch->corner_radius, &light);
        
        VenomF32 check_size = s / 4;
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                if ((x + y) % 2 == 1) {
                    VenomRectF check = { x * check_size, y * check_size, check_size, check_size };
                    venom_canvas_draw_rect(canvas, check, &dark);
                }
            }
        }
    }
    
    /* Draw color */
    VenomRectF rect = { 0, 0, s, s };
    VenomPaint color_paint = venom_paint_fill(swatch->color);
    venom_canvas_draw_rounded_rect(canvas, rect, swatch->corner_radius, &color_paint);
    
    /* Draw border */
    if (swatch->show_border) {
        VenomColor border = swatch->selected ? 
            (VenomColor){ 63, 81, 181, 255 } : (VenomColor){ 180, 180, 180, 255 };
        VenomF32 border_width = swatch->selected ? 3.0f : 1.0f;
        VenomPaint border_paint = venom_paint_stroke(border, border_width);
        venom_canvas_draw_rounded_rect(canvas, rect, swatch->corner_radius, &border_paint);
    }
}

static VenomBool color_swatch_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomColorSwatch* swatch = (VenomColorSwatch*)widget;
    
    if (event->type == VENOM_EVENT_MOUSE_BUTTON_DOWN && 
        event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
        
        if (swatch->selectable) {
            swatch->selected = !swatch->selected;
            widget->needs_redraw = VENOM_TRUE;
        }
        
        if (swatch->on_click) {
            swatch->on_click(swatch, swatch->color, swatch->callback_data);
        }
        return VENOM_TRUE;
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_color_swatch_class = {
    .class_name = "VenomColorSwatch",
    .instance_size = sizeof(VenomColorSwatch),
    .parent_class = &venom_widget_class,
    .init = color_swatch_init,
    .destroy = NULL,
    .measure = color_swatch_measure,
    .layout = NULL,
    .draw = color_swatch_draw,
    .on_event = color_swatch_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_color_swatch_create(void) {
    return venom_widget_create(&venom_color_swatch_class);
}

void venom_color_swatch_set_color(VenomColorSwatch* swatch, VenomColor color) {
    if (swatch) {
        swatch->color = color;
        venom_widget_invalidate((VenomWidget*)swatch);
    }
}

VenomColor venom_color_swatch_get_color(const VenomColorSwatch* swatch) {
    return swatch ? swatch->color : (VenomColor){ 0, 0, 0, 0 };
}

void venom_color_swatch_set_size(VenomColorSwatch* swatch, VenomF32 size) {
    if (swatch && size > 0) {
        swatch->size = size;
        venom_widget_invalidate((VenomWidget*)swatch);
    }
}

void venom_color_swatch_set_on_click(VenomColorSwatch* swatch, VenomColorSwatchCallback callback, void* data) {
    if (swatch) {
        swatch->on_click = callback;
        swatch->callback_data = data;
    }
}

VenomWidget* _venom_color_swatch_build(const VenomColorSwatchConfig* config) {
    VenomResultPtr result = venom_color_swatch_create();
    if (!result.ok) return NULL;
    
    VenomColorSwatch* swatch = (VenomColorSwatch*)result.value;
    
    if (config->color.a > 0) swatch->color = config->color;
    if (config->size > 0) swatch->size = config->size;
    swatch->selectable = config->selectable;
    swatch->on_click = config->on_click;
    swatch->callback_data = config->data;
    
    return (VenomWidget*)swatch;
}
