/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_popover.c - Popover widget implementation
 */

#include "vaxp/widgets/vaxp_popover.h"
#include "vaxp/core/vaxp_memory.h"
#include <string.h>

#define DEFAULT_OFFSET 8.0f
#define DEFAULT_ARROW_SIZE 8.0f
#define DEFAULT_CORNER_RADIUS 8.0f
#define DEFAULT_PADDING 12.0f
#define DEFAULT_SHADOW_BLUR 10.0f

/* ============================================================================
 * LIFECYCLE
 * ============================================================================ */

static void popover_init(VaxpWidget* widget) {
    VaxpPopover* popover = (VaxpPopover*)widget;
    
    popover->anchor = NULL;
    popover->content = NULL;
    
    popover->is_open = VAXP_FALSE;
    popover->placement = VAXP_POPOVER_BOTTOM;
    popover->trigger = VAXP_POPOVER_TRIGGER_CLICK;
    popover->show_arrow = VAXP_TRUE;
    popover->close_on_outside = VAXP_TRUE;
    popover->close_on_escape = VAXP_TRUE;
    
    popover->offset = DEFAULT_OFFSET;
    popover->arrow_size = DEFAULT_ARROW_SIZE;
    
    popover->background_color = (VaxpColor){ 255, 255, 255, 255 };
    popover->border_color = (VaxpColor){ 224, 224, 224, 255 };
    popover->shadow_color = (VaxpColor){ 0, 0, 0, 30 };
    popover->corner_radius = DEFAULT_CORNER_RADIUS;
    popover->border_width = 1.0f;
    popover->shadow_blur = DEFAULT_SHADOW_BLUR;
    popover->padding = DEFAULT_PADDING;
    
    popover->on_open = NULL;
    popover->on_close = NULL;
    popover->callback_data = NULL;
}

static void popover_destroy(VaxpWidget* widget) {
    VaxpPopover* popover = (VaxpPopover*)widget;
    
    if (popover->content) {
        popover->content->parent = NULL;
        vaxp_unref(popover->content);
        popover->content = NULL;
    }
    
    /* Note: anchor is not owned by popover */
    
    vaxp_widget_class.destroy(widget);
}

/* ============================================================================
 * POSITIONING
 * ============================================================================ */

static void calculate_position(VaxpPopover* popover, VaxpF32 anchor_x, VaxpF32 anchor_y,
                               VaxpF32 anchor_w, VaxpF32 anchor_h,
                               VaxpF32 content_w, VaxpF32 content_h,
                               VaxpF32* out_x, VaxpF32* out_y) {
    VaxpF32 offset = popover->offset + (popover->show_arrow ? popover->arrow_size : 0);
    
    switch (popover->placement) {
        case VAXP_POPOVER_TOP:
            *out_x = anchor_x + (anchor_w - content_w) / 2;
            *out_y = anchor_y - content_h - offset;
            break;
        case VAXP_POPOVER_TOP_START:
            *out_x = anchor_x;
            *out_y = anchor_y - content_h - offset;
            break;
        case VAXP_POPOVER_TOP_END:
            *out_x = anchor_x + anchor_w - content_w;
            *out_y = anchor_y - content_h - offset;
            break;
            
        case VAXP_POPOVER_BOTTOM:
            *out_x = anchor_x + (anchor_w - content_w) / 2;
            *out_y = anchor_y + anchor_h + offset;
            break;
        case VAXP_POPOVER_BOTTOM_START:
            *out_x = anchor_x;
            *out_y = anchor_y + anchor_h + offset;
            break;
        case VAXP_POPOVER_BOTTOM_END:
            *out_x = anchor_x + anchor_w - content_w;
            *out_y = anchor_y + anchor_h + offset;
            break;
            
        case VAXP_POPOVER_LEFT:
            *out_x = anchor_x - content_w - offset;
            *out_y = anchor_y + (anchor_h - content_h) / 2;
            break;
        case VAXP_POPOVER_LEFT_START:
            *out_x = anchor_x - content_w - offset;
            *out_y = anchor_y;
            break;
        case VAXP_POPOVER_LEFT_END:
            *out_x = anchor_x - content_w - offset;
            *out_y = anchor_y + anchor_h - content_h;
            break;
            
        case VAXP_POPOVER_RIGHT:
            *out_x = anchor_x + anchor_w + offset;
            *out_y = anchor_y + (anchor_h - content_h) / 2;
            break;
        case VAXP_POPOVER_RIGHT_START:
            *out_x = anchor_x + anchor_w + offset;
            *out_y = anchor_y;
            break;
        case VAXP_POPOVER_RIGHT_END:
            *out_x = anchor_x + anchor_w + offset;
            *out_y = anchor_y + anchor_h - content_h;
            break;
    }
}

/* ============================================================================
 * RENDERING
 * ============================================================================ */

static void popover_measure(VaxpWidget* widget, VaxpF32 available_width, VaxpF32 available_height,
                            VaxpF32* out_width, VaxpF32* out_height) {
    VaxpPopover* popover = (VaxpPopover*)widget;
    
    if (popover->content && popover->is_open) {
        VaxpF32 cw, ch;
        vaxp_widget_measure(popover->content, available_width - popover->padding * 2,
                            available_height - popover->padding * 2, &cw, &ch);
        *out_width = cw + popover->padding * 2;
        *out_height = ch + popover->padding * 2;
    } else {
        *out_width = 0;
        *out_height = 0;
    }
}

static void popover_layout(VaxpWidget* widget, VaxpRectF bounds) {
    VaxpPopover* popover = (VaxpPopover*)widget;
    widget->bounds = bounds;
    
    if (popover->content && popover->is_open) {
        VaxpRectF content_bounds = {
            popover->padding, popover->padding,
            bounds.width - popover->padding * 2,
            bounds.height - popover->padding * 2
        };
        vaxp_widget_layout(popover->content, content_bounds);
    }
}

static void draw_arrow(VaxpCanvas* canvas, VaxpPopover* popover, VaxpF32 x, VaxpF32 y, VaxpF32 w, VaxpF32 h) {
    VaxpF32 arrow = popover->arrow_size;
    VaxpPaint fill = vaxp_paint_fill(popover->background_color);
    VaxpPaint border = vaxp_paint_stroke(popover->border_color, popover->border_width);
    
    VaxpBool is_top = popover->placement >= VAXP_POPOVER_TOP && popover->placement <= VAXP_POPOVER_TOP_END;
    VaxpBool is_bottom = popover->placement >= VAXP_POPOVER_BOTTOM && popover->placement <= VAXP_POPOVER_BOTTOM_END;
    VaxpBool is_left = popover->placement >= VAXP_POPOVER_LEFT && popover->placement <= VAXP_POPOVER_LEFT_END;
    VaxpBool is_right = popover->placement >= VAXP_POPOVER_RIGHT && popover->placement <= VAXP_POPOVER_RIGHT_END;
    
    VaxpF32 ax, ay;
    
    if (is_top) {
        ax = x + w / 2;
        ay = y + h;
        /* Draw triangle pointing down */
        vaxp_canvas_draw_line(canvas, ax - arrow, ay, ax, ay + arrow, &fill);
        vaxp_canvas_draw_line(canvas, ax, ay + arrow, ax + arrow, ay, &fill);
    } else if (is_bottom) {
        ax = x + w / 2;
        ay = y;
        /* Draw triangle pointing up */
        vaxp_canvas_draw_line(canvas, ax - arrow, ay, ax, ay - arrow, &fill);
        vaxp_canvas_draw_line(canvas, ax, ay - arrow, ax + arrow, ay, &fill);
    } else if (is_left) {
        ax = x + w;
        ay = y + h / 2;
        /* Draw triangle pointing right */
        vaxp_canvas_draw_line(canvas, ax, ay - arrow, ax + arrow, ay, &fill);
        vaxp_canvas_draw_line(canvas, ax + arrow, ay, ax, ay + arrow, &fill);
    } else if (is_right) {
        ax = x;
        ay = y + h / 2;
        /* Draw triangle pointing left */
        vaxp_canvas_draw_line(canvas, ax, ay - arrow, ax - arrow, ay, &fill);
        vaxp_canvas_draw_line(canvas, ax - arrow, ay, ax, ay + arrow, &fill);
    }
    
    (void)border;  /* TODO: Draw arrow border */
}

static void popover_draw(VaxpWidget* widget, VaxpCanvas* canvas) {
    VaxpPopover* popover = (VaxpPopover*)widget;
    
    if (!popover->is_open || !popover->content) return;
    
    VaxpF32 w = widget->bounds.width;
    VaxpF32 h = widget->bounds.height;
    
    /* Draw shadow */
    for (int i = 0; i < 3; i++) {
        VaxpF32 offset = popover->shadow_blur * (0.5f + i * 0.2f);
        VaxpColor sc = popover->shadow_color;
        sc.a = (VaxpU8)(30 - i * 8);
        VaxpRectF shadow = { offset, offset, w, h };
        VaxpPaint shadow_paint = vaxp_paint_fill(sc);
        vaxp_canvas_draw_rounded_rect(canvas, shadow, popover->corner_radius, &shadow_paint);
    }
    
    /* Draw background */
    VaxpRectF bg = { 0, 0, w, h };
    VaxpPaint bg_paint = vaxp_paint_fill(popover->background_color);
    vaxp_canvas_draw_rounded_rect(canvas, bg, popover->corner_radius, &bg_paint);
    
    /* Draw border */
    if (popover->border_width > 0) {
        VaxpPaint border_paint = vaxp_paint_stroke(popover->border_color, popover->border_width);
        vaxp_canvas_draw_rounded_rect(canvas, bg, popover->corner_radius, &border_paint);
    }
    
    /* Draw arrow */
    if (popover->show_arrow) {
        draw_arrow(canvas, popover, 0, 0, w, h);
    }
    
    /* Draw content */
    if (popover->content && popover->content->visible) {
        vaxp_canvas_save(canvas);
        vaxp_canvas_translate(canvas, popover->content->bounds.x, popover->content->bounds.y);
        vaxp_widget_draw(popover->content, canvas);
        vaxp_canvas_restore(canvas);
    }
}

/* ============================================================================
 * EVENTS
 * ============================================================================ */

static VaxpBool popover_on_event(VaxpWidget* widget, const VaxpEvent* event) {
    VaxpPopover* popover = (VaxpPopover*)widget;
    
    if (!popover->is_open) return VAXP_FALSE;
    
    switch (event->type) {
        case VAXP_EVENT_KEY_DOWN:
            if (event->key.key == VAXP_KEY_ESCAPE && popover->close_on_escape) {
                vaxp_popover_close(popover);
                return VAXP_TRUE;
            }
            break;
            
        case VAXP_EVENT_MOUSE_BUTTON_DOWN:
            /* Check if click is outside popover */
            if (popover->close_on_outside) {
                VaxpF32 mx = (VaxpF32)event->mouse.x;
                VaxpF32 my = (VaxpF32)event->mouse.y;
                
                if (mx < 0 || my < 0 || mx > widget->bounds.width || my > widget->bounds.height) {
                    vaxp_popover_close(popover);
                    return VAXP_TRUE;
                }
            }
            break;
            
        default:
            break;
    }
    
    /* Pass to content */
    if (popover->content) {
        return vaxp_widget_dispatch_event(popover->content, event);
    }
    
    return VAXP_FALSE;
}

/* ============================================================================
 * CLASS
 * ============================================================================ */

const VaxpWidgetClass vaxp_popover_class = {
    .class_name = "VaxpPopover",
    .instance_size = sizeof(VaxpPopover),
    .parent_class = &vaxp_widget_class,
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

VaxpResultPtr vaxp_popover_create(void) {
    return vaxp_widget_create(&vaxp_popover_class);
}

void vaxp_popover_set_anchor(VaxpPopover* popover, VaxpWidget* anchor) {
    if (popover) {
        popover->anchor = anchor;
    }
}

VaxpResult vaxp_popover_set_content(VaxpPopover* popover, VaxpWidget* content) {
    VAXP_ENSURE_NOT_NULL(popover);
    
    if (popover->content) {
        popover->content->parent = NULL;
        vaxp_unref(popover->content);
    }
    
    popover->content = content;
    if (content) {
        vaxp_ref(content);
        content->parent = (VaxpWidget*)popover;
    }
    
    return VAXP_OK_UNIT();
}

void vaxp_popover_set_placement(VaxpPopover* popover, VaxpPopoverPlacement placement) {
    if (popover) {
        popover->placement = placement;
        if (popover->is_open) vaxp_widget_invalidate((VaxpWidget*)popover);
    }
}

void vaxp_popover_set_trigger(VaxpPopover* popover, VaxpPopoverTrigger trigger) {
    if (popover) popover->trigger = trigger;
}

void vaxp_popover_set_offset(VaxpPopover* popover, VaxpF32 offset) {
    if (popover) {
        popover->offset = offset;
        if (popover->is_open) vaxp_widget_invalidate((VaxpWidget*)popover);
    }
}

void vaxp_popover_set_arrow(VaxpPopover* popover, VaxpBool show) {
    if (popover) {
        popover->show_arrow = show;
        if (popover->is_open) vaxp_widget_invalidate((VaxpWidget*)popover);
    }
}

void vaxp_popover_open(VaxpPopover* popover) {
    if (popover && !popover->is_open) {
        popover->is_open = VAXP_TRUE;
        ((VaxpWidget*)popover)->visible = VAXP_TRUE;
        vaxp_widget_invalidate((VaxpWidget*)popover);
        
        if (popover->on_open) {
            popover->on_open(popover, VAXP_TRUE, popover->callback_data);
        }
    }
}

void vaxp_popover_close(VaxpPopover* popover) {
    if (popover && popover->is_open) {
        popover->is_open = VAXP_FALSE;
        ((VaxpWidget*)popover)->visible = VAXP_FALSE;
        vaxp_widget_invalidate((VaxpWidget*)popover);
        
        if (popover->on_close) {
            popover->on_close(popover, VAXP_FALSE, popover->callback_data);
        }
    }
}

void vaxp_popover_toggle(VaxpPopover* popover) {
    if (popover) {
        if (popover->is_open) {
            vaxp_popover_close(popover);
        } else {
            vaxp_popover_open(popover);
        }
    }
}

VaxpBool vaxp_popover_is_open(const VaxpPopover* popover) {
    return popover ? popover->is_open : VAXP_FALSE;
}

void vaxp_popover_set_on_open(VaxpPopover* popover, VaxpPopoverCallback cb, void* data) {
    if (popover) {
        popover->on_open = cb;
        popover->callback_data = data;
    }
}

void vaxp_popover_set_on_close(VaxpPopover* popover, VaxpPopoverCallback cb, void* data) {
    if (popover) {
        popover->on_close = cb;
        popover->callback_data = data;
    }
}

VaxpWidget* _vaxp_popover_build(const VaxpPopoverConfig* config) {
    VaxpResultPtr result = vaxp_popover_create();
    if (!result.ok) return NULL;
    
    VaxpPopover* popover = (VaxpPopover*)result.value;
    
    if (config->anchor) vaxp_popover_set_anchor(popover, config->anchor);
    if (config->content) vaxp_popover_set_content(popover, config->content);
    popover->placement = config->placement;
    popover->trigger = config->trigger;
    popover->show_arrow = config->show_arrow;
    if (config->offset > 0) popover->offset = config->offset;
    
    return (VaxpWidget*)popover;
}
