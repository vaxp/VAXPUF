/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_window.h - Window abstraction for multi-window support
 * 
 * Each VenomWindow represents a top-level OS window with its own
 * canvas, root widget, and event handling.
 */

#ifndef VENOM_WINDOW_H
#define VENOM_WINDOW_H

#include "venom/core/venom_types.h"
#include "venom/core/venom_ref.h"
#include "venom/core/venom_result.h"
#include "venom/widgets/venom_widget.h"
#include "venom/graphics/venom_canvas.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
typedef struct VenomWindow VenomWindow;
typedef struct VenomWindowManager VenomWindowManager;

/* ============================================================================
 * WINDOW FLAGS
 * ============================================================================ */

typedef enum VenomWindowFlags {
    VENOM_WINDOW_NONE        = 0,
    VENOM_WINDOW_RESIZABLE   = (1 << 0),
    VENOM_WINDOW_BORDERLESS  = (1 << 1),
    VENOM_WINDOW_ALWAYS_TOP  = (1 << 2),
    VENOM_WINDOW_CENTERED    = (1 << 3),
    VENOM_WINDOW_FULLSCREEN  = (1 << 4),
    VENOM_WINDOW_HIDDEN      = (1 << 5),
} VenomWindowFlags;

/* ============================================================================
 * WINDOW LIFECYCLE EVENTS
 * ============================================================================ */

typedef enum VenomWindowLifecycleEvent {
    VENOM_WINDOW_LIFECYCLE_SHOWN,
    VENOM_WINDOW_LIFECYCLE_HIDDEN,
    VENOM_WINDOW_LIFECYCLE_MOVED,
    VENOM_WINDOW_LIFECYCLE_RESIZED,
    VENOM_WINDOW_LIFECYCLE_FOCUS_GAINED,
    VENOM_WINDOW_LIFECYCLE_FOCUS_LOST,
    VENOM_WINDOW_LIFECYCLE_CLOSE_REQUESTED,
    VENOM_WINDOW_LIFECYCLE_CLOSED,
} VenomWindowLifecycleEvent;

/**
 * @brief Window lifecycle callback
 */
typedef void (*VenomWindowCallback)(VenomWindow* window, VenomWindowLifecycleEvent event, void* user_data);

/* ============================================================================
 * WINDOW STRUCTURE
 * ============================================================================ */

/**
 * @brief Window configuration
 */
typedef struct VenomWindowConfig {
    const char* title;
    VenomI32 x;              /* Position (< 0 = default) */
    VenomI32 y;
    VenomU32 width;
    VenomU32 height;
    VenomU32 min_width;
    VenomU32 min_height;
    VenomU32 max_width;
    VenomU32 max_height;
    VenomWindowFlags flags;
    VenomColor background;
    VenomWidget* root;       /* Optional initial root widget */
    VenomWindowCallback on_event;
    void* event_user_data;
} VenomWindowConfig;

/* ============================================================================
 * WINDOW API
 * ============================================================================ */

/**
 * @brief Create a new window
 */
VenomResultPtr venom_window_create(const VenomWindowConfig* config);

/**
 * @brief Convenience: create window with just title and size
 */
VenomResultPtr venom_window_create_simple(const char* title, VenomU32 width, VenomU32 height);

/**
 * @brief Destroy window
 */
void venom_window_destroy(VenomWindow* window);

/**
 * @brief Show window
 */
void venom_window_show(VenomWindow* window);

/**
 * @brief Hide window
 */
void venom_window_hide(VenomWindow* window);

/**
 * @brief Close window (triggers close event, then destroys)
 */
void venom_window_close(VenomWindow* window);

/**
 * @brief Check if window is visible
 */
VenomBool venom_window_is_visible(const VenomWindow* window);

/**
 * @brief Set window title
 */
void venom_window_set_title(VenomWindow* window, const char* title);

/**
 * @brief Set window position
 */
void venom_window_set_position(VenomWindow* window, VenomI32 x, VenomI32 y);

/**
 * @brief Get window position
 */
void venom_window_get_position(const VenomWindow* window, VenomI32* x, VenomI32* y);

/**
 * @brief Set window size
 */
void venom_window_set_size(VenomWindow* window, VenomU32 width, VenomU32 height);

/**
 * @brief Get window size
 */
void venom_window_get_size(const VenomWindow* window, VenomU32* width, VenomU32* height);

/**
 * @brief Set root widget for the window
 */
void venom_window_set_root(VenomWindow* window, VenomWidget* root);

/**
 * @brief Get root widget
 */
VenomWidget* venom_window_get_root(VenomWindow* window);

/**
 * @brief Set background color
 */
void venom_window_set_background(VenomWindow* window, VenomColor color);

/**
 * @brief Request window redraw
 */
void venom_window_invalidate(VenomWindow* window);

/**
 * @brief Get window ID (for internal use)
 */
VenomU32 venom_window_get_id(const VenomWindow* window);

/**
 * @brief Check if window has focus
 */
VenomBool venom_window_has_focus(const VenomWindow* window);

/**
 * @brief Request focus for window
 */
void venom_window_request_focus(VenomWindow* window);

/* ============================================================================
 * WINDOW MANAGER
 * ============================================================================ */

/**
 * @brief Initialize window manager (called by venom_init)
 */
VenomResult venom_window_manager_init(void);

/**
 * @brief Shutdown window manager
 */
void venom_window_manager_shutdown(void);

/**
 * @brief Get window count
 */
VenomU32 venom_window_count(void);

/**
 * @brief Get window by index
 */
VenomWindow* venom_window_at(VenomU32 index);

/**
 * @brief Get focused window
 */
VenomWindow* venom_window_get_focused(void);

/**
 * @brief Run main event loop for all windows
 * 
 * Returns when all windows are closed or venom_quit() is called.
 */
int venom_run(void);

/**
 * @brief Request application quit
 */
void venom_quit(void);

/**
 * @brief Check if app is running
 */
VenomBool venom_is_running(void);

/* ============================================================================
 * CONVENIENCE MACROS
 * ============================================================================ */

/**
 * @brief Create window with config struct literal
 */
#define VENOM_WINDOW(...) \
    venom_window_create(&(VenomWindowConfig){ \
        .width = 800, .height = 600, \
        .flags = VENOM_WINDOW_RESIZABLE | VENOM_WINDOW_CENTERED, \
        __VA_ARGS__ \
    })

#ifdef __cplusplus
}
#endif

#endif /* VENOM_WINDOW_H */
