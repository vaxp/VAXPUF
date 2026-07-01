/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_spinner.c - World-class circular progress spinner
 * Features: Auto-animation, smooth easing, multiple styles
 */

#include "vaxp/widgets/vaxp_spinner.h"
#include "vaxp/core/vaxp_memory.h"
#include <math.h>
#include <time.h>
#include <sys/time.h>

#define DEFAULT_SIZE 40.0f
#define DEFAULT_STROKE 3.0f
#define PI 3.14159265358979323846f
#define TWO_PI (PI * 2.0f)

/* Get current time in seconds (high precision) */
static VaxpF64 get_time_seconds(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (VaxpF64)tv.tv_sec + (VaxpF64)tv.tv_usec / 1000000.0;
}

/* Easing function: ease-in-out cubic */
static VaxpF32 ease_in_out_cubic(VaxpF32 t) {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

static void spinner_init(VaxpWidget* widget) {
    VaxpSpinner* spinner = (VaxpSpinner*)widget;
    
    spinner->size = DEFAULT_SIZE;
    spinner->stroke_width = DEFAULT_STROKE;
    spinner->angle = 0;
    spinner->color = (VaxpColor){ 63, 81, 181, 255 };  /* Material Blue */
    spinner->track_color = (VaxpColor){ 224, 224, 224, 255 };
    spinner->show_track = VAXP_TRUE;
    
    /* Animation state */
    spinner->start_time = get_time_seconds();
    spinner->animation_duration = 2.8f;  /* Full cycle in seconds (slower - 30 FPS feel) */
    spinner->arc_min = 0.08f;  /* Minimum arc length (fraction of circle) */
    spinner->arc_max = 0.75f;  /* Maximum arc length */
}

static void spinner_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                            VaxpF32* out_width, VaxpF32* out_height) {
    VaxpSpinner* spinner = (VaxpSpinner*)widget;
    (void)available_width; (void)available_height;
    
    *out_width = spinner->size;
    *out_height = spinner->size;
}

static void spinner_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpSpinner* spinner = (VaxpSpinner*)widget;
    
    VaxpF32 cx = spinner->size / 2.0f;
    VaxpF32 cy = spinner->size / 2.0f;
    VaxpF32 r = (spinner->size - spinner->stroke_width) / 2.0f;
    
    /* Auto-animate: Calculate current animation state based on time */
    VaxpF64 now = get_time_seconds();
    VaxpF64 elapsed = now - spinner->start_time;
    
    /* Simple continuous rotation (one full rotation per animation_duration) */
    VaxpF32 rotation = fmodf((VaxpF32)(elapsed / spinner->animation_duration) * TWO_PI, TWO_PI);
    
    /* Arc length oscillates between min and max using sine wave */
    VaxpF32 oscillation = (sinf((VaxpF32)(elapsed * 2.0) + PI) + 1.0f) / 2.0f;  /* 0 to 1 */
    VaxpF32 arc_length = spinner->arc_min + (spinner->arc_max - spinner->arc_min) * oscillation;
    
    /* Ensure minimum visibility */
    if (arc_length < 0.1f) arc_length = 0.1f;
    
    VaxpF32 start_angle = rotation - PI / 2.0f;  /* Start at top */
    VaxpF32 arc_angle = arc_length * TWO_PI;
    
    /* Draw track (background circle) */
    if (spinner->show_track) {
        VaxpPaint track_paint = vaxp_paint_stroke(spinner->track_color, spinner->stroke_width);
        vaxp_canvas_draw_circle(canvas, cx, cy, r, &track_paint);
    }
    
    /* Draw animated arc using line segments for smooth curve */
    VaxpPaint arc_paint = vaxp_paint_stroke(spinner->color, spinner->stroke_width);
    
    /* Fixed segment count for consistent rendering */
    int segments = 30;
    
    for (int i = 0; i < segments; i++) {
        VaxpF32 t1 = (VaxpF32)i / (VaxpF32)segments;
        VaxpF32 t2 = (VaxpF32)(i + 1) / (VaxpF32)segments;
        
        VaxpF32 a1 = start_angle + arc_angle * t1;
        VaxpF32 a2 = start_angle + arc_angle * t2;
        
        VaxpF32 x1 = cx + r * cosf(a1);
        VaxpF32 y1 = cy + r * sinf(a1);
        VaxpF32 x2 = cx + r * cosf(a2);
        VaxpF32 y2 = cy + r * sinf(a2);
        
        vaxp_canvas_draw_line(canvas, x1, y1, x2, y2, &arc_paint);
    }
    
    /* Always request redraw for continuous animation */
    widget->needs_redraw = VAXP_TRUE;
}

const VaxpWidgetClass vaxp_spinner_class = {
    .class_name = "VaxpSpinner",
    .instance_size = sizeof(VaxpSpinner),
    .parent_class = &vaxp_widget_class,
    .init = spinner_init,
    .destroy = NULL,
    .measure = spinner_measure,
    .layout = NULL,
    .draw = spinner_draw,
    .on_event = NULL,
    .on_state_changed = NULL,
};

VaxpResultPtr vaxp_spinner_create(void) {
    return vaxp_widget_create(&vaxp_spinner_class);
}

void vaxp_spinner_set_size(VaxpSpinner* spinner, VaxpF32 size) {
    if (spinner && size > 0) {
        spinner->size = size;
        vaxp_widget_invalidate((VaxpWidget*)spinner);
    }
}

void vaxp_spinner_set_color(VaxpSpinner* spinner, VaxpColor color) {
    if (spinner) {
        spinner->color = color;
        vaxp_widget_invalidate((VaxpWidget*)spinner);
    }
}

void vaxp_spinner_set_stroke_width(VaxpSpinner* spinner, VaxpF32 width) {
    if (spinner && width > 0) {
        spinner->stroke_width = width;
        vaxp_widget_invalidate((VaxpWidget*)spinner);
    }
}

void vaxp_spinner_set_duration(VaxpSpinner* spinner, VaxpF32 seconds) {
    if (spinner && seconds > 0) {
        spinner->animation_duration = seconds;
    }
}

void vaxp_spinner_animate(VaxpSpinner* spinner, VaxpF32 delta_time) {
    /* No longer needed - animation is automatic in draw() */
    (void)spinner; (void)delta_time;
}

VaxpWidget* _vaxp_spinner_build(const VaxpSpinnerConfig* config) {
    VaxpResultPtr result = vaxp_spinner_create();
    if (!result.ok) return NULL;
    
    VaxpSpinner* spinner = (VaxpSpinner*)result.value;
    
    if (config->size > 0) spinner->size = config->size;
    if (config->color.a > 0) spinner->color = config->color;
    
    return (VaxpWidget*)spinner;
}
