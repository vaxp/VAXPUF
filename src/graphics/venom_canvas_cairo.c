/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_canvas_cairo.c - Cairo implementation of canvas
 * 
 * Fallback when Skia is not available.
 */

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "venom/graphics/venom_canvas.h"
#include "venom/backend/venom_surface.h"
#include "venom/core/venom_memory.h"

#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <pango/pangocairo.h>
#include <string.h>

/* ============================================================================
 * CAIRO CANVAS STRUCTURE
 * ============================================================================ */

typedef struct VenomCairoCanvas {
    VenomCanvas base;
    
    cairo_surface_t* surface;
    cairo_t* cr;
    VenomBool owns_surface;
    
} VenomCairoCanvas;

/* ============================================================================
 * CAIRO PATH STRUCTURE
 * ============================================================================ */

struct VenomPath {
    VENOM_REF_HEADER;
    cairo_path_t* cairo_path;
    
    /* We build the path using a temporary cairo context */
    cairo_surface_t* temp_surface;
    cairo_t* temp_cr;
};

/* ============================================================================
 * HELPER: Convert types
 * ============================================================================ */

static void set_cairo_color(cairo_t* cr, VenomColor c) {
    cairo_set_source_rgba(cr, c.r / 255.0, c.g / 255.0, c.b / 255.0, c.a / 255.0);
}

static void apply_paint(cairo_t* cr, const VenomPaint* paint) {
    set_cairo_color(cr, paint->color);
    cairo_set_line_width(cr, paint->stroke_width);
    
    switch (paint->stroke_cap) {
        case VENOM_STROKE_CAP_BUTT: cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT); break;
        case VENOM_STROKE_CAP_ROUND: cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND); break;
        case VENOM_STROKE_CAP_SQUARE: cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE); break;
    }
    
    switch (paint->stroke_join) {
        case VENOM_STROKE_JOIN_MITER: cairo_set_line_join(cr, CAIRO_LINE_JOIN_MITER); break;
        case VENOM_STROKE_JOIN_ROUND: cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND); break;
        case VENOM_STROKE_JOIN_BEVEL: cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL); break;
    }
    
    cairo_set_miter_limit(cr, paint->stroke_miter);
    cairo_set_antialias(cr, paint->anti_alias ? CAIRO_ANTIALIAS_DEFAULT : CAIRO_ANTIALIAS_NONE);
}

static void do_draw(cairo_t* cr, const VenomPaint* paint) {
    switch (paint->style) {
        case VENOM_PAINT_FILL:
            cairo_fill(cr);
            break;
        case VENOM_PAINT_STROKE:
            cairo_stroke(cr);
            break;
        case VENOM_PAINT_FILL_AND_STROKE:
            cairo_fill_preserve(cr);
            cairo_stroke(cr);
            break;
    }
}

/* ============================================================================
 * CANVAS OPERATIONS
 * ============================================================================ */

static void cairo_canvas_destroy(VenomCanvas* canvas) {
    VenomCairoCanvas* c = (VenomCairoCanvas*)canvas;
    if (c->cr) {
        cairo_destroy(c->cr);
        c->cr = NULL;
    }
    if (c->owns_surface && c->surface) {
        cairo_surface_destroy(c->surface);
        c->surface = NULL;
    }
}

static void cairo_canvas_save(VenomCanvas* canvas) {
    VenomCairoCanvas* c = (VenomCairoCanvas*)canvas;
    cairo_save(c->cr);
}

static void cairo_canvas_restore(VenomCanvas* canvas) {
    VenomCairoCanvas* c = (VenomCairoCanvas*)canvas;
    cairo_restore(c->cr);
}

static void cairo_canvas_translate(VenomCanvas* canvas, VenomF32 dx, VenomF32 dy) {
    VenomCairoCanvas* c = (VenomCairoCanvas*)canvas;
    cairo_translate(c->cr, dx, dy);
}

static void cairo_canvas_scale(VenomCanvas* canvas, VenomF32 sx, VenomF32 sy) {
    VenomCairoCanvas* c = (VenomCairoCanvas*)canvas;
    cairo_scale(c->cr, sx, sy);
}

static void cairo_canvas_rotate(VenomCanvas* canvas, VenomF32 degrees) {
    VenomCairoCanvas* c = (VenomCairoCanvas*)canvas;
    cairo_rotate(c->cr, degrees * M_PI / 180.0);
}

static void cairo_canvas_clip_rect(VenomCanvas* canvas, VenomRectF rect) {
    VenomCairoCanvas* c = (VenomCairoCanvas*)canvas;
    cairo_rectangle(c->cr, rect.x, rect.y, rect.width, rect.height);
    cairo_clip(c->cr);
}

static void cairo_canvas_clip_rounded_rect(VenomCanvas* canvas, VenomRectF rect, VenomF32 radius) {
    VenomCairoCanvas* c = (VenomCairoCanvas*)canvas;
    
    VenomF32 x = rect.x, y = rect.y, w = rect.width, h = rect.height, r = radius;
    
    cairo_new_path(c->cr);
    cairo_arc(c->cr, x + w - r, y + r, r, -M_PI/2, 0);
    cairo_arc(c->cr, x + w - r, y + h - r, r, 0, M_PI/2);
    cairo_arc(c->cr, x + r, y + h - r, r, M_PI/2, M_PI);
    cairo_arc(c->cr, x + r, y + r, r, M_PI, 3*M_PI/2);
    cairo_close_path(c->cr);
    cairo_clip(c->cr);
}

static void cairo_canvas_clear(VenomCanvas* canvas, VenomColor color) {
    VenomCairoCanvas* c = (VenomCairoCanvas*)canvas;
    set_cairo_color(c->cr, color);
    cairo_paint(c->cr);
}

static void cairo_canvas_draw_rect(VenomCanvas* canvas, VenomRectF rect, const VenomPaint* paint) {
    VenomCairoCanvas* c = (VenomCairoCanvas*)canvas;
    apply_paint(c->cr, paint);
    cairo_rectangle(c->cr, rect.x, rect.y, rect.width, rect.height);
    do_draw(c->cr, paint);
}

static void cairo_canvas_draw_rounded_rect(VenomCanvas* canvas, VenomRectF rect, 
                                            VenomF32 radius, const VenomPaint* paint) {
    VenomCairoCanvas* c = (VenomCairoCanvas*)canvas;
    apply_paint(c->cr, paint);
    
    VenomF32 x = rect.x, y = rect.y, w = rect.width, h = rect.height, r = radius;
    
    cairo_new_path(c->cr);
    cairo_arc(c->cr, x + w - r, y + r, r, -M_PI/2, 0);
    cairo_arc(c->cr, x + w - r, y + h - r, r, 0, M_PI/2);
    cairo_arc(c->cr, x + r, y + h - r, r, M_PI/2, M_PI);
    cairo_arc(c->cr, x + r, y + r, r, M_PI, 3*M_PI/2);
    cairo_close_path(c->cr);
    
    do_draw(c->cr, paint);
}

static void cairo_canvas_draw_circle(VenomCanvas* canvas, VenomF32 cx, VenomF32 cy, 
                                      VenomF32 radius, const VenomPaint* paint) {
    VenomCairoCanvas* c = (VenomCairoCanvas*)canvas;
    apply_paint(c->cr, paint);
    cairo_arc(c->cr, cx, cy, radius, 0, 2 * M_PI);
    do_draw(c->cr, paint);
}

static void cairo_canvas_draw_oval(VenomCanvas* canvas, VenomRectF rect, const VenomPaint* paint) {
    VenomCairoCanvas* c = (VenomCairoCanvas*)canvas;
    apply_paint(c->cr, paint);
    
    cairo_save(c->cr);
    cairo_translate(c->cr, rect.x + rect.width / 2, rect.y + rect.height / 2);
    cairo_scale(c->cr, rect.width / 2, rect.height / 2);
    cairo_arc(c->cr, 0, 0, 1, 0, 2 * M_PI);
    cairo_restore(c->cr);
    
    do_draw(c->cr, paint);
}

static void cairo_canvas_draw_line(VenomCanvas* canvas, VenomF32 x1, VenomF32 y1, 
                                    VenomF32 x2, VenomF32 y2, const VenomPaint* paint) {
    VenomCairoCanvas* c = (VenomCairoCanvas*)canvas;
    apply_paint(c->cr, paint);
    cairo_move_to(c->cr, x1, y1);
    cairo_line_to(c->cr, x2, y2);
    cairo_stroke(c->cr);
}

static void cairo_canvas_draw_path(VenomCanvas* canvas, const VenomPath* path, const VenomPaint* paint) {
    VenomCairoCanvas* c = (VenomCairoCanvas*)canvas;
    if (!path || !path->cairo_path) return;
    
    apply_paint(c->cr, paint);
    cairo_append_path(c->cr, path->cairo_path);
    do_draw(c->cr, paint);
}

static void cairo_canvas_draw_text(VenomCanvas* canvas, const char* text, VenomF32 x, VenomF32 y,
                                    const VenomFont* font, const VenomPaint* paint) {
    VenomCairoCanvas* c = (VenomCairoCanvas*)canvas;
    (void)font;  /* TODO: custom font support */
    
    if (!text || text[0] == '\0') return;
    
    apply_paint(c->cr, paint);
    
    /* Use Pango for proper international text rendering (Arabic, CJK, etc.) */
    PangoLayout* layout = pango_cairo_create_layout(c->cr);
    
    /* Set font - use "Noto Sans" which has good Unicode coverage, fallback to Sans */
    PangoFontDescription* desc = pango_font_description_from_string("Noto Sans 14");
    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);
    
    /* Set text - Pango handles UTF-8 and RTL automatically */
    pango_layout_set_text(layout, text, -1);
    
    /* Get text dimensions for baseline positioning */
    PangoRectangle logical_rect;
    pango_layout_get_pixel_extents(layout, NULL, &logical_rect);
    
    /* Position - adjust for baseline (Cairo y is baseline, Pango uses top) */
    VenomF32 adjusted_y = y - logical_rect.height * 0.7f;  /* Approximate baseline */
    cairo_move_to(c->cr, x, adjusted_y);
    
    /* Render with Pango - this handles text shaping (Arabic joining) automatically */
    pango_cairo_show_layout(c->cr, layout);
    
    g_object_unref(layout);
}

static void cairo_canvas_draw_image(VenomCanvas* canvas, const VenomImage* image, VenomF32 x, VenomF32 y) {
    (void)canvas; (void)image; (void)x; (void)y;
    /* TODO: implement */
}

static void cairo_canvas_draw_image_rect(VenomCanvas* canvas, const VenomImage* image,
                                          VenomRectF src, VenomRectF dst, const VenomPaint* paint) {
    (void)canvas; (void)image; (void)src; (void)dst; (void)paint;
    /* TODO: implement */
}

static void cairo_canvas_flush(VenomCanvas* canvas) {
    VenomCairoCanvas* c = (VenomCairoCanvas*)canvas;
    cairo_surface_flush(c->surface);
}

static VenomSize2D cairo_canvas_get_size(VenomCanvas* canvas) {
    return (VenomSize2D){ .width = canvas->width, .height = canvas->height };
}

/* ============================================================================
 * CANVAS VTABLE
 * ============================================================================ */

static const VenomCanvasOps cairo_canvas_ops = {
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
    cairo_canvas_destroy((VenomCanvas*)self);
}

VenomResultPtr venom_canvas_create_for_xlib(Display* display, Window window, 
                                             Visual* visual, VenomU32 width, VenomU32 height) {
    cairo_surface_t* surface = cairo_xlib_surface_create(display, window, visual, width, height);
    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
        return VENOM_ERR_PTR(VENOM_ERROR_SURFACE_CREATE);
    }
    
    cairo_t* cr = cairo_create(surface);
    if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy(surface);
        return VENOM_ERR_PTR(VENOM_ERROR_CANVAS_CREATE);
    }
    
    VenomCairoCanvas* canvas = (VenomCairoCanvas*)VENOM_REF_NEW(VenomCairoCanvas, canvas_destructor);
    if (!canvas) {
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
        return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    canvas->base.ops = &cairo_canvas_ops;
    canvas->base.width = width;
    canvas->base.height = height;
    canvas->surface = surface;
    canvas->cr = cr;
    canvas->owns_surface = VENOM_TRUE;
    
    return VENOM_OK_PTR(canvas);
}

VenomResultPtr venom_canvas_create_offscreen(VenomU32 width, VenomU32 height) {
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
        return VENOM_ERR_PTR(VENOM_ERROR_SURFACE_CREATE);
    }
    
    cairo_t* cr = cairo_create(surface);
    if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy(surface);
        return VENOM_ERR_PTR(VENOM_ERROR_CANVAS_CREATE);
    }
    
    VenomCairoCanvas* canvas = (VenomCairoCanvas*)VENOM_REF_NEW(VenomCairoCanvas, canvas_destructor);
    if (!canvas) {
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
        return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    canvas->base.ops = &cairo_canvas_ops;
    canvas->base.width = width;
    canvas->base.height = height;
    canvas->surface = surface;
    canvas->cr = cr;
    canvas->owns_surface = VENOM_TRUE;
    
    return VENOM_OK_PTR(canvas);
}

/* ============================================================================
 * PATH IMPLEMENTATION
 * ============================================================================ */

static void path_destructor(void* self) {
    VenomPath* path = (VenomPath*)self;
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

VenomResultPtr venom_path_create(void) {
    VenomPath* path = (VenomPath*)VENOM_REF_NEW(VenomPath, path_destructor);
    if (!path) {
        return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    /* Create a small temporary surface for path building */
    path->temp_surface = cairo_image_surface_create(CAIRO_FORMAT_A1, 1, 1);
    path->temp_cr = cairo_create(path->temp_surface);
    path->cairo_path = NULL;
    
    return VENOM_OK_PTR(path);
}

void venom_path_move_to(VenomPath* path, VenomF32 x, VenomF32 y) {
    if (path && path->temp_cr) cairo_move_to(path->temp_cr, x, y);
}

void venom_path_line_to(VenomPath* path, VenomF32 x, VenomF32 y) {
    if (path && path->temp_cr) cairo_line_to(path->temp_cr, x, y);
}

void venom_path_quad_to(VenomPath* path, VenomF32 cx, VenomF32 cy, VenomF32 x, VenomF32 y) {
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

void venom_path_cubic_to(VenomPath* path, VenomF32 c1x, VenomF32 c1y, 
                          VenomF32 c2x, VenomF32 c2y, VenomF32 x, VenomF32 y) {
    if (path && path->temp_cr) cairo_curve_to(path->temp_cr, c1x, c1y, c2x, c2y, x, y);
}

void venom_path_close(VenomPath* path) {
    if (path && path->temp_cr) {
        cairo_close_path(path->temp_cr);
        /* Copy the path */
        if (path->cairo_path) cairo_path_destroy(path->cairo_path);
        path->cairo_path = cairo_copy_path(path->temp_cr);
    }
}

void venom_path_add_rect(VenomPath* path, VenomRectF rect) {
    if (path && path->temp_cr) {
        cairo_rectangle(path->temp_cr, rect.x, rect.y, rect.width, rect.height);
        if (path->cairo_path) cairo_path_destroy(path->cairo_path);
        path->cairo_path = cairo_copy_path(path->temp_cr);
    }
}

void venom_path_add_rounded_rect(VenomPath* path, VenomRectF rect, VenomF32 radius) {
    if (!path || !path->temp_cr) return;
    
    VenomF32 x = rect.x, y = rect.y, w = rect.width, h = rect.height, r = radius;
    
    cairo_new_path(path->temp_cr);
    cairo_arc(path->temp_cr, x + w - r, y + r, r, -M_PI/2, 0);
    cairo_arc(path->temp_cr, x + w - r, y + h - r, r, 0, M_PI/2);
    cairo_arc(path->temp_cr, x + r, y + h - r, r, M_PI/2, M_PI);
    cairo_arc(path->temp_cr, x + r, y + r, r, M_PI, 3*M_PI/2);
    cairo_close_path(path->temp_cr);
    
    if (path->cairo_path) cairo_path_destroy(path->cairo_path);
    path->cairo_path = cairo_copy_path(path->temp_cr);
}

void venom_path_add_circle(VenomPath* path, VenomF32 cx, VenomF32 cy, VenomF32 radius) {
    if (path && path->temp_cr) {
        cairo_arc(path->temp_cr, cx, cy, radius, 0, 2 * M_PI);
        if (path->cairo_path) cairo_path_destroy(path->cairo_path);
        path->cairo_path = cairo_copy_path(path->temp_cr);
    }
}

void venom_path_reset(VenomPath* path) {
    if (path) {
        if (path->temp_cr) cairo_new_path(path->temp_cr);
        if (path->cairo_path) {
            cairo_path_destroy(path->cairo_path);
            path->cairo_path = NULL;
        }
    }
}
