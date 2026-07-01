/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_window.h - Window abstraction for multi-window support
 * 
 * Each VaxpWindow represents a top-level OS window with its own
 * canvas, root widget, and event handling.
 */

#ifndef VAXP_WINDOW_H
#define VAXP_WINDOW_H

#include "vaxp/core/vaxp_types.h"
#include "vaxp/core/vaxp_ref.h"
#include "vaxp/core/vaxp_result.h"
#include "vaxp/widgets/vaxp_widget.h"
#include "vaxp/graphics/vaxp_canvas.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
typedef struct VaxpWindow VaxpWindow;
typedef struct VaxpWindowManager VaxpWindowManager;

/* ============================================================================
 * WINDOW FLAGS
 * ============================================================================ */

typedef enum VaxpWindowFlags {
    VAXP_WINDOW_NONE        = 0,
    VAXP_WINDOW_RESIZABLE   = (1 << 0),
    VAXP_WINDOW_BORDERLESS  = (1 << 1),
    VAXP_WINDOW_ALWAYS_TOP  = (1 << 2),
    VAXP_WINDOW_CENTERED    = (1 << 3),
    VAXP_WINDOW_FULLSCREEN  = (1 << 4),
    VAXP_WINDOW_HIDDEN      = (1 << 5),
} VaxpWindowFlags;

/* ============================================================================
 * WINDOW LIFECYCLE EVENTS
 * ============================================================================ */

typedef enum VaxpWindowLifecycleEvent {
    VAXP_WINDOW_LIFECYCLE_SHOWN,
    VAXP_WINDOW_LIFECYCLE_HIDDEN,
    VAXP_WINDOW_LIFECYCLE_MOVED,
    VAXP_WINDOW_LIFECYCLE_RESIZED,
    VAXP_WINDOW_LIFECYCLE_FOCUS_GAINED,
    VAXP_WINDOW_LIFECYCLE_FOCUS_LOST,
    VAXP_WINDOW_LIFECYCLE_CLOSE_REQUESTED,
    VAXP_WINDOW_LIFECYCLE_CLOSED,
} VaxpWindowLifecycleEvent;

/**
 * @brief Window lifecycle callback
 */
typedef void (*VaxpWindowCallback)(VaxpWindow* window, VaxpWindowLifecycleEvent event, void* user_data);

/* ============================================================================
 * WINDOW STRUCTURE
 * ============================================================================ */

/**
 * @brief Window configuration
 */
typedef struct VaxpWindowConfig {
    const char* title;
    VaxpI32 x;              /* Position (< 0 = default) */
    VaxpI32 y;
    VaxpU32 width;
    VaxpU32 height;
    VaxpU32 min_width;
    VaxpU32 min_height;
    VaxpU32 max_width;
    VaxpU32 max_height;
    VaxpWindowFlags flags;
    VaxpColor background;
    VaxpWidget* root;       /* Optional initial root widget */
    VaxpWindowCallback on_event;
    void* event_user_data;
} VaxpWindowConfig;

/* ============================================================================
 * WINDOW API
 * ============================================================================ */

/**
 * @brief Create a new window
 */
VaxpResultPtr vaxp_window_create(const VaxpWindowConfig* config);

/**
 * @brief Convenience: create window with just title and size
 */
VaxpResultPtr vaxp_window_create_simple(const char* title, VaxpU32 width, VaxpU32 height);

/**
 * @brief Destroy window
 */
void vaxp_window_destroy(VaxpWindow* window);

/**
 * @brief Show window
 */
void vaxp_window_show(VaxpWindow* window);

/**
 * @brief Hide window
 */
void vaxp_window_hide(VaxpWindow* window);

/**
 * @brief Close window (triggers close event, then destroys)
 */
void vaxp_window_close(VaxpWindow* window);

/**
 * @brief Check if window is visible
 */
VaxpBool vaxp_window_is_visible(const VaxpWindow* window);

/**
 * @brief Set window title
 */
void vaxp_window_set_title(VaxpWindow* window, const char* title);

/**
 * @brief Set window position
 */
void vaxp_window_set_position(VaxpWindow* window, VaxpI32 x, VaxpI32 y);

/**
 * @brief Get window position
 */
void vaxp_window_get_position(const VaxpWindow* window, VaxpI32* x, VaxpI32* y);

/**
 * @brief Set window size
 */
void vaxp_window_set_size(VaxpWindow* window, VaxpU32 width, VaxpU32 height);

/**
 * @brief Get window size
 */
void vaxp_window_get_size(const VaxpWindow* window, VaxpU32* width, VaxpU32* height);

/**
 * @brief Set root widget for the window
 */
void vaxp_window_set_root(VaxpWindow* window, VaxpWidget* root);

/**
 * @brief Get root widget
 */
VaxpWidget* vaxp_window_get_root(VaxpWindow* window);

/**
 * @brief Set background color
 */
void vaxp_window_set_background(VaxpWindow* window, VaxpColor color);

/**
 * @brief Request window redraw
 */
void vaxp_window_invalidate(VaxpWindow* window);

/**
 * @brief Get window ID (for internal use)
 */
VaxpU32 vaxp_window_get_id(const VaxpWindow* window);

/**
 * @brief Check if window has focus
 */
VaxpBool vaxp_window_has_focus(const VaxpWindow* window);

/**
 * @brief Request focus for window
 */
void vaxp_window_request_focus(VaxpWindow* window);

/* ============================================================================
 * WINDOW MANAGER
 * ============================================================================ */

/**
 * @brief Initialize window manager (called by vaxp_init)
 */
VaxpResult vaxp_window_manager_init(void);

/**
 * @brief Shutdown window manager
 */
void vaxp_window_manager_shutdown(void);

/**
 * @brief Get window count
 */
VaxpU32 vaxp_window_count(void);

/**
 * @brief Get window by index
 */
VaxpWindow* vaxp_window_at(VaxpU32 index);

/**
 * @brief Get focused window
 */
VaxpWindow* vaxp_window_get_focused(void);

/**
 * @brief Run main event loop for all windows
 * 
 * Returns when all windows are closed or vaxp_quit() is called.
 */
int vaxp_run(void);

/**
 * @brief Request application quit
 */
void vaxp_quit(void);

/**
 * @brief Check if app is running
 */
VaxpBool vaxp_is_running(void);

/* ============================================================================
 * CONVENIENCE MACROS
 * ============================================================================ */

/**
 * @brief Create window with config struct literal
 */
#define VAXP_WINDOW(...) \
    vaxp_window_create(&(VaxpWindowConfig){ \
        .width = 800, .height = 600, \
        .flags = VAXP_WINDOW_RESIZABLE | VAXP_WINDOW_CENTERED, \
        __VA_ARGS__ \
    })

#ifdef __cplusplus
}
#endif

#endif /* VAXP_WINDOW_H */
