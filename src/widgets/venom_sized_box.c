/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_sized_box.c - SizedBox widget implementation
 */

#include "venom/widgets/venom_sized_box.h"
#include "venom/core/venom_memory.h"

static void sized_box_init(VenomWidget* widget) {
    VenomSizedBox* box = (VenomSizedBox*)widget;
    box->child = NULL;
    box->width = 0;
    box->height = 0;
}

static void sized_box_destroy(VenomWidget* widget) {
    VenomSizedBox* box = (VenomSizedBox*)widget;
    
    if (box->child) {
        box->child->parent = NULL;
        venom_unref(box->child);
        box->child = NULL;
    }
    
    venom_widget_class.destroy(widget);
}

static void sized_box_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                              VenomF32* out_width, VenomF32* out_height) {
    VenomSizedBox* box = (VenomSizedBox*)widget;
    (void)available_width; (void)available_height;
    
    /* Use specified size, or child size if not specified */
    if (box->width > 0) {
        *out_width = box->width;
    } else if (box->child) {
        VenomF32 child_h;
        venom_widget_measure(box->child, available_width, available_height, out_width, &child_h);
    } else {
        *out_width = 0;
    }
    
    if (box->height > 0) {
        *out_height = box->height;
    } else if (box->child) {
        VenomF32 child_w;
        venom_widget_measure(box->child, available_width, available_height, &child_w, out_height);
    } else {
        *out_height = 0;
    }
}

static void sized_box_layout(VenomWidget* widget, VenomRectF bounds) {
    VenomSizedBox* box = (VenomSizedBox*)widget;
    widget->bounds = bounds;
    
    if (box->child && box->child->visible) {
        VenomRectF child_bounds = { 0, 0, bounds.width, bounds.height };
        venom_widget_layout(box->child, child_bounds);
    }
}

static void sized_box_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomSizedBox* box = (VenomSizedBox*)widget;
    
    if (box->child && box->child->visible) {
        venom_widget_draw(box->child, canvas);
    }
}

static VenomBool sized_box_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomSizedBox* box = (VenomSizedBox*)widget;
    
    if (box->child && box->child->visible) {
        return venom_widget_dispatch_event(box->child, event);
    }
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_sized_box_class = {
    .class_name = "VenomSizedBox",
    .instance_size = sizeof(VenomSizedBox),
    .parent_class = &venom_widget_class,
    .init = sized_box_init,
    .destroy = sized_box_destroy,
    .measure = sized_box_measure,
    .layout = sized_box_layout,
    .draw = sized_box_draw,
    .on_event = sized_box_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_sized_box_create(void) {
    return venom_widget_create(&venom_sized_box_class);
}

VenomResult venom_sized_box_set_child(VenomSizedBox* box, VenomWidget* child) {
    VENOM_ENSURE_NOT_NULL(box);
    
    if (box->child) {
        box->child->parent = NULL;
        venom_unref(box->child);
    }
    
    box->child = child;
    if (child) {
        venom_ref(child);
        child->parent = (VenomWidget*)box;
    }
    
    venom_widget_invalidate_layout((VenomWidget*)box);
    return VENOM_OK_UNIT();
}

void venom_sized_box_set_size(VenomSizedBox* box, VenomF32 width, VenomF32 height) {
    if (box) {
        box->width = width;
        box->height = height;
        venom_widget_invalidate_layout((VenomWidget*)box);
    }
}

VenomWidget* _venom_sized_box_build(const VenomSizedBoxConfig* config) {
    VenomResultPtr result = venom_sized_box_create();
    if (!result.ok) return NULL;
    
    VenomSizedBox* box = (VenomSizedBox*)result.value;
    box->width = config->width;
    box->height = config->height;
    
    if (config->child) {
        venom_sized_box_set_child(box, config->child);
    }
    
    return (VenomWidget*)box;
}
