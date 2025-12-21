/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_event_queue.c - Event queue with coalescing
 */

#include "venom/backend/venom_event_queue.h"
#include "venom/core/venom_memory.h"
#include <string.h>

VenomEventQueue* venom_event_queue_create(VenomU32 capacity) {
    if (capacity == 0) capacity = VENOM_EVENT_QUEUE_DEFAULT_CAPACITY;
    
    VenomEventQueue* queue = (VenomEventQueue*)venom_alloc(sizeof(VenomEventQueue));
    if (!queue) return NULL;
    
    queue->events = (VenomEvent*)venom_alloc(capacity * sizeof(VenomEvent));
    if (!queue->events) {
        venom_free(queue, sizeof(VenomEventQueue));
        return NULL;
    }
    
    queue->capacity = capacity;
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    queue->has_mouse_move = VENOM_FALSE;
    queue->has_mouse_scroll = VENOM_FALSE;
    
    return queue;
}

void venom_event_queue_destroy(VenomEventQueue* queue) {
    if (!queue) return;
    
    if (queue->events) {
        venom_free(queue->events, queue->capacity * sizeof(VenomEvent));
    }
    venom_free(queue, sizeof(VenomEventQueue));
}

void venom_event_queue_push(VenomEventQueue* queue, const VenomEvent* event) {
    if (!queue || !event) return;
    
    /* Coalesce MOUSE_MOVE - only keep the latest */
    if (event->type == VENOM_EVENT_MOUSE_MOVE) {
        queue->last_mouse_move = *event;
        queue->has_mouse_move = VENOM_TRUE;
        return;  /* Don't add to regular queue */
    }
    
    /* Coalesce MOUSE_SCROLL - accumulate deltas */
    if (event->type == VENOM_EVENT_MOUSE_SCROLL) {
        if (queue->has_mouse_scroll) {
            /* Accumulate scroll deltas */
            queue->last_mouse_scroll.scroll.delta_x += event->scroll.delta_x;
            queue->last_mouse_scroll.scroll.delta_y += event->scroll.delta_y;
        } else {
            queue->last_mouse_scroll = *event;
            queue->has_mouse_scroll = VENOM_TRUE;
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

void venom_event_queue_flush_coalesced(VenomEventQueue* queue) {
    if (!queue) return;
    
    /* Push coalesced mouse move to queue */
    if (queue->has_mouse_move) {
        if (queue->count < queue->capacity) {
            queue->events[queue->tail] = queue->last_mouse_move;
            queue->tail = (queue->tail + 1) % queue->capacity;
            queue->count++;
        }
        queue->has_mouse_move = VENOM_FALSE;
    }
    
    /* Push coalesced mouse scroll to queue */
    if (queue->has_mouse_scroll) {
        if (queue->count < queue->capacity) {
            queue->events[queue->tail] = queue->last_mouse_scroll;
            queue->tail = (queue->tail + 1) % queue->capacity;
            queue->count++;
        }
        queue->has_mouse_scroll = VENOM_FALSE;
    }
}

VenomBool venom_event_queue_pop(VenomEventQueue* queue, VenomEvent* out) {
    if (!queue || !out || queue->count == 0) return VENOM_FALSE;
    
    *out = queue->events[queue->head];
    queue->head = (queue->head + 1) % queue->capacity;
    queue->count--;
    
    return VENOM_TRUE;
}

VenomBool venom_event_queue_is_empty(const VenomEventQueue* queue) {
    if (!queue) return VENOM_TRUE;
    return queue->count == 0 && !queue->has_mouse_move && !queue->has_mouse_scroll;
}

void venom_event_queue_clear(VenomEventQueue* queue) {
    if (!queue) return;
    
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    queue->has_mouse_move = VENOM_FALSE;
    queue->has_mouse_scroll = VENOM_FALSE;
}

VenomU32 venom_event_queue_size(const VenomEventQueue* queue) {
    if (!queue) return 0;
    return queue->count + (queue->has_mouse_move ? 1 : 0) + (queue->has_mouse_scroll ? 1 : 0);
}
