/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_skeleton.c - Skeleton loading placeholder
 */

#include "vaxp/widgets/vaxp_skeleton.h"
#include "vaxp/core/vaxp_memory.h"

static void skeleton_init(VaxpWidget* widget) {
    VaxpSkeleton* skel = (VaxpSkeleton*)widget;
    
    skel->variant = VAXP_SKELETON_RECTANGULAR;
    skel->width = 100;
    skel->height = 20;
    skel->corner_radius = 4.0f;
    skel->animate = VAXP_TRUE;
    skel->animation_offset = 0;
    
    skel->base_color = (VaxpColor){ 224, 224, 224, 255 };
    skel->highlight_color = (VaxpColor){ 240, 240, 240, 255 };
}

static void skeleton_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                             VaxpF32* out_width, VaxpF32* out_height) {
    VaxpSkeleton* skel = (VaxpSkeleton*)widget;
    (void)available_width; (void)available_height;
    
    if (skel->variant == VAXP_SKELETON_CIRCULAR) {
        *out_width = skel->width;
        *out_height = skel->width;
    } else {
        *out_width = skel->width;
        *out_height = skel->height;
    }
}

static void skeleton_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpSkeleton* skel = (VaxpSkeleton*)widget;
    
    VaxpF32 w = skel->width;
    VaxpF32 h = skel->variant == VAXP_SKELETON_CIRCULAR ? w : skel->height;
    
    /* Draw base */
    VaxpPaint base_paint = vaxp_paint_fill(skel->base_color);
    
    if (skel->variant == VAXP_SKELETON_CIRCULAR) {
        vaxp_canvas_draw_circle(canvas, w / 2, w / 2, w / 2, &base_paint);
    } else if (skel->variant == VAXP_SKELETON_TEXT) {
        VaxpRectF rect = { 0, h * 0.3f, w, h * 0.4f };
        vaxp_canvas_draw_rounded_rect(canvas, rect, 2.0f, &base_paint);
    } else {
        VaxpRectF rect = { 0, 0, w, h };
        vaxp_canvas_draw_rounded_rect(canvas, rect, skel->corner_radius, &base_paint);
    }
    
    /* Draw shimmer if animating */
    if (skel->animate) {
        VaxpF32 shimmer_x = -w + skel->animation_offset * w * 2;
        VaxpF32 shimmer_w = w * 0.3f;
        
        if (shimmer_x >= -shimmer_w && shimmer_x <= w) {
            VaxpPaint shimmer_paint = vaxp_paint_fill(skel->highlight_color);
            VaxpRectF shimmer = { shimmer_x, 0, shimmer_w, h };
            
            vaxp_canvas_save(canvas);
            VaxpRectF clip = { 0, 0, w, h };
            vaxp_canvas_clip_rect(canvas, clip);
            vaxp_canvas_draw_rect(canvas, shimmer, &shimmer_paint);
            vaxp_canvas_restore(canvas);
        }
    }
}

const VaxpWidgetClass vaxp_skeleton_class = {
    .class_name = "VaxpSkeleton",
    .instance_size = sizeof(VaxpSkeleton),
    .parent_class = &vaxp_widget_class,
    .init = skeleton_init,
    .destroy = NULL,
    .measure = skeleton_measure,
    .layout = NULL,
    .draw = skeleton_draw,
    .on_event = NULL,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_skeleton_create(void) {
    return vaxp_widget_create(&vaxp_skeleton_class);
}

void vaxp_skeleton_set_variant(VaxpSkeleton* skel, VaxpSkeletonVariant variant) {
    if (skel) {
        skel->variant = variant;
        vaxp_widget_invalidate((VaxpWidget*)skel);
    }
}

void vaxp_skeleton_set_size(VaxpSkeleton* skel, VaxpF32 width, VaxpF32 height) {
    if (skel) {
        skel->width = width;
        skel->height = height;
        vaxp_widget_invalidate((VaxpWidget*)skel);
    }
}

void vaxp_skeleton_animate(VaxpSkeleton* skel, VaxpF32 delta_time) {
    if (skel && skel->animate) {
        skel->animation_offset += delta_time * 0.5f;
        if (skel->animation_offset > 1.0f) skel->animation_offset = 0;
        vaxp_widget_invalidate((VaxpWidget*)skel);
    }
}

VaxpWidget* _vaxp_skeleton_build(const VaxpSkeletonConfig* config) {
    VaxpResultPtr result = vaxp_skeleton_create();
    if (!result.ok) return NULL;
    
    VaxpSkeleton* skel = (VaxpSkeleton*)result.value;
    
    skel->variant = config->variant;
    if (config->width > 0) skel->width = config->width;
    if (config->height > 0) skel->height = config->height;
    
    return (VaxpWidget*)skel;
}
