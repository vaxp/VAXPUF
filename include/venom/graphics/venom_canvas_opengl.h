/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_canvas_opengl.h - OpenGL canvas public API
 * 
 * Provides OpenGL-specific functions for 3D rendering.
 */

#ifndef VENOM_CANVAS_OPENGL_H
#define VENOM_CANVAS_OPENGL_H

#include "venom/graphics/venom_canvas.h"
#include <X11/Xlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * OPENGL CANVAS CREATION
 * ============================================================================ */

/**
 * @brief Create an OpenGL canvas for an X11 window
 * 
 * Creates an OpenGL 3.3 Core Profile context and associates it with the window.
 * 
 * @param display X11 display connection
 * @param window X11 window to render to
 * @param width Canvas width in pixels
 * @param height Canvas height in pixels
 * @return VenomCanvas* or NULL on failure
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_canvas_create_opengl(Display* display, Window window, 
                                           VenomU32 width, VenomU32 height);

/* ============================================================================
 * 3D RENDERING API
 * ============================================================================ */

/**
 * @brief Begin 3D rendering mode
 * 
 * Enables depth testing and clears depth buffer.
 * Call this before drawing 3D objects.
 */
void venom_gl_begin_3d(VenomCanvas* canvas);

/**
 * @brief End 3D rendering mode
 * 
 * Disables depth testing. Call this after drawing 3D objects
 * and before resuming 2D drawing.
 */
void venom_gl_end_3d(VenomCanvas* canvas);

/**
 * @brief Draw a 3D cube with Phong lighting
 * 
 * Draws a colored cube at the specified 3D position with rotation.
 * Each face of the cube has a different color.
 * 
 * @param canvas OpenGL canvas
 * @param x X position in 3D space
 * @param y Y position in 3D space
 * @param z Z position in 3D space
 * @param size Size of the cube
 * @param rotX Rotation around X axis in radians
 * @param rotY Rotation around Y axis in radians
 * @param rotZ Rotation around Z axis in radians
 */
void venom_gl_draw_cube(VenomCanvas* canvas, float x, float y, float z,
                        float size, float rotX, float rotY, float rotZ);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_CANVAS_OPENGL_H */
