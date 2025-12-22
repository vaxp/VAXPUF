/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_dnd.h - Drag and Drop API
 * 
 * Implements XDND Protocol v5 for X11 backend.
 * Supports text, URI lists (files), and custom MIME types.
 */

#ifndef VENOM_DND_H
#define VENOM_DND_H

#include "venom/core/venom_types.h"
#include "venom/core/venom_result.h"

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
typedef enum VenomDndAction {
    VENOM_DND_ACTION_NONE    = 0,       /**< No action */
    VENOM_DND_ACTION_COPY    = (1 << 0), /**< Copy data */
    VENOM_DND_ACTION_MOVE    = (1 << 1), /**< Move data */
    VENOM_DND_ACTION_LINK    = (1 << 2), /**< Create link to data */
    VENOM_DND_ACTION_ASK     = (1 << 3), /**< Ask user which action */
    VENOM_DND_ACTION_PRIVATE = (1 << 4), /**< Private action (app-specific) */
} VenomDndAction;

/* ============================================================================
 * DND DATA TYPES
 * ============================================================================ */

/**
 * @brief Common MIME types for drag and drop
 */
#define VENOM_DND_MIME_TEXT_PLAIN    "text/plain"
#define VENOM_DND_MIME_TEXT_UTF8     "text/plain;charset=utf-8"
#define VENOM_DND_MIME_URI_LIST      "text/uri-list"
#define VENOM_DND_MIME_TEXT_HTML     "text/html"
#define VENOM_DND_MIME_IMAGE_PNG     "image/png"

/**
 * @brief Data container for drag operations
 */
typedef struct VenomDndData {
    char mime_type[64];         /**< MIME type of the data */
    void* data;                 /**< Pointer to the data (owned) */
    VenomSize size;             /**< Size of data in bytes */
} VenomDndData;

/**
 * @brief Array of DND data for multiple format support
 */
typedef struct VenomDndDataList {
    VenomDndData* items;        /**< Array of data items */
    VenomU32 count;             /**< Number of items */
    VenomU32 capacity;          /**< Allocated capacity */
} VenomDndDataList;

/* ============================================================================
 * DND STATE
 * ============================================================================ */

/**
 * @brief Current state of a drag operation
 */
typedef enum VenomDndState {
    VENOM_DND_STATE_IDLE,       /**< No drag in progress */
    VENOM_DND_STATE_DRAGGING,   /**< Drag is active */
    VENOM_DND_STATE_DROPPED,    /**< Drop occurred, waiting for finish */
} VenomDndState;

/**
 * @brief Drag operation context
 */
typedef struct VenomDndContext {
    VenomDndState state;        /**< Current state */
    VenomDndAction actions;     /**< Supported actions (bitmask) */
    VenomDndAction selected_action; /**< Action selected by drop target */
    
    VenomI32 x;                 /**< Current X position (screen coords) */
    VenomI32 y;                 /**< Current Y position (screen coords) */
    
    VenomU32 source_window_id;  /**< Window that initiated the drag */
    VenomU32 target_window_id;  /**< Window currently under cursor */
    
    VenomDndDataList data;      /**< Data being dragged */
    
    void* user_data;            /**< User-defined context */
} VenomDndContext;

/* ============================================================================
 * DND EVENT STRUCTURES
 * ============================================================================ */

/**
 * @brief DND Enter event - drag entered a window
 */
typedef struct VenomDndEnterEvent {
    VenomU32 window_id;         /**< Target window ID */
    VenomU32 source_window_id;  /**< Source window ID */
    char** mime_types;          /**< Available MIME types (NULL-terminated) */
    VenomU32 mime_type_count;   /**< Number of MIME types */
} VenomDndEnterEvent;

/**
 * @brief DND Position event - drag moved within window
 */
typedef struct VenomDndPositionEvent {
    VenomU32 window_id;         /**< Target window ID */
    VenomI32 x;                 /**< X position relative to window */
    VenomI32 y;                 /**< Y position relative to window */
    VenomI32 root_x;            /**< X position on screen */
    VenomI32 root_y;            /**< Y position on screen */
    VenomDndAction actions;     /**< Supported actions */
    VenomU64 timestamp;         /**< Event timestamp */
} VenomDndPositionEvent;

/**
 * @brief DND Leave event - drag left the window
 */
typedef struct VenomDndLeaveEvent {
    VenomU32 window_id;         /**< Window that was left */
} VenomDndLeaveEvent;

/**
 * @brief DND Drop event - drop occurred
 */
typedef struct VenomDndDropEvent {
    VenomU32 window_id;         /**< Target window ID */
    VenomI32 x;                 /**< X position of drop */
    VenomI32 y;                 /**< Y position of drop */
    VenomDndAction action;      /**< Action to perform */
    VenomU64 timestamp;         /**< Event timestamp */
} VenomDndDropEvent;

/* ============================================================================
 * DND API - SOURCE (Initiating drags)
 * ============================================================================ */

/**
 * @brief Begin a drag operation
 * 
 * @param window_id Source window
 * @param data Data to drag (ownership transferred)
 * @param actions Supported actions bitmask
 * @return VenomResult Success or error
 */
VenomResult venom_dnd_begin_drag(VenomU32 window_id, 
                                  VenomDndDataList* data,
                                  VenomDndAction actions);

/**
 * @brief Cancel current drag operation
 */
void venom_dnd_cancel_drag(void);

/**
 * @brief Check if a drag operation is in progress
 */
VenomBool venom_dnd_is_dragging(void);

/**
 * @brief Get current drag context
 */
const VenomDndContext* venom_dnd_get_context(void);

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
 * @return VenomResult Success or error
 */
VenomResult venom_dnd_accept(VenomU32 window_id, VenomI32 x, VenomI32 y,
                              VenomDndAction action);

/**
 * @brief Reject the current drag
 * 
 * Call this in response to DND_POSITION to indicate rejection.
 * 
 * @param window_id Target window
 */
void venom_dnd_reject(VenomU32 window_id);

/**
 * @brief Request the dropped data in a specific format
 * 
 * Call this in response to DND_DROP to retrieve the data.
 * 
 * @param window_id Target window
 * @param mime_type Preferred MIME type
 * @param out_data Output: received data (caller must free data->data)
 * @return VenomResult Success or error
 */
VenomResult venom_dnd_get_drop_data(VenomU32 window_id,
                                     const char* mime_type,
                                     VenomDndData* out_data);

/**
 * @brief Finish the drop operation
 * 
 * Call this after processing the dropped data.
 * 
 * @param window_id Target window
 * @param success Whether the drop was successfully processed
 */
void venom_dnd_finish_drop(VenomU32 window_id, VenomBool success);

/* ============================================================================
 * DND DATA HELPERS
 * ============================================================================ */

/**
 * @brief Create a DND data list
 */
VenomDndDataList* venom_dnd_data_list_create(void);

/**
 * @brief Destroy a DND data list and all its contents
 */
void venom_dnd_data_list_destroy(VenomDndDataList* list);

/**
 * @brief Add text data to the list
 */
VenomResult venom_dnd_data_list_add_text(VenomDndDataList* list, 
                                          const char* text);

/**
 * @brief Add URI list (files) to the list
 * 
 * @param list Data list
 * @param uris NULL-terminated array of URI strings
 */
VenomResult venom_dnd_data_list_add_uris(VenomDndDataList* list,
                                          const char** uris);

/**
 * @brief Add custom data to the list
 */
VenomResult venom_dnd_data_list_add(VenomDndDataList* list,
                                     const char* mime_type,
                                     const void* data,
                                     VenomSize size);

/**
 * @brief Find data by MIME type
 * 
 * @return Pointer to data entry or NULL if not found
 */
const VenomDndData* venom_dnd_data_list_find(const VenomDndDataList* list,
                                              const char* mime_type);

/**
 * @brief Parse URI list data into array of paths
 * 
 * @param data URI list data
 * @param out_paths Output: array of paths (caller must free each and the array)
 * @param out_count Output: number of paths
 * @return VenomResult Success or error
 */
VenomResult venom_dnd_parse_uri_list(const VenomDndData* data,
                                      char*** out_paths,
                                      VenomU32* out_count);

/* ============================================================================
 * INTERNAL - X11 Backend
 * ============================================================================ */

/**
 * @brief Initialize DND subsystem (called by display init)
 */
VenomResult venom_dnd_init(void* display_handle);

/**
 * @brief Shutdown DND subsystem
 */
void venom_dnd_shutdown(void);

/**
 * @brief Register a window to accept drops
 */
VenomResult venom_dnd_register_window(VenomU32 window_id, void* native_window);

/**
 * @brief Unregister a window from DND
 */
void venom_dnd_unregister_window(VenomU32 window_id);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_DND_H */
