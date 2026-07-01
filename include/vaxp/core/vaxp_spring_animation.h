/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_spring_animation.h - Physics-based spring animations
 * 
 * Features:
 * - Realistic spring physics simulation
 * - Configurable stiffness, damping, and mass
 * - Critically damped, underdamped, and overdamped modes
 * - Natural-feeling UI animations
 */

#ifndef VAXP_SPRING_ANIMATION_H
#define VAXP_SPRING_ANIMATION_H

#include "vaxp_animation.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * SPRING ANIMATION PRESETS
 * ============================================================================ */

/**
 * @brief Preset spring configurations
 */
typedef enum VaxpSpringPreset {
    VAXP_SPRING_DEFAULT = 0,       /* Balanced spring */
    VAXP_SPRING_GENTLE,            /* Slow and smooth */
    VAXP_SPRING_WOBBLY,            /* Bouncy feel */
    VAXP_SPRING_STIFF,             /* Quick and snappy */
    VAXP_SPRING_SLOW,              /* Very slow motion */
    VAXP_SPRING_MOLASSES,          /* Extra slow and heavy */
} VaxpSpringPreset;

/* ============================================================================
 * SPRING ANIMATION STRUCTURE
 * ============================================================================ */

/**
 * @brief Physics-based spring animation
 * 
 * Uses Hooke's law with damping for realistic motion.
 * Unlike duration-based animations, springs stop when they
 * naturally settle at the target value.
 * 
 * Memory: Reference counted. Call vaxp_unref() when done.
 * 
 * Physics model:
 *   F = -k * x - d * v
 *   where:
 *     k = stiffness (spring constant)
 *     d = damping coefficient
 *     x = displacement from target
 *     v = velocity
 */
typedef struct VaxpSpringAnimation {
    VAXP_REF_HEADER;
    
    /* Target values */
    VaxpF32 from_value;            /* Start value */
    VaxpF32 to_value;              /* Target value */
    
    /* Spring physics parameters */
    VaxpF32 stiffness;             /* Spring stiffness (k). Default: 100 */
    VaxpF32 damping;               /* Damping coefficient. Default: 10 */
    VaxpF32 mass;                  /* Mass. Default: 1 */
    
    /* Velocity threshold for completion */
    VaxpF32 velocity_threshold;    /* Below this = settled. Default: 0.001 */
    VaxpF32 displacement_threshold; /* Below this = settled. Default: 0.001 */
    
    /* Current state */
    VaxpAnimationState state;
    VaxpF32 current_value;         /* Current position */
    VaxpF32 velocity;              /* Current velocity */
    
    /* Callbacks */
    VaxpAnimationUpdateFn on_update;
    VaxpAnimationStateFn on_complete;
    void* callback_data;
    
} VaxpSpringAnimation;

/* ============================================================================
 * SPRING ANIMATION API
 * ============================================================================ */

/**
 * @brief Create a spring animation
 * 
 * @param from_value Starting value
 * @param to_value Target value
 * @return Result containing VaxpSpringAnimation* or error
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_spring_animation_create(VaxpF32 from_value, VaxpF32 to_value);

/**
 * @brief Create a spring animation with preset parameters
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_spring_animation_create_preset(
    VaxpF32 from_value,
    VaxpF32 to_value,
    VaxpSpringPreset preset
);

/**
 * @brief Set spring physics parameters
 * 
 * Higher stiffness = faster animation
 * Higher damping = less oscillation
 * Higher mass = slower, more inertia
 * 
 * For critically damped spring (no oscillation):
 *   damping = 2 * sqrt(stiffness * mass)
 */
void vaxp_spring_animation_set_params(
    VaxpSpringAnimation* VAXP_NONNULL spring,
    VaxpF32 stiffness,
    VaxpF32 damping,
    VaxpF32 mass
);

/**
 * @brief Apply a preset configuration
 */
void vaxp_spring_animation_apply_preset(
    VaxpSpringAnimation* VAXP_NONNULL spring,
    VaxpSpringPreset preset
);

/**
 * @brief Set completion thresholds
 * 
 * Animation completes when both velocity and displacement are below thresholds.
 */
void vaxp_spring_animation_set_thresholds(
    VaxpSpringAnimation* VAXP_NONNULL spring,
    VaxpF32 velocity_threshold,
    VaxpF32 displacement_threshold
);

/* ============================================================================
 * SPRING PLAYBACK CONTROL
 * ============================================================================ */

/**
 * @brief Start the spring animation
 */
void vaxp_spring_animation_start(VaxpSpringAnimation* VAXP_NONNULL spring);

/**
 * @brief Stop the spring animation
 */
void vaxp_spring_animation_stop(VaxpSpringAnimation* VAXP_NONNULL spring);

/**
 * @brief Update spring animation
 * 
 * Uses semi-implicit Euler integration for stability.
 * 
 * @param delta_time Time since last update in seconds
 * @return true if animation is still running
 */
VaxpBool vaxp_spring_animation_update(
    VaxpSpringAnimation* VAXP_NONNULL spring,
    VaxpF64 delta_time
);

/**
 * @brief Set target value (can be called while running)
 * 
 * This allows for "re-targeting" the spring to a new destination
 * while preserving momentum.
 */
void vaxp_spring_animation_set_target(
    VaxpSpringAnimation* VAXP_NONNULL spring,
    VaxpF32 to_value
);

/**
 * @brief Add velocity impulse
 * 
 * Adds to the current velocity, useful for flick gestures.
 */
void vaxp_spring_animation_add_velocity(
    VaxpSpringAnimation* VAXP_NONNULL spring,
    VaxpF32 velocity
);

/**
 * @brief Get current value
 */
VAXP_INLINE VaxpF32 vaxp_spring_animation_get_value(const VaxpSpringAnimation* spring) {
    return spring ? spring->current_value : 0.0f;
}

/**
 * @brief Get current velocity
 */
VAXP_INLINE VaxpF32 vaxp_spring_animation_get_velocity(const VaxpSpringAnimation* spring) {
    return spring ? spring->velocity : 0.0f;
}

/**
 * @brief Check if spring is running
 */
VAXP_INLINE VaxpBool vaxp_spring_animation_is_running(const VaxpSpringAnimation* spring) {
    return spring && spring->state == VAXP_ANIM_STATE_RUNNING;
}

/**
 * @brief Set update callback
 */
void vaxp_spring_animation_set_on_update(
    VaxpSpringAnimation* VAXP_NONNULL spring,
    VaxpAnimationUpdateFn VAXP_NULLABLE callback,
    void* VAXP_NULLABLE user_data
);

/**
 * @brief Set completion callback
 */
void vaxp_spring_animation_set_on_complete(
    VaxpSpringAnimation* VAXP_NONNULL spring,
    VaxpAnimationStateFn VAXP_NULLABLE callback,
    void* VAXP_NULLABLE user_data
);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_SPRING_ANIMATION_H */
