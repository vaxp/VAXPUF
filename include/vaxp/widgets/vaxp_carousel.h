/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_carousel.h - Image/Widget Carousel (Slider)
 */

#ifndef VAXP_CAROUSEL_H
#define VAXP_CAROUSEL_H

#include "vaxp_widget.h"
#include "../core/vaxp_types.h"

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
    VAXP_CAROUSEL_DOTS,        /* Dot indicators */
    VAXP_CAROUSEL_NUMBERS,     /* 1/5 style */
    VAXP_CAROUSEL_NONE         /* No indicators */
} VaxpCarouselIndicator;

/**
 * @brief Carousel structure
 */
typedef struct VaxpCarousel {
    VaxpWidget base;
    
    /* Content */
    VaxpWidget** items;
    VaxpU32 item_count;
    VaxpI32 current_index;
    
    /* Appearance */
    VaxpColor indicator_active;
    VaxpColor indicator_inactive;
    VaxpCarouselIndicator indicator_style;
    VaxpBool show_arrows;
    
    /* Animation */
    VaxpF32 offset_x;
    VaxpF32 target_offset;
    VaxpF32 drag_start_x;
    VaxpBool is_dragging;
    
    /* Auto-play */
    VaxpBool auto_play;
    VaxpU32 interval_ms;
    VaxpF64 last_slide_time;
    
    /* Callbacks */
    void (*on_change)(struct VaxpCarousel* carousel, VaxpI32 index, void* user_data);
    void* user_data;
} VaxpCarousel;

/* ============================================================================
 * CREATION
 * ============================================================================ */

/**
 * @brief Create a carousel
 */
VAXP_WARN_UNUSED
VaxpResultPtr vaxp_carousel_create(void);

/* ============================================================================
 * CONTENT
 * ============================================================================ */

/**
 * @brief Add item to carousel
 */
void vaxp_carousel_add_item(VaxpCarousel* carousel, VaxpWidget* item);

/**
 * @brief Remove all items
 */
void vaxp_carousel_clear(VaxpCarousel* carousel);

/**
 * @brief Go to specific slide
 */
void vaxp_carousel_goto(VaxpCarousel* carousel, VaxpI32 index);

/**
 * @brief Go to next slide
 */
void vaxp_carousel_next(VaxpCarousel* carousel);

/**
 * @brief Go to previous slide
 */
void vaxp_carousel_prev(VaxpCarousel* carousel);

/* ============================================================================
 * CONFIGURATION
 * ============================================================================ */

/**
 * @brief Enable/disable auto-play
 */
void vaxp_carousel_set_auto_play(VaxpCarousel* carousel, VaxpBool enabled, VaxpU32 interval_ms);

/**
 * @brief Set indicator style
 */
void vaxp_carousel_set_indicator(VaxpCarousel* carousel, VaxpCarouselIndicator style);

/**
 * @brief Show/hide navigation arrows
 */
void vaxp_carousel_set_show_arrows(VaxpCarousel* carousel, VaxpBool show);

/**
 * @brief Set indicator colors
 */
void vaxp_carousel_set_indicator_colors(VaxpCarousel* carousel,
                                          VaxpColor active,
                                          VaxpColor inactive);

/**
 * @brief Set change callback
 */
void vaxp_carousel_set_on_change(VaxpCarousel* carousel,
                                   void (*callback)(VaxpCarousel*, VaxpI32, void*),
                                   void* user_data);

/**
 * @brief Get current index
 */
VaxpI32 vaxp_carousel_get_current(VaxpCarousel* carousel);

/* ============================================================================
 * CLASS
 * ============================================================================ */

extern const VaxpWidgetClass vaxp_carousel_class;

#ifdef __cplusplus
}
#endif

#endif /* VAXP_CAROUSEL_H */
