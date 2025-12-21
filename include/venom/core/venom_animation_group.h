/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_animation_group.h - Animation groups for sequence and parallel animations
 * 
 * Features:
 * - Sequence: Run animations one after another
 * - Parallel: Run animations simultaneously
 * - Stagger: Run animations with incremental delays
 * - Reference counting with automatic cleanup of children
 */

#ifndef VENOM_ANIMATION_GROUP_H
#define VENOM_ANIMATION_GROUP_H

#include "venom_animation.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * ANIMATION GROUP TYPES
 * ============================================================================ */

/**
 * @brief Type of animation group
 */
typedef enum VenomAnimationGroupType {
    VENOM_ANIM_GROUP_SEQUENCE,      /* Run animations one after another */
    VENOM_ANIM_GROUP_PARALLEL,      /* Run all animations simultaneously */
    VENOM_ANIM_GROUP_STAGGER,       /* Run with incremental delay between each */
} VenomAnimationGroupType;

/* Forward declaration */
typedef struct VenomAnimationGroup VenomAnimationGroup;

/**
 * @brief Callback when group completes
 */
typedef void (*VenomAnimationGroupCallback)(
    VenomAnimationGroup* VENOM_NONNULL group,
    void* VENOM_NULLABLE user_data
);

/* ============================================================================
 * ANIMATION GROUP STRUCTURE
 * ============================================================================ */

/**
 * @brief A group of animations that can be played together or in sequence
 * 
 * Memory: Reference counted. Owns its child animations.
 * 
 * Usage:
 *   VenomAnimationGroup* group = venom_unwrap_ptr(
 *       venom_animation_group_create(VENOM_ANIM_GROUP_SEQUENCE)
 *   );
 *   venom_animation_group_add(group, anim1);
 *   venom_animation_group_add(group, anim2);
 *   venom_animation_group_start(group);
 *   // ...
 *   venom_unref(group);  // Also unrefs children
 */
struct VenomAnimationGroup {
    VENOM_REF_HEADER;
    
    VenomAnimationGroupType type;       /* Group type */
    
    /* Children (owned, ref counted) */
    VenomAnimation** VENOM_OWNED children;  /* Array of child animations */
    VenomU32 children_count;                /* Number of children */
    VenomU32 children_capacity;             /* Allocated capacity */
    
    /* Stagger configuration */
    VenomF32 stagger_delay;                 /* Delay between each animation (for stagger type) */
    
    /* Playback state */
    VenomAnimationState state;              /* Current group state */
    VenomU32 current_index;                 /* Current animation index (for sequence) */
    
    /* Callbacks */
    VenomAnimationGroupCallback on_complete;
    void* callback_data;
};

/* ============================================================================
 * ANIMATION GROUP API
 * ============================================================================ */

/**
 * @brief Create an animation group
 * 
 * @param type The type of group
 * @return Result containing VenomAnimationGroup* or error
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_animation_group_create(VenomAnimationGroupType type);

/**
 * @brief Add an animation to the group
 * 
 * Takes ownership of the animation (refs it).
 * 
 * @param group The group
 * @param anim The animation to add
 * @return Success or error
 */
VenomResult venom_animation_group_add(
    VenomAnimationGroup* VENOM_NONNULL group,
    VenomAnimation* VENOM_NONNULL anim
);

/**
 * @brief Remove an animation from the group
 * 
 * Releases ownership (unrefs it).
 * 
 * @param group The group
 * @param anim The animation to remove
 * @return Success or error
 */
VenomResult venom_animation_group_remove(
    VenomAnimationGroup* VENOM_NONNULL group,
    VenomAnimation* VENOM_NONNULL anim
);

/**
 * @brief Remove all animations from the group
 */
void venom_animation_group_clear(VenomAnimationGroup* VENOM_NONNULL group);

/**
 * @brief Set stagger delay (only for STAGGER type)
 * 
 * @param delay Time between starting each animation
 */
void venom_animation_group_set_stagger_delay(
    VenomAnimationGroup* VENOM_NONNULL group,
    VenomF32 delay
);

/* ============================================================================
 * GROUP PLAYBACK CONTROL
 * ============================================================================ */

/**
 * @brief Start all animations in the group
 */
void venom_animation_group_start(VenomAnimationGroup* VENOM_NONNULL group);

/**
 * @brief Pause all animations in the group
 */
void venom_animation_group_pause(VenomAnimationGroup* VENOM_NONNULL group);

/**
 * @brief Resume all animations in the group
 */
void venom_animation_group_resume(VenomAnimationGroup* VENOM_NONNULL group);

/**
 * @brief Stop all animations in the group
 */
void venom_animation_group_stop(VenomAnimationGroup* VENOM_NONNULL group);

/**
 * @brief Update all animations in the group
 * 
 * @param delta_time Time since last update in seconds
 * @return true if any animation is still running
 */
VenomBool venom_animation_group_update(
    VenomAnimationGroup* VENOM_NONNULL group,
    VenomF64 delta_time
);

/**
 * @brief Check if group is running
 */
VENOM_INLINE VenomBool venom_animation_group_is_running(const VenomAnimationGroup* group) {
    return group && group->state == VENOM_ANIM_STATE_RUNNING;
}

/**
 * @brief Check if group has completed
 */
VENOM_INLINE VenomBool venom_animation_group_is_completed(const VenomAnimationGroup* group) {
    return group && group->state == VENOM_ANIM_STATE_COMPLETED;
}

/**
 * @brief Get child count
 */
VENOM_INLINE VenomU32 venom_animation_group_child_count(const VenomAnimationGroup* group) {
    return group ? group->children_count : 0;
}

/**
 * @brief Set completion callback
 */
void venom_animation_group_set_on_complete(
    VenomAnimationGroup* VENOM_NONNULL group,
    VenomAnimationGroupCallback VENOM_NULLABLE callback,
    void* VENOM_NULLABLE user_data
);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_ANIMATION_GROUP_H */
