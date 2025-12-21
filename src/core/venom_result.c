/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_result.c - Error handling implementation
 */

#include "venom/core/venom_result.h"

const char* venom_error_string(VenomError error) {
    switch (error) {
        /* Success */
        case VENOM_OK: return "Success";
        
        /* General errors */
        case VENOM_ERROR_UNKNOWN: return "Unknown error";
        case VENOM_ERROR_NULL_POINTER: return "Null pointer";
        case VENOM_ERROR_INVALID_ARGUMENT: return "Invalid argument";
        case VENOM_ERROR_OUT_OF_MEMORY: return "Out of memory";
        case VENOM_ERROR_OUT_OF_BOUNDS: return "Index out of bounds";
        case VENOM_ERROR_NOT_FOUND: return "Not found";
        case VENOM_ERROR_ALREADY_EXISTS: return "Already exists";
        case VENOM_ERROR_NOT_INITIALIZED: return "Not initialized";
        case VENOM_ERROR_ALREADY_INITIALIZED: return "Already initialized";
        case VENOM_ERROR_INVALID_STATE: return "Invalid state";
        case VENOM_ERROR_NOT_SUPPORTED: return "Operation not supported";
        case VENOM_ERROR_TIMEOUT: return "Operation timed out";
        case VENOM_ERROR_CANCELLED: return "Operation cancelled";
        
        /* Display/Window errors */
        case VENOM_ERROR_DISPLAY_OPEN: return "Failed to open display";
        case VENOM_ERROR_DISPLAY_NO_SCREEN: return "No screen available";
        case VENOM_ERROR_WINDOW_CREATE: return "Failed to create window";
        case VENOM_ERROR_WINDOW_MAP: return "Failed to map window";
        case VENOM_ERROR_SURFACE_CREATE: return "Failed to create surface";
        case VENOM_ERROR_CONTEXT_CREATE: return "Failed to create context";
        
        /* Graphics errors */
        case VENOM_ERROR_SKIA_INIT: return "Failed to initialize Skia";
        case VENOM_ERROR_CANVAS_CREATE: return "Failed to create canvas";
        case VENOM_ERROR_FONT_LOAD: return "Failed to load font";
        case VENOM_ERROR_IMAGE_LOAD: return "Failed to load image";
        case VENOM_ERROR_SHADER_COMPILE: return "Failed to compile shader";
        
        /* Widget errors */
        case VENOM_ERROR_WIDGET_NOT_CHILD: return "Widget is not a child";
        case VENOM_ERROR_WIDGET_NO_PARENT: return "Widget has no parent";
        case VENOM_ERROR_LAYOUT_OVERFLOW: return "Layout overflow";
        
        /* I/O errors */
        case VENOM_ERROR_FILE_OPEN: return "Failed to open file";
        case VENOM_ERROR_FILE_READ: return "Failed to read file";
        case VENOM_ERROR_FILE_WRITE: return "Failed to write file";
        case VENOM_ERROR_PATH_NOT_FOUND: return "Path not found";
        
        default: return "Unknown error code";
    }
}
