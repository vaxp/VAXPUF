/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_padding.c - Padding widget implementation
 */

#include "venom/widgets/venom_padding.h"
#include "venom/core/venom_memory.h"

static void padding_init(VenomWidget* widget) {
    VenomPadding* pad = (VenomPadding*)widget;
    pad->child = NULL;
    pad->padding = (VenomInsets){ 0, 0, 0, 0 };
}

static void padding_destroy(VenomWidget* widget) {
    VenomPadding* pad = (VenomPadding*)widget;
    if (pad->child) {
        pad->child->parent = NULL;
        venom_unref(pad->child);
        pad->child = NULL;
    }
    venom_widget_class.destroy(widget);
}

static void padding_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                            VenomF32* out_width, VenomF32* out_height) {
    VenomPadding* pad = (VenomPadding*)widget;
    
    VenomF32 h_padding = pad->padding.left + pad->padding.right;
    VenomF32 v_padding = pad->padding.top + pad->padding.bottom;
    
    if (pad->child) {
        VenomF32 child_w, child_h;
        venom_widget_measure(pad->child, 
                             available_width - h_padding, 
                             available_height - v_padding,
                             &child_w, &child_h);
        *out_width = child_w + h_padding;
        *out_height = child_h + v_padding;
    } else {
        *out_width = h_padding;
        *out_height = v_padding;
    }
}

static void padding_layout(VenomWidget* widget, VenomRectF bounds) {
    VenomPadding* pad = (VenomPadding*)widget;
    widget->bounds = bounds;
    
    if (pad->child && pad->child->visible) {
        VenomRectF child_bounds = {
            pad->padding.left,
            pad->padding.top,
            bounds.width - pad->padding.left - pad->padding.right,
            bounds.height - pad->padding.top - pad->padding.bottom
        };
        venom_widget_layout(pad->child, child_bounds);
    }
}

static void padding_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomPadding* pad = (VenomPadding*)widget;
    if (pad->child && pad->child->visible) {
        venom_canvas_save(canvas);
        venom_canvas_translate(canvas, pad->child->bounds.x, pad->child->bounds.y);
        venom_widget_draw(pad->child, canvas);
        venom_canvas_restore(canvas);
    }
}

static VenomBool padding_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomPadding* pad = (VenomPadding*)widget;
    if (pad->child && pad->child->visible) {
        return venom_widget_dispatch_event(pad->child, event);
    }
    return VENOM_FALSE;
}

const VenomWidgetClass venom_padding_class = {
    .class_name = "VenomPadding",
    .instance_size = sizeof(VenomPadding),
    .parent_class = &venom_widget_class,
    .init = padding_init,
    .destroy = padding_destroy,
    .measure = padding_measure,
    .layout = padding_layout,
    .draw = padding_draw,
    .on_event = padding_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_padding_create(void) {
    return venom_widget_create(&venom_padding_class);
}

VenomResult venom_padding_set_child(VenomPadding* pad, VenomWidget* child) {
    VENOM_ENSURE_NOT_NULL(pad);
    if (pad->child) {
        pad->child->parent = NULL;
        venom_unref(pad->child);
    }
    pad->child = child;
    if (child) {
        venom_ref(child);
        child->parent = (VenomWidget*)pad;
    }
    venom_widget_invalidate_layout((VenomWidget*)pad);
    return VENOM_OK_UNIT();
}

void venom_padding_set_padding(VenomPadding* pad, VenomInsets padding) {
    if (pad) {
        pad->padding = padding;
        venom_widget_invalidate_layout((VenomWidget*)pad);
    }
}

void venom_padding_set_all(VenomPadding* pad, VenomF32 value) {
    if (pad) {
        pad->padding = (VenomInsets){ value, value, value, value };
        venom_widget_invalidate_layout((VenomWidget*)pad);
    }
}

VenomWidget* _venom_padding_build(const VenomPaddingConfig* config) {
    VenomResultPtr result = venom_padding_create();
    if (!result.ok) return NULL;
    
    VenomPadding* pad = (VenomPadding*)result.value;
    
    if (config->all > 0) {
        pad->padding = (VenomInsets){ config->all, config->all, config->all, config->all };
    } else {
        pad->padding = config->padding;
    }
    
    if (config->child) {
        venom_padding_set_child(pad, config->child);
    }
    
    return (VenomWidget*)pad;
}
