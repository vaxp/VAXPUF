/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_icon.c - Icon widget implementation
 */

#include "venom/widgets/venom_icon.h"
#include "venom/core/venom_memory.h"
#include <string.h>

#define DEFAULT_SIZE 24.0f

static void icon_init(VenomWidget* widget) {
    VenomIcon* icon = (VenomIcon*)widget;
    
    icon->icon = NULL;
    icon->size = DEFAULT_SIZE;
    icon->color = (VenomColor){ 97, 97, 97, 255 };
}

static void icon_destroy(VenomWidget* widget) {
    VenomIcon* icon = (VenomIcon*)widget;
    
    if (icon->icon) {
        venom_free(icon->icon, strlen(icon->icon) + 1);
        icon->icon = NULL;
    }
    
    venom_widget_class.destroy(widget);
}

static void icon_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                         VenomF32* out_width, VenomF32* out_height) {
    VenomIcon* icon = (VenomIcon*)widget;
    (void)available_width; (void)available_height;
    
    *out_width = icon->size;
    *out_height = icon->size;
}

static void icon_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomIcon* icon = (VenomIcon*)widget;
    
    if (!icon->icon) return;
    
    VenomPaint paint = venom_paint_fill(icon->color);
    
    /* Center the icon */
    VenomF32 tx = icon->size * 0.15f;
    VenomF32 ty = icon->size * 0.75f;
    
    venom_canvas_draw_text(canvas, icon->icon, tx, ty, NULL, &paint);
}

const VenomWidgetClass venom_icon_class = {
    .class_name = "VenomIcon",
    .instance_size = sizeof(VenomIcon),
    .parent_class = &venom_widget_class,
    .init = icon_init,
    .destroy = icon_destroy,
    .measure = icon_measure,
    .layout = NULL,
    .draw = icon_draw,
    .on_event = NULL,
    .on_state_changed = NULL,
};

VenomResultPtr venom_icon_create(void) {
    return venom_widget_create(&venom_icon_class);
}

void venom_icon_set_icon(VenomIcon* icon, const char* icon_str) {
    if (!icon) return;
    
    if (icon->icon) {
        venom_free(icon->icon, strlen(icon->icon) + 1);
        icon->icon = NULL;
    }
    
    if (icon_str) {
        VenomSize len = strlen(icon_str) + 1;
        icon->icon = (char*)venom_alloc(len);
        if (icon->icon) memcpy(icon->icon, icon_str, len);
    }
    
    venom_widget_invalidate((VenomWidget*)icon);
}

void venom_icon_set_size(VenomIcon* icon, VenomF32 size) {
    if (icon && size > 0) {
        icon->size = size;
        venom_widget_invalidate((VenomWidget*)icon);
    }
}

void venom_icon_set_color(VenomIcon* icon, VenomColor color) {
    if (icon) {
        icon->color = color;
        venom_widget_invalidate((VenomWidget*)icon);
    }
}

VenomWidget* _venom_icon_build(const VenomIconConfig* config) {
    VenomResultPtr result = venom_icon_create();
    if (!result.ok) return NULL;
    
    VenomIcon* icon = (VenomIcon*)result.value;
    
    if (config->icon) venom_icon_set_icon(icon, config->icon);
    if (config->size > 0) icon->size = config->size;
    if (config->color.a > 0) icon->color = config->color;
    
    return (VenomWidget*)icon;
}
