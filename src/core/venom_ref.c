/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_ref.c - Reference counting implementation
 */

#include "venom/core/venom_ref.h"
#include <stdatomic.h>

#ifdef VENOM_DEBUG
#include <stdio.h>
#endif

/* ============================================================================
 * REFERENCE COUNTING IMPLEMENTATION
 * ============================================================================ */

void venom_ref_init(
    void* obj,
    VenomSize size,
    VenomDestructor destructor,
    const char* type_name
) {
    VENOM_ASSERT(obj != NULL);
    VENOM_ASSERT(type_name != NULL);
    
    VenomRefHeader* header = (VenomRefHeader*)obj;
    header->ref_count = 1;
    header->object_size = size;
    header->destructor = destructor;
    header->type_name = type_name;
}

void* venom_ref(void* obj) {
    if (obj == NULL) return NULL;
    
    VenomRefHeader* header = (VenomRefHeader*)obj;
    
    /* Atomic increment for thread safety */
    VenomU32 old_count = atomic_fetch_add((atomic_uint*)&header->ref_count, 1);
    
    /* Sanity check: shouldn't be refing a dead object */
    VENOM_ASSERT_MSG(old_count > 0, "Attempting to ref already-freed %s", header->type_name);
    VENOM_ASSERT_MSG(old_count < VENOM_U32_MAX, "Reference count overflow on %s", header->type_name);
    
    return obj;
}

void venom_unref(void* obj) {
    if (obj == NULL) return;
    
    VenomRefHeader* header = (VenomRefHeader*)obj;
    
    /* Sanity check */
    VENOM_ASSERT_MSG(header->ref_count > 0, "Double-free of %s", header->type_name);
    
    /* Atomic decrement */
    VenomU32 old_count = atomic_fetch_sub((atomic_uint*)&header->ref_count, 1);
    
    if (old_count == 1) {
        /* We were the last reference - destroy and free */
        
#ifdef VENOM_DEBUG
        /* Debug: Print destruction info */
        fprintf(stderr, "[VENOM_REF] Destroying %s at %p (size=%zu)\n",
                header->type_name, obj, header->object_size);
#endif
        
        /* Call destructor if set */
        if (header->destructor != NULL) {
            header->destructor(obj);
        }
        
        /* Free the memory */
        VenomSize size = header->object_size;
        venom_free(obj, size);
    }
}

VenomU32 venom_ref_count(const void* obj) {
    if (obj == NULL) return 0;
    
    const VenomRefHeader* header = (const VenomRefHeader*)obj;
    return atomic_load((const atomic_uint*)&header->ref_count);
}

const char* venom_ref_type_name(const void* obj) {
    if (obj == NULL) return "null";
    
    const VenomRefHeader* header = (const VenomRefHeader*)obj;
    return header->type_name;
}

void* _venom_ref_new_impl(VenomSize size, VenomDestructor destructor, const char* type_name) {
    void* obj = venom_alloc_zeroed(size);
    if (obj == NULL) return NULL;
    
    venom_ref_init(obj, size, destructor, type_name);
    
#ifdef VENOM_DEBUG
    fprintf(stderr, "[VENOM_REF] Created %s at %p (size=%zu)\n",
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

VenomWeakRef venom_weak_ref_create(void* obj) {
    VenomWeakRef weak = { .object = obj, .control_block = NULL };
    return weak;
}

void* venom_weak_ref_upgrade(VenomWeakRef* weak) {
    if (weak == NULL || weak->object == NULL) return NULL;
    
    /* Simple implementation: just check ref count */
    VenomRefHeader* header = (VenomRefHeader*)weak->object;
    
    /* Try to atomically increment if not zero */
    VenomU32 count = atomic_load((atomic_uint*)&header->ref_count);
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

VenomBool venom_weak_ref_is_valid(const VenomWeakRef* weak) {
    if (weak == NULL || weak->object == NULL) return VENOM_FALSE;
    
    const VenomRefHeader* header = (const VenomRefHeader*)weak->object;
    return atomic_load((const atomic_uint*)&header->ref_count) > 0;
}

void venom_weak_ref_clear(VenomWeakRef* weak) {
    if (weak != NULL) {
        weak->object = NULL;
        weak->control_block = NULL;
    }
}

/* ============================================================================
 * DEBUG UTILITIES
 * ============================================================================ */

#ifdef VENOM_DEBUG

void venom_ref_debug_print(const void* obj) {
    if (obj == NULL) {
        fprintf(stderr, "[VENOM_REF] Object: (null)\n");
        return;
    }
    
    const VenomRefHeader* header = (const VenomRefHeader*)obj;
    fprintf(stderr, "[VENOM_REF] Object: %p\n", obj);
    fprintf(stderr, "  Type:      %s\n", header->type_name);
    fprintf(stderr, "  RefCount:  %u\n", header->ref_count);
    fprintf(stderr, "  Size:      %zu bytes\n", header->object_size);
    fprintf(stderr, "  Destructor: %s\n", header->destructor ? "yes" : "no");
}

#endif
