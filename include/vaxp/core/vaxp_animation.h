/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_animation.h - Professional animation system with easing functions
 * 
 * Features:
 * - 30+ Easing functions (Robert Penner's equations)
 * - Tween animations with full lifecycle control
 * - Keyframe animations for complex sequences
 * - Reference counting with automatic cleanup
 * - Thread-safe state management
 */

#ifndef VAXP_ANIMATION_H
#define VAXP_ANIMATION_H

#include "vaxp_types.h"
#include "vaxp_result.h"
#include "vaxp_ref.h"

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
typedef enum VaxpEasing {
    /* Linear */
    VAXP_EASING_LINEAR = 0,
    
    /* Quadratic (power of 2) */
    VAXP_EASING_QUAD_IN,
    VAXP_EASING_QUAD_OUT,
    VAXP_EASING_QUAD_IN_OUT,
    
    /* Cubic (power of 3) */
    VAXP_EASING_CUBIC_IN,
    VAXP_EASING_CUBIC_OUT,
    VAXP_EASING_CUBIC_IN_OUT,
    
    /* Quartic (power of 4) */
    VAXP_EASING_QUART_IN,
    VAXP_EASING_QUART_OUT,
    VAXP_EASING_QUART_IN_OUT,
    
    /* Quintic (power of 5) */
    VAXP_EASING_QUINT_IN,
    VAXP_EASING_QUINT_OUT,
    VAXP_EASING_QUINT_IN_OUT,
    
    /* Sine wave */
    VAXP_EASING_SINE_IN,
    VAXP_EASING_SINE_OUT,
    VAXP_EASING_SINE_IN_OUT,
    
    /* Exponential */
    VAXP_EASING_EXPO_IN,
    VAXP_EASING_EXPO_OUT,
    VAXP_EASING_EXPO_IN_OUT,
    
    /* Circular */
    VAXP_EASING_CIRC_IN,
    VAXP_EASING_CIRC_OUT,
    VAXP_EASING_CIRC_IN_OUT,
    
    /* Elastic (spring-like) */
    VAXP_EASING_ELASTIC_IN,
    VAXP_EASING_ELASTIC_OUT,
    VAXP_EASING_ELASTIC_IN_OUT,
    
    /* Back (overshoot) */
    VAXP_EASING_BACK_IN,
    VAXP_EASING_BACK_OUT,
    VAXP_EASING_BACK_IN_OUT,
    
    /* Bounce */
    VAXP_EASING_BOUNCE_IN,
    VAXP_EASING_BOUNCE_OUT,
    VAXP_EASING_BOUNCE_IN_OUT,
    
    VAXP_EASING_COUNT
} VaxpEasing;

/**
 * @brief Animation playback state
 */
typedef enum VaxpAnimationState {
    VAXP_ANIM_STATE_IDLE = 0,      /* Not started or finished */
    VAXP_ANIM_STATE_RUNNING,       /* Currently playing */
    VAXP_ANIM_STATE_PAUSED,        /* Paused mid-animation */
    VAXP_ANIM_STATE_COMPLETED,     /* Finished playing */
} VaxpAnimationState;

/**
 * @brief Animation repeat mode
 */
typedef enum VaxpRepeatMode {
    VAXP_REPEAT_NONE = 0,          /* Play once */
    VAXP_REPEAT_LOOP,              /* Loop forever */
    VAXP_REPEAT_PING_PONG,         /* Alternate direction each cycle */
} VaxpRepeatMode;

/**
 * @brief Animation direction
 */
typedef enum VaxpAnimationDirection {
    VAXP_ANIM_DIRECTION_FORWARD = 0,
    VAXP_ANIM_DIRECTION_REVERSE,
} VaxpAnimationDirection;

/* ============================================================================
 * ANIMATION CALLBACK TYPES
 * ============================================================================ */

/* Forward declaration */
typedef struct VaxpAnimation VaxpAnimation;

/**
 * @brief Called on each animation frame update
 * 
 * @param anim The animation
 * @param value Current interpolated value (between from and to)
 * @param progress Normalized progress (0.0 to 1.0)
 * @param user_data User-provided context
 */
typedef void (*VaxpAnimationUpdateFn)(
    VaxpAnimation* VAXP_NONNULL anim,
    VaxpF32 value,
    VaxpF32 progress,
    void* VAXP_NULLABLE user_data
);

/**
 * @brief Called when animation state changes
 * 
 * @param anim The animation
 * @param state New state
 * @param user_data User-provided context
 */
typedef void (*VaxpAnimationStateFn)(
    VaxpAnimation* VAXP_NONNULL anim,
    VaxpAnimationState state,
    void* VAXP_NULLABLE user_data
);

/* ============================================================================
 * ANIMATION STRUCTURE
 * ============================================================================ */

/**
 * @brief Tween animation for interpolating between two values
 * 
 * Memory: Reference counted. Call vaxp_unref() when done.
 * 
 * Usage:
 *   VaxpAnimation* anim = vaxp_unwrap_ptr(
 *       vaxp_animation_create(0.0f, 100.0f, 0.5f)
 *   );
 *   vaxp_animation_set_easing(anim, VAXP_EASING_CUBIC_OUT);
 *   vaxp_animation_set_on_update(anim, my_callback, my_data);
 *   vaxp_animation_start(anim);
 *   // In your update loop:
 *   vaxp_animation_update(anim, delta_time);
 *   // When done:
 *   vaxp_unref(anim);
 */
struct VaxpAnimation {
    VAXP_REF_HEADER;
    
    /* Animation parameters */
    VaxpF32 from_value;            /* Start value */
    VaxpF32 to_value;              /* End value */
    VaxpF32 duration;              /* Duration in seconds */
    VaxpF32 delay;                 /* Delay before starting in seconds */
    
    /* Easing configuration */
    VaxpEasing easing;             /* Easing function to use */
    
    /* Repeat configuration */
    VaxpRepeatMode repeat_mode;    /* How to repeat */
    VaxpI32 repeat_count;          /* Number of times to repeat (-1 = infinite) */
    VaxpI32 current_repeat;        /* Current repeat iteration */
    
    /* Playback state */
    VaxpAnimationState state;      /* Current state */
    VaxpAnimationDirection direction; /* Current direction */
    VaxpF32 elapsed;               /* Time elapsed since start (excluding delay) */
    VaxpF32 delay_elapsed;         /* Time elapsed in delay phase */
    VaxpF32 current_value;         /* Current interpolated value */
    VaxpF32 progress;              /* Current normalized progress (0-1) */
    
    /* Callbacks */
    VaxpAnimationUpdateFn on_update;       /* Called each frame */
    VaxpAnimationStateFn on_start;         /* Called when animation starts */
    VaxpAnimationStateFn on_complete;      /* Called when animation completes */
    VaxpAnimationStateFn on_repeat;        /* Called on each repeat cycle */
    void* callback_data;                    /* User data for callbacks */
    
    /* Internal timing */
    VaxpF64 start_time;            /* Timestamp when started (for sync) */
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
VaxpF32 vaxp_easing_apply(VaxpEasing easing, VaxpF32 t);

/**
 * @brief Get the name of an easing function
 * 
 * @param easing The easing type
 * @return String name (e.g., "cubic-in-out")
 */
const char* vaxp_easing_name(VaxpEasing easing);

/* ============================================================================
 * ANIMATION LIFECYCLE
 * ============================================================================ */

/**
 * @brief Create a new tween animation
 * 
 * @param from_value Starting value
 * @param to_value Ending value
 * @param duration Duration in seconds
 * @return Result containing VaxpAnimation* or error
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_animation_create(VaxpF32 from_value, VaxpF32 to_value, VaxpF32 duration);

/**
 * @brief Create animation with full configuration
 */
typedef struct VaxpAnimationConfig {
    VaxpF32 from;
    VaxpF32 to;
    VaxpF32 duration;
    VaxpF32 delay;
    VaxpEasing easing;
    VaxpRepeatMode repeat;
    VaxpI32 repeat_count;
    VaxpAnimationUpdateFn on_update;
    VaxpAnimationStateFn on_complete;
    void* user_data;
} VaxpAnimationConfig;

VAXP_WARN_UNUSED
VaxpResultPtr vaxp_animation_create_with_config(const VaxpAnimationConfig* VAXP_NONNULL config);

/* ============================================================================
 * ANIMATION PLAYBACK CONTROL
 * ============================================================================ */

/**
 * @brief Start or restart the animation
 * 
 * Resets elapsed time to 0 and sets state to RUNNING.
 */
void vaxp_animation_start(VaxpAnimation* VAXP_NONNULL anim);

/**
 * @brief Pause the animation
 * 
 * Can be resumed with vaxp_animation_resume().
 */
void vaxp_animation_pause(VaxpAnimation* VAXP_NONNULL anim);

/**
 * @brief Resume a paused animation
 */
void vaxp_animation_resume(VaxpAnimation* VAXP_NONNULL anim);

/**
 * @brief Stop the animation and reset to initial state
 */
void vaxp_animation_stop(VaxpAnimation* VAXP_NONNULL anim);

/**
 * @brief Reverse the animation direction
 */
void vaxp_animation_reverse(VaxpAnimation* VAXP_NONNULL anim);

/**
 * @brief Seek to a specific progress point
 * 
 * @param progress Normalized progress (0.0 to 1.0)
 */
void vaxp_animation_seek(VaxpAnimation* VAXP_NONNULL anim, VaxpF32 progress);

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
VaxpBool vaxp_animation_update(VaxpAnimation* VAXP_NONNULL anim, VaxpF64 delta_time);

/**
 * @brief Get current animated value
 * 
 * Returns the interpolated value based on current progress and easing.
 */
VAXP_INLINE VaxpF32 vaxp_animation_get_value(const VaxpAnimation* anim) {
    return anim ? anim->current_value : 0.0f;
}

/**
 * @brief Get current progress (0.0 to 1.0)
 */
VAXP_INLINE VaxpF32 vaxp_animation_get_progress(const VaxpAnimation* anim) {
    return anim ? anim->progress : 0.0f;
}

/**
 * @brief Check if animation is currently running
 */
VAXP_INLINE VaxpBool vaxp_animation_is_running(const VaxpAnimation* anim) {
    return anim && anim->state == VAXP_ANIM_STATE_RUNNING;
}

/**
 * @brief Check if animation has completed
 */
VAXP_INLINE VaxpBool vaxp_animation_is_completed(const VaxpAnimation* anim) {
    return anim && anim->state == VAXP_ANIM_STATE_COMPLETED;
}

/* ============================================================================
 * ANIMATION CONFIGURATION
 * ============================================================================ */

/**
 * @brief Set easing function
 */
void vaxp_animation_set_easing(VaxpAnimation* VAXP_NONNULL anim, VaxpEasing easing);

/**
 * @brief Set delay before animation starts
 */
void vaxp_animation_set_delay(VaxpAnimation* VAXP_NONNULL anim, VaxpF32 delay);

/**
 * @brief Set repeat mode and count
 * 
 * @param mode Repeat mode
 * @param count Number of times to repeat (-1 for infinite)
 */
void vaxp_animation_set_repeat(VaxpAnimation* VAXP_NONNULL anim, VaxpRepeatMode mode, VaxpI32 count);

/**
 * @brief Set update callback
 */
void vaxp_animation_set_on_update(
    VaxpAnimation* VAXP_NONNULL anim,
    VaxpAnimationUpdateFn VAXP_NULLABLE callback,
    void* VAXP_NULLABLE user_data
);

/**
 * @brief Set completion callback
 */
void vaxp_animation_set_on_complete(
    VaxpAnimation* VAXP_NONNULL anim,
    VaxpAnimationStateFn VAXP_NULLABLE callback,
    void* VAXP_NULLABLE user_data
);

/**
 * @brief Set start callback
 */
void vaxp_animation_set_on_start(
    VaxpAnimation* VAXP_NONNULL anim,
    VaxpAnimationStateFn VAXP_NULLABLE callback,
    void* VAXP_NULLABLE user_data
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
VaxpColor vaxp_color_lerp(VaxpColor from, VaxpColor to, VaxpF32 t);

/**
 * @brief Interpolate colors in HSL space (smoother for hue transitions)
 */
VaxpColor vaxp_color_lerp_hsl(VaxpColor from, VaxpColor to, VaxpF32 t);

/* ============================================================================
 * KEYFRAME ANIMATION
 * ============================================================================ */

/**
 * @brief A single keyframe in an animation
 */
typedef struct VaxpKeyframe {
    VaxpF32 time;          /* Time position (0.0 to 1.0, normalized) */
    VaxpF32 value;         /* Value at this keyframe */
    VaxpEasing easing;     /* Easing to next keyframe */
} VaxpKeyframe;

/**
 * @brief Keyframe animation for complex multi-point animations
 */
typedef struct VaxpKeyframeAnimation {
    VAXP_REF_HEADER;
    
    VaxpKeyframe* VAXP_OWNED keyframes;   /* Array of keyframes */
    VaxpU32 keyframe_count;                /* Number of keyframes */
    VaxpU32 keyframe_capacity;             /* Allocated capacity */
    
    VaxpF32 duration;                      /* Total duration in seconds */
    VaxpAnimationState state;              /* Current state */
    VaxpF32 elapsed;                       /* Elapsed time */
    VaxpF32 current_value;                 /* Current interpolated value */
    
    VaxpAnimationUpdateFn on_update;
    VaxpAnimationStateFn on_complete;
    void* callback_data;
} VaxpKeyframeAnimation;

/**
 * @brief Create a keyframe animation
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_keyframe_animation_create(VaxpF32 duration);

/**
 * @brief Add a keyframe
 * 
 * @param anim The animation
 * @param time Normalized time (0.0 to 1.0)
 * @param value Value at this keyframe
 * @param easing Easing to use when interpolating to next keyframe
 */
VaxpResult vaxp_keyframe_animation_add(
    VaxpKeyframeAnimation* VAXP_NONNULL anim,
    VaxpF32 time,
    VaxpF32 value,
    VaxpEasing easing
);

/**
 * @brief Start the keyframe animation
 */
void vaxp_keyframe_animation_start(VaxpKeyframeAnimation* VAXP_NONNULL anim);

/**
 * @brief Update the keyframe animation
 */
VaxpBool vaxp_keyframe_animation_update(VaxpKeyframeAnimation* VAXP_NONNULL anim, VaxpF64 delta_time);

/**
 * @brief Get current value
 */
VAXP_INLINE VaxpF32 vaxp_keyframe_animation_get_value(const VaxpKeyframeAnimation* anim) {
    return anim ? anim->current_value : 0.0f;
}

#ifdef __cplusplus
}
#endif

#endif /* VAXP_ANIMATION_H */
