/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_event_queue.h - Event queue with coalescing support
 * 
 * Features:
 * - FIFO event queue
 * - MOUSE_MOVE coalescing (multiple → one)
 * - Fixed capacity ring buffer
 */

#ifndef VAXP_EVENT_QUEUE_H
#define VAXP_EVENT_QUEUE_H

#include "vaxp/core/vaxp_types.h"
#include "vaxp/backend/vaxp_event.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VAXP_EVENT_QUEUE_DEFAULT_CAPACITY 256

typedef struct VaxpEventQueue {
    VaxpEvent* events;
    VaxpU32 capacity;
    VaxpU32 head;           /* Read position */
    VaxpU32 tail;           /* Write position */
    VaxpU32 count;
    
    /* Coalesced events - only keep latest */
    VaxpEvent last_mouse_move;
    VaxpBool has_mouse_move;
    
    VaxpEvent last_mouse_scroll;
    VaxpBool has_mouse_scroll;
} VaxpEventQueue;

/**
 * @brief Create event queue with given capacity
 */
VaxpEventQueue* vaxp_event_queue_create(VaxpU32 capacity);

/**
 * @brief Destroy event queue
 */
void vaxp_event_queue_destroy(VaxpEventQueue* queue);

/**
 * @brief Push event to queue (auto-coalesces MOUSE_MOVE/SCROLL)
 */
void vaxp_event_queue_push(VaxpEventQueue* queue, const VaxpEvent* event);

/**
 * @brief Pop event from queue
 * @return VAXP_TRUE if event was popped, VAXP_FALSE if empty
 */
VaxpBool vaxp_event_queue_pop(VaxpEventQueue* queue, VaxpEvent* out);

/**
 * @brief Flush coalesced events into queue (call before processing)
 */
void vaxp_event_queue_flush_coalesced(VaxpEventQueue* queue);

/**
 * @brief Check if queue is empty
 */
VaxpBool vaxp_event_queue_is_empty(const VaxpEventQueue* queue);

/**
 * @brief Clear all events
 */
void vaxp_event_queue_clear(VaxpEventQueue* queue);

/**
 * @brief Get number of events in queue
 */
VaxpU32 vaxp_event_queue_size(const VaxpEventQueue* queue);

#ifdef __cplusplus
}
#endif

#endif /* VAXP_EVENT_QUEUE_H */
