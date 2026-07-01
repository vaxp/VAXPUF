/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_icon.c - Icon widget implementation
 */

#include "vaxp/widgets/vaxp_icon.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

#define DEFAULT_SIZE 24.0f

static void icon_init(VaxpWidget* widget) {
    VaxpIcon* icon = (VaxpIcon*)widget;
    
    icon->icon = NULL;
    icon->size = DEFAULT_SIZE;
    icon->color = (VaxpColor){ 97, 97, 97, 255 };
}

static void icon_destroy(VaxpWidget* widget) {
    VaxpIcon* icon = (VaxpIcon*)widget;
    
    if (icon->icon) {
        vaxp_free(icon->icon, strlen(icon->icon) + 1);
        icon->icon = NULL;
    }
    
    vaxp_widget_class.destroy(widget);
}

static void icon_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                         VaxpF32* out_width, VaxpF32* out_height) {
    VaxpIcon* icon = (VaxpIcon*)widget;
    (void)available_width; (void)available_height;
    
    *out_width = icon->size;
    *out_height = icon->size;
}

static void icon_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpIcon* icon = (VaxpIcon*)widget;
    
    if (!icon->icon) return;
    
    VaxpPaint paint = vaxp_paint_fill(icon->color);
    
    /* Center the icon */
    VaxpF32 tx = icon->size * 0.15f;
    VaxpF32 ty = icon->size * 0.75f;
    
    vaxp_canvas_draw_text(canvas, icon->icon, tx, ty, NULL, &paint);
}

const VaxpWidgetClass vaxp_icon_class = {
    .class_name = "VaxpIcon",
    .instance_size = sizeof(VaxpIcon),
    .parent_class = &vaxp_widget_class,
    .init = icon_init,
    .destroy = icon_destroy,
    .measure = icon_measure,
    .layout = NULL,
    .draw = icon_draw,
    .on_event = NULL,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_icon_create(void) {
    return vaxp_widget_create(&vaxp_icon_class);
}

void vaxp_icon_set_icon(VaxpIcon* icon, const char* icon_str) {
    if (!icon) return;
    
    if (icon->icon) {
        vaxp_free(icon->icon, strlen(icon->icon) + 1);
        icon->icon = NULL;
    }
    
    if (icon_str) {
        VaxpSize len = strlen(icon_str) + 1;
        icon->icon = (char*)vaxp_alloc(len);
        if (icon->icon) memcpy(icon->icon, icon_str, len);
    }
    
    vaxp_widget_invalidate((VaxpWidget*)icon);
}

void vaxp_icon_set_size(VaxpIcon* icon, VaxpF32 size) {
    if (icon && size > 0) {
        icon->size = size;
        vaxp_widget_invalidate((VaxpWidget*)icon);
    }
}

void vaxp_icon_set_color(VaxpIcon* icon, VaxpColor color) {
    if (icon) {
        icon->color = color;
        vaxp_widget_invalidate((VaxpWidget*)icon);
    }
}

VaxpWidget* _vaxp_icon_build(const VaxpIconConfig* config) {
    VaxpResultPtr result = vaxp_icon_create();
    if (!result.ok) return NULL;
    
    VaxpIcon* icon = (VaxpIcon*)result.value;
    
    if (config->icon) vaxp_icon_set_icon(icon, config->icon);
    if (config->size > 0) icon->size = config->size;
    if (config->color.a > 0) icon->color = config->color;
    
    return (VaxpWidget*)icon;
}
