/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_spinner.c - Circular progress spinner implementation
 */

#include "venom/widgets/venom_spinner.h"
#include "venom/core/venom_memory.h"
#include <math.h>

#define DEFAULT_SIZE 40.0f
#define DEFAULT_STROKE 4.0f
#define PI 3.14159265358979323846f

static void spinner_init(VenomWidget* widget) {
    VenomSpinner* spinner = (VenomSpinner*)widget;
    
    spinner->size = DEFAULT_SIZE;
    spinner->stroke_width = DEFAULT_STROKE;
    spinner->angle = 0;
    spinner->color = (VenomColor){ 63, 81, 181, 255 };
    spinner->track_color = (VenomColor){ 224, 224, 224, 255 };
    spinner->show_track = VENOM_TRUE;
}

static void spinner_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                            VenomF32* out_width, VenomF32* out_height) {
    VenomSpinner* spinner = (VenomSpinner*)widget;
    (void)available_width; (void)available_height;
    
    *out_width = spinner->size;
    *out_height = spinner->size;
}

static void spinner_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomSpinner* spinner = (VenomSpinner*)widget;
    
    VenomF32 cx = spinner->size / 2;
    VenomF32 cy = spinner->size / 2;
    VenomF32 r = (spinner->size - spinner->stroke_width) / 2;
    
    /* Draw track */
    if (spinner->show_track) {
        VenomPaint track_paint = venom_paint_stroke(spinner->track_color, spinner->stroke_width);
        venom_canvas_draw_circle(canvas, cx, cy, r, &track_paint);
    }
    
    /* Draw arc - simulate with multiple line segments */
    VenomPaint arc_paint = venom_paint_stroke(spinner->color, spinner->stroke_width);
    
    VenomF32 start_angle = spinner->angle;
    VenomF32 arc_length = PI * 0.75f;  /* 3/4 of a circle */
    
    int segments = 20;
    for (int i = 0; i < segments; i++) {
        VenomF32 a1 = start_angle + arc_length * (VenomF32)i / (VenomF32)segments;
        VenomF32 a2 = start_angle + arc_length * (VenomF32)(i + 1) / (VenomF32)segments;
        
        VenomF32 x1 = cx + r * cosf(a1);
        VenomF32 y1 = cy + r * sinf(a1);
        VenomF32 x2 = cx + r * cosf(a2);
        VenomF32 y2 = cy + r * sinf(a2);
        
        venom_canvas_draw_line(canvas, x1, y1, x2, y2, &arc_paint);
    }
}

const VenomWidgetClass venom_spinner_class = {
    .class_name = "VenomSpinner",
    .instance_size = sizeof(VenomSpinner),
    .parent_class = &venom_widget_class,
    .init = spinner_init,
    .destroy = NULL,
    .measure = spinner_measure,
    .layout = NULL,
    .draw = spinner_draw,
    .on_event = NULL,
    .on_state_changed = NULL,
};

VenomResultPtr venom_spinner_create(void) {
    return venom_widget_create(&venom_spinner_class);
}

void venom_spinner_set_size(VenomSpinner* spinner, VenomF32 size) {
    if (spinner && size > 0) {
        spinner->size = size;
        venom_widget_invalidate((VenomWidget*)spinner);
    }
}

void venom_spinner_set_color(VenomSpinner* spinner, VenomColor color) {
    if (spinner) {
        spinner->color = color;
        venom_widget_invalidate((VenomWidget*)spinner);
    }
}

void venom_spinner_animate(VenomSpinner* spinner, VenomF32 delta_time) {
    if (spinner) {
        spinner->angle += delta_time * 5.0f;  /* Radians per second */
        if (spinner->angle > PI * 2) spinner->angle -= PI * 2;
        venom_widget_invalidate((VenomWidget*)spinner);
    }
}

VenomWidget* _venom_spinner_build(const VenomSpinnerConfig* config) {
    VenomResultPtr result = venom_spinner_create();
    if (!result.ok) return NULL;
    
    VenomSpinner* spinner = (VenomSpinner*)result.value;
    
    if (config->size > 0) spinner->size = config->size;
    if (config->color.a > 0) spinner->color = config->color;
    
    return (VenomWidget*)spinner;
}
