/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_popover.c - Popover widget implementation
 */

#include "venom/widgets/venom_popover.h"
#include "venom/core/venom_memory.h"
#include <string.h>

#define DEFAULT_OFFSET 8.0f
#define DEFAULT_ARROW_SIZE 8.0f
#define DEFAULT_CORNER_RADIUS 8.0f
#define DEFAULT_PADDING 12.0f
#define DEFAULT_SHADOW_BLUR 10.0f

/* ============================================================================
 * LIFECYCLE
 * ============================================================================ */

static void popover_init(VenomWidget* widget) {
    VenomPopover* popover = (VenomPopover*)widget;
    
    popover->anchor = NULL;
    popover->content = NULL;
    
    popover->is_open = VENOM_FALSE;
    popover->placement = VENOM_POPOVER_BOTTOM;
    popover->trigger = VENOM_POPOVER_TRIGGER_CLICK;
    popover->show_arrow = VENOM_TRUE;
    popover->close_on_outside = VENOM_TRUE;
    popover->close_on_escape = VENOM_TRUE;
    
    popover->offset = DEFAULT_OFFSET;
    popover->arrow_size = DEFAULT_ARROW_SIZE;
    
    popover->background_color = (VenomColor){ 255, 255, 255, 255 };
    popover->border_color = (VenomColor){ 224, 224, 224, 255 };
    popover->shadow_color = (VenomColor){ 0, 0, 0, 30 };
    popover->corner_radius = DEFAULT_CORNER_RADIUS;
    popover->border_width = 1.0f;
    popover->shadow_blur = DEFAULT_SHADOW_BLUR;
    popover->padding = DEFAULT_PADDING;
    
    popover->on_open = NULL;
    popover->on_close = NULL;
    popover->callback_data = NULL;
}

static void popover_destroy(VenomWidget* widget) {
    VenomPopover* popover = (VenomPopover*)widget;
    
    if (popover->content) {
        popover->content->parent = NULL;
        venom_unref(popover->content);
        popover->content = NULL;
    }
    
    /* Note: anchor is not owned by popover */
    
    venom_widget_class.destroy(widget);
}

/* ============================================================================
 * POSITIONING
 * ============================================================================ */

static void calculate_position(VenomPopover* popover, VenomF32 anchor_x, VenomF32 anchor_y,
                               VenomF32 anchor_w, VenomF32 anchor_h,
                               VenomF32 content_w, VenomF32 content_h,
                               VenomF32* out_x, VenomF32* out_y) {
    VenomF32 offset = popover->offset + (popover->show_arrow ? popover->arrow_size : 0);
    
    switch (popover->placement) {
        case VENOM_POPOVER_TOP:
            *out_x = anchor_x + (anchor_w - content_w) / 2;
            *out_y = anchor_y - content_h - offset;
            break;
        case VENOM_POPOVER_TOP_START:
            *out_x = anchor_x;
            *out_y = anchor_y - content_h - offset;
            break;
        case VENOM_POPOVER_TOP_END:
            *out_x = anchor_x + anchor_w - content_w;
            *out_y = anchor_y - content_h - offset;
            break;
            
        case VENOM_POPOVER_BOTTOM:
            *out_x = anchor_x + (anchor_w - content_w) / 2;
            *out_y = anchor_y + anchor_h + offset;
            break;
        case VENOM_POPOVER_BOTTOM_START:
            *out_x = anchor_x;
            *out_y = anchor_y + anchor_h + offset;
            break;
        case VENOM_POPOVER_BOTTOM_END:
            *out_x = anchor_x + anchor_w - content_w;
            *out_y = anchor_y + anchor_h + offset;
            break;
            
        case VENOM_POPOVER_LEFT:
            *out_x = anchor_x - content_w - offset;
            *out_y = anchor_y + (anchor_h - content_h) / 2;
            break;
        case VENOM_POPOVER_LEFT_START:
            *out_x = anchor_x - content_w - offset;
            *out_y = anchor_y;
            break;
        case VENOM_POPOVER_LEFT_END:
            *out_x = anchor_x - content_w - offset;
            *out_y = anchor_y + anchor_h - content_h;
            break;
            
        case VENOM_POPOVER_RIGHT:
            *out_x = anchor_x + anchor_w + offset;
            *out_y = anchor_y + (anchor_h - content_h) / 2;
            break;
        case VENOM_POPOVER_RIGHT_START:
            *out_x = anchor_x + anchor_w + offset;
            *out_y = anchor_y;
            break;
        case VENOM_POPOVER_RIGHT_END:
            *out_x = anchor_x + anchor_w + offset;
            *out_y = anchor_y + anchor_h - content_h;
            break;
    }
}

/* ============================================================================
 * RENDERING
 * ============================================================================ */

static void popover_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                            VenomF32* out_width, VenomF32* out_height) {
    VenomPopover* popover = (VenomPopover*)widget;
    
    if (popover->content && popover->is_open) {
        VenomF32 cw, ch;
        venom_widget_measure(popover->content, available_width - popover->padding * 2,
                            available_height - popover->padding * 2, &cw, &ch);
        *out_width = cw + popover->padding * 2;
        *out_height = ch + popover->padding * 2;
    } else {
        *out_width = 0;
        *out_height = 0;
    }
}

static void popover_layout(VenomWidget* widget, VenomRectF bounds) {
    VenomPopover* popover = (VenomPopover*)widget;
    widget->bounds = bounds;
    
    if (popover->content && popover->is_open) {
        VenomRectF content_bounds = {
            popover->padding, popover->padding,
            bounds.width - popover->padding * 2,
            bounds.height - popover->padding * 2
        };
        venom_widget_layout(popover->content, content_bounds);
    }
}

static void draw_arrow(VenomCanvas* canvas, VenomPopover* popover, VenomF32 x, VenomF32 y, VenomF32 w, VenomF32 h) {
    VenomF32 arrow = popover->arrow_size;
    VenomPaint fill = venom_paint_fill(popover->background_color);
    VenomPaint border = venom_paint_stroke(popover->border_color, popover->border_width);
    
    VenomBool is_top = popover->placement >= VENOM_POPOVER_TOP && popover->placement <= VENOM_POPOVER_TOP_END;
    VenomBool is_bottom = popover->placement >= VENOM_POPOVER_BOTTOM && popover->placement <= VENOM_POPOVER_BOTTOM_END;
    VenomBool is_left = popover->placement >= VENOM_POPOVER_LEFT && popover->placement <= VENOM_POPOVER_LEFT_END;
    VenomBool is_right = popover->placement >= VENOM_POPOVER_RIGHT && popover->placement <= VENOM_POPOVER_RIGHT_END;
    
    VenomF32 ax, ay;
    
    if (is_top) {
        ax = x + w / 2;
        ay = y + h;
        /* Draw triangle pointing down */
        venom_canvas_draw_line(canvas, ax - arrow, ay, ax, ay + arrow, &fill);
        venom_canvas_draw_line(canvas, ax, ay + arrow, ax + arrow, ay, &fill);
    } else if (is_bottom) {
        ax = x + w / 2;
        ay = y;
        /* Draw triangle pointing up */
        venom_canvas_draw_line(canvas, ax - arrow, ay, ax, ay - arrow, &fill);
        venom_canvas_draw_line(canvas, ax, ay - arrow, ax + arrow, ay, &fill);
    } else if (is_left) {
        ax = x + w;
        ay = y + h / 2;
        /* Draw triangle pointing right */
        venom_canvas_draw_line(canvas, ax, ay - arrow, ax + arrow, ay, &fill);
        venom_canvas_draw_line(canvas, ax + arrow, ay, ax, ay + arrow, &fill);
    } else if (is_right) {
        ax = x;
        ay = y + h / 2;
        /* Draw triangle pointing left */
        venom_canvas_draw_line(canvas, ax, ay - arrow, ax - arrow, ay, &fill);
        venom_canvas_draw_line(canvas, ax - arrow, ay, ax, ay + arrow, &fill);
    }
    
    (void)border;  /* TODO: Draw arrow border */
}

static void popover_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomPopover* popover = (VenomPopover*)widget;
    
    if (!popover->is_open || !popover->content) return;
    
    VenomF32 w = widget->bounds.width;
    VenomF32 h = widget->bounds.height;
    
    /* Draw shadow */
    for (int i = 0; i < 3; i++) {
        VenomF32 offset = popover->shadow_blur * (0.5f + i * 0.2f);
        VenomColor sc = popover->shadow_color;
        sc.a = (VenomU8)(30 - i * 8);
        VenomRectF shadow = { offset, offset, w, h };
        VenomPaint shadow_paint = venom_paint_fill(sc);
        venom_canvas_draw_rounded_rect(canvas, shadow, popover->corner_radius, &shadow_paint);
    }
    
    /* Draw background */
    VenomRectF bg = { 0, 0, w, h };
    VenomPaint bg_paint = venom_paint_fill(popover->background_color);
    venom_canvas_draw_rounded_rect(canvas, bg, popover->corner_radius, &bg_paint);
    
    /* Draw border */
    if (popover->border_width > 0) {
        VenomPaint border_paint = venom_paint_stroke(popover->border_color, popover->border_width);
        venom_canvas_draw_rounded_rect(canvas, bg, popover->corner_radius, &border_paint);
    }
    
    /* Draw arrow */
    if (popover->show_arrow) {
        draw_arrow(canvas, popover, 0, 0, w, h);
    }
    
    /* Draw content */
    if (popover->content && popover->content->visible) {
        venom_canvas_save(canvas);
        venom_canvas_translate(canvas, popover->content->bounds.x, popover->content->bounds.y);
        venom_widget_draw(popover->content, canvas);
        venom_canvas_restore(canvas);
    }
}

/* ============================================================================
 * EVENTS
 * ============================================================================ */

static VenomBool popover_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomPopover* popover = (VenomPopover*)widget;
    
    if (!popover->is_open) return VENOM_FALSE;
    
    switch (event->type) {
        case VENOM_EVENT_KEY_DOWN:
            if (event->key.key == VENOM_KEY_ESCAPE && popover->close_on_escape) {
                venom_popover_close(popover);
                return VENOM_TRUE;
            }
            break;
            
        case VENOM_EVENT_MOUSE_BUTTON_DOWN:
            /* Check if click is outside popover */
            if (popover->close_on_outside) {
                VenomF32 mx = (VenomF32)event->mouse.x;
                VenomF32 my = (VenomF32)event->mouse.y;
                
                if (mx < 0 || my < 0 || mx > widget->bounds.width || my > widget->bounds.height) {
                    venom_popover_close(popover);
                    return VENOM_TRUE;
                }
            }
            break;
            
        default:
            break;
    }
    
    /* Pass to content */
    if (popover->content) {
        return venom_widget_dispatch_event(popover->content, event);
    }
    
    return VENOM_FALSE;
}

/* ============================================================================
 * CLASS
 * ============================================================================ */

const VenomWidgetClass venom_popover_class = {
    .class_name = "VenomPopover",
    .instance_size = sizeof(VenomPopover),
    .parent_class = &venom_widget_class,
    .init = popover_init,
    .destroy = popover_destroy,
    .measure = popover_measure,
    .layout = popover_layout,
    .draw = popover_draw,
    .on_event = popover_on_event,
    .on_state_changed = NULL,
};

/* ============================================================================
 * PUBLIC API
 * ============================================================================ */

VenomResultPtr venom_popover_create(void) {
    return venom_widget_create(&venom_popover_class);
}

void venom_popover_set_anchor(VenomPopover* popover, VenomWidget* anchor) {
    if (popover) {
        popover->anchor = anchor;
    }
}

VenomResult venom_popover_set_content(VenomPopover* popover, VenomWidget* content) {
    VENOM_ENSURE_NOT_NULL(popover);
    
    if (popover->content) {
        popover->content->parent = NULL;
        venom_unref(popover->content);
    }
    
    popover->content = content;
    if (content) {
        venom_ref(content);
        content->parent = (VenomWidget*)popover;
    }
    
    return VENOM_OK_UNIT();
}

void venom_popover_set_placement(VenomPopover* popover, VenomPopoverPlacement placement) {
    if (popover) {
        popover->placement = placement;
        if (popover->is_open) venom_widget_invalidate((VenomWidget*)popover);
    }
}

void venom_popover_set_trigger(VenomPopover* popover, VenomPopoverTrigger trigger) {
    if (popover) popover->trigger = trigger;
}

void venom_popover_set_offset(VenomPopover* popover, VenomF32 offset) {
    if (popover) {
        popover->offset = offset;
        if (popover->is_open) venom_widget_invalidate((VenomWidget*)popover);
    }
}

void venom_popover_set_arrow(VenomPopover* popover, VenomBool show) {
    if (popover) {
        popover->show_arrow = show;
        if (popover->is_open) venom_widget_invalidate((VenomWidget*)popover);
    }
}

void venom_popover_open(VenomPopover* popover) {
    if (popover && !popover->is_open) {
        popover->is_open = VENOM_TRUE;
        ((VenomWidget*)popover)->visible = VENOM_TRUE;
        venom_widget_invalidate((VenomWidget*)popover);
        
        if (popover->on_open) {
            popover->on_open(popover, VENOM_TRUE, popover->callback_data);
        }
    }
}

void venom_popover_close(VenomPopover* popover) {
    if (popover && popover->is_open) {
        popover->is_open = VENOM_FALSE;
        ((VenomWidget*)popover)->visible = VENOM_FALSE;
        venom_widget_invalidate((VenomWidget*)popover);
        
        if (popover->on_close) {
            popover->on_close(popover, VENOM_FALSE, popover->callback_data);
        }
    }
}

void venom_popover_toggle(VenomPopover* popover) {
    if (popover) {
        if (popover->is_open) {
            venom_popover_close(popover);
        } else {
            venom_popover_open(popover);
        }
    }
}

VenomBool venom_popover_is_open(const VenomPopover* popover) {
    return popover ? popover->is_open : VENOM_FALSE;
}

void venom_popover_set_on_open(VenomPopover* popover, VenomPopoverCallback cb, void* data) {
    if (popover) {
        popover->on_open = cb;
        popover->callback_data = data;
    }
}

void venom_popover_set_on_close(VenomPopover* popover, VenomPopoverCallback cb, void* data) {
    if (popover) {
        popover->on_close = cb;
        popover->callback_data = data;
    }
}

VenomWidget* _venom_popover_build(const VenomPopoverConfig* config) {
    VenomResultPtr result = venom_popover_create();
    if (!result.ok) return NULL;
    
    VenomPopover* popover = (VenomPopover*)result.value;
    
    if (config->anchor) venom_popover_set_anchor(popover, config->anchor);
    if (config->content) venom_popover_set_content(popover, config->content);
    popover->placement = config->placement;
    popover->trigger = config->trigger;
    popover->show_arrow = config->show_arrow;
    if (config->offset > 0) popover->offset = config->offset;
    
    return (VenomWidget*)popover;
}
