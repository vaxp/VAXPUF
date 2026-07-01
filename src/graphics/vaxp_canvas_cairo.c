/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_canvas_cairo.c - Cairo implementation of canvas
 * 
 * Fallback when Skia is not available.
 */

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "vaxp/graphics/vaxp_canvas.h"
#include "vaxp/backend/vaxp_surface.h"
#include "vaxp/core/vaxp_memory.h"

#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <pango/pangocairo.h>
#include <string.h>

/* ============================================================================
 * CAIRO CANVAS STRUCTURE
 * ============================================================================ */

typedef struct VaxpCairoCanvas {
    VaxpCanvas base;
    
    cairo_surface_t* surface;
    cairo_t* cr;
    VaxpBool owns_surface;
    
} VaxpCairoCanvas;

/* ============================================================================
 * CAIRO PATH STRUCTURE
 * ============================================================================ */

struct VaxpPath {
    VAXP_REF_HEADER;
    cairo_path_t* cairo_path;
    
    /* We build the path using a temporary cairo context */
    cairo_surface_t* temp_surface;
    cairo_t* temp_cr;
};

/* ============================================================================
 * HELPER: Convert types
 * ============================================================================ */

static void set_cairo_color(cairo_t* cr, VaxpColor c) {
    cairo_set_source_rgba(cr, c.r / 255.0, c.g / 255.0, c.b / 255.0, c.a / 255.0);
}

static void apply_paint(cairo_t* cr, const VaxpPaint* paint) {
    set_cairo_color(cr, paint->color);
    cairo_set_line_width(cr, paint->stroke_width);
    
    switch (paint->stroke_cap) {
        case VAXP_STROKE_CAP_BUTT: cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT); break;
        case VAXP_STROKE_CAP_ROUND: cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND); break;
        case VAXP_STROKE_CAP_SQUARE: cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE); break;
    }
    
    switch (paint->stroke_join) {
        case VAXP_STROKE_JOIN_MITER: cairo_set_line_join(cr, CAIRO_LINE_JOIN_MITER); break;
        case VAXP_STROKE_JOIN_ROUND: cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND); break;
        case VAXP_STROKE_JOIN_BEVEL: cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL); break;
    }
    
    cairo_set_miter_limit(cr, paint->stroke_miter);
    cairo_set_antialias(cr, paint->anti_alias ? CAIRO_ANTIALIAS_DEFAULT : CAIRO_ANTIALIAS_NONE);
}

static void do_draw(cairo_t* cr, const VaxpPaint* paint) {
    switch (paint->style) {
        case VAXP_PAINT_FILL:
            cairo_fill(cr);
            break;
        case VAXP_PAINT_STROKE:
            cairo_stroke(cr);
            break;
        case VAXP_PAINT_FILL_AND_STROKE:
            cairo_fill_preserve(cr);
            cairo_stroke(cr);
            break;
    }
}

/* ============================================================================
 * CANVAS OPERATIONS
 * ============================================================================ */

static void cairo_canvas_destroy(VaxpCanvas* canvas) {
    VaxpCairoCanvas* c = (VaxpCairoCanvas*)canvas;
    if (c->cr) {
        cairo_destroy(c->cr);
        c->cr = NULL;
    }
    if (c->owns_surface && c->surface) {
        cairo_surface_destroy(c->surface);
        c->surface = NULL;
    }
}

static void cairo_canvas_save(VaxpCanvas* canvas) {
    VaxpCairoCanvas* c = (VaxpCairoCanvas*)canvas;
    cairo_save(c->cr);
}

static void cairo_canvas_restore(VaxpCanvas* canvas) {
    VaxpCairoCanvas* c = (VaxpCairoCanvas*)canvas;
    cairo_restore(c->cr);
}

static void cairo_canvas_translate(VaxpCanvas* canvas, VaxpF32 dx, VaxpF32 dy) {
    VaxpCairoCanvas* c = (VaxpCairoCanvas*)canvas;
    cairo_translate(c->cr, dx, dy);
}

static void cairo_canvas_scale(VaxpCanvas* canvas, VaxpF32 sx, VaxpF32 sy) {
    VaxpCairoCanvas* c = (VaxpCairoCanvas*)canvas;
    cairo_scale(c->cr, sx, sy);
}

static void cairo_canvas_rotate(VaxpCanvas* canvas, VaxpF32 degrees) {
    VaxpCairoCanvas* c = (VaxpCairoCanvas*)canvas;
    cairo_rotate(c->cr, degrees * M_PI / 180.0);
}

static void cairo_canvas_clip_rect(VaxpCanvas* canvas, VaxpRectF rect) {
    VaxpCairoCanvas* c = (VaxpCairoCanvas*)canvas;
    cairo_rectangle(c->cr, rect.x, rect.y, rect.width, rect.height);
    cairo_clip(c->cr);
}

static void cairo_canvas_clip_rounded_rect(VaxpCanvas* canvas, VaxpRectF rect, VaxpF32 radius) {
    VaxpCairoCanvas* c = (VaxpCairoCanvas*)canvas;
    
    VaxpF32 x = rect.x, y = rect.y, w = rect.width, h = rect.height, r = radius;
    
    cairo_new_path(c->cr);
    cairo_arc(c->cr, x + w - r, y + r, r, -M_PI/2, 0);
    cairo_arc(c->cr, x + w - r, y + h - r, r, 0, M_PI/2);
    cairo_arc(c->cr, x + r, y + h - r, r, M_PI/2, M_PI);
    cairo_arc(c->cr, x + r, y + r, r, M_PI, 3*M_PI/2);
    cairo_close_path(c->cr);
    cairo_clip(c->cr);
}

static void cairo_canvas_clear(VaxpCanvas* canvas, VaxpColor color) {
    VaxpCairoCanvas* c = (VaxpCairoCanvas*)canvas;
    set_cairo_color(c->cr, color);
    cairo_paint(c->cr);
}

static void cairo_canvas_draw_rect(VaxpCanvas* canvas, VaxpRectF rect, const VaxpPaint* paint) {
    VaxpCairoCanvas* c = (VaxpCairoCanvas*)canvas;
    apply_paint(c->cr, paint);
    cairo_rectangle(c->cr, rect.x, rect.y, rect.width, rect.height);
    do_draw(c->cr, paint);
}

static void cairo_canvas_draw_rounded_rect(VaxpCanvas* canvas, VaxpRectF rect, 
                                            VaxpF32 radius, const VaxpPaint* paint) {
    VaxpCairoCanvas* c = (VaxpCairoCanvas*)canvas;
    apply_paint(c->cr, paint);
    
    VaxpF32 x = rect.x, y = rect.y, w = rect.width, h = rect.height, r = radius;
    
    cairo_new_path(c->cr);
    cairo_arc(c->cr, x + w - r, y + r, r, -M_PI/2, 0);
    cairo_arc(c->cr, x + w - r, y + h - r, r, 0, M_PI/2);
    cairo_arc(c->cr, x + r, y + h - r, r, M_PI/2, M_PI);
    cairo_arc(c->cr, x + r, y + r, r, M_PI, 3*M_PI/2);
    cairo_close_path(c->cr);
    
    do_draw(c->cr, paint);
}

static void cairo_canvas_draw_circle(VaxpCanvas* canvas, VaxpF32 cx, VaxpF32 cy, 
                                      VaxpF32 radius, const VaxpPaint* paint) {
    VaxpCairoCanvas* c = (VaxpCairoCanvas*)canvas;
    apply_paint(c->cr, paint);
    cairo_arc(c->cr, cx, cy, radius, 0, 2 * M_PI);
    do_draw(c->cr, paint);
}

static void cairo_canvas_draw_oval(VaxpCanvas* canvas, VaxpRectF rect, const VaxpPaint* paint) {
    VaxpCairoCanvas* c = (VaxpCairoCanvas*)canvas;
    apply_paint(c->cr, paint);
    
    cairo_save(c->cr);
    cairo_translate(c->cr, rect.x + rect.width / 2, rect.y + rect.height / 2);
    cairo_scale(c->cr, rect.width / 2, rect.height / 2);
    cairo_arc(c->cr, 0, 0, 1, 0, 2 * M_PI);
    cairo_restore(c->cr);
    
    do_draw(c->cr, paint);
}

static void cairo_canvas_draw_line(VaxpCanvas* canvas, VaxpF32 x1, VaxpF32 y1, 
                                    VaxpF32 x2, VaxpF32 y2, const VaxpPaint* paint) {
    VaxpCairoCanvas* c = (VaxpCairoCanvas*)canvas;
    apply_paint(c->cr, paint);
    cairo_move_to(c->cr, x1, y1);
    cairo_line_to(c->cr, x2, y2);
    cairo_stroke(c->cr);
}

static void cairo_canvas_draw_path(VaxpCanvas* canvas, const VaxpPath* path, const VaxpPaint* paint) {
    VaxpCairoCanvas* c = (VaxpCairoCanvas*)canvas;
    if (!path || !path->cairo_path) return;
    
    apply_paint(c->cr, paint);
    cairo_append_path(c->cr, path->cairo_path);
    do_draw(c->cr, paint);
}

static void cairo_canvas_draw_text(VaxpCanvas* canvas, const char* text, VaxpF32 x, VaxpF32 y,
                                    const VaxpFont* font, const VaxpPaint* paint) {
    VaxpCairoCanvas* c = (VaxpCairoCanvas*)canvas;
    (void)font;  /* TODO: custom font support */
    
    if (!text || text[0] == '\0') return;
    
    apply_paint(c->cr, paint);
    
    /* Use Pango for proper international text rendering (Arabic, CJK, etc.) */
    PangoLayout* layout = pango_cairo_create_layout(c->cr);
    
    float actual_font_size = (font && font->size > 0) ? font->size : 14.0f;
    const char* actual_family = (font && font->family && font->family[0]) ? font->family : "Noto Sans";
    
    char font_desc_str[128];
    snprintf(font_desc_str, sizeof(font_desc_str), "%s %f", actual_family, actual_font_size);
    PangoFontDescription* desc = pango_font_description_from_string(font_desc_str);
    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);
    
    /* Set text - Pango handles UTF-8 and RTL automatically */
    pango_layout_set_text(layout, text, -1);
    
    /* Get text dimensions for baseline positioning */
    PangoRectangle logical_rect;
    pango_layout_get_pixel_extents(layout, NULL, &logical_rect);
    
    /* Position - adjust for baseline (Cairo y is baseline, Pango uses top) */
    VaxpF32 adjusted_y = y - logical_rect.height * 0.7f;  /* Approximate baseline */
    cairo_move_to(c->cr, x, adjusted_y);
    
    /* Render with Pango - this handles text shaping (Arabic joining) automatically */
    pango_cairo_show_layout(c->cr, layout);
    
    g_object_unref(layout);
}

static void cairo_canvas_draw_image(VaxpCanvas* canvas, const VaxpImage* image, VaxpF32 x, VaxpF32 y) {
    (void)canvas; (void)image; (void)x; (void)y;
    /* TODO: implement */
}

static void cairo_canvas_draw_image_rect(VaxpCanvas* canvas, const VaxpImage* image,
                                          VaxpRectF src, VaxpRectF dst, const VaxpPaint* paint) {
    (void)canvas; (void)image; (void)src; (void)dst; (void)paint;
    /* TODO: implement */
}

static void cairo_canvas_flush(VaxpCanvas* canvas) {
    VaxpCairoCanvas* c = (VaxpCairoCanvas*)canvas;
    cairo_surface_flush(c->surface);
}

static VaxpSize2D cairo_canvas_get_size(VaxpCanvas* canvas) {
    return (VaxpSize2D){ .width = canvas->width, .height = canvas->height };
}

/* ============================================================================
 * CANVAS VTABLE
 * ============================================================================ */

static const VaxpCanvasOps cairo_canvas_ops = {
    .destroy = cairo_canvas_destroy,
    .save = cairo_canvas_save,
    .restore = cairo_canvas_restore,
    .translate = cairo_canvas_translate,
    .scale = cairo_canvas_scale,
    .rotate = cairo_canvas_rotate,
    .clip_rect = cairo_canvas_clip_rect,
    .clip_rounded_rect = cairo_canvas_clip_rounded_rect,
    .clear = cairo_canvas_clear,
    .draw_rect = cairo_canvas_draw_rect,
    .draw_rounded_rect = cairo_canvas_draw_rounded_rect,
    .draw_circle = cairo_canvas_draw_circle,
    .draw_oval = cairo_canvas_draw_oval,
    .draw_line = cairo_canvas_draw_line,
    .draw_path = cairo_canvas_draw_path,
    .draw_text = cairo_canvas_draw_text,
    .draw_image = cairo_canvas_draw_image,
    .draw_image_rect = cairo_canvas_draw_image_rect,
    .flush = cairo_canvas_flush,
    .get_size = cairo_canvas_get_size,
};

/* ============================================================================
 * CANVAS CREATION
 * ============================================================================ */

static void canvas_destructor(void* self) {
    cairo_canvas_destroy((VaxpCanvas*)self);
}

VaxpResultPtr vaxp_canvas_create_for_xlib(Display* display, Window window, 
                                             Visual* visual, VaxpU32 width, VaxpU32 height) {
    cairo_surface_t* surface = cairo_xlib_surface_create(display, window, visual, width, height);
    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
        return VAXP_ERR_PTR(VAXP_ERROR_SURFACE_CREATE);
    }
    
    cairo_t* cr = cairo_create(surface);
    if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy(surface);
        return VAXP_ERR_PTR(VAXP_ERROR_CANVAS_CREATE);
    }
    
    VaxpCairoCanvas* canvas = (VaxpCairoCanvas*)VAXP_REF_NEW(VaxpCairoCanvas, canvas_destructor);
    if (!canvas) {
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
        return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
    }
    
    canvas->base.ops = &cairo_canvas_ops;
    canvas->base.width = width;
    canvas->base.height = height;
    canvas->surface = surface;
    canvas->cr = cr;
    canvas->owns_surface = VAXP_TRUE;
    
    return VAXP_OK_PTR(canvas);
}

VaxpResultPtr vaxp_canvas_create_offscreen(VaxpU32 width, VaxpU32 height) {
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
        return VAXP_ERR_PTR(VAXP_ERROR_SURFACE_CREATE);
    }
    
    cairo_t* cr = cairo_create(surface);
    if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy(surface);
        return VAXP_ERR_PTR(VAXP_ERROR_CANVAS_CREATE);
    }
    
    VaxpCairoCanvas* canvas = (VaxpCairoCanvas*)VAXP_REF_NEW(VaxpCairoCanvas, canvas_destructor);
    if (!canvas) {
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
        return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
    }
    
    canvas->base.ops = &cairo_canvas_ops;
    canvas->base.width = width;
    canvas->base.height = height;
    canvas->surface = surface;
    canvas->cr = cr;
    canvas->owns_surface = VAXP_TRUE;
    
    return VAXP_OK_PTR(canvas);
}

/* ============================================================================
 * PATH IMPLEMENTATION
 * ============================================================================ */

static void path_destructor(void* self) {
    VaxpPath* path = (VaxpPath*)self;
    if (path->cairo_path) {
        cairo_path_destroy(path->cairo_path);
        path->cairo_path = NULL;
    }
    if (path->temp_cr) {
        cairo_destroy(path->temp_cr);
        path->temp_cr = NULL;
    }
    if (path->temp_surface) {
        cairo_surface_destroy(path->temp_surface);
        path->temp_surface = NULL;
    }
}

VaxpResultPtr vaxp_path_create(void) {
    VaxpPath* path = (VaxpPath*)VAXP_REF_NEW(VaxpPath, path_destructor);
    if (!path) {
        return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
    }
    
    /* Create a small temporary surface for path building */
    path->temp_surface = cairo_image_surface_create(CAIRO_FORMAT_A1, 1, 1);
    path->temp_cr = cairo_create(path->temp_surface);
    path->cairo_path = NULL;
    
    return VAXP_OK_PTR(path);
}

void vaxp_path_move_to(VaxpPath* path, VaxpF32 x, VaxpF32 y) {
    if (path && path->temp_cr) cairo_move_to(path->temp_cr, x, y);
}

void vaxp_path_line_to(VaxpPath* path, VaxpF32 x, VaxpF32 y) {
    if (path && path->temp_cr) cairo_line_to(path->temp_cr, x, y);
}

void vaxp_path_quad_to(VaxpPath* path, VaxpF32 cx, VaxpF32 cy, VaxpF32 x, VaxpF32 y) {
    if (!path || !path->temp_cr) return;
    /* Cairo doesn't have quadratic bezier directly, convert to cubic */
    double x0, y0;
    cairo_get_current_point(path->temp_cr, &x0, &y0);
    double c1x = x0 + 2.0/3.0 * (cx - x0);
    double c1y = y0 + 2.0/3.0 * (cy - y0);
    double c2x = x + 2.0/3.0 * (cx - x);
    double c2y = y + 2.0/3.0 * (cy - y);
    cairo_curve_to(path->temp_cr, c1x, c1y, c2x, c2y, x, y);
}

void vaxp_path_cubic_to(VaxpPath* path, VaxpF32 c1x, VaxpF32 c1y, 
                          VaxpF32 c2x, VaxpF32 c2y, VaxpF32 x, VaxpF32 y) {
    if (path && path->temp_cr) cairo_curve_to(path->temp_cr, c1x, c1y, c2x, c2y, x, y);
}

void vaxp_path_close(VaxpPath* path) {
    if (path && path->temp_cr) {
        cairo_close_path(path->temp_cr);
        /* Copy the path */
        if (path->cairo_path) cairo_path_destroy(path->cairo_path);
        path->cairo_path = cairo_copy_path(path->temp_cr);
    }
}

void vaxp_path_add_rect(VaxpPath* path, VaxpRectF rect) {
    if (path && path->temp_cr) {
        cairo_rectangle(path->temp_cr, rect.x, rect.y, rect.width, rect.height);
        if (path->cairo_path) cairo_path_destroy(path->cairo_path);
        path->cairo_path = cairo_copy_path(path->temp_cr);
    }
}

void vaxp_path_add_rounded_rect(VaxpPath* path, VaxpRectF rect, VaxpF32 radius) {
    if (!path || !path->temp_cr) return;
    
    VaxpF32 x = rect.x, y = rect.y, w = rect.width, h = rect.height, r = radius;
    
    cairo_new_path(path->temp_cr);
    cairo_arc(path->temp_cr, x + w - r, y + r, r, -M_PI/2, 0);
    cairo_arc(path->temp_cr, x + w - r, y + h - r, r, 0, M_PI/2);
    cairo_arc(path->temp_cr, x + r, y + h - r, r, M_PI/2, M_PI);
    cairo_arc(path->temp_cr, x + r, y + r, r, M_PI, 3*M_PI/2);
    cairo_close_path(path->temp_cr);
    
    if (path->cairo_path) cairo_path_destroy(path->cairo_path);
    path->cairo_path = cairo_copy_path(path->temp_cr);
}

void vaxp_path_add_circle(VaxpPath* path, VaxpF32 cx, VaxpF32 cy, VaxpF32 radius) {
    if (path && path->temp_cr) {
        cairo_arc(path->temp_cr, cx, cy, radius, 0, 2 * M_PI);
        if (path->cairo_path) cairo_path_destroy(path->cairo_path);
        path->cairo_path = cairo_copy_path(path->temp_cr);
    }
}

void vaxp_path_reset(VaxpPath* path) {
    if (path) {
        if (path->temp_cr) cairo_new_path(path->temp_cr);
        if (path->cairo_path) {
            cairo_path_destroy(path->cairo_path);
            path->cairo_path = NULL;
        }
    }
}
