/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_spring_animation.c - Physics-based spring animations
 * 
 * Uses semi-implicit Euler integration for stable simulation.
 */

#include "venom/core/venom_spring_animation.h"
#include "venom/core/venom_memory.h"
#include <math.h>

/* ============================================================================
 * SPRING PRESET CONFIGURATIONS
 * ============================================================================ */

typedef struct SpringPresetConfig {
    VenomF32 stiffness;
    VenomF32 damping;
    VenomF32 mass;
} SpringPresetConfig;

static const SpringPresetConfig g_spring_presets[] = {
    [VENOM_SPRING_DEFAULT]  = { .stiffness = 100.0f, .damping = 10.0f,  .mass = 1.0f },
    [VENOM_SPRING_GENTLE]   = { .stiffness = 50.0f,  .damping = 14.0f,  .mass = 1.0f },
    [VENOM_SPRING_WOBBLY]   = { .stiffness = 180.0f, .damping = 12.0f,  .mass = 1.0f },
    [VENOM_SPRING_STIFF]    = { .stiffness = 210.0f, .damping = 20.0f,  .mass = 1.0f },
    [VENOM_SPRING_SLOW]     = { .stiffness = 40.0f,  .damping = 10.0f,  .mass = 1.5f },
    [VENOM_SPRING_MOLASSES] = { .stiffness = 20.0f,  .damping = 14.0f,  .mass = 2.0f },
};

/* ============================================================================
 * SPRING DESTRUCTOR
 * ============================================================================ */

static void spring_animation_destructor(void* self) {
    (void)self;
    /* VenomSpringAnimation has no owned resources */
}

/* ============================================================================
 * SPRING CREATION
 * ============================================================================ */

VenomResultPtr venom_spring_animation_create(VenomF32 from_value, VenomF32 to_value) {
    VenomSpringAnimation* spring = (VenomSpringAnimation*)venom_alloc_zeroed(sizeof(VenomSpringAnimation));
    if (!spring) {
        return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    venom_ref_init(spring, sizeof(VenomSpringAnimation), spring_animation_destructor, "VenomSpringAnimation");
    
    spring->from_value = from_value;
    spring->to_value = to_value;
    spring->current_value = from_value;
    spring->velocity = 0.0f;
    
    /* Default physics parameters (balanced spring) */
    spring->stiffness = 100.0f;
    spring->damping = 10.0f;
    spring->mass = 1.0f;
    
    /* Default thresholds */
    spring->velocity_threshold = 0.001f;
    spring->displacement_threshold = 0.001f;
    
    spring->state = VENOM_ANIM_STATE_IDLE;
    
    return VENOM_OK_PTR(spring);
}

VenomResultPtr venom_spring_animation_create_preset(VenomF32 from_value, VenomF32 to_value, VenomSpringPreset preset) {
    VenomResultPtr result = venom_spring_animation_create(from_value, to_value);
    if (!result.ok) return result;
    
    VenomSpringAnimation* spring = (VenomSpringAnimation*)result.value;
    venom_spring_animation_apply_preset(spring, preset);
    
    return VENOM_OK_PTR(spring);
}

/* ============================================================================
 * SPRING CONFIGURATION
 * ============================================================================ */

void venom_spring_animation_set_params(VenomSpringAnimation* spring, 
                                        VenomF32 stiffness, 
                                        VenomF32 damping, 
                                        VenomF32 mass) {
    VENOM_RETURN_VOID_IF_NULL(spring);
    
    spring->stiffness = stiffness > 0.0f ? stiffness : 1.0f;
    spring->damping = damping >= 0.0f ? damping : 0.0f;
    spring->mass = mass > 0.0f ? mass : 1.0f;
}

void venom_spring_animation_apply_preset(VenomSpringAnimation* spring, VenomSpringPreset preset) {
    VENOM_RETURN_VOID_IF_NULL(spring);
    
    if (preset >= 0 && preset <= VENOM_SPRING_MOLASSES) {
        const SpringPresetConfig* config = &g_spring_presets[preset];
        spring->stiffness = config->stiffness;
        spring->damping = config->damping;
        spring->mass = config->mass;
    }
}

void venom_spring_animation_set_thresholds(VenomSpringAnimation* spring,
                                            VenomF32 velocity_threshold,
                                            VenomF32 displacement_threshold) {
    VENOM_RETURN_VOID_IF_NULL(spring);
    
    spring->velocity_threshold = velocity_threshold > 0.0f ? velocity_threshold : 0.001f;
    spring->displacement_threshold = displacement_threshold > 0.0f ? displacement_threshold : 0.001f;
}

/* ============================================================================
 * SPRING PLAYBACK CONTROL
 * ============================================================================ */

void venom_spring_animation_start(VenomSpringAnimation* spring) {
    VENOM_RETURN_VOID_IF_NULL(spring);
    
    spring->current_value = spring->from_value;
    spring->velocity = 0.0f;
    spring->state = VENOM_ANIM_STATE_RUNNING;
}

void venom_spring_animation_stop(VenomSpringAnimation* spring) {
    VENOM_RETURN_VOID_IF_NULL(spring);
    
    spring->state = VENOM_ANIM_STATE_IDLE;
    spring->velocity = 0.0f;
}

void venom_spring_animation_set_target(VenomSpringAnimation* spring, VenomF32 to_value) {
    VENOM_RETURN_VOID_IF_NULL(spring);
    spring->to_value = to_value;
}

void venom_spring_animation_add_velocity(VenomSpringAnimation* spring, VenomF32 velocity) {
    VENOM_RETURN_VOID_IF_NULL(spring);
    spring->velocity += velocity;
}

/* ============================================================================
 * SPRING UPDATE (Physics Simulation)
 * ============================================================================ */

VenomBool venom_spring_animation_update(VenomSpringAnimation* spring, VenomF64 delta_time) {
    VENOM_RETURN_IF_NULL(spring, VENOM_FALSE);
    
    if (spring->state != VENOM_ANIM_STATE_RUNNING) {
        return VENOM_FALSE;
    }
    
    VenomF32 dt = (VenomF32)delta_time;
    
    /* Clamp dt to prevent instability with large time steps */
    if (dt > 0.033f) {
        dt = 0.033f;  /* Max ~30fps step */
    }
    
    /* Calculate displacement from target */
    VenomF32 displacement = spring->current_value - spring->to_value;
    
    /* Spring force: F = -k * x */
    VenomF32 spring_force = -spring->stiffness * displacement;
    
    /* Damping force: F = -d * v */
    VenomF32 damping_force = -spring->damping * spring->velocity;
    
    /* Total force */
    VenomF32 total_force = spring_force + damping_force;
    
    /* Acceleration: a = F / m */
    VenomF32 acceleration = total_force / spring->mass;
    
    /* Semi-implicit Euler integration (more stable than explicit Euler) */
    /* First update velocity, then use new velocity to update position */
    spring->velocity += acceleration * dt;
    spring->current_value += spring->velocity * dt;
    
    /* Calculate progress (approximate) */
    VenomF32 total_distance = spring->to_value - spring->from_value;
    VenomF32 current_distance = spring->current_value - spring->from_value;
    VenomF32 progress = total_distance != 0.0f ? current_distance / total_distance : 1.0f;
    
    /* Fire update callback */
    if (spring->on_update) {
        spring->on_update(NULL, spring->current_value, progress, spring->callback_data);
    }
    
    /* Check for completion (settled at target) */
    VenomF32 abs_displacement = displacement >= 0.0f ? displacement : -displacement;
    VenomF32 abs_velocity = spring->velocity >= 0.0f ? spring->velocity : -spring->velocity;
    
    if (abs_displacement < spring->displacement_threshold && 
        abs_velocity < spring->velocity_threshold) {
        /* Snap to target */
        spring->current_value = spring->to_value;
        spring->velocity = 0.0f;
        spring->state = VENOM_ANIM_STATE_COMPLETED;
        
        if (spring->on_complete) {
            spring->on_complete(NULL, VENOM_ANIM_STATE_COMPLETED, spring->callback_data);
        }
        
        return VENOM_FALSE;
    }
    
    return VENOM_TRUE;
}

/* ============================================================================
 * SPRING CALLBACKS
 * ============================================================================ */

void venom_spring_animation_set_on_update(VenomSpringAnimation* spring,
                                           VenomAnimationUpdateFn callback,
                                           void* user_data) {
    VENOM_RETURN_VOID_IF_NULL(spring);
    spring->on_update = callback;
    spring->callback_data = user_data;
}

void venom_spring_animation_set_on_complete(VenomSpringAnimation* spring,
                                             VenomAnimationStateFn callback,
                                             void* user_data) {
    VENOM_RETURN_VOID_IF_NULL(spring);
    spring->on_complete = callback;
    if (user_data) spring->callback_data = user_data;
}
