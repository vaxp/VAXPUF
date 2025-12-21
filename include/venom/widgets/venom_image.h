/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_image.h - Image widget for displaying PNG, JPEG, etc.
 * 
 * Features:
 * - Load from file or memory
 * - Automatic format detection
 * - Scaling modes (fill, contain, cover, stretch)
 * - Optional rounded corners
 */

#ifndef VENOM_IMAGE_WIDGET_H
#define VENOM_IMAGE_WIDGET_H

#include "venom/widgets/venom_widget.h"
#include "venom/graphics/venom_canvas.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * SCALING MODES
 * ============================================================================ */

typedef enum VenomImageFit {
    VENOM_IMAGE_FIT_CONTAIN,    /* Scale to fit, maintain aspect ratio, may have letterbox */
    VENOM_IMAGE_FIT_COVER,      /* Scale to cover, maintain aspect ratio, may crop */
    VENOM_IMAGE_FIT_FILL,       /* Scale to fill exactly, may distort */
    VENOM_IMAGE_FIT_NONE,       /* Original size, no scaling */
    VENOM_IMAGE_FIT_SCALE_DOWN, /* Like contain, but never scale up */
} VenomImageFit;

/* ============================================================================
 * IMAGE DATA
 * ============================================================================ */

typedef struct VenomImageData {
    VENOM_REF_HEADER;
    
    VenomU8* pixels;            /* RGBA pixel data */
    VenomU32 width;             /* Image width */
    VenomU32 height;            /* Image height */
    VenomU32 stride;            /* Bytes per row */
    
    /* Cairo surface for efficient rendering */
    void* cairo_surface;        /* cairo_surface_t* */
    
} VenomImageData;

/* ============================================================================
 * IMAGE WIDGET
 * ============================================================================ */

typedef struct VenomImageWidget {
    VenomWidget base;
    
    VenomImageData* image;      /* Image data (ref-counted) */
    VenomImageFit fit;          /* How to fit image in bounds */
    VenomF32 corner_radius;     /* Optional rounded corners */
    VenomColor background;      /* Background color for letterbox */
    VenomF32 opacity;           /* Image opacity (0.0 - 1.0) */
    
} VenomImageWidget;

/* ============================================================================
 * IMAGE LOADING API
 * ============================================================================ */

/**
 * @brief Load image from file (PNG, JPEG, etc.)
 */
VenomResultPtr venom_image_load_file(const char* path);

/**
 * @brief Load image from memory
 */
VenomResultPtr venom_image_load_memory(const VenomU8* data, VenomSize size);

/**
 * @brief Create empty image with size
 */
VenomResultPtr venom_image_create(VenomU32 width, VenomU32 height);

/**
 * @brief Get image dimensions
 */
void venom_image_get_size(const VenomImageData* image, VenomU32* width, VenomU32* height);

/* ============================================================================
 * IMAGE WIDGET API
 * ============================================================================ */

/**
 * @brief Create image widget
 */
VenomResultPtr venom_image_widget_create(void);

/**
 * @brief Set image data
 */
VenomResult venom_image_widget_set_image(VenomImageWidget* widget, VenomImageData* image);

/**
 * @brief Load and set image from file
 */
VenomResult venom_image_widget_load(VenomImageWidget* widget, const char* path);

/**
 * @brief Set fit mode
 */
void venom_image_widget_set_fit(VenomImageWidget* widget, VenomImageFit fit);

/**
 * @brief Set corner radius
 */
void venom_image_widget_set_corner_radius(VenomImageWidget* widget, VenomF32 radius);

/**
 * @brief Set opacity
 */
void venom_image_widget_set_opacity(VenomImageWidget* widget, VenomF32 opacity);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VenomWidgetClass venom_image_widget_class;

/* ============================================================================
 * CONVENIENCE MACRO
 * ============================================================================ */

#define venom_image(...) \
    _venom_image_widget_build(&(VenomImageWidgetConfig){ __VA_ARGS__ })

typedef struct VenomImageWidgetConfig {
    const char* src;            /* File path */
    VenomImageData* data;       /* Direct image data */
    VenomImageFit fit;
    VenomF32 corner_radius;
    VenomF32 opacity;
    VenomF32 width;
    VenomF32 height;
} VenomImageWidgetConfig;

VenomWidget* _venom_image_widget_build(const VenomImageWidgetConfig* config);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_IMAGE_WIDGET_H */
