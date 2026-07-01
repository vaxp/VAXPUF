/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_scrollable.c - Scrollable container implementation
 */

#include "vaxp/widgets/vaxp_scrollable.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>
#include <math.h>

/* ============================================================================
 * DEFAULT COLORS
 * ============================================================================ */

static const VaxpColor DEFAULT_TRACK = { 230, 230, 230, 255 };
static const VaxpColor DEFAULT_THUMB = { 180, 180, 180, 255 };
static const VaxpColor DEFAULT_THUMB_HOVER = { 140, 140, 140, 255 };

#define DEFAULT_SCROLLBAR_WIDTH 10.0f
#define DEFAULT_SCROLL_STEP 40.0f
#define SMOOTH_SCROLL_FACTOR 0.15f
#define MIN_THUMB_SIZE 30.0f

/* ============================================================================
 * SCROLLABLE CLASS METHODS
 * ============================================================================ */

static void scrollable_init(VaxpWidget* widget) {
    VaxpScrollable* scroll = (VaxpScrollable*)widget;
    
    /* Default configuration */
    scroll->direction = VAXP_SCROLL_VERTICAL;
    scroll->scrollbar_visibility = VAXP_SCROLLBAR_AUTO;
    scroll->scrollbar_width = DEFAULT_SCROLLBAR_WIDTH;
    scroll->scroll_step = DEFAULT_SCROLL_STEP;
    scroll->smooth_scrolling = VAXP_TRUE;
    
    /* Colors */
    scroll->scrollbar_track_color = DEFAULT_TRACK;
    scroll->scrollbar_thumb_color = DEFAULT_THUMB;
    scroll->scrollbar_thumb_hover_color = DEFAULT_THUMB_HOVER;
    
    /* State */
    scroll->scroll_x = 0;
    scroll->scroll_y = 0;
    scroll->content = NULL;
}

static void scrollable_destroy(VaxpWidget* widget) {
    VaxpScrollable* scroll = (VaxpScrollable*)widget;
    
    if (scroll->content) {
        vaxp_unref(scroll->content);
        scroll->content = NULL;
    }
    
    /* Call parent destroy */
    vaxp_widget_class.destroy(widget);
}

static void scrollable_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                                VaxpF32* out_width, VaxpF32* out_height) {
    VaxpScrollable* scroll = (VaxpScrollable*)widget;
    
    /* Use available space or preferred size */
    *out_width = widget->layout.preferred_width > 0 ? 
                 widget->layout.preferred_width : available_width;
    *out_height = widget->layout.preferred_height > 0 ? 
                  widget->layout.preferred_height : available_height;
    
    /* Measure content with infinite space in scroll direction */
    if (scroll->content) {
        VaxpF32 content_avail_w = (scroll->direction & VAXP_SCROLL_HORIZONTAL) ? 
                                   100000.0f : *out_width - scroll->scrollbar_width;
        VaxpF32 content_avail_h = (scroll->direction & VAXP_SCROLL_VERTICAL) ? 
                                   100000.0f : *out_height - scroll->scrollbar_width;
        
        vaxp_widget_measure(scroll->content, content_avail_w, content_avail_h,
                             &scroll->content_width, &scroll->content_height);
    }
    
    /* Apply min constraints */
    if (*out_width < widget->layout.min_width) *out_width = widget->layout.min_width;
    if (*out_height < widget->layout.min_height) *out_height = widget->layout.min_height;
}

static void scrollable_layout(VaxpWidget* widget, VaxpRectF bounds) {
    VaxpScrollable* scroll = (VaxpScrollable*)widget;
    widget->bounds = bounds;
    
    /* Calculate viewport size (space for content, minus scrollbars if shown) */
    VaxpF32 viewport_w = bounds.width;
    VaxpF32 viewport_h = bounds.height;
    
    VaxpBool show_v_scroll = VAXP_FALSE;
    VaxpBool show_h_scroll = VAXP_FALSE;
    
    if (scroll->scrollbar_visibility == VAXP_SCROLLBAR_ALWAYS ||
        (scroll->scrollbar_visibility == VAXP_SCROLLBAR_AUTO && 
         scroll->content_height > viewport_h && 
         (scroll->direction & VAXP_SCROLL_VERTICAL))) {
        show_v_scroll = VAXP_TRUE;
        viewport_w -= scroll->scrollbar_width;
    }
    
    if (scroll->scrollbar_visibility == VAXP_SCROLLBAR_ALWAYS ||
        (scroll->scrollbar_visibility == VAXP_SCROLLBAR_AUTO && 
         scroll->content_width > viewport_w && 
         (scroll->direction & VAXP_SCROLL_HORIZONTAL))) {
        show_h_scroll = VAXP_TRUE;
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
        VaxpRectF content_bounds = {
            -scroll->scroll_x,
            -scroll->scroll_y,
            scroll->content_width > viewport_w ? scroll->content_width : viewport_w,
            scroll->content_height > viewport_h ? scroll->content_height : viewport_h
        };
        vaxp_widget_layout(scroll->content, content_bounds);
    }
}

static void scrollable_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpScrollable* scroll = (VaxpScrollable*)widget;
    
    VaxpF32 viewport_w = widget->bounds.width;
    VaxpF32 viewport_h = widget->bounds.height;
    
    VaxpBool show_v_scroll = (scroll->scrollbar_visibility != VAXP_SCROLLBAR_NEVER) &&
                              scroll->max_scroll_y > 0 &&
                              (scroll->direction & VAXP_SCROLL_VERTICAL);
    VaxpBool show_h_scroll = (scroll->scrollbar_visibility != VAXP_SCROLLBAR_NEVER) &&
                              scroll->max_scroll_x > 0 &&
                              (scroll->direction & VAXP_SCROLL_HORIZONTAL);
    
    if (show_v_scroll) viewport_w -= scroll->scrollbar_width;
    if (show_h_scroll) viewport_h -= scroll->scrollbar_width;
    
    /* Save state and clip to viewport */
    vaxp_canvas_save(canvas);
    VaxpRectF clip_rect = { 0, 0, viewport_w, viewport_h };
    vaxp_canvas_clip_rect(canvas, clip_rect);
    
    /* Draw content with offset */
    if (scroll->content && scroll->content->visible) {
        vaxp_canvas_save(canvas);
        vaxp_canvas_translate(canvas, -scroll->scroll_x, -scroll->scroll_y);
        vaxp_widget_draw(scroll->content, canvas);
        vaxp_canvas_restore(canvas);
    }
    
    vaxp_canvas_restore(canvas);
    
    /* Draw vertical scrollbar */
    if (show_v_scroll) {
        VaxpF32 track_x = viewport_w;
        VaxpF32 track_h = show_h_scroll ? viewport_h : widget->bounds.height;
        
        /* Track */
        VaxpRectF track_rect = { track_x, 0, scroll->scrollbar_width, track_h };
        VaxpPaint track_paint = vaxp_paint_fill(scroll->scrollbar_track_color);
        vaxp_canvas_draw_rect(canvas, track_rect, &track_paint);
        
        /* Thumb */
        VaxpF32 thumb_ratio = viewport_h / scroll->content_height;
        VaxpF32 thumb_h = track_h * thumb_ratio;
        if (thumb_h < MIN_THUMB_SIZE) thumb_h = MIN_THUMB_SIZE;
        
        VaxpF32 scroll_ratio = scroll->scroll_y / scroll->max_scroll_y;
        VaxpF32 thumb_y = scroll_ratio * (track_h - thumb_h);
        
        VaxpColor thumb_color = scroll->vertical_thumb_hovered ? 
                                 scroll->scrollbar_thumb_hover_color : 
                                 scroll->scrollbar_thumb_color;
        VaxpRectF thumb_rect = { track_x + 2, thumb_y + 2, 
                                  scroll->scrollbar_width - 4, thumb_h - 4 };
        VaxpPaint thumb_paint = vaxp_paint_fill(thumb_color);
        vaxp_canvas_draw_rounded_rect(canvas, thumb_rect, 3, &thumb_paint);
    }
    
    /* Draw horizontal scrollbar */
    if (show_h_scroll) {
        VaxpF32 track_y = viewport_h;
        VaxpF32 track_w = show_v_scroll ? viewport_w : widget->bounds.width;
        
        /* Track */
        VaxpRectF track_rect = { 0, track_y, track_w, scroll->scrollbar_width };
        VaxpPaint track_paint = vaxp_paint_fill(scroll->scrollbar_track_color);
        vaxp_canvas_draw_rect(canvas, track_rect, &track_paint);
        
        /* Thumb */
        VaxpF32 thumb_ratio = viewport_w / scroll->content_width;
        VaxpF32 thumb_w = track_w * thumb_ratio;
        if (thumb_w < MIN_THUMB_SIZE) thumb_w = MIN_THUMB_SIZE;
        
        VaxpF32 scroll_ratio = scroll->scroll_x / scroll->max_scroll_x;
        VaxpF32 thumb_x = scroll_ratio * (track_w - thumb_w);
        
        VaxpColor thumb_color = scroll->horizontal_thumb_hovered ? 
                                 scroll->scrollbar_thumb_hover_color : 
                                 scroll->scrollbar_thumb_color;
        VaxpRectF thumb_rect = { thumb_x + 2, track_y + 2, 
                                  thumb_w - 4, scroll->scrollbar_width - 4 };
        VaxpPaint thumb_paint = vaxp_paint_fill(thumb_color);
        vaxp_canvas_draw_rounded_rect(canvas, thumb_rect, 3, &thumb_paint);
    }
}

static VaxpBool scrollable_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpScrollable* scroll = (VaxpScrollable*)widget;
    
    switch (event->type) {
        case VAXP_EVENT_MOUSE_SCROLL:
            if (scroll->direction & VAXP_SCROLL_VERTICAL) {
                scroll->scroll_y += event->scroll.delta_y * scroll->scroll_step;
                
                /* Clamp */
                if (scroll->scroll_y < 0) scroll->scroll_y = 0;
                if (scroll->scroll_y > scroll->max_scroll_y) 
                    scroll->scroll_y = scroll->max_scroll_y;
                
                widget->needs_redraw = VAXP_TRUE;
                return VAXP_TRUE;
            }
            if (scroll->direction & VAXP_SCROLL_HORIZONTAL) {
                scroll->scroll_x += event->scroll.delta_x * scroll->scroll_step;
                
                if (scroll->scroll_x < 0) scroll->scroll_x = 0;
                if (scroll->scroll_x > scroll->max_scroll_x) 
                    scroll->scroll_x = scroll->max_scroll_x;
                
                widget->needs_redraw = VAXP_TRUE;
                return VAXP_TRUE;
            }
            break;
            
        case VAXP_EVENT_KEY_DOWN:
            switch (event->key.key) {
                case VAXP_KEY_PAGE_UP:
                    scroll->scroll_y -= widget->bounds.height * 0.9f;
                    if (scroll->scroll_y < 0) scroll->scroll_y = 0;
                    widget->needs_redraw = VAXP_TRUE;
                    return VAXP_TRUE;
                    
                case VAXP_KEY_PAGE_DOWN:
                    scroll->scroll_y += widget->bounds.height * 0.9f;
                    if (scroll->scroll_y > scroll->max_scroll_y)
                        scroll->scroll_y = scroll->max_scroll_y;
                    widget->needs_redraw = VAXP_TRUE;
                    return VAXP_TRUE;
                    
                case VAXP_KEY_HOME:
                    if (event->key.modifiers & VAXP_KEYMOD_CTRL) {
                        scroll->scroll_y = 0;
                        widget->needs_redraw = VAXP_TRUE;
                        return VAXP_TRUE;
                    }
                    break;
                    
                case VAXP_KEY_END:
                    if (event->key.modifiers & VAXP_KEYMOD_CTRL) {
                        scroll->scroll_y = scroll->max_scroll_y;
                        widget->needs_redraw = VAXP_TRUE;
                        return VAXP_TRUE;
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
        return vaxp_widget_dispatch_event(scroll->content, event);
    }
    
    return VAXP_FALSE;
}

/* ============================================================================
 * SCROLLABLE CLASS
 * ============================================================================ */

const VaxpWidgetClass vaxp_scrollable_class = {
    .class_name = "VaxpScrollable",
    .instance_size = sizeof(VaxpScrollable),
    .parent_class = &vaxp_widget_class,
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

VaxpResultPtr vaxp_scrollable_create(void) {
    return vaxp_widget_create(&vaxp_scrollable_class);
}

VaxpResult vaxp_scrollable_set_content(VaxpScrollable* scroll, VaxpWidget* content) {
    VAXP_ENSURE_NOT_NULL(scroll);
    
    if (scroll->content) {
        scroll->content->parent = NULL;
        vaxp_unref(scroll->content);
    }
    
    scroll->content = content;
    if (content) {
        vaxp_ref(content);
        content->parent = (VaxpWidget*)scroll;
    }
    
    vaxp_widget_invalidate_layout((VaxpWidget*)scroll);
    return VAXP_OK_UNIT();
}

void vaxp_scrollable_get_scroll(const VaxpScrollable* scroll, VaxpF32* x, VaxpF32* y) {
    if (!scroll) return;
    if (x) *x = scroll->scroll_x;
    if (y) *y = scroll->scroll_y;
}

void vaxp_scrollable_set_scroll(VaxpScrollable* scroll, VaxpF32 x, VaxpF32 y) {
    if (!scroll) return;
    
    scroll->scroll_x = x;
    scroll->scroll_y = y;
    
    /* Clamp */
    if (scroll->scroll_x < 0) scroll->scroll_x = 0;
    if (scroll->scroll_y < 0) scroll->scroll_y = 0;
    if (scroll->scroll_x > scroll->max_scroll_x) scroll->scroll_x = scroll->max_scroll_x;
    if (scroll->scroll_y > scroll->max_scroll_y) scroll->scroll_y = scroll->max_scroll_y;
    
    vaxp_widget_invalidate((VaxpWidget*)scroll);
}

void vaxp_scrollable_scroll_by(VaxpScrollable* scroll, VaxpF32 dx, VaxpF32 dy) {
    if (!scroll) return;
    vaxp_scrollable_set_scroll(scroll, scroll->scroll_x + dx, scroll->scroll_y + dy);
}

void vaxp_scrollable_ensure_visible(VaxpScrollable* scroll, VaxpWidget* widget) {
    if (!scroll || !widget) return;
    
    /* Get widget position relative to content */
    VaxpF32 widget_top = widget->bounds.y;
    VaxpF32 widget_bottom = widget->bounds.y + widget->bounds.height;
    VaxpF32 viewport_h = ((VaxpWidget*)scroll)->bounds.height;
    
    /* Scroll to make widget visible */
    if (widget_top < scroll->scroll_y) {
        scroll->scroll_y = widget_top;
    } else if (widget_bottom > scroll->scroll_y + viewport_h) {
        scroll->scroll_y = widget_bottom - viewport_h;
    }
    
    vaxp_widget_invalidate((VaxpWidget*)scroll);
}

void vaxp_scrollable_set_direction(VaxpScrollable* scroll, VaxpScrollDirection direction) {
    if (scroll) scroll->direction = direction;
}

void vaxp_scrollable_set_scrollbar_visibility(VaxpScrollable* scroll, VaxpScrollbarVisibility visibility) {
    if (scroll) scroll->scrollbar_visibility = visibility;
}

void vaxp_scrollable_set_smooth(VaxpScrollable* scroll, VaxpBool smooth) {
    if (scroll) scroll->smooth_scrolling = smooth;
}

void vaxp_scrollable_set_scrollbar_colors(VaxpScrollable* scroll, VaxpColor track, VaxpColor thumb) {
    if (!scroll) return;
    scroll->scrollbar_track_color = track;
    scroll->scrollbar_thumb_color = thumb;
    vaxp_widget_invalidate((VaxpWidget*)scroll);
}

/* ============================================================================
 * CONVENIENCE BUILDER
 * ============================================================================ */

VaxpWidget* _vaxp_scrollable_build(const VaxpScrollableConfig* config) {
    VaxpResultPtr result = vaxp_scrollable_create();
    if (!result.ok) return NULL;
    
    VaxpScrollable* scroll = (VaxpScrollable*)result.value;
    VaxpWidget* widget = (VaxpWidget*)scroll;
    
    if (config->content) {
        vaxp_scrollable_set_content(scroll, config->content);
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
