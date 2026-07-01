/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_app.c - Flutter-like simplified application API implementation
 */

#define _DEFAULT_SOURCE  /* For usleep */
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#include "vaxp/vaxpui.h"
#include "vaxp/backend/vaxp_display.h"
#include "vaxp/graphics/vaxp_canvas.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>   /* XIconifyWindow */

#ifdef VAXP_USE_OPENGL
extern VaxpResultPtr vaxp_canvas_create_opengl(Display* display, Window window, 
                                               VaxpU32 width, VaxpU32 height);
#else
/* External canvas creation function */
extern VaxpResultPtr vaxp_canvas_create_for_xlib(Display* display, Window window, 
                                                    Visual* visual, VaxpU32 width, VaxpU32 height);
#endif

/* X11 display structure (internal) */
typedef struct {
    VaxpDisplay base;
    Display* xdisplay;
    int default_screen;
    Window root_window;
} VaxpX11DisplayInternal;

/* ============================================================================
 * GLOBAL APP STATE
 * ============================================================================ */

static struct {
    VaxpDisplay* display;
    VaxpCanvas* canvas;
    VaxpWidget* root;
    VaxpBool needs_rebuild;
    VaxpBool running;
    clock_t start_time;
    VaxpU32 window_width;
    VaxpU32 window_height;
    /* X11 window handle (needed by decoration callbacks) */
    Window xwindow;
    VaxpBool is_maximized;   /* current maximize state */
    
    /* Const Widget Registry */
    VaxpWidget** const_widgets;
    VaxpU32 const_count;
    VaxpU32 const_capacity;
} g_app = {0};


/* ============================================================================
 * DECORATION BAR CALLBACKS
 * ============================================================================ */

static void _dec_close_cb(VaxpButton* b, void* data) {
    (void)b; (void)data;
    g_app.running = VAXP_FALSE;
}

static void _dec_minimize_cb(VaxpButton* b, void* data) {
    (void)b; (void)data;
    if (!g_app.display || !g_app.xwindow) return;
    VaxpX11DisplayInternal* x11 = (VaxpX11DisplayInternal*)g_app.display;
    XIconifyWindow(x11->xdisplay, g_app.xwindow, x11->default_screen);
    XFlush(x11->xdisplay);
}

static void _dec_maximize_cb(VaxpButton* b, void* data) {
    (void)b; (void)data;
    if (!g_app.display || !g_app.xwindow) return;
    VaxpX11DisplayInternal* x11 = (VaxpX11DisplayInternal*)g_app.display;
    /* Toggle maximize via _NET_WM_STATE EWMH message */
    Atom wm_state = XInternAtom(x11->xdisplay, "_NET_WM_STATE", False);
    Atom max_h    = XInternAtom(x11->xdisplay, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
    Atom max_v    = XInternAtom(x11->xdisplay, "_NET_WM_STATE_MAXIMIZED_VERT", False);
    XEvent xev;
    xev.type                  = ClientMessage;
    xev.xclient.window        = g_app.xwindow;
    xev.xclient.message_type  = wm_state;
    xev.xclient.format        = 32;
    xev.xclient.data.l[0]     = 2; /* _NET_WM_STATE_TOGGLE */
    xev.xclient.data.l[1]     = (long)max_h;
    xev.xclient.data.l[2]     = (long)max_v;
    xev.xclient.data.l[3]     = 1; /* source: normal application */
    xev.xclient.data.l[4]     = 0;
    XSendEvent(x11->xdisplay, x11->root_window, False,
               SubstructureRedirectMask | SubstructureNotifyMask, &xev);
    XFlush(x11->xdisplay);
    g_app.is_maximized = !g_app.is_maximized;
}

#define _DEC_DOT_SIZE  14.0f

/**
 * @brief Create a circular button dot (VaxpButton styled as a macOS traffic-light circle).
 * The button locks its own size so the flex container cannot stretch it.
 */
static VaxpWidget* _dec_make_dot(VaxpColor color, VaxpColor hover_color,
                                  VaxpButtonCallback cb) {
    VaxpWidget* dot = _vaxp_btn_build(
        "",  /* empty label */
        &(VaxpButtonConfig){
            .color         = color,
            .text_color    = vaxp_color_rgba(0, 0, 0, 0),
            .corner_radius = _DEC_DOT_SIZE / 2.0f,
            .on_click      = cb
        }
    );
    if (!dot) return NULL;
    /* Lock to a perfect square so flex cannot stretch it */
    dot->layout.min_width        = _DEC_DOT_SIZE;
    dot->layout.min_height       = _DEC_DOT_SIZE;
    dot->layout.preferred_width  = _DEC_DOT_SIZE;
    dot->layout.preferred_height = _DEC_DOT_SIZE;
    dot->layout.max_width        = _DEC_DOT_SIZE;
    dot->layout.max_height       = _DEC_DOT_SIZE;
    dot->layout.flex_grow        = 0;
    dot->layout.flex_shrink      = 0;
    dot->layout.align_self       = VAXP_ALIGN_CENTER;
    /* Tweak hover/pressed colors */
    VaxpButton* btn = (VaxpButton*)dot;
    btn->bg_hover_color   = hover_color;
    btn->bg_pressed_color = hover_color;
    return dot;
}

/**
 * @brief Wraps user_root with a decoration bar that exactly matches
 *        the calculator_demo title bar style:
 *
 *   padding={8, 10, 8, 25}  JUSTIFY_SPACE_BETWEEN  ALIGN_CENTER
 *   ┌───────────────────────────────────────────────────┐
 *   │  Window Title               [yellow][green][red]  │
 *   ├───────────────────────────────────────────────────┤
 *   │                user content (flex)                │
 *   └───────────────────────────────────────────────────┘
 *
 * - No separate bar background (inherits window background → blends in).
 * - Dots are VaxpButton for hover + click support, locked 14×14.
 * - Order matches calculator_demo: yellow | green | red (left → right).
 * - DARK style: white title text.  DEFAULT/LIGHT: dark title text.
 *
 * Ownership: user_root ref CONSUMED; returns new widget (ref=1).
 * vaxp_widget_add_child() refs internally → call vaxp_unref() after each add.
 */
static VaxpWidget* _vaxp_build_decorated_root(VaxpWidget* user_root,
                                               const char* title,
                                               VaxpDecorationStyle style) {
    if (!user_root || style == VAXP_DECORATION_NONE) return user_root;

    /* ── Title color (no bar background — inherits window bg) ───────── */
    VaxpColor title_col = (style == VAXP_DECORATION_DARK)
        ? vaxp_color_rgb(255, 255, 255)   /* white text on dark bg  */
        : vaxp_color_rgb(255, 255, 255);  /* white text (matches calculator) */

    /* ── Three dots — same colors/order as calculator_demo ─────────── */
    /*    calculator order: yellow, green, red  (left → right)          */
    VaxpWidget* dot_y = _dec_make_dot(
        vaxp_color_rgb(255, 189, 46),
        vaxp_color_rgb(235, 165, 20),
        _dec_minimize_cb
    );
    VaxpWidget* dot_g = _dec_make_dot(
        vaxp_color_rgb(40,  200, 64),
        vaxp_color_rgb(15,  175, 40),
        _dec_maximize_cb
    );
    VaxpWidget* dot_r = _dec_make_dot(
        vaxp_color_rgb(255, 95,  87),
        vaxp_color_rgb(235, 65,  55),
        _dec_close_cb
    );

    /* Dots row: gap=8, ALIGN_CENTER so dots don't stretch */
    VaxpResultPtr dr_res = vaxp_container_create_row();
    VaxpWidget* dots_row = NULL;
    if (dr_res.ok) {
        VaxpContainer* dr = (VaxpContainer*)dr_res.value;
        vaxp_container_set_gap(dr, 8);
        vaxp_container_set_align(dr, VAXP_ALIGN_CENTER);
        ((VaxpWidget*)dr)->layout.flex_grow   = 0;
        ((VaxpWidget*)dr)->layout.flex_shrink = 0;
        ((VaxpWidget*)dr)->layout.align_self  = VAXP_ALIGN_CENTER;
        if (dot_y) { vaxp_widget_add_child((VaxpWidget*)dr, dot_y); vaxp_unref(dot_y); }
        if (dot_g) { vaxp_widget_add_child((VaxpWidget*)dr, dot_g); vaxp_unref(dot_g); }
        if (dot_r) { vaxp_widget_add_child((VaxpWidget*)dr, dot_r); vaxp_unref(dot_r); }
        dots_row = (VaxpWidget*)dr;
    }

    /* ── Title label (left side) ────────────────────────────────────── */
    VaxpWidget* title_lbl = _vaxp_text_build(
        title ? title : "",
        &(VaxpTextConfig){ .size = 20, .color = title_col }
    );
    if (title_lbl) {
        title_lbl->layout.align_self  = VAXP_ALIGN_CENTER;
        title_lbl->layout.flex_grow   = 0;
        title_lbl->layout.flex_shrink = 0;
    }

    /* ── Bar row: same as calculator_demo ───────────────────────────── */
    /*    padding={8,10,8,25}  JUSTIFY_SPACE_BETWEEN  ALIGN_CENTER      */
    /*    No background → inherits from outer column / window bg.       */
    VaxpResultPtr br_res = vaxp_container_create_row();
    VaxpWidget* dec_row = NULL;
    if (br_res.ok) {
        VaxpContainer* br = (VaxpContainer*)br_res.value;
        vaxp_container_set_justify(br, VAXP_JUSTIFY_SPACE_BETWEEN);
        vaxp_container_set_align(br, VAXP_ALIGN_CENTER);
        /* No vaxp_container_set_background → transparent, matches calculator */
        ((VaxpWidget*)br)->layout.padding    = (VaxpInsets){8, 10, 8, 25};
        ((VaxpWidget*)br)->layout.flex_grow  = 0;
        ((VaxpWidget*)br)->layout.flex_shrink = 0;
        /* Left: title  |  Right: dots */
        if (title_lbl) { vaxp_widget_add_child((VaxpWidget*)br, title_lbl); vaxp_unref(title_lbl); }
        if (dots_row)  { vaxp_widget_add_child((VaxpWidget*)br, dots_row);  vaxp_unref(dots_row);  }
        dec_row = (VaxpWidget*)br;
    }

    /* ── User content fills remaining height ────────────────────────── */
    user_root->layout.flex_grow   = 1;
    user_root->layout.flex_shrink = 1;

    /* ── Outer column: [bar | content] ─────────────────────────────── */
    VaxpResultPtr oc_res = vaxp_container_create_column();
    VaxpWidget* outer = NULL;
    if (oc_res.ok) {
        VaxpContainer* oc = (VaxpContainer*)oc_res.value;
        if (dec_row)   { vaxp_widget_add_child((VaxpWidget*)oc, dec_row);   vaxp_unref(dec_row);   }
        if (user_root) { vaxp_widget_add_child((VaxpWidget*)oc, user_root); vaxp_unref(user_root); }
        outer = (VaxpWidget*)oc;
    }

    return outer;
}


/* ============================================================================
 * CONST WIDGET REGISTRY
 * ============================================================================ */

/**
 * @brief Recursively collect const widgets before rebuild
 */
static void collect_const_widgets(VaxpWidget* widget) {
    if (!widget) return;
    
    if (widget->is_const && widget->const_key) {
        /* Grow registry if needed */
        if (g_app.const_count >= g_app.const_capacity) {
            VaxpU32 new_cap = g_app.const_capacity ? g_app.const_capacity * 2 : 16;
            VaxpWidget** new_arr = vaxp_realloc(g_app.const_widgets, 
                                                   g_app.const_capacity * sizeof(VaxpWidget*),
                                                   new_cap * sizeof(VaxpWidget*));
            if (new_arr) {
                g_app.const_widgets = new_arr;
                g_app.const_capacity = new_cap;
            }
        }
        
        if (g_app.const_count < g_app.const_capacity) {
            vaxp_ref(widget);  /* Keep alive during rebuild */
            g_app.const_widgets[g_app.const_count++] = widget;
        }
    }
    
    /* Process children */
    for (VaxpU32 i = 0; i < widget->children_count; i++) {
        collect_const_widgets(widget->children[i]);
    }
}

/**
 * @brief Find a const widget by key
 */
static VaxpWidget* find_const_widget(const char* key) {
    if (!key) return NULL;
    
    for (VaxpU32 i = 0; i < g_app.const_count; i++) {
        const char* widget_key = g_app.const_widgets[i]->const_key;
        if (widget_key && strcmp(widget_key, key) == 0) {
            return g_app.const_widgets[i];
        }
    }
    return NULL;
}

/**
 * @brief Release all stored const widgets
 */
static void release_const_widgets(void) {
    for (VaxpU32 i = 0; i < g_app.const_count; i++) {
        vaxp_unref(g_app.const_widgets[i]);
    }
    g_app.const_count = 0;
}

/**
 * @brief Free const registry memory
 */
static void free_const_registry(void) {
    release_const_widgets();
    if (g_app.const_widgets) {
        vaxp_free(g_app.const_widgets, g_app.const_capacity * sizeof(VaxpWidget*));
        g_app.const_widgets = NULL;
        g_app.const_capacity = 0;
    }
}

/**
 * @brief Recursively check if any widget in the tree needs redraw
 * 
 * This is essential for animations - child widgets can set needs_redraw
 * in their draw() function to request continuous updates.
 */
static VaxpBool any_widget_needs_redraw(VaxpWidget* widget) {
    if (!widget) return VAXP_FALSE;
    
    /* Check this widget */
    if (widget->needs_redraw) {
        return VAXP_TRUE;
    }
    
    /* Check children recursively */
    for (VaxpU32 i = 0; i < widget->children_count; i++) {
        if (any_widget_needs_redraw(widget->children[i])) {
            return VAXP_TRUE;
        }
    }
    
    return VAXP_FALSE;
}

/* ============================================================================
 * WIDGET BUILDERS
 * ============================================================================ */

VaxpWidget* _vaxp_text_build(const char* text, const VaxpTextConfig* config) {
    VaxpResultPtr result = vaxp_label_create(text);
    if (!result.ok) return NULL;
    
    VaxpLabel* label = (VaxpLabel*)result.value;
    
    if (config->font_family) {
        vaxp_label_set_font_family(label, config->font_family);
    }
    if (config->size > 0) {
        vaxp_label_set_font_size(label, config->size);
    }
    if (config->color.a > 0) {
        vaxp_label_set_color(label, config->color);
    } else {
        vaxp_label_set_color(label, vaxp_color_rgb(50, 50, 50));
    }
    if (config->align != 0) {
        vaxp_label_set_align(label, config->align);
    }
    
    return (VaxpWidget*)label;
}

VaxpWidget* _vaxp_btn_build(const char* label, const VaxpButtonConfig* config) {
    VaxpResultPtr result = vaxp_button_create(label);
    if (!result.ok) return NULL;
    
    VaxpButton* btn = (VaxpButton*)result.value;
    
    if (config->font_family) {
        vaxp_button_set_font_family(btn, config->font_family);
    }
    if (config->size > 0) {
        btn->font_size = config->size; /* Alternatively add vaxp_button_set_font_size */
    }
    
    VaxpColor primary = config->color.a > 0 ? config->color : VAXP_PRIMARY;
    VaxpColor hover = vaxp_color_rgba(
        VAXP_MIN(primary.r + 20, 255),
        VAXP_MIN(primary.g + 20, 255),
        VAXP_MIN(primary.b + 20, 255),
        primary.a
    );
    VaxpColor pressed = vaxp_color_rgba(
        VAXP_MAX(primary.r - 20, 0),
        VAXP_MAX(primary.g - 20, 0),
        VAXP_MAX(primary.b - 20, 0),
        primary.a
    );
    VaxpColor text = config->text_color.a > 0 ? config->text_color : VAXP_COLOR_WHITE;
    
    vaxp_button_set_colors(btn, primary, hover, pressed, text);
    
    if (config->corner_radius > 0) {
        vaxp_button_set_corner_radius(btn, config->corner_radius);
    }
    
    if (config->on_click) {
        vaxp_button_set_on_click(btn, config->on_click, config->on_click_data);
    }
    
    return (VaxpWidget*)btn;
}

static void add_children_to_container(VaxpContainer* container, const VaxpContainerConfig* config) {
    if (!config->children) return;
    
    /* Add children from NULL-terminated array */
    VaxpWidget** child = config->children;
    while (*child != NULL) {
        vaxp_widget_add_child((VaxpWidget*)container, *child);
        vaxp_unref(*child);  /* Container now owns it */
        child++;
    }
}

VaxpWidget* _vaxp_col_build(const VaxpContainerConfig* config) {
    VaxpResultPtr result = vaxp_container_create_column();
    if (!result.ok) return NULL;
    
    VaxpContainer* container = (VaxpContainer*)result.value;
    
    if (config->gap > 0) {
        vaxp_container_set_gap(container, config->gap);
    }
    vaxp_container_set_justify(container, config->justify);
    vaxp_container_set_align(container, config->align);
    
    if (config->background.a > 0) {
        vaxp_container_set_background(container, config->background);
    }
    if (config->corner_radius > 0) {
        vaxp_container_set_corner_radius(container, config->corner_radius);
    }
    if (config->padding.top > 0 || config->padding.right > 0 || 
        config->padding.bottom > 0 || config->padding.left > 0) {
        ((VaxpWidget*)container)->layout.padding = config->padding;
    }
    
    add_children_to_container(container, config);
    
    return (VaxpWidget*)container;
}

VaxpWidget* _vaxp_row_build(const VaxpContainerConfig* config) {
    VaxpResultPtr result = vaxp_container_create_row();
    if (!result.ok) return NULL;
    
    VaxpContainer* container = (VaxpContainer*)result.value;
    
    if (config->gap > 0) {
        vaxp_container_set_gap(container, config->gap);
    }
    vaxp_container_set_justify(container, config->justify);
    
    /* Default row align to STRETCH so children fill cross-axis */
    VaxpAlign align = config->align;
    if (align == VAXP_ALIGN_START && config->align == 0) {
        align = VAXP_ALIGN_STRETCH;  /* Better default for row */
    }
    vaxp_container_set_align(container, align);
    
    if (config->background.a > 0) {
        vaxp_container_set_background(container, config->background);
    }
    if (config->corner_radius > 0) {
        vaxp_container_set_corner_radius(container, config->corner_radius);
    }
    if (config->padding.top > 0 || config->padding.right > 0 || 
        config->padding.bottom > 0 || config->padding.left > 0) {
        ((VaxpWidget*)container)->layout.padding = config->padding;
    }
    
    add_children_to_container(container, config);
    
    return (VaxpWidget*)container;
}

VaxpWidget* vaxp_spacer(void) {
    VaxpResultPtr result = vaxp_container_create();
    if (!result.ok) return NULL;
    
    VaxpWidget* spacer = (VaxpWidget*)result.value;
    spacer->layout.flex_grow = 1.0f;
    
    return spacer;
}

VaxpWidget* vaxp_sized_box(VaxpF32 width, VaxpF32 height) {
    VaxpResultPtr result = vaxp_container_create();
    if (!result.ok) return NULL;
    
    VaxpWidget* box = (VaxpWidget*)result.value;
    box->layout.preferred_width = width;
    box->layout.preferred_height = height;
    box->layout.min_width = width;
    box->layout.min_height = height;
    
    return box;
}

/* ============================================================================
 * STATE MANAGEMENT
 * ============================================================================ */

void vaxp_rebuild(void) {
    g_app.needs_rebuild = VAXP_TRUE;
}

VaxpWidget* vaxp_get_const_widget(const char* key) {
    return find_const_widget(key);
}

VaxpWidget* _vaxp_const_get_or_create(const char* key, VaxpWidget* new_widget) {
    /* Try to find existing const widget */
    VaxpWidget* existing = find_const_widget(key);
    if (existing) {
        /* Found! Ref it and destroy the newly created one */
        if (new_widget) {
            vaxp_unref(new_widget);
        }
        return vaxp_ref(existing);
    }
    
    /* Not found - add to permanent registry */
    if (new_widget) {
        vaxp_widget_set_const(new_widget, VAXP_TRUE);
        vaxp_widget_set_const_key(new_widget, key);
        
        /* Add to registry (keep reference) */
        if (g_app.const_count >= g_app.const_capacity) {
            VaxpU32 new_cap = g_app.const_capacity ? g_app.const_capacity * 2 : 16;
            VaxpWidget** new_arr = vaxp_realloc(g_app.const_widgets, 
                                                   g_app.const_capacity * sizeof(VaxpWidget*),
                                                   new_cap * sizeof(VaxpWidget*));
            if (new_arr) {
                g_app.const_widgets = new_arr;
                g_app.const_capacity = new_cap;
            }
        }
        
        if (g_app.const_count < g_app.const_capacity) {
            vaxp_ref(new_widget);  /* Registry owns a reference */
            g_app.const_widgets[g_app.const_count++] = new_widget;
        }
    }
    return new_widget;
}

VaxpF64 vaxp_elapsed_time(void) {
    return (VaxpF64)(clock() - g_app.start_time) / CLOCKS_PER_SEC;
}

/* ============================================================================
 * MAIN APP RUNNER
 * ============================================================================ */

int vaxp_run_app(const VaxpAppConfig* config) {
    if (!config || !config->build) {
        fprintf(stderr, "VAXPUI: build function is required\n");
        return 1;
    }
    
    g_app.start_time = clock();
    
    /* Defaults */
    VaxpU32 width = config->width > 0 ? config->width : 800;
    VaxpU32 height = config->height > 0 ? config->height : 600;
    const char* title = config->title ? config->title : "VAXPUI App";
    VaxpColor bg = config->background.a > 0 ? config->background : vaxp_color_rgba(0, 0, 0, 77);
    
    g_app.window_width = width;
    g_app.window_height = height;
    
    if (config->debug) {
        printf("VAXPUI: Starting app '%s' (%ux%u)\n", title, width, height);
    }
    
    /* Initialize library */
    VaxpResult init_result = vaxp_init();
    if (!init_result.ok) {
        fprintf(stderr, "VAXPUI: Failed to initialize: %s\n", 
                vaxp_error_string(init_result.error));
        return 1;
    }
    
    /* Open display */
    VaxpResultPtr display_result = vaxp_display_open(VAXP_BACKEND_X11, NULL);
    if (!display_result.ok) {
        fprintf(stderr, "VAXPUI: Failed to open display: %s\n", 
                vaxp_error_string(display_result.error));
        return 1;
    }
    g_app.display = (VaxpDisplay*)display_result.value;
    VaxpX11DisplayInternal* x11 = (VaxpX11DisplayInternal*)g_app.display;
    
    /* Create window - use typed if special window type */
    VaxpResultPtr win_result;
    if (config->window_type != VAXP_WINDOW_NORMAL && 
        g_app.display->ops->create_window_typed) {
        win_result = g_app.display->ops->create_window_typed(
            g_app.display, config->window_type, config->position,
            title, width, height
        );
    } else {
        win_result = g_app.display->ops->create_window(
            g_app.display, title, 100, 100, width, height
        );
    }
    if (!win_result.ok) {
        fprintf(stderr, "VAXPUI: Failed to create window: %s\n", 
                vaxp_error_string(win_result.error));
        vaxp_display_close(g_app.display);
        return 1;
    }
    Window xwindow = (Window)(uintptr_t)win_result.value;
    g_app.xwindow = xwindow;   /* expose to decoration callbacks */
    
    /* Get actual window size (typed windows may have different size) */
    if (config->window_type != VAXP_WINDOW_NORMAL) {
        /* Wait for window to be mapped and get actual size */
        XSync(x11->xdisplay, False);
        XWindowAttributes attrs;
        if (XGetWindowAttributes(x11->xdisplay, xwindow, &attrs)) {
            width = (VaxpU32)attrs.width;
            height = (VaxpU32)attrs.height;
            g_app.window_width = width;
            g_app.window_height = height;
            if (config->debug) {
                printf("VAXPUI: Actual window size: %ux%u\n", width, height);
            }
        }
    }
    
    /* Create canvas */
#ifdef VAXP_USE_OPENGL
    VaxpResultPtr canvas_result = vaxp_canvas_create_opengl(
        x11->xdisplay, xwindow, width, height
    );
#else
    Visual* visual = DefaultVisual(x11->xdisplay, x11->default_screen);
    VaxpResultPtr canvas_result = vaxp_canvas_create_for_xlib(
        x11->xdisplay, xwindow, visual, width, height
    );
#endif
    if (!canvas_result.ok) {
        fprintf(stderr, "VAXPUI: Failed to create canvas: %s\n",
                vaxp_error_string(canvas_result.error));
        vaxp_display_close(g_app.display);
        return 1;
    }
    g_app.canvas = (VaxpCanvas*)canvas_result.value;
    
    /* Build initial widget tree */
    g_app.root = config->build(config->user_data);
    if (!g_app.root) {
        fprintf(stderr, "VAXPUI: build() returned NULL\n");
        vaxp_unref(g_app.canvas);
        vaxp_display_close(g_app.display);
        return 1;
    }
    
    /* Wrap with decoration bar if requested */
    if (config->decoration != VAXP_DECORATION_NONE) {
        g_app.root = _vaxp_build_decorated_root(g_app.root, title, config->decoration);
        if (!g_app.root) {
            fprintf(stderr, "VAXPUI: Failed to build decoration\n");
            vaxp_unref(g_app.canvas);
            vaxp_display_close(g_app.display);
            return 1;
        }
    }
    
    /* Initial layout */
    VaxpRectF bounds = { 0, 0, (VaxpF32)width, (VaxpF32)height };
    vaxp_widget_layout(g_app.root, bounds);
    
    /* Initialize focus system for keyboard navigation */
    vaxp_focus_init();
    vaxp_focus_set_root(g_app.root);
    
    if (config->debug) {
        printf("VAXPUI: Entering event loop\n");
        printf("VAXPUI: Use Tab to navigate, Enter/Space to activate buttons\n");
    }
    
    /* Event loop */
    g_app.running = VAXP_TRUE;
    VaxpBool needs_redraw = VAXP_TRUE;
    
    while (g_app.running) {
        /* Handle rebuild request */
        if (g_app.needs_rebuild) {
            vaxp_unref(g_app.root);
            g_app.root = config->build(config->user_data);
            /* Re-wrap with decoration on rebuild */
            if (g_app.root && config->decoration != VAXP_DECORATION_NONE) {
                g_app.root = _vaxp_build_decorated_root(g_app.root, title, config->decoration);
            }
            if (g_app.root) {
                vaxp_widget_layout(g_app.root, bounds);
                vaxp_focus_set_root(g_app.root);
            } else {
                vaxp_focus_clear();
            }
            g_app.needs_rebuild = VAXP_FALSE;
            needs_redraw = VAXP_TRUE;
        }
        
        /* Draw every frame */
        if (g_app.root) {
            vaxp_canvas_clear(g_app.canvas, bg);
            vaxp_widget_draw(g_app.root, g_app.canvas); // رسم دائم!
            vaxp_canvas_flush(g_app.canvas);
        }
        
        /* Process events */
        VaxpEvent event;
        while (vaxp_display_poll_event(g_app.display, &event)) {
            switch (event.type) {
                case VAXP_EVENT_WINDOW_CLOSE:
                    g_app.running = VAXP_FALSE;
                    break;
                    
                case VAXP_EVENT_KEY_DOWN:
                    if (event.key.key == VAXP_KEY_ESCAPE) {
                        g_app.running = VAXP_FALSE;
                    } else if (event.key.key == VAXP_KEY_TAB) {
                        /* Tab navigation between buttons */
                        if (event.key.modifiers & VAXP_KEYMOD_SHIFT) {
                            vaxp_focus_prev();
                        } else {
                            vaxp_focus_next();
                        }
                        needs_redraw = VAXP_TRUE;
                    } else if (event.key.key == VAXP_KEY_RETURN || 
                               event.key.key == VAXP_KEY_SPACE) {
                        /* Enter/Space activates focused widget */
                        VaxpWidget* focused = vaxp_focus_get();
                        if (focused) {
                            VaxpEvent click = { .type = VAXP_EVENT_MOUSE_BUTTON_DOWN };
                            click.mouse.button = VAXP_MOUSE_BUTTON_LEFT;
                            vaxp_widget_dispatch_event(focused, &click);
                            click.type = VAXP_EVENT_MOUSE_BUTTON_UP;
                            vaxp_widget_dispatch_event(focused, &click);
                            needs_redraw = VAXP_TRUE;
                        }
                    }
                    break;
                    
                case VAXP_EVENT_WINDOW_RESIZE:
                    if (g_app.window_width != event.window.width || g_app.window_height != event.window.height) {
                        g_app.window_width = event.window.width;
                        g_app.window_height = event.window.height;
                        bounds.width = (VaxpF32)event.window.width;
                        bounds.height = (VaxpF32)event.window.height;
                        if (g_app.canvas) {
                            g_app.canvas->width = event.window.width;
                            g_app.canvas->height = event.window.height;
                        }
                        if (g_app.root) {
                            vaxp_widget_layout(g_app.root, bounds);
                        }
                        needs_redraw = VAXP_TRUE;
                    }
                    break;
                    
                case VAXP_EVENT_WINDOW_EXPOSE:
                    needs_redraw = VAXP_TRUE;
                    break;
                    
                default:
                    break;
            }
            
            /* Dispatch to widgets */
            if (g_app.root && vaxp_widget_dispatch_event(g_app.root, &event)) {
                needs_redraw = VAXP_TRUE;
            }
        }
        
        /* Small sleep (~1000fps or V-Sync) */
        usleep(1000);
    }
    
    if (config->debug) {
        printf("VAXPUI: Shutting down\n");
    }
    
    /* Cleanup */
    if (g_app.root) vaxp_unref(g_app.root);
    free_const_registry();  /* Free const widget registry */
    vaxp_unref(g_app.canvas);
    vaxp_display_close(g_app.display);
    vaxp_shutdown();
    
    return 0;
}
