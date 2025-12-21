/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_scrollable.c - Scrollable container implementation
 */

#include "venom/widgets/venom_scrollable.h"
#include "venom/core/venom_memory.h"
#include <string.h>
#include <math.h>

/* ============================================================================
 * DEFAULT COLORS
 * ============================================================================ */

static const VenomColor DEFAULT_TRACK = { 230, 230, 230, 255 };
static const VenomColor DEFAULT_THUMB = { 180, 180, 180, 255 };
static const VenomColor DEFAULT_THUMB_HOVER = { 140, 140, 140, 255 };

#define DEFAULT_SCROLLBAR_WIDTH 10.0f
#define DEFAULT_SCROLL_STEP 40.0f
#define SMOOTH_SCROLL_FACTOR 0.15f
#define MIN_THUMB_SIZE 30.0f

/* ============================================================================
 * SCROLLABLE CLASS METHODS
 * ============================================================================ */

static void scrollable_init(VenomWidget* widget) {
    VenomScrollable* scroll = (VenomScrollable*)widget;
    
    /* Default configuration */
    scroll->direction = VENOM_SCROLL_VERTICAL;
    scroll->scrollbar_visibility = VENOM_SCROLLBAR_AUTO;
    scroll->scrollbar_width = DEFAULT_SCROLLBAR_WIDTH;
    scroll->scroll_step = DEFAULT_SCROLL_STEP;
    scroll->smooth_scrolling = VENOM_TRUE;
    
    /* Colors */
    scroll->scrollbar_track_color = DEFAULT_TRACK;
    scroll->scrollbar_thumb_color = DEFAULT_THUMB;
    scroll->scrollbar_thumb_hover_color = DEFAULT_THUMB_HOVER;
    
    /* State */
    scroll->scroll_x = 0;
    scroll->scroll_y = 0;
    scroll->content = NULL;
}

static void scrollable_destroy(VenomWidget* widget) {
    VenomScrollable* scroll = (VenomScrollable*)widget;
    
    if (scroll->content) {
        venom_unref(scroll->content);
        scroll->content = NULL;
    }
    
    /* Call parent destroy */
    venom_widget_class.destroy(widget);
}

static void scrollable_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                                VenomF32* out_width, VenomF32* out_height) {
    VenomScrollable* scroll = (VenomScrollable*)widget;
    
    /* Use available space or preferred size */
    *out_width = widget->layout.preferred_width > 0 ? 
                 widget->layout.preferred_width : available_width;
    *out_height = widget->layout.preferred_height > 0 ? 
                  widget->layout.preferred_height : available_height;
    
    /* Measure content with infinite space in scroll direction */
    if (scroll->content) {
        VenomF32 content_avail_w = (scroll->direction & VENOM_SCROLL_HORIZONTAL) ? 
                                   100000.0f : *out_width - scroll->scrollbar_width;
        VenomF32 content_avail_h = (scroll->direction & VENOM_SCROLL_VERTICAL) ? 
                                   100000.0f : *out_height - scroll->scrollbar_width;
        
        venom_widget_measure(scroll->content, content_avail_w, content_avail_h,
                             &scroll->content_width, &scroll->content_height);
    }
    
    /* Apply min constraints */
    if (*out_width < widget->layout.min_width) *out_width = widget->layout.min_width;
    if (*out_height < widget->layout.min_height) *out_height = widget->layout.min_height;
}

static void scrollable_layout(VenomWidget* widget, VenomRectF bounds) {
    VenomScrollable* scroll = (VenomScrollable*)widget;
    widget->bounds = bounds;
    
    /* Calculate viewport size (space for content, minus scrollbars if shown) */
    VenomF32 viewport_w = bounds.width;
    VenomF32 viewport_h = bounds.height;
    
    VenomBool show_v_scroll = VENOM_FALSE;
    VenomBool show_h_scroll = VENOM_FALSE;
    
    if (scroll->scrollbar_visibility == VENOM_SCROLLBAR_ALWAYS ||
        (scroll->scrollbar_visibility == VENOM_SCROLLBAR_AUTO && 
         scroll->content_height > viewport_h && 
         (scroll->direction & VENOM_SCROLL_VERTICAL))) {
        show_v_scroll = VENOM_TRUE;
        viewport_w -= scroll->scrollbar_width;
    }
    
    if (scroll->scrollbar_visibility == VENOM_SCROLLBAR_ALWAYS ||
        (scroll->scrollbar_visibility == VENOM_SCROLLBAR_AUTO && 
         scroll->content_width > viewport_w && 
         (scroll->direction & VENOM_SCROLL_HORIZONTAL))) {
        show_h_scroll = VENOM_TRUE;
        viewport_h -= scroll->scrollbar_width;
    }
    
    /* Calculate max scroll values */
    scroll->max_scroll_x = scroll->content_width > viewport_w ? 
                           scroll->content_width - viewport_w : 0;
    scroll->max_scroll_y = scroll->content_height > viewport_h ? 
                           scroll->content_height - viewport_h : 0;
    
    /* Clamp current scroll */
    if (scroll->scroll_x > scroll->max_scroll_x) scroll->scroll_x = scroll->max_scroll_x;
    if (scroll->scroll_y > scroll->max_scroll_y) scroll->scroll_y = scroll->max_scroll_y;
    if (scroll->scroll_x < 0) scroll->scroll_x = 0;
    if (scroll->scroll_y < 0) scroll->scroll_y = 0;
    
    /* Layout content at offset position */
    if (scroll->content) {
        VenomRectF content_bounds = {
            -scroll->scroll_x,
            -scroll->scroll_y,
            scroll->content_width > viewport_w ? scroll->content_width : viewport_w,
            scroll->content_height > viewport_h ? scroll->content_height : viewport_h
        };
        venom_widget_layout(scroll->content, content_bounds);
    }
}

static void scrollable_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomScrollable* scroll = (VenomScrollable*)widget;
    
    VenomF32 viewport_w = widget->bounds.width;
    VenomF32 viewport_h = widget->bounds.height;
    
    VenomBool show_v_scroll = (scroll->scrollbar_visibility != VENOM_SCROLLBAR_NEVER) &&
                              scroll->max_scroll_y > 0 &&
                              (scroll->direction & VENOM_SCROLL_VERTICAL);
    VenomBool show_h_scroll = (scroll->scrollbar_visibility != VENOM_SCROLLBAR_NEVER) &&
                              scroll->max_scroll_x > 0 &&
                              (scroll->direction & VENOM_SCROLL_HORIZONTAL);
    
    if (show_v_scroll) viewport_w -= scroll->scrollbar_width;
    if (show_h_scroll) viewport_h -= scroll->scrollbar_width;
    
    /* Save state and clip to viewport */
    venom_canvas_save(canvas);
    VenomRectF clip_rect = { 0, 0, viewport_w, viewport_h };
    venom_canvas_clip_rect(canvas, clip_rect);
    
    /* Draw content with offset */
    if (scroll->content && scroll->content->visible) {
        venom_canvas_save(canvas);
        venom_canvas_translate(canvas, -scroll->scroll_x, -scroll->scroll_y);
        venom_widget_draw(scroll->content, canvas);
        venom_canvas_restore(canvas);
    }
    
    venom_canvas_restore(canvas);
    
    /* Draw vertical scrollbar */
    if (show_v_scroll) {
        VenomF32 track_x = viewport_w;
        VenomF32 track_h = show_h_scroll ? viewport_h : widget->bounds.height;
        
        /* Track */
        VenomRectF track_rect = { track_x, 0, scroll->scrollbar_width, track_h };
        VenomPaint track_paint = venom_paint_fill(scroll->scrollbar_track_color);
        venom_canvas_draw_rect(canvas, track_rect, &track_paint);
        
        /* Thumb */
        VenomF32 thumb_ratio = viewport_h / scroll->content_height;
        VenomF32 thumb_h = track_h * thumb_ratio;
        if (thumb_h < MIN_THUMB_SIZE) thumb_h = MIN_THUMB_SIZE;
        
        VenomF32 scroll_ratio = scroll->scroll_y / scroll->max_scroll_y;
        VenomF32 thumb_y = scroll_ratio * (track_h - thumb_h);
        
        VenomColor thumb_color = scroll->vertical_thumb_hovered ? 
                                 scroll->scrollbar_thumb_hover_color : 
                                 scroll->scrollbar_thumb_color;
        VenomRectF thumb_rect = { track_x + 2, thumb_y + 2, 
                                  scroll->scrollbar_width - 4, thumb_h - 4 };
        VenomPaint thumb_paint = venom_paint_fill(thumb_color);
        venom_canvas_draw_rounded_rect(canvas, thumb_rect, 3, &thumb_paint);
    }
    
    /* Draw horizontal scrollbar */
    if (show_h_scroll) {
        VenomF32 track_y = viewport_h;
        VenomF32 track_w = show_v_scroll ? viewport_w : widget->bounds.width;
        
        /* Track */
        VenomRectF track_rect = { 0, track_y, track_w, scroll->scrollbar_width };
        VenomPaint track_paint = venom_paint_fill(scroll->scrollbar_track_color);
        venom_canvas_draw_rect(canvas, track_rect, &track_paint);
        
        /* Thumb */
        VenomF32 thumb_ratio = viewport_w / scroll->content_width;
        VenomF32 thumb_w = track_w * thumb_ratio;
        if (thumb_w < MIN_THUMB_SIZE) thumb_w = MIN_THUMB_SIZE;
        
        VenomF32 scroll_ratio = scroll->scroll_x / scroll->max_scroll_x;
        VenomF32 thumb_x = scroll_ratio * (track_w - thumb_w);
        
        VenomColor thumb_color = scroll->horizontal_thumb_hovered ? 
                                 scroll->scrollbar_thumb_hover_color : 
                                 scroll->scrollbar_thumb_color;
        VenomRectF thumb_rect = { thumb_x + 2, track_y + 2, 
                                  thumb_w - 4, scroll->scrollbar_width - 4 };
        VenomPaint thumb_paint = venom_paint_fill(thumb_color);
        venom_canvas_draw_rounded_rect(canvas, thumb_rect, 3, &thumb_paint);
    }
}

static VenomBool scrollable_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomScrollable* scroll = (VenomScrollable*)widget;
    
    switch (event->type) {
        case VENOM_EVENT_MOUSE_SCROLL:
            if (scroll->direction & VENOM_SCROLL_VERTICAL) {
                scroll->scroll_y += event->scroll.delta_y * scroll->scroll_step;
                
                /* Clamp */
                if (scroll->scroll_y < 0) scroll->scroll_y = 0;
                if (scroll->scroll_y > scroll->max_scroll_y) 
                    scroll->scroll_y = scroll->max_scroll_y;
                
                widget->needs_redraw = VENOM_TRUE;
                return VENOM_TRUE;
            }
            if (scroll->direction & VENOM_SCROLL_HORIZONTAL) {
                scroll->scroll_x += event->scroll.delta_x * scroll->scroll_step;
                
                if (scroll->scroll_x < 0) scroll->scroll_x = 0;
                if (scroll->scroll_x > scroll->max_scroll_x) 
                    scroll->scroll_x = scroll->max_scroll_x;
                
                widget->needs_redraw = VENOM_TRUE;
                return VENOM_TRUE;
            }
            break;
            
        case VENOM_EVENT_KEY_DOWN:
            switch (event->key.key) {
                case VENOM_KEY_PAGE_UP:
                    scroll->scroll_y -= widget->bounds.height * 0.9f;
                    if (scroll->scroll_y < 0) scroll->scroll_y = 0;
                    widget->needs_redraw = VENOM_TRUE;
                    return VENOM_TRUE;
                    
                case VENOM_KEY_PAGE_DOWN:
                    scroll->scroll_y += widget->bounds.height * 0.9f;
                    if (scroll->scroll_y > scroll->max_scroll_y)
                        scroll->scroll_y = scroll->max_scroll_y;
                    widget->needs_redraw = VENOM_TRUE;
                    return VENOM_TRUE;
                    
                case VENOM_KEY_HOME:
                    if (event->key.modifiers & VENOM_KEYMOD_CTRL) {
                        scroll->scroll_y = 0;
                        widget->needs_redraw = VENOM_TRUE;
                        return VENOM_TRUE;
                    }
                    break;
                    
                case VENOM_KEY_END:
                    if (event->key.modifiers & VENOM_KEYMOD_CTRL) {
                        scroll->scroll_y = scroll->max_scroll_y;
                        widget->needs_redraw = VENOM_TRUE;
                        return VENOM_TRUE;
                    }
                    break;
                    
                default:
                    break;
            }
            break;
            
        default:
            break;
    }
    
    /* Dispatch to content */
    if (scroll->content) {
        return venom_widget_dispatch_event(scroll->content, event);
    }
    
    return VENOM_FALSE;
}

/* ============================================================================
 * SCROLLABLE CLASS
 * ============================================================================ */

const VenomWidgetClass venom_scrollable_class = {
    .class_name = "VenomScrollable",
    .instance_size = sizeof(VenomScrollable),
    .parent_class = &venom_widget_class,
    .init = scrollable_init,
    .destroy = scrollable_destroy,
    .measure = scrollable_measure,
    .layout = scrollable_layout,
    .draw = scrollable_draw,
    .on_event = scrollable_on_event,
    .on_state_changed = NULL,
};

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VenomResultPtr venom_scrollable_create(void) {
    return venom_widget_create(&venom_scrollable_class);
}

VenomResult venom_scrollable_set_content(VenomScrollable* scroll, VenomWidget* content) {
    VENOM_ENSURE_NOT_NULL(scroll);
    
    if (scroll->content) {
        scroll->content->parent = NULL;
        venom_unref(scroll->content);
    }
    
    scroll->content = content;
    if (content) {
        venom_ref(content);
        content->parent = (VenomWidget*)scroll;
    }
    
    venom_widget_invalidate_layout((VenomWidget*)scroll);
    return VENOM_OK_UNIT();
}

void venom_scrollable_get_scroll(const VenomScrollable* scroll, VenomF32* x, VenomF32* y) {
    if (!scroll) return;
    if (x) *x = scroll->scroll_x;
    if (y) *y = scroll->scroll_y;
}

void venom_scrollable_set_scroll(VenomScrollable* scroll, VenomF32 x, VenomF32 y) {
    if (!scroll) return;
    
    scroll->scroll_x = x;
    scroll->scroll_y = y;
    
    /* Clamp */
    if (scroll->scroll_x < 0) scroll->scroll_x = 0;
    if (scroll->scroll_y < 0) scroll->scroll_y = 0;
    if (scroll->scroll_x > scroll->max_scroll_x) scroll->scroll_x = scroll->max_scroll_x;
    if (scroll->scroll_y > scroll->max_scroll_y) scroll->scroll_y = scroll->max_scroll_y;
    
    venom_widget_invalidate((VenomWidget*)scroll);
}

void venom_scrollable_scroll_by(VenomScrollable* scroll, VenomF32 dx, VenomF32 dy) {
    if (!scroll) return;
    venom_scrollable_set_scroll(scroll, scroll->scroll_x + dx, scroll->scroll_y + dy);
}

void venom_scrollable_ensure_visible(VenomScrollable* scroll, VenomWidget* widget) {
    if (!scroll || !widget) return;
    
    /* Get widget position relative to content */
    VenomF32 widget_top = widget->bounds.y;
    VenomF32 widget_bottom = widget->bounds.y + widget->bounds.height;
    VenomF32 viewport_h = ((VenomWidget*)scroll)->bounds.height;
    
    /* Scroll to make widget visible */
    if (widget_top < scroll->scroll_y) {
        scroll->scroll_y = widget_top;
    } else if (widget_bottom > scroll->scroll_y + viewport_h) {
        scroll->scroll_y = widget_bottom - viewport_h;
    }
    
    venom_widget_invalidate((VenomWidget*)scroll);
}

void venom_scrollable_set_direction(VenomScrollable* scroll, VenomScrollDirection direction) {
    if (scroll) scroll->direction = direction;
}

void venom_scrollable_set_scrollbar_visibility(VenomScrollable* scroll, VenomScrollbarVisibility visibility) {
    if (scroll) scroll->scrollbar_visibility = visibility;
}

void venom_scrollable_set_smooth(VenomScrollable* scroll, VenomBool smooth) {
    if (scroll) scroll->smooth_scrolling = smooth;
}

void venom_scrollable_set_scrollbar_colors(VenomScrollable* scroll, VenomColor track, VenomColor thumb) {
    if (!scroll) return;
    scroll->scrollbar_track_color = track;
    scroll->scrollbar_thumb_color = thumb;
    venom_widget_invalidate((VenomWidget*)scroll);
}

/* ============================================================================
 * CONVENIENCE BUILDER
 * ============================================================================ */

VenomWidget* _venom_scrollable_build(const VenomScrollableConfig* config) {
    VenomResultPtr result = venom_scrollable_create();
    if (!result.ok) return NULL;
    
    VenomScrollable* scroll = (VenomScrollable*)result.value;
    VenomWidget* widget = (VenomWidget*)scroll;
    
    if (config->content) {
        venom_scrollable_set_content(scroll, config->content);
    }
    if (config->direction != 0) {
        scroll->direction = config->direction;
    }
    if (config->scrollbar != 0) {
        scroll->scrollbar_visibility = config->scrollbar;
    }
    scroll->smooth_scrolling = config->smooth;
    
    if (config->width > 0) widget->layout.preferred_width = config->width;
    if (config->height > 0) widget->layout.preferred_height = config->height;
    
    return widget;
}
