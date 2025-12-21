/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_window.c - Window implementation for X11
 */

#define _DEFAULT_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "venom/backend/venom_window.h"
#include "venom/backend/venom_display.h"
#include "venom/backend/venom_event.h"
#include "venom/core/venom_memory.h"
#include "venom/core/venom_focus.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

/* External canvas creation */
extern VenomResultPtr venom_canvas_create_for_xlib(Display* display, Window window,
                                                    Visual* visual, VenomU32 width, VenomU32 height);

/* X11 display internal structure */
typedef struct {
    VenomDisplay base;
    Display* xdisplay;
    int default_screen;
    Window root_window;
} VenomX11DisplayInternal;

/* ============================================================================
 * WINDOW STRUCTURE
 * ============================================================================ */

struct VenomWindow {
    VENOM_REF_HEADER;
    
    VenomU32 id;                    /* Internal window ID */
    Window xwindow;                 /* X11 window handle */
    VenomCanvas* canvas;            /* Rendering canvas */
    VenomWidget* root;              /* Root widget */
    
    char* title;
    VenomI32 x, y;
    VenomU32 width, height;
    VenomColor background;
    VenomWindowFlags flags;
    
    VenomBool visible;
    VenomBool has_focus;
    VenomBool needs_redraw;
    VenomBool closed;
    
    VenomWindowCallback on_event;
    void* event_user_data;
};

/* ============================================================================
 * WINDOW MANAGER GLOBALS
 * ============================================================================ */

#define MAX_WINDOWS 64

static struct {
    VenomDisplay* display;
    VenomWindow* windows[MAX_WINDOWS];
    VenomU32 window_count;
    VenomU32 next_id;
    VenomWindow* focused;
    VenomBool running;
    VenomBool initialized;
} g_wm = {0};

/* ============================================================================
 * WINDOW LIFECYCLE
 * ============================================================================ */

static void window_destructor(void* ptr) {
    VenomWindow* win = (VenomWindow*)ptr;
    
    /* IMPORTANT: Destroy canvas FIRST (before X11 window) 
       because Cairo uses the X11 drawable */
    if (win->canvas) {
        venom_unref(win->canvas);
        win->canvas = NULL;
    }
    
    /* Now safe to destroy X11 window */
    if (win->xwindow && g_wm.display) {
        VenomX11DisplayInternal* x11 = (VenomX11DisplayInternal*)g_wm.display;
        XDestroyWindow(x11->xdisplay, win->xwindow);
        XFlush(x11->xdisplay);
        win->xwindow = 0;
    }
    
    if (win->root) {
        venom_unref(win->root);
        win->root = NULL;
    }
    
    if (win->title) {
        venom_free(win->title, strlen(win->title) + 1);
        win->title = NULL;
    }
    
    /* Remove from window list */
    for (VenomU32 i = 0; i < g_wm.window_count; i++) {
        if (g_wm.windows[i] == win) {
            for (VenomU32 j = i; j < g_wm.window_count - 1; j++) {
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

VenomResultPtr venom_window_create(const VenomWindowConfig* config) {
    if (!g_wm.initialized) {
        return VENOM_ERR_PTR(VENOM_ERROR_INVALID_STATE);
    }
    
    if (g_wm.window_count >= MAX_WINDOWS) {
        return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    /* Allocate window */
    VenomWindow* win = (VenomWindow*)venom_alloc_zeroed(sizeof(VenomWindow));
    if (!win) {
        return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    venom_ref_init(win, sizeof(VenomWindow), window_destructor, "VenomWindow");
    
    /* Set properties */
    win->id = ++g_wm.next_id;
    win->width = config->width > 0 ? config->width : 800;
    win->height = config->height > 0 ? config->height : 600;
    win->x = config->x;
    win->y = config->y;
    win->flags = config->flags;
    win->background = config->background.a > 0 ? config->background : venom_color_rgb(250, 250, 252);
    win->on_event = config->on_event;
    win->event_user_data = config->event_user_data;
    win->needs_redraw = VENOM_TRUE;
    
    /* Copy title */
    const char* title = config->title ? config->title : "VENOMUI Window";
    VenomSize title_len = strlen(title) + 1;
    win->title = (char*)venom_alloc(title_len);
    if (win->title) {
        memcpy(win->title, title, title_len);
    }
    
    /* Create X11 window */
    VenomX11DisplayInternal* x11 = (VenomX11DisplayInternal*)g_wm.display;
    
    VenomResultPtr xwin_result = g_wm.display->ops->create_window(
        g_wm.display, title,
        config->x >= 0 ? config->x : 100,
        config->y >= 0 ? config->y : 100,
        win->width, win->height
    );
    
    if (!xwin_result.ok) {
        venom_unref(win);
        return xwin_result;
    }
    
    win->xwindow = (Window)(uintptr_t)xwin_result.value;
    
    /* Create canvas */
    Visual* visual = DefaultVisual(x11->xdisplay, x11->default_screen);
    VenomResultPtr canvas_result = venom_canvas_create_for_xlib(
        x11->xdisplay, win->xwindow, visual, win->width, win->height
    );
    
    if (!canvas_result.ok) {
        venom_unref(win);
        return canvas_result;
    }
    
    win->canvas = (VenomCanvas*)canvas_result.value;
    
    /* Set root widget if provided */
    if (config->root) {
        win->root = (VenomWidget*)venom_ref(config->root);
        VenomRectF bounds = { 0, 0, (VenomF32)win->width, (VenomF32)win->height };
        venom_widget_layout(win->root, bounds);
    }
    
    /* Add to window list */
    g_wm.windows[g_wm.window_count++] = win;
    
    /* Show if not hidden */
    if (!(config->flags & VENOM_WINDOW_HIDDEN)) {
        win->visible = VENOM_TRUE;
    }
    
    return VENOM_OK_PTR(win);
}

VenomResultPtr venom_window_create_simple(const char* title, VenomU32 width, VenomU32 height) {
    VenomWindowConfig config = {
        .title = title,
        .width = width,
        .height = height,
        .x = -1,
        .y = -1,
        .flags = VENOM_WINDOW_RESIZABLE | VENOM_WINDOW_CENTERED,
    };
    return venom_window_create(&config);
}

void venom_window_destroy(VenomWindow* window) {
    if (!window) return;
    venom_unref(window);
}

/* ============================================================================
 * WINDOW VISIBILITY
 * ============================================================================ */

void venom_window_show(VenomWindow* window) {
    if (!window || window->visible) return;
    window->visible = VENOM_TRUE;
    window->needs_redraw = VENOM_TRUE;
    
    if (window->on_event) {
        window->on_event(window, VENOM_WINDOW_LIFECYCLE_SHOWN, window->event_user_data);
    }
}

void venom_window_hide(VenomWindow* window) {
    if (!window || !window->visible) return;
    window->visible = VENOM_FALSE;
    
    if (window->on_event) {
        window->on_event(window, VENOM_WINDOW_LIFECYCLE_HIDDEN, window->event_user_data);
    }
}

void venom_window_close(VenomWindow* window) {
    if (!window || window->closed) return;
    
    if (window->on_event) {
        window->on_event(window, VENOM_WINDOW_LIFECYCLE_CLOSE_REQUESTED, window->event_user_data);
    }
    
    /* Unmap (hide) the X11 window immediately */
    if (window->xwindow && g_wm.display) {
        VenomX11DisplayInternal* x11 = (VenomX11DisplayInternal*)g_wm.display;
        XUnmapWindow(x11->xdisplay, window->xwindow);
        XFlush(x11->xdisplay);
    }
    
    window->closed = VENOM_TRUE;
    window->visible = VENOM_FALSE;
    
    if (window->on_event) {
        window->on_event(window, VENOM_WINDOW_LIFECYCLE_CLOSED, window->event_user_data);
    }
}

VenomBool venom_window_is_visible(const VenomWindow* window) {
    return window ? window->visible : VENOM_FALSE;
}

/* ============================================================================
 * WINDOW PROPERTIES
 * ============================================================================ */

void venom_window_set_title(VenomWindow* window, const char* title) {
    if (!window || !title) return;
    
    if (window->title) {
        venom_free(window->title, strlen(window->title) + 1);
    }
    
    VenomSize len = strlen(title) + 1;
    window->title = (char*)venom_alloc(len);
    if (window->title) {
        memcpy(window->title, title, len);
    }
    
    /* Update X11 window title */
    VenomX11DisplayInternal* x11 = (VenomX11DisplayInternal*)g_wm.display;
    XStoreName(x11->xdisplay, window->xwindow, title);
}

void venom_window_set_position(VenomWindow* window, VenomI32 x, VenomI32 y) {
    if (!window) return;
    window->x = x;
    window->y = y;
    
    VenomX11DisplayInternal* x11 = (VenomX11DisplayInternal*)g_wm.display;
    XMoveWindow(x11->xdisplay, window->xwindow, x, y);
}

void venom_window_get_position(const VenomWindow* window, VenomI32* x, VenomI32* y) {
    if (!window) return;
    if (x) *x = window->x;
    if (y) *y = window->y;
}

void venom_window_set_size(VenomWindow* window, VenomU32 width, VenomU32 height) {
    if (!window) return;
    window->width = width;
    window->height = height;
    window->needs_redraw = VENOM_TRUE;
    
    VenomX11DisplayInternal* x11 = (VenomX11DisplayInternal*)g_wm.display;
    XResizeWindow(x11->xdisplay, window->xwindow, width, height);
    
    /* Relayout root widget */
    if (window->root) {
        VenomRectF bounds = { 0, 0, (VenomF32)width, (VenomF32)height };
        venom_widget_layout(window->root, bounds);
    }
}

void venom_window_get_size(const VenomWindow* window, VenomU32* width, VenomU32* height) {
    if (!window) return;
    if (width) *width = window->width;
    if (height) *height = window->height;
}

void venom_window_set_root(VenomWindow* window, VenomWidget* root) {
    if (!window) return;
    
    if (window->root) {
        venom_unref(window->root);
    }
    
    window->root = root ? (VenomWidget*)venom_ref(root) : NULL;
    
    if (window->root) {
        VenomRectF bounds = { 0, 0, (VenomF32)window->width, (VenomF32)window->height };
        venom_widget_layout(window->root, bounds);
    }
    
    window->needs_redraw = VENOM_TRUE;
}

VenomWidget* venom_window_get_root(VenomWindow* window) {
    return window ? window->root : NULL;
}

void venom_window_set_background(VenomWindow* window, VenomColor color) {
    if (!window) return;
    window->background = color;
    window->needs_redraw = VENOM_TRUE;
}

void venom_window_invalidate(VenomWindow* window) {
    if (window) window->needs_redraw = VENOM_TRUE;
}

VenomU32 venom_window_get_id(const VenomWindow* window) {
    return window ? window->id : 0;
}

VenomBool venom_window_has_focus(const VenomWindow* window) {
    return window ? window->has_focus : VENOM_FALSE;
}

void venom_window_request_focus(VenomWindow* window) {
    if (!window) return;
    
    VenomX11DisplayInternal* x11 = (VenomX11DisplayInternal*)g_wm.display;
    XSetInputFocus(x11->xdisplay, window->xwindow, RevertToParent, CurrentTime);
    
    if (g_wm.focused && g_wm.focused != window) {
        g_wm.focused->has_focus = VENOM_FALSE;
        if (g_wm.focused->on_event) {
            g_wm.focused->on_event(g_wm.focused, VENOM_WINDOW_LIFECYCLE_FOCUS_LOST, 
                                    g_wm.focused->event_user_data);
        }
    }
    
    window->has_focus = VENOM_TRUE;
    g_wm.focused = window;
    
    if (window->on_event) {
        window->on_event(window, VENOM_WINDOW_LIFECYCLE_FOCUS_GAINED, window->event_user_data);
    }
}

/* ============================================================================
 * WINDOW MANAGER
 * ============================================================================ */

VenomResult venom_window_manager_init(void) {
    if (g_wm.initialized) {
        return VENOM_OK_UNIT();
    }
    
    /* Open display */
    VenomResultPtr display_result = venom_display_open(VENOM_BACKEND_X11, NULL);
    if (!display_result.ok) {
        return VENOM_ERR_UNIT(display_result.error);
    }
    
    g_wm.display = (VenomDisplay*)display_result.value;
    g_wm.initialized = VENOM_TRUE;
    g_wm.running = VENOM_FALSE;
    
    return VENOM_OK_UNIT();
}

void venom_window_manager_shutdown(void) {
    if (!g_wm.initialized) return;
    
    /* Close all windows */
    while (g_wm.window_count > 0) {
        venom_window_destroy(g_wm.windows[0]);
    }
    
    if (g_wm.display) {
        venom_display_close(g_wm.display);
        g_wm.display = NULL;
    }
    
    g_wm.initialized = VENOM_FALSE;
}

VenomU32 venom_window_count(void) {
    return g_wm.window_count;
}

VenomWindow* venom_window_at(VenomU32 index) {
    return index < g_wm.window_count ? g_wm.windows[index] : NULL;
}

VenomWindow* venom_window_get_focused(void) {
    return g_wm.focused;
}

/* ============================================================================
 * MAIN EVENT LOOP
 * ============================================================================ */

static VenomWindow* find_window_by_xwindow(Window xwin) {
    for (VenomU32 i = 0; i < g_wm.window_count; i++) {
        if (g_wm.windows[i]->xwindow == xwin) {
            return g_wm.windows[i];
        }
    }
    return NULL;
}

int venom_run(void) {
    if (!g_wm.initialized) {
        fprintf(stderr, "VENOMUI: Window manager not initialized\n");
        return 1;
    }
    
    g_wm.running = VENOM_TRUE;
    
    while (g_wm.running && g_wm.window_count > 0) {
        /* Draw all windows that need redraw */
        for (VenomU32 i = 0; i < g_wm.window_count; i++) {
            VenomWindow* win = g_wm.windows[i];
            if (win->visible && win->needs_redraw && win->canvas) {
                venom_canvas_clear(win->canvas, win->background);
                if (win->root) {
                    venom_widget_draw(win->root, win->canvas);
                }
                venom_canvas_flush(win->canvas);
                win->needs_redraw = VENOM_FALSE;
            }
        }
        
        /* Process events */
        VenomEvent event;
        while (venom_display_poll_event(g_wm.display, &event)) {
            /* Find target window - for now use focused */
            VenomWindow* target = g_wm.focused;
            if (!target && g_wm.window_count > 0) {
                target = g_wm.windows[0];
            }
            
            if (!target) continue;
            
            switch (event.type) {
                case VENOM_EVENT_WINDOW_CLOSE:
                    venom_window_close(target);
                    break;
                    
                case VENOM_EVENT_KEY_DOWN:
                    if (event.key.key == VENOM_KEY_ESCAPE) {
                        venom_window_close(target);
                    } else if (event.key.key == VENOM_KEY_TAB) {
                        /* Tab navigation */
                        if (event.key.modifiers & VENOM_KEYMOD_SHIFT) {
                            venom_focus_prev();
                        } else {
                            venom_focus_next();
                        }
                        target->needs_redraw = VENOM_TRUE;
                    } else if (event.key.key == VENOM_KEY_RETURN || 
                               event.key.key == VENOM_KEY_SPACE) {
                        /* Activate focused widget (simulate click) */
                        VenomWidget* focused = venom_focus_get();
                        if (focused) {
                            VenomEvent click = { .type = VENOM_EVENT_MOUSE_BUTTON_DOWN };
                            click.mouse.button = VENOM_MOUSE_BUTTON_LEFT;
                            venom_widget_dispatch_event(focused, &click);
                            click.type = VENOM_EVENT_MOUSE_BUTTON_UP;
                            venom_widget_dispatch_event(focused, &click);
                            target->needs_redraw = VENOM_TRUE;
                        }
                    }
                    break;
                    
                case VENOM_EVENT_WINDOW_EXPOSE:
                    target->needs_redraw = VENOM_TRUE;
                    break;
                    
                default:
                    break;
            }
            
            /* Dispatch to widget tree */
            if (target->root && venom_widget_dispatch_event(target->root, &event)) {
                target->needs_redraw = VENOM_TRUE;
            }
        }
        
        /* Remove closed windows */
        for (VenomU32 i = 0; i < g_wm.window_count; ) {
            if (g_wm.windows[i]->closed) {
                venom_window_destroy(g_wm.windows[i]);
            } else {
                i++;
            }
        }
        
        usleep(16000);  /* ~60fps */
    }
    
    g_wm.running = VENOM_FALSE;
    return 0;
}

void venom_quit(void) {
    g_wm.running = VENOM_FALSE;
}

VenomBool venom_is_running(void) {
    return g_wm.running;
}
