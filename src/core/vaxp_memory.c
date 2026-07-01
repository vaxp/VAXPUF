/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_memory.c - Memory allocation implementation
 */

#include "vaxp/core/vaxp_memory.h"
#include <stdlib.h>
#include <string.h>

#ifdef VAXP_DEBUG
#include <stdio.h>
#include <pthread.h>
#endif

/* ============================================================================
 * DEFAULT ALLOCATOR
 * ============================================================================ */

static void* default_alloc(VaxpSize size, void* user_data) {
    (void)user_data;
    return malloc(size);
}

static void* default_realloc(void* ptr, VaxpSize old_size, VaxpSize new_size, void* user_data) {
    (void)user_data;
    (void)old_size;
    return realloc(ptr, new_size);
}

static void default_free(void* ptr, VaxpSize size, void* user_data) {
    (void)user_data;
    (void)size;
    free(ptr);
}

static const VaxpAllocator g_default_allocator = {
    .alloc = default_alloc,
    .realloc = default_realloc,
    .free = default_free,
    .user_data = NULL
};

static const VaxpAllocator* g_allocator = &g_default_allocator;

/* ============================================================================
 * DEBUG MODE: ALLOCATION TRACKING
 * ============================================================================ */

#ifdef VAXP_DEBUG

typedef struct AllocationRecord {
    void* ptr;
    VaxpSize size;
    const char* file;
    int line;
    struct AllocationRecord* next;
} AllocationRecord;

static struct {
    pthread_mutex_t lock;
    AllocationRecord* records;
    VaxpMemoryStats stats;
    VaxpBool tracking_enabled;
    VaxpBool initialized;
} g_memory_tracker = {
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .records = NULL,
    .stats = {0},
    .tracking_enabled = VAXP_TRUE,
    .initialized = VAXP_FALSE
};

static void tracker_init(void) {
    if (!g_memory_tracker.initialized) {
        pthread_mutex_init(&g_memory_tracker.lock, NULL);
        g_memory_tracker.initialized = VAXP_TRUE;
    }
}

static void tracker_add(void* ptr, VaxpSize size) {
    if (!g_memory_tracker.tracking_enabled) return;
    tracker_init();
    
    AllocationRecord* record = (AllocationRecord*)malloc(sizeof(AllocationRecord));
    if (!record) return;
    
    record->ptr = ptr;
    record->size = size;
    record->file = NULL;
    record->line = 0;
    
    pthread_mutex_lock(&g_memory_tracker.lock);
    record->next = g_memory_tracker.records;
    g_memory_tracker.records = record;
    
    g_memory_tracker.stats.total_allocated += size;
    g_memory_tracker.stats.current_usage += size;
    g_memory_tracker.stats.allocation_count++;
    
    if (g_memory_tracker.stats.current_usage > g_memory_tracker.stats.peak_usage) {
        g_memory_tracker.stats.peak_usage = g_memory_tracker.stats.current_usage;
    }
    pthread_mutex_unlock(&g_memory_tracker.lock);
}

static void tracker_remove(void* ptr, VaxpSize size) {
    if (!g_memory_tracker.tracking_enabled || !ptr) return;
    
    pthread_mutex_lock(&g_memory_tracker.lock);
    
    AllocationRecord** current = &g_memory_tracker.records;
    while (*current) {
        if ((*current)->ptr == ptr) {
            AllocationRecord* to_free = *current;
            *current = to_free->next;
            
            g_memory_tracker.stats.total_freed += to_free->size;
            g_memory_tracker.stats.current_usage -= to_free->size;
            g_memory_tracker.stats.free_count++;
            
            free(to_free);
            pthread_mutex_unlock(&g_memory_tracker.lock);
            return;
        }
        current = &(*current)->next;
    }
    
    /* Pointer not found in records - this is a bug! */
    fprintf(stderr, "[VAXP] WARNING: Freeing untracked pointer %p (size=%zu)\n", ptr, size);
    pthread_mutex_unlock(&g_memory_tracker.lock);
}

VaxpMemoryStats vaxp_memory_get_stats(void) {
    VaxpMemoryStats stats;
    pthread_mutex_lock(&g_memory_tracker.lock);
    stats = g_memory_tracker.stats;
    pthread_mutex_unlock(&g_memory_tracker.lock);
    return stats;
}

void vaxp_memory_report(void) {
    pthread_mutex_lock(&g_memory_tracker.lock);
    
    fprintf(stderr, "\n=== VAXP MEMORY REPORT ===\n");
    fprintf(stderr, "Total allocated: %zu bytes\n", g_memory_tracker.stats.total_allocated);
    fprintf(stderr, "Total freed:     %zu bytes\n", g_memory_tracker.stats.total_freed);
    fprintf(stderr, "Current usage:   %zu bytes\n", g_memory_tracker.stats.current_usage);
    fprintf(stderr, "Peak usage:      %zu bytes\n", g_memory_tracker.stats.peak_usage);
    fprintf(stderr, "Allocations:     %llu\n", (unsigned long long)g_memory_tracker.stats.allocation_count);
    fprintf(stderr, "Frees:           %llu\n", (unsigned long long)g_memory_tracker.stats.free_count);
    
    if (g_memory_tracker.records) {
        fprintf(stderr, "\n--- LEAKED ALLOCATIONS ---\n");
        VaxpU64 leak_count = 0;
        AllocationRecord* record = g_memory_tracker.records;
        while (record) {
            fprintf(stderr, "  Leak: %p (%zu bytes)\n", record->ptr, record->size);
            leak_count++;
            record = record->next;
        }
        fprintf(stderr, "Total leaks: %llu\n", (unsigned long long)leak_count);
    } else {
        fprintf(stderr, "\nNo memory leaks detected!\n");
    }
    fprintf(stderr, "===========================\n\n");
    
    pthread_mutex_unlock(&g_memory_tracker.lock);
}

VaxpU64 vaxp_memory_check_leaks(void) {
    VaxpU64 count = 0;
    pthread_mutex_lock(&g_memory_tracker.lock);
    
    AllocationRecord* record = g_memory_tracker.records;
    while (record) {
        count++;
        record = record->next;
    }
    
    pthread_mutex_unlock(&g_memory_tracker.lock);
    return count;
}

void vaxp_memory_set_tracking(VaxpBool enabled) {
    g_memory_tracker.tracking_enabled = enabled;
}

#endif /* VAXP_DEBUG */

/* ============================================================================
 * PUBLIC API IMPLEMENTATION
 * ============================================================================ */

void vaxp_set_allocator(const VaxpAllocator* allocator) {
    if (allocator) {
        g_allocator = allocator;
    } else {
        g_allocator = &g_default_allocator;
    }
}

const VaxpAllocator* vaxp_get_allocator(void) {
    return g_allocator;
}

void* vaxp_alloc(VaxpSize size) {
    if (size == 0) return NULL;
    
    void* ptr = g_allocator->alloc(size, g_allocator->user_data);
    
#ifdef VAXP_DEBUG
    if (ptr) {
        tracker_add(ptr, size);
    }
#endif
    
    return ptr;
}

void* vaxp_alloc_zeroed(VaxpSize size) {
    void* ptr = vaxp_alloc(size);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

void* vaxp_realloc(void* ptr, VaxpSize old_size, VaxpSize new_size) {
    if (new_size == 0) {
        vaxp_free(ptr, old_size);
        return NULL;
    }
    
    if (!ptr) {
        return vaxp_alloc(new_size);
    }
    
#ifdef VAXP_DEBUG
    tracker_remove(ptr, old_size);
#endif
    
    void* new_ptr = g_allocator->realloc(ptr, old_size, new_size, g_allocator->user_data);
    
#ifdef VAXP_DEBUG
    if (new_ptr) {
        tracker_add(new_ptr, new_size);
    }
#endif
    
    return new_ptr;
}

void vaxp_free(void* ptr, VaxpSize size) {
    if (!ptr) return;
    
#ifdef VAXP_DEBUG
    tracker_remove(ptr, size);
#endif
    
    g_allocator->free(ptr, size, g_allocator->user_data);
}

/* ============================================================================
 * MEMORY UTILITIES
 * ============================================================================ */

void vaxp_memcpy(void* dst, const void* src, VaxpSize size) {
    VAXP_ASSERT(dst != NULL);
    VAXP_ASSERT(src != NULL);
    memcpy(dst, src, size);
}

void vaxp_memmove(void* dst, const void* src, VaxpSize size) {
    VAXP_ASSERT(dst != NULL);
    VAXP_ASSERT(src != NULL);
    memmove(dst, src, size);
}

void vaxp_memset(void* dst, VaxpU8 value, VaxpSize size) {
    VAXP_ASSERT(dst != NULL);
    memset(dst, value, size);
}

VaxpI32 vaxp_memcmp(const void* a, const void* b, VaxpSize size) {
    VAXP_ASSERT(a != NULL);
    VAXP_ASSERT(b != NULL);
    return memcmp(a, b, size);
}
