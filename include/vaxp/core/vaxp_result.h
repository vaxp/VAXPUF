/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_result.h - Error handling with Result types (no exceptions)
 */

#ifndef VAXP_RESULT_H
#define VAXP_RESULT_H

#include "vaxp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * ERROR CODES
 * ============================================================================ */

typedef enum VaxpError {
    /* Success */
    VAXP_OK = 0,
    
    /* General errors (1-99) */
    VAXP_ERROR_UNKNOWN = 1,
    VAXP_ERROR_NULL_POINTER = 2,
    VAXP_ERROR_INVALID_ARGUMENT = 3,
    VAXP_ERROR_OUT_OF_MEMORY = 4,
    VAXP_ERROR_OUT_OF_BOUNDS = 5,
    VAXP_ERROR_NOT_FOUND = 6,
    VAXP_ERROR_ALREADY_EXISTS = 7,
    VAXP_ERROR_NOT_INITIALIZED = 8,
    VAXP_ERROR_ALREADY_INITIALIZED = 9,
    VAXP_ERROR_INVALID_STATE = 10,
    VAXP_ERROR_NOT_SUPPORTED = 11,
    VAXP_ERROR_TIMEOUT = 12,
    VAXP_ERROR_CANCELLED = 13,
    
    /* Display/Window errors (100-199) */
    VAXP_ERROR_DISPLAY_OPEN = 100,
    VAXP_ERROR_DISPLAY_NO_SCREEN = 101,
    VAXP_ERROR_WINDOW_CREATE = 102,
    VAXP_ERROR_WINDOW_MAP = 103,
    VAXP_ERROR_SURFACE_CREATE = 104,
    VAXP_ERROR_CONTEXT_CREATE = 105,
    
    /* Graphics errors (200-299) */
    VAXP_ERROR_SKIA_INIT = 200,
    VAXP_ERROR_CANVAS_CREATE = 201,
    VAXP_ERROR_FONT_LOAD = 202,
    VAXP_ERROR_IMAGE_LOAD = 203,
    VAXP_ERROR_SHADER_COMPILE = 204,
    
    /* Widget errors (300-399) */
    VAXP_ERROR_WIDGET_NOT_CHILD = 300,
    VAXP_ERROR_WIDGET_NO_PARENT = 301,
    VAXP_ERROR_LAYOUT_OVERFLOW = 302,
    
    /* I/O errors (400-499) */
    VAXP_ERROR_FILE_OPEN = 400,
    VAXP_ERROR_FILE_READ = 401,
    VAXP_ERROR_FILE_WRITE = 402,
    VAXP_ERROR_PATH_NOT_FOUND = 403,
    VAXP_ERROR_IO = 404,
    
} VaxpError;

/**
 * @brief Get human-readable error message
 */
const char* vaxp_error_string(VaxpError error);

/* ============================================================================
 * RESULT TYPE - Generic Pattern
 * ============================================================================ */

/**
 * @brief Result type storing either a value or an error
 * 
 * Usage pattern:
 *   VaxpResultPtr result = some_function_that_may_fail();
 *   if (!result.ok) {
 *       printf("Error: %s\n", vaxp_error_string(result.error));
 *       return;
 *   }
 *   use(result.value);
 */

/* Result for pointer types */
typedef struct VaxpResultPtr {
    VaxpBool ok;
    union {
        void* value;
        VaxpError error;
    };
} VaxpResultPtr;

/* Result for VaxpI32 */
typedef struct VaxpResultI32 {
    VaxpBool ok;
    union {
        VaxpI32 value;
        VaxpError error;
    };
} VaxpResultI32;

/* Result for VaxpU32 */
typedef struct VaxpResultU32 {
    VaxpBool ok;
    union {
        VaxpU32 value;
        VaxpError error;
    };
} VaxpResultU32;

/* Result for VaxpSize */
typedef struct VaxpResultSize {
    VaxpBool ok;
    union {
        VaxpSize value;
        VaxpError error;
    };
} VaxpResultSize;

/* Result for VaxpBool (when failure is distinct from false) */
typedef struct VaxpResultBool {
    VaxpBool ok;
    union {
        VaxpBool value;
        VaxpError error;
    };
} VaxpResultBool;

/* Unit result (just success/failure, no value) */
typedef struct VaxpResult {
    VaxpBool ok;
    VaxpError error;
} VaxpResult;

/* ============================================================================
 * RESULT CONSTRUCTION MACROS
 * ============================================================================ */

/**
 * @brief Create a successful result with a pointer value
 */
#define VAXP_OK_PTR(val) ((VaxpResultPtr){ .ok = VAXP_TRUE, .value = (val) })

/**
 * @brief Create a failed result with pointer type
 */
#define VAXP_ERR_PTR(err) ((VaxpResultPtr){ .ok = VAXP_FALSE, .error = (err) })

/**
 * @brief Create a successful result with i32 value
 */
#define VAXP_OK_I32(val) ((VaxpResultI32){ .ok = VAXP_TRUE, .value = (val) })

/**
 * @brief Create a failed result with i32 type
 */
#define VAXP_ERR_I32(err) ((VaxpResultI32){ .ok = VAXP_FALSE, .error = (err) })

/**
 * @brief Create a successful unit result
 */
#define VAXP_OK_UNIT() ((VaxpResult){ .ok = VAXP_TRUE, .error = VAXP_OK })

/**
 * @brief Create a failed unit result
 */
#define VAXP_ERR_UNIT(err) ((VaxpResult){ .ok = VAXP_FALSE, .error = (err) })

/* ============================================================================
 * ERROR PROPAGATION MACROS
 * ============================================================================ */

/**
 * @brief Early return if result is an error (for pointer results)
 * 
 * Usage:
 *   VaxpResultPtr result = do_something();
 *   VAXP_TRY_PTR(result);  // Returns if failed
 *   use(result.value);
 */
#define VAXP_TRY_PTR(result) \
    do { \
        VaxpResultPtr _r = (result); \
        if (!_r.ok) return _r; \
    } while (0)

/**
 * @brief Early return if unit result is an error
 */
#define VAXP_TRY(result) \
    do { \
        VaxpResult _r = (result); \
        if (!_r.ok) return _r; \
    } while (0)

/**
 * @brief Propagate error or extract value to variable
 * 
 * Usage:
 *   VaxpWidget* widget;
 *   VAXP_TRY_ASSIGN(widget, create_widget());
 */
#define VAXP_TRY_ASSIGN_PTR(var, result) \
    do { \
        VaxpResultPtr _r = (result); \
        if (!_r.ok) return _r; \
        (var) = _r.value; \
    } while (0)

/**
 * @brief Return error if condition is false
 */
#define VAXP_ENSURE(cond, err) \
    do { \
        if (VAXP_UNLIKELY(!(cond))) { \
            return VAXP_ERR_UNIT(err); \
        } \
    } while (0)

/**
 * @brief Return error if pointer is NULL
 */
#define VAXP_ENSURE_NOT_NULL(ptr) \
    VAXP_ENSURE((ptr) != NULL, VAXP_ERROR_NULL_POINTER)

/* ============================================================================
 * RESULT UNWRAPPING (Debug asserts on error)
 * ============================================================================ */

/**
 * @brief Unwrap pointer result, asserting on error
 * 
 * Use only when you're certain the result cannot be an error.
 */
VAXP_INLINE void* vaxp_unwrap_ptr(VaxpResultPtr result) {
    VAXP_ASSERT_MSG(result.ok, "Unwrap failed with error: %d", result.error);
    return result.value;
}

/**
 * @brief Unwrap i32 result, asserting on error
 */
VAXP_INLINE VaxpI32 vaxp_unwrap_i32(VaxpResultI32 result) {
    VAXP_ASSERT_MSG(result.ok, "Unwrap failed with error: %d", result.error);
    return result.value;
}

/**
 * @brief Unwrap with default value on error
 */
VAXP_INLINE void* vaxp_unwrap_or_ptr(VaxpResultPtr result, void* default_val) {
    return result.ok ? result.value : default_val;
}

VAXP_INLINE VaxpI32 vaxp_unwrap_or_i32(VaxpResultI32 result, VaxpI32 default_val) {
    return result.ok ? result.value : default_val;
}

#ifdef __cplusplus
}
#endif

#endif /* VAXP_RESULT_H */
