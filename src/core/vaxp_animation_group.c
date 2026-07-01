/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_animation_group.c - Animation groups for sequence and parallel animations
 */

#include "vaxp/core/vaxp_animation_group.h"
#include "vaxp/core/vaxp_memory.h"

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

#define INITIAL_CHILDREN_CAPACITY 8

/* ============================================================================
 * GROUP DESTRUCTOR
 * ============================================================================ */

static void animation_group_destructor(void* self) {
    VaxpAnimationGroup* group = (VaxpAnimationGroup*)self;
    
    /* Unref all children */
    for (VaxpU32 i = 0; i < group->children_count; i++) {
        if (group->children[i]) {
            vaxp_unref(group->children[i]);
            group->children[i] = NULL;
        }
    }
    
    /* Free children array */
    if (group->children) {
        vaxp_free(group->children, sizeof(VaxpAnimation*) * group->children_capacity);
        group->children = NULL;
        group->children_count = 0;
        group->children_capacity = 0;
    }
}

/* ============================================================================
 * GROUP CREATION
 * ============================================================================ */

VaxpResultPtr vaxp_animation_group_create(VaxpAnimationGroupType type) {
    VaxpAnimationGroup* group = (VaxpAnimationGroup*)vaxp_alloc_zeroed(sizeof(VaxpAnimationGroup));
    if (!group) {
        return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
    }
    
    vaxp_ref_init(group, sizeof(VaxpAnimationGroup), animation_group_destructor, "VaxpAnimationGroup");
    
    group->type = type;
    group->state = VAXP_ANIM_STATE_IDLE;
    group->stagger_delay = 0.1f;  /* Default stagger delay */
    
    /* Allocate initial children array */
    group->children = (VaxpAnimation**)vaxp_alloc_zeroed(sizeof(VaxpAnimation*) * INITIAL_CHILDREN_CAPACITY);
    if (!group->children) {
        vaxp_free(group, sizeof(VaxpAnimationGroup));
        return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
    }
    group->children_capacity = INITIAL_CHILDREN_CAPACITY;
    
    return VAXP_OK_PTR(group);
}

/* ============================================================================
 * CHILD MANAGEMENT
 * ============================================================================ */

VaxpResult vaxp_animation_group_add(VaxpAnimationGroup* group, VaxpAnimation* anim) {
    VAXP_ENSURE_NOT_NULL(group);
    VAXP_ENSURE_NOT_NULL(anim);
    
    /* Grow array if needed */
    if (group->children_count >= group->children_capacity) {
        VaxpU32 new_cap = group->children_capacity * 2;
        VaxpAnimation** new_children = VAXP_REALLOC_ARRAY(VaxpAnimation*, group->children,
                                                             group->children_capacity, new_cap);
        if (!new_children) {
            return VAXP_ERR_UNIT(VAXP_ERROR_OUT_OF_MEMORY);
        }
        group->children = new_children;
        group->children_capacity = new_cap;
    }
    
    /* Add and ref the child */
    group->children[group->children_count++] = (VaxpAnimation*)vaxp_ref(anim);
    
    return VAXP_OK_UNIT();
}

VaxpResult vaxp_animation_group_remove(VaxpAnimationGroup* group, VaxpAnimation* anim) {
    VAXP_ENSURE_NOT_NULL(group);
    VAXP_ENSURE_NOT_NULL(anim);
    
    /* Find the animation */
    for (VaxpU32 i = 0; i < group->children_count; i++) {
        if (group->children[i] == anim) {
            /* Unref the animation */
            vaxp_unref(anim);
            
            /* Shift remaining animations */
            for (VaxpU32 j = i; j < group->children_count - 1; j++) {
                group->children[j] = group->children[j + 1];
            }
            group->children_count--;
            group->children[group->children_count] = NULL;
            
            return VAXP_OK_UNIT();
        }
    }
    
    return VAXP_ERR_UNIT(VAXP_ERROR_NOT_FOUND);
}

void vaxp_animation_group_clear(VaxpAnimationGroup* group) {
    VAXP_RETURN_VOID_IF_NULL(group);
    
    for (VaxpU32 i = 0; i < group->children_count; i++) {
        if (group->children[i]) {
            vaxp_unref(group->children[i]);
            group->children[i] = NULL;
        }
    }
    group->children_count = 0;
}

void vaxp_animation_group_set_stagger_delay(VaxpAnimationGroup* group, VaxpF32 delay) {
    VAXP_RETURN_VOID_IF_NULL(group);
    group->stagger_delay = delay >= 0.0f ? delay : 0.0f;
}

/* ============================================================================
 * GROUP PLAYBACK CONTROL
 * ============================================================================ */

void vaxp_animation_group_start(VaxpAnimationGroup* group) {
    VAXP_RETURN_VOID_IF_NULL(group);
    
    group->state = VAXP_ANIM_STATE_RUNNING;
    group->current_index = 0;
    
    switch (group->type) {
        case VAXP_ANIM_GROUP_SEQUENCE:
            /* Start only the first animation */
            if (group->children_count > 0) {
                vaxp_animation_start(group->children[0]);
            }
            break;
            
        case VAXP_ANIM_GROUP_PARALLEL:
            /* Start all animations immediately */
            for (VaxpU32 i = 0; i < group->children_count; i++) {
                vaxp_animation_start(group->children[i]);
            }
            break;
            
        case VAXP_ANIM_GROUP_STAGGER:
            /* Start animations with incremental delays */
            for (VaxpU32 i = 0; i < group->children_count; i++) {
                VaxpAnimation* anim = group->children[i];
                VaxpF32 original_delay = anim->delay;
                anim->delay = original_delay + group->stagger_delay * (VaxpF32)i;
                vaxp_animation_start(anim);
            }
            break;
    }
}

void vaxp_animation_group_pause(VaxpAnimationGroup* group) {
    VAXP_RETURN_VOID_IF_NULL(group);
    
    if (group->state != VAXP_ANIM_STATE_RUNNING) return;
    
    group->state = VAXP_ANIM_STATE_PAUSED;
    
    for (VaxpU32 i = 0; i < group->children_count; i++) {
        vaxp_animation_pause(group->children[i]);
    }
}

void vaxp_animation_group_resume(VaxpAnimationGroup* group) {
    VAXP_RETURN_VOID_IF_NULL(group);
    
    if (group->state != VAXP_ANIM_STATE_PAUSED) return;
    
    group->state = VAXP_ANIM_STATE_RUNNING;
    
    for (VaxpU32 i = 0; i < group->children_count; i++) {
        vaxp_animation_resume(group->children[i]);
    }
}

void vaxp_animation_group_stop(VaxpAnimationGroup* group) {
    VAXP_RETURN_VOID_IF_NULL(group);
    
    group->state = VAXP_ANIM_STATE_IDLE;
    group->current_index = 0;
    
    for (VaxpU32 i = 0; i < group->children_count; i++) {
        vaxp_animation_stop(group->children[i]);
    }
}

/* ============================================================================
 * GROUP UPDATE
 * ============================================================================ */

VaxpBool vaxp_animation_group_update(VaxpAnimationGroup* group, VaxpF64 delta_time) {
    VAXP_RETURN_IF_NULL(group, VAXP_FALSE);
    
    if (group->state != VAXP_ANIM_STATE_RUNNING) {
        return VAXP_FALSE;
    }
    
    if (group->children_count == 0) {
        group->state = VAXP_ANIM_STATE_COMPLETED;
        return VAXP_FALSE;
    }
    
    VaxpBool any_running = VAXP_FALSE;
    
    switch (group->type) {
        case VAXP_ANIM_GROUP_SEQUENCE: {
            /* Update current animation */
            if (group->current_index < group->children_count) {
                VaxpAnimation* current = group->children[group->current_index];
                VaxpBool running = vaxp_animation_update(current, delta_time);
                
                if (!running) {
                    /* Current animation finished, move to next */
                    group->current_index++;
                    if (group->current_index < group->children_count) {
                        vaxp_animation_start(group->children[group->current_index]);
                        any_running = VAXP_TRUE;
                    }
                } else {
                    any_running = VAXP_TRUE;
                }
            }
            break;
        }
        
        case VAXP_ANIM_GROUP_PARALLEL:
        case VAXP_ANIM_GROUP_STAGGER: {
            /* Update all animations */
            for (VaxpU32 i = 0; i < group->children_count; i++) {
                if (vaxp_animation_update(group->children[i], delta_time)) {
                    any_running = VAXP_TRUE;
                }
            }
            break;
        }
    }
    
    if (!any_running) {
        group->state = VAXP_ANIM_STATE_COMPLETED;
        
        if (group->on_complete) {
            group->on_complete(group, group->callback_data);
        }
    }
    
    return any_running;
}

void vaxp_animation_group_set_on_complete(VaxpAnimationGroup* group, 
                                            VaxpAnimationGroupCallback callback, 
                                            void* user_data) {
    VAXP_RETURN_VOID_IF_NULL(group);
    group->on_complete = callback;
    group->callback_data = user_data;
}
