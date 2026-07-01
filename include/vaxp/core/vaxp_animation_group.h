/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_animation_group.h - Animation groups for sequence and parallel animations
 * 
 * Features:
 * - Sequence: Run animations one after another
 * - Parallel: Run animations simultaneously
 * - Stagger: Run animations with incremental delays
 * - Reference counting with automatic cleanup of children
 */

#ifndef VAXP_ANIMATION_GROUP_H
#define VAXP_ANIMATION_GROUP_H

#include "vaxp_animation.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * ANIMATION GROUP TYPES
 * ============================================================================ */

/**
 * @brief Type of animation group
 */
typedef enum VaxpAnimationGroupType {
    VAXP_ANIM_GROUP_SEQUENCE,      /* Run animations one after another */
    VAXP_ANIM_GROUP_PARALLEL,      /* Run all animations simultaneously */
    VAXP_ANIM_GROUP_STAGGER,       /* Run with incremental delay between each */
} VaxpAnimationGroupType;

/* Forward declaration */
typedef struct VaxpAnimationGroup VaxpAnimationGroup;

/**
 * @brief Callback when group completes
 */
typedef void (*VaxpAnimationGroupCallback)(
    VaxpAnimationGroup* VAXP_NONNULL group,
    void* VAXP_NULLABLE user_data
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
 *   VaxpAnimationGroup* group = vaxp_unwrap_ptr(
 *       vaxp_animation_group_create(VAXP_ANIM_GROUP_SEQUENCE)
 *   );
 *   vaxp_animation_group_add(group, anim1);
 *   vaxp_animation_group_add(group, anim2);
 *   vaxp_animation_group_start(group);
 *   // ...
 *   vaxp_unref(group);  // Also unrefs children
 */
struct VaxpAnimationGroup {
    VAXP_REF_HEADER;
    
    VaxpAnimationGroupType type;       /* Group type */
    
    /* Children (owned, ref counted) */
    VaxpAnimation** VAXP_OWNED children;  /* Array of child animations */
    VaxpU32 children_count;                /* Number of children */
    VaxpU32 children_capacity;             /* Allocated capacity */
    
    /* Stagger configuration */
    VaxpF32 stagger_delay;                 /* Delay between each animation (for stagger type) */
    
    /* Playback state */
    VaxpAnimationState state;              /* Current group state */
    VaxpU32 current_index;                 /* Current animation index (for sequence) */
    
    /* Callbacks */
    VaxpAnimationGroupCallback on_complete;
    void* callback_data;
};

/* ============================================================================
 * ANIMATION GROUP API
 * ============================================================================ */

/**
 * @brief Create an animation group
 * 
 * @param type The type of group
 * @return Result containing VaxpAnimationGroup* or error
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_animation_group_create(VaxpAnimationGroupType type);

/**
 * @brief Add an animation to the group
 * 
 * Takes ownership of the animation (refs it).
 * 
 * @param group The group
 * @param anim The animation to add
 * @return Success or error
 */
VaxpResult vaxp_animation_group_add(
    VaxpAnimationGroup* VAXP_NONNULL group,
    VaxpAnimation* VAXP_NONNULL anim
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
VaxpResult vaxp_animation_group_remove(
    VaxpAnimationGroup* VAXP_NONNULL group,
    VaxpAnimation* VAXP_NONNULL anim
);

/**
 * @brief Remove all animations from the group
 */
void vaxp_animation_group_clear(VaxpAnimationGroup* VAXP_NONNULL group);

/**
 * @brief Set stagger delay (only for STAGGER type)
 * 
 * @param delay Time between starting each animation
 */
void vaxp_animation_group_set_stagger_delay(
    VaxpAnimationGroup* VAXP_NONNULL group,
    VaxpF32 delay
);

/* ============================================================================
 * GROUP PLAYBACK CONTROL
 * ============================================================================ */

/**
 * @brief Start all animations in the group
 */
void vaxp_animation_group_start(VaxpAnimationGroup* VAXP_NONNULL group);

/**
 * @brief Pause all animations in the group
 */
void vaxp_animation_group_pause(VaxpAnimationGroup* VAXP_NONNULL group);

/**
 * @brief Resume all animations in the group
 */
void vaxp_animation_group_resume(VaxpAnimationGroup* VAXP_NONNULL group);

/**
 * @brief Stop all animations in the group
 */
void vaxp_animation_group_stop(VaxpAnimationGroup* VAXP_NONNULL group);

/**
 * @brief Update all animations in the group
 * 
 * @param delta_time Time since last update in seconds
 * @return true if any animation is still running
 */
VaxpBool vaxp_animation_group_update(
    VaxpAnimationGroup* VAXP_NONNULL group,
    VaxpF64 delta_time
);

/**
 * @brief Check if group is running
 */
VAXP_INLINE VaxpBool vaxp_animation_group_is_running(const VaxpAnimationGroup* group) {
    return group && group->state == VAXP_ANIM_STATE_RUNNING;
}

/**
 * @brief Check if group has completed
 */
VAXP_INLINE VaxpBool vaxp_animation_group_is_completed(const VaxpAnimationGroup* group) {
    return group && group->state == VAXP_ANIM_STATE_COMPLETED;
}

/**
 * @brief Get child count
 */
VAXP_INLINE VaxpU32 vaxp_animation_group_child_count(const VaxpAnimationGroup* group) {
    return group ? group->children_count : 0;
}

/**
 * @brief Set completion callback
 */
void vaxp_animation_group_set_on_complete(
    VaxpAnimationGroup* VAXP_NONNULL group,
    VaxpAnimationGroupCallback VAXP_NULLABLE callback,
    void* VAXP_NULLABLE user_data
);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_ANIMATION_GROUP_H */
