/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_animation.h - Professional animation system with easing functions
 * 
 * Features:
 * - 30+ Easing functions (Robert Penner's equations)
 * - Tween animations with full lifecycle control
 * - Keyframe animations for complex sequences
 * - Reference counting with automatic cleanup
 * - Thread-safe state management
 */

#ifndef VENOM_ANIMATION_H
#define VENOM_ANIMATION_H

#include "venom_types.h"
#include "venom_result.h"
#include "venom_ref.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * EASING FUNCTIONS
 * ============================================================================ */

/**
 * @brief Easing function types for smooth animations
 * 
 * Based on Robert Penner's easing equations.
 * Visual reference: https://easings.net/
 */
typedef enum VenomEasing {
    /* Linear */
    VENOM_EASING_LINEAR = 0,
    
    /* Quadratic (power of 2) */
    VENOM_EASING_QUAD_IN,
    VENOM_EASING_QUAD_OUT,
    VENOM_EASING_QUAD_IN_OUT,
    
    /* Cubic (power of 3) */
    VENOM_EASING_CUBIC_IN,
    VENOM_EASING_CUBIC_OUT,
    VENOM_EASING_CUBIC_IN_OUT,
    
    /* Quartic (power of 4) */
    VENOM_EASING_QUART_IN,
    VENOM_EASING_QUART_OUT,
    VENOM_EASING_QUART_IN_OUT,
    
    /* Quintic (power of 5) */
    VENOM_EASING_QUINT_IN,
    VENOM_EASING_QUINT_OUT,
    VENOM_EASING_QUINT_IN_OUT,
    
    /* Sine wave */
    VENOM_EASING_SINE_IN,
    VENOM_EASING_SINE_OUT,
    VENOM_EASING_SINE_IN_OUT,
    
    /* Exponential */
    VENOM_EASING_EXPO_IN,
    VENOM_EASING_EXPO_OUT,
    VENOM_EASING_EXPO_IN_OUT,
    
    /* Circular */
    VENOM_EASING_CIRC_IN,
    VENOM_EASING_CIRC_OUT,
    VENOM_EASING_CIRC_IN_OUT,
    
    /* Elastic (spring-like) */
    VENOM_EASING_ELASTIC_IN,
    VENOM_EASING_ELASTIC_OUT,
    VENOM_EASING_ELASTIC_IN_OUT,
    
    /* Back (overshoot) */
    VENOM_EASING_BACK_IN,
    VENOM_EASING_BACK_OUT,
    VENOM_EASING_BACK_IN_OUT,
    
    /* Bounce */
    VENOM_EASING_BOUNCE_IN,
    VENOM_EASING_BOUNCE_OUT,
    VENOM_EASING_BOUNCE_IN_OUT,
    
    VENOM_EASING_COUNT
} VenomEasing;

/**
 * @brief Animation playback state
 */
typedef enum VenomAnimationState {
    VENOM_ANIM_STATE_IDLE = 0,      /* Not started or finished */
    VENOM_ANIM_STATE_RUNNING,       /* Currently playing */
    VENOM_ANIM_STATE_PAUSED,        /* Paused mid-animation */
    VENOM_ANIM_STATE_COMPLETED,     /* Finished playing */
} VenomAnimationState;

/**
 * @brief Animation repeat mode
 */
typedef enum VenomRepeatMode {
    VENOM_REPEAT_NONE = 0,          /* Play once */
    VENOM_REPEAT_LOOP,              /* Loop forever */
    VENOM_REPEAT_PING_PONG,         /* Alternate direction each cycle */
} VenomRepeatMode;

/**
 * @brief Animation direction
 */
typedef enum VenomAnimationDirection {
    VENOM_ANIM_DIRECTION_FORWARD = 0,
    VENOM_ANIM_DIRECTION_REVERSE,
} VenomAnimationDirection;

/* ============================================================================
 * ANIMATION CALLBACK TYPES
 * ============================================================================ */

/* Forward declaration */
typedef struct VenomAnimation VenomAnimation;

/**
 * @brief Called on each animation frame update
 * 
 * @param anim The animation
 * @param value Current interpolated value (between from and to)
 * @param progress Normalized progress (0.0 to 1.0)
 * @param user_data User-provided context
 */
typedef void (*VenomAnimationUpdateFn)(
    VenomAnimation* VENOM_NONNULL anim,
    VenomF32 value,
    VenomF32 progress,
    void* VENOM_NULLABLE user_data
);

/**
 * @brief Called when animation state changes
 * 
 * @param anim The animation
 * @param state New state
 * @param user_data User-provided context
 */
typedef void (*VenomAnimationStateFn)(
    VenomAnimation* VENOM_NONNULL anim,
    VenomAnimationState state,
    void* VENOM_NULLABLE user_data
);

/* ============================================================================
 * ANIMATION STRUCTURE
 * ============================================================================ */

/**
 * @brief Tween animation for interpolating between two values
 * 
 * Memory: Reference counted. Call venom_unref() when done.
 * 
 * Usage:
 *   VenomAnimation* anim = venom_unwrap_ptr(
 *       venom_animation_create(0.0f, 100.0f, 0.5f)
 *   );
 *   venom_animation_set_easing(anim, VENOM_EASING_CUBIC_OUT);
 *   venom_animation_set_on_update(anim, my_callback, my_data);
 *   venom_animation_start(anim);
 *   // In your update loop:
 *   venom_animation_update(anim, delta_time);
 *   // When done:
 *   venom_unref(anim);
 */
struct VenomAnimation {
    VENOM_REF_HEADER;
    
    /* Animation parameters */
    VenomF32 from_value;            /* Start value */
    VenomF32 to_value;              /* End value */
    VenomF32 duration;              /* Duration in seconds */
    VenomF32 delay;                 /* Delay before starting in seconds */
    
    /* Easing configuration */
    VenomEasing easing;             /* Easing function to use */
    
    /* Repeat configuration */
    VenomRepeatMode repeat_mode;    /* How to repeat */
    VenomI32 repeat_count;          /* Number of times to repeat (-1 = infinite) */
    VenomI32 current_repeat;        /* Current repeat iteration */
    
    /* Playback state */
    VenomAnimationState state;      /* Current state */
    VenomAnimationDirection direction; /* Current direction */
    VenomF32 elapsed;               /* Time elapsed since start (excluding delay) */
    VenomF32 delay_elapsed;         /* Time elapsed in delay phase */
    VenomF32 current_value;         /* Current interpolated value */
    VenomF32 progress;              /* Current normalized progress (0-1) */
    
    /* Callbacks */
    VenomAnimationUpdateFn on_update;       /* Called each frame */
    VenomAnimationStateFn on_start;         /* Called when animation starts */
    VenomAnimationStateFn on_complete;      /* Called when animation completes */
    VenomAnimationStateFn on_repeat;        /* Called on each repeat cycle */
    void* callback_data;                    /* User data for callbacks */
    
    /* Internal timing */
    VenomF64 start_time;            /* Timestamp when started (for sync) */
};

/* ============================================================================
 * EASING FUNCTION API
 * ============================================================================ */

/**
 * @brief Apply easing function to a normalized time value
 * 
 * @param easing The easing type
 * @param t Normalized input time (0.0 to 1.0)
 * @return Eased output value (may be outside 0-1 for overshoot easings)
 */
VenomF32 venom_easing_apply(VenomEasing easing, VenomF32 t);

/**
 * @brief Get the name of an easing function
 * 
 * @param easing The easing type
 * @return String name (e.g., "cubic-in-out")
 */
const char* venom_easing_name(VenomEasing easing);

/* ============================================================================
 * ANIMATION LIFECYCLE
 * ============================================================================ */

/**
 * @brief Create a new tween animation
 * 
 * @param from_value Starting value
 * @param to_value Ending value
 * @param duration Duration in seconds
 * @return Result containing VenomAnimation* or error
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_animation_create(VenomF32 from_value, VenomF32 to_value, VenomF32 duration);

/**
 * @brief Create animation with full configuration
 */
typedef struct VenomAnimationConfig {
    VenomF32 from;
    VenomF32 to;
    VenomF32 duration;
    VenomF32 delay;
    VenomEasing easing;
    VenomRepeatMode repeat;
    VenomI32 repeat_count;
    VenomAnimationUpdateFn on_update;
    VenomAnimationStateFn on_complete;
    void* user_data;
} VenomAnimationConfig;

VENOM_WARN_UNUSED
VenomResultPtr venom_animation_create_with_config(const VenomAnimationConfig* VENOM_NONNULL config);

/* ============================================================================
 * ANIMATION PLAYBACK CONTROL
 * ============================================================================ */

/**
 * @brief Start or restart the animation
 * 
 * Resets elapsed time to 0 and sets state to RUNNING.
 */
void venom_animation_start(VenomAnimation* VENOM_NONNULL anim);

/**
 * @brief Pause the animation
 * 
 * Can be resumed with venom_animation_resume().
 */
void venom_animation_pause(VenomAnimation* VENOM_NONNULL anim);

/**
 * @brief Resume a paused animation
 */
void venom_animation_resume(VenomAnimation* VENOM_NONNULL anim);

/**
 * @brief Stop the animation and reset to initial state
 */
void venom_animation_stop(VenomAnimation* VENOM_NONNULL anim);

/**
 * @brief Reverse the animation direction
 */
void venom_animation_reverse(VenomAnimation* VENOM_NONNULL anim);

/**
 * @brief Seek to a specific progress point
 * 
 * @param progress Normalized progress (0.0 to 1.0)
 */
void venom_animation_seek(VenomAnimation* VENOM_NONNULL anim, VenomF32 progress);

/* ============================================================================
 * ANIMATION UPDATE
 * ============================================================================ */

/**
 * @brief Update animation by delta time
 * 
 * Call this every frame with the time since last frame.
 * 
 * @param anim The animation to update
 * @param delta_time Time since last update in seconds
 * @return true if animation is still running
 */
VenomBool venom_animation_update(VenomAnimation* VENOM_NONNULL anim, VenomF64 delta_time);

/**
 * @brief Get current animated value
 * 
 * Returns the interpolated value based on current progress and easing.
 */
VENOM_INLINE VenomF32 venom_animation_get_value(const VenomAnimation* anim) {
    return anim ? anim->current_value : 0.0f;
}

/**
 * @brief Get current progress (0.0 to 1.0)
 */
VENOM_INLINE VenomF32 venom_animation_get_progress(const VenomAnimation* anim) {
    return anim ? anim->progress : 0.0f;
}

/**
 * @brief Check if animation is currently running
 */
VENOM_INLINE VenomBool venom_animation_is_running(const VenomAnimation* anim) {
    return anim && anim->state == VENOM_ANIM_STATE_RUNNING;
}

/**
 * @brief Check if animation has completed
 */
VENOM_INLINE VenomBool venom_animation_is_completed(const VenomAnimation* anim) {
    return anim && anim->state == VENOM_ANIM_STATE_COMPLETED;
}

/* ============================================================================
 * ANIMATION CONFIGURATION
 * ============================================================================ */

/**
 * @brief Set easing function
 */
void venom_animation_set_easing(VenomAnimation* VENOM_NONNULL anim, VenomEasing easing);

/**
 * @brief Set delay before animation starts
 */
void venom_animation_set_delay(VenomAnimation* VENOM_NONNULL anim, VenomF32 delay);

/**
 * @brief Set repeat mode and count
 * 
 * @param mode Repeat mode
 * @param count Number of times to repeat (-1 for infinite)
 */
void venom_animation_set_repeat(VenomAnimation* VENOM_NONNULL anim, VenomRepeatMode mode, VenomI32 count);

/**
 * @brief Set update callback
 */
void venom_animation_set_on_update(
    VenomAnimation* VENOM_NONNULL anim,
    VenomAnimationUpdateFn VENOM_NULLABLE callback,
    void* VENOM_NULLABLE user_data
);

/**
 * @brief Set completion callback
 */
void venom_animation_set_on_complete(
    VenomAnimation* VENOM_NONNULL anim,
    VenomAnimationStateFn VENOM_NULLABLE callback,
    void* VENOM_NULLABLE user_data
);

/**
 * @brief Set start callback
 */
void venom_animation_set_on_start(
    VenomAnimation* VENOM_NONNULL anim,
    VenomAnimationStateFn VENOM_NULLABLE callback,
    void* VENOM_NULLABLE user_data
);

/* ============================================================================
 * COLOR ANIMATION HELPERS
 * ============================================================================ */

/**
 * @brief Interpolate between two colors
 * 
 * @param from Start color
 * @param to End color
 * @param t Interpolation factor (0.0 to 1.0)
 * @return Interpolated color
 */
VenomColor venom_color_lerp(VenomColor from, VenomColor to, VenomF32 t);

/**
 * @brief Interpolate colors in HSL space (smoother for hue transitions)
 */
VenomColor venom_color_lerp_hsl(VenomColor from, VenomColor to, VenomF32 t);

/* ============================================================================
 * KEYFRAME ANIMATION
 * ============================================================================ */

/**
 * @brief A single keyframe in an animation
 */
typedef struct VenomKeyframe {
    VenomF32 time;          /* Time position (0.0 to 1.0, normalized) */
    VenomF32 value;         /* Value at this keyframe */
    VenomEasing easing;     /* Easing to next keyframe */
} VenomKeyframe;

/**
 * @brief Keyframe animation for complex multi-point animations
 */
typedef struct VenomKeyframeAnimation {
    VENOM_REF_HEADER;
    
    VenomKeyframe* VENOM_OWNED keyframes;   /* Array of keyframes */
    VenomU32 keyframe_count;                /* Number of keyframes */
    VenomU32 keyframe_capacity;             /* Allocated capacity */
    
    VenomF32 duration;                      /* Total duration in seconds */
    VenomAnimationState state;              /* Current state */
    VenomF32 elapsed;                       /* Elapsed time */
    VenomF32 current_value;                 /* Current interpolated value */
    
    VenomAnimationUpdateFn on_update;
    VenomAnimationStateFn on_complete;
    void* callback_data;
} VenomKeyframeAnimation;

/**
 * @brief Create a keyframe animation
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_keyframe_animation_create(VenomF32 duration);

/**
 * @brief Add a keyframe
 * 
 * @param anim The animation
 * @param time Normalized time (0.0 to 1.0)
 * @param value Value at this keyframe
 * @param easing Easing to use when interpolating to next keyframe
 */
VenomResult venom_keyframe_animation_add(
    VenomKeyframeAnimation* VENOM_NONNULL anim,
    VenomF32 time,
    VenomF32 value,
    VenomEasing easing
);

/**
 * @brief Start the keyframe animation
 */
void venom_keyframe_animation_start(VenomKeyframeAnimation* VENOM_NONNULL anim);

/**
 * @brief Update the keyframe animation
 */
VenomBool venom_keyframe_animation_update(VenomKeyframeAnimation* VENOM_NONNULL anim, VenomF64 delta_time);

/**
 * @brief Get current value
 */
VENOM_INLINE VenomF32 venom_keyframe_animation_get_value(const VenomKeyframeAnimation* anim) {
    return anim ? anim->current_value : 0.0f;
}

#ifdef __cplusplus
}
#endif

#endif /* VENOM_ANIMATION_H */
