/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_canvas_skia.cpp - Skia implementation of canvas
 * 
 * This is C++ because Skia is a C++ library.
 * We expose only C interfaces to the rest of the framework.
 */

#include "vaxp/graphics/vaxp_canvas.h"
#include "vaxp/backend/vaxp_surface.h"
#include "vaxp/core/vaxp_memory.h"

/* Skia headers */
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkImage.h"
#include "include/core/SkData.h"
#include "include/core/SkColorSpace.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"

#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

extern "C" {

/* ============================================================================
 * SKIA CANVAS STRUCTURE
 * ============================================================================ */

typedef struct VaxpSkiaCanvas {
    VaxpCanvas base;
    
    SkCanvas* sk_canvas;      /* Skia canvas (borrowed from surface) */
    sk_sp<SkSurface> sk_surface;  /* Owned surface if offscreen */
    
} VaxpSkiaCanvas;

/* ============================================================================
 * SKIA PATH STRUCTURE
 * ============================================================================ */

struct VaxpPath {
    VAXP_REF_HEADER;
    SkPath sk_path;
};

/* ============================================================================
 * HELPER: Convert types
 * ============================================================================ */

static SkColor vaxp_color_to_sk(VaxpColor c) {
    return SkColorSetARGB(c.a, c.r, c.g, c.b);
}

static SkRect vaxp_rectf_to_sk(VaxpRectF r) {
    return SkRect::MakeXYWH(r.x, r.y, r.width, r.height);
}

static SkPaint vaxp_paint_to_sk(const VaxpPaint* paint) {
    SkPaint sk;
    sk.setColor(vaxp_color_to_sk(paint->color));
    sk.setAntiAlias(paint->anti_alias);
    sk.setStrokeWidth(paint->stroke_width);
    
    switch (paint->style) {
        case VAXP_PAINT_FILL:
            sk.setStyle(SkPaint::kFill_Style);
            break;
        case VAXP_PAINT_STROKE:
            sk.setStyle(SkPaint::kStroke_Style);
            break;
        case VAXP_PAINT_FILL_AND_STROKE:
            sk.setStyle(SkPaint::kStrokeAndFill_Style);
            break;
    }
    
    switch (paint->stroke_cap) {
        case VAXP_STROKE_CAP_BUTT: sk.setStrokeCap(SkPaint::kButt_Cap); break;
        case VAXP_STROKE_CAP_ROUND: sk.setStrokeCap(SkPaint::kRound_Cap); break;
        case VAXP_STROKE_CAP_SQUARE: sk.setStrokeCap(SkPaint::kSquare_Cap); break;
    }
    
    switch (paint->stroke_join) {
        case VAXP_STROKE_JOIN_MITER: sk.setStrokeJoin(SkPaint::kMiter_Join); break;
        case VAXP_STROKE_JOIN_ROUND: sk.setStrokeJoin(SkPaint::kRound_Join); break;
        case VAXP_STROKE_JOIN_BEVEL: sk.setStrokeJoin(SkPaint::kBevel_Join); break;
    }
    
    sk.setStrokeMiter(paint->stroke_miter);
    
    switch (paint->blend_mode) {
        case VAXP_BLEND_SRC_OVER: sk.setBlendMode(SkBlendMode::kSrcOver); break;
        case VAXP_BLEND_SRC: sk.setBlendMode(SkBlendMode::kSrc); break;
        case VAXP_BLEND_DST_OVER: sk.setBlendMode(SkBlendMode::kDstOver); break;
        case VAXP_BLEND_CLEAR: sk.setBlendMode(SkBlendMode::kClear); break;
        case VAXP_BLEND_MULTIPLY: sk.setBlendMode(SkBlendMode::kMultiply); break;
        case VAXP_BLEND_SCREEN: sk.setBlendMode(SkBlendMode::kScreen); break;
        case VAXP_BLEND_OVERLAY: sk.setBlendMode(SkBlendMode::kOverlay); break;
    }
    
    return sk;
}

/* ============================================================================
 * CANVAS OPERATIONS
 * ============================================================================ */

static void skia_canvas_destroy(VaxpCanvas* canvas) {
    VaxpSkiaCanvas* c = (VaxpSkiaCanvas*)canvas;
    c->sk_surface.reset();
    c->sk_canvas = nullptr;
}

static void skia_canvas_save(VaxpCanvas* canvas) {
    VaxpSkiaCanvas* c = (VaxpSkiaCanvas*)canvas;
    c->sk_canvas->save();
}

static void skia_canvas_restore(VaxpCanvas* canvas) {
    VaxpSkiaCanvas* c = (VaxpSkiaCanvas*)canvas;
    c->sk_canvas->restore();
}

static void skia_canvas_translate(VaxpCanvas* canvas, VaxpF32 dx, VaxpF32 dy) {
    VaxpSkiaCanvas* c = (VaxpSkiaCanvas*)canvas;
    c->sk_canvas->translate(dx, dy);
}

static void skia_canvas_scale(VaxpCanvas* canvas, VaxpF32 sx, VaxpF32 sy) {
    VaxpSkiaCanvas* c = (VaxpSkiaCanvas*)canvas;
    c->sk_canvas->scale(sx, sy);
}

static void skia_canvas_rotate(VaxpCanvas* canvas, VaxpF32 degrees) {
    VaxpSkiaCanvas* c = (VaxpSkiaCanvas*)canvas;
    c->sk_canvas->rotate(degrees);
}

static void skia_canvas_clip_rect(VaxpCanvas* canvas, VaxpRectF rect) {
    VaxpSkiaCanvas* c = (VaxpSkiaCanvas*)canvas;
    c->sk_canvas->clipRect(vaxp_rectf_to_sk(rect));
}

static void skia_canvas_clip_rounded_rect(VaxpCanvas* canvas, VaxpRectF rect, VaxpF32 radius) {
    VaxpSkiaCanvas* c = (VaxpSkiaCanvas*)canvas;
    SkRRect rrect = SkRRect::MakeRectXY(vaxp_rectf_to_sk(rect), radius, radius);
    c->sk_canvas->clipRRect(rrect);
}

static void skia_canvas_clear(VaxpCanvas* canvas, VaxpColor color) {
    VaxpSkiaCanvas* c = (VaxpSkiaCanvas*)canvas;
    c->sk_canvas->clear(vaxp_color_to_sk(color));
}

static void skia_canvas_draw_rect(VaxpCanvas* canvas, VaxpRectF rect, const VaxpPaint* paint) {
    VaxpSkiaCanvas* c = (VaxpSkiaCanvas*)canvas;
    SkPaint sk = vaxp_paint_to_sk(paint);
    c->sk_canvas->drawRect(vaxp_rectf_to_sk(rect), sk);
}

static void skia_canvas_draw_rounded_rect(VaxpCanvas* canvas, VaxpRectF rect, 
                                           VaxpF32 radius, const VaxpPaint* paint) {
    VaxpSkiaCanvas* c = (VaxpSkiaCanvas*)canvas;
    SkPaint sk = vaxp_paint_to_sk(paint);
    SkRRect rrect = SkRRect::MakeRectXY(vaxp_rectf_to_sk(rect), radius, radius);
    c->sk_canvas->drawRRect(rrect, sk);
}

static void skia_canvas_draw_circle(VaxpCanvas* canvas, VaxpF32 cx, VaxpF32 cy, 
                                     VaxpF32 radius, const VaxpPaint* paint) {
    VaxpSkiaCanvas* c = (VaxpSkiaCanvas*)canvas;
    SkPaint sk = vaxp_paint_to_sk(paint);
    c->sk_canvas->drawCircle(cx, cy, radius, sk);
}

static void skia_canvas_draw_oval(VaxpCanvas* canvas, VaxpRectF rect, const VaxpPaint* paint) {
    VaxpSkiaCanvas* c = (VaxpSkiaCanvas*)canvas;
    SkPaint sk = vaxp_paint_to_sk(paint);
    c->sk_canvas->drawOval(vaxp_rectf_to_sk(rect), sk);
}

static void skia_canvas_draw_line(VaxpCanvas* canvas, VaxpF32 x1, VaxpF32 y1, 
                                   VaxpF32 x2, VaxpF32 y2, const VaxpPaint* paint) {
    VaxpSkiaCanvas* c = (VaxpSkiaCanvas*)canvas;
    SkPaint sk = vaxp_paint_to_sk(paint);
    c->sk_canvas->drawLine(x1, y1, x2, y2, sk);
}

static void skia_canvas_draw_path(VaxpCanvas* canvas, const VaxpPath* path, const VaxpPaint* paint) {
    VaxpSkiaCanvas* c = (VaxpSkiaCanvas*)canvas;
    if (!path) return;
    SkPaint sk = vaxp_paint_to_sk(paint);
    c->sk_canvas->drawPath(path->sk_path, sk);
}

static void skia_canvas_draw_text(VaxpCanvas* canvas, const char* text, VaxpF32 x, VaxpF32 y,
                                   const VaxpFont* font, const VaxpPaint* paint) {
    VaxpSkiaCanvas* c = (VaxpSkiaCanvas*)canvas;
    SkPaint sk = vaxp_paint_to_sk(paint);
    
    /* Use default font for now */
    SkFont skfont;
    skfont.setSize(14.0f);  /* Default size */
    
    c->sk_canvas->drawString(text, x, y, skfont, sk);
}

static void skia_canvas_draw_image(VaxpCanvas* canvas, const VaxpImage* image, VaxpF32 x, VaxpF32 y) {
    /* TODO: Implement image drawing */
    (void)canvas;
    (void)image;
    (void)x;
    (void)y;
}

static void skia_canvas_draw_image_rect(VaxpCanvas* canvas, const VaxpImage* image,
                                         VaxpRectF src, VaxpRectF dst, const VaxpPaint* paint) {
    /* TODO: Implement image rect drawing */
    (void)canvas;
    (void)image;
    (void)src;
    (void)dst;
    (void)paint;
}

static void skia_canvas_flush(VaxpCanvas* canvas) {
    VaxpSkiaCanvas* c = (VaxpSkiaCanvas*)canvas;
    if (c->sk_surface) {
        c->sk_surface->flushAndSubmit();
    }
}

static VaxpSize2D skia_canvas_get_size(VaxpCanvas* canvas) {
    return (VaxpSize2D){ .width = canvas->width, .height = canvas->height };
}

/* ============================================================================
 * CANVAS VTABLE
 * ============================================================================ */

static const VaxpCanvasOps skia_canvas_ops = {
    .destroy = skia_canvas_destroy,
    .save = skia_canvas_save,
    .restore = skia_canvas_restore,
    .translate = skia_canvas_translate,
    .scale = skia_canvas_scale,
    .rotate = skia_canvas_rotate,
    .clip_rect = skia_canvas_clip_rect,
    .clip_rounded_rect = skia_canvas_clip_rounded_rect,
    .clear = skia_canvas_clear,
    .draw_rect = skia_canvas_draw_rect,
    .draw_rounded_rect = skia_canvas_draw_rounded_rect,
    .draw_circle = skia_canvas_draw_circle,
    .draw_oval = skia_canvas_draw_oval,
    .draw_line = skia_canvas_draw_line,
    .draw_path = skia_canvas_draw_path,
    .draw_text = skia_canvas_draw_text,
    .draw_image = skia_canvas_draw_image,
    .draw_image_rect = skia_canvas_draw_image_rect,
    .flush = skia_canvas_flush,
    .get_size = skia_canvas_get_size,
};

/* ============================================================================
 * CANVAS CREATION (for offscreen)
 * ============================================================================ */

VaxpResultPtr vaxp_canvas_create_offscreen(VaxpU32 width, VaxpU32 height) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
    sk_sp<SkSurface> surface = SkSurfaces::Raster(info);
    
    if (!surface) {
        return VAXP_ERR_PTR(VAXP_ERROR_CANVAS_CREATE);
    }
    
    VaxpSkiaCanvas* canvas = (VaxpSkiaCanvas*)VAXP_REF_NEW(VaxpSkiaCanvas, 
                                                               (VaxpDestructor)skia_canvas_destroy);
    if (!canvas) {
        return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
    }
    
    canvas->base.ops = &skia_canvas_ops;
    canvas->base.width = width;
    canvas->base.height = height;
    canvas->sk_surface = surface;
    canvas->sk_canvas = surface->getCanvas();
    
    return VAXP_OK_PTR(canvas);
}

/* ============================================================================
 * PATH IMPLEMENTATION
 * ============================================================================ */

static void path_destructor(void* self) {
    VaxpPath* path = (VaxpPath*)self;
    path->sk_path.reset();
}

VaxpResultPtr vaxp_path_create(void) {
    VaxpPath* path = (VaxpPath*)VAXP_REF_NEW(VaxpPath, path_destructor);
    if (!path) {
        return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
    }
    new (&path->sk_path) SkPath();  /* Placement new for C++ object */
    return VAXP_OK_PTR(path);
}

void vaxp_path_move_to(VaxpPath* path, VaxpF32 x, VaxpF32 y) {
    if (path) path->sk_path.moveTo(x, y);
}

void vaxp_path_line_to(VaxpPath* path, VaxpF32 x, VaxpF32 y) {
    if (path) path->sk_path.lineTo(x, y);
}

void vaxp_path_quad_to(VaxpPath* path, VaxpF32 cx, VaxpF32 cy, VaxpF32 x, VaxpF32 y) {
    if (path) path->sk_path.quadTo(cx, cy, x, y);
}

void vaxp_path_cubic_to(VaxpPath* path, VaxpF32 c1x, VaxpF32 c1y, 
                          VaxpF32 c2x, VaxpF32 c2y, VaxpF32 x, VaxpF32 y) {
    if (path) path->sk_path.cubicTo(c1x, c1y, c2x, c2y, x, y);
}

void vaxp_path_close(VaxpPath* path) {
    if (path) path->sk_path.close();
}

void vaxp_path_add_rect(VaxpPath* path, VaxpRectF rect) {
    if (path) path->sk_path.addRect(vaxp_rectf_to_sk(rect));
}

void vaxp_path_add_rounded_rect(VaxpPath* path, VaxpRectF rect, VaxpF32 radius) {
    if (path) {
        SkRRect rrect = SkRRect::MakeRectXY(vaxp_rectf_to_sk(rect), radius, radius);
        path->sk_path.addRRect(rrect);
    }
}

void vaxp_path_add_circle(VaxpPath* path, VaxpF32 cx, VaxpF32 cy, VaxpF32 radius) {
    if (path) path->sk_path.addCircle(cx, cy, radius);
}

void vaxp_path_reset(VaxpPath* path) {
    if (path) path->sk_path.reset();
}

} /* extern "C" */
