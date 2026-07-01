/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_event_queue.c - Event queue with coalescing
 */

#include "vaxp/backend/vaxp_event_queue.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

VaxpEventQueue* vaxp_event_queue_create(VaxpU32 capacity) {
    if (capacity == 0) capacity = VAXP_EVENT_QUEUE_DEFAULT_CAPACITY;
    
    VaxpEventQueue* queue = (VaxpEventQueue*)vaxp_alloc(sizeof(VaxpEventQueue));
    if (!queue) return NULL;
    
    queue->events = (VaxpEvent*)vaxp_alloc(capacity * sizeof(VaxpEvent));
    if (!queue->events) {
        vaxp_free(queue, sizeof(VaxpEventQueue));
        return NULL;
    }
    
    queue->capacity = capacity;
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    queue->has_mouse_move = VAXP_FALSE;
    queue->has_mouse_scroll = VAXP_FALSE;
    
    return queue;
}

void vaxp_event_queue_destroy(VaxpEventQueue* queue) {
    if (!queue) return;
    
    if (queue->events) {
        vaxp_free(queue->events, queue->capacity * sizeof(VaxpEvent));
    }
    vaxp_free(queue, sizeof(VaxpEventQueue));
}

void vaxp_event_queue_push(VaxpEventQueue* queue, const VaxpEvent* event) {
    if (!queue || !event) return;
    
    /* Coalesce MOUSE_MOVE - only keep the latest */
    if (event->type == VAXP_EVENT_MOUSE_MOVE) {
        queue->last_mouse_move = *event;
        queue->has_mouse_move = VAXP_TRUE;
        return;  /* Don't add to regular queue */
    }
    
    /* Coalesce MOUSE_SCROLL - accumulate deltas */
    if (event->type == VAXP_EVENT_MOUSE_SCROLL) {
        if (queue->has_mouse_scroll) {
            /* Accumulate scroll deltas */
            queue->last_mouse_scroll.scroll.delta_x += event->scroll.delta_x;
            queue->last_mouse_scroll.scroll.delta_y += event->scroll.delta_y;
        } else {
            queue->last_mouse_scroll = *event;
            queue->has_mouse_scroll = VAXP_TRUE;
        }
        return;
    }
    
    /* Regular events go to queue */
    if (queue->count >= queue->capacity) {
        /* Queue full - drop oldest event */
        queue->head = (queue->head + 1) % queue->capacity;
        queue->count--;
    }
    
    queue->events[queue->tail] = *event;
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->count++;
}

void vaxp_event_queue_flush_coalesced(VaxpEventQueue* queue) {
    if (!queue) return;
    
    /* Push coalesced mouse move to queue */
    if (queue->has_mouse_move) {
        if (queue->count < queue->capacity) {
            queue->events[queue->tail] = queue->last_mouse_move;
            queue->tail = (queue->tail + 1) % queue->capacity;
            queue->count++;
        }
        queue->has_mouse_move = VAXP_FALSE;
    }
    
    /* Push coalesced mouse scroll to queue */
    if (queue->has_mouse_scroll) {
        if (queue->count < queue->capacity) {
            queue->events[queue->tail] = queue->last_mouse_scroll;
            queue->tail = (queue->tail + 1) % queue->capacity;
            queue->count++;
        }
        queue->has_mouse_scroll = VAXP_FALSE;
    }
}

VaxpBool vaxp_event_queue_pop(VaxpEventQueue* queue, VaxpEvent* out) {
    if (!queue || !out || queue->count == 0) return VAXP_FALSE;
    
    *out = queue->events[queue->head];
    queue->head = (queue->head + 1) % queue->capacity;
    queue->count--;
    
    return VAXP_TRUE;
}

VaxpBool vaxp_event_queue_is_empty(const VaxpEventQueue* queue) {
    if (!queue) return VAXP_TRUE;
    return queue->count == 0 && !queue->has_mouse_move && !queue->has_mouse_scroll;
}

void vaxp_event_queue_clear(VaxpEventQueue* queue) {
    if (!queue) return;
    
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    queue->has_mouse_move = VAXP_FALSE;
    queue->has_mouse_scroll = VAXP_FALSE;
}

VaxpU32 vaxp_event_queue_size(const VaxpEventQueue* queue) {
    if (!queue) return 0;
    return queue->count + (queue->has_mouse_move ? 1 : 0) + (queue->has_mouse_scroll ? 1 : 0);
}
