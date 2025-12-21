/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_progress.c - ProgressBar widget implementation
 */

#include "venom/widgets/venom_progress.h"
#include "venom/core/venom_memory.h"

#define DEFAULT_HEIGHT 8.0f
#define DEFAULT_CORNER_RADIUS 4.0f

static void progress_init(VenomWidget* widget) {
    VenomProgressBar* bar = (VenomProgressBar*)widget;
    
    bar->value = 0.0f;
    bar->type = VENOM_PROGRESS_LINEAR;
    bar->animation_offset = 0;
    
    bar->height = DEFAULT_HEIGHT;
    bar->corner_radius = DEFAULT_CORNER_RADIUS;
    bar->track_color = (VenomColor){ 224, 224, 224, 255 };
    bar->fill_color = (VenomColor){ 63, 81, 181, 255 };
    
    widget->layout.preferred_width = 200;
    widget->layout.preferred_height = DEFAULT_HEIGHT;
}

static void progress_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                             VenomF32* out_width, VenomF32* out_height) {
    VenomProgressBar* bar = (VenomProgressBar*)widget;
    (void)available_height;
    
    *out_width = widget->layout.preferred_width > 0 ? 
                 widget->layout.preferred_width : available_width;
    *out_height = bar->height;
}

static void progress_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomProgressBar* bar = (VenomProgressBar*)widget;
    
    VenomF32 w = widget->bounds.width;
    VenomF32 h = bar->height;
    
    /* Draw track */
    VenomRectF track = { 0, 0, w, h };
    VenomPaint track_paint = venom_paint_fill(bar->track_color);
    venom_canvas_draw_rounded_rect(canvas, track, bar->corner_radius, &track_paint);
    
    if (bar->type == VENOM_PROGRESS_LINEAR) {
        /* Draw fill */
        if (bar->value > 0) {
            VenomF32 fill_width = w * bar->value;
            if (fill_width < bar->corner_radius * 2) {
                fill_width = bar->corner_radius * 2;
            }
            if (fill_width > w) fill_width = w;
            
            VenomRectF fill = { 0, 0, fill_width, h };
            VenomPaint fill_paint = venom_paint_fill(bar->fill_color);
            venom_canvas_draw_rounded_rect(canvas, fill, bar->corner_radius, &fill_paint);
        }
    } else {
        /* Indeterminate - animated block */
        VenomF32 block_width = w * 0.3f;
        VenomF32 x = bar->animation_offset * (w + block_width) - block_width;
        
        venom_canvas_save(canvas);
        venom_canvas_clip_rounded_rect(canvas, track, bar->corner_radius);
        
        VenomRectF fill = { x, 0, block_width, h };
        VenomPaint fill_paint = venom_paint_fill(bar->fill_color);
        venom_canvas_draw_rect(canvas, fill, &fill_paint);
        
        venom_canvas_restore(canvas);
    }
}

const VenomWidgetClass venom_progress_class = {
    .class_name = "VenomProgressBar",
    .instance_size = sizeof(VenomProgressBar),
    .parent_class = &venom_widget_class,
    .init = progress_init,
    .destroy = NULL,
    .measure = progress_measure,
    .layout = NULL,
    .draw = progress_draw,
    .on_event = NULL,
    .on_state_changed = NULL,
};

VenomResultPtr venom_progress_create(void) {
    return venom_widget_create(&venom_progress_class);
}

void venom_progress_set_value(VenomProgressBar* bar, VenomF32 value) {
    if (!bar) return;
    
    if (value < 0) value = 0;
    if (value > 1) value = 1;
    
    bar->value = value;
    venom_widget_invalidate((VenomWidget*)bar);
}

VenomF32 venom_progress_get_value(const VenomProgressBar* bar) {
    return bar ? bar->value : 0;
}

void venom_progress_set_indeterminate(VenomProgressBar* bar, VenomBool indeterminate) {
    if (bar) {
        bar->type = indeterminate ? VENOM_PROGRESS_INDETERMINATE : VENOM_PROGRESS_LINEAR;
        venom_widget_invalidate((VenomWidget*)bar);
    }
}

void venom_progress_set_colors(VenomProgressBar* bar, VenomColor track, VenomColor fill) {
    if (bar) {
        bar->track_color = track;
        bar->fill_color = fill;
        venom_widget_invalidate((VenomWidget*)bar);
    }
}

VenomWidget* _venom_progress_build(const VenomProgressConfig* config) {
    VenomResultPtr result = venom_progress_create();
    if (!result.ok) return NULL;
    
    VenomProgressBar* bar = (VenomProgressBar*)result.value;
    bar->value = config->value;
    if (config->indeterminate) bar->type = VENOM_PROGRESS_INDETERMINATE;
    if (config->fill_color.a > 0) bar->fill_color = config->fill_color;
    
    return (VenomWidget*)bar;
}
