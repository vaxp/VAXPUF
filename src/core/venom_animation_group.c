/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_animation_group.c - Animation groups for sequence and parallel animations
 */

#include "venom/core/venom_animation_group.h"
#include "venom/core/venom_memory.h"

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

#define INITIAL_CHILDREN_CAPACITY 8

/* ============================================================================
 * GROUP DESTRUCTOR
 * ============================================================================ */

static void animation_group_destructor(void* self) {
    VenomAnimationGroup* group = (VenomAnimationGroup*)self;
    
    /* Unref all children */
    for (VenomU32 i = 0; i < group->children_count; i++) {
        if (group->children[i]) {
            venom_unref(group->children[i]);
            group->children[i] = NULL;
        }
    }
    
    /* Free children array */
    if (group->children) {
        venom_free(group->children, sizeof(VenomAnimation*) * group->children_capacity);
        group->children = NULL;
        group->children_count = 0;
        group->children_capacity = 0;
    }
}

/* ============================================================================
 * GROUP CREATION
 * ============================================================================ */

VenomResultPtr venom_animation_group_create(VenomAnimationGroupType type) {
    VenomAnimationGroup* group = (VenomAnimationGroup*)venom_alloc_zeroed(sizeof(VenomAnimationGroup));
    if (!group) {
        return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
    }
    
    venom_ref_init(group, sizeof(VenomAnimationGroup), animation_group_destructor, "VenomAnimationGroup");
    
    group->type = type;
    group->state = VENOM_ANIM_STATE_IDLE;
    group->stagger_delay = 0.1f;  /* Default stagger delay */
    
    /* Allocate initial children array */
    group->children = (VenomAnimation**)venom_alloc_zeroed(sizeof(VenomAnimation*) * INITIAL_CHILDREN_CAPACITY);
    if (!group->children) {
        venom_free(group, sizeof(VenomAnimationGroup));
        return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
    }
    group->children_capacity = INITIAL_CHILDREN_CAPACITY;
    
    return VENOM_OK_PTR(group);
}

/* ============================================================================
 * CHILD MANAGEMENT
 * ============================================================================ */

VenomResult venom_animation_group_add(VenomAnimationGroup* group, VenomAnimation* anim) {
    VENOM_ENSURE_NOT_NULL(group);
    VENOM_ENSURE_NOT_NULL(anim);
    
    /* Grow array if needed */
    if (group->children_count >= group->children_capacity) {
        VenomU32 new_cap = group->children_capacity * 2;
        VenomAnimation** new_children = VENOM_REALLOC_ARRAY(VenomAnimation*, group->children,
                                                             group->children_capacity, new_cap);
        if (!new_children) {
            return VENOM_ERR_UNIT(VENOM_ERROR_OUT_OF_MEMORY);
        }
        group->children = new_children;
        group->children_capacity = new_cap;
    }
    
    /* Add and ref the child */
    group->children[group->children_count++] = (VenomAnimation*)venom_ref(anim);
    
    return VENOM_OK_UNIT();
}

VenomResult venom_animation_group_remove(VenomAnimationGroup* group, VenomAnimation* anim) {
    VENOM_ENSURE_NOT_NULL(group);
    VENOM_ENSURE_NOT_NULL(anim);
    
    /* Find the animation */
    for (VenomU32 i = 0; i < group->children_count; i++) {
        if (group->children[i] == anim) {
            /* Unref the animation */
            venom_unref(anim);
            
            /* Shift remaining animations */
            for (VenomU32 j = i; j < group->children_count - 1; j++) {
                group->children[j] = group->children[j + 1];
            }
            group->children_count--;
            group->children[group->children_count] = NULL;
            
            return VENOM_OK_UNIT();
        }
    }
    
    return VENOM_ERR_UNIT(VENOM_ERROR_NOT_FOUND);
}

void venom_animation_group_clear(VenomAnimationGroup* group) {
    VENOM_RETURN_VOID_IF_NULL(group);
    
    for (VenomU32 i = 0; i < group->children_count; i++) {
        if (group->children[i]) {
            venom_unref(group->children[i]);
            group->children[i] = NULL;
        }
    }
    group->children_count = 0;
}

void venom_animation_group_set_stagger_delay(VenomAnimationGroup* group, VenomF32 delay) {
    VENOM_RETURN_VOID_IF_NULL(group);
    group->stagger_delay = delay >= 0.0f ? delay : 0.0f;
}

/* ============================================================================
 * GROUP PLAYBACK CONTROL
 * ============================================================================ */

void venom_animation_group_start(VenomAnimationGroup* group) {
    VENOM_RETURN_VOID_IF_NULL(group);
    
    group->state = VENOM_ANIM_STATE_RUNNING;
    group->current_index = 0;
    
    switch (group->type) {
        case VENOM_ANIM_GROUP_SEQUENCE:
            /* Start only the first animation */
            if (group->children_count > 0) {
                venom_animation_start(group->children[0]);
            }
            break;
            
        case VENOM_ANIM_GROUP_PARALLEL:
            /* Start all animations immediately */
            for (VenomU32 i = 0; i < group->children_count; i++) {
                venom_animation_start(group->children[i]);
            }
            break;
            
        case VENOM_ANIM_GROUP_STAGGER:
            /* Start animations with incremental delays */
            for (VenomU32 i = 0; i < group->children_count; i++) {
                VenomAnimation* anim = group->children[i];
                VenomF32 original_delay = anim->delay;
                anim->delay = original_delay + group->stagger_delay * (VenomF32)i;
                venom_animation_start(anim);
            }
            break;
    }
}

void venom_animation_group_pause(VenomAnimationGroup* group) {
    VENOM_RETURN_VOID_IF_NULL(group);
    
    if (group->state != VENOM_ANIM_STATE_RUNNING) return;
    
    group->state = VENOM_ANIM_STATE_PAUSED;
    
    for (VenomU32 i = 0; i < group->children_count; i++) {
        venom_animation_pause(group->children[i]);
    }
}

void venom_animation_group_resume(VenomAnimationGroup* group) {
    VENOM_RETURN_VOID_IF_NULL(group);
    
    if (group->state != VENOM_ANIM_STATE_PAUSED) return;
    
    group->state = VENOM_ANIM_STATE_RUNNING;
    
    for (VenomU32 i = 0; i < group->children_count; i++) {
        venom_animation_resume(group->children[i]);
    }
}

void venom_animation_group_stop(VenomAnimationGroup* group) {
    VENOM_RETURN_VOID_IF_NULL(group);
    
    group->state = VENOM_ANIM_STATE_IDLE;
    group->current_index = 0;
    
    for (VenomU32 i = 0; i < group->children_count; i++) {
        venom_animation_stop(group->children[i]);
    }
}

/* ============================================================================
 * GROUP UPDATE
 * ============================================================================ */

VenomBool venom_animation_group_update(VenomAnimationGroup* group, VenomF64 delta_time) {
    VENOM_RETURN_IF_NULL(group, VENOM_FALSE);
    
    if (group->state != VENOM_ANIM_STATE_RUNNING) {
        return VENOM_FALSE;
    }
    
    if (group->children_count == 0) {
        group->state = VENOM_ANIM_STATE_COMPLETED;
        return VENOM_FALSE;
    }
    
    VenomBool any_running = VENOM_FALSE;
    
    switch (group->type) {
        case VENOM_ANIM_GROUP_SEQUENCE: {
            /* Update current animation */
            if (group->current_index < group->children_count) {
                VenomAnimation* current = group->children[group->current_index];
                VenomBool running = venom_animation_update(current, delta_time);
                
                if (!running) {
                    /* Current animation finished, move to next */
                    group->current_index++;
                    if (group->current_index < group->children_count) {
                        venom_animation_start(group->children[group->current_index]);
                        any_running = VENOM_TRUE;
                    }
                } else {
                    any_running = VENOM_TRUE;
                }
            }
            break;
        }
        
        case VENOM_ANIM_GROUP_PARALLEL:
        case VENOM_ANIM_GROUP_STAGGER: {
            /* Update all animations */
            for (VenomU32 i = 0; i < group->children_count; i++) {
                if (venom_animation_update(group->children[i], delta_time)) {
                    any_running = VENOM_TRUE;
                }
            }
            break;
        }
    }
    
    if (!any_running) {
        group->state = VENOM_ANIM_STATE_COMPLETED;
        
        if (group->on_complete) {
            group->on_complete(group, group->callback_data);
        }
    }
    
    return any_running;
}

void venom_animation_group_set_on_complete(VenomAnimationGroup* group, 
                                            VenomAnimationGroupCallback callback, 
                                            void* user_data) {
    VENOM_RETURN_VOID_IF_NULL(group);
    group->on_complete = callback;
    group->callback_data = user_data;
}
