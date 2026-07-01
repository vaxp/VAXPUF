/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_cursor.h - Cursor Management API
 * 
 * Provides cursor types and functions for changing mouse cursor appearance.
 */

#ifndef VAXP_CURSOR_H
#define VAXP_CURSOR_H

#include "vaxp/core/vaxp_types.h"
#include "vaxp/core/vaxp_result.h"

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
typedef enum VaxpCursorType {
    VAXP_CURSOR_DEFAULT = 0,   /**< Default arrow cursor */
    VAXP_CURSOR_ARROW,         /**< Arrow cursor (same as default) */
    VAXP_CURSOR_HAND,          /**< Pointing hand (for links) */
    VAXP_CURSOR_TEXT,          /**< I-beam text cursor */
    VAXP_CURSOR_WAIT,          /**< Busy/wait cursor */
    VAXP_CURSOR_HELP,          /**< Help cursor (arrow with ?) */
    VAXP_CURSOR_CROSSHAIR,     /**< Crosshair */
    VAXP_CURSOR_MOVE,          /**< Move cursor (4-directional arrows) */
    VAXP_CURSOR_NOT_ALLOWED,   /**< Not allowed / prohibited */
    
    /* Resize cursors */
    VAXP_CURSOR_RESIZE_N,      /**< North resize (vertical top) */
    VAXP_CURSOR_RESIZE_S,      /**< South resize (vertical bottom) */
    VAXP_CURSOR_RESIZE_E,      /**< East resize (horizontal right) */
    VAXP_CURSOR_RESIZE_W,      /**< West resize (horizontal left) */
    VAXP_CURSOR_RESIZE_NE,     /**< Northeast resize (diagonal) */
    VAXP_CURSOR_RESIZE_NW,     /**< Northwest resize (diagonal) */
    VAXP_CURSOR_RESIZE_SE,     /**< Southeast resize (diagonal) */
    VAXP_CURSOR_RESIZE_SW,     /**< Southwest resize (diagonal) */
    VAXP_CURSOR_RESIZE_EW,     /**< East-West resize (horizontal) */
    VAXP_CURSOR_RESIZE_NS,     /**< North-South resize (vertical) */
    VAXP_CURSOR_RESIZE_NESW,   /**< NE-SW diagonal resize */
    VAXP_CURSOR_RESIZE_NWSE,   /**< NW-SE diagonal resize */
    
    /* Drag cursors */
    VAXP_CURSOR_GRAB,          /**< Open hand for draggable */
    VAXP_CURSOR_GRABBING,      /**< Closed hand when dragging */
    
    /* DND cursors */
    VAXP_CURSOR_DND_COPY,      /**< DND copy action */
    VAXP_CURSOR_DND_MOVE,      /**< DND move action */
    VAXP_CURSOR_DND_LINK,      /**< DND link action */
    VAXP_CURSOR_DND_NO_DROP,   /**< DND no drop allowed */
    
    VAXP_CURSOR_NONE,          /**< Hidden cursor */
    
    VAXP_CURSOR_COUNT          /**< Number of cursor types */
} VaxpCursorType;

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
 * @return VaxpResult Success or error
 */
VaxpResult vaxp_cursor_set(VaxpU32 window_id, VaxpCursorType cursor);

/**
 * @brief Reset cursor to default for a window
 * 
 * @param window_id Window ID
 */
void vaxp_cursor_reset(VaxpU32 window_id);

/**
 * @brief Get current cursor type for a window
 * 
 * @param window_id Window ID
 * @return Current cursor type
 */
VaxpCursorType vaxp_cursor_get(VaxpU32 window_id);

/**
 * @brief Hide the cursor for a window
 * 
 * @param window_id Window ID
 */
void vaxp_cursor_hide(VaxpU32 window_id);

/**
 * @brief Show the cursor for a window (if hidden)
 * 
 * @param window_id Window ID
 */
void vaxp_cursor_show(VaxpU32 window_id);

/* ============================================================================
 * INTERNAL API
 * ============================================================================ */

/**
 * @brief Initialize cursor subsystem
 * 
 * @param display_handle Platform display handle
 * @return VaxpResult Success or error
 */
VaxpResult vaxp_cursor_init(void* display_handle);

/**
 * @brief Shutdown cursor subsystem
 */
void vaxp_cursor_shutdown(void);

/**
 * @brief Register a window for cursor management
 * 
 * @param window_id VAXPUI window ID
 * @param native_window Native window handle (X11 Window)
 */
VaxpResult vaxp_cursor_register_window(VaxpU32 window_id, void* native_window);

/**
 * @brief Unregister a window from cursor management
 */
void vaxp_cursor_unregister_window(VaxpU32 window_id);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_CURSOR_H */
