/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_app.h - Flutter-like simplified application API
 * 
 * This header provides a high-level API that hides all the complexity
 * of display, window, and canvas management.
 */

#ifndef VENOM_APP_H
#define VENOM_APP_H

#include "venom/core/venom_types.h"
#include "venom/widgets/venom_widget.h"
#include "venom/widgets/venom_button.h"
#include "venom/widgets/venom_label.h"
#include "venom/widgets/venom_container.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * APPLICATION CONFIG
 * ============================================================================ */

/**
 * @brief Build function - creates the root widget tree
 * 
 * Called by venom_run_app() to build the UI.
 * Return the root widget of your application.
 */
typedef VenomWidget* (*VenomBuildFunc)(void* user_data);

/**
 * @brief Application configuration
 */
typedef struct VenomAppConfig {
    const char* title;          /* Window title */
    VenomU32 width;             /* Window width (0 = default 800) */
    VenomU32 height;            /* Window height (0 = default 600) */
    VenomColor background;      /* Background color */
    VenomBuildFunc build;       /* Build function to create root widget */
    void* user_data;            /* User data passed to build function */
    VenomBool resizable;        /* Allow window resizing */
    VenomBool debug;            /* Enable debug output */
} VenomAppConfig;

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
int venom_run_app(const VenomAppConfig* config);

/**
 * @brief Simplified app runner macro
 * 
 * Usage:
 *   return VENOM_RUN_APP("My App", build_my_ui);
 */
#define VENOM_RUN_APP(title, build_func) \
    venom_run_app(&(VenomAppConfig){ \
        .title = (title), \
        .build = (build_func), \
    })

/**
 * @brief Full app runner macro with all options
 * 
 * Usage:
 *   return VENOM_APP(.title = "My App", .width = 800, .build = build_ui);
 */
#define VENOM_APP(...) \
    venom_run_app(&(VenomAppConfig){ __VA_ARGS__ })

/* ============================================================================
 * WIDGET BUILDER MACROS
 * ============================================================================ */

/*
 * These macros provide a Flutter-like declarative syntax for building UI.
 * They create widgets with inline configuration.
 */

/* --------------- Text / Label --------------- */

typedef struct VenomTextConfig {
    VenomColor color;
    VenomF32 size;
    VenomTextAlign align;
} VenomTextConfig;

/**
 * @brief Create a text label
 * 
 * Usage:
 *   venom_text("Hello World")
 *   venom_text("Title", .size = 24, .color = VENOM_COLOR_BLUE)
 */
#define venom_text(text_str, ...) \
    _venom_text_build((text_str), &(VenomTextConfig){ .size = 14, __VA_ARGS__ })

VenomWidget* _venom_text_build(const char* text, const VenomTextConfig* config);

/* --------------- Button --------------- */

typedef struct VenomButtonConfig {
    VenomColor color;           /* Primary color */
    VenomColor text_color;
    VenomF32 corner_radius;
    VenomButtonCallback on_click;
    void* on_click_data;
} VenomButtonConfig;

/**
 * @brief Create a button
 * 
 * Usage:
 *   venom_btn("Click Me")
 *   venom_btn("Submit", .color = VENOM_GREEN, .on_click = handle_submit)
 */
#define venom_btn(label, ...) \
    _venom_btn_build((label), &(VenomButtonConfig){ .corner_radius = 6, __VA_ARGS__ })

VenomWidget* _venom_btn_build(const char* label, const VenomButtonConfig* config);

/* --------------- Containers --------------- */

typedef struct VenomContainerConfig {
    VenomF32 gap;
    VenomJustify justify;
    VenomAlign align;
    VenomColor background;
    VenomF32 corner_radius;
    VenomInsets padding;
    VenomWidget** children;     /* NULL-terminated array */
    VenomU32 child_count;       /* Or explicit count */
} VenomContainerConfig;

/**
 * @brief Create a column (vertical layout)
 * 
 * Usage:
 *   venom_col(.gap = 10, .children = VENOM_CHILDREN(child1, child2, child3))
 */
#define venom_col(...) \
    _venom_col_build(&(VenomContainerConfig){ __VA_ARGS__ })

/**
 * @brief Create a row (horizontal layout)
 * 
 * Usage:
 *   venom_row(.gap = 10, .children = VENOM_CHILDREN(btn1, btn2))
 */
#define venom_row(...) \
    _venom_row_build(&(VenomContainerConfig){ __VA_ARGS__ })

/**
 * @brief Create a centered container
 */
#define venom_center(...) \
    _venom_col_build(&(VenomContainerConfig){ \
        .justify = VENOM_JUSTIFY_CENTER, \
        .align = VENOM_ALIGN_CENTER, \
        __VA_ARGS__ \
    })

VenomWidget* _venom_col_build(const VenomContainerConfig* config);
VenomWidget* _venom_row_build(const VenomContainerConfig* config);

/**
 * @brief Helper macro for children array
 * 
 * Usage:
 *   .children = VENOM_CHILDREN(widget1, widget2, widget3)
 */
#define VENOM_CHILDREN(...) \
    (VenomWidget*[]){ __VA_ARGS__, NULL }, \
    .child_count = (sizeof((VenomWidget*[]){ __VA_ARGS__ }) / sizeof(VenomWidget*))

/* --------------- Spacer --------------- */

/**
 * @brief Create a flexible spacer
 */
VenomWidget* venom_spacer(void);

/**
 * @brief Create a fixed-size spacer
 */
VenomWidget* venom_sized_box(VenomF32 width, VenomF32 height);

/* ============================================================================
 * PREDEFINED COLORS
 * ============================================================================ */

#define VENOM_PRIMARY   venom_color_rgb(60, 120, 220)
#define VENOM_SUCCESS   venom_color_rgb(60, 180, 100)
#define VENOM_DANGER    venom_color_rgb(220, 70, 70)
#define VENOM_WARNING   venom_color_rgb(240, 180, 40)
#define VENOM_INFO      venom_color_rgb(80, 180, 220)
#define VENOM_DARK      venom_color_rgb(40, 44, 52)
#define VENOM_LIGHT     venom_color_rgb(248, 249, 250)
#define VENOM_MUTED     venom_color_rgb(108, 117, 125)

/* ============================================================================
 * STATE MANAGEMENT (Simple)
 * ============================================================================ */

/**
 * @brief Request a rebuild of the UI
 * 
 * Call this when state changes and UI needs to update.
 */
void venom_rebuild(void);

/**
 * @brief Get a const widget by key (for reuse across rebuilds)
 * 
 * During build(), call this to get a previously created const widget.
 * If found, returns the cached widget (you must ref it to use).
 * If not found, create a new widget and mark it as const.
 */
VenomWidget* venom_get_const_widget(const char* key);

/**
 * @brief Get elapsed time since app start (for animations)
 */
VenomF64 venom_elapsed_time(void);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_APP_H */
