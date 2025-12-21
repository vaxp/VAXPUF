/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_label.c - Label widget implementation
 */

#include "venom/widgets/venom_label.h"
#include "venom/core/venom_memory.h"
#include <string.h>

/* ============================================================================
 * LABEL CLASS METHODS
 * ============================================================================ */

static void label_init(VenomWidget* widget) {
    VenomLabel* label = (VenomLabel*)widget;
    
    label->text_color = VENOM_COLOR_BLACK;
    label->font_size = 14.0f;
    label->align = VENOM_TEXT_ALIGN_LEFT;
    label->wrap = VENOM_FALSE;
}

static void label_destroy(VenomWidget* widget) {
    VenomLabel* label = (VenomLabel*)widget;
    
    if (label->text) {
        venom_free(label->text, strlen(label->text) + 1);
        label->text = NULL;
    }
    
    venom_widget_class.destroy(widget);
}

static void label_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                           VenomF32* out_width, VenomF32* out_height) {
    VenomLabel* label = (VenomLabel*)widget;
    (void)available_width;
    (void)available_height;
    
    VenomF32 text_width = 0;
    VenomF32 text_height = label->font_size * 1.2f;
    
    if (label->text) {
        text_width = strlen(label->text) * (label->font_size * 0.5f);
    }
    
    *out_width = text_width + widget->layout.padding.left + widget->layout.padding.right;
    *out_height = text_height + widget->layout.padding.top + widget->layout.padding.bottom;
}

static void label_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomLabel* label = (VenomLabel*)widget;
    
    if (!label->text) return;
    
    VenomColor color = label->text_color;
    if (venom_widget_has_state(widget, VENOM_WIDGET_STATE_DISABLED)) {
        color.a = 128;  /* Semi-transparent when disabled */
    }
    
    /* Calculate text position based on alignment */
    VenomF32 text_width = strlen(label->text) * (label->font_size * 0.5f);
    VenomF32 x = widget->layout.padding.left;
    
    switch (label->align) {
        case VENOM_TEXT_ALIGN_CENTER:
            x = (widget->bounds.width - text_width) / 2;
            break;
        case VENOM_TEXT_ALIGN_RIGHT:
            x = widget->bounds.width - text_width - widget->layout.padding.right;
            break;
        default:
            break;
    }
    
    VenomF32 y = (widget->bounds.height + label->font_size * 0.35f) / 2;
    
    VenomPaint paint = venom_paint_fill(color);
    venom_canvas_draw_text(canvas, label->text, x, y, NULL, &paint);
}

/* ============================================================================
 * LABEL CLASS
 * ============================================================================ */

const VenomWidgetClass venom_label_class = {
    .class_name = "VenomLabel",
    .instance_size = sizeof(VenomLabel),
    .parent_class = &venom_widget_class,
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

VenomResultPtr venom_label_create(const char* text) {
    VenomResultPtr result = venom_widget_create(&venom_label_class);
    if (!result.ok) return result;
    
    VenomLabel* label = (VenomLabel*)result.value;
    
    if (text) {
        VenomResult r = venom_label_set_text(label, text);
        if (!r.ok) {
            venom_unref(label);
            return VENOM_ERR_PTR(r.error);
        }
    }
    
    return VENOM_OK_PTR(label);
}

VenomResult venom_label_set_text(VenomLabel* label, const char* text) {
    VENOM_ENSURE_NOT_NULL(label);
    
    if (label->text) {
        venom_free(label->text, strlen(label->text) + 1);
        label->text = NULL;
    }
    
    if (text) {
        VenomSize len = strlen(text) + 1;
        label->text = (char*)venom_alloc(len);
        if (!label->text) {
            return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
        }
        memcpy(label->text, text, len);
    }
    
    venom_widget_invalidate_layout((VenomWidget*)label);
    return VENOM_OK_UNIT();
}

const char* venom_label_get_text(const VenomLabel* label) {
    return label ? label->text : NULL;
}

void venom_label_set_color(VenomLabel* label, VenomColor color) {
    if (!label) return;
    label->text_color = color;
    venom_widget_invalidate((VenomWidget*)label);
}

void venom_label_set_font_size(VenomLabel* label, VenomF32 size) {
    if (!label) return;
    label->font_size = size;
    venom_widget_invalidate_layout((VenomWidget*)label);
}

void venom_label_set_align(VenomLabel* label, VenomTextAlign align) {
    if (!label) return;
    label->align = align;
    venom_widget_invalidate((VenomWidget*)label);
}
