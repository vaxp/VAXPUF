/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_ref.c - Reference counting implementation
 */

#include "vaxp/core/vaxp_ref.h"
#include <stdatomic.h>

#ifdef VAXP_DEBUG
#include <stdio.h>
#endif

/* ============================================================================
 * REFERENCE COUNTING IMPLEMENTATION
 * ============================================================================ */

void vaxp_ref_init(
    void* obj,
    VaxpSize size,
    VaxpDestructor destructor,
    const char* type_name
) {
    VAXP_ASSERT(obj != NULL);
    VAXP_ASSERT(type_name != NULL);
    
    VaxpRefHeader* header = (VaxpRefHeader*)obj;
    header->ref_count = 1;
    header->object_size = size;
    header->destructor = destructor;
    header->type_name = type_name;
}

void* vaxp_ref(void* obj) {
    if (obj == NULL) return NULL;
    
    VaxpRefHeader* header = (VaxpRefHeader*)obj;
    
    /* Atomic increment for thread safety */
    VaxpU32 old_count = atomic_fetch_add((atomic_uint*)&header->ref_count, 1);
    
    /* Sanity check: shouldn't be refing a dead object */
    VAXP_ASSERT_MSG(old_count > 0, "Attempting to ref already-freed %s", header->type_name);
    VAXP_ASSERT_MSG(old_count < VAXP_U32_MAX, "Reference count overflow on %s", header->type_name);
    
    return obj;
}

void vaxp_unref(void* obj) {
    if (obj == NULL) return;
    
    VaxpRefHeader* header = (VaxpRefHeader*)obj;
    
    /* Sanity check */
    VAXP_ASSERT_MSG(header->ref_count > 0, "Double-free of %s", header->type_name);
    
    /* Atomic decrement */
    VaxpU32 old_count = atomic_fetch_sub((atomic_uint*)&header->ref_count, 1);
    
    if (old_count == 1) {
        /* We were the last reference - destroy and free */
        
#ifdef VAXP_DEBUG
        /* Debug: Print destruction info */
        fprintf(stderr, "[VAXP_REF] Destroying %s at %p (size=%zu)\n",
                header->type_name, obj, header->object_size);
#endif
        
        /* Call destructor if set */
        if (header->destructor != NULL) {
            header->destructor(obj);
        }
        
        /* Free the memory */
        VaxpSize size = header->object_size;
        vaxp_free(obj, size);
    }
}

VaxpU32 vaxp_ref_count(const void* obj) {
    if (obj == NULL) return 0;
    
    const VaxpRefHeader* header = (const VaxpRefHeader*)obj;
    return atomic_load((const atomic_uint*)&header->ref_count);
}

const char* vaxp_ref_type_name(const void* obj) {
    if (obj == NULL) return "null";
    
    const VaxpRefHeader* header = (const VaxpRefHeader*)obj;
    return header->type_name;
}

void* _vaxp_ref_new_impl(VaxpSize size, VaxpDestructor destructor, const char* type_name) {
    void* obj = vaxp_alloc_zeroed(size);
    if (obj == NULL) return NULL;
    
    vaxp_ref_init(obj, size, destructor, type_name);
    
#ifdef VAXP_DEBUG
    fprintf(stderr, "[VAXP_REF] Created %s at %p (size=%zu)\n",
            type_name, obj, size);
#endif
    
    return obj;
}

/* ============================================================================
 * WEAK REFERENCE IMPLEMENTATION
 * ============================================================================ */

/* 
 * TODO: Full weak reference implementation requires a control block
 * that outlives the object. For now, provide a simple version.
 */

VaxpWeakRef vaxp_weak_ref_create(void* obj) {
    VaxpWeakRef weak = { .object = obj, .control_block = NULL };
    return weak;
}

void* vaxp_weak_ref_upgrade(VaxpWeakRef* weak) {
    if (weak == NULL || weak->object == NULL) return NULL;
    
    /* Simple implementation: just check ref count */
    VaxpRefHeader* header = (VaxpRefHeader*)weak->object;
    
    /* Try to atomically increment if not zero */
    VaxpU32 count = atomic_load((atomic_uint*)&header->ref_count);
    while (count > 0) {
        if (atomic_compare_exchange_weak(
                (atomic_uint*)&header->ref_count,
                &count,
                count + 1)) {
            return weak->object;
        }
    }
    
    /* Object was freed */
    weak->object = NULL;
    return NULL;
}

VaxpBool vaxp_weak_ref_is_valid(const VaxpWeakRef* weak) {
    if (weak == NULL || weak->object == NULL) return VAXP_FALSE;
    
    const VaxpRefHeader* header = (const VaxpRefHeader*)weak->object;
    return atomic_load((const atomic_uint*)&header->ref_count) > 0;
}

void vaxp_weak_ref_clear(VaxpWeakRef* weak) {
    if (weak != NULL) {
        weak->object = NULL;
        weak->control_block = NULL;
    }
}

/* ============================================================================
 * DEBUG UTILITIES
 * ============================================================================ */

#ifdef VAXP_DEBUG

void vaxp_ref_debug_print(const void* obj) {
    if (obj == NULL) {
        fprintf(stderr, "[VAXP_REF] Object: (null)\n");
        return;
    }
    
    const VaxpRefHeader* header = (const VaxpRefHeader*)obj;
    fprintf(stderr, "[VAXP_REF] Object: %p\n", obj);
    fprintf(stderr, "  Type:      %s\n", header->type_name);
    fprintf(stderr, "  RefCount:  %u\n", header->ref_count);
    fprintf(stderr, "  Size:      %zu bytes\n", header->object_size);
    fprintf(stderr, "  Destructor: %s\n", header->destructor ? "yes" : "no");
}

#endif
