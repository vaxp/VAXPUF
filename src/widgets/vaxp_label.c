/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_label.c - Label widget implementation
 */

#include "vaxp/widgets/vaxp_label.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

/* ============================================================================
 * LABEL CLASS METHODS
 * ============================================================================ */

static void label_init(VaxpWidget* widget) {
    VaxpLabel* label = (VaxpLabel*)widget;
    
    label->text_color = VAXP_COLOR_BLACK;
    label->font_size = 14.0f;
    label->align = VAXP_TEXT_ALIGN_LEFT;
    label->wrap = VAXP_FALSE;
}

static void label_destroy(VaxpWidget* widget) {
    VaxpLabel* label = (VaxpLabel*)widget;
    
    if (label->text) {
        vaxp_free(label->text, strlen(label->text) + 1);
        label->text = NULL;
    }
    
    vaxp_widget_class.destroy(widget);
}

static void label_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                           VaxpF32* out_width, VaxpF32* out_height) {
    VaxpLabel* label = (VaxpLabel*)widget;
    (void)available_width;
    (void)available_height;
    
    VaxpF32 text_width = 0;
    VaxpF32 text_height = label->font_size * 1.4f;
    
    if (label->text) {
        text_width = strlen(label->text) * (label->font_size * 1.0f);
    }
    
    *out_width = text_width + widget->layout.padding.left + widget->layout.padding.right;
    *out_height = text_height + widget->layout.padding.top + widget->layout.padding.bottom;
}

static void label_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpLabel* label = (VaxpLabel*)widget;
    
    if (!label->text) return;
    
    VaxpColor color = label->text_color;
    if (vaxp_widget_has_state(widget, VAXP_WIDGET_STATE_DISABLED)) {
        color.a = 128;  /* Semi-transparent when disabled */
    }
    
    /* Calculate text position based on alignment */
    VaxpF32 text_width = strlen(label->text) * (label->font_size * 1.0f);
    VaxpF32 x = widget->layout.padding.left;
    
    switch (label->align) {
        case VAXP_TEXT_ALIGN_CENTER:
            x = (widget->bounds.width - text_width) / 2;
            break;
        case VAXP_TEXT_ALIGN_RIGHT:
            x = widget->bounds.width - text_width - widget->layout.padding.right;
            break;
        default:
            break;
    }
    
    VaxpF32 y = (widget->bounds.height + label->font_size * 0.35f) / 2;
    
    VaxpPaint paint = vaxp_paint_fill(color);
    /* Use the font pointer to pass font_size for now, as VaxpFont is not implemented */
    vaxp_canvas_draw_text(canvas, label->text, x, y, (const VaxpFont*)&label->font_size, &paint);
}

/* ============================================================================
 * LABEL CLASS
 * ============================================================================ */

const VaxpWidgetClass vaxp_label_class = {
    .class_name = "VaxpLabel",
    .instance_size = sizeof(VaxpLabel),
    .parent_class = &vaxp_widget_class,
    .init = label_init,
    .destroy = label_destroy,
    .measure = label_measure,
    .layout = NULL,  /* Will use parent's at runtime */
    .draw = label_draw,
    .on_event = NULL,  /* Will use parent's at runtime */
    .on_state_changed = NULL,  /* Will use parent's at runtime */
};

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VaxpResultPtr vaxp_label_create(const char* text) {
    VaxpResultPtr result = vaxp_widget_create(&vaxp_label_class);
    if (!result.ok) return result;
    
    VaxpLabel* label = (VaxpLabel*)result.value;
    
    if (text) {
        VaxpResult r = vaxp_label_set_text(label, text);
        if (!r.ok) {
            vaxp_unref(label);
            return VAXP_ERR_PTR(r.error);
        }
    }
    
    return VAXP_OK_PTR(label);
}

VaxpResult vaxp_label_set_text(VaxpLabel* label, const char* text) {
    VAXP_ENSURE_NOT_NULL(label);
    
    if (label->text) {
        vaxp_free(label->text, strlen(label->text) + 1);
        label->text = NULL;
    }
    
    if (text) {
        VaxpSize len = strlen(text) + 1;
        label->text = (char*)vaxp_alloc(len);
        if (!label->text) {
            return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_MEMORY);
        }
        memcpy(label->text, text, len);
    }
    
    vaxp_widget_invalidate_layout((VaxpWidget*)label);
    return VAXP_OK_UNIT();
}

const char* vaxp_label_get_text(const VaxpLabel* label) {
    return label ? label->text : NULL;
}

void vaxp_label_set_color(VaxpLabel* label, VaxpColor color) {
    if (!label) return;
    label->text_color = color;
    vaxp_widget_invalidate((VaxpWidget*)label);
}

void vaxp_label_set_font_size(VaxpLabel* label, VaxpF32 size) {
    if (!label) return;
    label->font_size = size;
    vaxp_widget_invalidate_layout((VaxpWidget*)label);
}

void vaxp_label_set_align(VaxpLabel* label, VaxpTextAlign align) {
    if (!label) return;
    label->align = align;
    vaxp_widget_invalidate((VaxpWidget*)label);
}
