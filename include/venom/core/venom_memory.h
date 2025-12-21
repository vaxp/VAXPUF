/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_memory.h - Memory allocation with leak tracking
 */

#ifndef VENOM_MEMORY_H
#define VENOM_MEMORY_H

#include "venom_types.h"

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
typedef struct VenomAllocator {
    void* (*alloc)(VenomSize size, void* user_data);
    void* (*realloc)(void* ptr, VenomSize old_size, VenomSize new_size, void* user_data);
    void  (*free)(void* ptr, VenomSize size, void* user_data);
    void* user_data;
} VenomAllocator;

/**
 * @brief Set the global allocator
 * 
 * Must be called before any allocations. If not called, uses system malloc/free.
 * 
 * @param allocator The allocator to use (NULL to reset to default)
 */
void venom_set_allocator(const VenomAllocator* VENOM_NULLABLE allocator);

/**
 * @brief Get the current allocator
 */
const VenomAllocator* venom_get_allocator(void);

/* ============================================================================
 * ALLOCATION FUNCTIONS
 * ============================================================================ */

/**
 * @brief Allocate memory
 * 
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory, or NULL on failure
 */
VENOM_WARN_UNUSED VENOM_MALLOC
VENOM_OWNED void* venom_alloc(VenomSize size);

/**
 * @brief Allocate and zero-initialize memory
 */
VENOM_WARN_UNUSED VENOM_MALLOC
VENOM_OWNED void* venom_alloc_zeroed(VenomSize size);

/**
 * @brief Reallocate memory
 * 
 * @param ptr Pointer to existing allocation (may be NULL)
 * @param old_size Original size of allocation
 * @param new_size New size requested
 * @return Pointer to reallocated memory, or NULL on failure
 */
VENOM_WARN_UNUSED
VENOM_OWNED void* venom_realloc(VENOM_OWNED void* ptr, VenomSize old_size, VenomSize new_size);

/**
 * @brief Free memory
 * 
 * @param ptr Pointer to free (may be NULL)
 * @param size Size of the allocation being freed
 */
void venom_free(VENOM_OWNED void* ptr, VenomSize size);

/* ============================================================================
 * TYPED ALLOCATION MACROS
 * ============================================================================ */

/**
 * @brief Allocate a single object of type T
 */
#define VENOM_NEW(T) ((T*)venom_alloc_zeroed(sizeof(T)))

/**
 * @brief Allocate an array of N objects of type T
 */
#define VENOM_NEW_ARRAY(T, n) ((T*)venom_alloc_zeroed(sizeof(T) * (n)))

/**
 * @brief Free a single object of type T
 */
#define VENOM_DELETE(T, ptr) venom_free((ptr), sizeof(T))

/**
 * @brief Free an array of N objects of type T
 */
#define VENOM_DELETE_ARRAY(T, ptr, n) venom_free((ptr), sizeof(T) * (n))

/**
 * @brief Reallocate an array from old_n to new_n objects of type T
 */
#define VENOM_REALLOC_ARRAY(T, ptr, old_n, new_n) \
    ((T*)venom_realloc((ptr), sizeof(T) * (old_n), sizeof(T) * (new_n)))

/* ============================================================================
 * MEMORY TRACKING (Debug Mode)
 * ============================================================================ */

#ifdef VENOM_DEBUG

/**
 * @brief Memory allocation statistics
 */
typedef struct VenomMemoryStats {
    VenomSize total_allocated;
    VenomSize total_freed;
    VenomSize current_usage;
    VenomSize peak_usage;
    VenomU64  allocation_count;
    VenomU64  free_count;
} VenomMemoryStats;

/**
 * @brief Get current memory statistics
 */
VenomMemoryStats venom_memory_get_stats(void);

/**
 * @brief Print memory report to stderr
 * 
 * Shows current allocations, useful for detecting leaks
 */
void venom_memory_report(void);

/**
 * @brief Check for memory leaks
 * 
 * @return Number of unfreed allocations
 */
VenomU64 venom_memory_check_leaks(void);

/**
 * @brief Enable/disable allocation tracking
 * 
 * Tracking adds overhead but provides detailed leak information.
 */
void venom_memory_set_tracking(VenomBool enabled);

#endif /* VENOM_DEBUG */

/* ============================================================================
 * MEMORY UTILITIES
 * ============================================================================ */

/**
 * @brief Copy memory
 */
void venom_memcpy(void* VENOM_NONNULL dst, const void* VENOM_NONNULL src, VenomSize size);

/**
 * @brief Move memory (handles overlapping regions)
 */
void venom_memmove(void* VENOM_NONNULL dst, const void* VENOM_NONNULL src, VenomSize size);

/**
 * @brief Set memory to a value
 */
void venom_memset(void* VENOM_NONNULL dst, VenomU8 value, VenomSize size);

/**
 * @brief Zero memory
 */
VENOM_INLINE void venom_memzero(void* VENOM_NONNULL dst, VenomSize size) {
    venom_memset(dst, 0, size);
}

/**
 * @brief Compare memory
 * 
 * @return 0 if equal, negative if a < b, positive if a > b
 */
VenomI32 venom_memcmp(const void* VENOM_NONNULL a, const void* VENOM_NONNULL b, VenomSize size);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_MEMORY_H */
