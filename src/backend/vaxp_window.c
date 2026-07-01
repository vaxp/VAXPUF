/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_window.c - Window implementation for X11
 */

#define _DEFAULT_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "vaxp/backend/vaxp_window.h"
#include "vaxp/backend/vaxp_display.h"
#include "vaxp/backend/vaxp_event.h"
#include "vaxp/core/vaxp_memory.h"
#include "vaxp/core/vaxp_focus.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

/* External canvas creation */
extern VaxpResultPtr vaxp_canvas_create_for_xlib(Display* display, Window window,
                                                    Visual* visual, VaxpU32 width, VaxpU32 height);

/* X11 display internal structure */
typedef struct {
    VaxpDisplay base;
    Display* xdisplay;
    int default_screen;
    Window root_window;
} VaxpX11DisplayInternal;

/* ============================================================================
 * WINDOW STRUCTURE
 * ============================================================================ */

struct VaxpWindow {
    VAXP_REF_HEADER;
    
    VaxpU32 id;                    /* Internal window ID */
    Window xwindow;                 /* X11 window handle */
    VaxpCanvas* canvas;            /* Rendering canvas */
    VaxpWidget* root;              /* Root widget */
    
    char* title;
    VaxpI32 x, y;
    VaxpU32 width, height;
    VaxpColor background;
    VaxpWindowFlags flags;
    
    VaxpBool visible;
    VaxpBool has_focus;
    VaxpBool needs_redraw;
    VaxpBool closed;
    
    VaxpWindowCallback on_event;
    void* event_user_data;
};

/* ============================================================================
 * WINDOW MANAGER GLOBALS
 * ============================================================================ */

#define MAX_WINDOWS 64

static struct {
    VaxpDisplay* display;
    VaxpWindow* windows[MAX_WINDOWS];
    VaxpU32 window_count;
    VaxpU32 next_id;
    VaxpWindow* focused;
    VaxpBool running;
    VaxpBool initialized;
} g_wm = {0};

/* ============================================================================
 * WINDOW LIFECYCLE
 * ============================================================================ */

static void window_destructor(void* ptr) {
    VaxpWindow* win = (VaxpWindow*)ptr;
    
    /* IMPORTANT: Destroy canvas FIRST (before X11 window) 
       because Cairo uses the X11 drawable */
    if (win->canvas) {
        vaxp_unref(win->canvas);
        win->canvas = NULL;
    }
    
    /* Now safe to destroy X11 window */
    if (win->xwindow && g_wm.display) {
        VaxpX11DisplayInternal* x11 = (VaxpX11DisplayInternal*)g_wm.display;
        XDestroyWindow(x11->xdisplay, win->xwindow);
        XFlush(x11->xdisplay);
        win->xwindow = 0;
    }
    
    if (win->root) {
        vaxp_unref(win->root);
        win->root = NULL;
    }
    
    if (win->title) {
        vaxp_free(win->title, strlen(win->title) + 1);
        win->title = NULL;
    }
    
    /* Remove from window list */
    for (VaxpU32 i = 0; i < g_wm.window_count; i++) {
        if (g_wm.windows[i] == win) {
            for (VaxpU32 j = i; j < g_wm.window_count - 1; j++) {
                g_wm.windows[j] = g_wm.windows[j + 1];
            }
            g_wm.window_count--;
            break;
        }
    }
    
    if (g_wm.focused == win) {
        g_wm.focused = NULL;
    }
}

VaxpResultPtr vaxp_window_create(const VaxpWindowConfig* config) {
    if (!g_wm.initialized) {
        return VAXP_ERR_PTR(VAXP_ERROR_INVALID_STATE);
    }
    
    if (g_wm.window_count >= MAX_WINDOWS) {
        return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
    }
    
    /* Allocate window */
    VaxpWindow* win = (VaxpWindow*)vaxp_alloc_zeroed(sizeof(VaxpWindow));
    if (!win) {
        return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
    }
    
    vaxp_ref_init(win, sizeof(VaxpWindow), window_destructor, "VaxpWindow");
    
    /* Set properties */
    win->id = ++g_wm.next_id;
    win->width = config->width > 0 ? config->width : 800;
    win->height = config->height > 0 ? config->height : 600;
    win->x = config->x;
    win->y = config->y;
    win->flags = config->flags;
    win->background = config->background.a > 0 ? config->background : vaxp_color_rgb(250, 250, 252);
    win->on_event = config->on_event;
    win->event_user_data = config->event_user_data;
    win->needs_redraw = VAXP_TRUE;
    
    /* Copy title */
    const char* title = config->title ? config->title : "VAXPUI Window";
    VaxpSize title_len = strlen(title) + 1;
    win->title = (char*)vaxp_alloc(title_len);
    if (win->title) {
        memcpy(win->title, title, title_len);
    }
    
    /* Create X11 window */
    VaxpX11DisplayInternal* x11 = (VaxpX11DisplayInternal*)g_wm.display;
    
    VaxpResultPtr xwin_result = g_wm.display->ops->create_window(
        g_wm.display, title,
        config->x >= 0 ? config->x : 100,
        config->y >= 0 ? config->y : 100,
        win->width, win->height
    );
    
    if (!xwin_result.ok) {
        vaxp_unref(win);
        return xwin_result;
    }
    
    win->xwindow = (Window)(uintptr_t)xwin_result.value;
    
    /* Create canvas */
#ifdef VAXP_USE_OPENGL
    extern VaxpResultPtr vaxp_canvas_create_opengl(Display* display, Window window, VaxpU32 width, VaxpU32 height);
    VaxpResultPtr canvas_result = vaxp_canvas_create_opengl(
        x11->xdisplay, win->xwindow, win->width, win->height
    );
#else
    Visual* visual = DefaultVisual(x11->xdisplay, x11->default_screen);
    VaxpResultPtr canvas_result = vaxp_canvas_create_for_xlib(
        x11->xdisplay, win->xwindow, visual, win->width, win->height
    );
#endif
    
    if (!canvas_result.ok) {
        vaxp_unref(win);
        return canvas_result;
    }
    
    win->canvas = (VaxpCanvas*)canvas_result.value;
    
    /* Set root widget if provided */
    if (config->root) {
        win->root = (VaxpWidget*)vaxp_ref(config->root);
        VaxpRectF bounds = { 0, 0, (VaxpF32)win->width, (VaxpF32)win->height };
        vaxp_widget_layout(win->root, bounds);
    }
    
    /* Add to window list */
    g_wm.windows[g_wm.window_count++] = win;
    
    /* Show if not hidden */
    if (!(config->flags & VAXP_WINDOW_HIDDEN)) {
        win->visible = VAXP_TRUE;
    }
    
    return VAXP_OK_PTR(win);
}

VaxpResultPtr vaxp_window_create_simple(const char* title, VaxpU32 width, VaxpU32 height) {
    VaxpWindowConfig config = {
        .title = title,
        .width = width,
        .height = height,
        .x = -1,
        .y = -1,
        .flags = VAXP_WINDOW_RESIZABLE | VAXP_WINDOW_CENTERED,
    };
    return vaxp_window_create(&config);
}

void vaxp_window_destroy(VaxpWindow* window) {
    if (!window) return;
    vaxp_unref(window);
}

/* ============================================================================
 * WINDOW VISIBILITY
 * ============================================================================ */

void vaxp_window_show(VaxpWindow* window) {
    if (!window || window->visible) return;
    window->visible = VAXP_TRUE;
    window->needs_redraw = VAXP_TRUE;
    
    if (window->on_event) {
        window->on_event(window, VAXP_WINDOW_LIFECYCLE_SHOWN, window->event_user_data);
    }
}

void vaxp_window_hide(VaxpWindow* window) {
    if (!window || !window->visible) return;
    window->visible = VAXP_FALSE;
    
    if (window->on_event) {
        window->on_event(window, VAXP_WINDOW_LIFECYCLE_HIDDEN, window->event_user_data);
    }
}

void vaxp_window_close(VaxpWindow* window) {
    if (!window || window->closed) return;
    
    if (window->on_event) {
        window->on_event(window, VAXP_WINDOW_LIFECYCLE_CLOSE_REQUESTED, window->event_user_data);
    }
    
    /* Unmap (hide) the X11 window immediately */
    if (window->xwindow && g_wm.display) {
        VaxpX11DisplayInternal* x11 = (VaxpX11DisplayInternal*)g_wm.display;
        XUnmapWindow(x11->xdisplay, window->xwindow);
        XFlush(x11->xdisplay);
    }
    
    window->closed = VAXP_TRUE;
    window->visible = VAXP_FALSE;
    
    if (window->on_event) {
        window->on_event(window, VAXP_WINDOW_LIFECYCLE_CLOSED, window->event_user_data);
    }
}

VaxpBool vaxp_window_is_visible(const VaxpWindow* window) {
    return window ? window->visible : VAXP_FALSE;
}

/* ============================================================================
 * WINDOW PROPERTIES
 * ============================================================================ */

void vaxp_window_set_title(VaxpWindow* window, const char* title) {
    if (!window || !title) return;
    
    if (window->title) {
        vaxp_free(window->title, strlen(window->title) + 1);
    }
    
    VaxpSize len = strlen(title) + 1;
    window->title = (char*)vaxp_alloc(len);
    if (window->title) {
        memcpy(window->title, title, len);
    }
    
    /* Update X11 window title */
    VaxpX11DisplayInternal* x11 = (VaxpX11DisplayInternal*)g_wm.display;
    XStoreName(x11->xdisplay, window->xwindow, title);
}

void vaxp_window_set_position(VaxpWindow* window, VaxpI32 x, VaxpI32 y) {
    if (!window) return;
    window->x = x;
    window->y = y;
    
    VaxpX11DisplayInternal* x11 = (VaxpX11DisplayInternal*)g_wm.display;
    XMoveWindow(x11->xdisplay, window->xwindow, x, y);
}

void vaxp_window_get_position(const VaxpWindow* window, VaxpI32* x, VaxpI32* y) {
    if (!window) return;
    if (x) *x = window->x;
    if (y) *y = window->y;
}

void vaxp_window_set_size(VaxpWindow* window, VaxpU32 width, VaxpU32 height) {
    if (!window) return;
    window->width = width;
    window->height = height;
    window->needs_redraw = VAXP_TRUE;
    
    VaxpX11DisplayInternal* x11 = (VaxpX11DisplayInternal*)g_wm.display;
    XResizeWindow(x11->xdisplay, window->xwindow, width, height);
    
    /* Relayout root widget */
    if (window->root) {
        VaxpRectF bounds = { 0, 0, (VaxpF32)width, (VaxpF32)height };
        vaxp_widget_layout(window->root, bounds);
    }
}

void vaxp_window_get_size(const VaxpWindow* window, VaxpU32* width, VaxpU32* height) {
    if (!window) return;
    if (width) *width = window->width;
    if (height) *height = window->height;
}

void vaxp_window_set_root(VaxpWindow* window, VaxpWidget* root) {
    if (!window) return;
    
    if (window->root) {
        vaxp_unref(window->root);
    }
    
    window->root = root ? (VaxpWidget*)vaxp_ref(root) : NULL;
    
    if (window->root) {
        VaxpRectF bounds = { 0, 0, (VaxpF32)window->width, (VaxpF32)window->height };
        vaxp_widget_layout(window->root, bounds);
    }
    
    window->needs_redraw = VAXP_TRUE;
}

VaxpWidget* vaxp_window_get_root(VaxpWindow* window) {
    return window ? window->root : NULL;
}

void vaxp_window_set_background(VaxpWindow* window, VaxpColor color) {
    if (!window) return;
    window->background = color;
    window->needs_redraw = VAXP_TRUE;
}

void vaxp_window_invalidate(VaxpWindow* window) {
    if (window) window->needs_redraw = VAXP_TRUE;
}

VaxpU32 vaxp_window_get_id(const VaxpWindow* window) {
    return window ? window->id : 0;
}

VaxpBool vaxp_window_has_focus(const VaxpWindow* window) {
    return window ? window->has_focus : VAXP_FALSE;
}

void vaxp_window_request_focus(VaxpWindow* window) {
    if (!window) return;
    
    VaxpX11DisplayInternal* x11 = (VaxpX11DisplayInternal*)g_wm.display;
    XSetInputFocus(x11->xdisplay, window->xwindow, RevertToParent, CurrentTime);
    
    if (g_wm.focused && g_wm.focused != window) {
        g_wm.focused->has_focus = VAXP_FALSE;
        if (g_wm.focused->on_event) {
            g_wm.focused->on_event(g_wm.focused, VAXP_WINDOW_LIFECYCLE_FOCUS_LOST, 
                                    g_wm.focused->event_user_data);
        }
    }
    
    window->has_focus = VAXP_TRUE;
    g_wm.focused = window;
    
    if (window->on_event) {
        window->on_event(window, VAXP_WINDOW_LIFECYCLE_FOCUS_GAINED, window->event_user_data);
    }
}

/* ============================================================================
 * WINDOW MANAGER
 * ============================================================================ */

VaxpResult vaxp_window_manager_init(void) {
    if (g_wm.initialized) {
        return VAXP_OK_UNIT();
    }
    
    /* Open display */
    VaxpResultPtr display_result = vaxp_display_open(VAXP_BACKEND_X11, NULL);
    if (!display_result.ok) {
        return VAXP_ERR_UNIT(display_result.error);
    }
    
    g_wm.display = (VaxpDisplay*)display_result.value;
    g_wm.initialized = VAXP_TRUE;
    g_wm.running = VAXP_FALSE;
    
    return VAXP_OK_UNIT();
}

void vaxp_window_manager_shutdown(void) {
    if (!g_wm.initialized) return;
    
    /* Close all windows */
    while (g_wm.window_count > 0) {
        vaxp_window_destroy(g_wm.windows[0]);
    }
    
    if (g_wm.display) {
        vaxp_display_close(g_wm.display);
        g_wm.display = NULL;
    }
    
    g_wm.initialized = VAXP_FALSE;
}

VaxpU32 vaxp_window_count(void) {
    return g_wm.window_count;
}

VaxpWindow* vaxp_window_at(VaxpU32 index) {
    return index < g_wm.window_count ? g_wm.windows[index] : NULL;
}

VaxpWindow* vaxp_window_get_focused(void) {
    return g_wm.focused;
}

/* ============================================================================
 * MAIN EVENT LOOP
 * ============================================================================ */

static VaxpWindow* find_window_by_xwindow(Window xwin) {
    for (VaxpU32 i = 0; i < g_wm.window_count; i++) {
        if (g_wm.windows[i]->xwindow == xwin) {
            return g_wm.windows[i];
        }
    }
    return NULL;
}

int vaxp_run(void) {
    if (!g_wm.initialized) {
        fprintf(stderr, "VAXPUI: Window manager not initialized\n");
        return 1;
    }
    
    g_wm.running = VAXP_TRUE;
    
    while (g_wm.running && g_wm.window_count > 0) {
        /* Draw all windows that need redraw */
        for (VaxpU32 i = 0; i < g_wm.window_count; i++) {
            VaxpWindow* win = g_wm.windows[i];
            if (win->visible && win->needs_redraw && win->canvas) {
                vaxp_canvas_clear(win->canvas, win->background);
                if (win->root) {
                    vaxp_widget_draw(win->root, win->canvas);
                }
                vaxp_canvas_flush(win->canvas);
                win->needs_redraw = VAXP_FALSE;
            }
        }
        
        /* Process events */
        VaxpEvent event;
        while (vaxp_display_poll_event(g_wm.display, &event)) {
            /* Find target window - for now use focused */
            VaxpWindow* target = g_wm.focused;
            if (!target && g_wm.window_count > 0) {
                target = g_wm.windows[0];
            }
            
            if (!target) continue;
            
            switch (event.type) {
                case VAXP_EVENT_WINDOW_CLOSE:
                    vaxp_window_close(target);
                    break;
                    
                case VAXP_EVENT_KEY_DOWN:
                    if (event.key.key == VAXP_KEY_ESCAPE) {
                        vaxp_window_close(target);
                    } else if (event.key.key == VAXP_KEY_TAB) {
                        /* Tab navigation */
                        if (event.key.modifiers & VAXP_KEYMOD_SHIFT) {
                            vaxp_focus_prev();
                        } else {
                            vaxp_focus_next();
                        }
                        target->needs_redraw = VAXP_TRUE;
                    } else if (event.key.key == VAXP_KEY_RETURN || 
                               event.key.key == VAXP_KEY_SPACE) {
                        /* Activate focused widget (simulate click) */
                        VaxpWidget* focused = vaxp_focus_get();
                        if (focused) {
                            VaxpEvent click = { .type = VAXP_EVENT_MOUSE_BUTTON_DOWN };
                            click.mouse.button = VAXP_MOUSE_BUTTON_LEFT;
                            vaxp_widget_dispatch_event(focused, &click);
                            click.type = VAXP_EVENT_MOUSE_BUTTON_UP;
                            vaxp_widget_dispatch_event(focused, &click);
                            target->needs_redraw = VAXP_TRUE;
                        }
                    }
                    break;
                    
                case VAXP_EVENT_WINDOW_RESIZE:
                    if (target->width != event.window.width || target->height != event.window.height) {
                        target->width = event.window.width;
                        target->height = event.window.height;
                        if (target->canvas) {
                            target->canvas->width = event.window.width;
                            target->canvas->height = event.window.height;
                        }
                        if (target->root) {
                            VaxpRectF bounds = {0, 0, (VaxpF32)event.window.width, (VaxpF32)event.window.height};
                            vaxp_widget_layout(target->root, bounds);
                        }
                        target->needs_redraw = VAXP_TRUE;
                    }
                    break;
                    
                case VAXP_EVENT_WINDOW_EXPOSE:
                    target->needs_redraw = VAXP_TRUE;
                    break;
                    
                default:
                    break;
            }
            
            /* Dispatch to widget tree */
            if (target->root && vaxp_widget_dispatch_event(target->root, &event)) {
                target->needs_redraw = VAXP_TRUE;
            }
        }
        
        /* Remove closed windows */
        for (VaxpU32 i = 0; i < g_wm.window_count; ) {
            if (g_wm.windows[i]->closed) {
                vaxp_window_destroy(g_wm.windows[i]);
            } else {
                i++;
            }
        }
        
        usleep(1000);  /* Unlocked ~1000fps or V-Sync driven */
    }
    
    g_wm.running = VAXP_FALSE;
    return 0;
}

void vaxp_quit(void) {
    g_wm.running = VAXP_FALSE;
}

VaxpBool vaxp_is_running(void) {
    return g_wm.running;
}
