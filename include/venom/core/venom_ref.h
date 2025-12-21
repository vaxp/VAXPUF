/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_ref.h - Reference counting system for automatic memory management
 */

#ifndef VENOM_REF_H
#define VENOM_REF_H

#include "venom_types.h"
#include "venom_memory.h"

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
typedef void (*VenomDestructor)(void* self);

/**
 * @brief Reference counting header
 * 
 * Every ref-counted object must have this as its FIRST member.
 * This allows safe casting between object and header.
 */
typedef struct VenomRefHeader {
    volatile VenomU32 ref_count;    /* Current reference count */
    VenomSize         object_size;  /* Size of the full object (for freeing) */
    VenomDestructor   destructor;   /* Called before freeing */
    const char*       type_name;    /* Type name for debugging */
} VenomRefHeader;

/**
 * @brief Embed this macro as the FIRST member of any ref-counted struct
 * 
 * Example:
 *   typedef struct MyObject {
 *       VENOM_REF_HEADER;
 *       int my_data;
 *   } MyObject;
 */
#define VENOM_REF_HEADER VenomRefHeader _venom_ref

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
void venom_ref_init(
    void* VENOM_NONNULL obj,
    VenomSize size,
    VenomDestructor VENOM_NULLABLE destructor,
    const char* VENOM_NONNULL type_name
);

/**
 * @brief Increment reference count
 * 
 * @param obj The object to reference
 * @return The same object (for chaining)
 */
void* venom_ref(void* VENOM_NULLABLE obj);

/**
 * @brief Decrement reference count, freeing if it reaches zero
 * 
 * If ref_count reaches 0:
 *   1. Calls destructor (if set)
 *   2. Frees the memory
 * 
 * @param obj The object to unref (may be NULL)
 */
void venom_unref(void* VENOM_NULLABLE obj);

/**
 * @brief Get current reference count
 * 
 * @return Reference count, or 0 if obj is NULL
 */
VenomU32 venom_ref_count(const void* VENOM_NULLABLE obj);

/**
 * @brief Get type name of ref-counted object
 * 
 * @return Type name, or "null" if obj is NULL
 */
const char* venom_ref_type_name(const void* VENOM_NULLABLE obj);

/* ============================================================================
 * CONVENIENCE MACROS
 * ============================================================================ */

/**
 * @brief Allocate and initialize a ref-counted object
 * 
 * Usage:
 *   MyObject* obj = VENOM_REF_NEW(MyObject, my_destructor);
 */
#define VENOM_REF_NEW(T, destructor) \
    _venom_ref_new_impl(sizeof(T), (destructor), #T)

/**
 * @brief Initialize an already-allocated ref-counted object
 */
#define VENOM_REF_INIT(obj, destructor) \
    venom_ref_init((obj), sizeof(*(obj)), (destructor), VENOM_STRINGIFY(typeof(*(obj))))

/**
 * @brief Assign with proper ref counting
 * 
 * Unrefs the old value and refs the new value.
 * Handles self-assignment correctly.
 * 
 * Usage:
 *   VENOM_REF_ASSIGN(my_ptr, new_value);
 */
#define VENOM_REF_ASSIGN(lvalue, rvalue) \
    do { \
        void* _new = (rvalue); \
        void* _old = (lvalue); \
        if (_old != _new) { \
            venom_ref(_new); \
            venom_unref(_old); \
            (lvalue) = _new; \
        } \
    } while (0)

/**
 * @brief Clear a ref-counted pointer (unref and set to NULL)
 */
#define VENOM_REF_CLEAR(lvalue) \
    do { \
        venom_unref(lvalue); \
        (lvalue) = NULL; \
    } while (0)

/**
 * @brief Helper to create with type name
 */
VENOM_WARN_UNUSED
void* _venom_ref_new_impl(VenomSize size, VenomDestructor destructor, const char* type_name);

/* ============================================================================
 * WEAK REFERENCES (Optional)
 * ============================================================================ */

/**
 * @brief Weak reference container
 * 
 * A weak reference does not prevent the object from being freed.
 * Must check if the object is still alive before using.
 */
typedef struct VenomWeakRef {
    void* object;
    VenomU32* control_block;  /* Shared with strong refs */
} VenomWeakRef;

/**
 * @brief Create a weak reference from a strong reference
 */
VenomWeakRef venom_weak_ref_create(void* VENOM_NULLABLE obj);

/**
 * @brief Attempt to upgrade weak ref to strong ref
 * 
 * @return Strong reference if object still alive, NULL if freed
 */
void* venom_weak_ref_upgrade(VenomWeakRef* weak);

/**
 * @brief Check if weak reference is still valid
 */
VenomBool venom_weak_ref_is_valid(const VenomWeakRef* weak);

/**
 * @brief Release weak reference
 */
void venom_weak_ref_clear(VenomWeakRef* weak);

/* ============================================================================
 * DEBUG UTILITIES
 * ============================================================================ */

#ifdef VENOM_DEBUG

/**
 * @brief Print reference count info for debugging
 */
void venom_ref_debug_print(const void* obj);

#endif

#ifdef __cplusplus
}
#endif

#endif /* VENOM_REF_H */
