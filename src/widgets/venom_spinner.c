/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_spinner.c - World-class circular progress spinner
 * Features: Auto-animation, smooth easing, multiple styles
 */

#include "venom/widgets/venom_spinner.h"
#include "venom/core/venom_memory.h"
#include <math.h>
#include <time.h>
#include <sys/time.h>

#define DEFAULT_SIZE 40.0f
#define DEFAULT_STROKE 3.0f
#define PI 3.14159265358979323846f
#define TWO_PI (PI * 2.0f)

/* Get current time in seconds (high precision) */
static VenomF64 get_time_seconds(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (VenomF64)tv.tv_sec + (VenomF64)tv.tv_usec / 1000000.0;
}

/* Easing function: ease-in-out cubic */
static VenomF32 ease_in_out_cubic(VenomF32 t) {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

static void spinner_init(VenomWidget* widget) {
    VenomSpinner* spinner = (VenomSpinner*)widget;
    
    spinner->size = DEFAULT_SIZE;
    spinner->stroke_width = DEFAULT_STROKE;
    spinner->angle = 0;
    spinner->color = (VenomColor){ 63, 81, 181, 255 };  /* Material Blue */
    spinner->track_color = (VenomColor){ 224, 224, 224, 255 };
    spinner->show_track = VENOM_TRUE;
    
    /* Animation state */
    spinner->start_time = get_time_seconds();
    spinner->animation_duration = 1.4f;  /* Full cycle in seconds */
    spinner->arc_min = 0.08f;  /* Minimum arc length (fraction of circle) */
    spinner->arc_max = 0.75f;  /* Maximum arc length */
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
    
    VenomF32 cx = spinner->size / 2.0f;
    VenomF32 cy = spinner->size / 2.0f;
    VenomF32 r = (spinner->size - spinner->stroke_width) / 2.0f;
    
    /* Auto-animate: Calculate current animation state based on time */
    VenomF64 now = get_time_seconds();
    VenomF64 elapsed = now - spinner->start_time;
    VenomF32 cycle = fmodf((VenomF32)elapsed / spinner->animation_duration, 1.0f);
    
    /* Phase 1 (0-0.5): Arc grows while rotating
     * Phase 2 (0.5-1): Arc shrinks while rotating faster */
    VenomF32 arc_length, rotation_offset;
    
    if (cycle < 0.5f) {
        /* Growing phase */
        VenomF32 t = ease_in_out_cubic(cycle * 2.0f);
        arc_length = spinner->arc_min + (spinner->arc_max - spinner->arc_min) * t;
        rotation_offset = cycle * TWO_PI * 1.5f;
    } else {
        /* Shrinking phase */
        VenomF32 t = ease_in_out_cubic((cycle - 0.5f) * 2.0f);
        arc_length = spinner->arc_max - (spinner->arc_max - spinner->arc_min) * t;
        rotation_offset = cycle * TWO_PI * 1.5f + (spinner->arc_max - arc_length) * TWO_PI;
    }
    
    /* Continuous base rotation */
    VenomF32 base_rotation = (VenomF32)elapsed * TWO_PI * 0.8f;  /* Slow continuous rotation */
    VenomF32 start_angle = base_rotation + rotation_offset - PI / 2.0f;  /* Start at top */
    VenomF32 arc_angle = arc_length * TWO_PI;
    
    /* Draw track (background circle) */
    if (spinner->show_track) {
        VenomPaint track_paint = venom_paint_stroke(spinner->track_color, spinner->stroke_width);
        venom_canvas_draw_circle(canvas, cx, cy, r, &track_paint);
    }
    
    /* Draw animated arc using line segments for smooth curve */
    VenomPaint arc_paint = venom_paint_stroke(spinner->color, spinner->stroke_width);
    
    /* Higher segment count for smoother arc */
    int segments = (int)(arc_length * 40.0f);
    if (segments < 8) segments = 8;
    if (segments > 60) segments = 60;
    
    for (int i = 0; i < segments; i++) {
        VenomF32 t1 = (VenomF32)i / (VenomF32)segments;
        VenomF32 t2 = (VenomF32)(i + 1) / (VenomF32)segments;
        
        VenomF32 a1 = start_angle + arc_angle * t1;
        VenomF32 a2 = start_angle + arc_angle * t2;
        
        VenomF32 x1 = cx + r * cosf(a1);
        VenomF32 y1 = cy + r * sinf(a1);
        VenomF32 x2 = cx + r * cosf(a2);
        VenomF32 y2 = cy + r * sinf(a2);
        
        venom_canvas_draw_line(canvas, x1, y1, x2, y2, &arc_paint);
    }
    
    /* Always request redraw for continuous animation */
    widget->needs_redraw = VENOM_TRUE;
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

void venom_spinner_set_stroke_width(VenomSpinner* spinner, VenomF32 width) {
    if (spinner && width > 0) {
        spinner->stroke_width = width;
        venom_widget_invalidate((VenomWidget*)spinner);
    }
}

void venom_spinner_set_duration(VenomSpinner* spinner, VenomF32 seconds) {
    if (spinner && seconds > 0) {
        spinner->animation_duration = seconds;
    }
}

void venom_spinner_animate(VenomSpinner* spinner, VenomF32 delta_time) {
    /* No longer needed - animation is automatic in draw() */
    (void)spinner; (void)delta_time;
}

VenomWidget* _venom_spinner_build(const VenomSpinnerConfig* config) {
    VenomResultPtr result = venom_spinner_create();
    if (!result.ok) return NULL;
    
    VenomSpinner* spinner = (VenomSpinner*)result.value;
    
    if (config->size > 0) spinner->size = config->size;
    if (config->color.a > 0) spinner->color = config->color;
    
    return (VenomWidget*)spinner;
}
