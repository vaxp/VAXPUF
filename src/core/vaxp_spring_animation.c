/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_spring_animation.c - Physics-based spring animations
 * 
 * Uses semi-implicit Euler integration for stable simulation.
 */

#include "vaxp/core/vaxp_spring_animation.h"
#include "vaxp/core/vaxp_memory.h"
#include <math.h>

/* ============================================================================
 * SPRING PRESET CONFIGURATIONS
 * ============================================================================ */

typedef struct SpringPresetConfig {
    VaxpF32 stiffness;
    VaxpF32 damping;
    VaxpF32 mass;
} SpringPresetConfig;

static const SpringPresetConfig g_spring_presets[] = {
    [VAXP_SPRING_DEFAULT]  = { .stiffness = 100.0f, .damping = 10.0f,  .mass = 1.0f },
    [VAXP_SPRING_GENTLE]   = { .stiffness = 50.0f,  .damping = 14.0f,  .mass = 1.0f },
    [VAXP_SPRING_WOBBLY]   = { .stiffness = 180.0f, .damping = 12.0f,  .mass = 1.0f },
    [VAXP_SPRING_STIFF]    = { .stiffness = 210.0f, .damping = 20.0f,  .mass = 1.0f },
    [VAXP_SPRING_SLOW]     = { .stiffness = 40.0f,  .damping = 10.0f,  .mass = 1.5f },
    [VAXP_SPRING_MOLASSES] = { .stiffness = 20.0f,  .damping = 14.0f,  .mass = 2.0f },
};

/* ============================================================================
 * SPRING DESTRUCTOR
 * ============================================================================ */

static void spring_animation_destructor(void* self) {
    (void)self;
    /* VaxpSpringAnimation has no owned resources */
}

/* ============================================================================
 * SPRING CREATION
 * ============================================================================ */

VaxpResultPtr vaxp_spring_animation_create(VaxpF32 from_value, VaxpF32 to_value) {
    VaxpSpringAnimation* spring = (VaxpSpringAnimation*)vaxp_alloc_zeroed(sizeof(VaxpSpringAnimation));
    if (!spring) {
        return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
    }
    
    vaxp_ref_init(spring, sizeof(VaxpSpringAnimation), spring_animation_destructor, "VaxpSpringAnimation");
    
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
    
    spring->state = VAXP_ANIM_STATE_IDLE;
    
    return VAXP_OK_PTR(spring);
}

VaxpResultPtr vaxp_spring_animation_create_preset(VaxpF32 from_value, VaxpF32 to_value, VaxpSpringPreset preset) {
    VaxpResultPtr result = vaxp_spring_animation_create(from_value, to_value);
    if (!result.ok) return result;
    
    VaxpSpringAnimation* spring = (VaxpSpringAnimation*)result.value;
    vaxp_spring_animation_apply_preset(spring, preset);
    
    return VAXP_OK_PTR(spring);
}

/* ============================================================================
 * SPRING CONFIGURATION
 * ============================================================================ */

void vaxp_spring_animation_set_params(VaxpSpringAnimation* spring, 
                                        VaxpF32 stiffness, 
                                        VaxpF32 damping, 
                                        VaxpF32 mass) {
    VAXP_RETURN_VOID_IF_NULL(spring);
    
    spring->stiffness = stiffness > 0.0f ? stiffness : 1.0f;
    spring->damping = damping >= 0.0f ? damping : 0.0f;
    spring->mass = mass > 0.0f ? mass : 1.0f;
}

void vaxp_spring_animation_apply_preset(VaxpSpringAnimation* spring, VaxpSpringPreset preset) {
    VAXP_RETURN_VOID_IF_NULL(spring);
    
    if (preset >= 0 && preset <= VAXP_SPRING_MOLASSES) {
        const SpringPresetConfig* config = &g_spring_presets[preset];
        spring->stiffness = config->stiffness;
        spring->damping = config->damping;
        spring->mass = config->mass;
    }
}

void vaxp_spring_animation_set_thresholds(VaxpSpringAnimation* spring,
                                            VaxpF32 velocity_threshold,
                                            VaxpF32 displacement_threshold) {
    VAXP_RETURN_VOID_IF_NULL(spring);
    
    spring->velocity_threshold = velocity_threshold > 0.0f ? velocity_threshold : 0.001f;
    spring->displacement_threshold = displacement_threshold > 0.0f ? displacement_threshold : 0.001f;
}

/* ============================================================================
 * SPRING PLAYBACK CONTROL
 * ============================================================================ */

void vaxp_spring_animation_start(VaxpSpringAnimation* spring) {
    VAXP_RETURN_VOID_IF_NULL(spring);
    
    spring->current_value = spring->from_value;
    spring->velocity = 0.0f;
    spring->state = VAXP_ANIM_STATE_RUNNING;
}

void vaxp_spring_animation_stop(VaxpSpringAnimation* spring) {
    VAXP_RETURN_VOID_IF_NULL(spring);
    
    spring->state = VAXP_ANIM_STATE_IDLE;
    spring->velocity = 0.0f;
}

void vaxp_spring_animation_set_target(VaxpSpringAnimation* spring, VaxpF32 to_value) {
    VAXP_RETURN_VOID_IF_NULL(spring);
    spring->to_value = to_value;
}

void vaxp_spring_animation_add_velocity(VaxpSpringAnimation* spring, VaxpF32 velocity) {
    VAXP_RETURN_VOID_IF_NULL(spring);
    spring->velocity += velocity;
}

/* ============================================================================
 * SPRING UPDATE (Physics Simulation)
 * ============================================================================ */

VaxpBool vaxp_spring_animation_update(VaxpSpringAnimation* spring, VaxpF64 delta_time) {
    VAXP_RETURN_IF_NULL(spring, VAXP_FALSE);
    
    if (spring->state != VAXP_ANIM_STATE_RUNNING) {
        return VAXP_FALSE;
    }
    
    VaxpF32 dt = (VaxpF32)delta_time;
    
    /* Clamp dt to prevent instability with large time steps */
    if (dt > 0.033f) {
        dt = 0.033f;  /* Max ~30fps step */
    }
    
    /* Calculate displacement from target */
    VaxpF32 displacement = spring->current_value - spring->to_value;
    
    /* Spring force: F = -k * x */
    VaxpF32 spring_force = -spring->stiffness * displacement;
    
    /* Damping force: F = -d * v */
    VaxpF32 damping_force = -spring->damping * spring->velocity;
    
    /* Total force */
    VaxpF32 total_force = spring_force + damping_force;
    
    /* Acceleration: a = F / m */
    VaxpF32 acceleration = total_force / spring->mass;
    
    /* Semi-implicit Euler integration (more stable than explicit Euler) */
    /* First update velocity, then use new velocity to update position */
    spring->velocity += acceleration * dt;
    spring->current_value += spring->velocity * dt;
    
    /* Calculate progress (approximate) */
    VaxpF32 total_distance = spring->to_value - spring->from_value;
    VaxpF32 current_distance = spring->current_value - spring->from_value;
    VaxpF32 progress = total_distance != 0.0f ? current_distance / total_distance : 1.0f;
    
    /* Fire update callback */
    if (spring->on_update) {
        spring->on_update(NULL, spring->current_value, progress, spring->callback_data);
    }
    
    /* Check for completion (settled at target) */
    VaxpF32 abs_displacement = displacement >= 0.0f ? displacement : -displacement;
    VaxpF32 abs_velocity = spring->velocity >= 0.0f ? spring->velocity : -spring->velocity;
    
    if (abs_displacement < spring->displacement_threshold && 
        abs_velocity < spring->velocity_threshold) {
        /* Snap to target */
        spring->current_value = spring->to_value;
        spring->velocity = 0.0f;
        spring->state = VAXP_ANIM_STATE_COMPLETED;
        
        if (spring->on_complete) {
            spring->on_complete(NULL, VAXP_ANIM_STATE_COMPLETED, spring->callback_data);
        }
        
        return VAXP_FALSE;
    }
    
    return VAXP_TRUE;
}

/* ============================================================================
 * SPRING CALLBACKS
 * ============================================================================ */

void vaxp_spring_animation_set_on_update(VaxpSpringAnimation* spring,
                                           VaxpAnimationUpdateFn callback,
                                           void* user_data) {
    VAXP_RETURN_VOID_IF_NULL(spring);
    spring->on_update = callback;
    spring->callback_data = user_data;
}

void vaxp_spring_animation_set_on_complete(VaxpSpringAnimation* spring,
                                             VaxpAnimationStateFn callback,
                                             void* user_data) {
    VAXP_RETURN_VOID_IF_NULL(spring);
    spring->on_complete = callback;
    if (user_data) spring->callback_data = user_data;
}
