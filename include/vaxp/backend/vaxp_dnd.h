/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_dnd.h - Drag and Drop API
 * 
 * Implements XDND Protocol v5 for X11 backend.
 * Supports text, URI lists (files), and custom MIME types.
 */

#ifndef VAXP_DND_H
#define VAXP_DND_H

#include "vaxp/core/vaxp_types.h"
#include "vaxp/core/vaxp_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * DND ACTIONS
 * ============================================================================ */

/**
 * @brief Actions that can be performed during drag and drop
 * 
 * These correspond to XDND action atoms.
 */
typedef enum VaxpDndAction {
    VAXP_DND_ACTION_NONE    = 0,       /**< No action */
    VAXP_DND_ACTION_COPY    = (1 << 0), /**< Copy data */
    VAXP_DND_ACTION_MOVE    = (1 << 1), /**< Move data */
    VAXP_DND_ACTION_LINK    = (1 << 2), /**< Create link to data */
    VAXP_DND_ACTION_ASK     = (1 << 3), /**< Ask user which action */
    VAXP_DND_ACTION_PRIVATE = (1 << 4), /**< Private action (app-specific) */
} VaxpDndAction;

/* ============================================================================
 * DND DATA TYPES
 * ============================================================================ */

/**
 * @brief Common MIME types for drag and drop
 */
#define VAXP_DND_MIME_TEXT_PLAIN    "text/plain"
#define VAXP_DND_MIME_TEXT_UTF8     "text/plain;charset=utf-8"
#define VAXP_DND_MIME_URI_LIST      "text/uri-list"
#define VAXP_DND_MIME_TEXT_HTML     "text/html"
#define VAXP_DND_MIME_IMAGE_PNG     "image/png"

/**
 * @brief Data container for drag operations
 */
typedef struct VaxpDndData {
    char mime_type[64];         /**< MIME type of the data */
    void* data;                 /**< Pointer to the data (owned) */
    VaxpSize size;             /**< Size of data in bytes */
} VaxpDndData;

/**
 * @brief Array of DND data for multiple format support
 */
typedef struct VaxpDndDataList {
    VaxpDndData* items;        /**< Array of data items */
    VaxpU32 count;             /**< Number of items */
    VaxpU32 capacity;          /**< Allocated capacity */
} VaxpDndDataList;

/* ============================================================================
 * DND STATE
 * ============================================================================ */

/**
 * @brief Current state of a drag operation
 */
typedef enum VaxpDndState {
    VAXP_DND_STATE_IDLE,       /**< No drag in progress */
    VAXP_DND_STATE_DRAGGING,   /**< Drag is active */
    VAXP_DND_STATE_DROPPED,    /**< Drop occurred, waiting for finish */
} VaxpDndState;

/**
 * @brief Drag operation context
 */
typedef struct VaxpDndContext {
    VaxpDndState state;        /**< Current state */
    VaxpDndAction actions;     /**< Supported actions (bitmask) */
    VaxpDndAction selected_action; /**< Action selected by drop target */
    
    VaxpI32 x;                 /**< Current X position (screen coords) */
    VaxpI32 y;                 /**< Current Y position (screen coords) */
    
    VaxpU32 source_window_id;  /**< Window that initiated the drag */
    VaxpU32 target_window_id;  /**< Window currently under cursor */
    
    VaxpDndDataList data;      /**< Data being dragged */
    
    void* user_data;            /**< User-defined context */
} VaxpDndContext;

/* ============================================================================
 * DND EVENT STRUCTURES
 * ============================================================================ */

/**
 * @brief DND Enter event - drag entered a window
 */
typedef struct VaxpDndEnterEvent {
    VaxpU32 window_id;         /**< Target window ID */
    VaxpU32 source_window_id;  /**< Source window ID */
    char** mime_types;          /**< Available MIME types (NULL-terminated) */
    VaxpU32 mime_type_count;   /**< Number of MIME types */
} VaxpDndEnterEvent;

/**
 * @brief DND Position event - drag moved within window
 */
typedef struct VaxpDndPositionEvent {
    VaxpU32 window_id;         /**< Target window ID */
    VaxpI32 x;                 /**< X position relative to window */
    VaxpI32 y;                 /**< Y position relative to window */
    VaxpI32 root_x;            /**< X position on screen */
    VaxpI32 root_y;            /**< Y position on screen */
    VaxpDndAction actions;     /**< Supported actions */
    VaxpU64 timestamp;         /**< Event timestamp */
} VaxpDndPositionEvent;

/**
 * @brief DND Leave event - drag left the window
 */
typedef struct VaxpDndLeaveEvent {
    VaxpU32 window_id;         /**< Window that was left */
} VaxpDndLeaveEvent;

/**
 * @brief DND Drop event - drop occurred
 */
typedef struct VaxpDndDropEvent {
    VaxpU32 window_id;         /**< Target window ID */
    VaxpI32 x;                 /**< X position of drop */
    VaxpI32 y;                 /**< Y position of drop */
    VaxpDndAction action;      /**< Action to perform */
    VaxpU64 timestamp;         /**< Event timestamp */
} VaxpDndDropEvent;

/* ============================================================================
 * DND API - SOURCE (Initiating drags)
 * ============================================================================ */

/**
 * @brief Begin a drag operation
 * 
 * @param window_id Source window
 * @param data Data to drag (ownership transferred)
 * @param actions Supported actions bitmask
 * @return VaxpResult Success or error
 */
VaxpResult vaxp_dnd_begin_drag(VaxpU32 window_id, 
                                  VaxpDndDataList* data,
                                  VaxpDndAction actions);

/**
 * @brief Cancel current drag operation
 */
void vaxp_dnd_cancel_drag(void);

/**
 * @brief Check if a drag operation is in progress
 */
VaxpBool vaxp_dnd_is_dragging(void);

/**
 * @brief Get current drag context
 */
const VaxpDndContext* vaxp_dnd_get_context(void);

/* ============================================================================
 * DND API - TARGET (Receiving drops)
 * ============================================================================ */

/**
 * @brief Accept the current drag at the given position
 * 
 * Call this in response to DND_POSITION to indicate acceptance.
 * 
 * @param window_id Target window
 * @param x X position
 * @param y Y position
 * @param action Preferred action (must be one of the offered actions)
 * @return VaxpResult Success or error
 */
VaxpResult vaxp_dnd_accept(VaxpU32 window_id, VaxpI32 x, VaxpI32 y,
                              VaxpDndAction action);

/**
 * @brief Reject the current drag
 * 
 * Call this in response to DND_POSITION to indicate rejection.
 * 
 * @param window_id Target window
 */
void vaxp_dnd_reject(VaxpU32 window_id);

/**
 * @brief Request the dropped data in a specific format
 * 
 * Call this in response to DND_DROP to retrieve the data.
 * 
 * @param window_id Target window
 * @param mime_type Preferred MIME type
 * @param out_data Output: received data (caller must free data->data)
 * @return VaxpResult Success or error
 */
VaxpResult vaxp_dnd_get_drop_data(VaxpU32 window_id,
                                     const char* mime_type,
                                     VaxpDndData* out_data);

/**
 * @brief Finish the drop operation
 * 
 * Call this after processing the dropped data.
 * 
 * @param window_id Target window
 * @param success Whether the drop was successfully processed
 */
void vaxp_dnd_finish_drop(VaxpU32 window_id, VaxpBool success);

/* ============================================================================
 * DND DATA HELPERS
 * ============================================================================ */

/**
 * @brief Create a DND data list
 */
VaxpDndDataList* vaxp_dnd_data_list_create(void);

/**
 * @brief Destroy a DND data list and all its contents
 */
void vaxp_dnd_data_list_destroy(VaxpDndDataList* list);

/**
 * @brief Add text data to the list
 */
VaxpResult vaxp_dnd_data_list_add_text(VaxpDndDataList* list, 
                                          const char* text);

/**
 * @brief Add URI list (files) to the list
 * 
 * @param list Data list
 * @param uris NULL-terminated array of URI strings
 */
VaxpResult vaxp_dnd_data_list_add_uris(VaxpDndDataList* list,
                                          const char** uris);

/**
 * @brief Add custom data to the list
 */
VaxpResult vaxp_dnd_data_list_add(VaxpDndDataList* list,
                                     const char* mime_type,
                                     const void* data,
                                     VaxpSize size);

/**
 * @brief Find data by MIME type
 * 
 * @return Pointer to data entry or NULL if not found
 */
const VaxpDndData* vaxp_dnd_data_list_find(const VaxpDndDataList* list,
                                              const char* mime_type);

/**
 * @brief Parse URI list data into array of paths
 * 
 * @param data URI list data
 * @param out_paths Output: array of paths (caller must free each and the array)
 * @param out_count Output: number of paths
 * @return VaxpResult Success or error
 */
VaxpResult vaxp_dnd_parse_uri_list(const VaxpDndData* data,
                                      char*** out_paths,
                                      VaxpU32* out_count);

/* ============================================================================
 * INTERNAL - X11 Backend
 * ============================================================================ */

/**
 * @brief Initialize DND subsystem (called by display init)
 */
VaxpResult vaxp_dnd_init(void* display_handle);

/**
 * @brief Shutdown DND subsystem
 */
void vaxp_dnd_shutdown(void);

/**
 * @brief Register a window to accept drops
 */
VaxpResult vaxp_dnd_register_window(VaxpU32 window_id, void* native_window);

/**
 * @brief Unregister a window from DND
 */
void vaxp_dnd_unregister_window(VaxpU32 window_id);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_DND_H */
