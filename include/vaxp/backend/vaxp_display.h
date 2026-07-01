/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_display.h - Abstract display/screen interface
 */

#ifndef VAXP_DISPLAY_H
#define VAXP_DISPLAY_H

#include "vaxp/core/vaxp_types.h"
#include "vaxp/core/vaxp_result.h"
#include "vaxp/core/vaxp_ref.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
typedef struct VaxpDisplay VaxpDisplay;
typedef struct VaxpWindow VaxpWindow;
typedef struct VaxpEvent VaxpEvent;

/* ============================================================================
 * DISPLAY BACKEND TYPE
 * ============================================================================ */

typedef enum VaxpBackendType {
    VAXP_BACKEND_X11,
    VAXP_BACKEND_WAYLAND,
    VAXP_BACKEND_AUTO,  /* Auto-detect best available */
} VaxpBackendType;

/* ============================================================================
 * DISPLAY OPERATIONS VTABLE
 * ============================================================================ */

typedef struct VaxpDisplayOps {
    /* Display lifecycle */
    void (*destroy)(VaxpDisplay* display);
    
    /* Screen info */
    VaxpI32 (*get_screen_count)(VaxpDisplay* display);
    VaxpSize2D (*get_screen_size)(VaxpDisplay* display, VaxpI32 screen);
    VaxpI32 (*get_default_screen)(VaxpDisplay* display);
    
    /* Event handling */
    VaxpBool (*poll_event)(VaxpDisplay* display, VaxpEvent* event_out);
    VaxpBool (*wait_event)(VaxpDisplay* display, VaxpEvent* event_out);
    void (*flush)(VaxpDisplay* display);
    
    /* Window creation (internal) */
    VaxpResultPtr (*create_window)(VaxpDisplay* display, const char* title,
                                     VaxpI32 x, VaxpI32 y,
                                     VaxpU32 width, VaxpU32 height);
    
    /* Typed window creation for desktop environments */
    VaxpResultPtr (*create_window_typed)(VaxpDisplay* display, 
                                          VaxpWindowType type,
                                          VaxpWindowPosition position,
                                          const char* title,
                                          VaxpU32 width, VaxpU32 height);
} VaxpDisplayOps;

/* ============================================================================
 * DISPLAY STRUCTURE
 * ============================================================================ */

struct VaxpDisplay {
    VAXP_REF_HEADER;
    
    const VaxpDisplayOps* ops;
    VaxpBackendType backend;
    
    /* Backend-specific data follows (accessed via cast) */
};

/* ============================================================================
 * DISPLAY PUBLIC API
 * ============================================================================ */

/**
 * @brief Open a connection to the display server
 * 
 * @param backend Which backend to use (or VAXP_BACKEND_AUTO)
 * @param display_name Display name (NULL for default, e.g., $DISPLAY on X11)
 * @return VaxpResultPtr containing VaxpDisplay* or error
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_display_open(VaxpBackendType backend, const char* VAXP_NULLABLE display_name);

/**
 * @brief Close the display connection
 * 
 * Equivalent to vaxp_unref(display).
 */
VAXP_INLINE void vaxp_display_close(VaxpDisplay* VAXP_NULLABLE display) {
    vaxp_unref(display);
}

/**
 * @brief Get number of screens/monitors
 */
VAXP_INLINE VaxpI32 vaxp_display_screen_count(VaxpDisplay* VAXP_NONNULL display) {
    return display->ops->get_screen_count(display);
}

/**
 * @brief Get size of a specific screen
 */
VAXP_INLINE VaxpSize2D vaxp_display_screen_size(VaxpDisplay* VAXP_NONNULL display, VaxpI32 screen) {
    return display->ops->get_screen_size(display, screen);
}

/**
 * @brief Get the default screen index
 */
VAXP_INLINE VaxpI32 vaxp_display_default_screen(VaxpDisplay* VAXP_NONNULL display) {
    return display->ops->get_default_screen(display);
}

/**
 * @brief Check for and retrieve the next event (non-blocking)
 * 
 * @return true if an event was available, false otherwise
 */
VAXP_INLINE VaxpBool vaxp_display_poll_event(VaxpDisplay* VAXP_NONNULL display, 
                                                  VaxpEvent* VAXP_NONNULL event_out) {
    return display->ops->poll_event(display, event_out);
}

/**
 * @brief Wait for the next event (blocking)
 * 
 * @return true if event received, false on error
 */
VAXP_INLINE VaxpBool vaxp_display_wait_event(VaxpDisplay* VAXP_NONNULL display,
                                                  VaxpEvent* VAXP_NONNULL event_out) {
    return display->ops->wait_event(display, event_out);
}

/**
 * @brief Flush pending requests to the display server
 */
VAXP_INLINE void vaxp_display_flush(VaxpDisplay* VAXP_NONNULL display) {
    display->ops->flush(display);
}

#ifdef __cplusplus
}
#endif

#endif /* VAXP_DISPLAY_H */
