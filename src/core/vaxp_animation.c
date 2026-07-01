/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_animation.c - Animation system implementation
 * 
 * Features:
 * - 30+ easing functions (Robert Penner's equations)
 * - Tween animations with full lifecycle control
 * - Keyframe animations for complex sequences
 * - Thread-safe with atomic state management
 */

#include "vaxp/core/vaxp_animation.h"
#include "vaxp/core/vaxp_memory.h"
#include <math.h>
#include <string.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

#define PI 3.14159265358979323846f
#define HALF_PI (PI / 2.0f)
#define TWO_PI (PI * 2.0f)

/* Elastic constants */
#define ELASTIC_AMPLITUDE 1.0f
#define ELASTIC_PERIOD 0.3f

/* Back overshoot amount */
#define BACK_OVERSHOOT 1.70158f

/* Initial keyframe capacity */
#define KEYFRAME_INITIAL_CAPACITY 8

/* ============================================================================
 * EASING FUNCTION IMPLEMENTATIONS
 * ============================================================================ */

/* Linear */
static VaxpF32 ease_linear(VaxpF32 t) {
    return t;
}

/* Quadratic */
static VaxpF32 ease_quad_in(VaxpF32 t) {
    return t * t;
}

static VaxpF32 ease_quad_out(VaxpF32 t) {
    return t * (2.0f - t);
}

static VaxpF32 ease_quad_in_out(VaxpF32 t) {
    return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
}

/* Cubic */
static VaxpF32 ease_cubic_in(VaxpF32 t) {
    return t * t * t;
}

static VaxpF32 ease_cubic_out(VaxpF32 t) {
    VaxpF32 f = t - 1.0f;
    return f * f * f + 1.0f;
}

static VaxpF32 ease_cubic_in_out(VaxpF32 t) {
    return t < 0.5f 
        ? 4.0f * t * t * t 
        : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

/* Quartic */
static VaxpF32 ease_quart_in(VaxpF32 t) {
    return t * t * t * t;
}

static VaxpF32 ease_quart_out(VaxpF32 t) {
    VaxpF32 f = t - 1.0f;
    return 1.0f - f * f * f * f;
}

static VaxpF32 ease_quart_in_out(VaxpF32 t) {
    return t < 0.5f 
        ? 8.0f * t * t * t * t 
        : 1.0f - powf(-2.0f * t + 2.0f, 4.0f) / 2.0f;
}

/* Quintic */
static VaxpF32 ease_quint_in(VaxpF32 t) {
    return t * t * t * t * t;
}

static VaxpF32 ease_quint_out(VaxpF32 t) {
    VaxpF32 f = t - 1.0f;
    return 1.0f + f * f * f * f * f;
}

static VaxpF32 ease_quint_in_out(VaxpF32 t) {
    return t < 0.5f 
        ? 16.0f * t * t * t * t * t 
        : 1.0f - powf(-2.0f * t + 2.0f, 5.0f) / 2.0f;
}

/* Sine */
static VaxpF32 ease_sine_in(VaxpF32 t) {
    return 1.0f - cosf(t * HALF_PI);
}

static VaxpF32 ease_sine_out(VaxpF32 t) {
    return sinf(t * HALF_PI);
}

static VaxpF32 ease_sine_in_out(VaxpF32 t) {
    return -(cosf(PI * t) - 1.0f) / 2.0f;
}

/* Exponential */
static VaxpF32 ease_expo_in(VaxpF32 t) {
    return t <= 0.0f ? 0.0f : powf(2.0f, 10.0f * (t - 1.0f));
}

static VaxpF32 ease_expo_out(VaxpF32 t) {
    return t >= 1.0f ? 1.0f : 1.0f - powf(2.0f, -10.0f * t);
}

static VaxpF32 ease_expo_in_out(VaxpF32 t) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    return t < 0.5f 
        ? powf(2.0f, 20.0f * t - 10.0f) / 2.0f 
        : (2.0f - powf(2.0f, -20.0f * t + 10.0f)) / 2.0f;
}

/* Circular */
static VaxpF32 ease_circ_in(VaxpF32 t) {
    return 1.0f - sqrtf(1.0f - t * t);
}

static VaxpF32 ease_circ_out(VaxpF32 t) {
    VaxpF32 f = t - 1.0f;
    return sqrtf(1.0f - f * f);
}

static VaxpF32 ease_circ_in_out(VaxpF32 t) {
    return t < 0.5f 
        ? (1.0f - sqrtf(1.0f - 4.0f * t * t)) / 2.0f 
        : (sqrtf(1.0f - powf(-2.0f * t + 2.0f, 2.0f)) + 1.0f) / 2.0f;
}

/* Elastic */
static VaxpF32 ease_elastic_in(VaxpF32 t) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    return -powf(2.0f, 10.0f * t - 10.0f) * sinf((t * 10.0f - 10.75f) * TWO_PI / 3.0f);
}

static VaxpF32 ease_elastic_out(VaxpF32 t) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    return powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * TWO_PI / 3.0f) + 1.0f;
}

static VaxpF32 ease_elastic_in_out(VaxpF32 t) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    VaxpF32 c = TWO_PI / 4.5f;
    return t < 0.5f
        ? -(powf(2.0f, 20.0f * t - 10.0f) * sinf((20.0f * t - 11.125f) * c)) / 2.0f
        : (powf(2.0f, -20.0f * t + 10.0f) * sinf((20.0f * t - 11.125f) * c)) / 2.0f + 1.0f;
}

/* Back */
static VaxpF32 ease_back_in(VaxpF32 t) {
    VaxpF32 c = BACK_OVERSHOOT + 1.0f;
    return c * t * t * t - BACK_OVERSHOOT * t * t;
}

static VaxpF32 ease_back_out(VaxpF32 t) {
    VaxpF32 c = BACK_OVERSHOOT + 1.0f;
    VaxpF32 f = t - 1.0f;
    return 1.0f + c * f * f * f + BACK_OVERSHOOT * f * f;
}

static VaxpF32 ease_back_in_out(VaxpF32 t) {
    VaxpF32 c = BACK_OVERSHOOT * 1.525f;
    return t < 0.5f
        ? (4.0f * t * t * ((c + 1.0f) * 2.0f * t - c)) / 2.0f
        : (powf(2.0f * t - 2.0f, 2.0f) * ((c + 1.0f) * (t * 2.0f - 2.0f) + c) + 2.0f) / 2.0f;
}

/* Bounce */
static VaxpF32 ease_bounce_out(VaxpF32 t) {
    VaxpF32 n1 = 7.5625f;
    VaxpF32 d1 = 2.75f;
    
    if (t < 1.0f / d1) {
        return n1 * t * t;
    } else if (t < 2.0f / d1) {
        t -= 1.5f / d1;
        return n1 * t * t + 0.75f;
    } else if (t < 2.5f / d1) {
        t -= 2.25f / d1;
        return n1 * t * t + 0.9375f;
    } else {
        t -= 2.625f / d1;
        return n1 * t * t + 0.984375f;
    }
}

static VaxpF32 ease_bounce_in(VaxpF32 t) {
    return 1.0f - ease_bounce_out(1.0f - t);
}

static VaxpF32 ease_bounce_in_out(VaxpF32 t) {
    return t < 0.5f
        ? (1.0f - ease_bounce_out(1.0f - 2.0f * t)) / 2.0f
        : (1.0f + ease_bounce_out(2.0f * t - 1.0f)) / 2.0f;
}

/* Easing function lookup table */
typedef VaxpF32 (*EasingFn)(VaxpF32);

static const EasingFn g_easing_functions[VAXP_EASING_COUNT] = {
    [VAXP_EASING_LINEAR] = ease_linear,
    
    [VAXP_EASING_QUAD_IN] = ease_quad_in,
    [VAXP_EASING_QUAD_OUT] = ease_quad_out,
    [VAXP_EASING_QUAD_IN_OUT] = ease_quad_in_out,
    
    [VAXP_EASING_CUBIC_IN] = ease_cubic_in,
    [VAXP_EASING_CUBIC_OUT] = ease_cubic_out,
    [VAXP_EASING_CUBIC_IN_OUT] = ease_cubic_in_out,
    
    [VAXP_EASING_QUART_IN] = ease_quart_in,
    [VAXP_EASING_QUART_OUT] = ease_quart_out,
    [VAXP_EASING_QUART_IN_OUT] = ease_quart_in_out,
    
    [VAXP_EASING_QUINT_IN] = ease_quint_in,
    [VAXP_EASING_QUINT_OUT] = ease_quint_out,
    [VAXP_EASING_QUINT_IN_OUT] = ease_quint_in_out,
    
    [VAXP_EASING_SINE_IN] = ease_sine_in,
    [VAXP_EASING_SINE_OUT] = ease_sine_out,
    [VAXP_EASING_SINE_IN_OUT] = ease_sine_in_out,
    
    [VAXP_EASING_EXPO_IN] = ease_expo_in,
    [VAXP_EASING_EXPO_OUT] = ease_expo_out,
    [VAXP_EASING_EXPO_IN_OUT] = ease_expo_in_out,
    
    [VAXP_EASING_CIRC_IN] = ease_circ_in,
    [VAXP_EASING_CIRC_OUT] = ease_circ_out,
    [VAXP_EASING_CIRC_IN_OUT] = ease_circ_in_out,
    
    [VAXP_EASING_ELASTIC_IN] = ease_elastic_in,
    [VAXP_EASING_ELASTIC_OUT] = ease_elastic_out,
    [VAXP_EASING_ELASTIC_IN_OUT] = ease_elastic_in_out,
    
    [VAXP_EASING_BACK_IN] = ease_back_in,
    [VAXP_EASING_BACK_OUT] = ease_back_out,
    [VAXP_EASING_BACK_IN_OUT] = ease_back_in_out,
    
    [VAXP_EASING_BOUNCE_IN] = ease_bounce_in,
    [VAXP_EASING_BOUNCE_OUT] = ease_bounce_out,
    [VAXP_EASING_BOUNCE_IN_OUT] = ease_bounce_in_out,
};

static const char* g_easing_names[VAXP_EASING_COUNT] = {
    [VAXP_EASING_LINEAR] = "linear",
    [VAXP_EASING_QUAD_IN] = "quad-in",
    [VAXP_EASING_QUAD_OUT] = "quad-out",
    [VAXP_EASING_QUAD_IN_OUT] = "quad-in-out",
    [VAXP_EASING_CUBIC_IN] = "cubic-in",
    [VAXP_EASING_CUBIC_OUT] = "cubic-out",
    [VAXP_EASING_CUBIC_IN_OUT] = "cubic-in-out",
    [VAXP_EASING_QUART_IN] = "quart-in",
    [VAXP_EASING_QUART_OUT] = "quart-out",
    [VAXP_EASING_QUART_IN_OUT] = "quart-in-out",
    [VAXP_EASING_QUINT_IN] = "quint-in",
    [VAXP_EASING_QUINT_OUT] = "quint-out",
    [VAXP_EASING_QUINT_IN_OUT] = "quint-in-out",
    [VAXP_EASING_SINE_IN] = "sine-in",
    [VAXP_EASING_SINE_OUT] = "sine-out",
    [VAXP_EASING_SINE_IN_OUT] = "sine-in-out",
    [VAXP_EASING_EXPO_IN] = "expo-in",
    [VAXP_EASING_EXPO_OUT] = "expo-out",
    [VAXP_EASING_EXPO_IN_OUT] = "expo-in-out",
    [VAXP_EASING_CIRC_IN] = "circ-in",
    [VAXP_EASING_CIRC_OUT] = "circ-out",
    [VAXP_EASING_CIRC_IN_OUT] = "circ-in-out",
    [VAXP_EASING_ELASTIC_IN] = "elastic-in",
    [VAXP_EASING_ELASTIC_OUT] = "elastic-out",
    [VAXP_EASING_ELASTIC_IN_OUT] = "elastic-in-out",
    [VAXP_EASING_BACK_IN] = "back-in",
    [VAXP_EASING_BACK_OUT] = "back-out",
    [VAXP_EASING_BACK_IN_OUT] = "back-in-out",
    [VAXP_EASING_BOUNCE_IN] = "bounce-in",
    [VAXP_EASING_BOUNCE_OUT] = "bounce-out",
    [VAXP_EASING_BOUNCE_IN_OUT] = "bounce-in-out",
};

/* ============================================================================
 * EASING PUBLIC API
 * ============================================================================ */

VaxpF32 vaxp_easing_apply(VaxpEasing easing, VaxpF32 t) {
    /* Clamp input to 0-1 range */
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    
    if (easing < 0 || easing >= VAXP_EASING_COUNT) {
        return t;  /* Fallback to linear */
    }
    
    EasingFn fn = g_easing_functions[easing];
    return fn ? fn(t) : t;
}

const char* vaxp_easing_name(VaxpEasing easing) {
    if (easing < 0 || easing >= VAXP_EASING_COUNT) {
        return "unknown";
    }
    return g_easing_names[easing];
}

/* ============================================================================
 * ANIMATION DESTRUCTOR
 * ============================================================================ */

static void animation_destructor(void* self) {
    (void)self;
    /* VaxpAnimation has no owned resources to free */
}

/* ============================================================================
 * ANIMATION LIFECYCLE
 * ============================================================================ */

VaxpResultPtr vaxp_animation_create(VaxpF32 from_value, VaxpF32 to_value, VaxpF32 duration) {
    if (duration < 0.0f) {
        return VAXP_ERR_PTR(VAXP_ERROR_INVALID_ARGUMENT);
    }
    
    VaxpAnimation* anim = (VaxpAnimation*)vaxp_alloc_zeroed(sizeof(VaxpAnimation));
    if (!anim) {
        return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
    }
    
    vaxp_ref_init(anim, sizeof(VaxpAnimation), animation_destructor, "VaxpAnimation");
    
    anim->from_value = from_value;
    anim->to_value = to_value;
    anim->duration = duration;
    anim->delay = 0.0f;
    anim->easing = VAXP_EASING_LINEAR;
    anim->repeat_mode = VAXP_REPEAT_NONE;
    anim->repeat_count = 0;
    anim->state = VAXP_ANIM_STATE_IDLE;
    anim->direction = VAXP_ANIM_DIRECTION_FORWARD;
    anim->current_value = from_value;
    
    return VAXP_OK_PTR(anim);
}

VaxpResultPtr vaxp_animation_create_with_config(const VaxpAnimationConfig* config) {
    VAXP_RETURN_IF_NULL(config, VAXP_ERR_PTR(VAXP_ERROR_NULL_POINTER));
    
    VaxpResultPtr result = vaxp_animation_create(config->from, config->to, config->duration);
    if (!result.ok) return result;
    
    VaxpAnimation* anim = (VaxpAnimation*)result.value;
    
    anim->delay = config->delay;
    anim->easing = config->easing;
    anim->repeat_mode = config->repeat;
    anim->repeat_count = config->repeat_count;
    anim->on_update = config->on_update;
    anim->on_complete = config->on_complete;
    anim->callback_data = config->user_data;
    
    return VAXP_OK_PTR(anim);
}

/* ============================================================================
 * ANIMATION PLAYBACK CONTROL
 * ============================================================================ */

void vaxp_animation_start(VaxpAnimation* anim) {
    VAXP_RETURN_VOID_IF_NULL(anim);
    
    anim->elapsed = 0.0f;
    anim->delay_elapsed = 0.0f;
    anim->current_repeat = 0;
    anim->state = VAXP_ANIM_STATE_RUNNING;
    anim->direction = VAXP_ANIM_DIRECTION_FORWARD;
    anim->current_value = anim->from_value;
    anim->progress = 0.0f;
    
    if (anim->on_start) {
        anim->on_start(anim, VAXP_ANIM_STATE_RUNNING, anim->callback_data);
    }
}

void vaxp_animation_pause(VaxpAnimation* anim) {
    VAXP_RETURN_VOID_IF_NULL(anim);
    
    if (anim->state == VAXP_ANIM_STATE_RUNNING) {
        anim->state = VAXP_ANIM_STATE_PAUSED;
    }
}

void vaxp_animation_resume(VaxpAnimation* anim) {
    VAXP_RETURN_VOID_IF_NULL(anim);
    
    if (anim->state == VAXP_ANIM_STATE_PAUSED) {
        anim->state = VAXP_ANIM_STATE_RUNNING;
    }
}

void vaxp_animation_stop(VaxpAnimation* anim) {
    VAXP_RETURN_VOID_IF_NULL(anim);
    
    anim->state = VAXP_ANIM_STATE_IDLE;
    anim->elapsed = 0.0f;
    anim->delay_elapsed = 0.0f;
    anim->current_value = anim->from_value;
    anim->progress = 0.0f;
}

void vaxp_animation_reverse(VaxpAnimation* anim) {
    VAXP_RETURN_VOID_IF_NULL(anim);
    
    if (anim->direction == VAXP_ANIM_DIRECTION_FORWARD) {
        anim->direction = VAXP_ANIM_DIRECTION_REVERSE;
    } else {
        anim->direction = VAXP_ANIM_DIRECTION_FORWARD;
    }
}

void vaxp_animation_seek(VaxpAnimation* anim, VaxpF32 progress) {
    VAXP_RETURN_VOID_IF_NULL(anim);
    
    progress = VAXP_CLAMP(progress, 0.0f, 1.0f);
    anim->elapsed = progress * anim->duration;
    anim->progress = progress;
    
    VaxpF32 eased = vaxp_easing_apply(anim->easing, progress);
    anim->current_value = anim->from_value + (anim->to_value - anim->from_value) * eased;
}

/* ============================================================================
 * ANIMATION UPDATE
 * ============================================================================ */

VaxpBool vaxp_animation_update(VaxpAnimation* anim, VaxpF64 delta_time) {
    VAXP_RETURN_IF_NULL(anim, VAXP_FALSE);
    
    if (anim->state != VAXP_ANIM_STATE_RUNNING) {
        return VAXP_FALSE;
    }
    
    /* Handle delay phase */
    if (anim->delay_elapsed < anim->delay) {
        anim->delay_elapsed += (VaxpF32)delta_time;
        if (anim->delay_elapsed < anim->delay) {
            return VAXP_TRUE;  /* Still in delay */
        }
        /* Delay completed, adjust delta for overshoot */
        delta_time = anim->delay_elapsed - anim->delay;
    }
    
    /* Update elapsed time */
    VaxpF32 dt = (VaxpF32)delta_time;
    if (anim->direction == VAXP_ANIM_DIRECTION_REVERSE) {
        anim->elapsed -= dt;
    } else {
        anim->elapsed += dt;
    }
    
    /* Calculate raw progress */
    VaxpF32 raw_progress;
    if (anim->duration <= 0.0f) {
        raw_progress = 1.0f;
    } else {
        raw_progress = anim->elapsed / anim->duration;
    }
    
    /* Check for completion or looping */
    VaxpBool completed = VAXP_FALSE;
    
    if (anim->direction == VAXP_ANIM_DIRECTION_FORWARD && raw_progress >= 1.0f) {
        /* Forward completion */
        if (anim->repeat_mode == VAXP_REPEAT_NONE ||
            (anim->repeat_count >= 0 && anim->current_repeat >= anim->repeat_count)) {
            completed = VAXP_TRUE;
            raw_progress = 1.0f;
        } else {
            /* Handle repeat */
            anim->current_repeat++;
            
            if (anim->on_repeat) {
                anim->on_repeat(anim, anim->state, anim->callback_data);
            }
            
            if (anim->repeat_mode == VAXP_REPEAT_PING_PONG) {
                anim->direction = VAXP_ANIM_DIRECTION_REVERSE;
                anim->elapsed = anim->duration;
            } else {
                anim->elapsed = fmodf(anim->elapsed, anim->duration);
            }
            raw_progress = anim->elapsed / anim->duration;
        }
    } else if (anim->direction == VAXP_ANIM_DIRECTION_REVERSE && raw_progress <= 0.0f) {
        /* Reverse completion (in ping-pong mode) */
        anim->current_repeat++;
        
        if (anim->repeat_count >= 0 && anim->current_repeat >= anim->repeat_count) {
            completed = VAXP_TRUE;
            raw_progress = 0.0f;
        } else {
            if (anim->on_repeat) {
                anim->on_repeat(anim, anim->state, anim->callback_data);
            }
            anim->direction = VAXP_ANIM_DIRECTION_FORWARD;
            anim->elapsed = 0.0f;
            raw_progress = 0.0f;
        }
    }
    
    /* Clamp and store progress */
    anim->progress = VAXP_CLAMP(raw_progress, 0.0f, 1.0f);
    
    /* Apply easing */
    VaxpF32 eased = vaxp_easing_apply(anim->easing, anim->progress);
    
    /* Calculate current value */
    anim->current_value = anim->from_value + (anim->to_value - anim->from_value) * eased;
    
    /* Fire update callback */
    if (anim->on_update) {
        anim->on_update(anim, anim->current_value, anim->progress, anim->callback_data);
    }
    
    /* Handle completion */
    if (completed) {
        anim->state = VAXP_ANIM_STATE_COMPLETED;
        if (anim->on_complete) {
            anim->on_complete(anim, VAXP_ANIM_STATE_COMPLETED, anim->callback_data);
        }
        return VAXP_FALSE;
    }
    
    return VAXP_TRUE;
}

/* ============================================================================
 * ANIMATION CONFIGURATION
 * ============================================================================ */

void vaxp_animation_set_easing(VaxpAnimation* anim, VaxpEasing easing) {
    VAXP_RETURN_VOID_IF_NULL(anim);
    anim->easing = easing;
}

void vaxp_animation_set_delay(VaxpAnimation* anim, VaxpF32 delay) {
    VAXP_RETURN_VOID_IF_NULL(anim);
    anim->delay = delay >= 0.0f ? delay : 0.0f;
}

void vaxp_animation_set_repeat(VaxpAnimation* anim, VaxpRepeatMode mode, VaxpI32 count) {
    VAXP_RETURN_VOID_IF_NULL(anim);
    anim->repeat_mode = mode;
    anim->repeat_count = count;
}

void vaxp_animation_set_on_update(VaxpAnimation* anim, VaxpAnimationUpdateFn callback, void* user_data) {
    VAXP_RETURN_VOID_IF_NULL(anim);
    anim->on_update = callback;
    anim->callback_data = user_data;
}

void vaxp_animation_set_on_complete(VaxpAnimation* anim, VaxpAnimationStateFn callback, void* user_data) {
    VAXP_RETURN_VOID_IF_NULL(anim);
    anim->on_complete = callback;
    if (user_data) anim->callback_data = user_data;
}

void vaxp_animation_set_on_start(VaxpAnimation* anim, VaxpAnimationStateFn callback, void* user_data) {
    VAXP_RETURN_VOID_IF_NULL(anim);
    anim->on_start = callback;
    if (user_data) anim->callback_data = user_data;
}

/* ============================================================================
 * COLOR INTERPOLATION
 * ============================================================================ */

VaxpColor vaxp_color_lerp(VaxpColor from, VaxpColor to, VaxpF32 t) {
    t = VAXP_CLAMP(t, 0.0f, 1.0f);
    
    return (VaxpColor){
        .r = (VaxpU8)(from.r + (VaxpI32)(to.r - from.r) * t),
        .g = (VaxpU8)(from.g + (VaxpI32)(to.g - from.g) * t),
        .b = (VaxpU8)(from.b + (VaxpI32)(to.b - from.b) * t),
        .a = (VaxpU8)(from.a + (VaxpI32)(to.a - from.a) * t),
    };
}

/* RGB to HSL conversion helper */
static void rgb_to_hsl(VaxpColor c, VaxpF32* h, VaxpF32* s, VaxpF32* l) {
    VaxpF32 r = c.r / 255.0f;
    VaxpF32 g = c.g / 255.0f;
    VaxpF32 b = c.b / 255.0f;
    
    VaxpF32 max = VAXP_MAX(VAXP_MAX(r, g), b);
    VaxpF32 min = VAXP_MIN(VAXP_MIN(r, g), b);
    VaxpF32 d = max - min;
    
    *l = (max + min) / 2.0f;
    
    if (d == 0.0f) {
        *h = *s = 0.0f;
    } else {
        *s = *l > 0.5f ? d / (2.0f - max - min) : d / (max + min);
        
        if (max == r) {
            *h = (g - b) / d + (g < b ? 6.0f : 0.0f);
        } else if (max == g) {
            *h = (b - r) / d + 2.0f;
        } else {
            *h = (r - g) / d + 4.0f;
        }
        *h /= 6.0f;
    }
}

/* HSL to RGB conversion helper */
static VaxpF32 hue_to_rgb(VaxpF32 p, VaxpF32 q, VaxpF32 t) {
    if (t < 0.0f) t += 1.0f;
    if (t > 1.0f) t -= 1.0f;
    if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
    if (t < 1.0f / 2.0f) return q;
    if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
    return p;
}

static VaxpColor hsl_to_rgb(VaxpF32 h, VaxpF32 s, VaxpF32 l, VaxpU8 a) {
    VaxpF32 r, g, b;
    
    if (s == 0.0f) {
        r = g = b = l;
    } else {
        VaxpF32 q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
        VaxpF32 p = 2.0f * l - q;
        r = hue_to_rgb(p, q, h + 1.0f / 3.0f);
        g = hue_to_rgb(p, q, h);
        b = hue_to_rgb(p, q, h - 1.0f / 3.0f);
    }
    
    return (VaxpColor){
        .r = (VaxpU8)(r * 255.0f),
        .g = (VaxpU8)(g * 255.0f),
        .b = (VaxpU8)(b * 255.0f),
        .a = a,
    };
}

VaxpColor vaxp_color_lerp_hsl(VaxpColor from, VaxpColor to, VaxpF32 t) {
    t = VAXP_CLAMP(t, 0.0f, 1.0f);
    
    VaxpF32 h1, s1, l1, h2, s2, l2;
    rgb_to_hsl(from, &h1, &s1, &l1);
    rgb_to_hsl(to, &h2, &s2, &l2);
    
    /* Interpolate in HSL space */
    VaxpF32 h = h1 + (h2 - h1) * t;
    VaxpF32 s = s1 + (s2 - s1) * t;
    VaxpF32 l = l1 + (l2 - l1) * t;
    VaxpU8 a = (VaxpU8)(from.a + (VaxpI32)(to.a - from.a) * t);
    
    return hsl_to_rgb(h, s, l, a);
}

/* ============================================================================
 * KEYFRAME ANIMATION IMPLEMENTATION
 * ============================================================================ */

static void keyframe_animation_destructor(void* self) {
    VaxpKeyframeAnimation* anim = (VaxpKeyframeAnimation*)self;
    
    if (anim->keyframes) {
        vaxp_free(anim->keyframes, sizeof(VaxpKeyframe) * anim->keyframe_capacity);
        anim->keyframes = NULL;
        anim->keyframe_count = 0;
        anim->keyframe_capacity = 0;
    }
}

VaxpResultPtr vaxp_keyframe_animation_create(VaxpF32 duration) {
    if (duration <= 0.0f) {
        return VAXP_ERR_PTR(VAXP_ERROR_INVALID_ARGUMENT);
    }
    
    VaxpKeyframeAnimation* anim = (VaxpKeyframeAnimation*)vaxp_alloc_zeroed(sizeof(VaxpKeyframeAnimation));
    if (!anim) {
        return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
    }
    
    vaxp_ref_init(anim, sizeof(VaxpKeyframeAnimation), keyframe_animation_destructor, "VaxpKeyframeAnimation");
    
    anim->duration = duration;
    anim->state = VAXP_ANIM_STATE_IDLE;
    
    /* Allocate initial keyframe array */
    anim->keyframes = (VaxpKeyframe*)vaxp_alloc_zeroed(sizeof(VaxpKeyframe) * KEYFRAME_INITIAL_CAPACITY);
    if (!anim->keyframes) {
        vaxp_free(anim, sizeof(VaxpKeyframeAnimation));
        return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
    }
    anim->keyframe_capacity = KEYFRAME_INITIAL_CAPACITY;
    
    return VAXP_OK_PTR(anim);
}

VaxpResult vaxp_keyframe_animation_add(VaxpKeyframeAnimation* anim, VaxpF32 time, VaxpF32 value, VaxpEasing easing) {
    VAXP_ENSURE_NOT_NULL(anim);
    
    if (time < 0.0f || time > 1.0f) {
        return VAXP_ERR_UNIT(VAXP_ERROR_INVALID_ARGUMENT);
    }
    
    /* Grow array if needed */
    if (anim->keyframe_count >= anim->keyframe_capacity) {
        VaxpU32 new_cap = anim->keyframe_capacity * 2;
        VaxpKeyframe* new_keyframes = VAXP_REALLOC_ARRAY(VaxpKeyframe, anim->keyframes,
                                                           anim->keyframe_capacity, new_cap);
        if (!new_keyframes) {
            return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_MEMORY);
        }
        anim->keyframes = new_keyframes;
        anim->keyframe_capacity = new_cap;
    }
    
    /* Add keyframe */
    VaxpKeyframe kf = {
        .time = time,
        .value = value,
        .easing = easing,
    };
    anim->keyframes[anim->keyframe_count++] = kf;
    
    /* Sort keyframes by time (insertion sort for small arrays) */
    for (VaxpU32 i = anim->keyframe_count - 1; i > 0; i--) {
        if (anim->keyframes[i].time < anim->keyframes[i - 1].time) {
            VaxpKeyframe tmp = anim->keyframes[i];
            anim->keyframes[i] = anim->keyframes[i - 1];
            anim->keyframes[i - 1] = tmp;
        } else {
            break;
        }
    }
    
    return VAXP_OK_UNIT();
}

void vaxp_keyframe_animation_start(VaxpKeyframeAnimation* anim) {
    VAXP_RETURN_VOID_IF_NULL(anim);
    
    anim->elapsed = 0.0f;
    anim->state = VAXP_ANIM_STATE_RUNNING;
    
    if (anim->keyframe_count > 0) {
        anim->current_value = anim->keyframes[0].value;
    }
}

VaxpBool vaxp_keyframe_animation_update(VaxpKeyframeAnimation* anim, VaxpF64 delta_time) {
    VAXP_RETURN_IF_NULL(anim, VAXP_FALSE);
    
    if (anim->state != VAXP_ANIM_STATE_RUNNING) {
        return VAXP_FALSE;
    }
    
    if (anim->keyframe_count == 0) {
        anim->state = VAXP_ANIM_STATE_COMPLETED;
        return VAXP_FALSE;
    }
    
    anim->elapsed += (VaxpF32)delta_time;
    VaxpF32 progress = anim->elapsed / anim->duration;
    
    if (progress >= 1.0f) {
        progress = 1.0f;
        anim->state = VAXP_ANIM_STATE_COMPLETED;
        anim->current_value = anim->keyframes[anim->keyframe_count - 1].value;
        
        if (anim->on_complete) {
            anim->on_complete(NULL, VAXP_ANIM_STATE_COMPLETED, anim->callback_data);
        }
        
        return VAXP_FALSE;
    }
    
    /* Find surrounding keyframes */
    VaxpU32 next_idx = 0;
    for (VaxpU32 i = 0; i < anim->keyframe_count; i++) {
        if (anim->keyframes[i].time > progress) {
            next_idx = i;
            break;
        }
        next_idx = i + 1;
    }
    
    if (next_idx == 0) {
        anim->current_value = anim->keyframes[0].value;
    } else if (next_idx >= anim->keyframe_count) {
        anim->current_value = anim->keyframes[anim->keyframe_count - 1].value;
    } else {
        /* Interpolate between keyframes */
        VaxpKeyframe* prev = &anim->keyframes[next_idx - 1];
        VaxpKeyframe* next = &anim->keyframes[next_idx];
        
        VaxpF32 local_t = (progress - prev->time) / (next->time - prev->time);
        VaxpF32 eased_t = vaxp_easing_apply(prev->easing, local_t);
        
        anim->current_value = prev->value + (next->value - prev->value) * eased_t;
    }
    
    /* Fire update callback */
    if (anim->on_update) {
        anim->on_update(NULL, anim->current_value, progress, anim->callback_data);
    }
    
    return VAXP_TRUE;
}
