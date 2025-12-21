/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_event_queue.h - Event queue with coalescing support
 * 
 * Features:
 * - FIFO event queue
 * - MOUSE_MOVE coalescing (multiple → one)
 * - Fixed capacity ring buffer
 */

#ifndef VENOM_EVENT_QUEUE_H
#define VENOM_EVENT_QUEUE_H

#include "venom/core/venom_types.h"
#include "venom/backend/venom_event.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VENOM_EVENT_QUEUE_DEFAULT_CAPACITY 256

typedef struct VenomEventQueue {
    VenomEvent* events;
    VenomU32 capacity;
    VenomU32 head;           /* Read position */
    VenomU32 tail;           /* Write position */
    VenomU32 count;
    
    /* Coalesced events - only keep latest */
    VenomEvent last_mouse_move;
    VenomBool has_mouse_move;
    
    VenomEvent last_mouse_scroll;
    VenomBool has_mouse_scroll;
} VenomEventQueue;

/**
 * @brief Create event queue with given capacity
 */
VenomEventQueue* venom_event_queue_create(VenomU32 capacity);

/**
 * @brief Destroy event queue
 */
void venom_event_queue_destroy(VenomEventQueue* queue);

/**
 * @brief Push event to queue (auto-coalesces MOUSE_MOVE/SCROLL)
 */
void venom_event_queue_push(VenomEventQueue* queue, const VenomEvent* event);

/**
 * @brief Pop event from queue
 * @return VENOM_TRUE if event was popped, VENOM_FALSE if empty
 */
VenomBool venom_event_queue_pop(VenomEventQueue* queue, VenomEvent* out);

/**
 * @brief Flush coalesced events into queue (call before processing)
 */
void venom_event_queue_flush_coalesced(VenomEventQueue* queue);

/**
 * @brief Check if queue is empty
 */
VenomBool venom_event_queue_is_empty(const VenomEventQueue* queue);

/**
 * @brief Clear all events
 */
void venom_event_queue_clear(VenomEventQueue* queue);

/**
 * @brief Get number of events in queue
 */
VenomU32 venom_event_queue_size(const VenomEventQueue* queue);

#ifdef __cplusplus
}
#endif

#endif /* VENOM_EVENT_QUEUE_H */
