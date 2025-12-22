/*
 * VENOMUI - OpenGL Demo
 * 
 * Showcases OpenGL rendering capabilities including:
 * - 2D shapes with smooth anti-aliasing
 * - Rotating 3D cube with Phong lighting
 * - Performance comparison
 */

#define _POSIX_C_SOURCE 199309L
#define _DEFAULT_SOURCE
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <time.h>
#include <unistd.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

/* Include OpenGL headers */
#include <GL/gl.h>
#include <GL/glx.h>

#include "venom/core/venom_types.h"
#include "venom/core/venom_result.h"
#include "venom/core/venom_ref.h"
#include "venom/graphics/venom_canvas.h"
#include "venom/graphics/venom_canvas_opengl.h"

/* ============================================================================
 * DEMO STATE
 * ============================================================================ */

static double get_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    
    printf("===========================================\n");
    printf("   VENOMUI OpenGL Demo\n");
    printf("   Showcasing GPU-Accelerated Rendering\n");
    printf("===========================================\n\n");
    
    /* Initialize X11 */
    Display* display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Failed to open X display\n");
        return 1;
    }
    
    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);
    
    /* Create window */
    int width = 900;
    int height = 700;
    
    /* Get visual for OpenGL */
    static int visual_attribs[] = {
        GLX_X_RENDERABLE, True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_DOUBLEBUFFER, True,
        None
    };
    
    int fbcount;
    GLXFBConfig* fbc = glXChooseFBConfig(display, screen, visual_attribs, &fbcount);
    if (!fbc || fbcount == 0) {
        fprintf(stderr, "Failed to find suitable framebuffer config\n");
        XCloseDisplay(display);
        return 1;
    }
    
    XVisualInfo* vi = glXGetVisualFromFBConfig(display, fbc[0]);
    XFree(fbc);
    
    /* Create colormap */
    Colormap colormap = XCreateColormap(display, root, vi->visual, AllocNone);
    
    XSetWindowAttributes swa;
    swa.colormap = colormap;
    swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;
    swa.background_pixel = 0;
    swa.border_pixel = 0;
    
    Window window = XCreateWindow(display, root, 100, 100, width, height, 0,
                                   vi->depth, InputOutput, vi->visual,
                                   CWColormap | CWEventMask | CWBackPixel | CWBorderPixel,
                                   &swa);
    XFree(vi);
    
    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        XCloseDisplay(display);
        return 1;
    }
    
    /* Set window title */
    XStoreName(display, window, "VENOMUI - OpenGL 3D Demo");
    
    /* Show window */
    XMapWindow(display, window);
    XFlush(display);
    
    /* Wait for window to be mapped */
    XEvent event;
    while (1) {
        XNextEvent(display, &event);
        if (event.type == Expose) break;
    }
    
    /* Create OpenGL canvas */
    printf("Creating OpenGL canvas...\n");
    VenomResultPtr canvas_result = venom_canvas_create_opengl(display, window, width, height);
    if (!canvas_result.ok) {
        fprintf(stderr, "Failed to create OpenGL canvas\n");
        XDestroyWindow(display, window);
        XCloseDisplay(display);
        return 1;
    }
    VenomCanvas* canvas = (VenomCanvas*)canvas_result.value;
    printf("OpenGL canvas created successfully!\n\n");
    
    /* Disable VSync for maximum FPS */
    typedef int (*glXSwapIntervalEXTProc)(Display*, GLXDrawable, int);
    typedef int (*glXSwapIntervalMESAProc)(int);
    
    glXSwapIntervalEXTProc glXSwapIntervalEXT = 
        (glXSwapIntervalEXTProc)glXGetProcAddress((unsigned char*)"glXSwapIntervalEXT");
    glXSwapIntervalMESAProc glXSwapIntervalMESA = 
        (glXSwapIntervalMESAProc)glXGetProcAddress((unsigned char*)"glXSwapIntervalMESA");
    
    if (glXSwapIntervalEXT) {
        glXSwapIntervalEXT(display, window, 0);  /* 0 = VSync OFF */
        printf("VSync disabled via EXT extension\n");
    } else if (glXSwapIntervalMESA) {
        glXSwapIntervalMESA(0);  /* 0 = VSync OFF */
        printf("VSync disabled via MESA extension\n");
    } else {
        printf("Warning: Could not disable VSync\n");
    }
    
    /* Setup window close handling */
    Atom wm_delete = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wm_delete, 1);
    
    /* Animation state */
    double start_time = get_time();
    double last_frame_time = start_time;
    int frame_count = 0;
    double fps = 0.0;
    
    printf("Starting render loop...\n");
    printf("Press ESC or close window to exit.\n\n");
    
    /* Main loop */
    int running = 1;
    while (running) {
        /* Handle events */
        while (XPending(display)) {
            XNextEvent(display, &event);
            
            switch (event.type) {
                case KeyPress: {
                    KeySym key = XLookupKeysym(&event.xkey, 0);
                    if (key == XK_Escape || key == XK_q) {
                        running = 0;
                    }
                    break;
                }
                case ClientMessage:
                    if ((Atom)event.xclient.data.l[0] == wm_delete) {
                        running = 0;
                    }
                    break;
                case ConfigureNotify:
                    width = event.xconfigure.width;
                    height = event.xconfigure.height;
                    glViewport(0, 0, width, height);
                    break;
            }
        }
        
        double current_time = get_time();
        double elapsed = current_time - start_time;
        
        /* Calculate FPS */
        frame_count++;
        if (current_time - last_frame_time >= 1.0) {
            fps = frame_count / (current_time - last_frame_time);
            frame_count = 0;
            last_frame_time = current_time;
            printf("\rFPS: %.1f  ", fps);
            fflush(stdout);
        }
        
        /* Clear with dark gradient-like background */
        VenomColor bg = venom_color_rgb(25, 25, 35);
        venom_canvas_clear(canvas, bg);
        
        /* ===== 3D SECTION ===== */
        venom_gl_begin_3d(canvas);
        
        /* Draw rotating cubes */
        float rotation = (float)elapsed * 0.8f;
        
        /* Central large cube */
        venom_gl_draw_cube(canvas, 0.0f, 0.0f, 0.0f, 1.5f, 
                           rotation, rotation * 1.3f, rotation * 0.7f);
        
        /* Orbiting smaller cubes */
        for (int i = 0; i < 4; i++) {
            float angle = rotation * 0.5f + i * (M_PI / 2.0f);
            float orbit_radius = 2.5f;
            float x = cosf(angle) * orbit_radius;
            float z = sinf(angle) * orbit_radius;
            float y = sinf(elapsed * 2.0f + i) * 0.5f;
            
            venom_gl_draw_cube(canvas, x, y, z, 0.5f,
                               rotation * 2.0f, rotation * 1.5f, rotation);
        }
        
        venom_gl_end_3d(canvas);
        
        /* ===== 2D OVERLAY ===== */
        
        /* Draw UI elements using 2D canvas API */
        VenomPaint paint_white = venom_paint_fill(venom_color_rgba(255, 255, 255, 200));
        VenomPaint paint_accent = venom_paint_fill(venom_color_rgba(100, 150, 255, 180));
        VenomPaint paint_success = venom_paint_fill(venom_color_rgba(80, 200, 120, 200));
        
        /* Title bar */
        VenomRectF title_bg = { 20, 20, 280, 50 };
        venom_canvas_draw_rounded_rect(canvas, title_bg, 12.0f, &paint_accent);
        
        /* Status area */
        VenomRectF status_bg = { 20, 80, 200, 35 };
        venom_canvas_draw_rounded_rect(canvas, status_bg, 8.0f, &paint_success);
        
        /* Decorative circles */
        float pulse = (sinf((float)elapsed * 3.0f) + 1.0f) * 0.5f;
        VenomPaint paint_pulse = venom_paint_fill(venom_color_rgba(255, 100, 150, (int)(150 + pulse * 100)));
        venom_canvas_draw_circle(canvas, (float)width - 50, 50, 20 + pulse * 10, &paint_pulse);
        
        /* Draw some animated rectangles at bottom */
        for (int i = 0; i < 5; i++) {
            float offset = sinf((float)elapsed * 2.0f + i * 0.5f) * 20.0f;
            VenomRectF rect = { 
                30.0f + i * 80.0f, 
                (float)height - 60.0f + offset, 
                60.0f, 
                40.0f 
            };
            
            VenomColor color = venom_color_rgba(
                (VenomU8)(100 + i * 30),
                (VenomU8)(150 + i * 20),
                (VenomU8)(200 - i * 20),
                200
            );
            VenomPaint paint = venom_paint_fill(color);
            venom_canvas_draw_rounded_rect(canvas, rect, 8.0f, &paint);
        }
        
        /* Draw lines */
        VenomPaint line_paint = venom_paint_stroke(venom_color_rgba(200, 200, 200, 100), 2.0f);
        for (int i = 0; i < 3; i++) {
            float y = 150.0f + i * 30.0f;
            venom_canvas_draw_line(canvas, 20.0f, y, 300.0f, y + sinf(elapsed + i) * 20.0f, &line_paint);
        }
        
        /* Swap buffers */
        venom_canvas_flush(canvas);
        
        /* No sleep - unlimited FPS! */
    }
    
    printf("\n\n");
    printf("Demo finished!\n");
    printf("Final stats:\n");
    printf("  - Average FPS: %.1f\n", fps);
    printf("  - Total frames: calculated during runtime\n");
    
    /* Cleanup */
    venom_unref(canvas);
    XFreeColormap(display, colormap);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    
    printf("\nOpenGL demo completed successfully!\n");
    return 0;
}
