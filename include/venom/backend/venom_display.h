/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_display.h - Abstract display/screen interface
 */

#ifndef VENOM_DISPLAY_H
#define VENOM_DISPLAY_H

#include "venom/core/venom_types.h"
#include "venom/core/venom_result.h"
#include "venom/core/venom_ref.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
typedef struct VenomDisplay VenomDisplay;
typedef struct VenomWindow VenomWindow;
typedef struct VenomEvent VenomEvent;

/* ============================================================================
 * DISPLAY BACKEND TYPE
 * ============================================================================ */

typedef enum VenomBackendType {
    VENOM_BACKEND_X11,
    VENOM_BACKEND_WAYLAND,
    VENOM_BACKEND_AUTO,  /* Auto-detect best available */
} VenomBackendType;

/* ============================================================================
 * DISPLAY OPERATIONS VTABLE
 * ============================================================================ */

typedef struct VenomDisplayOps {
    /* Display lifecycle */
    void (*destroy)(VenomDisplay* display);
    
    /* Screen info */
    VenomI32 (*get_screen_count)(VenomDisplay* display);
    VenomSize2D (*get_screen_size)(VenomDisplay* display, VenomI32 screen);
    VenomI32 (*get_default_screen)(VenomDisplay* display);
    
    /* Event handling */
    VenomBool (*poll_event)(VenomDisplay* display, VenomEvent* event_out);
    VenomBool (*wait_event)(VenomDisplay* display, VenomEvent* event_out);
    void (*flush)(VenomDisplay* display);
    
    /* Window creation (internal) */
    VenomResultPtr (*create_window)(VenomDisplay* display, const char* title,
                                     VenomI32 x, VenomI32 y,
                                     VenomU32 width, VenomU32 height);
} VenomDisplayOps;

/* ============================================================================
 * DISPLAY STRUCTURE
 * ============================================================================ */

struct VenomDisplay {
    VENOM_REF_HEADER;
    
    const VenomDisplayOps* ops;
    VenomBackendType backend;
    
    /* Backend-specific data follows (accessed via cast) */
};

/* ============================================================================
 * DISPLAY PUBLIC API
 * ============================================================================ */

/**
 * @brief Open a connection to the display server
 * 
 * @param backend Which backend to use (or VENOM_BACKEND_AUTO)
 * @param display_name Display name (NULL for default, e.g., $DISPLAY on X11)
 * @return VenomResultPtr containing VenomDisplay* or error
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_display_open(VenomBackendType backend, const char* VENOM_NULLABLE display_name);

/**
 * @brief Close the display connection
 * 
 * Equivalent to venom_unref(display).
 */
VENOM_INLINE void venom_display_close(VenomDisplay* VENOM_NULLABLE display) {
    venom_unref(display);
}

/**
 * @brief Get number of screens/monitors
 */
VENOM_INLINE VenomI32 venom_display_screen_count(VenomDisplay* VENOM_NONNULL display) {
    return display->ops->get_screen_count(display);
}

/**
 * @brief Get size of a specific screen
 */
VENOM_INLINE VenomSize2D venom_display_screen_size(VenomDisplay* VENOM_NONNULL display, VenomI32 screen) {
    return display->ops->get_screen_size(display, screen);
}

/**
 * @brief Get the default screen index
 */
VENOM_INLINE VenomI32 venom_display_default_screen(VenomDisplay* VENOM_NONNULL display) {
    return display->ops->get_default_screen(display);
}

/**
 * @brief Check for and retrieve the next event (non-blocking)
 * 
 * @return true if an event was available, false otherwise
 */
VENOM_INLINE VenomBool venom_display_poll_event(VenomDisplay* VENOM_NONNULL display, 
                                                  VenomEvent* VENOM_NONNULL event_out) {
    return display->ops->poll_event(display, event_out);
}

/**
 * @brief Wait for the next event (blocking)
 * 
 * @return true if event received, false on error
 */
VENOM_INLINE VenomBool venom_display_wait_event(VenomDisplay* VENOM_NONNULL display,
                                                  VenomEvent* VENOM_NONNULL event_out) {
    return display->ops->wait_event(display, event_out);
}

/**
 * @brief Flush pending requests to the display server
 */
VENOM_INLINE void venom_display_flush(VenomDisplay* VENOM_NONNULL display) {
    display->ops->flush(display);
}

#ifdef __cplusplus
}
#endif

#endif /* VENOM_DISPLAY_H */
