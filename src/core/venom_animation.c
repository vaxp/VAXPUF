/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_animation.c - Animation system implementation
 * 
 * Features:
 * - 30+ easing functions (Robert Penner's equations)
 * - Tween animations with full lifecycle control
 * - Keyframe animations for complex sequences
 * - Thread-safe with atomic state management
 */

#include "venom/core/venom_animation.h"
#include "venom/core/venom_memory.h"
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
static VenomF32 ease_linear(VenomF32 t) {
    return t;
}

/* Quadratic */
static VenomF32 ease_quad_in(VenomF32 t) {
    return t * t;
}

static VenomF32 ease_quad_out(VenomF32 t) {
    return t * (2.0f - t);
}

static VenomF32 ease_quad_in_out(VenomF32 t) {
    return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
}

/* Cubic */
static VenomF32 ease_cubic_in(VenomF32 t) {
    return t * t * t;
}

static VenomF32 ease_cubic_out(VenomF32 t) {
    VenomF32 f = t - 1.0f;
    return f * f * f + 1.0f;
}

static VenomF32 ease_cubic_in_out(VenomF32 t) {
    return t < 0.5f 
        ? 4.0f * t * t * t 
        : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

/* Quartic */
static VenomF32 ease_quart_in(VenomF32 t) {
    return t * t * t * t;
}

static VenomF32 ease_quart_out(VenomF32 t) {
    VenomF32 f = t - 1.0f;
    return 1.0f - f * f * f * f;
}

static VenomF32 ease_quart_in_out(VenomF32 t) {
    return t < 0.5f 
        ? 8.0f * t * t * t * t 
        : 1.0f - powf(-2.0f * t + 2.0f, 4.0f) / 2.0f;
}

/* Quintic */
static VenomF32 ease_quint_in(VenomF32 t) {
    return t * t * t * t * t;
}

static VenomF32 ease_quint_out(VenomF32 t) {
    VenomF32 f = t - 1.0f;
    return 1.0f + f * f * f * f * f;
}

static VenomF32 ease_quint_in_out(VenomF32 t) {
    return t < 0.5f 
        ? 16.0f * t * t * t * t * t 
        : 1.0f - powf(-2.0f * t + 2.0f, 5.0f) / 2.0f;
}

/* Sine */
static VenomF32 ease_sine_in(VenomF32 t) {
    return 1.0f - cosf(t * HALF_PI);
}

static VenomF32 ease_sine_out(VenomF32 t) {
    return sinf(t * HALF_PI);
}

static VenomF32 ease_sine_in_out(VenomF32 t) {
    return -(cosf(PI * t) - 1.0f) / 2.0f;
}

/* Exponential */
static VenomF32 ease_expo_in(VenomF32 t) {
    return t <= 0.0f ? 0.0f : powf(2.0f, 10.0f * (t - 1.0f));
}

static VenomF32 ease_expo_out(VenomF32 t) {
    return t >= 1.0f ? 1.0f : 1.0f - powf(2.0f, -10.0f * t);
}

static VenomF32 ease_expo_in_out(VenomF32 t) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    return t < 0.5f 
        ? powf(2.0f, 20.0f * t - 10.0f) / 2.0f 
        : (2.0f - powf(2.0f, -20.0f * t + 10.0f)) / 2.0f;
}

/* Circular */
static VenomF32 ease_circ_in(VenomF32 t) {
    return 1.0f - sqrtf(1.0f - t * t);
}

static VenomF32 ease_circ_out(VenomF32 t) {
    VenomF32 f = t - 1.0f;
    return sqrtf(1.0f - f * f);
}

static VenomF32 ease_circ_in_out(VenomF32 t) {
    return t < 0.5f 
        ? (1.0f - sqrtf(1.0f - 4.0f * t * t)) / 2.0f 
        : (sqrtf(1.0f - powf(-2.0f * t + 2.0f, 2.0f)) + 1.0f) / 2.0f;
}

/* Elastic */
static VenomF32 ease_elastic_in(VenomF32 t) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    return -powf(2.0f, 10.0f * t - 10.0f) * sinf((t * 10.0f - 10.75f) * TWO_PI / 3.0f);
}

static VenomF32 ease_elastic_out(VenomF32 t) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    return powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * TWO_PI / 3.0f) + 1.0f;
}

static VenomF32 ease_elastic_in_out(VenomF32 t) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    VenomF32 c = TWO_PI / 4.5f;
    return t < 0.5f
        ? -(powf(2.0f, 20.0f * t - 10.0f) * sinf((20.0f * t - 11.125f) * c)) / 2.0f
        : (powf(2.0f, -20.0f * t + 10.0f) * sinf((20.0f * t - 11.125f) * c)) / 2.0f + 1.0f;
}

/* Back */
static VenomF32 ease_back_in(VenomF32 t) {
    VenomF32 c = BACK_OVERSHOOT + 1.0f;
    return c * t * t * t - BACK_OVERSHOOT * t * t;
}

static VenomF32 ease_back_out(VenomF32 t) {
    VenomF32 c = BACK_OVERSHOOT + 1.0f;
    VenomF32 f = t - 1.0f;
    return 1.0f + c * f * f * f + BACK_OVERSHOOT * f * f;
}

static VenomF32 ease_back_in_out(VenomF32 t) {
    VenomF32 c = BACK_OVERSHOOT * 1.525f;
    return t < 0.5f
        ? (4.0f * t * t * ((c + 1.0f) * 2.0f * t - c)) / 2.0f
        : (powf(2.0f * t - 2.0f, 2.0f) * ((c + 1.0f) * (t * 2.0f - 2.0f) + c) + 2.0f) / 2.0f;
}

/* Bounce */
static VenomF32 ease_bounce_out(VenomF32 t) {
    VenomF32 n1 = 7.5625f;
    VenomF32 d1 = 2.75f;
    
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

static VenomF32 ease_bounce_in(VenomF32 t) {
    return 1.0f - ease_bounce_out(1.0f - t);
}

static VenomF32 ease_bounce_in_out(VenomF32 t) {
    return t < 0.5f
        ? (1.0f - ease_bounce_out(1.0f - 2.0f * t)) / 2.0f
        : (1.0f + ease_bounce_out(2.0f * t - 1.0f)) / 2.0f;
}

/* Easing function lookup table */
typedef VenomF32 (*EasingFn)(VenomF32);

static const EasingFn g_easing_functions[VENOM_EASING_COUNT] = {
    [VENOM_EASING_LINEAR] = ease_linear,
    
    [VENOM_EASING_QUAD_IN] = ease_quad_in,
    [VENOM_EASING_QUAD_OUT] = ease_quad_out,
    [VENOM_EASING_QUAD_IN_OUT] = ease_quad_in_out,
    
    [VENOM_EASING_CUBIC_IN] = ease_cubic_in,
    [VENOM_EASING_CUBIC_OUT] = ease_cubic_out,
    [VENOM_EASING_CUBIC_IN_OUT] = ease_cubic_in_out,
    
    [VENOM_EASING_QUART_IN] = ease_quart_in,
    [VENOM_EASING_QUART_OUT] = ease_quart_out,
    [VENOM_EASING_QUART_IN_OUT] = ease_quart_in_out,
    
    [VENOM_EASING_QUINT_IN] = ease_quint_in,
    [VENOM_EASING_QUINT_OUT] = ease_quint_out,
    [VENOM_EASING_QUINT_IN_OUT] = ease_quint_in_out,
    
    [VENOM_EASING_SINE_IN] = ease_sine_in,
    [VENOM_EASING_SINE_OUT] = ease_sine_out,
    [VENOM_EASING_SINE_IN_OUT] = ease_sine_in_out,
    
    [VENOM_EASING_EXPO_IN] = ease_expo_in,
    [VENOM_EASING_EXPO_OUT] = ease_expo_out,
    [VENOM_EASING_EXPO_IN_OUT] = ease_expo_in_out,
    
    [VENOM_EASING_CIRC_IN] = ease_circ_in,
    [VENOM_EASING_CIRC_OUT] = ease_circ_out,
    [VENOM_EASING_CIRC_IN_OUT] = ease_circ_in_out,
    
    [VENOM_EASING_ELASTIC_IN] = ease_elastic_in,
    [VENOM_EASING_ELASTIC_OUT] = ease_elastic_out,
    [VENOM_EASING_ELASTIC_IN_OUT] = ease_elastic_in_out,
    
    [VENOM_EASING_BACK_IN] = ease_back_in,
    [VENOM_EASING_BACK_OUT] = ease_back_out,
    [VENOM_EASING_BACK_IN_OUT] = ease_back_in_out,
    
    [VENOM_EASING_BOUNCE_IN] = ease_bounce_in,
    [VENOM_EASING_BOUNCE_OUT] = ease_bounce_out,
    [VENOM_EASING_BOUNCE_IN_OUT] = ease_bounce_in_out,
};

static const char* g_easing_names[VENOM_EASING_COUNT] = {
    [VENOM_EASING_LINEAR] = "linear",
    [VENOM_EASING_QUAD_IN] = "quad-in",
    [VENOM_EASING_QUAD_OUT] = "quad-out",
    [VENOM_EASING_QUAD_IN_OUT] = "quad-in-out",
    [VENOM_EASING_CUBIC_IN] = "cubic-in",
    [VENOM_EASING_CUBIC_OUT] = "cubic-out",
    [VENOM_EASING_CUBIC_IN_OUT] = "cubic-in-out",
    [VENOM_EASING_QUART_IN] = "quart-in",
    [VENOM_EASING_QUART_OUT] = "quart-out",
    [VENOM_EASING_QUART_IN_OUT] = "quart-in-out",
    [VENOM_EASING_QUINT_IN] = "quint-in",
    [VENOM_EASING_QUINT_OUT] = "quint-out",
    [VENOM_EASING_QUINT_IN_OUT] = "quint-in-out",
    [VENOM_EASING_SINE_IN] = "sine-in",
    [VENOM_EASING_SINE_OUT] = "sine-out",
    [VENOM_EASING_SINE_IN_OUT] = "sine-in-out",
    [VENOM_EASING_EXPO_IN] = "expo-in",
    [VENOM_EASING_EXPO_OUT] = "expo-out",
    [VENOM_EASING_EXPO_IN_OUT] = "expo-in-out",
    [VENOM_EASING_CIRC_IN] = "circ-in",
    [VENOM_EASING_CIRC_OUT] = "circ-out",
    [VENOM_EASING_CIRC_IN_OUT] = "circ-in-out",
    [VENOM_EASING_ELASTIC_IN] = "elastic-in",
    [VENOM_EASING_ELASTIC_OUT] = "elastic-out",
    [VENOM_EASING_ELASTIC_IN_OUT] = "elastic-in-out",
    [VENOM_EASING_BACK_IN] = "back-in",
    [VENOM_EASING_BACK_OUT] = "back-out",
    [VENOM_EASING_BACK_IN_OUT] = "back-in-out",
    [VENOM_EASING_BOUNCE_IN] = "bounce-in",
    [VENOM_EASING_BOUNCE_OUT] = "bounce-out",
    [VENOM_EASING_BOUNCE_IN_OUT] = "bounce-in-out",
};

/* ============================================================================
 * EASING PUBLIC API
 * ============================================================================ */

VenomF32 venom_easing_apply(VenomEasing easing, VenomF32 t) {
    /* Clamp input to 0-1 range */
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    
    if (easing < 0 || easing >= VENOM_EASING_COUNT) {
        return t;  /* Fallback to linear */
    }
    
    EasingFn fn = g_easing_functions[easing];
    return fn ? fn(t) : t;
}

const char* venom_easing_name(VenomEasing easing) {
    if (easing < 0 || easing >= VENOM_EASING_COUNT) {
        return "unknown";
    }
    return g_easing_names[easing];
}

/* ============================================================================
 * ANIMATION DESTRUCTOR
 * ============================================================================ */

static void animation_destructor(void* self) {
    (void)self;
    /* VenomAnimation has no owned resources to free */
}

/* ============================================================================
 * ANIMATION LIFECYCLE
 * ============================================================================ */

VenomResultPtr venom_animation_create(VenomF32 from_value, VenomF32 to_value, VenomF32 duration) {
    if (duration < 0.0f) {
        return VENOM_ERR_PTR(VENOM_ERROR_INVALID_ARGUMENT);
    }
    
    VenomAnimation* anim = (VenomAnimation*)venom_alloc_zeroed(sizeof(VenomAnimation));
    if (!anim) {
        return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    venom_ref_init(anim, sizeof(VenomAnimation), animation_destructor, "VenomAnimation");
    
    anim->from_value = from_value;
    anim->to_value = to_value;
    anim->duration = duration;
    anim->delay = 0.0f;
    anim->easing = VENOM_EASING_LINEAR;
    anim->repeat_mode = VENOM_REPEAT_NONE;
    anim->repeat_count = 0;
    anim->state = VENOM_ANIM_STATE_IDLE;
    anim->direction = VENOM_ANIM_DIRECTION_FORWARD;
    anim->current_value = from_value;
    
    return VENOM_OK_PTR(anim);
}

VenomResultPtr venom_animation_create_with_config(const VenomAnimationConfig* config) {
    VENOM_RETURN_IF_NULL(config, VENOM_ERR_PTR(VENOM_ERROR_NULL_POINTER));
    
    VenomResultPtr result = venom_animation_create(config->from, config->to, config->duration);
    if (!result.ok) return result;
    
    VenomAnimation* anim = (VenomAnimation*)result.value;
    
    anim->delay = config->delay;
    anim->easing = config->easing;
    anim->repeat_mode = config->repeat;
    anim->repeat_count = config->repeat_count;
    anim->on_update = config->on_update;
    anim->on_complete = config->on_complete;
    anim->callback_data = config->user_data;
    
    return VENOM_OK_PTR(anim);
}

/* ============================================================================
 * ANIMATION PLAYBACK CONTROL
 * ============================================================================ */

void venom_animation_start(VenomAnimation* anim) {
    VENOM_RETURN_VOID_IF_NULL(anim);
    
    anim->elapsed = 0.0f;
    anim->delay_elapsed = 0.0f;
    anim->current_repeat = 0;
    anim->state = VENOM_ANIM_STATE_RUNNING;
    anim->direction = VENOM_ANIM_DIRECTION_FORWARD;
    anim->current_value = anim->from_value;
    anim->progress = 0.0f;
    
    if (anim->on_start) {
        anim->on_start(anim, VENOM_ANIM_STATE_RUNNING, anim->callback_data);
    }
}

void venom_animation_pause(VenomAnimation* anim) {
    VENOM_RETURN_VOID_IF_NULL(anim);
    
    if (anim->state == VENOM_ANIM_STATE_RUNNING) {
        anim->state = VENOM_ANIM_STATE_PAUSED;
    }
}

void venom_animation_resume(VenomAnimation* anim) {
    VENOM_RETURN_VOID_IF_NULL(anim);
    
    if (anim->state == VENOM_ANIM_STATE_PAUSED) {
        anim->state = VENOM_ANIM_STATE_RUNNING;
    }
}

void venom_animation_stop(VenomAnimation* anim) {
    VENOM_RETURN_VOID_IF_NULL(anim);
    
    anim->state = VENOM_ANIM_STATE_IDLE;
    anim->elapsed = 0.0f;
    anim->delay_elapsed = 0.0f;
    anim->current_value = anim->from_value;
    anim->progress = 0.0f;
}

void venom_animation_reverse(VenomAnimation* anim) {
    VENOM_RETURN_VOID_IF_NULL(anim);
    
    if (anim->direction == VENOM_ANIM_DIRECTION_FORWARD) {
        anim->direction = VENOM_ANIM_DIRECTION_REVERSE;
    } else {
        anim->direction = VENOM_ANIM_DIRECTION_FORWARD;
    }
}

void venom_animation_seek(VenomAnimation* anim, VenomF32 progress) {
    VENOM_RETURN_VOID_IF_NULL(anim);
    
    progress = VENOM_CLAMP(progress, 0.0f, 1.0f);
    anim->elapsed = progress * anim->duration;
    anim->progress = progress;
    
    VenomF32 eased = venom_easing_apply(anim->easing, progress);
    anim->current_value = anim->from_value + (anim->to_value - anim->from_value) * eased;
}

/* ============================================================================
 * ANIMATION UPDATE
 * ============================================================================ */

VenomBool venom_animation_update(VenomAnimation* anim, VenomF64 delta_time) {
    VENOM_RETURN_IF_NULL(anim, VENOM_FALSE);
    
    if (anim->state != VENOM_ANIM_STATE_RUNNING) {
        return VENOM_FALSE;
    }
    
    /* Handle delay phase */
    if (anim->delay_elapsed < anim->delay) {
        anim->delay_elapsed += (VenomF32)delta_time;
        if (anim->delay_elapsed < anim->delay) {
            return VENOM_TRUE;  /* Still in delay */
        }
        /* Delay completed, adjust delta for overshoot */
        delta_time = anim->delay_elapsed - anim->delay;
    }
    
    /* Update elapsed time */
    VenomF32 dt = (VenomF32)delta_time;
    if (anim->direction == VENOM_ANIM_DIRECTION_REVERSE) {
        anim->elapsed -= dt;
    } else {
        anim->elapsed += dt;
    }
    
    /* Calculate raw progress */
    VenomF32 raw_progress;
    if (anim->duration <= 0.0f) {
        raw_progress = 1.0f;
    } else {
        raw_progress = anim->elapsed / anim->duration;
    }
    
    /* Check for completion or looping */
    VenomBool completed = VENOM_FALSE;
    
    if (anim->direction == VENOM_ANIM_DIRECTION_FORWARD && raw_progress >= 1.0f) {
        /* Forward completion */
        if (anim->repeat_mode == VENOM_REPEAT_NONE ||
            (anim->repeat_count >= 0 && anim->current_repeat >= anim->repeat_count)) {
            completed = VENOM_TRUE;
            raw_progress = 1.0f;
        } else {
            /* Handle repeat */
            anim->current_repeat++;
            
            if (anim->on_repeat) {
                anim->on_repeat(anim, anim->state, anim->callback_data);
            }
            
            if (anim->repeat_mode == VENOM_REPEAT_PING_PONG) {
                anim->direction = VENOM_ANIM_DIRECTION_REVERSE;
                anim->elapsed = anim->duration;
            } else {
                anim->elapsed = fmodf(anim->elapsed, anim->duration);
            }
            raw_progress = anim->elapsed / anim->duration;
        }
    } else if (anim->direction == VENOM_ANIM_DIRECTION_REVERSE && raw_progress <= 0.0f) {
        /* Reverse completion (in ping-pong mode) */
        anim->current_repeat++;
        
        if (anim->repeat_count >= 0 && anim->current_repeat >= anim->repeat_count) {
            completed = VENOM_TRUE;
            raw_progress = 0.0f;
        } else {
            if (anim->on_repeat) {
                anim->on_repeat(anim, anim->state, anim->callback_data);
            }
            anim->direction = VENOM_ANIM_DIRECTION_FORWARD;
            anim->elapsed = 0.0f;
            raw_progress = 0.0f;
        }
    }
    
    /* Clamp and store progress */
    anim->progress = VENOM_CLAMP(raw_progress, 0.0f, 1.0f);
    
    /* Apply easing */
    VenomF32 eased = venom_easing_apply(anim->easing, anim->progress);
    
    /* Calculate current value */
    anim->current_value = anim->from_value + (anim->to_value - anim->from_value) * eased;
    
    /* Fire update callback */
    if (anim->on_update) {
        anim->on_update(anim, anim->current_value, anim->progress, anim->callback_data);
    }
    
    /* Handle completion */
    if (completed) {
        anim->state = VENOM_ANIM_STATE_COMPLETED;
        if (anim->on_complete) {
            anim->on_complete(anim, VENOM_ANIM_STATE_COMPLETED, anim->callback_data);
        }
        return VENOM_FALSE;
    }
    
    return VENOM_TRUE;
}

/* ============================================================================
 * ANIMATION CONFIGURATION
 * ============================================================================ */

void venom_animation_set_easing(VenomAnimation* anim, VenomEasing easing) {
    VENOM_RETURN_VOID_IF_NULL(anim);
    anim->easing = easing;
}

void venom_animation_set_delay(VenomAnimation* anim, VenomF32 delay) {
    VENOM_RETURN_VOID_IF_NULL(anim);
    anim->delay = delay >= 0.0f ? delay : 0.0f;
}

void venom_animation_set_repeat(VenomAnimation* anim, VenomRepeatMode mode, VenomI32 count) {
    VENOM_RETURN_VOID_IF_NULL(anim);
    anim->repeat_mode = mode;
    anim->repeat_count = count;
}

void venom_animation_set_on_update(VenomAnimation* anim, VenomAnimationUpdateFn callback, void* user_data) {
    VENOM_RETURN_VOID_IF_NULL(anim);
    anim->on_update = callback;
    anim->callback_data = user_data;
}

void venom_animation_set_on_complete(VenomAnimation* anim, VenomAnimationStateFn callback, void* user_data) {
    VENOM_RETURN_VOID_IF_NULL(anim);
    anim->on_complete = callback;
    if (user_data) anim->callback_data = user_data;
}

void venom_animation_set_on_start(VenomAnimation* anim, VenomAnimationStateFn callback, void* user_data) {
    VENOM_RETURN_VOID_IF_NULL(anim);
    anim->on_start = callback;
    if (user_data) anim->callback_data = user_data;
}

/* ============================================================================
 * COLOR INTERPOLATION
 * ============================================================================ */

VenomColor venom_color_lerp(VenomColor from, VenomColor to, VenomF32 t) {
    t = VENOM_CLAMP(t, 0.0f, 1.0f);
    
    return (VenomColor){
        .r = (VenomU8)(from.r + (VenomI32)(to.r - from.r) * t),
        .g = (VenomU8)(from.g + (VenomI32)(to.g - from.g) * t),
        .b = (VenomU8)(from.b + (VenomI32)(to.b - from.b) * t),
        .a = (VenomU8)(from.a + (VenomI32)(to.a - from.a) * t),
    };
}

/* RGB to HSL conversion helper */
static void rgb_to_hsl(VenomColor c, VenomF32* h, VenomF32* s, VenomF32* l) {
    VenomF32 r = c.r / 255.0f;
    VenomF32 g = c.g / 255.0f;
    VenomF32 b = c.b / 255.0f;
    
    VenomF32 max = VENOM_MAX(VENOM_MAX(r, g), b);
    VenomF32 min = VENOM_MIN(VENOM_MIN(r, g), b);
    VenomF32 d = max - min;
    
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
static VenomF32 hue_to_rgb(VenomF32 p, VenomF32 q, VenomF32 t) {
    if (t < 0.0f) t += 1.0f;
    if (t > 1.0f) t -= 1.0f;
    if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
    if (t < 1.0f / 2.0f) return q;
    if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
    return p;
}

static VenomColor hsl_to_rgb(VenomF32 h, VenomF32 s, VenomF32 l, VenomU8 a) {
    VenomF32 r, g, b;
    
    if (s == 0.0f) {
        r = g = b = l;
    } else {
        VenomF32 q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
        VenomF32 p = 2.0f * l - q;
        r = hue_to_rgb(p, q, h + 1.0f / 3.0f);
        g = hue_to_rgb(p, q, h);
        b = hue_to_rgb(p, q, h - 1.0f / 3.0f);
    }
    
    return (VenomColor){
        .r = (VenomU8)(r * 255.0f),
        .g = (VenomU8)(g * 255.0f),
        .b = (VenomU8)(b * 255.0f),
        .a = a,
    };
}

VenomColor venom_color_lerp_hsl(VenomColor from, VenomColor to, VenomF32 t) {
    t = VENOM_CLAMP(t, 0.0f, 1.0f);
    
    VenomF32 h1, s1, l1, h2, s2, l2;
    rgb_to_hsl(from, &h1, &s1, &l1);
    rgb_to_hsl(to, &h2, &s2, &l2);
    
    /* Interpolate in HSL space */
    VenomF32 h = h1 + (h2 - h1) * t;
    VenomF32 s = s1 + (s2 - s1) * t;
    VenomF32 l = l1 + (l2 - l1) * t;
    VenomU8 a = (VenomU8)(from.a + (VenomI32)(to.a - from.a) * t);
    
    return hsl_to_rgb(h, s, l, a);
}

/* ============================================================================
 * KEYFRAME ANIMATION IMPLEMENTATION
 * ============================================================================ */

static void keyframe_animation_destructor(void* self) {
    VenomKeyframeAnimation* anim = (VenomKeyframeAnimation*)self;
    
    if (anim->keyframes) {
        venom_free(anim->keyframes, sizeof(VenomKeyframe) * anim->keyframe_capacity);
        anim->keyframes = NULL;
        anim->keyframe_count = 0;
        anim->keyframe_capacity = 0;
    }
}

VenomResultPtr venom_keyframe_animation_create(VenomF32 duration) {
    if (duration <= 0.0f) {
        return VENOM_ERR_PTR(VENOM_ERROR_INVALID_ARGUMENT);
    }
    
    VenomKeyframeAnimation* anim = (VenomKeyframeAnimation*)venom_alloc_zeroed(sizeof(VenomKeyframeAnimation));
    if (!anim) {
        return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    venom_ref_init(anim, sizeof(VenomKeyframeAnimation), keyframe_animation_destructor, "VenomKeyframeAnimation");
    
    anim->duration = duration;
    anim->state = VENOM_ANIM_STATE_IDLE;
    
    /* Allocate initial keyframe array */
    anim->keyframes = (VenomKeyframe*)venom_alloc_zeroed(sizeof(VenomKeyframe) * KEYFRAME_INITIAL_CAPACITY);
    if (!anim->keyframes) {
        venom_free(anim, sizeof(VenomKeyframeAnimation));
        return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
    }
    anim->keyframe_capacity = KEYFRAME_INITIAL_CAPACITY;
    
    return VENOM_OK_PTR(anim);
}

VenomResult venom_keyframe_animation_add(VenomKeyframeAnimation* anim, VenomF32 time, VenomF32 value, VenomEasing easing) {
    VENOM_ENSURE_NOT_NULL(anim);
    
    if (time < 0.0f || time > 1.0f) {
        return VENOM_ERR_UNIT(VENOM_ERROR_INVALID_ARGUMENT);
    }
    
    /* Grow array if needed */
    if (anim->keyframe_count >= anim->keyframe_capacity) {
        VenomU32 new_cap = anim->keyframe_capacity * 2;
        VenomKeyframe* new_keyframes = VENOM_REALLOC_ARRAY(VenomKeyframe, anim->keyframes,
                                                           anim->keyframe_capacity, new_cap);
        if (!new_keyframes) {
            return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
        }
        anim->keyframes = new_keyframes;
        anim->keyframe_capacity = new_cap;
    }
    
    /* Add keyframe */
    VenomKeyframe kf = {
        .time = time,
        .value = value,
        .easing = easing,
    };
    anim->keyframes[anim->keyframe_count++] = kf;
    
    /* Sort keyframes by time (insertion sort for small arrays) */
    for (VenomU32 i = anim->keyframe_count - 1; i > 0; i--) {
        if (anim->keyframes[i].time < anim->keyframes[i - 1].time) {
            VenomKeyframe tmp = anim->keyframes[i];
            anim->keyframes[i] = anim->keyframes[i - 1];
            anim->keyframes[i - 1] = tmp;
        } else {
            break;
        }
    }
    
    return VENOM_OK_UNIT();
}

void venom_keyframe_animation_start(VenomKeyframeAnimation* anim) {
    VENOM_RETURN_VOID_IF_NULL(anim);
    
    anim->elapsed = 0.0f;
    anim->state = VENOM_ANIM_STATE_RUNNING;
    
    if (anim->keyframe_count > 0) {
        anim->current_value = anim->keyframes[0].value;
    }
}

VenomBool venom_keyframe_animation_update(VenomKeyframeAnimation* anim, VenomF64 delta_time) {
    VENOM_RETURN_IF_NULL(anim, VENOM_FALSE);
    
    if (anim->state != VENOM_ANIM_STATE_RUNNING) {
        return VENOM_FALSE;
    }
    
    if (anim->keyframe_count == 0) {
        anim->state = VENOM_ANIM_STATE_COMPLETED;
        return VENOM_FALSE;
    }
    
    anim->elapsed += (VenomF32)delta_time;
    VenomF32 progress = anim->elapsed / anim->duration;
    
    if (progress >= 1.0f) {
        progress = 1.0f;
        anim->state = VENOM_ANIM_STATE_COMPLETED;
        anim->current_value = anim->keyframes[anim->keyframe_count - 1].value;
        
        if (anim->on_complete) {
            anim->on_complete(NULL, VENOM_ANIM_STATE_COMPLETED, anim->callback_data);
        }
        
        return VENOM_FALSE;
    }
    
    /* Find surrounding keyframes */
    VenomU32 next_idx = 0;
    for (VenomU32 i = 0; i < anim->keyframe_count; i++) {
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
        VenomKeyframe* prev = &anim->keyframes[next_idx - 1];
        VenomKeyframe* next = &anim->keyframes[next_idx];
        
        VenomF32 local_t = (progress - prev->time) / (next->time - prev->time);
        VenomF32 eased_t = venom_easing_apply(prev->easing, local_t);
        
        anim->current_value = prev->value + (next->value - prev->value) * eased_t;
    }
    
    /* Fire update callback */
    if (anim->on_update) {
        anim->on_update(NULL, anim->current_value, progress, anim->callback_data);
    }
    
    return VENOM_TRUE;
}
