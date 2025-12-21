/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_carousel.h - Image/Widget Carousel (Slider)
 */

#ifndef VENOM_CAROUSEL_H
#define VENOM_CAROUSEL_H

#include "venom_widget.h"
#include "../core/venom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * TYPES
 * ============================================================================ */

/**
 * @brief Carousel indicator style
 */
typedef enum {
    VENOM_CAROUSEL_DOTS,        /* Dot indicators */
    VENOM_CAROUSEL_NUMBERS,     /* 1/5 style */
    VENOM_CAROUSEL_NONE         /* No indicators */
} VenomCarouselIndicator;

/**
 * @brief Carousel structure
 */
typedef struct VenomCarousel {
    VenomWidget base;
    
    /* Content */
    VenomWidget** items;
    VenomU32 item_count;
    VenomI32 current_index;
    
    /* Appearance */
    VenomColor indicator_active;
    VenomColor indicator_inactive;
    VenomCarouselIndicator indicator_style;
    VenomBool show_arrows;
    
    /* Animation */
    VenomF32 offset_x;
    VenomF32 target_offset;
    VenomF32 drag_start_x;
    VenomBool is_dragging;
    
    /* Auto-play */
    VenomBool auto_play;
    VenomU32 interval_ms;
    VenomF64 last_slide_time;
    
    /* Callbacks */
    void (*on_change)(struct VenomCarousel* carousel, VenomI32 index, void* user_data);
    void* user_data;
} VenomCarousel;

/* ============================================================================
 * CREATION
 * ============================================================================ */

/**
 * @brief Create a carousel
 */
VENOM_WARN_UNUSED
VenomResultPtr venom_carousel_create(void);

/* ============================================================================
 * CONTENT
 * ============================================================================ */

/**
 * @brief Add item to carousel
 */
void venom_carousel_add_item(VenomCarousel* carousel, VenomWidget* item);

/**
 * @brief Remove all items
 */
void venom_carousel_clear(VenomCarousel* carousel);

/**
 * @brief Go to specific slide
 */
void venom_carousel_goto(VenomCarousel* carousel, VenomI32 index);

/**
 * @brief Go to next slide
 */
void venom_carousel_next(VenomCarousel* carousel);

/**
 * @brief Go to previous slide
 */
void venom_carousel_prev(VenomCarousel* carousel);

/* ============================================================================
 * CONFIGURATION
 * ============================================================================ */

/**
 * @brief Enable/disable auto-play
 */
void venom_carousel_set_auto_play(VenomCarousel* carousel, VenomBool enabled, VenomU32 interval_ms);

/**
 * @brief Set indicator style
 */
void venom_carousel_set_indicator(VenomCarousel* carousel, VenomCarouselIndicator style);

/**
 * @brief Show/hide navigation arrows
 */
void venom_carousel_set_show_arrows(VenomCarousel* carousel, VenomBool show);

/**
 * @brief Set indicator colors
 */
void venom_carousel_set_indicator_colors(VenomCarousel* carousel,
                                          VenomColor active,
                                          VenomColor inactive);

/**
 * @brief Set change callback
 */
void venom_carousel_set_on_change(VenomCarousel* carousel,
                                   void (*callback)(VenomCarousel*, VenomI32, void*),
                                   void* user_data);

/**
 * @brief Get current index
 */
VenomI32 venom_carousel_get_current(VenomCarousel* carousel);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VenomWidgetClass venom_carousel_class;

#ifdef __cplusplus
}
#endif

#endif /* VENOM_CAROUSEL_H */
