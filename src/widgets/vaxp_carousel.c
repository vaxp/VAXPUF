/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_carousel.c - Image/Widget Carousel implementation
 */

#include "vaxp/widgets/vaxp_carousel.h"
#include "vaxp/core/vaxp_memory.h"
#include "vaxp/graphics/vaxp_canvas.h"
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

static void carousel_init(VaxpWidget* widget);
static void carousel_destroy(VaxpWidget* widget);
static void carousel_measure(VaxpWidget* widget, VaxpF32 aw, VaxpF32 ah,
                              VaxpF32* ow, VaxpF32* oh);
static void carousel_layout(VaxpWidget* widget, VaxpRectF bounds);
static void carousel_draw(VaxpWidget* widget, VaxpCanvas* canvas);
static VaxpBool carousel_on_event(VaxpWidget* widget, const VaxpEvent* event);

/* ============================================================================
 * CLASS DEFINITION
 * ============================================================================ */

const VaxpWidgetClass vaxp_carousel_class = {
    .class_name = "VaxpCarousel",
    .instance_size = sizeof(VaxpCarousel),
    .parent_class = &vaxp_widget_class,
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

static VaxpF64 get_current_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

/* ============================================================================
 * LIFECYCLE
 * ============================================================================ */

static void carousel_init(VaxpWidget* widget) {
    VaxpCarousel* c = (VaxpCarousel*)widget;
    
    c->items = NULL;
    c->item_count = 0;
    c->current_index = 0;
    c->indicator_active = vaxp_color_rgb(60, 120, 220);
    c->indicator_inactive = vaxp_color_rgba(255, 255, 255, 150);
    c->indicator_style = VAXP_CAROUSEL_DOTS;
    c->show_arrows = VAXP_TRUE;
    c->offset_x = 0;
    c->target_offset = 0;
    c->drag_start_x = 0;
    c->is_dragging = VAXP_FALSE;
    c->auto_play = VAXP_FALSE;
    c->interval_ms = 3000;
    c->last_slide_time = 0;
    c->on_change = NULL;
    c->user_data = NULL;
}

static void carousel_destroy(VaxpWidget* widget) {
    VaxpCarousel* c = (VaxpCarousel*)widget;
    
    for (VaxpU32 i = 0; i < c->item_count; i++) {
        vaxp_unref(c->items[i]);
    }
    if (c->items) {
        vaxp_free(c->items, c->item_count * sizeof(VaxpWidget*));
        c->items = NULL;
    }
    c->item_count = 0;
    
    vaxp_widget_class.destroy(widget);
}

/* ============================================================================
 * LAYOUT
 * ============================================================================ */

static void carousel_measure(VaxpWidget* widget, VaxpF32 aw, VaxpF32 ah,
                              VaxpF32* ow, VaxpF32* oh) {
    (void)widget;
    *ow = aw;
    *oh = ah > 0 ? ah : 200;
}

static void carousel_layout(VaxpWidget* widget, VaxpRectF bounds) {
    VaxpCarousel* c = (VaxpCarousel*)widget;
    widget->bounds = bounds;
    
    /* Layout items at full width */
    for (VaxpU32 i = 0; i < c->item_count; i++) {
        VaxpRectF item_bounds = {0, 0, bounds.width, bounds.height - 40};
        vaxp_widget_layout(c->items[i], item_bounds);
    }
}

/* ============================================================================
 * DRAWING
 * ============================================================================ */

static void carousel_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpCarousel* carousel = (VaxpCarousel*)widget;
    VaxpRectF bounds = { 0, 0, widget->bounds.width, widget->bounds.height };
    
    if (carousel->item_count == 0) return;
    
    /* Auto-play */
    if (carousel->auto_play && !carousel->is_dragging) {
        VaxpF64 now = get_current_time_ms();
        if (now - carousel->last_slide_time >= carousel->interval_ms) {
            vaxp_carousel_next(carousel);
            carousel->last_slide_time = now;
        }
        widget->needs_redraw = VAXP_TRUE;
    }
    
    /* Animate slide */
    carousel->target_offset = -carousel->current_index * bounds.width;
    carousel->offset_x += (carousel->target_offset - carousel->offset_x) * 0.15f;
    
    /* Draw visible items */
    for (VaxpI32 i = -1; i <= (VaxpI32)carousel->item_count; i++) {
        VaxpI32 idx = i;
        if (idx < 0) idx = carousel->item_count - 1;
        if (idx >= (VaxpI32)carousel->item_count) idx = 0;
        
        VaxpF32 item_x = bounds.x + carousel->offset_x + i * bounds.width;
        
        /* Skip if not visible */
        if (item_x + bounds.width < bounds.x || item_x > bounds.x + bounds.width) {
            continue;
        }
        
        /* Save and translate */
        VaxpRectF item_area = {item_x, bounds.y, bounds.width, bounds.height - 40};
        
        /* Clip and draw (manual clip not needed if we rely on widget system, but here we draw manually) */
        /* Translation handles position, just draw item */
        /* Actually items are widgets, we need to manually draw them at offset position? */
        /* Normally widgets draw at their bounds. We need to force draw at new position */
        /* But vaxp_widget_draw translates by child->bounds. */
        /* We should temporarily move child */
        
        /* simplified: assumes child fills area */
        /* Wait, vaxp_widget_draw translates by child->bounds.x/y. */
        /* We want to draw at item_x. */
        /* Let's manually translate canvas and call child->draw directly? No, messy. */
        
        /* For now, just fix the compilation error using `carousel` variable */
        
        /* But wait, `c` was used below too */
        if (idx >= 0 && idx < (VaxpI32)carousel->item_count) {
             /* Drawing child widgets in a carousel is complex because of offsets. */
             /* We need to cheat: modify child pos, draw, restore */
             VaxpWidget* child = carousel->items[idx];
             VaxpF32 old_x = child->bounds.x;
             child->bounds.x = item_x; 
             vaxp_widget_draw(child, canvas);
             child->bounds.x = old_x;
        }
    }
    
    /* Draw indicators */
    if (carousel->indicator_style == VAXP_CAROUSEL_DOTS && carousel->item_count > 1) {
        VaxpF32 total_width = carousel->item_count * CAROUSEL_DOT_SIZE + 
                               (carousel->item_count - 1) * CAROUSEL_DOT_GAP;
        VaxpF32 start_x = bounds.x + (bounds.width - total_width) / 2.0f;
        VaxpF32 dot_y = bounds.y + bounds.height - CAROUSEL_INDICATOR_MARGIN;
        
        for (VaxpU32 i = 0; i < carousel->item_count; i++) {
            VaxpBool is_current = ((VaxpI32)i == carousel->current_index);
            VaxpColor dot_color = is_current ? carousel->indicator_active : carousel->indicator_inactive;
            VaxpF32 radius = is_current ? CAROUSEL_DOT_SIZE / 2 + 1 : CAROUSEL_DOT_SIZE / 2;
            
            VaxpPaint dp = vaxp_paint_fill(dot_color);
            vaxp_canvas_draw_circle(canvas, 
                start_x + i * (CAROUSEL_DOT_SIZE + CAROUSEL_DOT_GAP) + CAROUSEL_DOT_SIZE / 2,
                dot_y, radius, &dp);
        }
    } else if (carousel->indicator_style == VAXP_CAROUSEL_NUMBERS && carousel->item_count > 1) {
        char num_str[16];
        snprintf(num_str, sizeof(num_str), "%d / %u", carousel->current_index + 1, carousel->item_count);
        
        VaxpF32 text_y = bounds.y + bounds.height - CAROUSEL_INDICATOR_MARGIN + 5;
        VaxpPaint tp = vaxp_paint_fill(carousel->indicator_active);
        vaxp_canvas_draw_text(canvas, num_str, bounds.x + bounds.width / 2, text_y, NULL, &tp);
    }
    
    /* Draw arrows */
    if (carousel->show_arrows && carousel->item_count > 1) {
        VaxpF32 arrow_y = bounds.y + (bounds.height - 40) / 2;
        VaxpColor arrow_bg = vaxp_color_rgba(0, 0, 0, 80);
        VaxpPaint abp = vaxp_paint_fill(arrow_bg);
        
        /* Left arrow */
        VaxpRectF left_arrow = {
            bounds.x + 8, arrow_y - CAROUSEL_ARROW_SIZE / 2,
            CAROUSEL_ARROW_SIZE, CAROUSEL_ARROW_SIZE
        };
        vaxp_canvas_draw_rounded_rect(canvas, left_arrow, CAROUSEL_ARROW_SIZE / 2, &abp);
        
        /* Right arrow */
        VaxpRectF right_arrow = {
            bounds.x + bounds.width - CAROUSEL_ARROW_SIZE - 8,
            arrow_y - CAROUSEL_ARROW_SIZE / 2,
            CAROUSEL_ARROW_SIZE, CAROUSEL_ARROW_SIZE
        };
        vaxp_canvas_draw_rounded_rect(canvas, right_arrow, CAROUSEL_ARROW_SIZE / 2, &abp);
        
        /* Draw arrow symbols */
        VaxpPaint awp = vaxp_paint_fill(VAXP_COLOR_WHITE);
        vaxp_canvas_draw_text(canvas, "<", 
            left_arrow.x + CAROUSEL_ARROW_SIZE / 2, arrow_y + 6, NULL, &awp);
        vaxp_canvas_draw_text(canvas, ">",
            right_arrow.x + CAROUSEL_ARROW_SIZE / 2, arrow_y + 6, NULL, &awp);
    }
    
    /* Request redraw for animation */
    if (carousel->offset_x != carousel->target_offset || carousel->auto_play) {
        widget->needs_redraw = VAXP_TRUE;
    }
}

/* ============================================================================
 * EVENTS
 * ============================================================================ */

static VaxpBool carousel_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpCarousel* c = (VaxpCarousel*)widget;
    VaxpRectF bounds = widget->bounds;
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_DOWN && c->item_count > 1) {
        VaxpF32 mx = event->mouse.x;
        VaxpF32 my = event->mouse.y;
        
        /* Check arrow clicks */
        if (c->show_arrows) {
            VaxpF32 arrow_y = bounds.y + (bounds.height - 40) / 2;
            
            /* Left arrow */
            if (mx >= bounds.x + 8 && mx <= bounds.x + 8 + CAROUSEL_ARROW_SIZE &&
                my >= arrow_y - CAROUSEL_ARROW_SIZE / 2 && 
                my <= arrow_y + CAROUSEL_ARROW_SIZE / 2) {
                vaxp_carousel_prev(c);
                return VAXP_TRUE;
            }
            
            /* Right arrow */
            if (mx >= bounds.x + bounds.width - CAROUSEL_ARROW_SIZE - 8 &&
                mx <= bounds.x + bounds.width - 8 &&
                my >= arrow_y - CAROUSEL_ARROW_SIZE / 2 &&
                my <= arrow_y + CAROUSEL_ARROW_SIZE / 2) {
                vaxp_carousel_next(c);
                return VAXP_TRUE;
            }
        }
        
        /* Start drag */
        c->is_dragging = VAXP_TRUE;
        c->drag_start_x = mx;
        return VAXP_TRUE;
    }
    
    if (event->type == VAXP_EVENT_MOUSE_MOVE && c->is_dragging) {
        VaxpF32 delta = event->mouse.x - c->drag_start_x;
        c->offset_x = c->target_offset + delta;
        vaxp_widget_invalidate(widget);
        return VAXP_TRUE;
    }
    
    if (event->type == VAXP_EVENT_MOUSE_BUTTON_UP && c->is_dragging) {
        c->is_dragging = VAXP_FALSE;
        
        VaxpF32 delta = event->mouse.x - c->drag_start_x;
        if (delta < -CAROUSEL_DRAG_THRESHOLD && c->current_index < (VaxpI32)c->item_count - 1) {
            vaxp_carousel_next(c);
        } else if (delta > CAROUSEL_DRAG_THRESHOLD && c->current_index > 0) {
            vaxp_carousel_prev(c);
        }
        
        vaxp_widget_invalidate(widget);
        return VAXP_TRUE;
    }
    
    return VAXP_FALSE;
}

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VaxpResultPtr vaxp_carousel_create(void) {
    VaxpResultPtr result = vaxp_widget_create(&vaxp_carousel_class);
    if (result.ok) {
        VaxpCarousel* c = (VaxpCarousel*)result.value;
        c->last_slide_time = get_current_time_ms();
    }
    return result;
}

void vaxp_carousel_add_item(VaxpCarousel* carousel, VaxpWidget* item) {
    if (!carousel || !item) return;
    
    VaxpU32 new_count = carousel->item_count + 1;
    VaxpWidget** new_items = vaxp_realloc(carousel->items,
        carousel->item_count * sizeof(VaxpWidget*),
        new_count * sizeof(VaxpWidget*));
    
    if (!new_items) return;
    
    carousel->items = new_items;
    carousel->items[carousel->item_count] = vaxp_ref(item);
    carousel->item_count = new_count;
    
    vaxp_widget_invalidate_layout((VaxpWidget*)carousel);
}

void vaxp_carousel_clear(VaxpCarousel* carousel) {
    if (!carousel) return;
    
    for (VaxpU32 i = 0; i < carousel->item_count; i++) {
        vaxp_unref(carousel->items[i]);
    }
    if (carousel->items) {
        vaxp_free(carousel->items, carousel->item_count * sizeof(VaxpWidget*));
        carousel->items = NULL;
    }
    carousel->item_count = 0;
    carousel->current_index = 0;
    
    vaxp_widget_invalidate((VaxpWidget*)carousel);
}

void vaxp_carousel_goto(VaxpCarousel* carousel, VaxpI32 index) {
    if (!carousel || carousel->item_count == 0) return;
    
    if (index < 0) index = 0;
    if (index >= (VaxpI32)carousel->item_count) index = carousel->item_count - 1;
    
    if (carousel->current_index != index) {
        carousel->current_index = index;
        
        if (carousel->on_change) {
            carousel->on_change(carousel, index, carousel->user_data);
        }
        
        vaxp_widget_invalidate((VaxpWidget*)carousel);
    }
}

void vaxp_carousel_next(VaxpCarousel* carousel) {
    if (!carousel || carousel->item_count == 0) return;
    
    VaxpI32 next = carousel->current_index + 1;
    if (next >= (VaxpI32)carousel->item_count) next = 0;  /* Loop */
    
    vaxp_carousel_goto(carousel, next);
}

void vaxp_carousel_prev(VaxpCarousel* carousel) {
    if (!carousel || carousel->item_count == 0) return;
    
    VaxpI32 prev = carousel->current_index - 1;
    if (prev < 0) prev = carousel->item_count - 1;  /* Loop */
    
    vaxp_carousel_goto(carousel, prev);
}

void vaxp_carousel_set_auto_play(VaxpCarousel* carousel, VaxpBool enabled, VaxpU32 interval_ms) {
    if (!carousel) return;
    carousel->auto_play = enabled;
    carousel->interval_ms = interval_ms > 0 ? interval_ms : 3000;
    carousel->last_slide_time = get_current_time_ms();
    vaxp_widget_invalidate((VaxpWidget*)carousel);
}

void vaxp_carousel_set_indicator(VaxpCarousel* carousel, VaxpCarouselIndicator style) {
    if (!carousel) return;
    carousel->indicator_style = style;
    vaxp_widget_invalidate((VaxpWidget*)carousel);
}

void vaxp_carousel_set_show_arrows(VaxpCarousel* carousel, VaxpBool show) {
    if (!carousel) return;
    carousel->show_arrows = show;
    vaxp_widget_invalidate((VaxpWidget*)carousel);
}

void vaxp_carousel_set_indicator_colors(VaxpCarousel* carousel,
                                          VaxpColor active,
                                          VaxpColor inactive) {
    if (!carousel) return;
    carousel->indicator_active = active;
    carousel->indicator_inactive = inactive;
    vaxp_widget_invalidate((VaxpWidget*)carousel);
}

void vaxp_carousel_set_on_change(VaxpCarousel* carousel,
                                   void (*callback)(VaxpCarousel*, VaxpI32, void*),
                                   void* user_data) {
    if (!carousel) return;
    carousel->on_change = callback;
    carousel->user_data = user_data;
}

VaxpI32 vaxp_carousel_get_current(VaxpCarousel* carousel) {
    return carousel ? carousel->current_index : -1;
}
