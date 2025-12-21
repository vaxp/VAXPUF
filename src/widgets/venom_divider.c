/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_divider.c - Divider widget implementation
 */

#include "venom/widgets/venom_divider.h"
#include "venom/core/venom_memory.h"

static void divider_init(VenomWidget* widget) {
    VenomDivider* div = (VenomDivider*)widget;
    div->direction = VENOM_DIVIDER_HORIZONTAL;
    div->thickness = 1.0f;
    div->color = (VenomColor){ 200, 200, 200, 255 };
    div->indent_start = 0;
    div->indent_end = 0;
}

static void divider_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                            VenomF32* out_width, VenomF32* out_height) {
    VenomDivider* div = (VenomDivider*)widget;
    
    if (div->direction == VENOM_DIVIDER_HORIZONTAL) {
        *out_width = available_width;
        *out_height = div->thickness;
    } else {
        *out_width = div->thickness;
        *out_height = available_height;
    }
}

static void divider_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomDivider* div = (VenomDivider*)widget;
    VenomPaint paint = venom_paint_fill(div->color);
    
    VenomRectF rect;
    if (div->direction == VENOM_DIVIDER_HORIZONTAL) {
        rect = (VenomRectF){
            div->indent_start, 0,
            widget->bounds.width - div->indent_start - div->indent_end,
            div->thickness
        };
    } else {
        rect = (VenomRectF){
            0, div->indent_start,
            div->thickness,
            widget->bounds.height - div->indent_start - div->indent_end
        };
    }
    
    venom_canvas_draw_rect(canvas, rect, &paint);
}

const VenomWidgetClass venom_divider_class = {
    .class_name = "VenomDivider",
    .instance_size = sizeof(VenomDivider),
    .parent_class = &venom_widget_class,
    .init = divider_init,
    .destroy = NULL,
    .measure = divider_measure,
    .layout = NULL,
    .draw = divider_draw,
    .on_event = NULL,
    .on_state_changed = NULL,
};

VenomResultPtr venom_divider_create(void) {
    return venom_widget_create(&venom_divider_class);
}

void venom_divider_set_direction(VenomDivider* div, VenomDividerDirection dir) {
    if (div) {
        div->direction = dir;
        venom_widget_invalidate((VenomWidget*)div);
    }
}

void venom_divider_set_thickness(VenomDivider* div, VenomF32 thickness) {
    if (div) {
        div->thickness = thickness;
        venom_widget_invalidate((VenomWidget*)div);
    }
}

void venom_divider_set_color(VenomDivider* div, VenomColor color) {
    if (div) {
        div->color = color;
        venom_widget_invalidate((VenomWidget*)div);
    }
}

void venom_divider_set_indent(VenomDivider* div, VenomF32 start, VenomF32 end) {
    if (div) {
        div->indent_start = start;
        div->indent_end = end;
        venom_widget_invalidate((VenomWidget*)div);
    }
}

VenomWidget* _venom_divider_build(const VenomDividerConfig* config) {
    VenomResultPtr result = venom_divider_create();
    if (!result.ok) return NULL;
    
    VenomDivider* div = (VenomDivider*)result.value;
    
    if (config->direction) div->direction = config->direction;
    if (config->thickness > 0) div->thickness = config->thickness;
    if (config->color.a > 0) div->color = config->color;
    div->indent_start = config->indent_start;
    div->indent_end = config->indent_end;
    
    return (VenomWidget*)div;
}
