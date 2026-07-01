/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_color_swatch.c - Color swatch implementation
 */

#include "vaxp/widgets/vaxp_color_swatch.h"
#include "vaxp/core/vaxp_memory.h"

#define DEFAULT_SIZE 32.0f

static void color_swatch_init(VaxpWidget* widget) {
    VaxpColorSwatch* swatch = (VaxpColorSwatch*)widget;
    
    swatch->color = (VaxpColor){ 63, 81, 181, 255 };
    swatch->size = DEFAULT_SIZE;
    swatch->corner_radius = 4.0f;
    swatch->show_border = VAXP_TRUE;
    swatch->selectable = VAXP_FALSE;
    swatch->selected = VAXP_FALSE;
    
    swatch->on_click = NULL;
    swatch->callback_data = NULL;
    
    widget->focusable = VAXP_TRUE;
}

static void color_swatch_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                                 VaxpF32* out_width, VaxpF32* out_height) {
    VaxpColorSwatch* swatch = (VaxpColorSwatch*)widget;
    (void)available_width; (void)available_height;
    
    *out_width = swatch->size;
    *out_height = swatch->size;
}

static void color_swatch_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpColorSwatch* swatch = (VaxpColorSwatch*)widget;
    
    VaxpF32 s = swatch->size;
    
    /* Draw checkerboard for transparency */
    if (swatch->color.a < 255) {
        VaxpPaint light = vaxp_paint_fill((VaxpColor){ 255, 255, 255, 255 });
        VaxpPaint dark = vaxp_paint_fill((VaxpColor){ 200, 200, 200, 255 });
        
        VaxpRectF bg = { 0, 0, s, s };
        vaxp_canvas_draw_rounded_rect(canvas, bg, swatch->corner_radius, &light);
        
        VaxpF32 check_size = s / 4;
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                if ((x + y) % 2 == 1) {
                    VaxpRectF check = { x * check_size, y * check_size, check_size, check_size };
                    vaxp_canvas_draw_rect(canvas, check, &dark);
                }
            }
        }
    }
    
    /* Draw color */
    VaxpRectF rect = { 0, 0, s, s };
    VaxpPaint color_paint = vaxp_paint_fill(swatch->color);
    vaxp_canvas_draw_rounded_rect(canvas, rect, swatch->corner_radius, &color_paint);
    
    /* Draw border */
    if (swatch->show_border) {
        VaxpColor border = swatch->selected ? 
            (VaxpColor){ 63, 81, 181, 255 } : (VaxpColor){ 180, 180, 180, 255 };
        VaxpF32 border_width = swatch->selected ? 3.0f : 1.0f;
        VaxpPaint border_paint = vaxp_paint_stroke(border, border_width);
        vaxp_canvas_draw_rounded_rect(canvas, rect, swatch->corner_radius, &border_paint);
    }
}

static VaxpBool color_swatch_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpColorSwatch* swatch = (VaxpColorSwatch*)widget;
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN && 
        event->mouse.button == VAXP_MOUSE_BUTTON_LEFT) {
        
        if (swatch->selectable) {
            swatch->selected = !swatch->selected;
            widget->needs_redraw = VAXP_TRUE;
        }
        
        if (swatch->on_click) {
            swatch->on_click(swatch, swatch->color, swatch->callback_data);
        }
        return VAXP_TRUE;
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_color_swatch_class = {
    .class_name = "VaxpColorSwatch",
    .instance_size = sizeof(VaxpColorSwatch),
    .parent_class = &vaxp_widget_class,
    .init = color_swatch_init,
    .destroy = NULL,
    .measure = color_swatch_measure,
    .layout = NULL,
    .draw = color_swatch_draw,
    .on_event = color_swatch_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_color_swatch_create(void) {
    return vaxp_widget_create(&vaxp_color_swatch_class);
}

void vaxp_color_swatch_set_color(VaxpColorSwatch* swatch, VaxpColor color) {
    if (swatch) {
        swatch->color = color;
        vaxp_widget_invalidate((VaxpWidget*)swatch);
    }
}

VaxpColor vaxp_color_swatch_get_color(const VaxpColorSwatch* swatch) {
    return swatch ? swatch->color : (VaxpColor){ 0, 0, 0, 0 };
}

void vaxp_color_swatch_set_size(VaxpColorSwatch* swatch, VaxpF32 size) {
    if (swatch && size > 0) {
        swatch->size = size;
        vaxp_widget_invalidate((VaxpWidget*)swatch);
    }
}

void vaxp_color_swatch_set_on_click(VaxpColorSwatch* swatch, VaxpColorSwatchCallback callback, void* data) {
    if (swatch) {
        swatch->on_click = callback;
        swatch->callback_data = data;
    }
}

VaxpWidget* _vaxp_color_swatch_build(const VaxpColorSwatchConfig* config) {
    VaxpResultPtr result = vaxp_color_swatch_create();
    if (!result.ok) return NULL;
    
    VaxpColorSwatch* swatch = (VaxpColorSwatch*)result.value;
    
    if (config->color.a > 0) swatch->color = config->color;
    if (config->size > 0) swatch->size = config->size;
    swatch->selectable = config->selectable;
    swatch->on_click = config->on_click;
    swatch->callback_data = config->data;
    
    return (VaxpWidget*)swatch;
}
