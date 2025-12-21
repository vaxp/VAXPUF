/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_canvas_skia.cpp - Skia implementation of canvas
 * 
 * This is C++ because Skia is a C++ library.
 * We expose only C interfaces to the rest of the framework.
 */

#include "venom/graphics/venom_canvas.h"
#include "venom/backend/venom_surface.h"
#include "venom/core/venom_memory.h"

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

typedef struct VenomSkiaCanvas {
    VenomCanvas base;
    
    SkCanvas* sk_canvas;      /* Skia canvas (borrowed from surface) */
    sk_sp<SkSurface> sk_surface;  /* Owned surface if offscreen */
    
} VenomSkiaCanvas;

/* ============================================================================
 * SKIA PATH STRUCTURE
 * ============================================================================ */

struct VenomPath {
    VENOM_REF_HEADER;
    SkPath sk_path;
};

/* ============================================================================
 * HELPER: Convert types
 * ============================================================================ */

static SkColor venom_color_to_sk(VenomColor c) {
    return SkColorSetARGB(c.a, c.r, c.g, c.b);
}

static SkRect venom_rectf_to_sk(VenomRectF r) {
    return SkRect::MakeXYWH(r.x, r.y, r.width, r.height);
}

static SkPaint venom_paint_to_sk(const VenomPaint* paint) {
    SkPaint sk;
    sk.setColor(venom_color_to_sk(paint->color));
    sk.setAntiAlias(paint->anti_alias);
    sk.setStrokeWidth(paint->stroke_width);
    
    switch (paint->style) {
        case VENOM_PAINT_FILL:
            sk.setStyle(SkPaint::kFill_Style);
            break;
        case VENOM_PAINT_STROKE:
            sk.setStyle(SkPaint::kStroke_Style);
            break;
        case VENOM_PAINT_FILL_AND_STROKE:
            sk.setStyle(SkPaint::kStrokeAndFill_Style);
            break;
    }
    
    switch (paint->stroke_cap) {
        case VENOM_STROKE_CAP_BUTT: sk.setStrokeCap(SkPaint::kButt_Cap); break;
        case VENOM_STROKE_CAP_ROUND: sk.setStrokeCap(SkPaint::kRound_Cap); break;
        case VENOM_STROKE_CAP_SQUARE: sk.setStrokeCap(SkPaint::kSquare_Cap); break;
    }
    
    switch (paint->stroke_join) {
        case VENOM_STROKE_JOIN_MITER: sk.setStrokeJoin(SkPaint::kMiter_Join); break;
        case VENOM_STROKE_JOIN_ROUND: sk.setStrokeJoin(SkPaint::kRound_Join); break;
        case VENOM_STROKE_JOIN_BEVEL: sk.setStrokeJoin(SkPaint::kBevel_Join); break;
    }
    
    sk.setStrokeMiter(paint->stroke_miter);
    
    switch (paint->blend_mode) {
        case VENOM_BLEND_SRC_OVER: sk.setBlendMode(SkBlendMode::kSrcOver); break;
        case VENOM_BLEND_SRC: sk.setBlendMode(SkBlendMode::kSrc); break;
        case VENOM_BLEND_DST_OVER: sk.setBlendMode(SkBlendMode::kDstOver); break;
        case VENOM_BLEND_CLEAR: sk.setBlendMode(SkBlendMode::kClear); break;
        case VENOM_BLEND_MULTIPLY: sk.setBlendMode(SkBlendMode::kMultiply); break;
        case VENOM_BLEND_SCREEN: sk.setBlendMode(SkBlendMode::kScreen); break;
        case VENOM_BLEND_OVERLAY: sk.setBlendMode(SkBlendMode::kOverlay); break;
    }
    
    return sk;
}

/* ============================================================================
 * CANVAS OPERATIONS
 * ============================================================================ */

static void skia_canvas_destroy(VenomCanvas* canvas) {
    VenomSkiaCanvas* c = (VenomSkiaCanvas*)canvas;
    c->sk_surface.reset();
    c->sk_canvas = nullptr;
}

static void skia_canvas_save(VenomCanvas* canvas) {
    VenomSkiaCanvas* c = (VenomSkiaCanvas*)canvas;
    c->sk_canvas->save();
}

static void skia_canvas_restore(VenomCanvas* canvas) {
    VenomSkiaCanvas* c = (VenomSkiaCanvas*)canvas;
    c->sk_canvas->restore();
}

static void skia_canvas_translate(VenomCanvas* canvas, VenomF32 dx, VenomF32 dy) {
    VenomSkiaCanvas* c = (VenomSkiaCanvas*)canvas;
    c->sk_canvas->translate(dx, dy);
}

static void skia_canvas_scale(VenomCanvas* canvas, VenomF32 sx, VenomF32 sy) {
    VenomSkiaCanvas* c = (VenomSkiaCanvas*)canvas;
    c->sk_canvas->scale(sx, sy);
}

static void skia_canvas_rotate(VenomCanvas* canvas, VenomF32 degrees) {
    VenomSkiaCanvas* c = (VenomSkiaCanvas*)canvas;
    c->sk_canvas->rotate(degrees);
}

static void skia_canvas_clip_rect(VenomCanvas* canvas, VenomRectF rect) {
    VenomSkiaCanvas* c = (VenomSkiaCanvas*)canvas;
    c->sk_canvas->clipRect(venom_rectf_to_sk(rect));
}

static void skia_canvas_clip_rounded_rect(VenomCanvas* canvas, VenomRectF rect, VenomF32 radius) {
    VenomSkiaCanvas* c = (VenomSkiaCanvas*)canvas;
    SkRRect rrect = SkRRect::MakeRectXY(venom_rectf_to_sk(rect), radius, radius);
    c->sk_canvas->clipRRect(rrect);
}

static void skia_canvas_clear(VenomCanvas* canvas, VenomColor color) {
    VenomSkiaCanvas* c = (VenomSkiaCanvas*)canvas;
    c->sk_canvas->clear(venom_color_to_sk(color));
}

static void skia_canvas_draw_rect(VenomCanvas* canvas, VenomRectF rect, const VenomPaint* paint) {
    VenomSkiaCanvas* c = (VenomSkiaCanvas*)canvas;
    SkPaint sk = venom_paint_to_sk(paint);
    c->sk_canvas->drawRect(venom_rectf_to_sk(rect), sk);
}

static void skia_canvas_draw_rounded_rect(VenomCanvas* canvas, VenomRectF rect, 
                                           VenomF32 radius, const VenomPaint* paint) {
    VenomSkiaCanvas* c = (VenomSkiaCanvas*)canvas;
    SkPaint sk = venom_paint_to_sk(paint);
    SkRRect rrect = SkRRect::MakeRectXY(venom_rectf_to_sk(rect), radius, radius);
    c->sk_canvas->drawRRect(rrect, sk);
}

static void skia_canvas_draw_circle(VenomCanvas* canvas, VenomF32 cx, VenomF32 cy, 
                                     VenomF32 radius, const VenomPaint* paint) {
    VenomSkiaCanvas* c = (VenomSkiaCanvas*)canvas;
    SkPaint sk = venom_paint_to_sk(paint);
    c->sk_canvas->drawCircle(cx, cy, radius, sk);
}

static void skia_canvas_draw_oval(VenomCanvas* canvas, VenomRectF rect, const VenomPaint* paint) {
    VenomSkiaCanvas* c = (VenomSkiaCanvas*)canvas;
    SkPaint sk = venom_paint_to_sk(paint);
    c->sk_canvas->drawOval(venom_rectf_to_sk(rect), sk);
}

static void skia_canvas_draw_line(VenomCanvas* canvas, VenomF32 x1, VenomF32 y1, 
                                   VenomF32 x2, VenomF32 y2, const VenomPaint* paint) {
    VenomSkiaCanvas* c = (VenomSkiaCanvas*)canvas;
    SkPaint sk = venom_paint_to_sk(paint);
    c->sk_canvas->drawLine(x1, y1, x2, y2, sk);
}

static void skia_canvas_draw_path(VenomCanvas* canvas, const VenomPath* path, const VenomPaint* paint) {
    VenomSkiaCanvas* c = (VenomSkiaCanvas*)canvas;
    if (!path) return;
    SkPaint sk = venom_paint_to_sk(paint);
    c->sk_canvas->drawPath(path->sk_path, sk);
}

static void skia_canvas_draw_text(VenomCanvas* canvas, const char* text, VenomF32 x, VenomF32 y,
                                   const VenomFont* font, const VenomPaint* paint) {
    VenomSkiaCanvas* c = (VenomSkiaCanvas*)canvas;
    SkPaint sk = venom_paint_to_sk(paint);
    
    /* Use default font for now */
    SkFont skfont;
    skfont.setSize(14.0f);  /* Default size */
    
    c->sk_canvas->drawString(text, x, y, skfont, sk);
}

static void skia_canvas_draw_image(VenomCanvas* canvas, const VenomImage* image, VenomF32 x, VenomF32 y) {
    /* TODO: Implement image drawing */
    (void)canvas;
    (void)image;
    (void)x;
    (void)y;
}

static void skia_canvas_draw_image_rect(VenomCanvas* canvas, const VenomImage* image,
                                         VenomRectF src, VenomRectF dst, const VenomPaint* paint) {
    /* TODO: Implement image rect drawing */
    (void)canvas;
    (void)image;
    (void)src;
    (void)dst;
    (void)paint;
}

static void skia_canvas_flush(VenomCanvas* canvas) {
    VenomSkiaCanvas* c = (VenomSkiaCanvas*)canvas;
    if (c->sk_surface) {
        c->sk_surface->flushAndSubmit();
    }
}

static VenomSize2D skia_canvas_get_size(VenomCanvas* canvas) {
    return (VenomSize2D){ .width = canvas->width, .height = canvas->height };
}

/* ============================================================================
 * CANVAS VTABLE
 * ============================================================================ */

static const VenomCanvasOps skia_canvas_ops = {
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

VenomResultPtr venom_canvas_create_offscreen(VenomU32 width, VenomU32 height) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
    sk_sp<SkSurface> surface = SkSurfaces::Raster(info);
    
    if (!surface) {
        return VENOM_ERR_PTR(VENOM_ERROR_CANVAS_CREATE);
    }
    
    VenomSkiaCanvas* canvas = (VenomSkiaCanvas*)VENOM_REF_NEW(VenomSkiaCanvas, 
                                                               (VenomDestructor)skia_canvas_destroy);
    if (!canvas) {
        return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    canvas->base.ops = &skia_canvas_ops;
    canvas->base.width = width;
    canvas->base.height = height;
    canvas->sk_surface = surface;
    canvas->sk_canvas = surface->getCanvas();
    
    return VENOM_OK_PTR(canvas);
}

/* ============================================================================
 * PATH IMPLEMENTATION
 * ============================================================================ */

static void path_destructor(void* self) {
    VenomPath* path = (VenomPath*)self;
    path->sk_path.reset();
}

VenomResultPtr venom_path_create(void) {
    VenomPath* path = (VenomPath*)VENOM_REF_NEW(VenomPath, path_destructor);
    if (!path) {
        return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
    }
    new (&path->sk_path) SkPath();  /* Placement new for C++ object */
    return VENOM_OK_PTR(path);
}

void venom_path_move_to(VenomPath* path, VenomF32 x, VenomF32 y) {
    if (path) path->sk_path.moveTo(x, y);
}

void venom_path_line_to(VenomPath* path, VenomF32 x, VenomF32 y) {
    if (path) path->sk_path.lineTo(x, y);
}

void venom_path_quad_to(VenomPath* path, VenomF32 cx, VenomF32 cy, VenomF32 x, VenomF32 y) {
    if (path) path->sk_path.quadTo(cx, cy, x, y);
}

void venom_path_cubic_to(VenomPath* path, VenomF32 c1x, VenomF32 c1y, 
                          VenomF32 c2x, VenomF32 c2y, VenomF32 x, VenomF32 y) {
    if (path) path->sk_path.cubicTo(c1x, c1y, c2x, c2y, x, y);
}

void venom_path_close(VenomPath* path) {
    if (path) path->sk_path.close();
}

void venom_path_add_rect(VenomPath* path, VenomRectF rect) {
    if (path) path->sk_path.addRect(venom_rectf_to_sk(rect));
}

void venom_path_add_rounded_rect(VenomPath* path, VenomRectF rect, VenomF32 radius) {
    if (path) {
        SkRRect rrect = SkRRect::MakeRectXY(venom_rectf_to_sk(rect), radius, radius);
        path->sk_path.addRRect(rrect);
    }
}

void venom_path_add_circle(VenomPath* path, VenomF32 cx, VenomF32 cy, VenomF32 radius) {
    if (path) path->sk_path.addCircle(cx, cy, radius);
}

void venom_path_reset(VenomPath* path) {
    if (path) path->sk_path.reset();
}

} /* extern "C" */
