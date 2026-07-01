/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_ref.h - Reference counting system for automatic memory management
 */

#ifndef VAXP_REF_H
#define VAXP_REF_H

#include "vaxp_types.h"
#include "vaxp_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * REFERENCE COUNTING HEADER
 * ============================================================================ */

/**
 * @brief Destructor function type
 * 
 * Called when reference count reaches zero.
 * Should free all owned resources but NOT the object itself.
 */
typedef void (*VaxpDestructor)(void* self);

/**
 * @brief Reference counting header
 * 
 * Every ref-counted object must have this as its FIRST member.
 * This allows safe casting between object and header.
 */
typedef struct VaxpRefHeader {
    volatile VaxpU32 ref_count;    /* Current reference count */
    VaxpSize         object_size;  /* Size of the full object (for freeing) */
    VaxpDestructor   destructor;   /* Called before freeing */
    const char*       type_name;    /* Type name for debugging */
} VaxpRefHeader;

/**
 * @brief Embed this macro as the FIRST member of any ref-counted struct
 * 
 * Example:
 *   typedef struct MyObject {
 *       VAXP_REF_HEADER;
 *       int my_data;
 *   } MyObject;
 */
#define VAXP_REF_HEADER VaxpRefHeader _vaxp_ref

/* ============================================================================
 * REFERENCE OPERATIONS
 * ============================================================================ */

/**
 * @brief Initialize a ref-counted object
 * 
 * Sets ref_count to 1. Must be called after allocation.
 * 
 * @param obj The object to initialize
 * @param size Size of the object (for proper freeing)
 * @param destructor Function to call before freeing (may be NULL)
 * @param type_name Type name for debugging (should be a string literal)
 */
void vaxp_ref_init(
    void* VAXP_NONNULL obj,
    VaxpSize size,
    VaxpDestructor VAXP_NULLABLE destructor,
    const char* VAXP_NONNULL type_name
);

/**
 * @brief Increment reference count
 * 
 * @param obj The object to reference
 * @return The same object (for chaining)
 */
void* vaxp_ref(void* VAXP_NULLABLE obj);

/**
 * @brief Decrement reference count, freeing if it reaches zero
 * 
 * If ref_count reaches 0:
 *   1. Calls destructor (if set)
 *   2. Frees the memory
 * 
 * @param obj The object to unref (may be NULL)
 */
void vaxp_unref(void* VAXP_NULLABLE obj);

/**
 * @brief Get current reference count
 * 
 * @return Reference count, or 0 if obj is NULL
 */
VaxpU32 vaxp_ref_count(const void* VAXP_NULLABLE obj);

/**
 * @brief Get type name of ref-counted object
 * 
 * @return Type name, or "null" if obj is NULL
 */
const char* vaxp_ref_type_name(const void* VAXP_NULLABLE obj);

/* ============================================================================
 * CONVENIENCE MACROS
 * ============================================================================ */

/**
 * @brief Allocate and initialize a ref-counted object
 * 
 * Usage:
 *   MyObject* obj = VAXP_REF_NEW(MyObject, my_destructor);
 */
#define VAXP_REF_NEW(T, destructor) \
    _vaxp_ref_new_impl(sizeof(T), (destructor), #T)

/**
 * @brief Initialize an already-allocated ref-counted object
 */
#define VAXP_REF_INIT(obj, destructor) \
    vaxp_ref_init((obj), sizeof(*(obj)), (destructor), VAXP_STRINGIFY(typeof(*(obj))))

/**
 * @brief Assign with proper ref counting
 * 
 * Unrefs the old value and refs the new value.
 * Handles self-assignment correctly.
 * 
 * Usage:
 *   VAXP_REF_ASSIGN(my_ptr, new_value);
 */
#define VAXP_REF_ASSIGN(lvalue, rvalue) \
    do { \
        void* _new = (rvalue); \
        void* _old = (lvalue); \
        if (_old != _new) { \
            vaxp_ref(_new); \
            vaxp_unref(_old); \
            (lvalue) = _new; \
        } \
    } while (0)

/**
 * @brief Clear a ref-counted pointer (unref and set to NULL)
 */
#define VAXP_REF_CLEAR(lvalue) \
    do { \
        vaxp_unref(lvalue); \
        (lvalue) = NULL; \
    } while (0)

/**
 * @brief Helper to create with type name
 */
VAXP_WARN_UNUSED
void* _vaxp_ref_new_impl(VaxpSize size, VaxpDestructor destructor, const char* type_name);

/* ============================================================================
 * WEAK REFERENCES (Optional)
 * ============================================================================ */

/**
 * @brief Weak reference container
 * 
 * A weak reference does not prevent the object from being freed.
 * Must check if the object is still alive before using.
 */
typedef struct VaxpWeakRef {
    void* object;
    VaxpU32* control_block;  /* Shared with strong refs */
} VaxpWeakRef;

/**
 * @brief Create a weak reference from a strong reference
 */
VaxpWeakRef vaxp_weak_ref_create(void* VAXP_NULLABLE obj);

/**
 * @brief Attempt to upgrade weak ref to strong ref
 * 
 * @return Strong reference if object still alive, NULL if freed
 */
void* vaxp_weak_ref_upgrade(VaxpWeakRef* weak);

/**
 * @brief Check if weak reference is still valid
 */
VaxpBool vaxp_weak_ref_is_valid(const VaxpWeakRef* weak);

/**
 * @brief Release weak reference
 */
void vaxp_weak_ref_clear(VaxpWeakRef* weak);

/* ============================================================================
 * DEBUG UTILITIES
 * ============================================================================ */

#ifdef VAXP_DEBUG

/**
 * @brief Print reference count info for debugging
 */
void vaxp_ref_debug_print(const void* obj);

#endif

#ifdef __cplusplus
}
#endif

#endif /* VAXP_REF_H */
