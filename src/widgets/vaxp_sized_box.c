/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_sized_box.c - SizedBox widget implementation
 */

#include "vaxp/widgets/vaxp_sized_box.h"
#include "vaxp/core/vaxp_memory.h"

static void sized_box_init(VaxpWidget* widget) {
    VaxpSizedBox* box = (VaxpSizedBox*)widget;
    box->child = NULL;
    box->width = 0;
    box->height = 0;
}

static void sized_box_destroy(VaxpWidget* widget) {
    VaxpSizedBox* box = (VaxpSizedBox*)widget;
    
    if (box->child) {
        box->child->parent = NULL;
        vaxp_unref(box->child);
        box->child = NULL;
    }
    
    vaxp_widget_class.destroy(widget);
}

static void sized_box_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                              VaxpF32* out_width, VaxpF32* out_height) {
    VaxpSizedBox* box = (VaxpSizedBox*)widget;
    (void)available_width; (void)available_height;
    
    /* Use specified size, or child size if not specified */
    if (box->width > 0) {
        *out_width = box->width;
    } else if (box->child) {
        VaxpF32 child_h;
        vaxp_widget_measure(box->child, available_width, available_height, out_width, &child_h);
    } else {
        *out_width = 0;
    }
    
    if (box->height > 0) {
        *out_height = box->height;
    } else if (box->child) {
        VaxpF32 child_w;
        vaxp_widget_measure(box->child, available_width, available_height, &child_w, out_height);
    } else {
        *out_height = 0;
    }
}

static void sized_box_layout(VaxpWidget* widget, VaxpRectF bounds) {
    VaxpSizedBox* box = (VaxpSizedBox*)widget;
    widget->bounds = bounds;
    
    if (box->child && box->child->visible) {
        VaxpRectF child_bounds = { 0, 0, bounds.width, bounds.height };
        vaxp_widget_layout(box->child, child_bounds);
    }
}

static void sized_box_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpSizedBox* box = (VaxpSizedBox*)widget;
    
    if (box->child && box->child->visible) {
        vaxp_widget_draw(box->child, canvas);
    }
}

static VaxpBool sized_box_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpSizedBox* box = (VaxpSizedBox*)widget;
    
    if (box->child && box->child->visible) {
        return vaxp_widget_dispatch_event(box->child, event);
    }
    
    return VAXP_FALSE;
}

const VaxpWidgetClass vaxp_sized_box_class = {
    .class_name = "VaxpSizedBox",
    .instance_size = sizeof(VaxpSizedBox),
    .parent_class = &vaxp_widget_class,
    .init = sized_box_init,
    .destroy = sized_box_destroy,
    .measure = sized_box_measure,
    .layout = sized_box_layout,
    .draw = sized_box_draw,
    .on_event = sized_box_on_event,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_sized_box_create(void) {
    return vaxp_widget_create(&vaxp_sized_box_class);
}

VaxpResult vaxp_sized_box_set_child(VaxpSizedBox* box, VaxpWidget* child) {
    VAXP_ENSURE_NOT_NULL(box);
    
    if (box->child) {
        box->child->parent = NULL;
        vaxp_unref(box->child);
    }
    
    box->child = child;
    if (child) {
        vaxp_ref(child);
        child->parent = (VaxpWidget*)box;
    }
    
    vaxp_widget_invalidate_layout((VaxpWidget*)box);
    return VAXP_OK_UNIT();
}

void vaxp_sized_box_set_size(VaxpSizedBox* box, VaxpF32 width, VaxpF32 height) {
    if (box) {
        box->width = width;
        box->height = height;
        vaxp_widget_invalidate_layout((VaxpWidget*)box);
    }
}

VaxpWidget* _vaxp_sized_box_build(const VaxpSizedBoxConfig* config) {
    VaxpResultPtr result = vaxp_sized_box_create();
    if (!result.ok) return NULL;
    
    VaxpSizedBox* box = (VaxpSizedBox*)result.value;
    box->width = config->width;
    box->height = config->height;
    
    if (config->child) {
        vaxp_sized_box_set_child(box, config->child);
    }
    
    return (VaxpWidget*)box;
}
