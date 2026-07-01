/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_clipboard.h - Clipboard API
 * 
 * Implements X11 Selections protocol for clipboard operations.
 * Supports CLIPBOARD and PRIMARY selections.
 */

#ifndef VAXP_CLIPBOARD_H
#define VAXP_CLIPBOARD_H

#include "vaxp/core/vaxp_types.h"
#include "vaxp/core/vaxp_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * CLIPBOARD TYPES
 * ============================================================================ */

/**
 * @brief Clipboard selection type
 */
typedef enum VaxpClipboardSelection {
    VAXP_CLIPBOARD_PRIMARY,   /**< PRIMARY selection (middle-click paste) */
    VAXP_CLIPBOARD_CLIPBOARD, /**< CLIPBOARD selection (Ctrl+C/V) */
} VaxpClipboardSelection;

/**
 * @brief Supported clipboard data formats
 */
typedef enum VaxpClipboardFormat {
    VAXP_CLIPBOARD_FORMAT_TEXT,      /**< Plain text (UTF-8) */
    VAXP_CLIPBOARD_FORMAT_HTML,      /**< HTML text */
    VAXP_CLIPBOARD_FORMAT_IMAGE_PNG, /**< PNG image data */
    VAXP_CLIPBOARD_FORMAT_URI_LIST,  /**< List of URIs/file paths */
} VaxpClipboardFormat;

/* ============================================================================
 * TEXT CLIPBOARD API
 * ============================================================================ */

/**
 * @brief Set text to clipboard
 * 
 * Sets the text content of the specified selection.
 * The text is copied internally, so the caller can free their copy.
 * 
 * @param selection Which selection to use (CLIPBOARD or PRIMARY)
 * @param text UTF-8 encoded text to copy
 * @return VaxpResult Success or error
 */
VaxpResult vaxp_clipboard_set_text(VaxpClipboardSelection selection,
                                      const char* text);

/**
 * @brief Get text from clipboard
 * 
 * Gets the text content of the specified selection.
 * 
 * @param selection Which selection to use (CLIPBOARD or PRIMARY)
 * @param out_text Output: pointer to text (caller must free with vaxp_free)
 * @return VaxpResult Success or error
 * 
 * @note Returns VAXP_ERROR_NOT_FOUND if clipboard is empty or doesn't contain text.
 */
VaxpResult vaxp_clipboard_get_text(VaxpClipboardSelection selection,
                                      char** out_text);

/**
 * @brief Check if clipboard has text content
 * 
 * @param selection Which selection to check
 * @return VAXP_TRUE if text is available
 */
VaxpBool vaxp_clipboard_has_text(VaxpClipboardSelection selection);

/**
 * @brief Clear clipboard contents
 * 
 * Clears the specified selection if we own it.
 * 
 * @param selection Which selection to clear
 */
void vaxp_clipboard_clear(VaxpClipboardSelection selection);

/* ============================================================================
 * CONVENIENCE MACROS
 * ============================================================================ */

/**
 * @brief Set text to the main clipboard (CLIPBOARD selection)
 */
#define vaxp_clipboard_copy(text) \
    vaxp_clipboard_set_text(VAXP_CLIPBOARD_CLIPBOARD, (text))

/**
 * @brief Get text from the main clipboard (CLIPBOARD selection)
 */
#define vaxp_clipboard_paste(out_text) \
    vaxp_clipboard_get_text(VAXP_CLIPBOARD_CLIPBOARD, (out_text))

/**
 * @brief Check if main clipboard has text
 */
#define vaxp_clipboard_can_paste() \
    vaxp_clipboard_has_text(VAXP_CLIPBOARD_CLIPBOARD)

/* ============================================================================
 * INTERNAL API
 * ============================================================================ */

/**
 * @brief Initialize clipboard subsystem
 * 
 * Called during display initialization.
 * 
 * @param display_handle Platform display handle (X11 Display*)
 * @param window_handle Window to use for clipboard operations
 * @return VaxpResult Success or error
 */
VaxpResult vaxp_clipboard_init(void* display_handle, void* window_handle);

/**
 * @brief Shutdown clipboard subsystem
 */
void vaxp_clipboard_shutdown(void);

/**
 * @brief Process clipboard-related X events
 * 
 * Called from the event loop to handle SelectionRequest, SelectionClear, etc.
 * 
 * @param xevent X event to process
 * @return VAXP_TRUE if the event was handled
 */
VaxpBool vaxp_clipboard_process_event(void* xevent);

/**
 * @brief Called before window destruction to transfer clipboard ownership
 */
void vaxp_clipboard_on_window_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_CLIPBOARD_H */
