/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_app.h - Flutter-like simplified application API
 * 
 * This header provides a high-level API that hides all the complexity
 * of display, window, and canvas management.
 */

#ifndef VAXP_APP_H
#define VAXP_APP_H

#include "vaxp/core/vaxp_types.h"
#include "vaxp/widgets/vaxp_widget.h"
#include "vaxp/widgets/vaxp_button.h"
#include "vaxp/widgets/vaxp_label.h"
#include "vaxp/widgets/vaxp_container.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * APPLICATION CONFIG
 * ============================================================================ */

/**
 * @brief Build function - creates the root widget tree
 * 
 * Called by vaxp_run_app() to build the UI.
 * Return the root widget of your application.
 */
typedef VaxpWidget* (*VaxpBuildFunc)(void* user_data);

/**
 * @brief Application configuration
 */
typedef struct VaxpAppConfig {
    const char* title;          /* Window title */
    VaxpU32 width;             /* Window width (0 = default 800) */
    VaxpU32 height;            /* Window height (0 = default 600) */
    VaxpColor background;      /* Background color */
    VaxpBuildFunc build;       /* Build function to create root widget */
    void* user_data;            /* User data passed to build function */
    VaxpBool resizable;        /* Allow window resizing */
    VaxpBool debug;            /* Enable debug output */
    VaxpWindowType window_type;     /* Window type (default: NORMAL) */
    VaxpWindowPosition position;    /* Window position hint */
} VaxpAppConfig;

/**
 * @brief Run the application
 * 
 * This is the main entry point. It handles:
 * - Display connection
 * - Window creation
 * - Canvas setup
 * - Event loop
 * - Cleanup
 * 
 * @param config Application configuration
 * @return Exit code (0 = success)
 */
int vaxp_run_app(const VaxpAppConfig* config);

/**
 * @brief Simplified app runner macro
 * 
 * Usage:
 *   return VAXP_RUN_APP("My App", build_my_ui);
 */
#define VAXP_RUN_APP(title, build_func) \
    vaxp_run_app(&(VaxpAppConfig){ \
        .title = (title), \
        .build = (build_func), \
    })

/**
 * @brief Full app runner macro with all options
 * 
 * Usage:
 *   return VAXP_APP(.title = "My App", .width = 800, .build = build_ui);
 */
#define VAXP_APP(...) \
    vaxp_run_app(&(VaxpAppConfig){ __VA_ARGS__ })

/**
 * @brief Panel app macro - creates a top dock window
 */
#define VAXP_PANEL_APP(...) \
    vaxp_run_app(&(VaxpAppConfig){ \
        .window_type = VAXP_WINDOW_PANEL, \
        .position = VAXP_POSITION_TOP, \
        __VA_ARGS__ \
    })

/**
 * @brief Dock app macro - creates a bottom dock window
 */
#define VAXP_DOCK_APP(...) \
    vaxp_run_app(&(VaxpAppConfig){ \
        .window_type = VAXP_WINDOW_DOCK, \
        .position = VAXP_POSITION_BOTTOM, \
        __VA_ARGS__ \
    })

/**
 * @brief Popup app macro - creates a popup window (control center, etc.)
 */
#define VAXP_POPUP_APP(...) \
    vaxp_run_app(&(VaxpAppConfig){ \
        .window_type = VAXP_WINDOW_POPUP, \
        __VA_ARGS__ \
    })

/**
 * @brief Launcher app macro - creates a fullscreen overlay
 */
#define VAXP_LAUNCHER_APP(...) \
    vaxp_run_app(&(VaxpAppConfig){ \
        .window_type = VAXP_WINDOW_LAUNCHER, \
        .position = VAXP_POSITION_FULLSCREEN, \
        __VA_ARGS__ \
    })

/* ============================================================================
 * WIDGET BUILDER MACROS
 * ============================================================================ */

/*
 * These macros provide a Flutter-like declarative syntax for building UI.
 * They create widgets with inline configuration.
 */

/* --------------- Text / Label --------------- */

typedef struct VaxpTextConfig {
    const char* font_family;
    VaxpColor color;
    VaxpF32 size;
    VaxpTextAlign align;
} VaxpTextConfig;

/**
 * @brief Create a text label
 * 
 * Usage:
 *   vaxp_text("Hello World")
 *   vaxp_text("Title", .size = 24, .color = VAXP_COLOR_BLUE)
 */
#define vaxp_text(text_str, ...) \
    _vaxp_text_build((text_str), &(VaxpTextConfig){ .size = 14, __VA_ARGS__ })

VaxpWidget* _vaxp_text_build(const char* text, const VaxpTextConfig* config);

/* --------------- Button --------------- */

typedef struct VaxpButtonConfig {
    const char* font_family;
    VaxpF32 size;
    VaxpColor color;           /* Primary color */
    VaxpColor text_color;
    VaxpF32 corner_radius;
    VaxpButtonCallback on_click;
    void* on_click_data;
} VaxpButtonConfig;

/**
 * @brief Create a button
 * 
 * Usage:
 *   vaxp_btn("Click Me")
 *   vaxp_btn("Submit", .color = VAXP_GREEN, .on_click = handle_submit)
 */
#define vaxp_btn(label, ...) \
    _vaxp_btn_build((label), &(VaxpButtonConfig){ .corner_radius = 6, __VA_ARGS__ })

VaxpWidget* _vaxp_btn_build(const char* label, const VaxpButtonConfig* config);

/* --------------- Containers --------------- */

typedef struct VaxpContainerConfig {
    VaxpF32 gap;
    VaxpJustify justify;
    VaxpAlign align;
    VaxpColor background;
    VaxpF32 corner_radius;
    VaxpInsets padding;
    VaxpWidget** children;     /* NULL-terminated array */
    VaxpU32 child_count;       /* Or explicit count */
} VaxpContainerConfig;

/**
 * @brief Create a column (vertical layout)
 * 
 * Usage:
 *   vaxp_col(.gap = 10, .children = VAXP_CHILDREN(child1, child2, child3))
 */
#define vaxp_col(...) \
    _vaxp_col_build(&(VaxpContainerConfig){ __VA_ARGS__ })

/**
 * @brief Create a row (horizontal layout)
 * 
 * Usage:
 *   vaxp_row(.gap = 10, .children = VAXP_CHILDREN(btn1, btn2))
 */
#define vaxp_row(...) \
    _vaxp_row_build(&(VaxpContainerConfig){ __VA_ARGS__ })

/**
 * @brief Create a centered container
 */
#define vaxp_center(...) \
    _vaxp_col_build(&(VaxpContainerConfig){ \
        .justify = VAXP_JUSTIFY_CENTER, \
        .align = VAXP_ALIGN_CENTER, \
        __VA_ARGS__ \
    })

VaxpWidget* _vaxp_col_build(const VaxpContainerConfig* config);
VaxpWidget* _vaxp_row_build(const VaxpContainerConfig* config);

/**
 * @brief Helper macro for children array
 * 
 * Usage:
 *   .children = VAXP_CHILDREN(widget1, widget2, widget3)
 */
#define VAXP_CHILDREN(...) \
    (VaxpWidget*[]){ __VA_ARGS__, NULL }, \
    .child_count = (sizeof((VaxpWidget*[]){ __VA_ARGS__ }) / sizeof(VaxpWidget*))

/* --------------- Spacer --------------- */

/**
 * @brief Create a flexible spacer
 */
VaxpWidget* vaxp_spacer(void);

/**
 * @brief Create a fixed-size spacer
 */
VaxpWidget* vaxp_sized_box(VaxpF32 width, VaxpF32 height);

/* ============================================================================
 * PREDEFINED COLORS
 * ============================================================================ */

#define VAXP_PRIMARY   vaxp_color_rgb(60, 120, 220)
#define VAXP_SUCCESS   vaxp_color_rgb(60, 180, 100)
#define VAXP_DANGER    vaxp_color_rgb(220, 70, 70)
#define VAXP_WARNING   vaxp_color_rgb(240, 180, 40)
#define VAXP_INFO      vaxp_color_rgb(80, 180, 220)
#define VAXP_DARK      vaxp_color_rgb(40, 44, 52)
#define VAXP_LIGHT     vaxp_color_rgb(248, 249, 250)
#define VAXP_MUTED     vaxp_color_rgb(108, 117, 125)

/* ============================================================================
 * STATE MANAGEMENT (Simple)
 * ============================================================================ */

/**
 * @brief Request a rebuild of the UI
 * 
 * Call this when state changes and UI needs to update.
 */
void vaxp_rebuild(void);

/**
 * @brief Get a const widget by key (for reuse across rebuilds)
 * 
 * During build(), call this to get a previously created const widget.
 * If found, returns the cached widget (you must ref it to use).
 * If not found, create a new widget and mark it as const.
 */
VaxpWidget* vaxp_get_const_widget(const char* key);

/**
 * @brief Get elapsed time since app start (for animations)
 */
VaxpF64 vaxp_elapsed_time(void);

/* ============================================================================
 * CONST WIDGETS (Simple Flutter-like syntax)
 * ============================================================================ */

/**
 * @brief Internal helper to get or create a const widget
 */
VaxpWidget* _vaxp_const_get_or_create(const char* key, VaxpWidget* new_widget);

/**
 * @brief Mark a widget as const (won't be recreated during rebuilds)
 * 
 * Usage:
 *   vaxp_col(.children = VAXP_CHILDREN(
 *       VAXP_CONST(vaxp_text("Static Title")),   // Cached
 *       VAXP_CONST(vaxp_btn("Fixed Button")),    // Cached
 *       vaxp_text("Dynamic: %d", count)            // Recreated
 *   ))
 */
#define _VAXP_STRINGIFY(x) #x
#define _VAXP_TOSTRING(x) _VAXP_STRINGIFY(x)
#define VAXP_CONST(widget_expr) \
    _vaxp_const_get_or_create(__FILE__ ":" _VAXP_TOSTRING(__LINE__), (widget_expr))

#ifdef __cplusplus
}
#endif

#endif /* VAXP_APP_H */
