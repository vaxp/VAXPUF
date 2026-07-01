/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_result.c - Error handling implementation
 */

#include "vaxp/core/vaxp_result.h"

const char* vaxp_error_string(VaxpError error) {
    switch (error) {
        /* Success */
        case VAXP_OK: return "Success";
        
        /* General errors */
        case VAXP_ERROR_UNKNOWN: return "Unknown error";
        case VAXP_ERROR_NULL_POINTER: return "Null pointer";
        case VAXP_ERROR_INVALID_ARGUMENT: return "Invalid argument";
        case VAXP_ERROR_OUT_OF_MEMORY: return "Out of memory";
        case VAXP_ERROR_OUT_OF_BOUNDS: return "Index out of bounds";
        case VAXP_ERROR_NOT_FOUND: return "Not found";
        case VAXP_ERROR_ALREADY_EXISTS: return "Already exists";
        case VAXP_ERROR_NOT_INITIALIZED: return "Not initialized";
        case VAXP_ERROR_ALREADY_INITIALIZED: return "Already initialized";
        case VAXP_ERROR_INVALID_STATE: return "Invalid state";
        case VAXP_ERROR_NOT_SUPPORTED: return "Operation not supported";
        case VAXP_ERROR_TIMEOUT: return "Operation timed out";
        case VAXP_ERROR_CANCELLED: return "Operation cancelled";
        
        /* Display/Window errors */
        case VAXP_ERROR_DISPLAY_OPEN: return "Failed to open display";
        case VAXP_ERROR_DISPLAY_NO_SCREEN: return "No screen available";
        case VAXP_ERROR_WINDOW_CREATE: return "Failed to create window";
        case VAXP_ERROR_WINDOW_MAP: return "Failed to map window";
        case VAXP_ERROR_SURFACE_CREATE: return "Failed to create surface";
        case VAXP_ERROR_CONTEXT_CREATE: return "Failed to create context";
        
        /* Graphics errors */
        case VAXP_ERROR_SKIA_INIT: return "Failed to initialize Skia";
        case VAXP_ERROR_CANVAS_CREATE: return "Failed to create canvas";
        case VAXP_ERROR_FONT_LOAD: return "Failed to load font";
        case VAXP_ERROR_IMAGE_LOAD: return "Failed to load image";
        case VAXP_ERROR_SHADER_COMPILE: return "Failed to compile shader";
        
        /* Widget errors */
        case VAXP_ERROR_WIDGET_NOT_CHILD: return "Widget is not a child";
        case VAXP_ERROR_WIDGET_NO_PARENT: return "Widget has no parent";
        case VAXP_ERROR_LAYOUT_OVERFLOW: return "Layout overflow";
        
        /* I/O errors */
        case VAXP_ERROR_FILE_OPEN: return "Failed to open file";
        case VAXP_ERROR_FILE_READ: return "Failed to read file";
        case VAXP_ERROR_FILE_WRITE: return "Failed to write file";
        case VAXP_ERROR_PATH_NOT_FOUND: return "Path not found";
        
        default: return "Unknown error code";
    }
}
