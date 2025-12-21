/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_canvas.h - Abstract canvas interface for 2D drawing
 * 
 * This provides a high-level API that wraps Skia for rendering.
 */

#ifndef VENOM_CANVAS_H
#define VENOM_CANVAS_H

#include "venom/core/venom_types.h"
#include "venom/core/venom_result.h"
#include "venom/core/venom_ref.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
typedef struct VenomCanvas VenomCanvas;
typedef struct VenomPaint VenomPaint;
typedef struct VenomPath VenomPath;
typedef struct VenomImage VenomImage;
typedef struct VenomFont VenomFont;

/* ============================================================================
 * PAINT - Describes how to draw
 * ============================================================================ */

typedef enum VenomPaintStyle {
    VENOM_PAINT_FILL,
    VENOM_PAINT_STROKE,
    VENOM_PAINT_FILL_AND_STROKE,
} VenomPaintStyle;

typedef enum VenomStrokeCap {
    VENOM_STROKE_CAP_BUTT,
    VENOM_STROKE_CAP_ROUND,
    VENOM_STROKE_CAP_SQUARE,
} VenomStrokeCap;

typedef enum VenomStrokeJoin {
    VENOM_STROKE_JOIN_MITER,
    VENOM_STROKE_JOIN_ROUND,
    VENOM_STROKE_JOIN_BEVEL,
} VenomStrokeJoin;

typedef enum VenomBlendMode {
    VENOM_BLEND_SRC_OVER,  /* Default - draw over existing */
    VENOM_BLEND_SRC,       /* Replace */
    VENOM_BLEND_DST_OVER,
    VENOM_BLEND_CLEAR,
    VENOM_BLEND_MULTIPLY,
    VENOM_BLEND_SCREEN,
    VENOM_BLEND_OVERLAY,
} VenomBlendMode;

struct VenomPaint {
    VenomColor color;
    VenomPaintStyle style;
    VenomF32 stroke_width;
    VenomStrokeCap stroke_cap;
    VenomStrokeJoin stroke_join;
    VenomF32 stroke_miter;
    VenomBlendMode blend_mode;
    VenomBool anti_alias;
};

/**
 * @brief Create default paint (black fill, anti-aliased)
 */
VENOM_INLINE VenomPaint venom_paint_default(void) {
    return (VenomPaint){
        .color = VENOM_COLOR_BLACK,
        .style = VENOM_PAINT_FILL,
        .stroke_width = 1.0f,
        .stroke_cap = VENOM_STROKE_CAP_BUTT,
        .stroke_join = VENOM_STROKE_JOIN_MITER,
        .stroke_miter = 4.0f,
        .blend_mode = VENOM_BLEND_SRC_OVER,
        .anti_alias = VENOM_TRUE,
    };
}

/**
 * @brief Create a fill paint with specified color
 */
VENOM_INLINE VenomPaint venom_paint_fill(VenomColor color) {
    VenomPaint paint = venom_paint_default();
    paint.color = color;
    return paint;
}

/**
 * @brief Create a stroke paint with specified color and width
 */
VENOM_INLINE VenomPaint venom_paint_stroke(VenomColor color, VenomF32 width) {
    VenomPaint paint = venom_paint_default();
    paint.color = color;
    paint.style = VENOM_PAINT_STROKE;
    paint.stroke_width = width;
    return paint;
}

/* ============================================================================
 * CANVAS OPERATIONS VTABLE
 * ============================================================================ */

typedef struct VenomCanvasOps {
    void (*destroy)(VenomCanvas* canvas);
    
    /* Canvas state */
    void (*save)(VenomCanvas* canvas);
    void (*restore)(VenomCanvas* canvas);
    
    /* Transformations */
    void (*translate)(VenomCanvas* canvas, VenomF32 dx, VenomF32 dy);
    void (*scale)(VenomCanvas* canvas, VenomF32 sx, VenomF32 sy);
    void (*rotate)(VenomCanvas* canvas, VenomF32 degrees);
    
    /* Clipping */
    void (*clip_rect)(VenomCanvas* canvas, VenomRectF rect);
    void (*clip_rounded_rect)(VenomCanvas* canvas, VenomRectF rect, VenomF32 radius);
    
    /* Drawing primitives */
    void (*clear)(VenomCanvas* canvas, VenomColor color);
    void (*draw_rect)(VenomCanvas* canvas, VenomRectF rect, const VenomPaint* paint);
    void (*draw_rounded_rect)(VenomCanvas* canvas, VenomRectF rect, VenomF32 radius, const VenomPaint* paint);
    void (*draw_circle)(VenomCanvas* canvas, VenomF32 cx, VenomF32 cy, VenomF32 radius, const VenomPaint* paint);
    void (*draw_oval)(VenomCanvas* canvas, VenomRectF rect, const VenomPaint* paint);
    void (*draw_line)(VenomCanvas* canvas, VenomF32 x1, VenomF32 y1, VenomF32 x2, VenomF32 y2, const VenomPaint* paint);
    void (*draw_path)(VenomCanvas* canvas, const VenomPath* path, const VenomPaint* paint);
    
    /* Text */
    void (*draw_text)(VenomCanvas* canvas, const char* text, VenomF32 x, VenomF32 y, 
                      const VenomFont* font, const VenomPaint* paint);
    
    /* Images */
    void (*draw_image)(VenomCanvas* canvas, const VenomImage* image, VenomF32 x, VenomF32 y);
    void (*draw_image_rect)(VenomCanvas* canvas, const VenomImage* image, 
                            VenomRectF src, VenomRectF dst, const VenomPaint* paint);
    
    /* Surface operations */
    void (*flush)(VenomCanvas* canvas);
    VenomSize2D (*get_size)(VenomCanvas* canvas);
} VenomCanvasOps;

/* ============================================================================
 * CANVAS STRUCTURE
 * ============================================================================ */

struct VenomCanvas {
    VENOM_REF_HEADER;
    const VenomCanvasOps* ops;
    VenomU32 width;
    VenomU32 height;
};

/* ============================================================================
 * CANVAS PUBLIC API
 * ============================================================================ */

/* State management */
VENOM_INLINE void venom_canvas_save(VenomCanvas* canvas) {
    canvas->ops->save(canvas);
}

VENOM_INLINE void venom_canvas_restore(VenomCanvas* canvas) {
    canvas->ops->restore(canvas);
}

/* Transformations */
VENOM_INLINE void venom_canvas_translate(VenomCanvas* canvas, VenomF32 dx, VenomF32 dy) {
    canvas->ops->translate(canvas, dx, dy);
}

VENOM_INLINE void venom_canvas_scale(VenomCanvas* canvas, VenomF32 sx, VenomF32 sy) {
    canvas->ops->scale(canvas, sx, sy);
}

VENOM_INLINE void venom_canvas_rotate(VenomCanvas* canvas, VenomF32 degrees) {
    canvas->ops->rotate(canvas, degrees);
}

/* Clipping */
VENOM_INLINE void venom_canvas_clip_rect(VenomCanvas* canvas, VenomRectF rect) {
    canvas->ops->clip_rect(canvas, rect);
}

VENOM_INLINE void venom_canvas_clip_rounded_rect(VenomCanvas* canvas, VenomRectF rect, VenomF32 radius) {
    canvas->ops->clip_rounded_rect(canvas, rect, radius);
}

/* Drawing */
VENOM_INLINE void venom_canvas_clear(VenomCanvas* canvas, VenomColor color) {
    canvas->ops->clear(canvas, color);
}

VENOM_INLINE void venom_canvas_draw_rect(VenomCanvas* canvas, VenomRectF rect, const VenomPaint* paint) {
    canvas->ops->draw_rect(canvas, rect, paint);
}

VENOM_INLINE void venom_canvas_draw_rounded_rect(VenomCanvas* canvas, VenomRectF rect, 
                                                   VenomF32 radius, const VenomPaint* paint) {
    canvas->ops->draw_rounded_rect(canvas, rect, radius, paint);
}

VENOM_INLINE void venom_canvas_draw_circle(VenomCanvas* canvas, VenomF32 cx, VenomF32 cy, 
                                            VenomF32 radius, const VenomPaint* paint) {
    canvas->ops->draw_circle(canvas, cx, cy, radius, paint);
}

VENOM_INLINE void venom_canvas_draw_oval(VenomCanvas* canvas, VenomRectF rect, const VenomPaint* paint) {
    canvas->ops->draw_oval(canvas, rect, paint);
}

VENOM_INLINE void venom_canvas_draw_line(VenomCanvas* canvas, VenomF32 x1, VenomF32 y1, 
                                          VenomF32 x2, VenomF32 y2, const VenomPaint* paint) {
    canvas->ops->draw_line(canvas, x1, y1, x2, y2, paint);
}

VENOM_INLINE void venom_canvas_draw_text(VenomCanvas* canvas, const char* text, 
                                          VenomF32 x, VenomF32 y, 
                                          const VenomFont* font, const VenomPaint* paint) {
    canvas->ops->draw_text(canvas, text, x, y, font, paint);
}

VENOM_INLINE void venom_canvas_draw_image(VenomCanvas* canvas, const VenomImage* image, 
                                           VenomF32 x, VenomF32 y) {
    canvas->ops->draw_image(canvas, image, x, y);
}

VENOM_INLINE void venom_canvas_flush(VenomCanvas* canvas) {
    canvas->ops->flush(canvas);
}

VENOM_INLINE VenomSize2D venom_canvas_get_size(VenomCanvas* canvas) {
    return canvas->ops->get_size(canvas);
}

/* ============================================================================
 * PATH BUILDER
 * ============================================================================ */

/**
 * @brief Create a new path builder
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_path_create(void);

/**
 * @brief Move to a point (start new contour)
 */
void venom_path_move_to(VenomPath* path, VenomF32 x, VenomF32 y);

/**
 * @brief Draw line to a point
 */
void venom_path_line_to(VenomPath* path, VenomF32 x, VenomF32 y);

/**
 * @brief Draw quadratic bezier curve
 */
void venom_path_quad_to(VenomPath* path, VenomF32 cx, VenomF32 cy, VenomF32 x, VenomF32 y);

/**
 * @brief Draw cubic bezier curve
 */
void venom_path_cubic_to(VenomPath* path, VenomF32 c1x, VenomF32 c1y, 
                          VenomF32 c2x, VenomF32 c2y, VenomF32 x, VenomF32 y);

/**
 * @brief Close current contour
 */
void venom_path_close(VenomPath* path);

/**
 * @brief Add rectangle to path
 */
void venom_path_add_rect(VenomPath* path, VenomRectF rect);

/**
 * @brief Add rounded rectangle to path
 */
void venom_path_add_rounded_rect(VenomPath* path, VenomRectF rect, VenomF32 radius);

/**
 * @brief Add circle to path
 */
void venom_path_add_circle(VenomPath* path, VenomF32 cx, VenomF32 cy, VenomF32 radius);

/**
 * @brief Reset path to empty
 */
void venom_path_reset(VenomPath* path);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_CANVAS_H */
