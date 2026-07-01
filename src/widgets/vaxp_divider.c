/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_divider.c - Divider widget implementation
 */

#include "vaxp/widgets/vaxp_divider.h"
#include "vaxp/core/vaxp_memory.h"

static void divider_init(VaxpWidget* widget) {
    VaxpDivider* div = (VaxpDivider*)widget;
    div->direction = VAXP_DIVIDER_HORIZONTAL;
    div->thickness = 1.0f;
    div->color = (VaxpColor){ 200, 200, 200, 255 };
    div->indent_start = 0;
    div->indent_end = 0;
}

static void divider_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                            VaxpF32* out_width, VaxpF32* out_height) {
    VaxpDivider* div = (VaxpDivider*)widget;
    
    if (div->direction == VAXP_DIVIDER_HORIZONTAL) {
        *out_width = available_width;
        *out_height = div->thickness;
    } else {
        *out_width = div->thickness;
        *out_height = available_height;
    }
}

static void divider_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpDivider* div = (VaxpDivider*)widget;
    VaxpPaint paint = vaxp_paint_fill(div->color);
    
    VaxpRectF rect;
    if (div->direction == VAXP_DIVIDER_HORIZONTAL) {
        rect = (VaxpRectF){
            div->indent_start, 0,
            widget->bounds.width - div->indent_start - div->indent_end,
            div->thickness
        };
    } else {
        rect = (VaxpRectF){
            0, div->indent_start,
            div->thickness,
            widget->bounds.height - div->indent_start - div->indent_end
        };
    }
    
    vaxp_canvas_draw_rect(canvas, rect, &paint);
}

const VaxpWidgetClass vaxp_divider_class = {
    .class_name = "VaxpDivider",
    .instance_size = sizeof(VaxpDivider),
    .parent_class = &vaxp_widget_class,
    .init = divider_init,
    .destroy = NULL,
    .measure = divider_measure,
    .layout = NULL,
    .draw = divider_draw,
    .on_event = NULL,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_divider_create(void) {
    return vaxp_widget_create(&vaxp_divider_class);
}

void vaxp_divider_set_direction(VaxpDivider* div, VaxpDividerDirection dir) {
    if (div) {
        div->direction = dir;
        vaxp_widget_invalidate((VaxpWidget*)div);
    }
}

void vaxp_divider_set_thickness(VaxpDivider* div, VaxpF32 thickness) {
    if (div) {
        div->thickness = thickness;
        vaxp_widget_invalidate((VaxpWidget*)div);
    }
}

void vaxp_divider_set_color(VaxpDivider* div, VaxpColor color) {
    if (div) {
        div->color = color;
        vaxp_widget_invalidate((VaxpWidget*)div);
    }
}

void vaxp_divider_set_indent(VaxpDivider* div, VaxpF32 start, VaxpF32 end) {
    if (div) {
        div->indent_start = start;
        div->indent_end = end;
        vaxp_widget_invalidate((VaxpWidget*)div);
    }
}

VaxpWidget* _vaxp_divider_build(const VaxpDividerConfig* config) {
    VaxpResultPtr result = vaxp_divider_create();
    if (!result.ok) return NULL;
    
    VaxpDivider* div = (VaxpDivider*)result.value;
    
    if (config->direction) div->direction = config->direction;
    if (config->thickness > 0) div->thickness = config->thickness;
    if (config->color.a > 0) div->color = config->color;
    div->indent_start = config->indent_start;
    div->indent_end = config->indent_end;
    
    return (VaxpWidget*)div;
}
