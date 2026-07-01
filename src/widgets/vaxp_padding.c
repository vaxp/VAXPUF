/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_padding.c - Padding widget implementation
 */

#include "vaxp/widgets/vaxp_padding.h"
#include "vaxp/core/vaxp_memory.h"

static void padding_init(VaxpWidget* widget) {
    VaxpPadding* pad = (VaxpPadding*)widget;
    pad->child = NULL;
    pad->padding = (VaxpInsets){ 0, 0, 0, 0 };
}

static void padding_destroy(VaxpWidget* widget) {
    VaxpPadding* pad = (VaxpPadding*)widget;
    if (pad->child) {
        pad->child->parent = NULL;
        vaxp_unref(pad->child);
        pad->child = NULL;
    }
    vaxp_widget_class.destroy(widget);
}

static void padding_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                            VaxpF32* out_width, VaxpF32* out_height) {
    VaxpPadding* pad = (VaxpPadding*)widget;
    
    VaxpF32 h_padding = pad->padding.left + pad->padding.right;
    VaxpF32 v_padding = pad->padding.top + pad->padding.bottom;
    
    if (pad->child) {
        VaxpF32 child_w, child_h;
        vaxp_widget_measure(pad->child, 
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

static void padding_layout(VaxpWidget* widget, VaxpRectF bounds) {
    VaxpPadding* pad = (VaxpPadding*)widget;
    widget->bounds = bounds;
    
    if (pad->child && pad->child->visible) {
        VaxpRectF child_bounds = {
            pad->padding.left,
            pad->padding.top,
            bounds.width - pad->padding.left - pad->padding.right,
            bounds.height - pad->padding.top - pad->padding.bottom
        };
        vaxp_widget_layout(pad->child, child_bounds);
    }
}

static void padding_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpPadding* pad = (VaxpPadding*)widget;
    if (pad->child && pad->child->visible) {
        vaxp_canvas_save(canvas);
        vaxp_canvas_translate(canvas, pad->child->bounds.x, pad->child->bounds.y);
        vaxp_widget_draw(pad->child, canvas);
        vaxp_canvas_restore(canvas);
    }
}

static VaxpBool padding_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpPadding* pad = (VaxpPadding*)widget;
    if (pad->child && pad->child->visible) {
        return vaxp_widget_dispatch_event(pad->child, event);
    }
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_padding_class = {
    .class_name = "VaxpPadding",
    .instance_size = sizeof(VaxpPadding),
    .parent_class = &vaxp_widget_class,
    .init = padding_init,
    .destroy = padding_destroy,
    .measure = padding_measure,
    .layout = padding_layout,
    .draw = padding_draw,
    .on_event = padding_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_padding_create(void) {
    return vaxp_widget_create(&vaxp_padding_class);
}

VaxpResult vaxp_padding_set_child(VaxpPadding* pad, VaxpWidget* child) {
    VAXP_ENSURE_NOT_NULL(pad);
    if (pad->child) {
        pad->child->parent = NULL;
        vaxp_unref(pad->child);
    }
    pad->child = child;
    if (child) {
        vaxp_ref(child);
        child->parent = (VaxpWidget*)pad;
    }
    vaxp_widget_invalidate_layout((VaxpWidget*)pad);
    return VAXP_OK_UNIT();
}

void vaxp_padding_set_padding(VaxpPadding* pad, VaxpInsets padding) {
    if (pad) {
        pad->padding = padding;
        vaxp_widget_invalidate_layout((VaxpWidget*)pad);
    }
}

void vaxp_padding_set_all(VaxpPadding* pad, VaxpF32 value) {
    if (pad) {
        pad->padding = (VaxpInsets){ value, value, value, value };
        vaxp_widget_invalidate_layout((VaxpWidget*)pad);
    }
}

VaxpWidget* _vaxp_padding_build(const VaxpPaddingConfig* config) {
    VaxpResultPtr result = vaxp_padding_create();
    if (!result.ok) return NULL;
    
    VaxpPadding* pad = (VaxpPadding*)result.value;
    
    if (config->all > 0) {
        pad->padding = (VaxpInsets){ config->all, config->all, config->all, config->all };
    } else {
        pad->padding = config->padding;
    }
    
    if (config->child) {
        vaxp_padding_set_child(pad, config->child);
    }
    
    return (VaxpWidget*)pad;
}
