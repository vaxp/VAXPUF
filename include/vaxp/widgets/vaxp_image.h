/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_image.h - Image widget for displaying PNG, JPEG, etc.
 * 
 * Features:
 * - Load from file or memory
 * - Automatic format detection
 * - Scaling modes (fill, contain, cover, stretch)
 * - Optional rounded corners
 */

#ifndef VAXP_IMAGE_WIDGET_H
#define VAXP_IMAGE_WIDGET_H

#include "vaxp/widgets/vaxp_widget.h"
#include "vaxp/graphics/vaxp_canvas.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * SCALING MODES
 * ============================================================================ */

typedef enum VaxpImageFit {
    VAXP_IMAGE_FIT_CONTAIN,    /* Scale to fit, maintain aspect ratio, may have letterbox */
    VAXP_IMAGE_FIT_COVER,      /* Scale to cover, maintain aspect ratio, may crop */
    VAXP_IMAGE_FIT_FILL,       /* Scale to fill exactly, may distort */
    VAXP_IMAGE_FIT_NONE,       /* Original size, no scaling */
    VAXP_IMAGE_FIT_SCALE_DOWN, /* Like contain, but never scale up */
} VaxpImageFit;

/* ============================================================================
 * IMAGE DATA
 * ============================================================================ */

typedef struct VaxpImageData {
    VAXP_REF_HEADER;
    
    VaxpU8* pixels;            /* RGBA pixel data */
    VaxpU32 width;             /* Image width */
    VaxpU32 height;            /* Image height */
    VaxpU32 stride;            /* Bytes per row */
    
    /* Cairo surface for efficient rendering */
    void* cairo_surface;        /* cairo_surface_t* */
    
    /* OpenGL texture handle */
    VaxpU32 gl_texture;         
    
} VaxpImageData;

/* ============================================================================
 * IMAGE WIDGET
 * ============================================================================ */

typedef struct VaxpImageWidget {
    VaxpWidget base;
    
    VaxpImageData* image;      /* Image data (ref-counted) */
    VaxpImageFit fit;          /* How to fit image in bounds */
    VaxpF32 corner_radius;     /* Optional rounded corners */
    VaxpColor background;      /* Background color for letterbox */
    VaxpF32 opacity;           /* Image opacity (0.0 - 1.0) */
    
} VaxpImageWidget;

/* ============================================================================
 * IMAGE LOADING API
 * ============================================================================ */

/**
 * @brief Load image from file (PNG, JPEG, etc.)
 */
VaxpResultPtr vaxp_image_load_file(const char* path);

/**
 * @brief Load image from memory
 */
VaxpResultPtr vaxp_image_load_memory(const VaxpU8* data, VaxpSize size);

/**
 * @brief Create empty image with size
 */
VaxpResultPtr vaxp_image_create(VaxpU32 width, VaxpU32 height);

/**
 * @brief Get image dimensions
 */
void vaxp_image_get_size(const VaxpImageData* image, VaxpU32* width, VaxpU32* height);

/* ============================================================================
 * IMAGE WIDGET API
 * ============================================================================ */

/**
 * @brief Create image widget
 */
VaxpResultPtr vaxp_image_widget_create(void);

/**
 * @brief Set image data
 */
VaxpResult vaxp_image_widget_set_image(VaxpImageWidget* widget, VaxpImageData* image);

/**
 * @brief Load and set image from file
 */
VaxpResult vaxp_image_widget_load(VaxpImageWidget* widget, const char* path);

/**
 * @brief Set fit mode
 */
void vaxp_image_widget_set_fit(VaxpImageWidget* widget, VaxpImageFit fit);

/**
 * @brief Set corner radius
 */
void vaxp_image_widget_set_corner_radius(VaxpImageWidget* widget, VaxpF32 radius);

/**
 * @brief Set opacity
 */
void vaxp_image_widget_set_opacity(VaxpImageWidget* widget, VaxpF32 opacity);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VaxpWidgetClass vaxp_image_widget_class;

/* ============================================================================
 * CONVENIENCE MACRO
 * ============================================================================ */

#define vaxp_image(...) \
    _vaxp_image_widget_build(&(VaxpImageWidgetConfig){ __VA_ARGS__ })

typedef struct VaxpImageWidgetConfig {
    const char* src;            /* File path */
    VaxpImageData* data;       /* Direct image data */
    VaxpImageFit fit;
    VaxpF32 corner_radius;
    VaxpF32 opacity;
    VaxpF32 width;
    VaxpF32 height;
} VaxpImageWidgetConfig;

VaxpWidget* _vaxp_image_widget_build(const VaxpImageWidgetConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_IMAGE_WIDGET_H */
