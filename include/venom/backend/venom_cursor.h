/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_cursor.h - Cursor Management API
 * 
 * Provides cursor types and functions for changing mouse cursor appearance.
 */

#ifndef VENOM_CURSOR_H
#define VENOM_CURSOR_H

#include "venom/core/venom_types.h"
#include "venom/core/venom_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * CURSOR TYPES
 * ============================================================================ */

/**
 * @brief Standard cursor types
 * 
 * These map to X11 standard cursors from X11/cursorfont.h
 */
typedef enum VenomCursorType {
    VENOM_CURSOR_DEFAULT = 0,   /**< Default arrow cursor */
    VENOM_CURSOR_ARROW,         /**< Arrow cursor (same as default) */
    VENOM_CURSOR_HAND,          /**< Pointing hand (for links) */
    VENOM_CURSOR_TEXT,          /**< I-beam text cursor */
    VENOM_CURSOR_WAIT,          /**< Busy/wait cursor */
    VENOM_CURSOR_HELP,          /**< Help cursor (arrow with ?) */
    VENOM_CURSOR_CROSSHAIR,     /**< Crosshair */
    VENOM_CURSOR_MOVE,          /**< Move cursor (4-directional arrows) */
    VENOM_CURSOR_NOT_ALLOWED,   /**< Not allowed / prohibited */
    
    /* Resize cursors */
    VENOM_CURSOR_RESIZE_N,      /**< North resize (vertical top) */
    VENOM_CURSOR_RESIZE_S,      /**< South resize (vertical bottom) */
    VENOM_CURSOR_RESIZE_E,      /**< East resize (horizontal right) */
    VENOM_CURSOR_RESIZE_W,      /**< West resize (horizontal left) */
    VENOM_CURSOR_RESIZE_NE,     /**< Northeast resize (diagonal) */
    VENOM_CURSOR_RESIZE_NW,     /**< Northwest resize (diagonal) */
    VENOM_CURSOR_RESIZE_SE,     /**< Southeast resize (diagonal) */
    VENOM_CURSOR_RESIZE_SW,     /**< Southwest resize (diagonal) */
    VENOM_CURSOR_RESIZE_EW,     /**< East-West resize (horizontal) */
    VENOM_CURSOR_RESIZE_NS,     /**< North-South resize (vertical) */
    VENOM_CURSOR_RESIZE_NESW,   /**< NE-SW diagonal resize */
    VENOM_CURSOR_RESIZE_NWSE,   /**< NW-SE diagonal resize */
    
    /* Drag cursors */
    VENOM_CURSOR_GRAB,          /**< Open hand for draggable */
    VENOM_CURSOR_GRABBING,      /**< Closed hand when dragging */
    
    /* DND cursors */
    VENOM_CURSOR_DND_COPY,      /**< DND copy action */
    VENOM_CURSOR_DND_MOVE,      /**< DND move action */
    VENOM_CURSOR_DND_LINK,      /**< DND link action */
    VENOM_CURSOR_DND_NO_DROP,   /**< DND no drop allowed */
    
    VENOM_CURSOR_NONE,          /**< Hidden cursor */
    
    VENOM_CURSOR_COUNT          /**< Number of cursor types */
} VenomCursorType;

/* ============================================================================
 * CURSOR API
 * ============================================================================ */

/**
 * @brief Set the cursor for a window
 * 
 * Changes the mouse cursor when it's over the specified window.
 * 
 * @param window_id Window ID
 * @param cursor Cursor type to set
 * @return VenomResult Success or error
 */
VenomResult venom_cursor_set(VenomU32 window_id, VenomCursorType cursor);

/**
 * @brief Reset cursor to default for a window
 * 
 * @param window_id Window ID
 */
void venom_cursor_reset(VenomU32 window_id);

/**
 * @brief Get current cursor type for a window
 * 
 * @param window_id Window ID
 * @return Current cursor type
 */
VenomCursorType venom_cursor_get(VenomU32 window_id);

/**
 * @brief Hide the cursor for a window
 * 
 * @param window_id Window ID
 */
void venom_cursor_hide(VenomU32 window_id);

/**
 * @brief Show the cursor for a window (if hidden)
 * 
 * @param window_id Window ID
 */
void venom_cursor_show(VenomU32 window_id);

/* ============================================================================
 * INTERNAL API
 * ============================================================================ */

/**
 * @brief Initialize cursor subsystem
 * 
 * @param display_handle Platform display handle
 * @return VenomResult Success or error
 */
VenomResult venom_cursor_init(void* display_handle);

/**
 * @brief Shutdown cursor subsystem
 */
void venom_cursor_shutdown(void);

/**
 * @brief Register a window for cursor management
 * 
 * @param window_id VENOMUI window ID
 * @param native_window Native window handle (X11 Window)
 */
VenomResult venom_cursor_register_window(VenomU32 window_id, void* native_window);

/**
 * @brief Unregister a window from cursor management
 */
void venom_cursor_unregister_window(VenomU32 window_id);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_CURSOR_H */
