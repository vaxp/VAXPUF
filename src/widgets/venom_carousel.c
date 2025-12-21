/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_carousel.c - Image/Widget Carousel implementation
 */

#include "venom/widgets/venom_carousel.h"
#include "venom/core/venom_memory.h"
#include "venom/graphics/venom_canvas.h"
#include <time.h>

/* Dimensions */
#define CAROUSEL_ARROW_SIZE 40.0f
#define CAROUSEL_DOT_SIZE 8.0f
#define CAROUSEL_DOT_GAP 12.0f
#define CAROUSEL_INDICATOR_MARGIN 16.0f
#define CAROUSEL_DRAG_THRESHOLD 50.0f

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

static void carousel_init(VenomWidget* widget);
static void carousel_destroy(VenomWidget* widget);
static void carousel_measure(VenomWidget* widget, VenomF32 aw, VenomF32 ah,
                              VenomF32* ow, VenomF32* oh);
static void carousel_layout(VenomWidget* widget, VenomRectF bounds);
static void carousel_draw(VenomWidget* widget, VenomCanvas* canvas);
static VenomBool carousel_on_event(VenomWidget* widget, const VenomEvent* event);

/* ============================================================================
 * CLASS DEFINITION
 * ============================================================================ */

const VenomWidgetClass venom_carousel_class = {
    .class_name = "VenomCarousel",
    .instance_size = sizeof(VenomCarousel),
    .parent_class = &venom_widget_class,
    .init = carousel_init,
    .destroy = carousel_destroy,
    .measure = carousel_measure,
    .layout = carousel_layout,
    .draw = carousel_draw,
    .on_event = carousel_on_event,
    .on_state_changed = NULL,
};

/* ============================================================================
 * HELPERS
 * ============================================================================ */

static VenomF64 get_current_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

/* ============================================================================
 * LIFECYCLE
 * ============================================================================ */

static void carousel_init(VenomWidget* widget) {
    VenomCarousel* c = (VenomCarousel*)widget;
    
    c->items = NULL;
    c->item_count = 0;
    c->current_index = 0;
    c->indicator_active = venom_color_rgb(60, 120, 220);
    c->indicator_inactive = venom_color_rgba(255, 255, 255, 150);
    c->indicator_style = VENOM_CAROUSEL_DOTS;
    c->show_arrows = VENOM_TRUE;
    c->offset_x = 0;
    c->target_offset = 0;
    c->drag_start_x = 0;
    c->is_dragging = VENOM_FALSE;
    c->auto_play = VENOM_FALSE;
    c->interval_ms = 3000;
    c->last_slide_time = 0;
    c->on_change = NULL;
    c->user_data = NULL;
}

static void carousel_destroy(VenomWidget* widget) {
    VenomCarousel* c = (VenomCarousel*)widget;
    
    for (VenomU32 i = 0; i < c->item_count; i++) {
        venom_unref(c->items[i]);
    }
    if (c->items) {
        venom_free(c->items, c->item_count * sizeof(VenomWidget*));
        c->items = NULL;
    }
    c->item_count = 0;
    
    venom_widget_class.destroy(widget);
}

/* ============================================================================
 * LAYOUT
 * ============================================================================ */

static void carousel_measure(VenomWidget* widget, VenomF32 aw, VenomF32 ah,
                              VenomF32* ow, VenomF32* oh) {
    (void)widget;
    *ow = aw;
    *oh = ah > 0 ? ah : 200;
}

static void carousel_layout(VenomWidget* widget, VenomRectF bounds) {
    VenomCarousel* c = (VenomCarousel*)widget;
    widget->bounds = bounds;
    
    /* Layout items at full width */
    for (VenomU32 i = 0; i < c->item_count; i++) {
        VenomRectF item_bounds = {0, 0, bounds.width, bounds.height - 40};
        venom_widget_layout(c->items[i], item_bounds);
    }
}

/* ============================================================================
 * DRAWING
 * ============================================================================ */

static void carousel_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomCarousel* c = (VenomCarousel*)widget;
    VenomRectF bounds = widget->bounds;
    
    if (c->item_count == 0) return;
    
    /* Auto-play */
    if (c->auto_play && !c->is_dragging) {
        VenomF64 now = get_current_time_ms();
        if (now - c->last_slide_time >= c->interval_ms) {
            venom_carousel_next(c);
            c->last_slide_time = now;
        }
        widget->needs_redraw = VENOM_TRUE;
    }
    
    /* Animate slide */
    c->target_offset = -c->current_index * bounds.width;
    c->offset_x += (c->target_offset - c->offset_x) * 0.15f;
    
    /* Draw visible items */
    for (VenomI32 i = -1; i <= (VenomI32)c->item_count; i++) {
        VenomI32 idx = i;
        if (idx < 0) idx = c->item_count - 1;
        if (idx >= (VenomI32)c->item_count) idx = 0;
        
        VenomF32 item_x = bounds.x + c->offset_x + i * bounds.width;
        
        /* Skip if not visible */
        if (item_x + bounds.width < bounds.x || item_x > bounds.x + bounds.width) {
            continue;
        }
        
        /* Save and translate */
        VenomRectF item_area = {item_x, bounds.y, bounds.width, bounds.height - 40};
        
        /* Clip and draw */
        if (idx >= 0 && idx < (VenomI32)c->item_count) {
            venom_widget_draw(c->items[idx], canvas);
        }
    }
    
    /* Draw indicators */
    if (c->indicator_style == VENOM_CAROUSEL_DOTS && c->item_count > 1) {
        VenomF32 total_width = c->item_count * CAROUSEL_DOT_SIZE + 
                               (c->item_count - 1) * CAROUSEL_DOT_GAP;
        VenomF32 start_x = bounds.x + (bounds.width - total_width) / 2.0f;
        VenomF32 dot_y = bounds.y + bounds.height - CAROUSEL_INDICATOR_MARGIN;
        
        for (VenomU32 i = 0; i < c->item_count; i++) {
            VenomBool is_current = ((VenomI32)i == c->current_index);
            VenomColor dot_color = is_current ? c->indicator_active : c->indicator_inactive;
            VenomF32 radius = is_current ? CAROUSEL_DOT_SIZE / 2 + 1 : CAROUSEL_DOT_SIZE / 2;
            
            VenomPaint dp = venom_paint_fill(dot_color);
            venom_canvas_draw_circle(canvas, 
                start_x + i * (CAROUSEL_DOT_SIZE + CAROUSEL_DOT_GAP) + CAROUSEL_DOT_SIZE / 2,
                dot_y, radius, &dp);
        }
    } else if (c->indicator_style == VENOM_CAROUSEL_NUMBERS && c->item_count > 1) {
        char num_str[16];
        snprintf(num_str, sizeof(num_str), "%d / %u", c->current_index + 1, c->item_count);
        
        VenomF32 text_y = bounds.y + bounds.height - CAROUSEL_INDICATOR_MARGIN + 5;
        VenomPaint tp = venom_paint_fill(c->indicator_active);
        venom_canvas_draw_text(canvas, num_str, bounds.x + bounds.width / 2, text_y, NULL, &tp);
    }
    
    /* Draw arrows */
    if (c->show_arrows && c->item_count > 1) {
        VenomF32 arrow_y = bounds.y + (bounds.height - 40) / 2;
        VenomColor arrow_bg = venom_color_rgba(0, 0, 0, 80);
        VenomPaint abp = venom_paint_fill(arrow_bg);
        
        /* Left arrow */
        VenomRectF left_arrow = {
            bounds.x + 8, arrow_y - CAROUSEL_ARROW_SIZE / 2,
            CAROUSEL_ARROW_SIZE, CAROUSEL_ARROW_SIZE
        };
        venom_canvas_draw_rounded_rect(canvas, left_arrow, CAROUSEL_ARROW_SIZE / 2, &abp);
        
        /* Right arrow */
        VenomRectF right_arrow = {
            bounds.x + bounds.width - CAROUSEL_ARROW_SIZE - 8,
            arrow_y - CAROUSEL_ARROW_SIZE / 2,
            CAROUSEL_ARROW_SIZE, CAROUSEL_ARROW_SIZE
        };
        venom_canvas_draw_rounded_rect(canvas, right_arrow, CAROUSEL_ARROW_SIZE / 2, &abp);
        
        /* Draw arrow symbols */
        VenomPaint awp = venom_paint_fill(VENOM_COLOR_WHITE);
        venom_canvas_draw_text(canvas, "<", 
            left_arrow.x + CAROUSEL_ARROW_SIZE / 2, arrow_y + 6, NULL, &awp);
        venom_canvas_draw_text(canvas, ">",
            right_arrow.x + CAROUSEL_ARROW_SIZE / 2, arrow_y + 6, NULL, &awp);
    }
    
    /* Request redraw for animation */
    if (c->offset_x != c->target_offset || c->auto_play) {
        widget->needs_redraw = VENOM_TRUE;
    }
}

/* ============================================================================
 * EVENTS
 * ============================================================================ */

static VenomBool carousel_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomCarousel* c = (VenomCarousel*)widget;
    VenomRectF bounds = widget->bounds;
    
    if (event->type == VENOM_EVENT_MOUSE_BUTTON_DOWN && c->item_count > 1) {
        VenomF32 mx = event->mouse.x;
        VenomF32 my = event->mouse.y;
        
        /* Check arrow clicks */
        if (c->show_arrows) {
            VenomF32 arrow_y = bounds.y + (bounds.height - 40) / 2;
            
            /* Left arrow */
            if (mx >= bounds.x + 8 && mx <= bounds.x + 8 + CAROUSEL_ARROW_SIZE &&
                my >= arrow_y - CAROUSEL_ARROW_SIZE / 2 && 
                my <= arrow_y + CAROUSEL_ARROW_SIZE / 2) {
                venom_carousel_prev(c);
                return VENOM_TRUE;
            }
            
            /* Right arrow */
            if (mx >= bounds.x + bounds.width - CAROUSEL_ARROW_SIZE - 8 &&
                mx <= bounds.x + bounds.width - 8 &&
                my >= arrow_y - CAROUSEL_ARROW_SIZE / 2 &&
                my <= arrow_y + CAROUSEL_ARROW_SIZE / 2) {
                venom_carousel_next(c);
                return VENOM_TRUE;
            }
        }
        
        /* Start drag */
        c->is_dragging = VENOM_TRUE;
        c->drag_start_x = mx;
        return VENOM_TRUE;
    }
    
    if (event->type == VENOM_EVENT_MOUSE_MOVE && c->is_dragging) {
        VenomF32 delta = event->mouse.x - c->drag_start_x;
        c->offset_x = c->target_offset + delta;
        venom_widget_invalidate(widget);
        return VENOM_TRUE;
    }
    
    if (event->type == VENOM_EVENT_MOUSE_BUTTON_UP && c->is_dragging) {
        c->is_dragging = VENOM_FALSE;
        
        VenomF32 delta = event->mouse.x - c->drag_start_x;
        if (delta < -CAROUSEL_DRAG_THRESHOLD && c->current_index < (VenomI32)c->item_count - 1) {
            venom_carousel_next(c);
        } else if (delta > CAROUSEL_DRAG_THRESHOLD && c->current_index > 0) {
            venom_carousel_prev(c);
        }
        
        venom_widget_invalidate(widget);
        return VENOM_TRUE;
    }
    
    return VENOM_FALSE;
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VenomResultPtr venom_carousel_create(void) {
    VenomResultPtr result = venom_widget_create(&venom_carousel_class);
    if (result.ok) {
        VenomCarousel* c = (VenomCarousel*)result.value;
        c->last_slide_time = get_current_time_ms();
    }
    return result;
}

void venom_carousel_add_item(VenomCarousel* carousel, VenomWidget* item) {
    if (!carousel || !item) return;
    
    VenomU32 new_count = carousel->item_count + 1;
    VenomWidget** new_items = venom_realloc(carousel->items,
        carousel->item_count * sizeof(VenomWidget*),
        new_count * sizeof(VenomWidget*));
    
    if (!new_items) return;
    
    carousel->items = new_items;
    carousel->items[carousel->item_count] = venom_ref(item);
    carousel->item_count = new_count;
    
    venom_widget_invalidate_layout((VenomWidget*)carousel);
}

void venom_carousel_clear(VenomCarousel* carousel) {
    if (!carousel) return;
    
    for (VenomU32 i = 0; i < carousel->item_count; i++) {
        venom_unref(carousel->items[i]);
    }
    if (carousel->items) {
        venom_free(carousel->items, carousel->item_count * sizeof(VenomWidget*));
        carousel->items = NULL;
    }
    carousel->item_count = 0;
    carousel->current_index = 0;
    
    venom_widget_invalidate((VenomWidget*)carousel);
}

void venom_carousel_goto(VenomCarousel* carousel, VenomI32 index) {
    if (!carousel || carousel->item_count == 0) return;
    
    if (index < 0) index = 0;
    if (index >= (VenomI32)carousel->item_count) index = carousel->item_count - 1;
    
    if (carousel->current_index != index) {
        carousel->current_index = index;
        
        if (carousel->on_change) {
            carousel->on_change(carousel, index, carousel->user_data);
        }
        
        venom_widget_invalidate((VenomWidget*)carousel);
    }
}

void venom_carousel_next(VenomCarousel* carousel) {
    if (!carousel || carousel->item_count == 0) return;
    
    VenomI32 next = carousel->current_index + 1;
    if (next >= (VenomI32)carousel->item_count) next = 0;  /* Loop */
    
    venom_carousel_goto(carousel, next);
}

void venom_carousel_prev(VenomCarousel* carousel) {
    if (!carousel || carousel->item_count == 0) return;
    
    VenomI32 prev = carousel->current_index - 1;
    if (prev < 0) prev = carousel->item_count - 1;  /* Loop */
    
    venom_carousel_goto(carousel, prev);
}

void venom_carousel_set_auto_play(VenomCarousel* carousel, VenomBool enabled, VenomU32 interval_ms) {
    if (!carousel) return;
    carousel->auto_play = enabled;
    carousel->interval_ms = interval_ms > 0 ? interval_ms : 3000;
    carousel->last_slide_time = get_current_time_ms();
    venom_widget_invalidate((VenomWidget*)carousel);
}

void venom_carousel_set_indicator(VenomCarousel* carousel, VenomCarouselIndicator style) {
    if (!carousel) return;
    carousel->indicator_style = style;
    venom_widget_invalidate((VenomWidget*)carousel);
}

void venom_carousel_set_show_arrows(VenomCarousel* carousel, VenomBool show) {
    if (!carousel) return;
    carousel->show_arrows = show;
    venom_widget_invalidate((VenomWidget*)carousel);
}

void venom_carousel_set_indicator_colors(VenomCarousel* carousel,
                                          VenomColor active,
                                          VenomColor inactive) {
    if (!carousel) return;
    carousel->indicator_active = active;
    carousel->indicator_inactive = inactive;
    venom_widget_invalidate((VenomWidget*)carousel);
}

void venom_carousel_set_on_change(VenomCarousel* carousel,
                                   void (*callback)(VenomCarousel*, VenomI32, void*),
                                   void* user_data) {
    if (!carousel) return;
    carousel->on_change = callback;
    carousel->user_data = user_data;
}

VenomI32 venom_carousel_get_current(VenomCarousel* carousel) {
    return carousel ? carousel->current_index : -1;
}
