/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_spring_animation.h - Physics-based spring animations
 * 
 * Features:
 * - Realistic spring physics simulation
 * - Configurable stiffness, damping, and mass
 * - Critically damped, underdamped, and overdamped modes
 * - Natural-feeling UI animations
 */

#ifndef VENOM_SPRING_ANIMATION_H
#define VENOM_SPRING_ANIMATION_H

#include "venom_animation.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * SPRING ANIMATION PRESETS
 * ============================================================================ */

/**
 * @brief Preset spring configurations
 */
typedef enum VenomSpringPreset {
    VENOM_SPRING_DEFAULT = 0,       /* Balanced spring */
    VENOM_SPRING_GENTLE,            /* Slow and smooth */
    VENOM_SPRING_WOBBLY,            /* Bouncy feel */
    VENOM_SPRING_STIFF,             /* Quick and snappy */
    VENOM_SPRING_SLOW,              /* Very slow motion */
    VENOM_SPRING_MOLASSES,          /* Extra slow and heavy */
} VenomSpringPreset;

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
 * Memory: Reference counted. Call venom_unref() when done.
 * 
 * Physics model:
 *   F = -k * x - d * v
 *   where:
 *     k = stiffness (spring constant)
 *     d = damping coefficient
 *     x = displacement from target
 *     v = velocity
 */
typedef struct VenomSpringAnimation {
    VENOM_REF_HEADER;
    
    /* Target values */
    VenomF32 from_value;            /* Start value */
    VenomF32 to_value;              /* Target value */
    
    /* Spring physics parameters */
    VenomF32 stiffness;             /* Spring stiffness (k). Default: 100 */
    VenomF32 damping;               /* Damping coefficient. Default: 10 */
    VenomF32 mass;                  /* Mass. Default: 1 */
    
    /* Velocity threshold for completion */
    VenomF32 velocity_threshold;    /* Below this = settled. Default: 0.001 */
    VenomF32 displacement_threshold; /* Below this = settled. Default: 0.001 */
    
    /* Current state */
    VenomAnimationState state;
    VenomF32 current_value;         /* Current position */
    VenomF32 velocity;              /* Current velocity */
    
    /* Callbacks */
    VenomAnimationUpdateFn on_update;
    VenomAnimationStateFn on_complete;
    void* callback_data;
    
} VenomSpringAnimation;

/* ============================================================================
 * SPRING ANIMATION API
 * ============================================================================ */

/**
 * @brief Create a spring animation
 * 
 * @param from_value Starting value
 * @param to_value Target value
 * @return Result containing VenomSpringAnimation* or error
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_spring_animation_create(VenomF32 from_value, VenomF32 to_value);

/**
 * @brief Create a spring animation with preset parameters
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_spring_animation_create_preset(
    VenomF32 from_value,
    VenomF32 to_value,
    VenomSpringPreset preset
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
void venom_spring_animation_set_params(
    VenomSpringAnimation* VENOM_NONNULL spring,
    VenomF32 stiffness,
    VenomF32 damping,
    VenomF32 mass
);

/**
 * @brief Apply a preset configuration
 */
void venom_spring_animation_apply_preset(
    VenomSpringAnimation* VENOM_NONNULL spring,
    VenomSpringPreset preset
);

/**
 * @brief Set completion thresholds
 * 
 * Animation completes when both velocity and displacement are below thresholds.
 */
void venom_spring_animation_set_thresholds(
    VenomSpringAnimation* VENOM_NONNULL spring,
    VenomF32 velocity_threshold,
    VenomF32 displacement_threshold
);

/* ============================================================================
 * SPRING PLAYBACK CONTROL
 * ============================================================================ */

/**
 * @brief Start the spring animation
 */
void venom_spring_animation_start(VenomSpringAnimation* VENOM_NONNULL spring);

/**
 * @brief Stop the spring animation
 */
void venom_spring_animation_stop(VenomSpringAnimation* VENOM_NONNULL spring);

/**
 * @brief Update spring animation
 * 
 * Uses semi-implicit Euler integration for stability.
 * 
 * @param delta_time Time since last update in seconds
 * @return true if animation is still running
 */
VenomBool venom_spring_animation_update(
    VenomSpringAnimation* VENOM_NONNULL spring,
    VenomF64 delta_time
);

/**
 * @brief Set target value (can be called while running)
 * 
 * This allows for "re-targeting" the spring to a new destination
 * while preserving momentum.
 */
void venom_spring_animation_set_target(
    VenomSpringAnimation* VENOM_NONNULL spring,
    VenomF32 to_value
);

/**
 * @brief Add velocity impulse
 * 
 * Adds to the current velocity, useful for flick gestures.
 */
void venom_spring_animation_add_velocity(
    VenomSpringAnimation* VENOM_NONNULL spring,
    VenomF32 velocity
);

/**
 * @brief Get current value
 */
VENOM_INLINE VenomF32 venom_spring_animation_get_value(const VenomSpringAnimation* spring) {
    return spring ? spring->current_value : 0.0f;
}

/**
 * @brief Get current velocity
 */
VENOM_INLINE VenomF32 venom_spring_animation_get_velocity(const VenomSpringAnimation* spring) {
    return spring ? spring->velocity : 0.0f;
}

/**
 * @brief Check if spring is running
 */
VENOM_INLINE VenomBool venom_spring_animation_is_running(const VenomSpringAnimation* spring) {
    return spring && spring->state == VENOM_ANIM_STATE_RUNNING;
}

/**
 * @brief Set update callback
 */
void venom_spring_animation_set_on_update(
    VenomSpringAnimation* VENOM_NONNULL spring,
    VenomAnimationUpdateFn VENOM_NULLABLE callback,
    void* VENOM_NULLABLE user_data
);

/**
 * @brief Set completion callback
 */
void venom_spring_animation_set_on_complete(
    VenomSpringAnimation* VENOM_NONNULL spring,
    VenomAnimationStateFn VENOM_NULLABLE callback,
    void* VENOM_NULLABLE user_data
);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_SPRING_ANIMATION_H */
