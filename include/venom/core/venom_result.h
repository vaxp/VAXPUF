/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_result.h - Error handling with Result types (no exceptions)
 */

#ifndef VENOM_RESULT_H
#define VENOM_RESULT_H

#include "venom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * ERROR CODES
 * ============================================================================ */

typedef enum VenomError {
    /* Success */
    VENOM_OK = 0,
    
    /* General errors (1-99) */
    VENOM_ERROR_UNKNOWN = 1,
    VENOM_ERROR_NULL_POINTER = 2,
    VENOM_ERROR_INVALID_ARGUMENT = 3,
    VENOM_ERROR_OUT_OF_MEMORY = 4,
    VENOM_ERROR_OUT_OF_BOUNDS = 5,
    VENOM_ERROR_NOT_FOUND = 6,
    VENOM_ERROR_ALREADY_EXISTS = 7,
    VENOM_ERROR_NOT_INITIALIZED = 8,
    VENOM_ERROR_ALREADY_INITIALIZED = 9,
    VENOM_ERROR_INVALID_STATE = 10,
    VENOM_ERROR_NOT_SUPPORTED = 11,
    VENOM_ERROR_TIMEOUT = 12,
    VENOM_ERROR_CANCELLED = 13,
    
    /* Display/Window errors (100-199) */
    VENOM_ERROR_DISPLAY_OPEN = 100,
    VENOM_ERROR_DISPLAY_NO_SCREEN = 101,
    VENOM_ERROR_WINDOW_CREATE = 102,
    VENOM_ERROR_WINDOW_MAP = 103,
    VENOM_ERROR_SURFACE_CREATE = 104,
    VENOM_ERROR_CONTEXT_CREATE = 105,
    
    /* Graphics errors (200-299) */
    VENOM_ERROR_SKIA_INIT = 200,
    VENOM_ERROR_CANVAS_CREATE = 201,
    VENOM_ERROR_FONT_LOAD = 202,
    VENOM_ERROR_IMAGE_LOAD = 203,
    VENOM_ERROR_SHADER_COMPILE = 204,
    
    /* Widget errors (300-399) */
    VENOM_ERROR_WIDGET_NOT_CHILD = 300,
    VENOM_ERROR_WIDGET_NO_PARENT = 301,
    VENOM_ERROR_LAYOUT_OVERFLOW = 302,
    
    /* I/O errors (400-499) */
    VENOM_ERROR_FILE_OPEN = 400,
    VENOM_ERROR_FILE_READ = 401,
    VENOM_ERROR_FILE_WRITE = 402,
    VENOM_ERROR_PATH_NOT_FOUND = 403,
    
} VenomError;

/**
 * @brief Get human-readable error message
 */
const char* venom_error_string(VenomError error);

/* ============================================================================
 * RESULT TYPE - Generic Pattern
 * ============================================================================ */

/**
 * @brief Result type storing either a value or an error
 * 
 * Usage pattern:
 *   VenomResultPtr result = some_function_that_may_fail();
 *   if (!result.ok) {
 *       printf("Error: %s\n", venom_error_string(result.error));
 *       return;
 *   }
 *   use(result.value);
 */

/* Result for pointer types */
typedef struct VenomResultPtr {
    VenomBool ok;
    union {
        void* value;
        VenomError error;
    };
} VenomResultPtr;

/* Result for VenomI32 */
typedef struct VenomResultI32 {
    VenomBool ok;
    union {
        VenomI32 value;
        VenomError error;
    };
} VenomResultI32;

/* Result for VenomU32 */
typedef struct VenomResultU32 {
    VenomBool ok;
    union {
        VenomU32 value;
        VenomError error;
    };
} VenomResultU32;

/* Result for VenomSize */
typedef struct VenomResultSize {
    VenomBool ok;
    union {
        VenomSize value;
        VenomError error;
    };
} VenomResultSize;

/* Result for VenomBool (when failure is distinct from false) */
typedef struct VenomResultBool {
    VenomBool ok;
    union {
        VenomBool value;
        VenomError error;
    };
} VenomResultBool;

/* Unit result (just success/failure, no value) */
typedef struct VenomResult {
    VenomBool ok;
    VenomError error;
} VenomResult;

/* ============================================================================
 * RESULT CONSTRUCTION MACROS
 * ============================================================================ */

/**
 * @brief Create a successful result with a pointer value
 */
#define VENOM_OK_PTR(val) ((VenomResultPtr){ .ok = VENOM_TRUE, .value = (val) })

/**
 * @brief Create a failed result with pointer type
 */
#define VENOM_ERR_PTR(err) ((VenomResultPtr){ .ok = VENOM_FALSE, .error = (err) })

/**
 * @brief Create a successful result with i32 value
 */
#define VENOM_OK_I32(val) ((VenomResultI32){ .ok = VENOM_TRUE, .value = (val) })

/**
 * @brief Create a failed result with i32 type
 */
#define VENOM_ERR_I32(err) ((VenomResultI32){ .ok = VENOM_FALSE, .error = (err) })

/**
 * @brief Create a successful unit result
 */
#define VENOM_OK_UNIT() ((VenomResult){ .ok = VENOM_TRUE, .error = VENOM_OK })

/**
 * @brief Create a failed unit result
 */
#define VENOM_ERR_UNIT(err) ((VenomResult){ .ok = VENOM_FALSE, .error = (err) })

/* ============================================================================
 * ERROR PROPAGATION MACROS
 * ============================================================================ */

/**
 * @brief Early return if result is an error (for pointer results)
 * 
 * Usage:
 *   VenomResultPtr result = do_something();
 *   VENOM_TRY_PTR(result);  // Returns if failed
 *   use(result.value);
 */
#define VENOM_TRY_PTR(result) \
    do { \
        VenomResultPtr _r = (result); \
        if (!_r.ok) return _r; \
    } while (0)

/**
 * @brief Early return if unit result is an error
 */
#define VENOM_TRY(result) \
    do { \
        VenomResult _r = (result); \
        if (!_r.ok) return _r; \
    } while (0)

/**
 * @brief Propagate error or extract value to variable
 * 
 * Usage:
 *   VenomWidget* widget;
 *   VENOM_TRY_ASSIGN(widget, create_widget());
 */
#define VENOM_TRY_ASSIGN_PTR(var, result) \
    do { \
        VenomResultPtr _r = (result); \
        if (!_r.ok) return _r; \
        (var) = _r.value; \
    } while (0)

/**
 * @brief Return error if condition is false
 */
#define VENOM_ENSURE(cond, err) \
    do { \
        if (VENOM_UNLIKELY(!(cond))) { \
            return VENOM_ERR_UNIT(err); \
        } \
    } while (0)

/**
 * @brief Return error if pointer is NULL
 */
#define VENOM_ENSURE_NOT_NULL(ptr) \
    VENOM_ENSURE((ptr) != NULL, VENOM_ERROR_NULL_POINTER)

/* ============================================================================
 * RESULT UNWRAPPING (Debug asserts on error)
 * ============================================================================ */

/**
 * @brief Unwrap pointer result, asserting on error
 * 
 * Use only when you're certain the result cannot be an error.
 */
VENOM_INLINE void* venom_unwrap_ptr(VenomResultPtr result) {
    VENOM_ASSERT_MSG(result.ok, "Unwrap failed with error: %d", result.error);
    return result.value;
}

/**
 * @brief Unwrap i32 result, asserting on error
 */
VENOM_INLINE VenomI32 venom_unwrap_i32(VenomResultI32 result) {
    VENOM_ASSERT_MSG(result.ok, "Unwrap failed with error: %d", result.error);
    return result.value;
}

/**
 * @brief Unwrap with default value on error
 */
VENOM_INLINE void* venom_unwrap_or_ptr(VenomResultPtr result, void* default_val) {
    return result.ok ? result.value : default_val;
}

VENOM_INLINE VenomI32 venom_unwrap_or_i32(VenomResultI32 result, VenomI32 default_val) {
    return result.ok ? result.value : default_val;
}

#ifdef __cplusplus
}
#endif

#endif /* VENOM_RESULT_H */
