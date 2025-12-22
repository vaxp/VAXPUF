/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_clipboard.h - Clipboard API
 * 
 * Implements X11 Selections protocol for clipboard operations.
 * Supports CLIPBOARD and PRIMARY selections.
 */

#ifndef VENOM_CLIPBOARD_H
#define VENOM_CLIPBOARD_H

#include "venom/core/venom_types.h"
#include "venom/core/venom_result.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * CLIPBOARD TYPES
 * ============================================================================ */

/**
 * @brief Clipboard selection type
 */
typedef enum VenomClipboardSelection {
    VENOM_CLIPBOARD_PRIMARY,   /**< PRIMARY selection (middle-click paste) */
    VENOM_CLIPBOARD_CLIPBOARD, /**< CLIPBOARD selection (Ctrl+C/V) */
} VenomClipboardSelection;

/**
 * @brief Supported clipboard data formats
 */
typedef enum VenomClipboardFormat {
    VENOM_CLIPBOARD_FORMAT_TEXT,      /**< Plain text (UTF-8) */
    VENOM_CLIPBOARD_FORMAT_HTML,      /**< HTML text */
    VENOM_CLIPBOARD_FORMAT_IMAGE_PNG, /**< PNG image data */
    VENOM_CLIPBOARD_FORMAT_URI_LIST,  /**< List of URIs/file paths */
} VenomClipboardFormat;

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
 * @return VenomResult Success or error
 */
VenomResult venom_clipboard_set_text(VenomClipboardSelection selection,
                                      const char* text);

/**
 * @brief Get text from clipboard
 * 
 * Gets the text content of the specified selection.
 * 
 * @param selection Which selection to use (CLIPBOARD or PRIMARY)
 * @param out_text Output: pointer to text (caller must free with venom_free)
 * @return VenomResult Success or error
 * 
 * @note Returns VENOM_ERROR_NOT_FOUND if clipboard is empty or doesn't contain text.
 */
VenomResult venom_clipboard_get_text(VenomClipboardSelection selection,
                                      char** out_text);

/**
 * @brief Check if clipboard has text content
 * 
 * @param selection Which selection to check
 * @return VENOM_TRUE if text is available
 */
VenomBool venom_clipboard_has_text(VenomClipboardSelection selection);

/**
 * @brief Clear clipboard contents
 * 
 * Clears the specified selection if we own it.
 * 
 * @param selection Which selection to clear
 */
void venom_clipboard_clear(VenomClipboardSelection selection);

/* ============================================================================
 * CONVENIENCE MACROS
 * ============================================================================ */

/**
 * @brief Set text to the main clipboard (CLIPBOARD selection)
 */
#define venom_clipboard_copy(text) \
    venom_clipboard_set_text(VENOM_CLIPBOARD_CLIPBOARD, (text))

/**
 * @brief Get text from the main clipboard (CLIPBOARD selection)
 */
#define venom_clipboard_paste(out_text) \
    venom_clipboard_get_text(VENOM_CLIPBOARD_CLIPBOARD, (out_text))

/**
 * @brief Check if main clipboard has text
 */
#define venom_clipboard_can_paste() \
    venom_clipboard_has_text(VENOM_CLIPBOARD_CLIPBOARD)

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
 * @return VenomResult Success or error
 */
VenomResult venom_clipboard_init(void* display_handle, void* window_handle);

/**
 * @brief Shutdown clipboard subsystem
 */
void venom_clipboard_shutdown(void);

/**
 * @brief Process clipboard-related X events
 * 
 * Called from the event loop to handle SelectionRequest, SelectionClear, etc.
 * 
 * @param xevent X event to process
 * @return VENOM_TRUE if the event was handled
 */
VenomBool venom_clipboard_process_event(void* xevent);

/**
 * @brief Called before window destruction to transfer clipboard ownership
 */
void venom_clipboard_on_window_destroy(void);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_CLIPBOARD_H */
