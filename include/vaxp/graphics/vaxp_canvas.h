/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_canvas.h - Abstract canvas interface for 2D drawing
 * 
 * This provides a high-level API that wraps Skia for rendering.
 */

#ifndef VAXP_CANVAS_H
#define VAXP_CANVAS_H

#include "vaxp/core/vaxp_types.h"
#include "vaxp/core/vaxp_result.h"
#include "vaxp/core/vaxp_ref.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
typedef struct VaxpCanvas VaxpCanvas;
typedef struct VaxpPaint VaxpPaint;
typedef struct VaxpPath VaxpPath;
typedef struct VaxpImage VaxpImage;
typedef struct VaxpFont {
    const char* family;
    VaxpF32 size;
    VaxpBool bold;
    VaxpBool italic;
} VaxpFont;

/* ============================================================================
 * PAINT - Describes how to draw
 * ============================================================================ */

typedef enum VaxpPaintStyle {
    VAXP_PAINT_FILL,
    VAXP_PAINT_STROKE,
    VAXP_PAINT_FILL_AND_STROKE,
} VaxpPaintStyle;

typedef enum VaxpStrokeCap {
    VAXP_STROKE_CAP_BUTT,
    VAXP_STROKE_CAP_ROUND,
    VAXP_STROKE_CAP_SQUARE,
} VaxpStrokeCap;

typedef enum VaxpStrokeJoin {
    VAXP_STROKE_JOIN_MITER,
    VAXP_STROKE_JOIN_ROUND,
    VAXP_STROKE_JOIN_BEVEL,
} VaxpStrokeJoin;

typedef enum VaxpBlendMode {
    VAXP_BLEND_SRC_OVER,  /* Default - draw over existing */
    VAXP_BLEND_SRC,       /* Replace */
    VAXP_BLEND_DST_OVER,
    VAXP_BLEND_CLEAR,
    VAXP_BLEND_MULTIPLY,
    VAXP_BLEND_SCREEN,
    VAXP_BLEND_OVERLAY,
} VaxpBlendMode;

struct VaxpPaint {
    VaxpColor color;
    VaxpPaintStyle style;
    VaxpF32 stroke_width;
    VaxpStrokeCap stroke_cap;
    VaxpStrokeJoin stroke_join;
    VaxpF32 stroke_miter;
    VaxpBlendMode blend_mode;
    VaxpBool anti_alias;
};

/**
 * @brief Create default paint (black fill, anti-aliased)
 */
VAXP_INLINE VaxpPaint vaxp_paint_default(void) {
    return (VaxpPaint){
        .color = VAXP_COLOR_BLACK,
        .style = VAXP_PAINT_FILL,
        .stroke_width = 1.0f,
        .stroke_cap = VAXP_STROKE_CAP_BUTT,
        .stroke_join = VAXP_STROKE_JOIN_MITER,
        .stroke_miter = 4.0f,
        .blend_mode = VAXP_BLEND_SRC_OVER,
        .anti_alias = VAXP_TRUE,
    };
}

/**
 * @brief Create a fill paint with specified color
 */
VAXP_INLINE VaxpPaint vaxp_paint_fill(VaxpColor color) {
    VaxpPaint paint = vaxp_paint_default();
    paint.color = color;
    return paint;
}

/**
 * @brief Create a stroke paint with specified color and width
 */
VAXP_INLINE VaxpPaint vaxp_paint_stroke(VaxpColor color, VaxpF32 width) {
    VaxpPaint paint = vaxp_paint_default();
    paint.color = color;
    paint.style = VAXP_PAINT_STROKE;
    paint.stroke_width = width;
    return paint;
}

/* ============================================================================
 * CANVAS OPERATIONS VTABLE
 * ============================================================================ */

typedef struct VaxpCanvasOps {
    void (*destroy)(VaxpCanvas* canvas);
    
    /* Canvas state */
    void (*save)(VaxpCanvas* canvas);
    void (*restore)(VaxpCanvas* canvas);
    
    /* Transformations */
    void (*translate)(VaxpCanvas* canvas, VaxpF32 dx, VaxpF32 dy);
    void (*scale)(VaxpCanvas* canvas, VaxpF32 sx, VaxpF32 sy);
    void (*rotate)(VaxpCanvas* canvas, VaxpF32 degrees);
    
    /* Clipping */
    void (*clip_rect)(VaxpCanvas* canvas, VaxpRectF rect);
    void (*clip_rounded_rect)(VaxpCanvas* canvas, VaxpRectF rect, VaxpF32 radius);
    
    /* Drawing primitives */
    void (*clear)(VaxpCanvas* canvas, VaxpColor color);
    void (*draw_rect)(VaxpCanvas* canvas, VaxpRectF rect, const VaxpPaint* paint);
    void (*draw_rounded_rect)(VaxpCanvas* canvas, VaxpRectF rect, VaxpF32 radius, const VaxpPaint* paint);
    void (*draw_circle)(VaxpCanvas* canvas, VaxpF32 cx, VaxpF32 cy, VaxpF32 radius, const VaxpPaint* paint);
    void (*draw_oval)(VaxpCanvas* canvas, VaxpRectF rect, const VaxpPaint* paint);
    void (*draw_line)(VaxpCanvas* canvas, VaxpF32 x1, VaxpF32 y1, VaxpF32 x2, VaxpF32 y2, const VaxpPaint* paint);
    void (*draw_path)(VaxpCanvas* canvas, const VaxpPath* path, const VaxpPaint* paint);
    
    /* Text */
    void (*draw_text)(VaxpCanvas* canvas, const char* text, VaxpF32 x, VaxpF32 y, 
                      const VaxpFont* font, const VaxpPaint* paint);
    
    /* Images */
    void (*draw_image)(VaxpCanvas* canvas, const VaxpImage* image, VaxpF32 x, VaxpF32 y);
    void (*draw_image_rect)(VaxpCanvas* canvas, const VaxpImage* image, 
                            VaxpRectF src, VaxpRectF dst, const VaxpPaint* paint);
    
    /* Surface operations */
    void (*flush)(VaxpCanvas* canvas);
    VaxpSize2D (*get_size)(VaxpCanvas* canvas);
} VaxpCanvasOps;

/* ============================================================================
 * CANVAS STRUCTURE
 * ============================================================================ */

struct VaxpCanvas {
    VAXP_REF_HEADER;
    const VaxpCanvasOps* ops;
    VaxpU32 width;
    VaxpU32 height;
};

/* ============================================================================
 * CANVAS PUBLIC API
 * ============================================================================ */

/* State management */
VAXP_INLINE void vaxp_canvas_save(VaxpCanvas* canvas) {
    canvas->ops->save(canvas);
}

VAXP_INLINE void vaxp_canvas_restore(VaxpCanvas* canvas) {
    canvas->ops->restore(canvas);
}

/* Transformations */
VAXP_INLINE void vaxp_canvas_translate(VaxpCanvas* canvas, VaxpF32 dx, VaxpF32 dy) {
    canvas->ops->translate(canvas, dx, dy);
}

VAXP_INLINE void vaxp_canvas_scale(VaxpCanvas* canvas, VaxpF32 sx, VaxpF32 sy) {
    canvas->ops->scale(canvas, sx, sy);
}

VAXP_INLINE void vaxp_canvas_rotate(VaxpCanvas* canvas, VaxpF32 degrees) {
    canvas->ops->rotate(canvas, degrees);
}

/* Clipping */
VAXP_INLINE void vaxp_canvas_clip_rect(VaxpCanvas* canvas, VaxpRectF rect) {
    canvas->ops->clip_rect(canvas, rect);
}

VAXP_INLINE void vaxp_canvas_clip_rounded_rect(VaxpCanvas* canvas, VaxpRectF rect, VaxpF32 radius) {
    canvas->ops->clip_rounded_rect(canvas, rect, radius);
}

/* Drawing */
VAXP_INLINE void vaxp_canvas_clear(VaxpCanvas* canvas, VaxpColor color) {
    canvas->ops->clear(canvas, color);
}

VAXP_INLINE void vaxp_canvas_draw_rect(VaxpCanvas* canvas, VaxpRectF rect, const VaxpPaint* paint) {
    canvas->ops->draw_rect(canvas, rect, paint);
}

VAXP_INLINE void vaxp_canvas_draw_rounded_rect(VaxpCanvas* canvas, VaxpRectF rect, 
                                                   VaxpF32 radius, const VaxpPaint* paint) {
    canvas->ops->draw_rounded_rect(canvas, rect, radius, paint);
}

VAXP_INLINE void vaxp_canvas_draw_circle(VaxpCanvas* canvas, VaxpF32 cx, VaxpF32 cy, 
                                            VaxpF32 radius, const VaxpPaint* paint) {
    canvas->ops->draw_circle(canvas, cx, cy, radius, paint);
}

VAXP_INLINE void vaxp_canvas_draw_oval(VaxpCanvas* canvas, VaxpRectF rect, const VaxpPaint* paint) {
    canvas->ops->draw_oval(canvas, rect, paint);
}

VAXP_INLINE void vaxp_canvas_draw_line(VaxpCanvas* canvas, VaxpF32 x1, VaxpF32 y1, 
                                          VaxpF32 x2, VaxpF32 y2, const VaxpPaint* paint) {
    canvas->ops->draw_line(canvas, x1, y1, x2, y2, paint);
}

VAXP_INLINE void vaxp_canvas_draw_text(VaxpCanvas* canvas, const char* text, 
                                          VaxpF32 x, VaxpF32 y, 
                                          const VaxpFont* font, const VaxpPaint* paint) {
    canvas->ops->draw_text(canvas, text, x, y, font, paint);
}

VAXP_INLINE void vaxp_canvas_draw_image(VaxpCanvas* canvas, const VaxpImage* image, 
                                           VaxpF32 x, VaxpF32 y) {
    canvas->ops->draw_image(canvas, image, x, y);
}

VAXP_INLINE void vaxp_canvas_flush(VaxpCanvas* canvas) {
    canvas->ops->flush(canvas);
}

VAXP_INLINE VaxpSize2D vaxp_canvas_get_size(VaxpCanvas* canvas) {
    return canvas->ops->get_size(canvas);
}

/* ============================================================================
 * PATH BUILDER
 * ============================================================================ */

/**
 * @brief Create a new path builder
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_path_create(void);

/**
 * @brief Move to a point (start new contour)
 */
void vaxp_path_move_to(VaxpPath* path, VaxpF32 x, VaxpF32 y);

/**
 * @brief Draw line to a point
 */
void vaxp_path_line_to(VaxpPath* path, VaxpF32 x, VaxpF32 y);

/**
 * @brief Draw quadratic bezier curve
 */
void vaxp_path_quad_to(VaxpPath* path, VaxpF32 cx, VaxpF32 cy, VaxpF32 x, VaxpF32 y);

/**
 * @brief Draw cubic bezier curve
 */
void vaxp_path_cubic_to(VaxpPath* path, VaxpF32 c1x, VaxpF32 c1y, 
                          VaxpF32 c2x, VaxpF32 c2y, VaxpF32 x, VaxpF32 y);

/**
 * @brief Close current contour
 */
void vaxp_path_close(VaxpPath* path);

/**
 * @brief Add rectangle to path
 */
void vaxp_path_add_rect(VaxpPath* path, VaxpRectF rect);

/**
 * @brief Add rounded rectangle to path
 */
void vaxp_path_add_rounded_rect(VaxpPath* path, VaxpRectF rect, VaxpF32 radius);

/**
 * @brief Add circle to path
 */
void vaxp_path_add_circle(VaxpPath* path, VaxpF32 cx, VaxpF32 cy, VaxpF32 radius);

/**
 * @brief Reset path to empty
 */
void vaxp_path_reset(VaxpPath* path);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_CANVAS_H */
