/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_surface.h - Rendering surface abstraction
 * 
 * A surface represents a drawable target (window, offscreen buffer, etc.)
 */

#ifndef VAXP_SURFACE_H
#define VAXP_SURFACE_H

#include "vaxp/core/vaxp_types.h"
#include "vaxp/core/vaxp_result.h"
#include "vaxp/core/vaxp_ref.h"
#include "vaxp/graphics/vaxp_canvas.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
typedef struct VaxpSurface VaxpSurface;
typedef struct VaxpDisplay VaxpDisplay;

/* ============================================================================
 * SURFACE TYPE
 * ============================================================================ */

typedef enum VaxpSurfaceType {
    VAXP_SURFACE_WINDOW,     /* Backed by a native window */
    VAXP_SURFACE_OFFSCREEN,  /* Offscreen render target */
    VAXP_SURFACE_IMAGE,      /* Backed by an image */
} VaxpSurfaceType;

/* ============================================================================
 * SURFACE OPERATIONS VTABLE
 * ============================================================================ */

typedef struct VaxpSurfaceOps {
    void (*destroy)(VaxpSurface* surface);
    
    VaxpCanvas* (*get_canvas)(VaxpSurface* surface);
    void (*present)(VaxpSurface* surface);  /* Swap buffers / present to screen */
    void (*resize)(VaxpSurface* surface, VaxpU32 width, VaxpU32 height);
    
    VaxpSize2D (*get_size)(VaxpSurface* surface);
    VaxpSurfaceType (*get_type)(VaxpSurface* surface);
} VaxpSurfaceOps;

/* ============================================================================
 * SURFACE STRUCTURE
 * ============================================================================ */

struct VaxpSurface {
    VAXP_REF_HEADER;
    const VaxpSurfaceOps* ops;
    VaxpSurfaceType type;
    VaxpU32 width;
    VaxpU32 height;
};

/* ============================================================================
 * SURFACE PUBLIC API
 * ============================================================================ */

/**
 * @brief Create a surface for a native window (X11)
 * 
 * @param display The display connection
 * @param native_window The native window handle (e.g., X11 Window)
 * @param width Surface width
 * @param height Surface height
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_surface_create_for_window(
    VaxpDisplay* VAXP_NONNULL display,
    void* native_window,
    VaxpU32 width,
    VaxpU32 height
);

/**
 * @brief Create an offscreen surface
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_surface_create_offscreen(VaxpU32 width, VaxpU32 height);

/**
 * @brief Get canvas to draw on this surface
 * 
 * The canvas is valid until present() is called.
 */
VAXP_INLINE VaxpCanvas* vaxp_surface_get_canvas(VaxpSurface* VAXP_NONNULL surface) {
    return surface->ops->get_canvas(surface);
}

/**
 * @brief Present the surface (show on screen for window surfaces)
 */
VAXP_INLINE void vaxp_surface_present(VaxpSurface* VAXP_NONNULL surface) {
    surface->ops->present(surface);
}

/**
 * @brief Resize the surface
 */
VAXP_INLINE void vaxp_surface_resize(VaxpSurface* VAXP_NONNULL surface, 
                                        VaxpU32 width, VaxpU32 height) {
    surface->ops->resize(surface, width, height);
}

/**
 * @brief Get surface dimensions
 */
VAXP_INLINE VaxpSize2D vaxp_surface_get_size(VaxpSurface* VAXP_NONNULL surface) {
    return surface->ops->get_size(surface);
}

/**
 * @brief Get surface type
 */
VAXP_INLINE VaxpSurfaceType vaxp_surface_get_type(VaxpSurface* VAXP_NONNULL surface) {
    return surface->ops->get_type(surface);
}

#ifdef __cplusplus
}
#endif

#endif /* VAXP_SURFACE_H */
