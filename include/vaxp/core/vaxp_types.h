/*
 * VAXPUI - High Performance GUI Framework
 * Copyright (c) 2024
 * 
 * vaxp_types.h - Core type definitions with explicit sizes and ownership semantics
 */

#ifndef VAXP_TYPES_H
#define VAXP_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * VERSION
 * ============================================================================ */

#define VAXP_VERSION_MAJOR 0
#define VAXP_VERSION_MINOR 1
#define VAXP_VERSION_PATCH 0

/* ============================================================================
 * COMPILER DETECTION & ATTRIBUTES
 * ============================================================================ */

#if defined(__GNUC__) || defined(__clang__)
    #define VAXP_LIKELY(x)       __builtin_expect(!!(x), 1)
    #define VAXP_UNLIKELY(x)     __builtin_expect(!!(x), 0)
    #define VAXP_INLINE          static inline __attribute__((always_inline))
    #define VAXP_NOINLINE        __attribute__((noinline))
    #define VAXP_UNUSED          __attribute__((unused))
    #define VAXP_DEPRECATED(msg) __attribute__((deprecated(msg)))
    #define VAXP_PRINTF(fmt, args) __attribute__((format(printf, fmt, args)))
    #define VAXP_MALLOC          __attribute__((malloc))
    #define VAXP_WARN_UNUSED     __attribute__((warn_unused_result))
    #define VAXP_PACKED          __attribute__((packed))
    #define VAXP_ALIGNED(n)      __attribute__((aligned(n)))
    #define VAXP_NORETURN        __attribute__((noreturn))
#else
    #define VAXP_LIKELY(x)       (x)
    #define VAXP_UNLIKELY(x)     (x)
    #define VAXP_INLINE          static inline
    #define VAXP_NOINLINE
    #define VAXP_UNUSED
    #define VAXP_DEPRECATED(msg)
    #define VAXP_PRINTF(fmt, args)
    #define VAXP_MALLOC
    #define VAXP_WARN_UNUSED
    #define VAXP_PACKED
    #define VAXP_ALIGNED(n)
    #define VAXP_NORETURN
#endif

/* ============================================================================
 * OWNERSHIP SEMANTICS (Documentation Markers)
 * ============================================================================ */

/**
 * @brief Pointer owns the memory - caller is responsible for freeing
 * 
 * When a function returns VAXP_OWNED, the caller must eventually free it.
 * When a function parameter is VAXP_OWNED, ownership is transferred to the function.
 */
#define VAXP_OWNED

/**
 * @brief Pointer borrows the memory - caller must NOT free
 * 
 * The pointer is valid only during the scope of use.
 * Do not store or free borrowed pointers.
 */
#define VAXP_BORROWED

/**
 * @brief Pointer may be NULL
 */
#define VAXP_NULLABLE

/**
 * @brief Pointer must NOT be NULL
 */
#define VAXP_NONNULL

/**
 * @brief Output parameter - function writes to this pointer
 */
#define VAXP_OUT

/**
 * @brief Input/Output parameter
 */
#define VAXP_INOUT

/* ============================================================================
 * PRIMITIVE TYPES (Explicit Sizes)
 * ============================================================================ */

typedef int8_t    VaxpI8;
typedef int16_t   VaxpI16;
typedef int32_t   VaxpI32;
typedef int64_t   VaxpI64;

typedef uint8_t   VaxpU8;
typedef uint16_t  VaxpU16;
typedef uint32_t  VaxpU32;
typedef uint64_t  VaxpU64;

typedef float     VaxpF32;
typedef double    VaxpF64;

typedef size_t    VaxpSize;
typedef ptrdiff_t VaxpPtrDiff;
typedef uintptr_t VaxpUPtr;
typedef intptr_t  VaxpIPtr;

typedef bool      VaxpBool;

#define VAXP_TRUE  true
#define VAXP_FALSE false

/* ============================================================================
 * NUMERIC LIMITS
 * ============================================================================ */

#define VAXP_I8_MIN   INT8_MIN
#define VAXP_I8_MAX   INT8_MAX
#define VAXP_I16_MIN  INT16_MIN
#define VAXP_I16_MAX  INT16_MAX
#define VAXP_I32_MIN  INT32_MIN
#define VAXP_I32_MAX  INT32_MAX
#define VAXP_I64_MIN  INT64_MIN
#define VAXP_I64_MAX  INT64_MAX

#define VAXP_U8_MAX   UINT8_MAX
#define VAXP_U16_MAX  UINT16_MAX
#define VAXP_U32_MAX  UINT32_MAX
#define VAXP_U64_MAX  UINT64_MAX

/* ============================================================================
 * GEOMETRY TYPES
 * ============================================================================ */

typedef struct VaxpPoint {
    VaxpI32 x;
    VaxpI32 y;
} VaxpPoint;

typedef struct VaxpPointF {
    VaxpF32 x;
    VaxpF32 y;
} VaxpPointF;

typedef struct VaxpSize2D {
    VaxpU32 width;
    VaxpU32 height;
} VaxpSize2D;

typedef struct VaxpSize2DF {
    VaxpF32 width;
    VaxpF32 height;
} VaxpSize2DF;

typedef struct VaxpRect {
    VaxpI32 x;
    VaxpI32 y;
    VaxpU32 width;
    VaxpU32 height;
} VaxpRect;

typedef struct VaxpRectF {
    VaxpF32 x;
    VaxpF32 y;
    VaxpF32 width;
    VaxpF32 height;
} VaxpRectF;

typedef struct VaxpInsets {
    VaxpI32 top;
    VaxpI32 right;
    VaxpI32 bottom;
    VaxpI32 left;
} VaxpInsets;

/* ============================================================================
 * GEOMETRY HELPER FUNCTIONS
 * ============================================================================ */

VAXP_INLINE VaxpPoint vaxp_point(VaxpI32 x, VaxpI32 y) {
    return (VaxpPoint){ .x = x, .y = y };
}

VAXP_INLINE VaxpSize2D vaxp_size(VaxpU32 width, VaxpU32 height) {
    return (VaxpSize2D){ .width = width, .height = height };
}

VAXP_INLINE VaxpRect vaxp_rect(VaxpI32 x, VaxpI32 y, VaxpU32 w, VaxpU32 h) {
    return (VaxpRect){ .x = x, .y = y, .width = w, .height = h };
}

VAXP_INLINE VaxpBool vaxp_rect_contains(const VaxpRect* r, VaxpI32 x, VaxpI32 y) {
    return x >= r->x && x < (r->x + (VaxpI32)r->width) &&
           y >= r->y && y < (r->y + (VaxpI32)r->height);
}

VAXP_INLINE VaxpBool vaxp_rect_intersects(const VaxpRect* a, const VaxpRect* b) {
    return !(a->x + (VaxpI32)a->width <= b->x ||
             b->x + (VaxpI32)b->width <= a->x ||
             a->y + (VaxpI32)a->height <= b->y ||
             b->y + (VaxpI32)b->height <= a->y);
}

/* ============================================================================
 * COLOR TYPES
 * ============================================================================ */

typedef struct VaxpColor {
    VaxpU8 r;
    VaxpU8 g;
    VaxpU8 b;
    VaxpU8 a;
} VaxpColor;

typedef struct VaxpColorF {
    VaxpF32 r;
    VaxpF32 g;
    VaxpF32 b;
    VaxpF32 a;
} VaxpColorF;

/* Pre-defined colors */
#define VAXP_COLOR_TRANSPARENT ((VaxpColor){ 0, 0, 0, 0 })
#define VAXP_COLOR_BLACK       ((VaxpColor){ 0, 0, 0, 255 })
#define VAXP_COLOR_WHITE       ((VaxpColor){ 255, 255, 255, 255 })
#define VAXP_COLOR_RED         ((VaxpColor){ 255, 0, 0, 255 })
#define VAXP_COLOR_GREEN       ((VaxpColor){ 0, 255, 0, 255 })
#define VAXP_COLOR_BLUE        ((VaxpColor){ 0, 0, 255, 255 })

VAXP_INLINE VaxpColor vaxp_color_rgba(VaxpU8 r, VaxpU8 g, VaxpU8 b, VaxpU8 a) {
    return (VaxpColor){ .r = r, .g = g, .b = b, .a = a };
}

VAXP_INLINE VaxpColor vaxp_color_rgb(VaxpU8 r, VaxpU8 g, VaxpU8 b) {
    return vaxp_color_rgba(r, g, b, 255);
}

VAXP_INLINE VaxpU32 vaxp_color_to_u32(VaxpColor c) {
    return ((VaxpU32)c.a << 24) | ((VaxpU32)c.r << 16) | 
           ((VaxpU32)c.g << 8) | (VaxpU32)c.b;
}

VAXP_INLINE VaxpColor vaxp_color_from_u32(VaxpU32 argb) {
    return (VaxpColor){
        .a = (VaxpU8)((argb >> 24) & 0xFF),
        .r = (VaxpU8)((argb >> 16) & 0xFF),
        .g = (VaxpU8)((argb >> 8) & 0xFF),
        .b = (VaxpU8)(argb & 0xFF)
    };
}

/* ============================================================================
 * UTILITY MACROS
 * ============================================================================ */

#define VAXP_MIN(a, b) ((a) < (b) ? (a) : (b))
#define VAXP_MAX(a, b) ((a) > (b) ? (a) : (b))
#define VAXP_CLAMP(x, lo, hi) VAXP_MIN(VAXP_MAX(x, lo), hi)
#define VAXP_ABS(x) ((x) < 0 ? -(x) : (x))

#define VAXP_ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

#define VAXP_ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))
#define VAXP_ALIGN_DOWN(x, align) ((x) & ~((align) - 1))

#define VAXP_KB(x) ((VaxpSize)(x) * 1024)
#define VAXP_MB(x) ((VaxpSize)(x) * 1024 * 1024)
#define VAXP_GB(x) ((VaxpSize)(x) * 1024 * 1024 * 1024)

/* ============================================================================
 * ASSERTION MACROS
 * ============================================================================ */

#ifdef VAXP_DEBUG
    #include <stdio.h>
    #include <stdlib.h>
    
    #define VAXP_ASSERT(cond) \
        do { \
            if (VAXP_UNLIKELY(!(cond))) { \
                fprintf(stderr, "VAXP ASSERT FAILED: %s\n  at %s:%d in %s()\n", \
                        #cond, __FILE__, __LINE__, __func__); \
                abort(); \
            } \
        } while (0)
    
    #define VAXP_ASSERT_MSG(cond, fmt, ...) \
        do { \
            if (VAXP_UNLIKELY(!(cond))) { \
                fprintf(stderr, "VAXP ASSERT FAILED: %s\n  " fmt "\n  at %s:%d in %s()\n", \
                        #cond, ##__VA_ARGS__, __FILE__, __LINE__, __func__); \
                abort(); \
            } \
        } while (0)
    
    #define VAXP_UNREACHABLE() \
        do { \
            fprintf(stderr, "VAXP UNREACHABLE: at %s:%d in %s()\n", \
                    __FILE__, __LINE__, __func__); \
            abort(); \
        } while (0)
#else
    #define VAXP_ASSERT(cond) ((void)0)
    #define VAXP_ASSERT_MSG(cond, fmt, ...) ((void)0)
    #define VAXP_UNREACHABLE() __builtin_unreachable()
#endif

/* ============================================================================
 * NULL POINTER CHECKS
 * ============================================================================ */

#define VAXP_RETURN_IF_NULL(ptr, ret) \
    do { if (VAXP_UNLIKELY((ptr) == NULL)) return (ret); } while (0)

#define VAXP_RETURN_VOID_IF_NULL(ptr) \
    do { if (VAXP_UNLIKELY((ptr) == NULL)) return; } while (0)

/* ============================================================================
 * WINDOW TYPES (Desktop Environment)
 * ============================================================================ */

/**
 * @brief Window type for desktop environment integration
 */
typedef enum VaxpWindowType {
    VAXP_WINDOW_NORMAL,         /**< Default window with decorations */
    VAXP_WINDOW_PANEL,          /**< Top panel - dock, strut, no decorations */
    VAXP_WINDOW_DOCK,           /**< Bottom dock - dock, strut */
    VAXP_WINDOW_POPUP,          /**< Popup/Control center - no decorations */
    VAXP_WINDOW_LAUNCHER,       /**< App launcher - fullscreen overlay */
    VAXP_WINDOW_DESKTOP,        /**< Desktop background window */
} VaxpWindowType;

/**
 * @brief Window position hints
 */
typedef enum VaxpWindowPosition {
    VAXP_POSITION_DEFAULT,      /**< Let window manager decide */
    VAXP_POSITION_CENTER,       /**< Center on screen */
    VAXP_POSITION_TOP,          /**< Full width at top */
    VAXP_POSITION_BOTTOM,       /**< Full width at bottom */
    VAXP_POSITION_LEFT,         /**< Full height at left */
    VAXP_POSITION_RIGHT,        /**< Full height at right */
    VAXP_POSITION_FULLSCREEN,   /**< Cover entire screen */
} VaxpWindowPosition;

#ifdef __cplusplus
}
#endif

#endif /* VAXP_TYPES_H */
