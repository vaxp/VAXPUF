/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_app.c - Flutter-like simplified application API implementation
 */

#define _DEFAULT_SOURCE  /* For usleep */
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#include "venom/venomui.h"
#include "venom/backend/venom_display.h"
#include "venom/graphics/venom_canvas.h"

#include <X11/Xlib.h>

/* External canvas creation function */
extern VenomResultPtr venom_canvas_create_for_xlib(Display* display, Window window, 
                                                    Visual* visual, VenomU32 width, VenomU32 height);

/* X11 display structure (internal) */
typedef struct {
    VenomDisplay base;
    Display* xdisplay;
    int default_screen;
    Window root_window;
} VenomX11DisplayInternal;

/* ============================================================================
 * GLOBAL APP STATE
 * ============================================================================ */

static struct {
    VenomDisplay* display;
    VenomCanvas* canvas;
    VenomWidget* root;
    VenomBool needs_rebuild;
    VenomBool running;
    clock_t start_time;
    VenomU32 window_width;
    VenomU32 window_height;
} g_app = {0};

/* ============================================================================
 * WIDGET BUILDERS
 * ============================================================================ */

VenomWidget* _venom_text_build(const char* text, const VenomTextConfig* config) {
    VenomResultPtr result = venom_label_create(text);
    if (!result.ok) return NULL;
    
    VenomLabel* label = (VenomLabel*)result.value;
    
    if (config->size > 0) {
        venom_label_set_font_size(label, config->size);
    }
    if (config->color.a > 0) {
        venom_label_set_color(label, config->color);
    } else {
        venom_label_set_color(label, venom_color_rgb(50, 50, 50));
    }
    if (config->align != 0) {
        venom_label_set_align(label, config->align);
    }
    
    return (VenomWidget*)label;
}

VenomWidget* _venom_btn_build(const char* label, const VenomButtonConfig* config) {
    VenomResultPtr result = venom_button_create(label);
    if (!result.ok) return NULL;
    
    VenomButton* btn = (VenomButton*)result.value;
    
    /* Set colors */
    VenomColor primary = config->color.a > 0 ? config->color : VENOM_PRIMARY;
    VenomColor hover = venom_color_rgba(
        VENOM_MIN(primary.r + 20, 255),
        VENOM_MIN(primary.g + 20, 255),
        VENOM_MIN(primary.b + 20, 255),
        primary.a
    );
    VenomColor pressed = venom_color_rgba(
        VENOM_MAX(primary.r - 20, 0),
        VENOM_MAX(primary.g - 20, 0),
        VENOM_MAX(primary.b - 20, 0),
        primary.a
    );
    VenomColor text = config->text_color.a > 0 ? config->text_color : VENOM_COLOR_WHITE;
    
    venom_button_set_colors(btn, primary, hover, pressed, text);
    
    if (config->corner_radius > 0) {
        venom_button_set_corner_radius(btn, config->corner_radius);
    }
    
    if (config->on_click) {
        venom_button_set_on_click(btn, config->on_click, config->on_click_data);
    }
    
    return (VenomWidget*)btn;
}

static void add_children_to_container(VenomContainer* container, const VenomContainerConfig* config) {
    if (!config->children) return;
    
    /* Add children from NULL-terminated array */
    VenomWidget** child = config->children;
    while (*child != NULL) {
        venom_widget_add_child((VenomWidget*)container, *child);
        venom_unref(*child);  /* Container now owns it */
        child++;
    }
}

VenomWidget* _venom_col_build(const VenomContainerConfig* config) {
    VenomResultPtr result = venom_container_create_column();
    if (!result.ok) return NULL;
    
    VenomContainer* container = (VenomContainer*)result.value;
    
    if (config->gap > 0) {
        venom_container_set_gap(container, config->gap);
    }
    venom_container_set_justify(container, config->justify);
    venom_container_set_align(container, config->align);
    
    if (config->background.a > 0) {
        venom_container_set_background(container, config->background);
    }
    if (config->corner_radius > 0) {
        venom_container_set_corner_radius(container, config->corner_radius);
    }
    if (config->padding.top > 0 || config->padding.right > 0 || 
        config->padding.bottom > 0 || config->padding.left > 0) {
        ((VenomWidget*)container)->layout.padding = config->padding;
    }
    
    add_children_to_container(container, config);
    
    return (VenomWidget*)container;
}

VenomWidget* _venom_row_build(const VenomContainerConfig* config) {
    VenomResultPtr result = venom_container_create_row();
    if (!result.ok) return NULL;
    
    VenomContainer* container = (VenomContainer*)result.value;
    
    if (config->gap > 0) {
        venom_container_set_gap(container, config->gap);
    }
    venom_container_set_justify(container, config->justify);
    venom_container_set_align(container, config->align);
    
    if (config->background.a > 0) {
        venom_container_set_background(container, config->background);
    }
    if (config->corner_radius > 0) {
        venom_container_set_corner_radius(container, config->corner_radius);
    }
    if (config->padding.top > 0 || config->padding.right > 0 || 
        config->padding.bottom > 0 || config->padding.left > 0) {
        ((VenomWidget*)container)->layout.padding = config->padding;
    }
    
    add_children_to_container(container, config);
    
    return (VenomWidget*)container;
}

VenomWidget* venom_spacer(void) {
    VenomResultPtr result = venom_container_create();
    if (!result.ok) return NULL;
    
    VenomWidget* spacer = (VenomWidget*)result.value;
    spacer->layout.flex_grow = 1.0f;
    
    return spacer;
}

VenomWidget* venom_sized_box(VenomF32 width, VenomF32 height) {
    VenomResultPtr result = venom_container_create();
    if (!result.ok) return NULL;
    
    VenomWidget* box = (VenomWidget*)result.value;
    box->layout.preferred_width = width;
    box->layout.preferred_height = height;
    box->layout.min_width = width;
    box->layout.min_height = height;
    
    return box;
}

/* ============================================================================
 * STATE MANAGEMENT
 * ============================================================================ */

void venom_rebuild(void) {
    g_app.needs_rebuild = VENOM_TRUE;
}

VenomF64 venom_elapsed_time(void) {
    return (VenomF64)(clock() - g_app.start_time) / CLOCKS_PER_SEC;
}

/* ============================================================================
 * MAIN APP RUNNER
 * ============================================================================ */

int venom_run_app(const VenomAppConfig* config) {
    if (!config || !config->build) {
        fprintf(stderr, "VENOMUI: build function is required\n");
        return 1;
    }
    
    g_app.start_time = clock();
    
    /* Defaults */
    VenomU32 width = config->width > 0 ? config->width : 800;
    VenomU32 height = config->height > 0 ? config->height : 600;
    const char* title = config->title ? config->title : "VENOMUI App";
    VenomColor bg = config->background.a > 0 ? config->background : venom_color_rgb(250, 250, 252);
    
    g_app.window_width = width;
    g_app.window_height = height;
    
    if (config->debug) {
        printf("VENOMUI: Starting app '%s' (%ux%u)\n", title, width, height);
    }
    
    /* Initialize library */
    VenomResult init_result = venom_init();
    if (!init_result.ok) {
        fprintf(stderr, "VENOMUI: Failed to initialize: %s\n", 
                venom_error_string(init_result.error));
        return 1;
    }
    
    /* Open display */
    VenomResultPtr display_result = venom_display_open(VENOM_BACKEND_X11, NULL);
    if (!display_result.ok) {
        fprintf(stderr, "VENOMUI: Failed to open display: %s\n", 
                venom_error_string(display_result.error));
        return 1;
    }
    g_app.display = (VenomDisplay*)display_result.value;
    VenomX11DisplayInternal* x11 = (VenomX11DisplayInternal*)g_app.display;
    
    /* Create window */
    VenomResultPtr win_result = g_app.display->ops->create_window(
        g_app.display, title, 100, 100, width, height
    );
    if (!win_result.ok) {
        fprintf(stderr, "VENOMUI: Failed to create window: %s\n", 
                venom_error_string(win_result.error));
        venom_display_close(g_app.display);
        return 1;
    }
    Window xwindow = (Window)(uintptr_t)win_result.value;
    
    /* Create canvas */
    Visual* visual = DefaultVisual(x11->xdisplay, x11->default_screen);
    VenomResultPtr canvas_result = venom_canvas_create_for_xlib(
        x11->xdisplay, xwindow, visual, width, height
    );
    if (!canvas_result.ok) {
        fprintf(stderr, "VENOMUI: Failed to create canvas: %s\n",
                venom_error_string(canvas_result.error));
        venom_display_close(g_app.display);
        return 1;
    }
    g_app.canvas = (VenomCanvas*)canvas_result.value;
    
    /* Build initial widget tree */
    g_app.root = config->build(config->user_data);
    if (!g_app.root) {
        fprintf(stderr, "VENOMUI: build() returned NULL\n");
        venom_unref(g_app.canvas);
        venom_display_close(g_app.display);
        return 1;
    }
    
    /* Initial layout */
    VenomRectF bounds = { 0, 0, (VenomF32)width, (VenomF32)height };
    venom_widget_layout(g_app.root, bounds);
    
    /* Initialize focus system for keyboard navigation */
    venom_focus_init();
    venom_focus_set_root(g_app.root);
    
    if (config->debug) {
        printf("VENOMUI: Entering event loop\n");
        printf("VENOMUI: Use Tab to navigate, Enter/Space to activate buttons\n");
    }
    
    /* Event loop */
    g_app.running = VENOM_TRUE;
    VenomBool needs_redraw = VENOM_TRUE;
    
    while (g_app.running) {
        /* Handle rebuild request */
        if (g_app.needs_rebuild) {
            venom_unref(g_app.root);
            g_app.root = config->build(config->user_data);
            if (g_app.root) {
                venom_widget_layout(g_app.root, bounds);
            }
            g_app.needs_rebuild = VENOM_FALSE;
            needs_redraw = VENOM_TRUE;
        }
        
        /* Draw */
        if (needs_redraw && g_app.root) {
            venom_canvas_clear(g_app.canvas, bg);
            venom_widget_draw(g_app.root, g_app.canvas);
            venom_canvas_flush(g_app.canvas);
            needs_redraw = VENOM_FALSE;
        }
        
        /* Process events */
        VenomEvent event;
        while (venom_display_poll_event(g_app.display, &event)) {
            switch (event.type) {
                case VENOM_EVENT_WINDOW_CLOSE:
                    g_app.running = VENOM_FALSE;
                    break;
                    
                case VENOM_EVENT_KEY_DOWN:
                    if (event.key.key == VENOM_KEY_ESCAPE) {
                        g_app.running = VENOM_FALSE;
                    } else if (event.key.key == VENOM_KEY_TAB) {
                        /* Tab navigation between buttons */
                        if (event.key.modifiers & VENOM_KEYMOD_SHIFT) {
                            venom_focus_prev();
                        } else {
                            venom_focus_next();
                        }
                        needs_redraw = VENOM_TRUE;
                    } else if (event.key.key == VENOM_KEY_RETURN || 
                               event.key.key == VENOM_KEY_SPACE) {
                        /* Enter/Space activates focused widget */
                        VenomWidget* focused = venom_focus_get();
                        if (focused) {
                            VenomEvent click = { .type = VENOM_EVENT_MOUSE_BUTTON_DOWN };
                            click.mouse.button = VENOM_MOUSE_BUTTON_LEFT;
                            venom_widget_dispatch_event(focused, &click);
                            click.type = VENOM_EVENT_MOUSE_BUTTON_UP;
                            venom_widget_dispatch_event(focused, &click);
                            needs_redraw = VENOM_TRUE;
                        }
                    }
                    break;
                    
                case VENOM_EVENT_WINDOW_EXPOSE:
                    needs_redraw = VENOM_TRUE;
                    break;
                    
                default:
                    break;
            }
            
            /* Dispatch to widgets */
            if (g_app.root && venom_widget_dispatch_event(g_app.root, &event)) {
                needs_redraw = VENOM_TRUE;
            }
        }
        
        /* Small sleep */
        usleep(16000);  /* ~60fps */
    }
    
    if (config->debug) {
        printf("VENOMUI: Shutting down\n");
    }
    
    /* Cleanup */
    if (g_app.root) venom_unref(g_app.root);
    venom_unref(g_app.canvas);
    venom_display_close(g_app.display);
    venom_shutdown();
    
    return 0;
}
