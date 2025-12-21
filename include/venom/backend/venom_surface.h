/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_surface.h - Rendering surface abstraction
 * 
 * A surface represents a drawable target (window, offscreen buffer, etc.)
 */

#ifndef VENOM_SURFACE_H
#define VENOM_SURFACE_H

#include "venom/core/venom_types.h"
#include "venom/core/venom_result.h"
#include "venom/core/venom_ref.h"
#include "venom/graphics/venom_canvas.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
typedef struct VenomSurface VenomSurface;
typedef struct VenomDisplay VenomDisplay;

/* ============================================================================
 * SURFACE TYPE
 * ============================================================================ */

typedef enum VenomSurfaceType {
    VENOM_SURFACE_WINDOW,     /* Backed by a native window */
    VENOM_SURFACE_OFFSCREEN,  /* Offscreen render target */
    VENOM_SURFACE_IMAGE,      /* Backed by an image */
} VenomSurfaceType;

/* ============================================================================
 * SURFACE OPERATIONS VTABLE
 * ============================================================================ */

typedef struct VenomSurfaceOps {
    void (*destroy)(VenomSurface* surface);
    
    VenomCanvas* (*get_canvas)(VenomSurface* surface);
    void (*present)(VenomSurface* surface);  /* Swap buffers / present to screen */
    void (*resize)(VenomSurface* surface, VenomU32 width, VenomU32 height);
    
    VenomSize2D (*get_size)(VenomSurface* surface);
    VenomSurfaceType (*get_type)(VenomSurface* surface);
} VenomSurfaceOps;

/* ============================================================================
 * SURFACE STRUCTURE
 * ============================================================================ */

struct VenomSurface {
    VENOM_REF_HEADER;
    const VenomSurfaceOps* ops;
    VenomSurfaceType type;
    VenomU32 width;
    VenomU32 height;
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
VENOM_WARN_UNUSED
VenomResultPtr venom_surface_create_for_window(
    VenomDisplay* VENOM_NONNULL display,
    void* native_window,
    VenomU32 width,
    VenomU32 height
);

/**
 * @brief Create an offscreen surface
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_surface_create_offscreen(VenomU32 width, VenomU32 height);

/**
 * @brief Get canvas to draw on this surface
 * 
 * The canvas is valid until present() is called.
 */
VENOM_INLINE VenomCanvas* venom_surface_get_canvas(VenomSurface* VENOM_NONNULL surface) {
    return surface->ops->get_canvas(surface);
}

/**
 * @brief Present the surface (show on screen for window surfaces)
 */
VENOM_INLINE void venom_surface_present(VenomSurface* VENOM_NONNULL surface) {
    surface->ops->present(surface);
}

/**
 * @brief Resize the surface
 */
VENOM_INLINE void venom_surface_resize(VenomSurface* VENOM_NONNULL surface, 
                                        VenomU32 width, VenomU32 height) {
    surface->ops->resize(surface, width, height);
}

/**
 * @brief Get surface dimensions
 */
VENOM_INLINE VenomSize2D venom_surface_get_size(VenomSurface* VENOM_NONNULL surface) {
    return surface->ops->get_size(surface);
}

/**
 * @brief Get surface type
 */
VENOM_INLINE VenomSurfaceType venom_surface_get_type(VenomSurface* VENOM_NONNULL surface) {
    return surface->ops->get_type(surface);
}

#ifdef __cplusplus
}
#endif

#endif /* VENOM_SURFACE_H */
