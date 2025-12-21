/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_split_pane.c - Split pane implementation
 */

#include "venom/widgets/venom_split_pane.h"
#include "venom/core/venom_memory.h"

#define DEFAULT_DIVIDER_SIZE 6.0f
#define DEFAULT_MIN_SIZE 50.0f

static void split_pane_init(VenomWidget* widget) {
    VenomSplitPane* pane = (VenomSplitPane*)widget;
    
    pane->first = NULL;
    pane->second = NULL;
    
    pane->direction = VENOM_SPLIT_HORIZONTAL;
    pane->position = 0.5f;
    pane->min_first = DEFAULT_MIN_SIZE;
    pane->min_second = DEFAULT_MIN_SIZE;
    pane->divider_size = DEFAULT_DIVIDER_SIZE;
    
    pane->dragging = VENOM_FALSE;
    pane->divider_color = (VenomColor){ 224, 224, 224, 255 };
    pane->divider_hover = (VenomColor){ 63, 81, 181, 255 };
    pane->hovering = VENOM_FALSE;
}

static void split_pane_destroy(VenomWidget* widget) {
    VenomSplitPane* pane = (VenomSplitPane*)widget;
    
    if (pane->first) {
        pane->first->parent = NULL;
        venom_unref(pane->first);
    }
    if (pane->second) {
        pane->second->parent = NULL;
        venom_unref(pane->second);
    }
    
    venom_widget_class.destroy(widget);
}

static void split_pane_measure(VenomWidget* widget, VenomF32 available_width, VenomF32 available_height,
                               VenomF32* out_width, VenomF32* out_height) {
    *out_width = available_width;
    *out_height = available_height;
}

static VenomF32 get_split_position(VenomSplitPane* pane, VenomF32 total_size) {
    VenomF32 pos;
    if (pane->position <= 1.0f) {
        pos = total_size * pane->position;
    } else {
        pos = pane->position;
    }
    
    if (pos < pane->min_first) pos = pane->min_first;
    if (pos > total_size - pane->min_second - pane->divider_size) {
        pos = total_size - pane->min_second - pane->divider_size;
    }
    
    return pos;
}

static void split_pane_layout(VenomWidget* widget, VenomRectF bounds) {
    VenomSplitPane* pane = (VenomSplitPane*)widget;
    widget->bounds = bounds;
    
    VenomBool horiz = (pane->direction == VENOM_SPLIT_HORIZONTAL);
    VenomF32 total = horiz ? bounds.width : bounds.height;
    VenomF32 split_pos = get_split_position(pane, total);
    
    if (pane->first && pane->first->visible) {
        VenomRectF first_bounds;
        if (horiz) {
            first_bounds = (VenomRectF){ 0, 0, split_pos, bounds.height };
        } else {
            first_bounds = (VenomRectF){ 0, 0, bounds.width, split_pos };
        }
        venom_widget_layout(pane->first, first_bounds);
    }
    
    if (pane->second && pane->second->visible) {
        VenomRectF second_bounds;
        if (horiz) {
            second_bounds = (VenomRectF){ 
                split_pos + pane->divider_size, 0, 
                bounds.width - split_pos - pane->divider_size, bounds.height 
            };
        } else {
            second_bounds = (VenomRectF){ 
                0, split_pos + pane->divider_size,
                bounds.width, bounds.height - split_pos - pane->divider_size 
            };
        }
        venom_widget_layout(pane->second, second_bounds);
    }
}

static void split_pane_draw(VenomWidget* widget, VenomCanvas* canvas) {
    VenomSplitPane* pane = (VenomSplitPane*)widget;
    
    VenomBool horiz = (pane->direction == VENOM_SPLIT_HORIZONTAL);
    VenomF32 total = horiz ? widget->bounds.width : widget->bounds.height;
    VenomF32 split_pos = get_split_position(pane, total);
    
    /* Draw first panel */
    if (pane->first && pane->first->visible) {
        venom_canvas_save(canvas);
        venom_canvas_translate(canvas, pane->first->bounds.x, pane->first->bounds.y);
        venom_widget_draw(pane->first, canvas);
        venom_canvas_restore(canvas);
    }
    
    /* Draw divider */
    VenomRectF divider;
    if (horiz) {
        divider = (VenomRectF){ split_pos, 0, pane->divider_size, widget->bounds.height };
    } else {
        divider = (VenomRectF){ 0, split_pos, widget->bounds.width, pane->divider_size };
    }
    
    VenomColor div_color = (pane->hovering || pane->dragging) ? pane->divider_hover : pane->divider_color;
    VenomPaint div_paint = venom_paint_fill(div_color);
    venom_canvas_draw_rect(canvas, divider, &div_paint);
    
    /* Draw grip */
    VenomF32 grip_x = divider.x + divider.width / 2;
    VenomF32 grip_y = divider.y + divider.height / 2;
    VenomPaint grip_paint = venom_paint_fill((VenomColor){ 150, 150, 150, 255 });
    
    if (horiz) {
        for (int i = -2; i <= 2; i++) {
            venom_canvas_draw_circle(canvas, grip_x, grip_y + i * 6, 2, &grip_paint);
        }
    } else {
        for (int i = -2; i <= 2; i++) {
            venom_canvas_draw_circle(canvas, grip_x + i * 6, grip_y, 2, &grip_paint);
        }
    }
    
    /* Draw second panel */
    if (pane->second && pane->second->visible) {
        venom_canvas_save(canvas);
        venom_canvas_translate(canvas, pane->second->bounds.x, pane->second->bounds.y);
        venom_widget_draw(pane->second, canvas);
        venom_canvas_restore(canvas);
    }
}

static VenomBool is_on_divider(VenomSplitPane* pane, VenomF32 mx, VenomF32 my) {
    VenomWidget* widget = (VenomWidget*)pane;
    VenomBool horiz = (pane->direction == VENOM_SPLIT_HORIZONTAL);
    VenomF32 total = horiz ? widget->bounds.width : widget->bounds.height;
    VenomF32 split_pos = get_split_position(pane, total);
    
    if (horiz) {
        return mx >= split_pos && mx <= split_pos + pane->divider_size;
    } else {
        return my >= split_pos && my <= split_pos + pane->divider_size;
    }
}

static VenomBool split_pane_on_event(VenomWidget* widget, const VenomEvent* event) {
    VenomSplitPane* pane = (VenomSplitPane*)widget;
    
    switch (event->type) {
        case VENOM_EVENT_MOUSE_MOVE: {
            VenomF32 mx = (VenomF32)event->mouse.x;
            VenomF32 my = (VenomF32)event->mouse.y;
            
            if (pane->dragging) {
                VenomBool horiz = (pane->direction == VENOM_SPLIT_HORIZONTAL);
                VenomF32 total = horiz ? widget->bounds.width : widget->bounds.height;
                VenomF32 pos = horiz ? mx : my;
                
                pane->position = pos / total;
                if (pane->position < 0.1f) pane->position = 0.1f;
                if (pane->position > 0.9f) pane->position = 0.9f;
                
                venom_widget_invalidate_layout(widget);
                return VENOM_TRUE;
            }
            
            VenomBool on_div = is_on_divider(pane, mx, my);
            if (on_div != pane->hovering) {
                pane->hovering = on_div;
                widget->needs_redraw = VENOM_TRUE;
            }
            break;
        }
        
        case VENOM_EVENT_MOUSE_BUTTON_DOWN:
            if (event->mouse.button == VENOM_MOUSE_BUTTON_LEFT) {
                if (is_on_divider(pane, (VenomF32)event->mouse.x, (VenomF32)event->mouse.y)) {
                    pane->dragging = VENOM_TRUE;
                    return VENOM_TRUE;
                }
            }
            break;
            
        case VENOM_EVENT_MOUSE_BUTTON_UP:
            if (pane->dragging) {
                pane->dragging = VENOM_FALSE;
                return VENOM_TRUE;
            }
            break;
            
        case VENOM_EVENT_MOUSE_LEAVE:
            pane->hovering = VENOM_FALSE;
            pane->dragging = VENOM_FALSE;
            widget->needs_redraw = VENOM_TRUE;
            break;
            
        default:
            break;
    }
    
    /* Pass events to panels */
    if (pane->first && venom_widget_dispatch_event(pane->first, event)) return VENOM_TRUE;
    if (pane->second && venom_widget_dispatch_event(pane->second, event)) return VENOM_TRUE;
    
    return VENOM_FALSE;
}

const VenomWidgetClass venom_split_pane_class = {
    .class_name = "VenomSplitPane",
    .instance_size = sizeof(VenomSplitPane),
    .parent_class = &venom_widget_class,
    .init = split_pane_init,
    .destroy = split_pane_destroy,
    .measure = split_pane_measure,
    .layout = split_pane_layout,
    .draw = split_pane_draw,
    .on_event = split_pane_on_event,
    .on_state_changed = NULL,
};

VenomResultPtr venom_split_pane_create(void) {
    return venom_widget_create(&venom_split_pane_class);
}

VenomResult venom_split_pane_set_first(VenomSplitPane* pane, VenomWidget* widget) {
    VENOM_ENSURE_NOT_NULL(pane);
    
    if (pane->first) {
        pane->first->parent = NULL;
        venom_unref(pane->first);
    }
    
    pane->first = widget;
    if (widget) {
        venom_ref(widget);
        widget->parent = (VenomWidget*)pane;
    }
    
    return VENOM_OK_UNIT();
}

VenomResult venom_split_pane_set_second(VenomSplitPane* pane, VenomWidget* widget) {
    VENOM_ENSURE_NOT_NULL(pane);
    
    if (pane->second) {
        pane->second->parent = NULL;
        venom_unref(pane->second);
    }
    
    pane->second = widget;
    if (widget) {
        venom_ref(widget);
        widget->parent = (VenomWidget*)pane;
    }
    
    return VENOM_OK_UNIT();
}

void venom_split_pane_set_position(VenomSplitPane* pane, VenomF32 position) {
    if (pane) {
        pane->position = position;
        venom_widget_invalidate_layout((VenomWidget*)pane);
    }
}

void venom_split_pane_set_direction(VenomSplitPane* pane, VenomSplitDirection dir) {
    if (pane) {
        pane->direction = dir;
        venom_widget_invalidate_layout((VenomWidget*)pane);
    }
}

void venom_split_pane_set_min_sizes(VenomSplitPane* pane, VenomF32 min_first, VenomF32 min_second) {
    if (pane) {
        pane->min_first = min_first;
        pane->min_second = min_second;
    }
}

VenomWidget* _venom_split_pane_build(const VenomSplitPaneConfig* config) {
    VenomResultPtr result = venom_split_pane_create();
    if (!result.ok) return NULL;
    
    VenomSplitPane* pane = (VenomSplitPane*)result.value;
    
    if (config->first) venom_split_pane_set_first(pane, config->first);
    if (config->second) venom_split_pane_set_second(pane, config->second);
    pane->direction = config->direction;
    if (config->position > 0) pane->position = config->position;
    
    return (VenomWidget*)pane;
}
