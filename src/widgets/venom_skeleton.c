/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_skeleton.c - Skeleton loading placeholder
 */

#include "venom/widgets/venom_skeleton.h"
#include "venom/core/venom_memory.h"

static void skeleton_init(VenomWidget* widget) {
    VenomSkeleton* skel = (VenomSkeleton*)widget;
    
    skel->variant = VENOM_SKELETON_RECTANGULAR;
    skel->width = 100;
    skel->height = 20;
    skel->corner_radius = 4.0f;
    skel->animate = VENOM_TRUE;
    skel->animation_offset = 0;
    
    skel->base_color = (VenomColor){ 224, 224, 224, 255 };
    skel->highlight_color = (VenomColor){ 240, 240, 240, 255 };
}

static void skeleton_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                             VenomF32* out_width, VenomF32* out_height) {
    VenomSkeleton* skel = (VenomSkeleton*)widget;
    (void)available_width; (void)available_height;
    
    if (skel->variant == VENOM_SKELETON_CIRCULAR) {
        *out_width = skel->width;
        *out_height = skel->width;
    } else {
        *out_width = skel->width;
        *out_height = skel->height;
    }
}

static void skeleton_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomSkeleton* skel = (VenomSkeleton*)widget;
    
    VenomF32 w = skel->width;
    VenomF32 h = skel->variant == VENOM_SKELETON_CIRCULAR ? w : skel->height;
    
    /* Draw base */
    VenomPaint base_paint = venom_paint_fill(skel->base_color);
    
    if (skel->variant == VENOM_SKELETON_CIRCULAR) {
        venom_canvas_draw_circle(canvas, w / 2, w / 2, w / 2, &base_paint);
    } else if (skel->variant == VENOM_SKELETON_TEXT) {
        VenomRectF rect = { 0, h * 0.3f, w, h * 0.4f };
        venom_canvas_draw_rounded_rect(canvas, rect, 2.0f, &base_paint);
    } else {
        VenomRectF rect = { 0, 0, w, h };
        venom_canvas_draw_rounded_rect(canvas, rect, skel->corner_radius, &base_paint);
    }
    
    /* Draw shimmer if animating */
    if (skel->animate) {
        VenomF32 shimmer_x = -w + skel->animation_offset * w * 2;
        VenomF32 shimmer_w = w * 0.3f;
        
        if (shimmer_x >= -shimmer_w && shimmer_x <= w) {
            VenomPaint shimmer_paint = venom_paint_fill(skel->highlight_color);
            VenomRectF shimmer = { shimmer_x, 0, shimmer_w, h };
            
            venom_canvas_save(canvas);
            VenomRectF clip = { 0, 0, w, h };
            venom_canvas_clip_rect(canvas, clip);
            venom_canvas_draw_rect(canvas, shimmer, &shimmer_paint);
            venom_canvas_restore(canvas);
        }
    }
}

const VenomWidgetClass venom_skeleton_class = {
    .class_name = "VenomSkeleton",
    .instance_size = sizeof(VenomSkeleton),
    .parent_class = &venom_widget_class,
    .init = skeleton_init,
    .destroy = NULL,
    .measure = skeleton_measure,
    .layout = NULL,
    .draw = skeleton_draw,
    .on_event = NULL,
    .on_state_changed = NULL,
};

VenomResultPtr venom_skeleton_create(void) {
    return venom_widget_create(&venom_skeleton_class);
}

void venom_skeleton_set_variant(VenomSkeleton* skel, VenomSkeletonVariant variant) {
    if (skel) {
        skel->variant = variant;
        venom_widget_invalidate((VenomWidget*)skel);
    }
}

void venom_skeleton_set_size(VenomSkeleton* skel, VenomF32 width, VenomF32 height) {
    if (skel) {
        skel->width = width;
        skel->height = height;
        venom_widget_invalidate((VenomWidget*)skel);
    }
}

void venom_skeleton_animate(VenomSkeleton* skel, VenomF32 delta_time) {
    if (skel && skel->animate) {
        skel->animation_offset += delta_time * 0.5f;
        if (skel->animation_offset > 1.0f) skel->animation_offset = 0;
        venom_widget_invalidate((VenomWidget*)skel);
    }
}

VenomWidget* _venom_skeleton_build(const VenomSkeletonConfig* config) {
    VenomResultPtr result = venom_skeleton_create();
    if (!result.ok) return NULL;
    
    VenomSkeleton* skel = (VenomSkeleton*)result.value;
    
    skel->variant = config->variant;
    if (config->width > 0) skel->width = config->width;
    if (config->height > 0) skel->height = config->height;
    
    return (VenomWidget*)skel;
}
