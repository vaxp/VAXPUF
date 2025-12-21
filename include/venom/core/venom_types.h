/*
 * VENOMUI - High Performance GUI Framework
 * Copyright (c) 2024
 * 
 * venom_types.h - Core type definitions with explicit sizes and ownership semantics
 */

#ifndef VENOM_TYPES_H
#define VENOM_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * VERSION
 * ============================================================================ */

#define VENOM_VERSION_MAJOR 0
#define VENOM_VERSION_MINOR 1
#define VENOM_VERSION_PATCH 0

/* ============================================================================
 * COMPILER DETECTION & ATTRIBUTES
 * ============================================================================ */

#if defined(__GNUC__) || defined(__clang__)
    #define VENOM_LIKELY(x)       __builtin_expect(!!(x), 1)
    #define VENOM_UNLIKELY(x)     __builtin_expect(!!(x), 0)
    #define VENOM_INLINE          static inline __attribute__((always_inline))
    #define VENOM_NOINLINE        __attribute__((noinline))
    #define VENOM_UNUSED          __attribute__((unused))
    #define VENOM_DEPRECATED(msg) __attribute__((deprecated(msg)))
    #define VENOM_PRINTF(fmt, args) __attribute__((format(printf, fmt, args)))
    #define VENOM_MALLOC          __attribute__((malloc))
    #define VENOM_WARN_UNUSED     __attribute__((warn_unused_result))
    #define VENOM_PACKED          __attribute__((packed))
    #define VENOM_ALIGNED(n)      __attribute__((aligned(n)))
    #define VENOM_NORETURN        __attribute__((noreturn))
#else
    #define VENOM_LIKELY(x)       (x)
    #define VENOM_UNLIKELY(x)     (x)
    #define VENOM_INLINE          static inline
    #define VENOM_NOINLINE
    #define VENOM_UNUSED
    #define VENOM_DEPRECATED(msg)
    #define VENOM_PRINTF(fmt, args)
    #define VENOM_MALLOC
    #define VENOM_WARN_UNUSED
    #define VENOM_PACKED
    #define VENOM_ALIGNED(n)
    #define VENOM_NORETURN
#endif

/* ============================================================================
 * OWNERSHIP SEMANTICS (Documentation Markers)
 * ============================================================================ */

/**
 * @brief Pointer owns the memory - caller is responsible for freeing
 * 
 * When a function returns VENOM_OWNED, the caller must eventually free it.
 * When a function parameter is VENOM_OWNED, ownership is transferred to the function.
 */
#define VENOM_OWNED

/**
 * @brief Pointer borrows the memory - caller must NOT free
 * 
 * The pointer is valid only during the scope of use.
 * Do not store or free borrowed pointers.
 */
#define VENOM_BORROWED

/**
 * @brief Pointer may be NULL
 */
#define VENOM_NULLABLE

/**
 * @brief Pointer must NOT be NULL
 */
#define VENOM_NONNULL

/**
 * @brief Output parameter - function writes to this pointer
 */
#define VENOM_OUT

/**
 * @brief Input/Output parameter
 */
#define VENOM_INOUT

/* ============================================================================
 * PRIMITIVE TYPES (Explicit Sizes)
 * ============================================================================ */

typedef int8_t    VenomI8;
typedef int16_t   VenomI16;
typedef int32_t   VenomI32;
typedef int64_t   VenomI64;

typedef uint8_t   VenomU8;
typedef uint16_t  VenomU16;
typedef uint32_t  VenomU32;
typedef uint64_t  VenomU64;

typedef float     VenomF32;
typedef double    VenomF64;

typedef size_t    VenomSize;
typedef ptrdiff_t VenomPtrDiff;
typedef uintptr_t VenomUPtr;
typedef intptr_t  VenomIPtr;

typedef bool      VenomBool;

#define VENOM_TRUE  true
#define VENOM_FALSE false

/* ============================================================================
 * NUMERIC LIMITS
 * ============================================================================ */

#define VENOM_I8_MIN   INT8_MIN
#define VENOM_I8_MAX   INT8_MAX
#define VENOM_I16_MIN  INT16_MIN
#define VENOM_I16_MAX  INT16_MAX
#define VENOM_I32_MIN  INT32_MIN
#define VENOM_I32_MAX  INT32_MAX
#define VENOM_I64_MIN  INT64_MIN
#define VENOM_I64_MAX  INT64_MAX

#define VENOM_U8_MAX   UINT8_MAX
#define VENOM_U16_MAX  UINT16_MAX
#define VENOM_U32_MAX  UINT32_MAX
#define VENOM_U64_MAX  UINT64_MAX

/* ============================================================================
 * GEOMETRY TYPES
 * ============================================================================ */

typedef struct VenomPoint {
    VenomI32 x;
    VenomI32 y;
} VenomPoint;

typedef struct VenomPointF {
    VenomF32 x;
    VenomF32 y;
} VenomPointF;

typedef struct VenomSize2D {
    VenomU32 width;
    VenomU32 height;
} VenomSize2D;

typedef struct VenomSize2DF {
    VenomF32 width;
    VenomF32 height;
} VenomSize2DF;

typedef struct VenomRect {
    VenomI32 x;
    VenomI32 y;
    VenomU32 width;
    VenomU32 height;
} VenomRect;

typedef struct VenomRectF {
    VenomF32 x;
    VenomF32 y;
    VenomF32 width;
    VenomF32 height;
} VenomRectF;

typedef struct VenomInsets {
    VenomI32 top;
    VenomI32 right;
    VenomI32 bottom;
    VenomI32 left;
} VenomInsets;

/* ============================================================================
 * GEOMETRY HELPER FUNCTIONS
 * ============================================================================ */

VENOM_INLINE VenomPoint venom_point(VenomI32 x, VenomI32 y) {
    return (VenomPoint){ .x = x, .y = y };
}

VENOM_INLINE VenomSize2D venom_size(VenomU32 width, VenomU32 height) {
    return (VenomSize2D){ .width = width, .height = height };
}

VENOM_INLINE VenomRect venom_rect(VenomI32 x, VenomI32 y, VenomU32 w, VenomU32 h) {
    return (VenomRect){ .x = x, .y = y, .width = w, .height = h };
}

VENOM_INLINE VenomBool venom_rect_contains(const VenomRect* r, VenomI32 x, VenomI32 y) {
    return x >= r->x && x < (r->x + (VenomI32)r->width) &&
           y >= r->y && y < (r->y + (VenomI32)r->height);
}

VENOM_INLINE VenomBool venom_rect_intersects(const VenomRect* a, const VenomRect* b) {
    return !(a->x + (VenomI32)a->width <= b->x ||
             b->x + (VenomI32)b->width <= a->x ||
             a->y + (VenomI32)a->height <= b->y ||
             b->y + (VenomI32)b->height <= a->y);
}

/* ============================================================================
 * COLOR TYPES
 * ============================================================================ */

typedef struct VenomColor {
    VenomU8 r;
    VenomU8 g;
    VenomU8 b;
    VenomU8 a;
} VenomColor;

typedef struct VenomColorF {
    VenomF32 r;
    VenomF32 g;
    VenomF32 b;
    VenomF32 a;
} VenomColorF;

/* Pre-defined colors */
#define VENOM_COLOR_TRANSPARENT ((VenomColor){ 0, 0, 0, 0 })
#define VENOM_COLOR_BLACK       ((VenomColor){ 0, 0, 0, 255 })
#define VENOM_COLOR_WHITE       ((VenomColor){ 255, 255, 255, 255 })
#define VENOM_COLOR_RED         ((VenomColor){ 255, 0, 0, 255 })
#define VENOM_COLOR_GREEN       ((VenomColor){ 0, 255, 0, 255 })
#define VENOM_COLOR_BLUE        ((VenomColor){ 0, 0, 255, 255 })

VENOM_INLINE VenomColor venom_color_rgba(VenomU8 r, VenomU8 g, VenomU8 b, VenomU8 a) {
    return (VenomColor){ .r = r, .g = g, .b = b, .a = a };
}

VENOM_INLINE VenomColor venom_color_rgb(VenomU8 r, VenomU8 g, VenomU8 b) {
    return venom_color_rgba(r, g, b, 255);
}

VENOM_INLINE VenomU32 venom_color_to_u32(VenomColor c) {
    return ((VenomU32)c.a << 24) | ((VenomU32)c.r << 16) | 
           ((VenomU32)c.g << 8) | (VenomU32)c.b;
}

VENOM_INLINE VenomColor venom_color_from_u32(VenomU32 argb) {
    return (VenomColor){
        .a = (VenomU8)((argb >> 24) & 0xFF),
        .r = (VenomU8)((argb >> 16) & 0xFF),
        .g = (VenomU8)((argb >> 8) & 0xFF),
        .b = (VenomU8)(argb & 0xFF)
    };
}

/* ============================================================================
 * UTILITY MACROS
 * ============================================================================ */

#define VENOM_MIN(a, b) ((a) < (b) ? (a) : (b))
#define VENOM_MAX(a, b) ((a) > (b) ? (a) : (b))
#define VENOM_CLAMP(x, lo, hi) VENOM_MIN(VENOM_MAX(x, lo), hi)
#define VENOM_ABS(x) ((x) < 0 ? -(x) : (x))

#define VENOM_ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

#define VENOM_ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))
#define VENOM_ALIGN_DOWN(x, align) ((x) & ~((align) - 1))

#define VENOM_KB(x) ((VenomSize)(x) * 1024)
#define VENOM_MB(x) ((VenomSize)(x) * 1024 * 1024)
#define VENOM_GB(x) ((VenomSize)(x) * 1024 * 1024 * 1024)

/* ============================================================================
 * ASSERTION MACROS
 * ============================================================================ */

#ifdef VENOM_DEBUG
    #include <stdio.h>
    #include <stdlib.h>
    
    #define VENOM_ASSERT(cond) \
        do { \
            if (VENOM_UNLIKELY(!(cond))) { \
                fprintf(stderr, "VENOM ASSERT FAILED: %s\n  at %s:%d in %s()\n", \
                        #cond, __FILE__, __LINE__, __func__); \
                abort(); \
            } \
        } while (0)
    
    #define VENOM_ASSERT_MSG(cond, fmt, ...) \
        do { \
            if (VENOM_UNLIKELY(!(cond))) { \
                fprintf(stderr, "VENOM ASSERT FAILED: %s\n  " fmt "\n  at %s:%d in %s()\n", \
                        #cond, ##__VA_ARGS__, __FILE__, __LINE__, __func__); \
                abort(); \
            } \
        } while (0)
    
    #define VENOM_UNREACHABLE() \
        do { \
            fprintf(stderr, "VENOM UNREACHABLE: at %s:%d in %s()\n", \
                    __FILE__, __LINE__, __func__); \
            abort(); \
        } while (0)
#else
    #define VENOM_ASSERT(cond) ((void)0)
    #define VENOM_ASSERT_MSG(cond, fmt, ...) ((void)0)
    #define VENOM_UNREACHABLE() __builtin_unreachable()
#endif

/* ============================================================================
 * NULL POINTER CHECKS
 * ============================================================================ */

#define VENOM_RETURN_IF_NULL(ptr, ret) \
    do { if (VENOM_UNLIKELY((ptr) == NULL)) return (ret); } while (0)

#define VENOM_RETURN_VOID_IF_NULL(ptr) \
    do { if (VENOM_UNLIKELY((ptr) == NULL)) return; } while (0)

#ifdef __cplusplus
}
#endif

#endif /* VENOM_TYPES_H */
