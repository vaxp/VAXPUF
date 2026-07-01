/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_progress.c - ProgressBar widget implementation
 */

#include "vaxp/widgets/vaxp_progress.h"
#include "vaxp/core/vaxp_memory.h"

#define DEFAULT_HEIGHT 8.0f
#define DEFAULT_CORNER_RADIUS 4.0f

static void progress_init(VaxpWidget* widget) {
    VaxpProgressBar* bar = (VaxpProgressBar*)widget;
    
    bar->value = 0.0f;
    bar->type = VAXP_PROGRESS_LINEAR;
    bar->animation_offset = 0;
    
    bar->height = DEFAULT_HEIGHT;
    bar->corner_radius = DEFAULT_CORNER_RADIUS;
    bar->track_color = (VaxpColor){ 224, 224, 224, 255 };
    bar->fill_color = (VaxpColor){ 63, 81, 181, 255 };
    
    widget->layout.preferred_width = 200;
    widget->layout.preferred_height = DEFAULT_HEIGHT;
}

static void progress_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                             VaxpF32* out_width, VaxpF32* out_height) {
    VaxpProgressBar* bar = (VaxpProgressBar*)widget;
    (void)available_height;
    
    *out_width = widget->layout.preferred_width > 0 ? 
                 widget->layout.preferred_width : available_width;
    *out_height = bar->height;
}

static void progress_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpProgressBar* bar = (VaxpProgressBar*)widget;
    
    VaxpF32 w = widget->bounds.width;
    VaxpF32 h = bar->height;
    
    /* Draw track */
    VaxpRectF track = { 0, 0, w, h };
    VaxpPaint track_paint = vaxp_paint_fill(bar->track_color);
    vaxp_canvas_draw_rounded_rect(canvas, track, bar->corner_radius, &track_paint);
    
    if (bar->type == VAXP_PROGRESS_LINEAR) {
        /* Draw fill */
        if (bar->value > 0) {
            VaxpF32 fill_width = w * bar->value;
            if (fill_width < bar->corner_radius * 2) {
                fill_width = bar->corner_radius * 2;
            }
            if (fill_width > w) fill_width = w;
            
            VaxpRectF fill = { 0, 0, fill_width, h };
            VaxpPaint fill_paint = vaxp_paint_fill(bar->fill_color);
            vaxp_canvas_draw_rounded_rect(canvas, fill, bar->corner_radius, &fill_paint);
        }
    } else {
        /* Indeterminate - animated block */
        VaxpF32 block_width = w * 0.3f;
        VaxpF32 x = bar->animation_offset * (w + block_width) - block_width;
        
        vaxp_canvas_save(canvas);
        vaxp_canvas_clip_rounded_rect(canvas, track, bar->corner_radius);
        
        VaxpRectF fill = { x, 0, block_width, h };
        VaxpPaint fill_paint = vaxp_paint_fill(bar->fill_color);
        vaxp_canvas_draw_rect(canvas, fill, &fill_paint);
        
        vaxp_canvas_restore(canvas);
    }
}

const VaxpWidgetClass vaxp_progress_class = {
    .class_name = "VaxpProgressBar",
    .instance_size = sizeof(VaxpProgressBar),
    .parent_class = &vaxp_widget_class,
    .init = progress_init,
    .destroy = NULL,
    .measure = progress_measure,
    .layout = NULL,
    .draw = progress_draw,
    .on_event = NULL,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_progress_create(void) {
    return vaxp_widget_create(&vaxp_progress_class);
}

void vaxp_progress_set_value(VaxpProgressBar* bar, VaxpF32 value) {
    if (!bar) return;
    
    if (value < 0) value = 0;
    if (value > 1) value = 1;
    
    bar->value = value;
    vaxp_widget_invalidate((VaxpWidget*)bar);
}

VaxpF32 vaxp_progress_get_value(const VaxpProgressBar* bar) {
    return bar ? bar->value : 0;
}

void vaxp_progress_set_indeterminate(VaxpProgressBar* bar, VaxpBool indeterminate) {
    if (bar) {
        bar->type = indeterminate ? VAXP_PROGRESS_INDETERMINATE : VAXP_PROGRESS_LINEAR;
        vaxp_widget_invalidate((VaxpWidget*)bar);
    }
}

void vaxp_progress_set_colors(VaxpProgressBar* bar, VaxpColor track, VaxpColor fill) {
    if (bar) {
        bar->track_color = track;
        bar->fill_color = fill;
        vaxp_widget_invalidate((VaxpWidget*)bar);
    }
}

VaxpWidget* _vaxp_progress_build(const VaxpProgressConfig* config) {
    VaxpResultPtr result = vaxp_progress_create();
    if (!result.ok) return NULL;
    
    VaxpProgressBar* bar = (VaxpProgressBar*)result.value;
    bar->value = config->value;
    if (config->indeterminate) bar->type = VAXP_PROGRESS_INDETERMINATE;
    if (config->fill_color.a > 0) bar->fill_color = config->fill_color;
    
    return (VaxpWidget*)bar;
}
