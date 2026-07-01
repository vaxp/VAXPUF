/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_memory.h - Memory allocation with leak tracking
 */

#ifndef VAXP_MEMORY_H
#define VAXP_MEMORY_H

#include "vaxp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * ALLOCATOR INTERFACE
 * ============================================================================ */

/**
 * @brief Custom allocator interface
 * 
 * Allows users to provide their own memory allocation functions.
 * All functions must be thread-safe if multi-threading is used.
 */
typedef struct VaxpAllocator {
    void* (*alloc)(VaxpSize size, void* user_data);
    void* (*realloc)(void* ptr, VaxpSize old_size, VaxpSize new_size, void* user_data);
    void  (*free)(void* ptr, VaxpSize size, void* user_data);
    void* user_data;
} VaxpAllocator;

/**
 * @brief Set the global allocator
 * 
 * Must be called before any allocations. If not called, uses system malloc/free.
 * 
 * @param allocator The allocator to use (NULL to reset to default)
 */
void vaxp_set_allocator(const VaxpAllocator* VAXP_NULLABLE allocator);

/**
 * @brief Get the current allocator
 */
const VaxpAllocator* vaxp_get_allocator(void);

/* ============================================================================
 * ALLOCATION FUNCTIONS
 * ============================================================================ */

/**
 * @brief Allocate memory
 * 
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory, or NULL on failure
 */
VAXP_WARN_UNUSED VAXP_MALLOC
VAXP_OWNED void* vaxp_alloc(VaxpSize size);

/**
 * @brief Allocate and zero-initialize memory
 */
VAXP_WARN_UNUSED VAXP_MALLOC
VAXP_OWNED void* vaxp_alloc_zeroed(VaxpSize size);

/**
 * @brief Reallocate memory
 * 
 * @param ptr Pointer to existing allocation (may be NULL)
 * @param old_size Original size of allocation
 * @param new_size New size requested
 * @return Pointer to reallocated memory, or NULL on failure
 */
VAXP_WARN_UNUSED
VAXP_OWNED void* vaxp_realloc(VAXP_OWNED void* ptr, VaxpSize old_size, VaxpSize new_size);

/**
 * @brief Free memory
 * 
 * @param ptr Pointer to free (may be NULL)
 * @param size Size of the allocation being freed
 */
void vaxp_free(VAXP_OWNED void* ptr, VaxpSize size);

/* ============================================================================
 * TYPED ALLOCATION MACROS
 * ============================================================================ */

/**
 * @brief Allocate a single object of type T
 */
#define VAXP_NEW(T) ((T*)vaxp_alloc_zeroed(sizeof(T)))

/**
 * @brief Allocate an array of N objects of type T
 */
#define VAXP_NEW_ARRAY(T, n) ((T*)vaxp_alloc_zeroed(sizeof(T) * (n)))

/**
 * @brief Free a single object of type T
 */
#define VAXP_DELETE(T, ptr) vaxp_free((ptr), sizeof(T))

/**
 * @brief Free an array of N objects of type T
 */
#define VAXP_DELETE_ARRAY(T, ptr, n) vaxp_free((ptr), sizeof(T) * (n))

/**
 * @brief Reallocate an array from old_n to new_n objects of type T
 */
#define VAXP_REALLOC_ARRAY(T, ptr, old_n, new_n) \
    ((T*)vaxp_realloc((ptr), sizeof(T) * (old_n), sizeof(T) * (new_n)))

/* ============================================================================
 * MEMORY TRACKING (Debug Mode)
 * ============================================================================ */

#ifdef VAXP_DEBUG

/**
 * @brief Memory allocation statistics
 */
typedef struct VaxpMemoryStats {
    VaxpSize total_allocated;
    VaxpSize total_freed;
    VaxpSize current_usage;
    VaxpSize peak_usage;
    VaxpU64  allocation_count;
    VaxpU64  free_count;
} VaxpMemoryStats;

/**
 * @brief Get current memory statistics
 */
VaxpMemoryStats vaxp_memory_get_stats(void);

/**
 * @brief Print memory report to stderr
 * 
 * Shows current allocations, useful for detecting leaks
 */
void vaxp_memory_report(void);

/**
 * @brief Check for memory leaks
 * 
 * @return Number of unfreed allocations
 */
VaxpU64 vaxp_memory_check_leaks(void);

/**
 * @brief Enable/disable allocation tracking
 * 
 * Tracking adds overhead but provides detailed leak information.
 */
void vaxp_memory_set_tracking(VaxpBool enabled);

#endif /* VAXP_DEBUG */

/* ============================================================================
 * MEMORY UTILITIES
 * ============================================================================ */

/**
 * @brief Copy memory
 */
void vaxp_memcpy(void* VAXP_NONNULL dst, const void* VAXP_NONNULL src, VaxpSize size);

/**
 * @brief Move memory (handles overlapping regions)
 */
void vaxp_memmove(void* VAXP_NONNULL dst, const void* VAXP_NONNULL src, VaxpSize size);

/**
 * @brief Set memory to a value
 */
void vaxp_memset(void* VAXP_NONNULL dst, VaxpU8 value, VaxpSize size);

/**
 * @brief Zero memory
 */
VAXP_INLINE void vaxp_memzero(void* VAXP_NONNULL dst, VaxpSize size) {
    vaxp_memset(dst, 0, size);
}

/**
 * @brief Compare memory
 * 
 * @return 0 if equal, negative if a < b, positive if a > b
 */
VaxpI32 vaxp_memcmp(const void* VAXP_NONNULL a, const void* VAXP_NONNULL b, VaxpSize size);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_MEMORY_H */
